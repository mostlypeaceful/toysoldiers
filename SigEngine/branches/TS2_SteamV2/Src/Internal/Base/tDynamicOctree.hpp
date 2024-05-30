#ifndef __tDynamicOctree__
#define __tDynamicOctree__
#include "tOctree.hpp"
#include "Math/tIntersectionAabbObb.hpp"

namespace Sig { namespace Gfx
{
	class tDebugGeometryContainer;
}}

namespace Sig
{
	class tDynamicOctreeRoot;

	template<class tVolume>
	static inline b32 fOctreeIntersectAabbWithVolume( const Math::tAabbf& aabb, const tVolume& v, b32* aabbWhollyContained = 0 )
	{
		sigassert( !"shouldn't get here evar" );
		return false;
	}

	template<>
	static inline b32 fOctreeIntersectAabbWithVolume<Math::tFrustumf>( const Math::tAabbf& aabb, const Math::tFrustumf& v, b32* aabbWhollyContained )
	{
		if( aabbWhollyContained )
			return v.fIntersectsOrContains( aabb, *aabbWhollyContained );
		return v.fIntersects( aabb );
	}

	template<>
	static inline b32 fOctreeIntersectAabbWithVolume<Math::tAabbf>( const Math::tAabbf& aabb, const Math::tAabbf& v, b32* aabbWhollyContained )
	{
		const b32 intersects = aabb.fIntersects( v );
		if( aabbWhollyContained )
			*aabbWhollyContained = intersects && v.fContains( aabb );
		return intersects;
	}

	template<>
	static inline b32 fOctreeIntersectAabbWithVolume<Math::tObbf>( const Math::tAabbf& aabb, const Math::tObbf& v, b32* aabbWhollyContained )
	{
		Math::tIntersectionAabbObb<f32> intersection( aabb, v );
		if( aabbWhollyContained )
			*aabbWhollyContained = intersection.fContained( );
		return intersection.fIntersects( );
	}

	template<>
	static inline b32 fOctreeIntersectAabbWithVolume<Math::tSpheref>( const Math::tAabbf& aabb, const Math::tSpheref& v, b32* aabbWhollyContained )
	{
		return fOctreeIntersectAabbWithVolume<Math::tAabbf>( aabb, Math::tAabbf( v ), aabbWhollyContained );
	}

	///
	/// \brief Recursive octree node for dynamic objects, splits and prunes as objects are
	/// added and removed to maintain a minimal memory footprint. Performance is quite suitable
	/// for dynamic objects, though if you know you're going to be moving many objects quickly
	/// over the course of several frames, you might consider putting them temporarily in
	/// the "fast-lane" (see below) until they've settled down.
	/// \note You should generally be using tDynamicOctreeRoot, rather than using tDynamicOctree directly.
	class base_export tDynamicOctree : public tOctree
	{
		friend class tDynamicOctreeRoot;
		define_class_pool_new_delete( tDynamicOctree, 1024 );
	public:

		typedef tSpatialTree::tCellKey<tDynamicOctree> tCellKey;
		friend class tSpatialObject<tDynamicOctree>;
		typedef tSpatialObject<tDynamicOctree> tObject;
		typedef tObject* tObjectPtr;
		typedef tGrowableArray<tObjectPtr> tCellContents;

	private:

		Math::tAabbf							mBounds;
		tDynamicOctree*							mParent;
		s32										mIndex;
		tFixedArray<tDynamicOctree*,cCellCount>	mChildren;
		tCellContents							mContents;

	public:
		tDynamicOctree( );
		tDynamicOctree( const Math::tAabbf& bounds, tDynamicOctree* parent, s32 index );
		~tDynamicOctree( );

		const Math::tAabbf& fBounds( ) const { return mBounds; }
		u32 fDepth( ) const { u32 depth = 0; for( tDynamicOctree* parent = mParent; parent; parent = parent->mParent ) ++depth; return depth; }

		void fClearChildren( );
		void fClearContents( );
		void fClearAll( );
		void fClean( );
		
		b32 fInsert( 
			const tObjectPtr& object, 
			u32 depth, 
			u32 maxDepth,
			tDynamicOctree* ignore = 0 );

	private:
		void fRemove( u32 itemIndex );
		
	public:

		template<class tVolume>
		struct tIntersectVolumeCallback
		{
			inline b32 fQuickAabbTest( const tVolume& v, const tObjectPtr& i, b32 aabbWhollyContained ) const
			{
				return aabbWhollyContained || fOctreeIntersectAabbWithVolume( i->mWorldSpaceBox, v );
			}

			///
			/// \brief This method will be called once for each potentially
			/// intersecting object (i.e., the volume is guaranteed to intersect
			/// the object's world box).
			//void operator()( const tVolume& v, const tObjectPtr& i, b32 aabbWhollyContained ) const;
		};

		typedef tIntersectVolumeCallback<Math::tFrustumf>	tIntersectFrustumCallback;
		typedef tIntersectVolumeCallback<Math::tAabbf>		tIntersectAabbCallback;
		typedef tIntersectVolumeCallback<Math::tObbf>		tIntersectObbCallback;
		typedef tIntersectVolumeCallback<Math::tSpheref>	tIntersectSphereCallback;

		struct base_export tRayCastCallback
		{
			///
			/// \brief This method will be called once for each potentially
			/// intersecting object (i.e., the ray is guaranteed to intersect
			/// the object's world box).
			//void operator()( const Math::tRayf& ray, const tObjectPtr& i ) const = 0;
		};

	private:

		void fUpdateBounds( const Math::tAabbf& aabb );

		template<class tVolume, class tIntersectionOperator>
		void fIntersect(
			u32 depth,
			const tVolume& v,
			b32 aabbWhollyContained,
			const tIntersectionOperator& intersectCb ) const;

		template<class tCollectionOperator>
		void fCollect(
			u32 depth,
			const tCollectionOperator& collectCb ) const;

		void fRayCastCollect(
			u32 depth,
			const Math::tRayf& ray,
			tGrowableArray<const tDynamicOctree*>& cells ) const;

		void fRenderDebug( 
			u32 depth, 
			s32 targetDepth,
			b32 objectBoxesOnly,
			Gfx::tDebugGeometryContainer& debugGeom ) const;

	};

	template<class tVolume, class tIntersectionOperator>
	void tDynamicOctree::fIntersect(
		u32 depth,
		const tVolume& v,
		b32 aabbWhollyContained,
		const tIntersectionOperator& intersectCb ) const
	{
		b32 recurse = true;

		if( !aabbWhollyContained )
		{
			const b32 intersects = fOctreeIntersectAabbWithVolume( mBounds, v, &aabbWhollyContained );

			if( !intersects )
			{
				if( depth == 0 )
				{
					recurse = false; // for the root, we might actually contain objects that are outside of the cell bounds
					aabbWhollyContained = false;
				}
				else
					return; // quick rejection, don't have to test contents or children
			}
		}

		// pass each object to the callback
		const u32 divBy4Count = ( mContents.fCount( ) / 4 ) * 4;
		for( u32 i = 0; i < divBy4Count; i += 4 )
		{
			intersectCb( v, mContents[ i + 0 ], aabbWhollyContained );
			intersectCb( v, mContents[ i + 1 ], aabbWhollyContained );
			intersectCb( v, mContents[ i + 2 ], aabbWhollyContained );
			intersectCb( v, mContents[ i + 3 ], aabbWhollyContained );
		}
		for( u32 i = divBy4Count; i < mContents.fCount( ); ++i )
			intersectCb( v, mContents[ i ], aabbWhollyContained );

		if( recurse )
		{
			// if we're here, we know that the frustum intersects our volume, so attempt to recurse on children
			for( u32 i = 0; i < mChildren.fCount( ); ++i )
			{
				if( mChildren[ i ] )
					mChildren[ i ]->fIntersect( depth + 1, v, aabbWhollyContained, intersectCb );
			}
		}
	}

	template<class tCollectionOperator>
	void tDynamicOctree::fCollect(
		u32 depth,
		const tCollectionOperator& collectCb ) const
	{
		// pass each object to the callback
		const u32 divBy4Count = ( mContents.fCount( ) / 4 ) * 4;
		for( u32 i = 0; i < divBy4Count; i += 4 )
		{
			collectCb( mContents[ i + 0 ] );
			collectCb( mContents[ i + 1 ] );
			collectCb( mContents[ i + 2 ] );
			collectCb( mContents[ i + 3 ] );
		}
		for( u32 i = divBy4Count; i < mContents.fCount( ); ++i )
			collectCb( mContents[ i ] );

		// recurse on children
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
		{
			if( mChildren[ i ] )
				mChildren[ i ]->fCollect( depth + 1, collectCb );
		}
	}

	///
	/// \brief Serves as a container for a recursive tDynamicOctree, storing root info including
	/// bounding box and maximum split depth. Access to the tDynamicOctree is mediated by this type.
	/// \todo For "move" functionality, test if object is still contained in existing cell within some threshold (maybe check size too), and skip removal/insertion if good enough
	/// \todo Add "fast-lane" cell which is just a single, unsplittable cell, for objects with many instances that are moving every frame (optimization)
	class tDynamicOctreeRoot : public tRefCounter
	{
	public:
		typedef tDynamicOctree::tObject				tObject;
		typedef tDynamicOctree::tObject*			tObjectPtr;

	private:
		u32					mMaxDepth;
		tDynamicOctree		mRoot;

	public:
		tDynamicOctreeRoot( );
		~tDynamicOctreeRoot( );

		u32 fMaxDepth( ) const { return mMaxDepth; }
		const Math::tAabbf& fBounds( ) const { return mRoot.fBounds( ); }

		void fCreate( const Math::tAabbf& rootBounds, u32 maxDepth = 5 );
		void fClear( );

		void fClean( );

		void fInsert( const tObjectPtr& object );
		void fRemove( const tObjectPtr& object );
		void fMove	( const tObjectPtr& object );

		template<class tVolume, class tIntersectionOperator>
		void fIntersect(
			const tVolume& v,
			const tIntersectionOperator& intersectCb ) const
		{
			mRoot.fIntersect( 0, v, false, intersectCb );
		}

		template<class tCollectionOperator>
		void fCollect(
			const tCollectionOperator& collectCb ) const
		{
			mRoot.fCollect( 0, collectCb );
		}

		template<class tRayCastOperator>
		void fRayCast(
			const Math::tRayf& ray,
			const tRayCastOperator& rayCastCb ) const
		{
			// first gather all potential cells
			tGrowableArray<const tDynamicOctree*> potentialCells;
			mRoot.fRayCastCollect( 0, ray, potentialCells );

			// go through all potential cells
			for( u32 icell = 0; icell < potentialCells.fCount( ); ++icell )
			{
				// the ray intersects this cell, so pass each leaf object to the callback after
				// first performing a rough AABB/ray test

				const tDynamicOctree::tCellContents& contents = potentialCells[ icell ]->mContents;
				for( u32 i = 0; i < contents.fCount( ); ++i )
					rayCastCb( ray, contents[ i ] );
			}
		}

		void fRenderDebug( 
			s32 targetDepth, 
			b32 objectBoxesOnly,
			Gfx::tDebugGeometryContainer& debugGeom ) const;
	};

	typedef tRefCounterPtr<tDynamicOctreeRoot> tDynamicOctreeRootPtr;
}

#endif//__tDynamicOctree__
