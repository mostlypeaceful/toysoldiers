#include "BasePch.hpp"
#include "tColorPicker.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tDevice.hpp"
#include "tFont.hpp"
#include "Gfx/tScreen.hpp"

using namespace Sig::Math;

namespace Sig { namespace Gui
{

	tColorPickerQuad::tColorPickerQuad( )
		: mAllocators( Gfx::tDefaultAllocators::fInstance( ) )
	{
		fCommonCtor( );
	}
	tColorPickerQuad::tColorPickerQuad( Gfx::tDefaultAllocators& allocators )
		: mAllocators( allocators )
	{
		fCommonCtor( );
	}
	void tColorPickerQuad::fCommonCtor( )
	{
		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mGeometry.fSetRenderStateOverride( &mRenderState );
		mGeometry.fSetPrimTypeOverride( Gfx::tIndexFormat::cPrimitiveTriangleList );
		fResetDeviceObjects( *Gfx::tDevice::fGetDefaultDevice( ) );
		fSetRect( Math::tRect( ) );
	}
	tColorPickerQuad::~tColorPickerQuad( )
	{
	}
	void tColorPickerQuad::fOnDeviceReset( Gfx::tDevice* device )
	{
		fSetRect( fLocalXform( ).mRect );
	}
	void tColorPickerQuad::fSetRect( const tVec2f& topLeft, const tVec2f& widthHeight )
	{
		fSetRect( Math::tRect( topLeft, widthHeight ) );
	}
	void tColorPickerQuad::fSetRect( const tVec2f& widthHeight )
	{
		fSetRect( Math::tRect( widthHeight ) );
	}
	void tColorPickerQuad::fSetRect( const Math::tRect& rect )
	{
		fSetBounds( rect );
		fUpdateGeometry( );
	}
	void tColorPickerQuad::fResetDeviceObjects( Gfx::tDevice& device )
	{
		mMaterial.fReset( mAllocators.mSolidColorMaterial->fDynamicCast< Gfx::tSolidColorMaterial >( ) );

		mGeometry.fResetDeviceObjects( mAllocators.mSolidColorGeomAllocator, mAllocators.mIndexAllocator );
		mGeometry.fChangeMaterial( *mMaterial );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
		fRegisterWithDevice( &device );
	}

	// http://en.wikipedia.org/wiki/HSL_and_HSV#Converting_to_RGB
	// http://code.activestate.com/recipes/576919-python-rgb-and-hsv-conversion/
	tVec3f tColorPickerQuad::fRGBFromHSV( const tVec3f& hsv )
	{
		f32 chroma = hsv.y * hsv.z;
		f32 hPrime = hsv.x * 6;
		f32 x = chroma * (1 - fAbs( fmod( hPrime, 2.f ) - 1 ));
		
		tVec3f rgb( chroma, x, 0 ); //fallback
		if( hPrime < 1.f )		rgb = tVec3f( chroma, x, 0 );
		else if( hPrime < 2.f ) rgb = tVec3f( x, chroma, 0 );
		else if( hPrime < 3.f ) rgb = tVec3f( 0, chroma, x );
		else if( hPrime < 4.f ) rgb = tVec3f( 0, x, chroma );
		else if( hPrime < 5.f ) rgb = tVec3f( x, 0, chroma );
		else if( hPrime < 6.f ) rgb = tVec3f( chroma, 0, x );

		f32 m = hsv.z - chroma;
		return rgb + tVec3f( m );
	}

	tVec3f tColorPickerQuad::fHSVFromRGB( const tVec3f& rgb )
	{
		f32 h = 0;
		f32 s = 0;
		f32 v = 0;

		f32 max = rgb.fMax( );
		f32 min = rgb.fMin( );
		f32 df = max-min;

		if( max == min )
			h = 0;
		else if( max == rgb.x )
			h = fmod( (60 * (rgb.y-rgb.z)/df + 360), 360 );
		else if( max == rgb.y )
			h = fmod( (60 * (rgb.z-rgb.x)/df + 120), 360 );
		else if( max == rgb.z )
			h = fmod( (60 * (rgb.x-rgb.y)/df + 240), 360 );

		if( max > 0 )
			s = df/max;

		v = max;

		return tVec3f( h / 360.f, s, v );
	}

	void tColorPickerQuad::fUpdateGeometry( )
	{
		const tRectf& rect = fLocalXform( ).mRect;
		const tVec2f topLeft = rect.fTopLeft( );
		const tVec2f widthHeight = rect.fWidthHeight( );

		//// color unit tests
		//{
		//	sigassert( fEqual( tVec3f(1,0,0), fRGBFromHSV( tVec3f( 0,1.f, 1.f ) ) ) );

		//	tVec3f testVec( 0.235f, 0.5f, 0.25f );
		//	sigassert( fEqual( testVec, fHSVFromRGB( fRGBFromHSV( testVec ) ) ) );
		//}


		// This is a slightly denser quad.
		/*
		 Verts - 
		  0---1---2---3
		  |\  |\  |\  |
		  |  \|  \|  \|
		  4---5---6---7
		*/

		// need to highly tessellate this, or the bottom left tri's will look fucked up.
		//  Take the case of 0,4,5. it doesnt have the color of 1 to consider.
		const u32 numAcross = 200;

		const u32 numPrims = (numAcross-1) * 2;
		const u32 numVerts = numAcross * 2;

		tGrowableArray<u16> indices;
		indices.fSetCount( numPrims * 3 );

		for( u32 i = 0; i < numAcross - 1; ++i )
		{
			u32 startIndex = i * 6;
			indices[ startIndex + 0 ] = i;
			indices[ startIndex + 1 ] = i + numAcross;
			indices[ startIndex + 2 ] = i + numAcross +  1;

			indices[ startIndex + 3 ] = i;
			indices[ startIndex + 4 ] = i + numAcross + 1;
			indices[ startIndex + 5 ] = i + 1;
		}

		mGeometry.fAllocateIndices( *mMaterial, indices.fCount( ), numPrims );
		mGeometry.fCopyIndicesToGpu( indices.fBegin( ), indices.fCount( ) );

		if( !mGeometry.fAllocateVertices( *mMaterial, numVerts ) )
		{
			fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
			return; // couldn't get geometry
		}

		sigassert( mGeometry.fGetRenderBatch( )->fBatchData( ).mGeometryBuffer->fVertexFormat( ).fVertexSize( ) == sizeof( Gfx::tSolidColorRenderVertex ) );

		// we're using a solid color material
		tGrowableArray<Gfx::tSolidColorRenderVertex> verts;
		verts.fSetCount( numVerts );

		u32 numAcrossMinusOne = numAcross - 1;
		f32 widthStep = widthHeight.x / numAcrossMinusOne;

		for( u32 i = 0; i < numAcross; ++i )
		{
			f32 colorAngle = (f32)i / numAcrossMinusOne;
			f32 xPos = topLeft.x + i * widthStep;

			verts[ i ].mP = tVec3f( xPos, topLeft.y, 0.f );
			verts[ i ].mColor = Gfx::tVertexColor( tVec4f( fRGBFromHSV( tVec3f( colorAngle, 1.f, 1.f ) ), 1.f ) ).fForGpu( );
			
			verts[ i + numAcross ].mP = tVec3f( xPos, topLeft.y + widthHeight.y, 0.f );
			verts[ i + numAcross ].mColor = Gfx::tVertexColor( tVec4f(0,0,0,1) ).fForGpu( );
		}

		mGeometry.fCopyVertsToGpu( verts.fBegin( ), verts.fCount( ) );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

	tColorPicker::tColorPicker( )
		: mPos( tVec3f::cZeroVector )
		, mRect( tVec2f( (f32)cDefaultSize ) )
		, mHasFont( false )
		, mLineHeight( 10 )
		, mShowAlpha( true )
		, mMin( 0 )
		, mMax( 1 )
		, mIntensity( 1 )
	{ 
		fSetColor( tVec4f( 0.75f, 0.6f, 0, 0.5f ), 0, 1 );

		tVec4f white( 1,1,1,1 );
		tVec4f black( 0,0,0,1 );
		tVec4f red( 1,0,0,1 );
		tVec4f gray( 0.4f,0.4f,0.4f,1 );
		mSaturationCursor.fSetVertColors( black, black, black, black );
		mColorCursor.fSetVertColors( white, white, white, white );
		mAlphaBar.fSetVertColors( black, white, black, white );
		mAlphaCursor.fSetVertColors( red, red, red, red );
		mIntensityBar.fSetVertColors( white, white, gray, gray );
		mIntensityCursor.fSetVertColors( red, red, red, red );
	}

	void tColorPicker::fSetPosition( const tVec3f& pos )
	{
		mPos = pos;
		fResize( );
	}

	void tColorPicker::fSetRect( const tVec2f& rect )
	{
		mRect = rect;
		fResize( );
	}

	void tColorPicker::fSetFont( const tResourcePtr& smallFont )
	{
		for( u32 i = 0; i < mValues.fCount( ); ++i )
			mValues[ i ].fSetFont( smallFont );
		mInstructions.fSetFont( smallFont );
		mIntensityText.fSetFont( smallFont );
		mHasFont = true;
		mLineHeight = smallFont->fCast<Gui::tFont>( )->mDesc.mLineHeight;
		fResize( );
	}

	void tColorPicker::fResize( )
	{
		mColorPicker.fSetRect( mRect );
		mColorPicker.fSetPosition( mPos );

		mSatPos = mPos + tVec3f( mRect.x + cSpacer, 0, 0 );
		mSatRect = tVec2f( (f32)cSaturationWidth, mRect.y );
		mSaturation.fSetPosition( mSatPos );
		mSaturation.fSetRect( mSatRect );

		const f32 cPreviewStart = (mRect.y - cPreviewSize) / 2.f;
		tVec3f prevPos = mSatPos + tVec3f( (f32)cSaturationWidth + cSpacer, (f32)cPreviewStart, 0 );
		mPreview.fSetPosition( prevPos );
		mPreview.fSetRect( tVec2f( (f32)cPreviewSize ) );

		mIntensityPos = mSatPos + tVec3f( cSaturationWidth + cPreviewSize + cSpacer*2, 0, 0 );
		mIntensityBar.fSetPosition( mIntensityPos );
		mIntensityBar.fSetRect( mSatRect );
		mIntensityText.fSetPosition( mIntensityPos + tVec3f( -30, -(f32)mLineHeight, 0 ) );
		if( mHasFont )
			mIntensityText.fBakeBox( cInfinity, L"Extra Intensity:", 0, Gui::tText::cAlignLeft );

		mSaturationCursor.fSetRect( tVec2f( (f32)cSaturationWidth, (f32)cCursorWidth ) );
		mIntensityCursor.fSetRect( tVec2f( (f32)cSaturationWidth, (f32)cCursorWidth ) );
		mColorCursor.fSetRect( tVec2f( (f32)cCursorWidth ) );

		tVec3f valPos = mPos + tVec3f( 0, mRect.y + 4, 0 );
		for( u32 i = 0; i < mValues.fCount( ); ++i )
			mValues[ i ].fSetPosition( valPos + tVec3f(55.f * i,0,0) );

		mAlphaPos = valPos + tVec3f( 0, (f32)mLineHeight * 2 + 10, 0 );
		mAlphaBar.fSetRect( tVec2f( mRect.x, (f32)cSaturationWidth ) );
		mAlphaBar.fSetPosition( mAlphaPos );

		mAlphaCursor.fSetRect( tVec2f( (f32)cCursorWidth, (f32)cSaturationWidth ) );

		fRefreshColor( );
	}

	void tColorPicker::fDraw( Gfx::tScreen& screen, b32 showAlpha )
	{
		// this doesnt seem to work. trying to remove text pop
		//if( mShowAlpha != showAlpha )
		//	fRefreshColor( ); //rebakes text

		mShowAlpha = showAlpha;

		screen.fAddScreenSpaceDrawCall( mColorPicker.fDrawCall( ) );
		screen.fAddScreenSpaceDrawCall( mSaturation.fDrawCall( ) );
		screen.fAddScreenSpaceDrawCall( mPreview.fDrawCall( ) );
		screen.fAddScreenSpaceDrawCall( mSaturationCursor.fDrawCall( ) );
		screen.fAddScreenSpaceDrawCall( mColorCursor.fDrawCall( ) );
		screen.fAddScreenSpaceDrawCall( mInstructions.fDrawCall( ) );

		u32 components = 3;
		if( mShowAlpha )
		{
			components = 4;
			screen.fAddScreenSpaceDrawCall( mAlphaBar.fDrawCall( ) );
			screen.fAddScreenSpaceDrawCall( mAlphaCursor.fDrawCall( ) );
		}

		for( u32 i = 0; i < components; ++i )
			screen.fAddScreenSpaceDrawCall( mValues[ i ].fDrawCall( ) );

		if( mMax > 1.f )
		{
			screen.fAddScreenSpaceDrawCall( mIntensityBar.fDrawCall( ) );
			screen.fAddScreenSpaceDrawCall( mIntensityCursor.fDrawCall( ) );
			screen.fAddScreenSpaceDrawCall( mIntensityText.fDrawCall( ) );			
		}
	}

	void tColorPicker::fSetColor( const tVec4f& rgba, f32 min, f32 max )
	{
		mRGBA = rgba;
		mMin = min;
		mMax = max;
		fRefreshColor( );
	}

	void tColorPicker::fSetColor( const tVec3f& rgb, f32 min, f32 max )
	{
		mRGBA = tVec4f( rgb, mRGBA.w );
		mMin = min;
		mMax = max;
		fRefreshColor( );
	}

	void tColorPicker::fIncrementColor( tVec3f& rgb, const tVec3f& hsvDelta, f32 min, f32 max )
	{
		tVec3f hsv = Gui::tColorPickerQuad::fHSVFromRGB( rgb );
		hsv += hsvDelta;

		// dont let saturation go to zero completely or it will destroy your hue.
		// dont let hue go to 1.0, or it will wrap.
		tVec3f minVec = fMin( tVec3f( min ), tVec3f(0,0.01f,0) );
		hsv = fClamp( hsv, minVec, tVec3f( 0.995f, 1.f, max ) );
		rgb = Gui::tColorPickerQuad::fRGBFromHSV( hsv );
	}

	namespace
	{
		void f255Number( std::wstringstream& ss, const char* prefix, u32 zeroTo255 )
		{
			ss << prefix << std::fixed << std::setprecision( 0 ) << std::setfill( L' ' )  << std::setw( 4 ) << zeroTo255;
		}

		void fZeroOneNumber( std::wstringstream& ss, const char* prefix, f32 zeroToOne )
		{
			ss << prefix << std::fixed << std::setprecision( 3 ) << std::setw( 4 ) << zeroToOne;
		}

		void fComponent( std::wstringstream& ss, const char* prefix, f32 zeroToOne, u32 zeroTo255 )
		{
			ss.clear( );
			fZeroOneNumber( ss, prefix, zeroToOne );
			ss << std::endl;
			f255Number( ss, prefix, zeroTo255 );
		}
	}

	void tColorPicker::fRefreshColor( )
	{
		// extract intensity
		tVec3f rgb = mRGBA.fXYZ( );
		f32 maxValue = rgb.fMaxMagnitude( );
		mIntensity = fMax( maxValue, 1.f );
		rgb /= mIntensity;
		maxValue /= mIntensity;

		// Compute colors involved in saturation bar
		tVec3f hsv = Gui::tColorPickerQuad::fHSVFromRGB( rgb );
		tVec4f fullSaturation( Gui::tColorPickerQuad::fRGBFromHSV( tVec3f( hsv.x, 1.f, hsv.z ) ), 1 );
		tVec4f noSaturation( Gui::tColorPickerQuad::fRGBFromHSV( tVec3f( hsv.x, 0.f, hsv.z ) ), 1 );

		// modulate the saturation cursor so it's inverse of the current color.
		tVec4f satCursorColor( tVec3f( 1 - maxValue ), 1.f );
		mSaturationCursor.fSetVertColors( satCursorColor, satCursorColor, satCursorColor, satCursorColor );
		mSaturation.fSetVertColors( fullSaturation, fullSaturation, noSaturation, noSaturation );

		// display the extra intensity (values over 1, in a separate, read-only output)
		tVec3f satPos( mSatPos.x, mPos.y + (1.f - hsv.y) * mRect.y, 0 );
		mSaturationCursor.fSetPosition( satPos );

		if( mMax > 1 )
		{
			f32 intensityRange = mMax - 1.f;
			f32 intensityLerp = (mIntensity - 1.f) / intensityRange;
			f32 intesityPos = mRect.y * (1.f - intensityLerp);
			mIntensityCursor.fSetPosition( mIntensityPos + tVec3f( 0,intesityPos,0 ) );
		}

		tVec3f colPos( mRect * tVec2f( hsv.x, (1.f-hsv.z) ), 0 );
		colPos += mPos;
		mColorCursor.fSetPosition( colPos );

		tVec4f previewColor( rgb, 1.f );
		mPreview.fSetVertColors( previewColor, previewColor, previewColor, previewColor );

		mAlphaCursor.fSetPosition( mAlphaPos + tVec3f( mRect.x * mRGBA.w, 0, 0 ) );

		if( mHasFont )
		{
			const char* cPrefixes [ 4 ] = { "R: ", "G: ", "B: ", "A: " };
			const u32 componentCount = mShowAlpha ? 4 : 3;
			const tVec4u zeroTo255( u32(rgb.x * 255), u32(rgb.y * 255), u32(rgb.z * 255), u32(mRGBA.w * 255) );

			for( u32 i = 0; i < componentCount; ++i )
			{
				std::wstringstream ss;
				fComponent( ss, cPrefixes[ i ], mRGBA[ i ], zeroTo255[ i ] );
				mValues[ i ].fBakeBox( cInfinity, ss.str( ).c_str( ), 0, Gui::tText::cAlignLeft );
			}

			u32 iLineCnt = 1;
			std::wstringstream instructions;
			instructions << "Color: LStick. Saturation: RStick.";
			if( mShowAlpha )
			{
				iLineCnt = 2;
				instructions << std::endl << "    Alpha: Bumpers.";
			}

			mInstructions.fBakeBox( cInfinity, instructions.str( ).c_str( ), 0, Gui::tText::cAlignLeft );
			mInstructions.fSetPosition( mPos + tVec3f( 0, -(f32)mLineHeight * iLineCnt, 0 ) );
		}
	}
}}