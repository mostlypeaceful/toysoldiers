#include "BasePch.hpp"
#include "tHeightFieldQuadTree.hpp"
#include "tHeightFieldMesh.hpp"
//#include "tApplication.hpp"

#include "Math/tIntersectionRayTriangle.hpp"
#include "Math/tIntersectionTriangleFrustum.hpp"
#include "Math/tIntersectionAabbFrustum.hpp"

namespace Sig
{

	tHeightFieldQuadTree::tHeightFieldQuadTree( )
		: mMinMaxHeight( 0.f, 0.f )
		, mMinMaxLogicalX( 0, 0 )
		, mMinMaxLogicalZ( 0, 0 )
	{
		fZeroOut( mChildren );
	}

	tHeightFieldQuadTree::tHeightFieldQuadTree( tNoOpTag )
		: mChildren( cNoOpTag )
		, mMinMaxHeight( cNoOpTag )
		, mMinMaxLogicalX( cNoOpTag )
		, mMinMaxLogicalZ( cNoOpTag )
	{
	}

	tHeightFieldQuadTree::~tHeightFieldQuadTree( )
	{
		fDestroy( );
	}

	void tHeightFieldQuadTree::fConstruct( 
		const Math::tAabbf& myCellAabb,
		const tHeightFieldMesh& hfMesh, 
		u32 maxDepth, u32 splitThreshold )
	{
		// now call the real method (recursive)
		fConstruct( myCellAabb, hfMesh, 0, maxDepth, splitThreshold );
	}

	void tHeightFieldQuadTree::fConstruct( 
		const Math::tAabbf& myCellAabb,
		const tHeightFieldMesh& hfMesh, 
		u32 currentDepth, u32 maxDepth, u32 splitThreshold )
	{
		sigassert( currentDepth <= maxDepth );
		sigassert( hfMesh.fRez( ).fVertexDeltaX( ) <= myCellAabb.mMax.x - myCellAabb.mMin.x );
		sigassert( hfMesh.fRez( ).fVertexDeltaZ( ) <= myCellAabb.mMax.z - myCellAabb.mMin.z );

		// store min/max indices based on bounds
		mMinMaxLogicalX.x = hfMesh.fRez( ).fWorldXLogicalQuad( myCellAabb.mMin.x, fRoundDown<u32> );
		mMinMaxLogicalX.y = hfMesh.fRez( ).fWorldXLogicalQuad( myCellAabb.mMax.x, fRoundUp<u32> );
		mMinMaxLogicalZ.x = hfMesh.fRez( ).fWorldZLogicalQuad( myCellAabb.mMin.z, fRoundDown<u32> );
		mMinMaxLogicalZ.y = hfMesh.fRez( ).fWorldZLogicalQuad( myCellAabb.mMax.z, fRoundUp<u32> );

		const u32 numQuads = fQuadCount( currentDepth );

		// only split if we 1) have too many quads 2) aren't at the max. depth 3) triangles aren't too big for children
		const b32 tooManyQuads = numQuads > splitThreshold;
		const b32 atMaxDepth = currentDepth == maxDepth;
		const b32 trisTooBigForKids = 
			( hfMesh.fRez( ).fVertexDeltaX( ) > 0.5f * ( myCellAabb.mMax.x - myCellAabb.mMin.x ) || 
			  hfMesh.fRez( ).fVertexDeltaZ( ) > 0.5f * ( myCellAabb.mMax.z - myCellAabb.mMin.z ) );
		if( tooManyQuads && !atMaxDepth && !trisTooBigForKids )
		{
			// let children take the quads
			for( u32 i = 0; i < cCellCount; ++i )
			{
				//log_line( 0, "depth = " << currentDepth << ", cell = " << i );

				sigassert( !mChildren[ i ] );
				mChildren[ i ] = NEW tHeightFieldQuadTree( );
				mChildren[ i ]->fConstruct( fComputeChildAabb( myCellAabb, i ), hfMesh, currentDepth + 1, maxDepth, splitThreshold );
			}
		}
		else
		{
			//log_line( 0, "added [" << numQuads << "] quads to cell at depth [" << currentDepth << "]" );
		}
	}

	void tHeightFieldQuadTree::fCopy( const tHeightFieldQuadTree& other )
	{
		fDestroy( );
		
		mMinMaxHeight = other.mMinMaxHeight;
		mMinMaxLogicalX = other.mMinMaxLogicalX;
		mMinMaxLogicalZ = other.mMinMaxLogicalZ;

		for( u32 i = 0; i < cCellCount; ++i )
		{
			if( other.mChildren[ i ] )
			{
				mChildren[ i ] = NEW tHeightFieldQuadTree( );
				mChildren[ i ]->fCopy( *other.mChildren[ i ] );
			}
		}
	}

	void tHeightFieldQuadTree::fDestroy( )
	{
		for( u32 i = 0; i < cCellCount; ++i )
			delete mChildren[ i ];
		fZeroOut( mChildren );
		mMinMaxHeight = Math::tVec2f::cZeroVector;
		mMinMaxLogicalX = mMinMaxLogicalZ = Math::tVec2u::cZeroVector;
	}

	void tHeightFieldQuadTree::fCollectTris(
		tGrowableArray<Math::tTrianglef>& trisOut,
		const Math::tAabbf& aabb,
		const Math::tAabbf& myCellAabb,
		const tHeightFieldMesh& hfMesh ) const
	{
		if( !myCellAabb.fIntersects( aabb ) )
			return; // quick rejection, don't have to test children or tris

		for( u32 i = 0; i < cCellCount; ++i )
		{
			if( mChildren[ i ] )
				mChildren[ i ]->fCollectTris( trisOut, aabb, fComputeChildAabb( myCellAabb, i ), hfMesh );
		}

		if( !fIsLeaf( ) )
			return; // only leaf nodes contain quads

		tFixedArray< Math::tTrianglef, 2 > quadTris;

		for( u32 z = mMinMaxLogicalZ.x; z < mMinMaxLogicalZ.y; ++z )
		{
			for( u32 x = mMinMaxLogicalX.x; x < mMinMaxLogicalX.y; ++x )
			{
				if( !hfMesh.fComputeLogicalTris( x, z, quadTris[ 0 ], quadTris[ 1 ] ) )
					continue; // quad is removed

				for( u32 itri = 0; itri < quadTris.fCount( ); ++itri )
				{
					if( aabb.fIntersects( quadTris[ itri ].fGetAabb( ) ) )
						trisOut.fPushBack( quadTris[ itri ] );
				}
			}
		}
	}

	void tHeightFieldQuadTree::fTestRay(
		tHeightFieldRayCastHit& bestHit,
		const Math::tRayf& ray,
		const Math::tAabbf& myCellAabb,
		const tHeightFieldMesh& hfMesh ) const
	{
		if( !myCellAabb.fIntersectsOrContains( ray ) )
		{
			//if( fIsLeaf( ) )
			//{
			//	tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( ray, Math::tVec4f( 0.f, 1.f, 0.f, 0.5f ) );
			//	tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( myCellAabb, Math::tVec4f( 1.f, 0.f, 0.f, 0.5f ) );
			//	for( u32 z = mMinMaxLogicalZ.x; z < mMinMaxLogicalZ.y; ++z )
			//	{
			//		for( u32 x = mMinMaxLogicalX.x; x < mMinMaxLogicalX.y; ++x )
			//		{
			//			tFixedArray< Math::tTrianglef, 2 > quadTris;
			//			if( !hfMesh.fComputeLogicalTris( x, z, quadTris[ 0 ], quadTris[ 1 ] ) )
			//				continue; // quad is removed
			//			tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( quadTris[ 0 ], Math::tVec4f( 0.f, 1.f, 1.f, 0.5f ) );
			//			tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( quadTris[ 1 ], Math::tVec4f( 0.f, 1.f, 1.f, 0.5f ) );
			//		}
			//	}
			//}
			return; // quick rejection, don't have to test children or tris
		}

		for( u32 i = 0; i < cCellCount; ++i )
		{
			if( mChildren[ i ] )
				mChildren[ i ]->fTestRay( bestHit, ray, fComputeChildAabb( myCellAabb, i ), hfMesh );
		}

		if( !fIsLeaf( ) )
			return; // only leaf nodes contain quads

		Math::tIntersectionRayTriangle<f32> intersection;
		tFixedArray< Math::tTrianglef, 2 > quadTris;

		for( u32 z = mMinMaxLogicalZ.x; z < mMinMaxLogicalZ.y; ++z )
		{
			for( u32 x = mMinMaxLogicalX.x; x < mMinMaxLogicalX.y; ++x )
			{
				if( !hfMesh.fComputeLogicalTris( x, z, quadTris[ 0 ], quadTris[ 1 ] ) )
					continue; // quad is removed

				for( u32 itri = 0; itri < quadTris.fCount( ); ++itri )
				{
					intersection.fIntersect( ray, quadTris[ itri ] );
					if( intersection.fIntersects( ) && intersection.fT( ) < bestHit.mT )
					{
						bestHit.mT = intersection.fT( );
						bestHit.mN = quadTris[ itri ].fComputeUnitNormal( );
						bestHit.mX = x;
						bestHit.mZ = z;
					}
				}
			}
		}
	}

	b32 tHeightFieldQuadTree::fTestFrustum(
		const Math::tFrustumf& frustum,
		const Math::tAabbf& myCellAabb,
		const tHeightFieldMesh& hfMesh ) const
	{
		Math::tIntersectionAabbFrustum<f32> intersection( myCellAabb, frustum );
		if( !intersection.fIntersects( ) )
			return false; // quick rejection, don't have to test children or tris
		if( intersection.fContained( ) )
			return true; // quick acceptance, don't have to test children or tris

		for( u32 i = 0; i < cCellCount; ++i )
		{
			if( mChildren[ i ] )
			{
				if( mChildren[ i ]->fTestFrustum( frustum, fComputeChildAabb( myCellAabb, i ), hfMesh ) )
					return true;
			}
		}

		if( !fIsLeaf( ) )
			return false; // only leaf nodes contain quads

		Math::tAabbf quadAabb;

		for( u32 z = mMinMaxLogicalZ.x; z < mMinMaxLogicalZ.y; ++z )
		{
			for( u32 x = mMinMaxLogicalX.x; x < mMinMaxLogicalX.y; ++x )
			{
				hfMesh.fComputeLogicalQuadAabb( x, z, quadAabb );
				if( Math::tIntersectionAabbFrustum<f32>( quadAabb, frustum ).fIntersects( ) )
				{
					// TODO real tri/frustum, though for smallish tris this should be fine,
					// as frustum tests are generally less precise then ray tests
					return true;
				}
			}
		}

		return false;
	}

	void tHeightFieldQuadTree::fUpdateBounds( u32 logicalX, u32 logicalZ, const tHeightFieldMesh& hfMesh )
	{
		if( !fInBounds( logicalX, mMinMaxLogicalX.x, mMinMaxLogicalX.y ) ||
			!fInBounds( logicalZ, mMinMaxLogicalZ.x, mMinMaxLogicalZ.y ) )
			return; // i don't contain this vertex, nor do any of my children

		mMinMaxHeight.x = +Math::cInfinity;
		mMinMaxHeight.y = -Math::cInfinity;

		if( fIsLeaf( ) )
		{
			for( u32 z = mMinMaxLogicalZ.x; z <= mMinMaxLogicalZ.y; ++z )
			{
				for( u32 x = mMinMaxLogicalX.x; x <= mMinMaxLogicalX.y; ++x )
				{
					const f32 h = hfMesh.fLogicalHeight( x, z );
					mMinMaxHeight.x = fMin( h, mMinMaxHeight.x );
					mMinMaxHeight.y = fMax( h, mMinMaxHeight.y );
				}
			}
		}
		else
		{
			for( u32 i = 0; i < cCellCount; ++i )
			{
				mChildren[ i ]->fUpdateBounds( logicalX, logicalZ, hfMesh );

				const Math::tVec2f h = mChildren[ i ]->mMinMaxHeight;
				mMinMaxHeight.x = fMin( h.x, mMinMaxHeight.x );
				mMinMaxHeight.y = fMax( h.y, mMinMaxHeight.y );
			}
		}
	}

	Math::tAabbf tHeightFieldQuadTree::fComputeChildAabb( const Math::tAabbf& myCellAabb, u32 cell ) const
	{
		const Math::tVec3f c = myCellAabb.fComputeCenter( ); // center
		const Math::tVec3f d = 0.5f * myCellAabb.fComputeDiagonal( ); // diagonal
		const Math::tVec2f heightMinMax = mChildren[ cell ] ? mChildren[ cell ]->mMinMaxHeight : Math::tVec2f::cZeroVector;

		switch( cell )
		{
		case cCell_pXpZ:
			return Math::tAabbf( Math::tVec3f( c.x, heightMinMax.x, c.z ), Math::tVec3f( c.x + d.x, heightMinMax.y, c.z + d.z ) );
		case cCell_nXpZ:
			return Math::tAabbf( Math::tVec3f( c.x - d.x, heightMinMax.x, c.z ), Math::tVec3f( c.x, heightMinMax.y, c.z + d.z ) );
		case cCell_pXnZ:
			return Math::tAabbf( Math::tVec3f( c.x, heightMinMax.x, c.z - d.z ), Math::tVec3f( c.x + d.x, heightMinMax.y, c.z ) );
		case cCell_nXnZ:
			return Math::tAabbf( Math::tVec3f( c.x - d.x, heightMinMax.x, c.z - d.z ), Math::tVec3f( c.x, heightMinMax.y, c.z ) );

		default: sigassert( !"invalid cell index" ); break;
		}

		return myCellAabb;
	}

	b32 tHeightFieldQuadTree::fComputeMinMaxHeightEstimateRecursive( 
		Math::tVec2f& out,
		const Math::tVec2u& minMaxLogicalX,
		const Math::tVec2u& minMaxLogicalZ ) const
	{
		if( !fInBounds( minMaxLogicalX.x, mMinMaxLogicalX.x, mMinMaxLogicalX.y ) ||
			!fInBounds( minMaxLogicalX.y, mMinMaxLogicalX.x, mMinMaxLogicalX.y ) ||
			!fInBounds( minMaxLogicalZ.x, mMinMaxLogicalZ.x, mMinMaxLogicalZ.y ) ||
			!fInBounds( minMaxLogicalZ.y, mMinMaxLogicalZ.x, mMinMaxLogicalZ.y ) )
			return false; // i don't contain this rectangle, nor do any of my children

		for( u32 i = 0; i < cCellCount; ++i )
		{
			if( mChildren[ i ] && 
				mChildren[ i ]->fComputeMinMaxHeightEstimateRecursive( out, minMaxLogicalX, minMaxLogicalZ ) )
			{
				return true;
			}
		}

		out = mMinMaxHeight;
		return true;
	}

	Math::tVec2f tHeightFieldQuadTree::fComputeMinMaxHeightEstimate( 
		const Math::tVec2u& minMaxLogicalX,
		const Math::tVec2u& minMaxLogicalZ ) const
	{
		Math::tVec2f o = mMinMaxHeight;
		fComputeMinMaxHeightEstimateRecursive( o, minMaxLogicalX, minMaxLogicalZ );
		return o;
	}

	b32 tHeightFieldQuadTree::fIsLeaf( ) const
	{
		for( u32 i = 0; i < cCellCount; ++i )
			if( mChildren[ i ] )
				return false;
		return true;
	}

	u32 tHeightFieldQuadTree::fQuadCount( u32 depth ) const
	{
		const u32 quadCount = ( mMinMaxLogicalX.y - mMinMaxLogicalX.x ) * ( mMinMaxLogicalZ.y - mMinMaxLogicalZ.x );
		return quadCount;
	}

}

