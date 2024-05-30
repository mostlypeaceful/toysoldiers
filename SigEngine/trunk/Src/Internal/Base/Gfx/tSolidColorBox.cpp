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



	tSolidColorCylinder::tSolidColorCylinder( )
		: mHalfHeight( 1.f )
		, mRadius( 1.f )
	{
		fSetPrimTypeOverride( tIndexFormat::cPrimitiveTriangleList );
	}

	void tSolidColorCylinder::fOnDeviceReset( tDevice* device )
	{
		fGenerate( mHalfHeight, mRadius );
	}

	void tSolidColorCylinder::fGenerate( f32 halfHeight, f32 radius )
	{
		mHalfHeight = halfHeight;
		mRadius = radius;

		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		tGrowableArray< u16 > inds;
		u32 color = 0xffffffff;

		const u32 cSegments = 25;
		f32 step = Math::c2Pi / (f32)cSegments;

		verts.fSetCount( cSegments * 2 + 2 );

		for( u32 i = 0; i < cSegments; ++i )
		{
			Math::tVec3f dir;
			dir.fSetXZHeading( i * step );

			verts[ i ] = Gfx::tSolidColorRenderVertex( dir * mRadius + Math::tVec3f( 0, mHalfHeight, 0 ), color );
			verts[ i + cSegments ] = Gfx::tSolidColorRenderVertex( dir * mRadius + Math::tVec3f( 0, -mHalfHeight, 0 ), color );
		}

		u32 cap1 = cSegments * 2;
		u32 cap2 = cSegments * 2 + 1;
		verts[ cap1 ] = Gfx::tSolidColorRenderVertex( Math::tVec3f( 0, mHalfHeight, 0 ), color );
		verts[ cap2 ] = Gfx::tSolidColorRenderVertex( Math::tVec3f( 0, -mHalfHeight, 0 ), color );

		inds.fSetCount( cSegments * 3 * 4 );

		u32 cap1Start = 0;
		u32 cap2Start = cSegments * 3;
		u32 sideStart = cap2Start + cSegments * 3;

		for( u32 i = 0; i < cSegments; ++i )
		{
			u32 seg1 = i;
			u32 seg2 = i + 1;
			if( i == cSegments - 1 )
				seg2 = 0;

			inds[ cap1Start + i * 3 + 0 ] = cap1;
			inds[ cap1Start + i * 3 + 1 ] = seg1;
			inds[ cap1Start + i * 3 + 2 ] = seg2;

			inds[ cap2Start + i * 3 + 0 ] = cap2;
			inds[ cap2Start + i * 3 + 1 ] = seg2 + cSegments;
			inds[ cap2Start + i * 3 + 2 ] = seg1 + cSegments;

			inds[ sideStart + i * 6 + 0 ] = seg1;
			inds[ sideStart + i * 6 + 1 ] = seg1 + cSegments;
			inds[ sideStart + i * 6 + 2 ] = seg2 + cSegments;

			inds[ sideStart + i * 6 + 3 ] = seg2 + cSegments;
			inds[ sideStart + i * 6 + 4 ] = seg2;
			inds[ sideStart + i * 6 + 5 ] = seg1;
		}

		fBake( verts, inds, inds.fCount( ) / 3 );
	}

	b32 tSolidColorCylinder::fIntersectsRay( const Math::tRayf& ray, f32& tOut ) const
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

	b32 tSolidColorCylinder::fIntersectsFrustum( const Math::tFrustumf& frustum ) const
	{
		const Math::tAabbf aabb = fGetBounds( );

		Math::tIntersectionAabbFrustum<f32> intersection( frustum, aabb );
		if( intersection.fIntersects( ) )
		{
			return true;
		}

		return false;
	}


}}

