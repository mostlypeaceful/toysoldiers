#include "BasePch.hpp"
#include "tTexturedQuad.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tDevice.hpp"

namespace Sig { namespace Gui
{
	tTexturedQuad::tTexturedQuad( )
		: mAllocators( Gfx::tDefaultAllocators::fInstance( ) )
	{
		fCommonCtor( );
	}
	tTexturedQuad::tTexturedQuad( Gfx::tDefaultAllocators& allocators )
		: mAllocators( allocators )
	{
		fCommonCtor( );
	}
	void tTexturedQuad::fCommonCtor( )
	{
		mTextureDimensions = Math::tVec2f::cZeroVector;
		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mGeometry.fSetRenderStateOverride( &mRenderState );
		mGeometry.fSetPrimTypeOverride( Gfx::tIndexFormat::cPrimitiveTriangleList );
		fResetDeviceObjects( *Gfx::tDevice::fGetDefaultDevice( ) );
		fSetRect( tRect( ) );
		fRegisterWithDevice( Gfx::tDevice::fGetDefaultDevice( ).fGetRawPtr( ) );
	}
	b32 tTexturedQuad::fHasTexture( ) const
	{
		return mColorMap && mColorMap->fLoaded( );
	}
	void tTexturedQuad::fSetTexture( const tFilePathPtr& texturePath )
	{
		tResourcePtr colorMap = mAllocators.mResourceDepot->fQuery( tResourceId::fMake<Gfx::tTextureFile>( texturePath ) );
		fSetTexture( colorMap );
	}
	void tTexturedQuad::fSetTexture( Gfx::tTextureReference::tPlatformHandle handle, const Math::tVec2f & dims )
	{
		sigassert( mMaterial );
		mMaterial->mColorMap.fSetRaw( handle );
		mMaterial->mColorMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeClamp );
		mTextureDimensions = dims;

		fSetRectFromTexture( );
	}
	void tTexturedQuad::fSetTexture( const tResourcePtr& textureResource )
	{
		log_assert( textureResource && textureResource->fLoaded( ), textureResource->fGetPath( ) << " wasn't loaded in ScreenSpaceQuad.SetTexture." );

		mColorMap = textureResource;
		sigassert( mMaterial );
		mMaterial->mColorMap.fSetDynamic( mColorMap.fGetRawPtr( ) );
		mMaterial->mColorMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeClamp );

		fSetRectFromTexture( );
	}
	Math::tVec2f tTexturedQuad::fTextureDimensions( ) const
	{
		const Gfx::tTextureFile* textureFile = mColorMap ? mColorMap->fCast< Gfx::tTextureFile >( ) : 0;
		if( textureFile )
			return Math::tVec2f( ( f32 )textureFile->mWidth, ( f32 )textureFile->mHeight );
		else if( mMaterial->mColorMap.fGetRaw( ) )
			return mTextureDimensions;
		else
			return Math::tVec2f::cZeroVector;
	}
	void tTexturedQuad::fResetDeviceObjects( Gfx::tDevice& device )
	{
		mMaterial.fReset( NEW Gfx::tFullBrightMaterial( ) );
		mMaterial->fSetMaterialFileResourcePtrOwned( mAllocators.mFullBrightMaterialFile );

		mGeometry.fResetDeviceObjects( mAllocators.mFullBrightGeomAllocator, mAllocators.mIndexAllocator );
		mGeometry.fChangeMaterial( *mMaterial );

		// generate indices (as these never change)
		const u16 indices[] = { 0, 2, 1, 1, 2, 3 };
		mGeometry.fAllocateIndices( *mMaterial, array_length( indices ), 2 );
		mGeometry.fCopyIndicesToGpu( indices, array_length( indices ) );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
		fRegisterWithDevice( &device );
	}
	void tTexturedQuad::fOnDeviceReset( Gfx::tDevice *device )
	{
		mGeometry.fResetDeviceObjects( mAllocators.mFullBrightGeomAllocator, mAllocators.mIndexAllocator );
		mGeometry.fChangeMaterial( *mMaterial );

		// generate indices (as these never change)
		const u16 indices[] = { 0, 2, 1, 1, 2, 3 };
		mGeometry.fAllocateIndices( *mMaterial, array_length( indices ), 2 );
		mGeometry.fCopyIndicesToGpu( indices, array_length( indices ) );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );


		if (mLastSetRectMode == 1)
			fSetRect( mLastSetRect );
		else if (mLastSetRectMode == 2)
		{
			fSetRect( mLastSetRect );
			fSetTextureRect( mLastSetCorner, mLastSetExtent );
		}
	}

	void tTexturedQuad::fCenterPivot( )
	{
		fSetRectFromTexture( true );
	}
	void tTexturedQuad::fSetPivot( const Math::tVec2f& point )
	{
		const Math::tVec2f texDims = fTextureDimensions( );
		fSetRect( tRect( -point, texDims ) );
	}
	void tTexturedQuad::fSetRectFromTexture( b32 center )
	{
		const Math::tVec2f texDims = fTextureDimensions( );
		if( center )
			fSetRect( tRect( -0.5f * texDims, texDims ) );
		else
			fSetRect( tRect( texDims ) );
	}
	void tTexturedQuad::fSetRect( const Math::tVec2f& topLeft, const Math::tVec2f& widthHeight )
	{
		fSetRect( tRect( topLeft, widthHeight ) );
	}
	void tTexturedQuad::fSetRect( const Math::tVec2f& widthHeight )
	{
		fSetRect( tRect( widthHeight ) );
	}
	void tTexturedQuad::fSetRect( const tRect& rect )
	{
		mLastSetRectMode = 1;
		mLastSetRect = rect;

		fSetBounds( rect );

		const Math::tVec2f topLeft = rect.fTopLeft( );
		const Math::tVec2f widthHeight = rect.fWidthHeight( );

		const u32 numVerts	= 4;

		if( !mGeometry.fAllocateVertices( *mMaterial, numVerts ) )
		{
			fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
			return; // couldn't get geometry
		}

		f32 xOffset	= topLeft.x;
		f32 yOffset	= topLeft.y;
		f32 width	= widthHeight.x;
		f32 height	= widthHeight.y;

		const Math::tVec2f uvCorners[4] = {	Math::tVec2f( 0.f, 0.f ),
											Math::tVec2f( 1.f, 0.f ),
											Math::tVec2f( 0.f, 1.f ),
											Math::tVec2f( 1.f, 1.f ) };

		// copy vert data to gpu
		const Math::tVec2f uvOffset	= Math::tVec2f( 0.5f / fTextureDimensions( ).x, 0.5f / fTextureDimensions( ).y );

		sigassert( mGeometry.fGetRenderBatch( )->fBatchData( ).mGeometryBuffer->fVertexFormat( ).fVertexSize( ) == sizeof( Gfx::tFullBrightRenderVertex ) );

		// we're using a full bright material
		mVerts[ 0 ].mP = Math::tVec3f( xOffset,				yOffset,				0.f );
		mVerts[ 0 ].mColor = 0xffffffff;
		mVerts[ 0 ].mUv = uvCorners[ 0 ] + uvOffset;
		mVerts[ 1 ].mP = Math::tVec3f( xOffset + width,		yOffset,				0.f );
		mVerts[ 1 ].mColor = 0xffffffff;
		mVerts[ 1 ].mUv = uvCorners[ 1 ] + uvOffset;
		mVerts[ 2 ].mP = Math::tVec3f( xOffset,				yOffset + height,		0.f );
		mVerts[ 2 ].mColor = 0xffffffff;
		mVerts[ 2 ].mUv = uvCorners[ 2 ] + uvOffset;
		mVerts[ 3 ].mP = Math::tVec3f( xOffset + width,		yOffset + height,		0.f );
		mVerts[ 3 ].mColor = 0xffffffff;
		mVerts[ 3 ].mUv = uvCorners[ 3 ] + uvOffset;
		mGeometry.fCopyVertsToGpu( mVerts.fBegin( ), mVerts.fCount( ) );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}
	void tTexturedQuad::fSetTextureRect( const Math::tVec2f& corner, const Math::tVec2f& extent )
	{
		mLastSetRectMode = 2;
		mLastSetCorner = corner;
		mLastSetExtent = extent;

		const Math::tVec2f uvOffset	= Math::tVec2f( 0.5f / fTextureDimensions( ).x, 0.5f / fTextureDimensions( ).y );

		//const Math::tVec2f uvCorners[4] = {	Math::tVec2f( 0.f, 0.f ),
		//	Math::tVec2f( 1.f, 0.f ),
		//	Math::tVec2f( 0.f, 1.f ),
		//	Math::tVec2f( 1.f, 1.f ) };
		const Math::tVec2f uvCorners[4] = {	corner,
			corner + Math::tVec2f( extent.x, 0.f ),
			corner + Math::tVec2f( 0.f, extent.y ),
			corner + Math::tVec2f( extent.x, extent.y ) };

		// we're using a full bright material
		for( u32 i = 0; i < 4; ++i )
			mVerts[ i ].mUv = uvCorners[ i ] + uvOffset;
		mGeometry.fCopyVertsToGpu( mVerts.fBegin( ), mVerts.fCount( ) );
	}
	void tTexturedQuad::fSetFilterMode( Gfx::tTextureFile::tFilterMode filterMode )
	{
		mMaterial->mColorMap.fSetSamplingModes( filterMode, mMaterial->mColorMap.fAddressModeU( ) );
	}
	void tTexturedQuad::fAutoConfigureSamplingModes( )
	{
		const b32 a = ( fWorldXform( ).mAngle != 0.f );
		const b32 aOrB = a || ( fWorldXform( ).mScale != Math::tVec2f::cOnesVector );
		const b32 aOrBOrC = aOrB || !fEqual( 0.f, std::fmod( fWorldXform( ).mPosition.x, 1.f ) + std::fmod( fWorldXform( ).mPosition.y, 1.f ) );

		if( aOrBOrC )
			fSetFilterMode( Gfx::tTextureFile::cFilterModeWithMip );
		else
			fSetFilterMode( Gfx::tTextureFile::cFilterModeWithMip );
	}
	void tTexturedQuad::fOnMoved( )
	{
		tRenderableCanvas::fOnMoved( );
    // we want to MIP on PC because resolutions aren't always just 720p
		//const b32 autoSample = true; // TODO REFACTOR consider making this a parameter of the class
		//if( autoSample )
			//fAutoConfigureSamplingModes( );
	}
	void tTexturedQuad::fOnParentMoved( )
	{
		tRenderableCanvas::fOnParentMoved( );
    // we want to MIP on PC because resolutions aren't always just 720p
		//const b32 autoSample = true; // TODO REFACTOR consider making this a parameter of the class
		//if( autoSample )
			//fAutoConfigureSamplingModes( );
	}
}}

namespace Sig { namespace Gui
{
	void tTexturedQuad::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tTexturedQuad, tRenderableCanvas, Sqrat::NoCopy<tTexturedQuad> > classDesc( vm.fSq( ) );

		classDesc
			.Func<void (tTexturedQuad::*)(const tFilePathPtr&)>( "SetTexture", &tTexturedQuad::fSetTexture )
			.Overload<void (tTexturedQuad::*)(const Math::tVec2f&, const Math::tVec2f&)>( "SetRect", &tTexturedQuad::fSetRect )
			.Overload<void (tTexturedQuad::*)(const Math::tVec2f&)>( "SetRect", &tTexturedQuad::fSetRect )
			// Don't enable next line: currently (v.0.8.1) Sqrat can't distiniguish by type, the overloads must have a different number of parameters
					//.Overload<void (tTexturedQuad::*)(const tRect&)>( "SetRect", &fSetRect )
			.Func( "TextureDimensions", &tTexturedQuad::fTextureDimensions )	
			.Func( "SetPivot",	&tTexturedQuad::fSetPivot)
			.Func( "CenterPivot", &tTexturedQuad::fCenterPivot )
			.Func( "SetTextureRect", &tTexturedQuad::fSetTextureRect )
			.Prop(_SC("ColorMap"), &tTexturedQuad::fColorMapScript)
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("TexturedQuad"), classDesc);
	}
}}
