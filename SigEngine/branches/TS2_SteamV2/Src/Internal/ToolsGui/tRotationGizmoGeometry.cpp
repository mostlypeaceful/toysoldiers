#include "ToolsGuiPch.hpp"
#include "tRotationGizmoGeometry.hpp"

namespace Sig
{
	tRotationGizmoGeometry::tRotationGizmoGeometry( 
		const Gfx::tDevicePtr& device,
		const Gfx::tMaterialPtr& material, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		fResetDeviceObjects( device, material, geometryAllocator, indexAllocator );
		fGenerate( );
	}

	void tRotationGizmoGeometry::fOnDeviceReset( Gfx::tDevice* device )
	{
		fGenerate( );
	}

	void tRotationGizmoGeometry::fGenerate( )
	{

		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		tGrowableArray< u16 > ids;

		const u32 numRings = 32;
		const u32 numSlices = 16;
		const u32 numVertsPerAxis = numRings * numSlices;

		for( s32 iaxis = 2; iaxis >= 0; --iaxis )
		{
			const u32 axisColor = Gfx::tVertexColor( iaxis==0?1.f:0.f, iaxis==1?1.f:0.f, iaxis==2?1.f:0.f ).fForGpu( );

			// outer basis axis indices
			const u32 iu0 = ( iaxis + 2 ) % 3;
			const u32 iv0 = ( iaxis + 0 ) % 3;

			const f32 outerRadius = 1.0f;
			const f32 innerRadius = 0.05f;

			// setup basis for outer circle
			Math::tVec3f outerU = Math::tVec3f::cZeroVector;
			outerU.fAxis( iu0 ) = 1.0f;
			Math::tVec3f outerV = Math::tVec3f::cZeroVector;
			outerV.fAxis( iv0 ) = 1.0f;

			// inner basis axis indices
			const u32 iu1 = ( iaxis + 0 ) % 3;
			const u32 iv1 = ( iaxis + 1 ) % 3;

			// step around the outer circle
			for( u32 iring = 0; iring < numRings; ++iring )
			{
				// compute center of inner circle
				const f32 u = std::sin( Math::c2Pi * ( f32 )iring / ( f32 )numRings );
				const f32 v = std::cos( Math::c2Pi * ( f32 )iring / ( f32 )numRings );
				const Math::tVec3f c = ( u * outerU + v * outerV ).fNormalize( ) * ( outerRadius - ( 2 * innerRadius ) * iaxis );

				const Math::tVec3f ringU = -innerRadius * Math::tVec3f( c ).fNormalizeSafe( );
				Math::tVec3f ringV = Math::tVec3f::cZeroVector;
				ringV.fAxis( iv1 ) = innerRadius;

				// step around inner circle
				for( u32 islice = 0; islice < numSlices; ++islice )
				{
					const f32 angle = Math::c2Pi * ( f32 )islice / ( f32 )numSlices;
					const f32 s = std::sin( angle );
					const f32 t = std::cos( angle );
					const Math::tVec3f p = c + s * ringU + t * ringV;

					// shader the inner part of the circle dark
					const f32 shade = ( p.fLength( ) - c.fLength( ) + innerRadius ) / ( 2.f * innerRadius );
					const u32 axisColor = Gfx::tVertexColor( iaxis==0?shade:0.f, iaxis==1?shade:0.f, iaxis==2?shade:0.f ).fForGpu( );

					// add vertex
					verts.fPushBack( Gfx::tSolidColorRenderVertex( p, axisColor ) );
				}
			}

			// create the triangles
			for( u32 iring = 0; iring < numRings; ++iring )
			{
				const u32 baseVertex = ( 2 - iaxis ) * numVertsPerAxis;
				const u32 prevRing = ( iring == 0 ) ? numRings - 1 : iring - 1;
				const u32 currRing = iring;

				for( u32 islice = 0; islice < numSlices; ++islice )
				{
					const u32 currSlice = islice;
					const u32 nextSlice = ( islice == ( numSlices - 1 ) ) ? 0 : islice + 1;

					ids.fPushBack( baseVertex + prevRing * numSlices + currSlice );
					ids.fPushBack( baseVertex + prevRing * numSlices + nextSlice );
					ids.fPushBack( baseVertex + currRing * numSlices + currSlice );
					fAddLastTriangle( mAxisTris[ iaxis ], verts, ids );

					ids.fPushBack( baseVertex + currRing * numSlices + currSlice );
					ids.fPushBack( baseVertex + prevRing * numSlices + nextSlice );
					ids.fPushBack( baseVertex + currRing * numSlices + nextSlice );
					fAddLastTriangle( mAxisTris[ iaxis ], verts, ids );
				}
			}
		}

		fBake( verts, ids );
	}

}

