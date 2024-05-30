#include "ToolsGuiPch.hpp"
#include "tScaleGizmoGeometry.hpp"

namespace Sig
{

	tScaleGizmoGeometry::tScaleGizmoGeometry( 
		const Gfx::tDevicePtr& device,
		const Gfx::tMaterialPtr& material, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		fResetDeviceObjects( device, material, geometryAllocator, indexAllocator );
		fGenerate( );
	}

	void tScaleGizmoGeometry::fOnDeviceReset( Gfx::tDevice* device )
	{
		fGenerate( );
	}

	void tScaleGizmoGeometry::fGenerate( )
	{
		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		tGrowableArray< u16 > ids;

		const u32 numBoxesPerAxis = 1;
		const u32 numFacesPerBox = 6;
		const u32 numVertsPerFace = 4;
		const u32 numVertsPerAxis = numVertsPerFace * numFacesPerBox * numBoxesPerAxis;
		const f32 longAxisLen = 1.f;
		const f32 shortAxisLen = 0.075f;

		for( u32 iaxis = 0; iaxis < 3; ++iaxis )
		{
			Math::tVec3f x = Math::tVec3f::cZeroVector;
			x.fAxis( iaxis ) = longAxisLen;

			Math::tVec3f y = Math::tVec3f::cZeroVector;
			y.fAxis( ( iaxis + 1 ) % 3 ) = shortAxisLen;

			Math::tVec3f z = Math::tVec3f::cZeroVector;
			z.fAxis( ( iaxis + 2 ) % 3 ) = shortAxisLen;

			// create big box
			fAddBoxFace( y - z, -y - z, -y + z, y + z, verts, ids, iaxis );
			fAddBoxFace( y + z, -y + z, x - y + z, x + y + z, verts, ids, iaxis );
			fAddBoxFace( y - z, y + z, x + y + z, x + y - z, verts, ids, iaxis );
			fAddBoxFace( x + y + z, x - y + z, x - y - z, x + y - z, verts, ids, iaxis );
			fAddBoxFace( x + y - z, x - y - z, -y - z, y - z, verts, ids, iaxis );
			fAddBoxFace( x - y - z, x - y + z, -y + z, -y - z, verts, ids, iaxis );

			// create small box
			const Math::tVec3f sy = 1.5f * y;
			const Math::tVec3f sz = 1.5f * z;
			const Math::tVec3f min = x;
			const Math::tVec3f max = x + 2.f * sy.fLength( ) * Math::tVec3f( x ).fNormalize( );
			fAddBoxFace( min + sy - sz, min - sy - sz, min - sy + sz, min + sy + sz, verts, ids, iaxis );
			fAddBoxFace( min + sy + sz, min - sy + sz, max - sy + sz, max + sy + sz, verts, ids, iaxis );
			fAddBoxFace( min + sy - sz, min + sy + sz, max + sy + sz, max + sy - sz, verts, ids, iaxis );
			fAddBoxFace( max + sy + sz, max - sy + sz, max - sy - sz, max + sy - sz, verts, ids, iaxis );
			fAddBoxFace( max + sy - sz, max - sy - sz, min - sy - sz, min + sy - sz, verts, ids, iaxis );
			fAddBoxFace( max - sy - sz, max - sy + sz, min - sy + sz, min - sy - sz, verts, ids, iaxis );
		}

		//const f32 webbingLength = longAxisLen * 0.75f;
		//fAddTwoAxisWebbing( webbingLength, verts, ids );

		// create box for 3-axis scaling
		const Math::tVec3f sy = 0.25f * Math::tVec3f::cYAxis;
		const Math::tVec3f sz = 0.25f * Math::tVec3f::cZAxis;
		const Math::tVec3f min = -0.25f * Math::tVec3f::cXAxis + 0.125f * Math::tVec3f::cOnesVector;
		const Math::tVec3f max = +0.25f * Math::tVec3f::cXAxis + 0.125f * Math::tVec3f::cOnesVector;
		const u32 threeAxisColor = Gfx::tVertexColor( 0xff, 0xff, 0xff, 0x88 ).fForGpu( );
		fAddBoxFace( min + sy - sz, min - sy - sz, min - sy + sz, min + sy + sz, verts, ids, threeAxisColor, mAxisTris[ cAxisXYZ ] );
		fAddBoxFace( min + sy + sz, min - sy + sz, max - sy + sz, max + sy + sz, verts, ids, threeAxisColor, mAxisTris[ cAxisXYZ ] );
		fAddBoxFace( min + sy - sz, min + sy + sz, max + sy + sz, max + sy - sz, verts, ids, threeAxisColor, mAxisTris[ cAxisXYZ ] );
		fAddBoxFace( max + sy + sz, max - sy + sz, max - sy - sz, max + sy - sz, verts, ids, threeAxisColor, mAxisTris[ cAxisXYZ ] );
		fAddBoxFace( max + sy - sz, max - sy - sz, min - sy - sz, min + sy - sz, verts, ids, threeAxisColor, mAxisTris[ cAxisXYZ ] );
		fAddBoxFace( max - sy - sz, max - sy + sz, min - sy + sz, min - sy - sz, verts, ids, threeAxisColor, mAxisTris[ cAxisXYZ ] );

		fBake( verts, ids );
	}

	void tScaleGizmoGeometry::fAddBoxFace( 
		const Math::tVec3f& v0, const Math::tVec3f& v1, const Math::tVec3f& v2, const Math::tVec3f& v3,
		tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
		tGrowableArray< u16 >& ids,
		u32 iaxis )
	{
		const Math::tVec3f n = ( v1 - v0 ).fCross( v2 - v0 ).fNormalizeSafe( );
		const f32 shade = fClamp( 0.5f + fMax( 0.f, n.fDot( Math::tVec3f( 1.0f, 0.5f, 2.0f ).fNormalize( ) ) ), 0.f, 1.f );
		const u32 axisColor = Gfx::tVertexColor( iaxis==0?shade:0.f, iaxis==1?shade:0.f, iaxis==2?shade:0.f ).fForGpu( );

		fAddBoxFace( v0, v1, v2, v3, verts, ids, axisColor, mAxisTris[ iaxis ] );
	}

	void tScaleGizmoGeometry::fAddBoxFace( 
		const Math::tVec3f& v0, const Math::tVec3f& v1, const Math::tVec3f& v2, const Math::tVec3f& v3,
		tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
		tGrowableArray< u16 >& ids,
		const u32 vtxColor,
		tTriangleArray& triArray )
	{
		const u32 baseVertex = verts.fCount( );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( v0, vtxColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( v1, vtxColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( v2, vtxColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( v3, vtxColor ) );

		ids.fPushBack( baseVertex + 0 );
		ids.fPushBack( baseVertex + 1 );
		ids.fPushBack( baseVertex + 2 );
		fAddLastTriangle( triArray, verts, ids );

		ids.fPushBack( baseVertex + 2 );
		ids.fPushBack( baseVertex + 3 );
		ids.fPushBack( baseVertex + 0 );
		fAddLastTriangle( triArray, verts, ids );
	}

}

