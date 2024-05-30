#include "BasePch.hpp"
#include "tPolySoupOctree.hpp"

// intersections
#include "Math/tIntersectionRayTriangle.hpp"
#include "Math/tIntersectionAabbFrustum.hpp"

namespace Sig
{

	///
	/// \section tPolySoupOctree
	///

	tPolySoupOctree::tPolySoupOctree( )
	{
		fZeroOut( mChildren );
	}

	tPolySoupOctree::tPolySoupOctree( tNoOpTag )
		: mTriIndices( cNoOpTag )
		, mChildren( cNoOpTag )
	{
	}

	tPolySoupOctree::~tPolySoupOctree( )
	{
		fDestroy( );
	}

	void tPolySoupOctree::fConstruct( 
		const Math::tAabbf& myCellAabb,
		const tPolySoupVertexList& verts, 
		const tPolySoupTriangleList& tris, 
		u32 maxDepth, u32 splitThreshold, f32 minEdgeLength )
	{
		// create the initial array of potential indices (include all of them)
		tGrowableArray< u32 > potentialIndices;
		potentialIndices.fSetCount( tris.fCount( ) );
		for( u32 i = 0; i < tris.fCount( ); ++i )
			potentialIndices[ i ] = i;

		// now call the real method (recursive)
		fConstruct( potentialIndices, myCellAabb, verts, tris, 0, maxDepth, splitThreshold, minEdgeLength );
	}

	void tPolySoupOctree::fConstruct( 
		tGrowableArray< u32 >& potentialIndices,
		const Math::tAabbf& myCellAabb,
		const tPolySoupVertexList& verts,
		const tPolySoupTriangleList& tris,
		u32 currentDepth, u32 maxDepth, u32 splitThreshold, f32 minEdgeLength )
	{
		if( potentialIndices.fCount( ) == 0 )
			return; // nothing to insert

		if( currentDepth > maxDepth )
			return; // we've gone too far

		if( myCellAabb.fComputeDiagonal( ).fMaxMagnitude( ) < minEdgeLength )
			return; // we've gotten too small

		// before recursing on children, filter out the triangles
		// that actually are contained within me; we'll supply these
		// to my children, so that we're not all redundantly checking ALL the tris
		tGrowableArray< u32 > myIndices;
		myIndices.fSetCapacity( 256 );

		// try and take as many tris as will fit inside my walls
		for( s32 i = 0; i < ( s32 )potentialIndices.fCount( ); ++i )
		{
			const Math::tVec3u& vtxIds = tris[ potentialIndices[ i ] ];
			const Math::tTrianglef tri( verts[ vtxIds.x ], verts[ vtxIds.y ], verts[ vtxIds.z ] );

			if( myCellAabb.fContains( tri.fGetAabb( ) ) )
			{
				// we contain this triangle completely, add the tri index
				myIndices.fPushBack( potentialIndices[ i ] );
				// remove it from potential list
				potentialIndices.fErase( i );
				--i;
			}
		}

		if( myIndices.fCount( ) > 0 ) // don't try to add tris to children if none of them are contained in me
		{
			// only split if there are enough tris
			if( myIndices.fCount( ) > splitThreshold )
			{
				// attempt to hand off all the triangles to children
				for( u32 i = 0; i < cCellCount; ++i )
				{
					//log_line( 0, "depth = " << currentDepth << ", cell = " << i );

					sigassert( !mChildren[ i ] );
					mChildren[ i ] = NEW tPolySoupOctree( );
					mChildren[ i ]->fConstruct( myIndices, fComputeChildAabb( myCellAabb, i ), verts, tris, currentDepth + 1, maxDepth, splitThreshold, minEdgeLength );
					if( mChildren[ i ]->fTriangleCountRecursive( ) == 0 )
					{
						// child didn't take any, useless offspring
						delete mChildren[ i ];
						mChildren[ i ] = 0;
					}
				}
			}

			// take whatever triangles my children didn't want
			if( myIndices.fCount( ) > 0 )
			{
				//log_line( 0, "added [" << myIndices.fCount( ) << "] tris to cell at depth [" << currentDepth << "]" );

				mTriIndices.fNewArray( myIndices.fCount( ) );
				fMemCpy( mTriIndices.fBegin( ), myIndices.fBegin( ), sizeof( mTriIndices[0] ) * mTriIndices.fCount( ) );
			}
		}

		if( currentDepth == 0 && potentialIndices.fCount( ) > 0 )
		{
			// we have no choice, the buck stops here; eat the rest of the indices
			mTriIndices.fNewArray( potentialIndices.fCount( ) );
			fMemCpy( mTriIndices.fBegin( ), potentialIndices.fBegin( ), sizeof( mTriIndices[0] ) * mTriIndices.fCount( ) );

			// clear the list of potentials, we took them all
			potentialIndices.fSetCount( 0 );
		}
	}

	void tPolySoupOctree::fDestroy( )
	{
		for( u32 i = 0; i < cCellCount; ++i )
			delete mChildren[ i ];
		fZeroOut( mChildren );
		mTriIndices.fDeleteArray( );
	}

	void tPolySoupOctree::fCollectTris(
		tGrowableArray<Math::tTrianglef>& trisOut,
		const Math::tAabbf& aabb,
		const Math::tAabbf& myCellAabb,
		const tPolySoupVertexList& verts, 
		const tPolySoupTriangleList& tris ) const
	{
		if( !myCellAabb.fIntersects( aabb ) )
			return; // quick rejection, don't have to test children or tris

		for( u32 i = 0; i < cCellCount; ++i )
		{
			if( mChildren[ i ] )
				mChildren[ i ]->fCollectTris( trisOut, aabb, fComputeChildAabb( myCellAabb, i ), verts, tris );
		}

		for( u32 itri = 0; itri < mTriIndices.fCount( ); ++itri )
		{
			const u32 triIndex = mTriIndices[ itri ];
			const Math::tVec3u& vtxIds = tris[ triIndex ];
			const Math::tTrianglef tri = Math::tTrianglef( verts[ vtxIds.x ], verts[ vtxIds.y ], verts[ vtxIds.z ] );
			if( aabb.fIntersects( tri.fGetAabb( ) ) )
				trisOut.fPushBack( tri );
		}
	}

	void tPolySoupOctree::fTestRay(
		tPolySoupRayCastHit& bestHit,
		const Math::tRayf& ray,
		const Math::tAabbf& myCellAabb,
		const tPolySoupVertexList& verts, 
		const tPolySoupTriangleList& tris ) const
	{
		if( !myCellAabb.fIntersectsOrContains( ray ) )
			return; // quick rejection, don't have to test children or tris

		for( u32 i = 0; i < cCellCount; ++i )
		{
			if( mChildren[ i ] )
				mChildren[ i ]->fTestRay( bestHit, ray, fComputeChildAabb( myCellAabb, i ), verts, tris );
		}

		Math::tIntersectionRayTriangle<f32> intersection;
		for( u32 itri = 0; itri < mTriIndices.fCount( ); ++itri )
		{
			const u32 triIndex = mTriIndices[ itri ];
			const Math::tVec3u& vtxIds = tris[ triIndex ];
			const Math::tTrianglef tri( verts[ vtxIds.x ], verts[ vtxIds.y ], verts[ vtxIds.z ] );
			intersection.fIntersect( ray, tri );
			if( intersection.fIntersects( ) && intersection.fT( ) < bestHit.mT )
			{
				bestHit.mT			= intersection.fT( );
				bestHit.mN			= intersection.fNormal( );
				bestHit.mTriIndex	= triIndex;
			}
		}

		if( bestHit.fHit( ) )
		{
			bestHit.mN.fNormalizeSafe( Math::tVec3f::cZeroVector );

			sync_event_v_c( bestHit.mT, tSync::cSCRaycast );
			sync_event_v_c( bestHit.mN, tSync::cSCRaycast );
			sync_event_v_c( bestHit.mTriIndex, tSync::cSCRaycast );
		}
	}

	b32 tPolySoupOctree::fTestFrustum(
		const Math::tFrustumf& frustum,
		const Math::tAabbf& myCellAabb,
		const tPolySoupVertexList& verts, 
		const tPolySoupTriangleList& tris ) const
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
				if( mChildren[ i ]->fTestFrustum( frustum, fComputeChildAabb( myCellAabb, i ), verts, tris ) )
					return true;
			}
		}

		for( u32 itri = 0; itri < mTriIndices.fCount( ); ++itri )
		{
			const Math::tVec3u& vtxIds = tris[ mTriIndices[ itri ] ];
			const Math::tAabbf triAabb = Math::tTrianglef( verts[ vtxIds.x ], verts[ vtxIds.y ], verts[ vtxIds.z ] ).fGetAabb( );
			if( Math::tIntersectionAabbFrustum<f32>( triAabb, frustum ).fIntersects( ) )
			{
				// TODO real tri/frustum, though for smallish tris this should be fine,
				// as frustum tests are generally less precise then ray tests
				return true;
			}
		}

		return false;
	}

	u32 tPolySoupOctree::fTriangleCountRecursive( ) const
	{
		u32 count = 0;

		for( u32 i = 0; i < cCellCount; ++i )
			if( mChildren[ i ] )
				count += mChildren[ i ]->fTriangleCountRecursive( );

		count += mTriIndices.fCount( );
		return count;
	}
	
	u32 tPolySoupOctree::fMemorySizeRecursive( ) const
	{
		u32 memSize = sizeof( tPolySoupOctree );
		
		memSize += sizeof( u32 ) * mTriIndices.fCount( );

		for( u32 i = 0; i < cCellCount; ++i )
			if( mChildren[ i ] )
				memSize += mChildren[ i ]->fMemorySizeRecursive( );
		
		return memSize;
	}

	b32 tPolySoupOctree::fEqual( const tPolySoupOctree& other ) const
	{
		if( mTriIndices.fCount( ) != other.mTriIndices.fCount( ) )
			return false;
		if( fMemCmp( mTriIndices.fBegin( ), other.mTriIndices.fBegin( ), sizeof( *mTriIndices.fBegin( ) ) * mTriIndices.fCount( ) ) != 0 )
			return false;

		for( u32 i = 0; i < cCellCount; ++i )
		{
			if( !mChildren[ i ] && !other.mChildren[ i ] )
				continue;
			if( !mChildren[ i ] || !other.mChildren[ i ] )
				return false;
			sigassert( mChildren[ i ] && other.mChildren[ i ] );
			if( !mChildren[ i ]->fEqual( *other.mChildren[ i ] ) )
				return false;
		}

		return true;
	}

}
