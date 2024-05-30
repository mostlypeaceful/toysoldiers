#include "ToolsGuiPch.hpp"
#include "tGizmoGeometry.hpp"
#include "Gfx/tDevice.hpp"
#include "Math/tIntersectionRayTriangle.hpp"

namespace Sig
{

	tGizmoGeometry::tGizmoGeometry( )
		: tDynamicGeometry( Gfx::tRenderBatchData::cBehaviorIgnoreStats )
		, mRenderState( Gfx::tRenderState::cDefaultColorTransparent )
	{
		mRenderState.fEnableDisable( Gfx::tRenderState::cDepthBuffer, true );
		mRenderState.fEnableDisable( Gfx::tRenderState::cDepthWrite, true );
		fSetRenderStateOverride( &mRenderState );
		fSetPrimTypeOverride( Gfx::tIndexFormat::cPrimitiveTriangleList );
	}

	tGizmoGeometry::~tGizmoGeometry( )
	{
	}

	void tGizmoGeometry::fResetDeviceObjects( 
		const Gfx::tDevicePtr& device,
		const Gfx::tMaterialPtr& material, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		mMaterial = material;
		tDynamicGeometry::fResetDeviceObjects( geometryAllocator, indexAllocator );
		fRegisterWithDevice( device.fGetRawPtr( ) );
	}

	void tGizmoGeometry::fRayCast( const Math::tRayf& rayInLocal, Math::tVec3f& intersectionPointInLocal, b32& x, b32& y, b32& z )
	{
		x = false;
		y = false;
		z = false;
		u32 bestAxis = ~0;

		f32 bestT = Math::cInfinity;

		for( u32 iaxis = 0; iaxis < mAxisTris.fCount( ); ++iaxis )
		{
			for( u32 itri = 0; itri < mAxisTris[ iaxis ].fCount( ); ++itri )
			{
				Math::tIntersectionRayTriangle<f32> intersection( rayInLocal, mAxisTris[ iaxis ][ itri ] );
				if( intersection.fIntersects( ) && intersection.fT( ) < bestT )
				{
					bestT = intersection.fT( );
					bestAxis = iaxis;
				}
			}
		}

		if( bestAxis < mAxisTris.fCount( ) )
		{
			intersectionPointInLocal = rayInLocal.fPointAtTime( bestT );
			switch( bestAxis )
			{
			case cAxisX: x = true; break;
			case cAxisY: y = true; break;
			case cAxisZ: z = true; break;
			case cAxisXY: x = true; y = true; break;
			case cAxisXZ: x = true; z = true; break;
			case cAxisYZ: y = true; z = true; break;
			case cAxisXYZ: x = true; y = true; z = true; break;
			default: sigassert( !"invalid axis!" ); break;
			}
		}
	}

	void tGizmoGeometry::fAddLastTriangle( 
		tTriangleArray& triArray,
		const tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
		const tGrowableArray< u16 >& ids )
	{
		triArray.fPushBack( Math::tTrianglef( 
			verts[ ids[ ids.fCount( ) - 3 ] ].mP, 
			verts[ ids[ ids.fCount( ) - 2 ] ].mP, 
			verts[ ids[ ids.fCount( ) - 1 ] ].mP ) );
	}

	void tGizmoGeometry::fAddTwoAxisWebbing( 
		f32 webbingLength,
		tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
		tGrowableArray< u16 >& ids )
	{
		const u32 twoAxisColor = Gfx::tVertexColor( 0xff, 0xff, 0x11, 0x88 ).fForGpu( );

		const f32 halfWeb = webbingLength * 0.5;
		const f32 quarterWeb = webbingLength * 0.15f;

		// add tris for x - y
		verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cZeroVector, twoAxisColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cXAxis * webbingLength, twoAxisColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cYAxis * webbingLength, twoAxisColor ) );
		ids.fPushBack( verts.fCount( ) - 3 );
		ids.fPushBack( verts.fCount( ) - 2 );
		ids.fPushBack( verts.fCount( ) - 1 );
		fAddLastTriangle( mAxisTris[ cAxisXY ], verts, ids );
		ids.fPushBack( verts.fCount( ) - 2 );
		ids.fPushBack( verts.fCount( ) - 3 );
		ids.fPushBack( verts.fCount( ) - 1 );
		fAddLastTriangle( mAxisTris[ cAxisXY ], verts, ids );

		// add tris for x - z
		verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cZeroVector - Math::tVec3f::cXAxis * halfWeb - Math::tVec3f::cZAxis * quarterWeb, twoAxisColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cZeroVector - Math::tVec3f::cZAxis * halfWeb - Math::tVec3f::cXAxis * quarterWeb, twoAxisColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cZAxis * webbingLength, twoAxisColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cXAxis * webbingLength, twoAxisColor ) );
		ids.fPushBack( verts.fCount( ) - 3 );
		ids.fPushBack( verts.fCount( ) - 2 );
		ids.fPushBack( verts.fCount( ) - 1 );
		fAddLastTriangle( mAxisTris[ cAxisXZ ], verts, ids );
		ids.fPushBack( verts.fCount( ) - 2 );
		ids.fPushBack( verts.fCount( ) - 3 );
		ids.fPushBack( verts.fCount( ) - 1 );
		fAddLastTriangle( mAxisTris[ cAxisXZ ], verts, ids );
		ids.fPushBack( verts.fCount( ) - 4 );
		ids.fPushBack( verts.fCount( ) - 3 );
		ids.fPushBack( verts.fCount( ) - 2 );
		fAddLastTriangle( mAxisTris[ cAxisXZ ], verts, ids );
		ids.fPushBack( verts.fCount( ) - 3 );
		ids.fPushBack( verts.fCount( ) - 4 );
		ids.fPushBack( verts.fCount( ) - 2 );
		fAddLastTriangle( mAxisTris[ cAxisXZ ], verts, ids );


		// add tris for y - z
		verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cZeroVector, twoAxisColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cYAxis * webbingLength, twoAxisColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cZAxis * webbingLength, twoAxisColor ) );
		ids.fPushBack( verts.fCount( ) - 3 );
		ids.fPushBack( verts.fCount( ) - 2 );
		ids.fPushBack( verts.fCount( ) - 1 );
		fAddLastTriangle( mAxisTris[ cAxisYZ ], verts, ids );
		ids.fPushBack( verts.fCount( ) - 2 );
		ids.fPushBack( verts.fCount( ) - 3 );
		ids.fPushBack( verts.fCount( ) - 1 );
		fAddLastTriangle( mAxisTris[ cAxisYZ ], verts, ids );
	}

	void tGizmoGeometry::fBake( const tGrowableArray<Gfx::tSolidColorRenderVertex>& verts, const tGrowableArray<u16>& ids )
	{
		if( !fAllocateGeometry( *mMaterial, verts.fCount( ), ids.fCount( ), ids.fCount( ) / 3 ) )
			return; // couldn't get geometry

		// copy vert data to gpu
		Sig::byte* gpuVerts = fGeometry( ).mBuffer->fQuickLock( fGeometry( ).mStartVertex, fGeometry( ).mNumVerts );
		sigassert( gpuVerts );
		fMemCpy( gpuVerts, verts.fBegin( ), verts.fCount( ) * fGeometry( ).mBuffer->fVertexFormat( ).fVertexSize( ) );
		fGeometry( ).mBuffer->fQuickUnlock( gpuVerts );

		// generate indices
		Sig::byte* gpuIndices = fIndices( ).mBuffer->fQuickLock( fIndices( ).mStartIndex, fIndices( ).mNumIds );
		sigassert( gpuIndices );
		fMemCpy( gpuIndices, ids.fBegin( ), ids.fCount( ) * sizeof( ids[ 0 ] ) );
		fIndices( ).mBuffer->fQuickUnlock( gpuIndices );
	}

}

