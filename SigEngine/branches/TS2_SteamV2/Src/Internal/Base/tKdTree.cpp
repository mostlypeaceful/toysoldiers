#include "BasePch.hpp"
#include "tKdTree.hpp"

#include "Gfx/tDebugGeometry.hpp"

// intersections
#include "Math/tIntersectionRayTriangle.hpp"
#include "Math/tIntersectionAabbFrustum.hpp"

using namespace Sig::Math;

namespace Sig
{

	static const b32 cDontUseParentsSplitDirection = false;

	class base_export tSortedKdData
	{
	public:
		struct tItem
		{
			u32 mID;
			Math::tAabbf mBounds;

			tItem( u32 id = 0, const Math::tAabbf& bounds = Math::tAabbf() ) : mID( id ), mBounds( bounds ) { }
		};

		struct tSplit
		{
			f32 mPosition;
			u32 mItemIndex;

			tSplit( f32 pos = 0.0f, u32 itemIndex = 0 ) : mPosition( pos ), mItemIndex( itemIndex ) { }

			b32 operator < ( const tSplit& right ) const { return mPosition < right.mPosition; }
		};

		struct tSplitSet
		{
			tGrowableArray< tSplit > mMins, mMaxs;

			void fSort( );
			void fSetCapacity( u32 count );

			u32 fCount( ) const;
			const tSplit& operator [ ] ( u32 index ) const;
		};

		Math::tAabbf			mBounds;
		tGrowableArray< tItem >	mItems;
		tSplitSet				mPotentialSplits[ 3 ]; //one for each axis

		tSortedKdData( );

		void fSortItems( const tDynamicArray< Math::tAabbf >& aabbs );

		void fAddItem( const tItem& item );

		void fCountItems( u32 axis, f32 pos, u32& leftOut, u32& rightOut ) const;
		void fFindItems( u32 axis, f32 pos, tSortedKdData& leftOut, tSortedKdData& rightOut ) const;

		void fSetCapacity( u32 items );
		void fSortSplits( );

		const Math::tAabbf& fBounds( ) const { return mBounds; }

		u32 fCount( ) const { return mItems.fCount( ); }
	};

	void tSortedKdData::tSplitSet::fSort( )
	{
		std::sort( mMins.fBegin( ), mMins.fEnd( ) );
		std::sort( mMaxs.fBegin( ), mMaxs.fEnd( ) );
	}

	void tSortedKdData::tSplitSet::fSetCapacity( u32 count )
	{
		mMins.fSetCapacity( count );
		mMaxs.fSetCapacity( count );
	}

	u32 tSortedKdData::tSplitSet::fCount( ) const
	{ 
		return mMins.fCount( ) + mMaxs.fCount( ); 
	}

	const tSortedKdData::tSplit& tSortedKdData::tSplitSet::operator [ ] ( u32 index ) const
	{ 
		if( index >= mMins.fCount( ) )
			return mMaxs[ index - mMins.fCount( ) ];
		else
			return mMins[ index ];
	}

	/////////////////////////////////////////////////////////////////////////////////////

	tSortedKdData::tSortedKdData( )
	{
		mBounds.fInvalidate( );
	}

	void tSortedKdData::fSortItems( const tDynamicArray< Math::tAabbf >& aabbs )
	{
		mBounds.fInvalidate( );

		u32 itemCount = aabbs.fCount( );
		fSetCapacity( itemCount );

		for( u32 t = 0; t < itemCount; ++t )
			fAddItem( tItem( t, aabbs[ t ] ) );	

		fSortSplits( );
	}

	void tSortedKdData::fAddItem( const tItem& item )
	{
		u32 id = mItems.fCount( );
		mItems.fPushBack( item );
		mBounds |= item.mBounds;

		//static const tVec3f refVecs[ 3 ] = { tVec3f::cXAxis, tVec3f::cYAxis, tVec3f::cZAxis };
		const f32 fudge = 0.001f;

		for( u32 i = 0; i < 3; ++i )
		{
			mPotentialSplits[ i ].mMins.fPushBack( tSplit( item.mBounds.mMin.fAxis( i ) - fudge, id ) );
			mPotentialSplits[ i ].mMaxs.fPushBack( tSplit( item.mBounds.mMax.fAxis( i ) + fudge, id ) );
		}
	}

	void tSortedKdData::fCountItems( u32 axis, f32 pos, u32& leftOut, u32& rightOut ) const
	{
		const tGrowableArray< tItem >& list = mItems;

		for( u32 t = 0; t < list.fCount( ); ++t )
		{
			const tItem &item = list[ t ];

			f32 min = item.mBounds.mMin.fAxis( axis ) - pos;
			f32 max = item.mBounds.mMax.fAxis( axis ) - pos;

			if( min	<= 0.0f ) ++rightOut;
			if( max >= 0.0f ) ++leftOut;
		}
	}

	void tSortedKdData::fFindItems( u32 axis, f32 pos, tSortedKdData& leftOut, tSortedKdData& rightOut ) const
	{
		const tGrowableArray< tItem >& list = mItems;

		leftOut.fSetCapacity( fCount( ) );
		rightOut.fSetCapacity( fCount( ) );
		for( u32 t = 0; t < list.fCount( ); ++t )
		{
			const tItem &item = list[ t ];

			f32 min = item.mBounds.mMin.fAxis( axis ) - pos;
			f32 max = item.mBounds.mMax.fAxis( axis ) - pos;

			if( min	<= 0.0f ) rightOut.fAddItem( item );
			if( max >= 0.0f ) leftOut.fAddItem( item );
		}

		leftOut.fSortSplits( );
		rightOut.fSortSplits( );


		//const tGrowableArray< tItem >& list = mItems;
		//const tSplitSet& set = mPotentialSplits[ axis ];

		////mMins stores the aabb's minimum edge
		//// test these against the + (left) side of the split
		//leftOut.fSetCapacity( fCount( ) );
		//for( u32 s = 0; s < set.mMins.fCount( ); ++s )
		//{
		//	const tSortedKdData::tSplit& split = set.mMins[ s ];

		//	if( split.mPosition >= pos ) 
		//		leftOut.fAddItem( mItems[ split.mItemIndex ] );
		//}

		////mMaxs stores the aabb's maximum edge
		//// test these against the - (right) side of the split
		//rightOut.fSetCapacity( fCount( ) );
		//for( u32 s = 0; s < set.mMaxs.fCount( ); ++s )
		//{
		//	const tSortedKdData::tSplit& split = set.mMaxs[ s ];

		//	if( split.mPosition <= pos )
		//		rightOut.fAddItem( mItems[ split.mItemIndex ] ); 
		//}
	}

	void tSortedKdData::fSetCapacity( u32 items )
	{
		mItems.fSetCapacity( items );

		for( u32 i = 0; i < 3; ++i )
			mPotentialSplits[ i ].fSetCapacity( items );
	}

	void tSortedKdData::fSortSplits( )
	{
		for( u32 i = 0; i < 3; ++i )
			mPotentialSplits[ i ].fSort( );
	}

	////////////////////////////////////////////////////////////////////////

	namespace
	{
		//static inline tPlanef fSplitPlane( tKdNode::tAxis axis, f32 pos )
		//{
		//	tVec3f normal( 0 );
		//	normal.fAxis( axis ) = 1;

		//	return tPlanef( normal, -pos );
		//}

		static inline tAabbf fSubVoxel( tKdNode::tAxis axis, f32 pos, b32 rightSide, const tAabbf& parentVoxel )
		{
			tAabbf subVox = parentVoxel;

			if( rightSide )
			{
				// negative side, clip max
				f32 diff = pos - parentVoxel.mMax.fAxis( axis );
				subVox.mMax.fAxis( axis ) += diff;
			}
			else
			{
				// positive side, clip min
				f32 diff = pos - parentVoxel.mMin.fAxis( axis );
				subVox.mMin.fAxis( axis ) += diff;
			}			

			return subVox;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////

	tKdNode::tKdNode( )
	{
		for( u32 i = 0; i < 2; ++i )
			mChildren[ i ] = NULL;
	}

	tKdNode::~tKdNode( )
	{
		fDestroy( );
	}

	f32 tKdNode::fComputeSplitCost( const tSortedKdData& data, tAxis axis, f32 pos, const tAabbf& parentVoxel )
	{
		u32 leftCost = 0;
		u32 rightCost = 0;

		data.fCountItems( axis, pos, leftCost, rightCost );

		f32 surfaceAreaParent = parentVoxel.fSurfaceArea( );
		f32 leftSA = fSubVoxel( axis, pos, false, parentVoxel ).fSurfaceArea( );
		f32 rightSA = fSubVoxel( axis, pos, true, parentVoxel ).fSurfaceArea( );
		f32 leftProbability = leftSA / surfaceAreaParent;
		f32 rightProbability = rightSA / surfaceAreaParent;

		const f32 splitCost = 10.0f;

		f32 totalCost = splitCost + leftCost * leftProbability + rightCost * rightProbability;

		return totalCost;
	}

	f32 tKdNode::fComputeLeafCost( const tSortedKdData& data )
	{
		return (f32)data.fCount( );
	}

	void tKdNode::fBuild( const tSortedKdData& potentialItems, const tAabbf& parentVoxel, tAxis dontUseAxis )
	{
		// cost if this were a leave node
		f32 leafCost = fComputeLeafCost( potentialItems );
		f32 minCost = cInfinity;

		//don't try the ends
		u32 splitCnt = potentialItems.mPotentialSplits[ 0 ].fCount( );
		s32 count = s32(splitCnt) - 1;

		const s32 minSampleCnt = 100;
		s32 rate = fMax( 1, count / minSampleCnt );

		for( u32 axis = cXAxis; axis < cAxisCount; ++axis )
		{
			if( axis == dontUseAxis ) continue;

			const tSortedKdData::tSplitSet& splitList = potentialItems.mPotentialSplits[ axis ];

			for( s32 s = 1; s < count; s += rate )
			{
				const tSortedKdData::tSplit& split = splitList[ s ];

				f32 cost = fComputeSplitCost( potentialItems, tAxis( axis ), split.mPosition, parentVoxel );
				if( cost < minCost )
				{
					//use this split, but keep looking
					minCost = cost;
					mSplitPosition = split.mPosition;
					mAxis = tAxis( axis );
				}
			}
		}

		if( minCost < leafCost )
		{
			// find new subsets of items
			tSortedKdData left, right;
			potentialItems.fFindItems( mAxis, mSplitPosition, left, right );

			dontUseAxis = ( cDontUseParentsSplitDirection ) ? ( tAxis )mAxis : cAxisInvalid;

			// recursive build
			if( left.fCount( ) )
			{
				mChildren[ 0 ] = NEW tKdNode;
				mChildren[ 0 ]->fBuild( left, fSubVoxel( mAxis, mSplitPosition, false, parentVoxel ), dontUseAxis );
			}

			if( right.fCount( ) )
			{
				mChildren[ 1 ] = NEW tKdNode;
				mChildren[ 1 ]->fBuild( right, fSubVoxel( mAxis, mSplitPosition, true, parentVoxel ), dontUseAxis );
			}
		}
		else
		{
			//form leaf

			// call fNewArray on sleeve so that it doesnt call fDelete
			//  this allows us to hijack loaded in place data for debugging.
			mItems.fDisown( );
			mItems.fNewArray( potentialItems.mItems.fCount( ) );
			mItemsBounds.fDisown( );
			mItemsBounds.fNewArray( potentialItems.mItems.fCount( ) );

			for( u32 t = 0; t < potentialItems.mItems.fCount( ); ++t )
			{
				mItems[ t ] = potentialItems.mItems[ t ].mID;
				mItemsBounds[ t ] = potentialItems.mItems[ t ].mBounds;
			}
		}
	}

	void tKdNode::fDestroy( b32 wasLoadedInPlace )
	{
		for( u32 c = 0; c < 2; ++c )
		{
			if( !wasLoadedInPlace ) //else just abandon it, but only for debugging to utilize that behavior
				delete mChildren[ c ];
			mChildren[ c ] = NULL;
		}

		if( !wasLoadedInPlace ) //else just abandon it, but only for debugging to utilize that behavior
			mItems.fDeleteArray( );
	}

	void tKdNode::fDraw( const Math::tMat3f& xform, Gfx::tDebugGeometryContainer& dgc, const tAabbf& bounds, const tVec4f& color, s32 level ) const
	{
		if( level == -1 )
			dgc.fRenderOnce( tObbf( bounds, xform ), color );
		else if( !fIsLeaf( ) )
		{
			--level;
			if( mChildren[ 0 ] ) mChildren[ 0 ]->fDraw( xform, dgc, fSubVoxel( mAxis, mSplitPosition, false, bounds ), tVec4f( 1, 0, 0, 0.25f ), level );
			if( mChildren[ 1 ] ) mChildren[ 1 ]->fDraw( xform, dgc, fSubVoxel( mAxis, mSplitPosition, true, bounds ), tVec4f( 0, 1, 0, 0.25f ), level );
		}	
	}

	u32 tKdNode::fMemorySizeRecursive( ) const
	{
		u32 memSize = sizeof( tKdNode );
		memSize += sizeof( u32 ) * mItems.fCount( );

		for( u32 i = 0; i < 2; ++i )
			if( mChildren[ i ] )
				memSize += mChildren[ i ]->fMemorySizeRecursive( );

		return memSize;
	}

	void tKdNode::fCollectItems(
		tGrowableArray<u32>& itemsOut,
		const Math::tAabbf& aabb,
		const Math::tAabbf& cellBounds ) const
	{
		if( !cellBounds.fIntersects( aabb ) )
			return; // quick rejection, don't have to test children

		if( mChildren[ 0 ] )
		{
			tAabbf subVox = fSubVoxel( mAxis, mSplitPosition, false, cellBounds );
			mChildren[ 0 ]->fCollectItems( itemsOut, aabb, subVox );
		}

		if( mChildren[ 1 ] )
		{
			tAabbf subVox = fSubVoxel( mAxis, mSplitPosition, true, cellBounds );
			mChildren[ 1 ]->fCollectItems( itemsOut, aabb, subVox );
		}

		for( u32 i = 0; i < mItems.fCount( ); ++i )
		{
			if( aabb.fIntersects( mItemsBounds[ i ] ) )
				itemsOut.fPushBack( mItems[ i ] );
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////

	tKdTree::tKdTree( )
	{
		int breakpoint = 0;
	}

	void tKdTree::fConstruct( const tDynamicArray< Math::tAabbf >& aabbs, b32 wasLoadedInPlace )
	{
		mFirstSplit.fDestroy( wasLoadedInPlace );

		tSortedKdData data;
		data.fSortItems( aabbs );

		mBounds = data.fBounds( );

		//since most of our rays come from the top, don't use the top plane as the first split
		// that way our first split test will usually cull out half the results.
		mFirstSplit.fBuild( data, mBounds, tKdNode::cYAxis );
	}

	u32 tKdTree::fMemorySizeRecursive( ) const
	{
		u32 memSize = sizeof( tKdTree );

		memSize += mFirstSplit.fMemorySizeRecursive( );

		return memSize;
	}

	void tKdTree::fDraw( const Math::tMat3f& xform, Gfx::tDebugGeometryContainer& dgc, s32 level ) const
	{
		mFirstSplit.fDraw( xform, dgc, mBounds, tVec4f(1,0,0,0.25f), level );
	}

	void tKdTree::fCollectItems(
		tGrowableArray<u32>& itemsOut,
		const Math::tAabbf& aabb ) const
	{
		mFirstSplit.fCollectItems( itemsOut, aabb, mBounds );
	}

	//Fast kd-tree Construction with an Adaptive Error-Bounded Heuristic
	// summary of: file:///C:/Documents%20and%20Settings/Matt/My%20Documents/Downloads/10.1.1.113.550.pdf
	// *sorting step*
	// Create a "main" axis which is not parrallel to any of the split planes use: normalize(1,1,1)
	// Project each triangle onto this axis to get its min and max t
	// sort both min and max independently

	// *recursive scanning step*
	// Find minimum of cost function
	//  if cost of splitting here is less than not splitting
	//   for each split, repeat

	// *cost function*
	// voxel v 
	// for each ( split position x in (min, max) )
	//	cost( x ) = cL( x ) + (saL( v, x )/sa(v)) + cR( x ) * (saR( v, x )/sa(v))
	// 
	//  saL / sa is the probability of a ray intersecting the new voxel.
	// cL is cost of having a split (const, or memory)

	// *minimum finding*
	// sample domain at interval to get linear approximation of each component to the cost function
	//  interpolate components to get quadratic approximation (Saweet)

	// *fancy shit*
	// take additional samples to find segments with error
	//  resample segments with high error
}
