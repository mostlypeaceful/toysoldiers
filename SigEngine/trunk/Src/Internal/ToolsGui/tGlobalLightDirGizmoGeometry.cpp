#include "ToolsGuiPch.hpp"
#include "tGlobalLightDirGizmoGeometry.hpp"

namespace Sig
{
	tGlobalLightDirGizmoGeometry::tGlobalLightDirGizmoGeometry( 
		const Gfx::tDevicePtr& device,
		const Gfx::tMaterialPtr& material, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		fResetDeviceObjects( device, material, geometryAllocator, indexAllocator );
		fGenerate( );
	}

	void tGlobalLightDirGizmoGeometry::fOnDeviceLost( Gfx::tDevice* device )
	{
	}

	void tGlobalLightDirGizmoGeometry::fOnDeviceReset( Gfx::tDevice* device )
	{
		fGenerate( );
	}

	void tGlobalLightDirGizmoGeometry::fGenerate( )
	{
		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		tGrowableArray< u16 > ids;

		const u32 numSlices = 16;
		const u32 numVertsPerAxis = 2 + 2 * numSlices;
		const f32 longAxisLen = 1.f;
		const f32 shortAxisLen = 0.1f;

		for( u32 iaxis = 0; iaxis < 1; ++iaxis )
		{
			const u32 axisColor = Gfx::tVertexColor( iaxis==0?1.f:0.f, iaxis==1?1.f:0.f, iaxis==2?1.f:0.f ).fForGpu( );

			const u32 iu = ( iaxis + 1 ) % 3;
			const u32 iv = ( iaxis + 2 ) % 3;

			// first we create the cylinder

			Math::tVec3f centralAxis = Math::tVec3f::cZeroVector;
			centralAxis.fAxis( iaxis ) = longAxisLen;

			Math::tVec3f radialU = Math::tVec3f::cZeroVector;
			radialU.fAxis( iu ) = shortAxisLen;

			Math::tVec3f radialV = Math::tVec3f::cZeroVector;
			radialV.fAxis( iv ) = shortAxisLen;

			// insert both "end-points" first
			verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f::cZeroVector, axisColor ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( 1.25f * centralAxis, axisColor ) );

			// insert the two "end-circles"
			for( u32 islice = 0; islice < numSlices; ++islice )
			{
				const f32 u = std::sin( Math::c2Pi * ( f32 )islice / ( f32 )numSlices );
				const f32 v = std::cos( Math::c2Pi * ( f32 )islice / ( f32 )numSlices );
				const Math::tVec3f p = u * radialU + v * radialV;
				const Math::tVec3f n = Math::tVec3f( p ).fNormalize( );

				const f32 shade = fMax( 0.f, 0.5f * n.x + n.y ) + 0.5f;
				const u32 axisColor = Gfx::tVertexColor( iaxis==0?shade:0.f, iaxis==1?shade:0.f, iaxis==2?shade:0.f ).fForGpu( );

				verts.fPushBack( Gfx::tSolidColorRenderVertex( p, axisColor ) );
				verts.fPushBack( Gfx::tSolidColorRenderVertex( p + centralAxis, axisColor ) );
			}

			// create the triangles
			for( u32 islice = 0; islice < numSlices; ++islice )
			{
				const u32 baseVertex = iaxis * numVertsPerAxis;

				const u32 prevSlice = ( islice == 0 ) ? numSlices - 1 : islice - 1;
				const u32 currSlice = islice;

				ids.fPushBack( baseVertex + 0 );
				ids.fPushBack( baseVertex + 2 + prevSlice * 2 );
				ids.fPushBack( baseVertex + 2 + currSlice * 2 );
				fAddLastTriangle( mAxisTris[ iaxis ], verts, ids );

				ids.fPushBack( baseVertex + 2 + prevSlice * 2 );
				ids.fPushBack( baseVertex + 2 + prevSlice * 2 + 1 );
				ids.fPushBack( baseVertex + 2 + currSlice * 2 );
				fAddLastTriangle( mAxisTris[ iaxis ], verts, ids );

				ids.fPushBack( baseVertex + 2 + currSlice * 2 );
				ids.fPushBack( baseVertex + 2 + prevSlice * 2 + 1 );
				ids.fPushBack( baseVertex + 2 + currSlice * 2 + 1 );
				fAddLastTriangle( mAxisTris[ iaxis ], verts, ids );

				ids.fPushBack( baseVertex + 1 );
				ids.fPushBack( baseVertex + 2 + currSlice * 2 + 1 );
				ids.fPushBack( baseVertex + 2 + prevSlice * 2 + 1 );
				fAddLastTriangle( mAxisTris[ iaxis ], verts, ids );
			}
		}

		fBake( verts, ids );
	}

}

