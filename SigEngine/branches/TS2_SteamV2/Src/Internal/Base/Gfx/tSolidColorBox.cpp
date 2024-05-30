#include "BasePch.hpp"
#include "tSolidColorBox.hpp"
#include "tSolidColorMaterial.hpp"

#include "Math/tIntersectionRayAabb.hpp"
#include "Math/tIntersectionAabbFrustum.hpp"

namespace Sig { namespace Gfx
{
	tSolidColorBox::tSolidColorBox( )
		: mHalfEdgeLength( 1.f )
	{
		fSetPrimTypeOverride( tIndexFormat::cPrimitiveTriangleList );
	}

	void tSolidColorBox::fOnDeviceReset( tDevice* device )
	{
		fGenerate( mHalfEdgeLength );
	}

	void tSolidColorBox::fGenerate( f32 halfEdgeLen )
	{
		mHalfEdgeLength = halfEdgeLen;

		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		tGrowableArray< u16 > ids;

		const Math::tVec3f sx = mHalfEdgeLength * Math::tVec3f::cXAxis;
		const Math::tVec3f sy = mHalfEdgeLength * Math::tVec3f::cYAxis;
		const Math::tVec3f sz = mHalfEdgeLength * Math::tVec3f::cZAxis;
		fAddBoxFace( -sx + sy - sz, -sx - sy - sz, -sx - sy + sz, -sx + sy + sz, verts, ids );
		fAddBoxFace( -sx + sy + sz, -sx - sy + sz,  sx - sy + sz,  sx + sy + sz, verts, ids );
		fAddBoxFace( -sx + sy - sz, -sx + sy + sz,  sx + sy + sz,  sx + sy - sz, verts, ids );
		fAddBoxFace(  sx + sy + sz,  sx - sy + sz,  sx - sy - sz,  sx + sy - sz, verts, ids );
		fAddBoxFace(  sx + sy - sz,  sx - sy - sz, -sx - sy - sz, -sx + sy - sz, verts, ids );
		fAddBoxFace(  sx - sy - sz,  sx - sy + sz, -sx - sy + sz, -sx - sy - sz, verts, ids );

		fBake( verts, ids, ids.fCount( ) / 3 );
	}

	b32 tSolidColorBox::fIntersectsRay( const Math::tRayf& ray, f32& tOut ) const
	{
		const Math::tAabbf aabb = fGetBounds( );

		Math::tIntersectionRayAabb<f32> intersection( ray, aabb );
		if( intersection.fIntersects( ) )
		{
			tOut = intersection.fT( );
			return true;
		}

		return false;
	}

	b32 tSolidColorBox::fIntersectsFrustum( const Math::tFrustumf& frustum ) const
	{
		const Math::tAabbf aabb = fGetBounds( );

		Math::tIntersectionAabbFrustum<f32> intersection( frustum, aabb );
		if( intersection.fIntersects( ) )
		{
			return true;
		}

		return false;
	}

	void tSolidColorBox::fAddBoxFace( 
		const Math::tVec3f& v0, const Math::tVec3f& v1, const Math::tVec3f& v2, const Math::tVec3f& v3,
		tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
		tGrowableArray< u16 >& ids )
	{
		const Math::tVec3f n = ( v1 - v0 ).fCross( v2 - v0 ).fNormalizeSafe( Math::tVec3f::cZAxis );
		const f32 shade = fClamp( 0.5f + fMax( 0.f, n.fDot( Math::tVec3f( 1.0f, 0.5f, 2.0f ).fNormalizeSafe( Math::tVec3f::cZAxis ) ) ), 0.f, 1.f );
		const u32 vtxColor = Gfx::tVertexColor( shade, shade, shade ).fForGpu( );

		const u32 baseVertex = verts.fCount( );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( v0, vtxColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( v1, vtxColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( v2, vtxColor ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( v3, vtxColor ) );

		ids.fPushBack( baseVertex + 0 );
		ids.fPushBack( baseVertex + 1 );
		ids.fPushBack( baseVertex + 2 );

		ids.fPushBack( baseVertex + 2 );
		ids.fPushBack( baseVertex + 3 );
		ids.fPushBack( baseVertex + 0 );
	}


}}

