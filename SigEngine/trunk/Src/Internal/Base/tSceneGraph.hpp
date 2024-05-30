#ifndef __tSceneGraph__
#define __tSceneGraph__
#include "tEntity.hpp"
#include "tLogicThreadPool.hpp"
#include "Gfx/tDebugGeometry.hpp"
#include "Gfx/tRenderableEntity.hpp" // Required for tRenderableEntity::c* references bellow on GCC/iOS :(

namespace Sig
{
	namespace Gfx { class tCamera; class tPotentialVisibilitySet; }
	namespace Physics { class tPhysicsWorld; }

	class tSceneGraph;	
	define_smart_ptr( base_export, tRefCounterPtr, tSceneGraph );

	class tLinearFrustumCulling;

	///
	/// \brief The scene graph is the primary dynamic(*) object repository for the application.
	/// Note that this scene graph might not be considered a completely "traditional" scene graph.
	///
	/// The scene graph can be thought of as an entity database. While it's primary role is
	/// to provide traditional parent/child scene graph tree structure, it also encapsulates
	/// an optimized spatial set and entity run lists. Fundamentally, the scene graph is
	/// designed to answer different forms of entity queries efficiently, while remaining
	/// true to the logical relationships between entities.
	///
	/// The scene graph contains entities of all types. The word "dynamic" above does not
	/// just mean objects that move every frame, but rather objects that are considered
	/// dynamic in the sense of application scope. For example, static geometry, while
	/// static compared to a moving character, is still a dynamic object in the sense
	/// that it may be loaded from file and later destroyed. Additionally, the scene graph
	/// stores purely logical entities in addition to spatial entities; Gui objects are an
	/// example of such position-less entities.
	class base_export tSceneGraph : public tUncopyable, public tRefCounter
	{
		friend class tLogic;
		friend class tEntity;
	public:
		static u32 fNextSpatialSetIndex( );
		static b32 fForceRunListsST( );

	private:
		struct tSpawnData
		{
			tEntityPtr mChild;
			tEntityPtr mParent;
			inline tSpawnData( ) { }
			inline tSpawnData( const tEntityPtr& child, const tEntityPtr& parent ) : mChild( child ), mParent( parent ) { }
			inline b32 operator==( const tSpawnData& sd ) const { return mChild == sd.mChild; } // only the child matters for equality (it's the thing being spawned)
			inline b32 operator==( tEntity* ent ) const { return mChild == ent; }
		};

		struct tSpatialSet : public tEntityBVHRoot
		{
			tGrowableArray<tSpatialEntity*> mDirtyObjects;
		};
		typedef tRefCounterPtr<tSpatialSet> tSpatialSetPtr;
		typedef tGrowableArray<tSpatialSetPtr> tSpatialSetList;

		typedef tEntity											tRootEntity;
		typedef tEntity::tSpatialSetObject						tSpatialSetObject;
		typedef tGrowableArray<tSpawnData>						tSpawnList;
		typedef tGrowableArray<tEntityPtr>						tDeletionList;
		typedef tGrowableArray<tLogic*>							tStandAloneLogicList;

	public:
		typedef tGrowableArray<tEntityCloud*>					tCloudList;

	private:
		mutable Gfx::tDebugGeometryContainer					mDebugGeometry;
		mutable tSpatialSetList									mSpatialSets;
		tCloudList												mCloudList;
		tRootEntity												mRootEntity, mPreserveRoot;
		mutable tRefCounterPtr< tLinearFrustumCulling >			mRenderCulling;
		tRefCounterPtr< Gfx::tPotentialVisibilitySet >			mPotentialVisibilitySet;

		tLogic::tRunListSet										mRunLists;
		tLogicThreadPoolPtr										mLogicThreadPool;
		tStandAloneLogicList									mStandAloneLogic;

		tSpawnList												mSpawnList;
		tDeletionList											mDeletionList;
		tDeletionList											mPreserveList;
		f32														mElapsedTime;
		f32														mPausableTime;
		f32														mFrameDeltaTime;
		b16														mPaused;
		b16														mPauseNextFrame;
		u32														mNumDups;
		b32														mInMTRunList;
		u32														mNextLogicGuid;

		f32														mLastCoRenderTime;

		tRefCounterPtr<Physics::tPhysicsWorld>					mPhysics;

		Gfx::tScreen*											mScreen;

	private:
		tLogic::tRunListSet& fRunLists( ) { return mRunLists; } // available only for tEntity to add/remove self

	public:
		void fLogLogicCountsByType( );
		void fCaptureLogicCountByTypeDeltaReferenceFrame( );

	public:
		explicit tSceneGraph( const tLogicThreadPoolPtr& threadPool = tLogicThreadPoolPtr( ) );
	private:
		void fCommonCtor( );

	public:
		~tSceneGraph( );

		void fSetScreen( Gfx::tScreen* screen ) { mScreen = screen; }
		Gfx::tScreen* fScreen( ) { return mScreen; }

		///
		/// \brief Get root entity
		tEntity& fRootEntity( ) { return mRootEntity; }
		const tEntity& fRootEntity( ) const { return mRootEntity; }

		/// \brief Access to logic guids
		u32 fNextLogicGuid( ) { return ++mNextLogicGuid; }
		void fResetLogicGuids( ) { mNextLogicGuid = 0; }

		///
		/// \brief Get total elapsed time (elapsed time is the sum of all the 'dt' supplied to each call to fAdvanceTime).
		f32	fElapsedTime( ) const { return mElapsedTime; }

		///
		/// \brief Get total elapsed time, but accounting for pausing (doesn't increase when scene graph is paused).
		f32	fPausableTime( ) const { return mPausableTime; }

		///
		/// \brief Delta time for the current frame
		f32 fFrameDeltaTime( ) const { return mFrameDeltaTime; }

		///
		/// \brief For debugging/logging, query how long the co-render took
		f32 fLastCoRenderTime( ) const { return mLastCoRenderTime; }

		///
		/// \brief Access the debug geometry container.
		Gfx::tDebugGeometryContainer& fDebugGeometry( ) const { return mDebugGeometry; }

		///
		/// \brief Reset the bounds of the enitty spatial set.
		void fSetSpatialBounds( const Math::tAabbf& bounds, u32 maxDepth = 5 );

		///
		/// \brief Reset the bounds of the enitty spatial set.
		void fSetSpatialBounds( f32 worldHalfAxisLength, f32 xAxisOffset, f32 yAxisOffset, f32 zAxisOffset, u32 maxDepth = 5 );

		///
		/// \brief Clear the scene graph (clears everything).
		void fClear( );

		///
		/// \brief Removes all "unpreserved" entities (entities that have not explicitly been marked for preserve)
		void fRemoveUnpreserved( );

		///
		/// \brief Distribute each iteration in a for-loop.
		/// \note This is only valid from within a single-threaded run list (or other time when we are not running the logic threads).
		void fDistributeForLoop( Threads::tDistributedForLoopCallback& cb, u32 loopCount );

		///
		/// \brief Wait for all logic tasks to complete.
		void fWaitForLogicRunListsToComplete( );

		///
		/// \brief control what hw threads logic is allowed to run on
		void fDisallowLogicThread( u32 hwThreadId );
		void fAllowLogicThread( u32 hwThreadId );

		///
		/// \brief Step forward in time by specified time increment. Performs updating of entities, various other per-frame maintenance.
		void fAdvanceTime( f32 dt );
		void fKickCoRenderMTRunList( );

		///
		/// \brief If you want to advance time without actually performing any of the per-frame logic associated with fAdvanceTime, then this is your method.
		void fIncrementTimeCounters( f32 dt );

		///
		/// \brief Pause the simulation/scene... next frame.
		void fPauseNextFrame( b32 pause );

		///
		/// \brief Is the simulation/scene paused?
		b32 fIsPaused( ) const { return mPaused; }
		b32 fWillPauseOnNextFrame( ) const { return mPauseNextFrame; }

		///
		/// \brief See if we are currently processing multi-threaded run lists (i.e., generally for debugging).
		b32 fInMTRunList( ) const { return mInMTRunList; }

		void fPrepareEntityClouds( const Gfx::tCamera & camera );

		///
		/// \brief Obtain all triangles in world space intersecting the specified world space volume.
		template< class t >
		void fCollectTris( const Math::tAabbf& v, t& collector );

		///
		/// \brief Obtain all triangles in world space intersecting the specified world space volume.
		template< class t >
		void fCollectTris( const Math::tObbf& v, t& collector );

		template<class vol, class inter>
		struct tVolumeCallback
		{
			const vol & mV;
			const inter & mI;
			tDelegate<void( tEntityBVH::tObjectPtr* objects, u32 count )> mDelegate;

			inline tVolumeCallback( const vol & v, const inter & i )
				: mV( v ), mI( i ) 
			{
				mDelegate = make_delegate_memfn( 
					tDelegate<void( tEntityBVH::tObjectPtr*, u32 )>, tVolumeCallback, fCallback );
			}

			void fCallback( tEntityBVH::tObjectPtr* objs, u32 count ) {
				for( u32 i = 0; i < count; ++i )
					mI( mV, objs[ i ], false );
			}

		};

		///
		/// \brief Visit all the entities whose bounds intersect the specified volume. You can define your
		/// derived tIntersectionOperator to perform custom functionality (such as storing the entities in a container, etc).
		template<class tVolume, class tIntersectionOperator>
		void fIntersect( const tVolume& v, const tIntersectionOperator& intersectCb ) const
		{
			for( u32 spatialSetIndex = 0; spatialSetIndex < mSpatialSets.fCount( ); ++spatialSetIndex )
				mSpatialSets[ spatialSetIndex ]->fIntersect( v, intersectCb );
		}
		template<class tVolume, class tIntersectionOperator>
		void fIntersect( const tVolume& v, const tIntersectionOperator& intersectCb, u32 spatialSetIndex ) const
		{
			if( spatialSetIndex < mSpatialSets.fCount( ) )
				mSpatialSets[ spatialSetIndex ]->fIntersect( v, intersectCb );
		}
		template<class tVolume>
		void fIntersectDeferred( const tVolume& v, tGrowableArray< void* >& intersectingOwners, u32 spatialSetIndex ) const
		{
			if( spatialSetIndex < mSpatialSets.fCount( ) )
				mSpatialSets[ spatialSetIndex ]->fIntersectDeferred( v, intersectingOwners );
		}

		///
		/// \brief Visit all the tEntityClouds and gather entities whose bounds interesect the specified volume.
		template<class tVolume, class tIntersectionOperator>
		void fIntersectCloudRenderables( 
			const tVolume & v, 
			const tIntersectionOperator & intersectCb,
			b32 forShadows ) const;

		///
		/// \brief Visit all spatial entities in the scene graph. You can define your
		/// derived tCollectionOperator to perform custom functionality (such as storing the entities in a container, etc).
		template<class tCollectionOperator>
		void fCollect( const tCollectionOperator& collectCb ) const
		{
			for( u32 spatialSetIndex = 0; spatialSetIndex < mSpatialSets.fCount( ); ++spatialSetIndex )
				mSpatialSets[ spatialSetIndex ]->fCollect( collectCb );
		}
		template<class tCollectionOperator>
		void fCollect( const tCollectionOperator& collectCb, u32 spatialSetIndex ) const
		{
			if( spatialSetIndex < mSpatialSets.fCount( ) )
				mSpatialSets[ spatialSetIndex ]->fCollect( collectCb );
		}

		typedef tEntityBVH::tRayCastCallback tRayCastCallback;

		struct base_export tDefaultRayCastCallback : public tRayCastCallback
		{
			mutable Math::tRayCastHit	mHit;
			mutable tEntity*			mFirstEntity;
			tEntity* const *			mIgnoreList;
			u32							mNumToIgnore;

			tDefaultRayCastCallback( ) : mFirstEntity( 0 ), mIgnoreList( 0 ), mNumToIgnore( 0 ) { }
			tDefaultRayCastCallback( tEntity* const* ignoreList, u32 numToIgnore ) : mFirstEntity( 0 ), mIgnoreList( ignoreList ), mNumToIgnore( numToIgnore ) { }
			void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const;
			tSpatialEntity* fCullAgainstIgnoreList( tEntityBVH::tObjectPtr i ) const;
		};

		///
		/// \brief Visit all the entities whose bounds intersect the specified ray. You can define your
		/// own tRayCastCallback type to perform custom functionality.
		template<class tRayCastOperator>
		void fRayCast( const Math::tRayf& ray, const tRayCastOperator& rayCastCallback ) const
		{
			for( u32 spatialSetIndex = 0; spatialSetIndex < mSpatialSets.fCount( ); ++spatialSetIndex )
				fRayCast( ray, rayCastCallback, spatialSetIndex );
		}

		template<class tRayCastOperator>
		void fRayCast( const Math::tRayf& ray, const tRayCastOperator& rayCastCallback, u32 spatialSetIndex ) const
		{
			sigassert( !fEqual( ray.mExtent.fLengthSquared( ), 0.f ) && "tSceneGraph::fRayCast - ray length is zero! Results will be bogus. Future checks are omitted for performance." );
			sigassert( spatialSetIndex < mSpatialSets.fCount( ) );
			mSpatialSets[ spatialSetIndex ]->fRayCast( ray, rayCastCallback );
		}

		///
		/// \brief #include" tSceneGraphCollectTris.hpp" if you plan to use this function.
		template<class tRayCastOperator>
		void fRayCastAgainstRenderable( const Math::tRayf& ray, const tRayCastOperator& rayCastCallback ) const
		{
			// Originally this implementation was in tSceneGraphCollectTris.hpp, but this interacted poorly with base_export building Base as a dll for metro.
			// I assume it was there to break a circular dependency or, more likely, to reduce the amount of rebuild spam when modifying tRenderableEntity.
			// Feel free to move it back if/when we go back to staticly linking on metro.  Or don't.  --mrickert
			fRayCast( ray, rayCastCallback, Gfx::tRenderableEntity::cSpatialSetIndex );
			fRayCast( ray, rayCastCallback, Gfx::tRenderableEntity::cHeightFieldSpatialSetIndex );
		}

		///
		/// \brief Render debug geometry corresponding to the cells in the spatial set (octree) and their contained entities.
		void fVisualizeSpatialSetDebug( s32 targetDepth, b32 objectBoxesOnly ) const;
		void fVisualizeSubObjectSpatialDebug( s32 targetDepth ) const;

		///
		/// \brief
		Physics::tPhysicsWorld* fPhysics( ) { return mPhysics.fGetRawPtr( ); }

		Gfx::tPotentialVisibilitySet* fPotentialVisibilitySet( ) { return mPotentialVisibilitySet.fGetRawPtr( ); }

	private:

		void fUpdatePausedState( );
		void fSyncSpatialSet( u32 spatialSetIndex ) const;
		void fSyncAllSpatialSets( ) const;
		void fCleanSpatialSet( );
		void fProcessSpawns( );
		void fProcessDeletions( );

		void fInsertStandAloneLogic( tLogic& l );
		void fRemoveStandAloneLogic( tLogic& l );

		void fInsert(			tEntity& e );
		void fRemove(			tEntity& e );
		void fSpawn(			tEntity& e, tEntity& parent );
		void fDelete(			tEntity& e );
		void fPreserve(			tEntity& e );
		void fProcessRunListST( tLogic::tRunListId runListId, u32 profilerId, f32 dt );
		void fProcessRunListMT( tLogic::tRunListId runListId, u32 profilerId, f32 dt );

		b32  fInDeletionList( const tEntity& e ) const;

	public:

		void fSpatialMove(		tSpatialEntity& e );
		void fSpatialInsert(	tSpatialEntity& e );
		void fSpatialRemove(	tSpatialEntity& e );

		void fCloudInsert( tEntityCloud & e );
		void fCloudRemove( tEntityCloud & e );

		b32 fInSpawnList( tEntity* e );

		tLinearFrustumCulling* fRenderCulling( ) const { return mRenderCulling.fGetRawPtr( ); }

		const tCloudList& fCloudList( ) const { return mCloudList; }
		void fInitGCInstancing( );

	public: // script-specific
		static void	fExportScriptInterface( tScriptVm& vm );
	};

}

#endif//__tSceneGraph__
