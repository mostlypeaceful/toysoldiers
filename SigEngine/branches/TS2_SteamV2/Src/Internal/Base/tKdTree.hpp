#ifndef __tKdTree__
#define __tKdTree__
#include "tLoadInPlaceObjects.hpp"
#include "tPolySoupRaycastHit.hpp"

namespace Sig
{
	class tSortedKdData;

	///
	/// \class tKdNode
	/// \brief Basic KD-Tree node able to collect items by AABBs.
	class base_export tKdNode
	{
		declare_reflector( );
	public:
		//user accessable stuff
		enum tAxis { cXAxis, cYAxis, cZAxis, cAxisCount, cAxisInvalid };

		inline b32	fIsLeaf( ) const { return !mChildren[ 0 ] && !mChildren[ 1 ]; }
		void		fDraw( const Math::tMat3f& xform, Gfx::tDebugGeometryContainer& dgc, const Math::tAabbf& bounds, const Math::tVec4f& color, s32 level ) const;

		void		fCollectItems(
			tGrowableArray<u32>& itemsOut,
			const Math::tAabbf& aabb,
			const Math::tAabbf& cellBounds ) const;

	public:
		// building stuff
		tKdNode( );
		~tKdNode( );

		void fBuild( const tSortedKdData& potentialTris, const Math::tAabbf& parentVoxel, tAxis dontUseAxis );
		void fDestroy( b32 wasLoadedInPlace = false );
		u32 fMemorySizeRecursive( ) const;

	private:
		f32 fComputeSplitCost( const tSortedKdData& data, tAxis axis, f32 pos, const Math::tAabbf& parentVoxel );
		f32 fComputeLeafCost( const tSortedKdData& data );


	protected:
		tFixedArray< tLoadInPlacePtrWrapper< tKdNode >, 2 > mChildren;
		tEnum<tAxis, u32>	mAxis;
		f32					mSplitPosition; // along axis

		tDynamicArray< u32 > mItems; //only populated for leafs
		tDynamicArray< Math::tAabbf > mItemsBounds; //only populated for leafs
	};

	///
	/// \class tKdTree
	/// \brief Basic KD-Tree able to collect items by AABBs.
	class base_export tKdTree
	{
		declare_reflector( );

	public:
		tKdTree( );

		void fConstruct( const tDynamicArray< Math::tAabbf >& aabbs, b32 wasLoadedInPlace = false );
		u32 fMemorySizeRecursive( ) const;
		void fDraw( const Math::tMat3f& xform, Gfx::tDebugGeometryContainer& dgc, s32 level ) const;

		void fCollectItems(
			tGrowableArray<u32>& itemsOut,
			const Math::tAabbf& aabb ) const;

	protected:
		Math::tAabbf mBounds;
		tKdNode mFirstSplit;
	};

}

#endif//__tKdTree__

