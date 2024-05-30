#ifndef __tDynamicQuadtree__
#define __tDynamicQuadtree__
#include "tQuadtree.hpp"
#include "Math/tIntersectionAabbObb.hpp"

namespace Sig { namespace Gfx
{
	class tDebugGeometryContainer;
}}

namespace Sig
{
	class tDynamicQuadtreeRoot;

	template<class tVolume>
	static inline b32 fQuadtreeIntersectAabbWithVolume( const Math::tAabbf& aabb, const tVolume& v )
	{
		sigassert( !"shouldn't get here evar" );
		return false;
	}
	
	template<>
	static inline b32 fQuadtreeIntersectAabbWithVolume<Math::tFrustumf>( const Math::tAabbf& aabb, const Math::tFrustumf& v )
	{
		return v.fIntersects( aabb );
	}
	
	template<>
	static inline b32 fQuadtreeIntersectAabbWithVolume<Math::tAabbf>( const Math::tAabbf& aabb, const Math::tAabbf& v )
	{
		return aabb.fIntersects( v );
	}
	
	template<>
	static inline b32 fQuadtreeIntersectAabbWithVolume<Math::tObbf>( const Math::tAabbf& aabb, const Math::tObbf& v )
	{
		return Math::tIntersectionAabbObb<f32>( aabb, v ).fIntersects( );
	}
	
	template<>
	static inline b32 fQuadtreeIntersectAabbWithVolume<Math::tSpheref>( const Math::tAabbf& aabb, const Math::tSpheref& v )
	{
		return fQuadtreeIntersectAabbWithVolume<Math::tAabbf>( aabb, Math::tAabbf( v ) );
	}
	
	///
	/// \brief Recursive octree node for dynamic objects, splits and prunes as objects are
	/// added and removed to maintain a minimal memory footprint. Performance is quite suitable
	/// for dynamic objects, though if you know you're going to be moving many objects quickly
	/// over the course of several frames, you might consider putting them temporarily in
	/// the "fast-lane" (see below) until they've settled down.
	/// \note You should generally be using tDynamicQuadtreeRoot, rather than using tDynamicQuadtree directly.
	class base_export tDynamicQuadtree : public tQuadtree
	{
		friend class tDynamicQuadtreeRoot;
		define_class_pool_new_delete( tDynamicQuadtree, 512 );
	public:

		typedef tSpatialTree::tCellKey<tDynamicQuadtree> tCellKey;
		friend class tSpatialObject<tDynamicQuadtree>;
		typedef tSpatialObject<tDynamicQuadtree> tObject;
		typedef tObject* tObjectPtr;
		typedef tGrowableArray<tObjectPtr> tCellContents;

	private:

		Math::tAabbf								mBounds;
		tDynamicQuadtree*							mParent;
		s32											mIndex;
		tFixedArray<tDynamicQuadtree*,cCellCount>	mChildren;
		tCellContents								mContents;

	public:
		tDynamicQuadtree( );
		tDynamicQuadtree( const Math::tAabbf& bounds, tDynamicQuadtree* parent, s32 index );
		~tDynamicQuadtree( );

		const Math::tAabbf& fBounds( ) const { return mBounds; }
		u32 fDepth( ) const { u32 depth = 0; for( tDynamicQuadtree* parent = mParent; parent; parent = parent->mParent ) ++depth; return depth; }

		void fClearChildren( );
		void fClearContents( );
		void fClearAll( );
		void fClean( );
		
		b32 fInsert( 
			const tObjectPtr& object, 
			u32 depth, 
			u32 maxDepth,
			tDynamicQuadtree* ignore = 0 );

	private:

		void fRemove( u32 itemIndex );
		
	public:


		template<class tVolume>
		struct tIntersectVolumeCallback
		{
			inline b32 fQuickAabbTest( const tVolume& v, const tObjectPtr& i, b32 aabbWhollyContained ) const
			{
				return aabbWhollyContained || fQuadtreeIntersectAabbWithVolume( i->mWorldSpaceBox, v );
			}
			inline b32 fQuickAabbTest( const tVolume& v, const Math::tAabbf& worldSpaceBox, b32 aabbWhollyContained ) const
			{
				return aabbWhollyContained || fQuadtreeIntersectAabbWithVolume( worldSpaceBox, v );
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
			const tIntersectionOperator& intersectCb ) const;

		template<class tCollectionOperator>
		void fCollect(
			u32 depth,
			const tCollectionOperator& collectCb ) const;

		template<class tRayCastOperator>
		void fRayCastRecursive(
			u32 depth,
			const Math::tRayf& ray,
			const tRayCastOperator& rayCastCb ) const;

		void fRenderDebug( 
			u32 depth, 
			s32 targetDepth,
			b32 objectBoxesOnly,
			Gfx::tDebugGeometryContainer& debugGeom ) const;

	};

	template<class tVolume, class tIntersectionOperator>
	void tDynamicQuadtree::fIntersect(
		u32 depth,
		const tVolume& v,
		const tIntersectionOperator& intersectCb ) const
	{
		b32 recurse = true;

		const b32 intersects = fQuadtreeIntersectAabbWithVolume( mBounds, v );

		if( !intersects )
		{
			if( depth == 0 )
				recurse = false; // for the root, we might actually contain objects that are outside of the cell bounds
			else
				return; // quick rejection, don't have to test contents or children
		}

		// pass each object to the callback
		const u32 divBy4Count = ( mContents.fCount( ) / 4 ) * 4;
		for( u32 i = 0; i < divBy4Count; i += 4 )
		{
			intersectCb( v, mContents[ i + 0 ], false );
			intersectCb( v, mContents[ i + 1 ], false );
			intersectCb( v, mContents[ i + 2 ], false );
			intersectCb( v, mContents[ i + 3 ], false );
		}
		for( u32 i = divBy4Count; i < mContents.fCount( ); ++i )
			intersectCb( v, mContents[ i ], false );

		if( recurse )
		{
			// if we're here, we know that the frustum intersects our volume, so attempt to recurse on children
			for( u32 i = 0; i < mChildren.fCount( ); ++i )
			{
				if( mChildren[ i ] )
					mChildren[ i ]->fIntersect( depth + 1, v, intersectCb );
			}
		}
	}

	template<class tCollectionOperator>
	void tDynamicQuadtree::fCollect(
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

	template<class tRayCastOperator>
	void tDynamicQuadtree::fRayCastRecursive(
		u32 depth,
		const Math::tRayf& ray,
		const tRayCastOperator& rayCastCb ) const
	{
		b32 recurse = true;

		const b32 intersectsCell = mBounds.fIntersectsOrContains( ray );
		if( !intersectsCell )
		{
			if( depth == 0 )
				recurse = false; // for the root, we might actually contain objects that are outside of the cell bounds
			else
				return; // quick rejection, don't have to test contents or children
		}

		for( u32 i = 0; i < mContents.fCount( ); ++i )
			rayCastCb( ray, mContents[ i ] );

		if( recurse )
		{
			// if we're here, we know that the frustum intersects our volume, so attempt to recurse on children
			for( u32 i = 0; i < mChildren.fCount( ); ++i )
			{
				if( mChildren[ i ] )
					mChildren[ i ]->fRayCastRecursive( depth + 1, ray, rayCastCb );
			}
		}
	}

	///
	/// \brief Serves as a container for a recursive tDynamicQuadtree, storing root info including
	/// bounding box and maximum split depth. Access to the tDynamicQuadtree is mediated by this type.
	/// \todo For "move" functionality, test if object is still contained in existing cell within some threshold (maybe check size too), and skip removal/insertion if good enough
	/// \todo Add "fast-lane" cell which is just a single, unsplittable cell, for objects with many instances that are moving every frame (optimization)
	class tDynamicQuadtreeRoot : public tRefCounter
	{
	public:
		typedef tDynamicQuadtree::tObject			tObject;
		typedef tDynamicQuadtree::tObject*			tObjectPtr;

	private:
		u32					mMaxDepth;
		tDynamicQuadtree	mRoot;

	public:
		tDynamicQuadtreeRoot( );
		~tDynamicQuadtreeRoot( );

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
			mRoot.fIntersect( 0, v, intersectCb );
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
			sync_event_v_c( ray.mExtent, tSync::cSCRaycast );
			sync_event_v_c( ray.mOrigin, tSync::cSCRaycast );
			mRoot.fRayCastRecursive( 0, ray, rayCastCb );
		}

		void fRenderDebug( 
			s32 targetDepth, 
			b32 objectBoxesOnly,
			Gfx::tDebugGeometryContainer& debugGeom ) const;
	};

	typedef tRefCounterPtr<tDynamicQuadtreeRoot> tDynamicQuadtreeRootPtr;
}

#endif//__tDynamicQuadtree__
