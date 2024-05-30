#include "BasePch.hpp"
#include "tMutableTexturedQuad.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tDevice.hpp"

namespace Sig { namespace Gui
{

	tMutableTexturedQuad::tMutableTexturedQuad( )
	{
	}

	tMutableTexturedQuad::tMutableTexturedQuad( Gfx::tDefaultAllocators& allocators )
		: tTexturedQuad( allocators )
	{
	}

	tMutableTexturedQuad::~tMutableTexturedQuad( )
	{
	}

	void tMutableTexturedQuad::fSetVertexPos( u32 index, const Math::tVec2f& pos )
	{
	}

	void tMutableTexturedQuad::fSetVertexUV( u32 index, const Math::tVec2f& uv )
	{
		if( !fInBounds( index, 0u, 3u ) )
			return;

		const Math::tVec2f uvOffset	= Math::tVec2f( 0.5f / fTextureDimensions( ).x, 0.5f / fTextureDimensions( ).y );

		mVerts[ index ].mUv = uv + uvOffset;
		mGeometry.fCopyVertsToGpu( mVerts.fBegin( ), mVerts.fCount( ) );
	}

	namespace
	{
		b32 fIsUnderHalf( f32 angle, f32 half, u32 radialDirection )
		{
			return (radialDirection == cCLOCKWISE)? angle < half : angle > half;
		}

		b32 fIsBefore( f32 angle, f32 start, u32 radialDirection )
		{
			return (radialDirection == cCLOCKWISE)? angle < start : angle > start;
		}

		b32 fIsAfter( f32 angle, f32 end, u32 radialDirection )
		{
			return (radialDirection == cCLOCKWISE)? angle > end : angle < end;
		}
	}

	void tMutableTexturedQuad::fSetAngleLayout( u32 pivotIndex, f32 angle, const tRect& uvCoords, const tRect& size, u32 radialDirection )
	{
		u32 directionIndex = (radialDirection == cCLOCKWISE)? 0 : 1;
		const u32 cornerCount = 4;

		if( !fInBounds( pivotIndex, 0u, 3u ) )
			return;

		const f32 angleBounds[2][cornerCount][2] = {
			{
				{ 0, Math::cPiOver2 },
				{ Math::cPiOver2, Math::cPi },
				{ Math::c3PiOver2, Math::c2Pi },
				{ Math::cPi, Math::c3PiOver2} 
			},
			{
				{ Math::cPiOver2, 0 },
				{ Math::cPi, Math::cPiOver2 },
				{ Math::c2Pi, Math::c3PiOver2 },
				{ Math::c3PiOver2, Math::cPi } 
			},
		};

		f32 startAngle = angleBounds[ directionIndex ][ pivotIndex ][ 0 ];
		f32 endAngle = angleBounds[ directionIndex ][ pivotIndex ][ 1 ];
		const f32 halfAngle = startAngle + (endAngle - startAngle) / 2.0f;

		if( radialDirection == cCLOCKWISE )
			angle = fWrap( angle, 0.0f, Math::c2Pi );
		else
			angle = fWrap( -angle, 0.0f, Math::c2Pi );

		const Math::tVec2f uvOffset	= Math::tVec2f( 0.5f / fTextureDimensions( ).x, 0.5f / fTextureDimensions( ).y );
		const Math::tVec2f uvCorners[cornerCount] = {
			uvCoords.fTopLeft( ) + uvOffset,
			uvCoords.fTopRight( ) + uvOffset,
			uvCoords.fBottomLeft( ) + uvOffset,
			uvCoords.fBottomRight( ) + uvOffset
		};

		const u32 oppositeCorner[ cornerCount ] = { 3, 2, 1, 0 };
		u32 cwCorner[ cornerCount ] = { 1, 3, 0, 2 };
		u32 ccwCorner[ cornerCount ] = { 2, 0, 3, 1 };

		if( radialDirection == cCOUNTERCLOCKWISE )
		{
			for( u32 i = 0; i < cornerCount; ++i )
				fSwap( cwCorner[ i ], ccwCorner[ i ] );
		}

		const Math::tVec3f cornerCoords[ cornerCount ] = {
			Math::tVec3f( size.fTopLeft( ), 0.0f ),
			Math::tVec3f( size.fTopRight( ), 0.0f ),
			Math::tVec3f( size.fBottomLeft( ), 0.0f ),
			Math::tVec3f( size.fBottomRight( ), 0.0f )
		};

		// Trivial solutions
		if( fIsBefore( angle, startAngle, radialDirection ) )
		{
			const Math::tVec3f c = cornerCoords[ pivotIndex ];
			const Math::tVec2f uv = uvCorners[ cwCorner[ pivotIndex ] ];
			for( u32 i = 0; i < cornerCount; ++i )
			{
				mVerts[ i ].mP = c;
				mVerts[ i ].mUv = uv;
			}

			mGeometry.fCopyVertsToGpu( mVerts.fBegin( ), mVerts.fCount( ) );
			return;
		}
		else if( fIsAfter( angle, endAngle, radialDirection ) ) // If it's greater than the full angle show the full rectangle
		{
			fSetRect( size );
			fSetTextureRect( uvCoords.fTopLeft( ), uvCoords.fWidthHeight( ) );
			return;
		}

		const f32 cosAngle = Math::fCos( angle );
		const f32 sinAngle = Math::fSin( angle );
		sigassert( fAbs( cosAngle ) > 0.f );
		const f32 m = sinAngle / cosAngle;

		f32 t = 0.0f;
		if( fAbs( m ) > 1.0f )
		{
			if( angle < halfAngle )
				t = -1.0f / m;
			else
				t = 1 - (1.0f / m);
		}
		else
		{
			if( angle < halfAngle )
				t = m;
			else
				t = 1 - (-m);
		}

		if( radialDirection == cCOUNTERCLOCKWISE )
			t = 1 - t;

		if( fIsUnderHalf( angle, halfAngle, radialDirection ) )
		{
			const Math::tVec3f point = Math::fLerp( cornerCoords[ cwCorner[ pivotIndex ] ], cornerCoords[ oppositeCorner[ pivotIndex ] ], t );
			const Math::tVec2f uv = Math::fLerp( uvCorners[ cwCorner[ pivotIndex ] ], uvCorners[ oppositeCorner[ pivotIndex ] ], t );

			mVerts[ pivotIndex					].mP = cornerCoords[ pivotIndex ];
			mVerts[ cwCorner[ pivotIndex ]		].mP = cornerCoords[ cwCorner[ pivotIndex ] ];
			mVerts[ oppositeCorner[ pivotIndex ]	].mP = point;
			mVerts[ ccwCorner[ pivotIndex ]		].mP = point;

			mVerts[ pivotIndex					].mUv = uvCorners[ pivotIndex ];
			mVerts[ cwCorner[ pivotIndex ]		].mUv = uvCorners[ cwCorner[ pivotIndex ] ];
			mVerts[ oppositeCorner[ pivotIndex ]	].mUv = uv;
			mVerts[ ccwCorner[ pivotIndex ]		].mUv = uv;
		}
		else //if( angle > halfAngle )
		{
			const Math::tVec3f point = Math::fLerp( cornerCoords[ oppositeCorner[ pivotIndex ] ], cornerCoords[ ccwCorner[ pivotIndex ] ], t );
			const Math::tVec2f uv = Math::fLerp( uvCorners[ oppositeCorner[ pivotIndex ] ], uvCorners[ ccwCorner[ pivotIndex ] ], t );

			mVerts[ pivotIndex					].mP = cornerCoords[ pivotIndex ];
			mVerts[ cwCorner[ pivotIndex ]		].mP = cornerCoords[ cwCorner[ pivotIndex ] ];
			mVerts[ oppositeCorner[ pivotIndex ]	].mP = cornerCoords[ oppositeCorner[ pivotIndex ] ];
			mVerts[ ccwCorner[ pivotIndex ]		].mP = point;

			mVerts[ pivotIndex					].mUv = uvCorners[ pivotIndex ];
			mVerts[ cwCorner[ pivotIndex ]		].mUv = uvCorners[ cwCorner[ pivotIndex ] ];
			mVerts[ oppositeCorner[ pivotIndex ]	].mUv = uvCorners[ oppositeCorner[ pivotIndex ] ];
			mVerts[ ccwCorner[ pivotIndex ]		].mUv = uv;
		}

		// Done
		mGeometry.fCopyVertsToGpu( mVerts.fBegin( ), mVerts.fCount( ) );
	}

}}

namespace Sig { namespace Gui
{

	void tMutableTexturedQuad::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tMutableTexturedQuad, tTexturedQuad, Sqrat::NoCopy<tTexturedQuad> > classDesc( vm.fSq( ) );

		classDesc
			.Func( "SetVertexPos", &tMutableTexturedQuad::fSetVertexPos)
			.Func( "SetVertexUV", &tMutableTexturedQuad::fSetVertexUV )
			.Func( "SetAngleLayout", &tMutableTexturedQuad::fSetAngleLayout )
		;

		vm.fNamespace(_SC("Gui")).Bind(_SC("MutableTexturedQuad"), classDesc);
		vm.fConstTable( ).Const( _SC("DIRECTION_CLOCKWISE"), cCLOCKWISE );
		vm.fConstTable( ).Const( _SC("DIRECTION_COUNTERCLOCKWISE"), cCOUNTERCLOCKWISE );
	}

}}
