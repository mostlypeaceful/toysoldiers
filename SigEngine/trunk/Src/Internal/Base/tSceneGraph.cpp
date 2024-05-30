#include "BasePch.hpp"
#include "tApplication.hpp"
#include "tSpatialEntity.hpp"
#include "tProfiler.hpp"
#include "tEntityCloud.hpp"
#include "Physics/tPhysicsWorld.hpp"
#include "tLinearFrustumCulling.hpp"
#include "Gfx/tPotentialVisibilitySet.hpp"

//for debug rendering
#include "tMeshEntity.hpp"
#include "tMesh.hpp"

#ifdef sig_logging
#include "tSceneRefEntity.hpp"
#endif//sig_logging

namespace Sig
{
//#define spatial_correctness


	devvar( bool, Debug_SceneGraph_ForceRunListsST, false );
	devvar( bool, Debug_SceneGraph_DumpLogicCountsByType, false );
	devvar( bool, Debug_SceneGraph_CaptureLCBTDeltaReferenceFrame, false );
	devvar( bool, Debug_SceneGraph_DumpLogicCountsByTypeDeltasOnly, false );
	devvar( bool, Debug_SceneGraph_DumpPositiveLogicCountsByTypeDeltasOnly, false );
	devvar( bool, Debug_SceneGraph_DumpLogicsMissingResource, false );

#ifdef sig_logging
	typedef tHashTable<tStringPtr,u32> tLogicByDebugTypeNameTable;
	static tLogicByDebugTypeNameTable gReferenceInSg;
	static tLogicByDebugTypeNameTable gReferenceOutSg;

	static void fLogEntitiesMissingResource( )
	{
		const tGrowableArray<tLogic*>& globalLogicTracker = tLogic::fGlobalLogicTracker( );
		for( u32 i = 0; i < globalLogicTracker.fCount( ); ++i )
		{
			tLogic& logic = *globalLogicTracker[ i ];
			tSceneRefEntity* sceneRefEnt = logic.fDynamicCast< tSceneRefEntity >( );
			if( sceneRefEnt )
			{
				const tResourcePtr& res = sceneRefEnt->fSgResource( );
				if( !res->fLoaded( ) )
					log_warning( "!@ENTITY WITHOUT RESOURCE@! (" << sceneRefEnt << ") '" << sceneRefEnt->fDebugTypeName( ) << "' missing resource(" << &res << "). path: " << res->fAbsolutePhysicalPath( ) );
			}
		}
	}

	static void fCountLogicsByType( tLogicByDebugTypeNameTable& inSg, tLogicByDebugTypeNameTable& outOfSg )
	{
		const tGrowableArray<tLogic*>& globalLogicTracker = tLogic::fGlobalLogicTracker( );
		for( u32 i = 0; i < globalLogicTracker.fCount( ); ++i )
		{
			tLogic& logic = *globalLogicTracker[ i ];
			tLogicByDebugTypeNameTable& table = logic.fSceneGraph( ) ? inSg : outOfSg;

			if( tSceneRefEntity* sceneRefEnt = logic.fDynamicCast< tSceneRefEntity >( ) )
				++table[ tStringPtr( sceneRefEnt->fSgResource( )->fGetPath( ).fCStr( ) ) ];
			else
				++table[ tStringPtr( logic.fDebugTypeName( ) ) ];
		}
	}
	static void fLogLogicCountsByType( const tLogicByDebugTypeNameTable& table, const char* text )
	{
		u32 total = 0;
		tGrowableArray<std::string> output;
		for( tLogicByDebugTypeNameTable::tConstIterator i = table.fBegin( ), iend = table.fEnd( ); i != iend; ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			total += i->mValue;
			std::stringstream ss; ss << std::setw( 8 ) << i->mValue << ": " << i->mKey.fCStr( );
			output.fPushBack( ss.str( ) );
		}
		std::sort( output.fBegin( ), output.fEnd( ) );
		
		log_line( 0, "---------> " << text << " (begin) <---------" );
		for( u32 i = 0; i < output.fCount( ); ++i )
			log_line( 0, output[ i ] );
		log_line( 0, "Total Logics by type: " << total );
		log_line( 0, "---------> " << text << " (end) <---------" );
	}

	static void fLogLogicCountsByTypeDeltas( 
		const tLogicByDebugTypeNameTable& current,
		const tLogicByDebugTypeNameTable& previous,
		const char* text )
	{
		u32 newTotal = 0;
		tGrowableArray<std::string> output;
		for( tLogicByDebugTypeNameTable::tConstIterator i = current.fBegin( ), iend = current.fEnd( ); i != iend; ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;

			u32 oldValue = 0;
			if( u32 * oldValuePtr = previous.fFind( i->mKey ) )
				oldValue = *oldValuePtr;

			newTotal += i->mValue;

			const s32 diff = i->mValue - oldValue;

			// Only print changes
			if( !diff )
				continue;

			if( Debug_SceneGraph_DumpPositiveLogicCountsByTypeDeltasOnly && diff < 0 )
				continue;

			std::stringstream ss; 
			ss << i->mKey.fCStr( ) << ": " << oldValue << " ---> " << i->mValue << " (";
			if( diff > 0 )
				ss << "+";
			ss << diff << ")";

			output.fPushBack( ss.str( ) );
		}

		std::sort( output.fBegin( ), output.fEnd( ) );

		u32 oldTotal = 0;
		for( tLogicByDebugTypeNameTable::tConstIterator i = previous.fBegin( ), iend = previous.fEnd( ); i != iend; ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;

			oldTotal += i->mValue;
		}

		log_line( 0, "---------> " << text << " (begin) <---------" );
		for( u32 i = 0; i < output.fCount( ); ++i )
			log_line( 0, output[ i ] );
		log_line( 0, "Total Logics by type: " << oldTotal << " ---> " << newTotal << " (" << ( newTotal > oldTotal ? "+" : "" ) << (s32)(newTotal - oldTotal) << ")" );
		log_line( 0, "---------> " << text << " (end) <---------" );
	}
#endif//sig_logging

	void tSceneGraph::fLogLogicCountsByType( )
	{
#ifdef sig_logging

		if( tLogic::fGlobalLogicTracker( ).fCount( ) == 0 )
		{
			log_warning( "fLogLogicCountsByType() called but there are no entities in the GlobalLogicTracker!  Ensure 'sig_tracklogics' is defined in tLogic.cpp" );
			return;
		}

		tLogicByDebugTypeNameTable inSg( 128 ), outOfSg( 128 );
		fCountLogicsByType( inSg, outOfSg );
		
		if( !Debug_SceneGraph_DumpLogicCountsByTypeDeltasOnly )
		{
			::Sig::fLogLogicCountsByType( inSg, "Logic in Scene Graph" );
			::Sig::fLogLogicCountsByType( outOfSg, "Logic orphaned" );
		}
		else
		{
			::Sig::fLogLogicCountsByTypeDeltas( 
				inSg, gReferenceInSg, "Delta of Logic in Scene Graph" );

			::Sig::fLogLogicCountsByTypeDeltas( 
				outOfSg, gReferenceOutSg, "Delta of Logic orphaned" );
		}
		
#endif//sig_logging
	}

	namespace
	{
		// returns true if this spatial set should be done through the rendering culling system
		b32 fRenderCullSpatialSet( u32 spatialIndex )
		{
			return (spatialIndex == Gfx::tRenderableEntity::cSpatialSetIndex || spatialIndex == Gfx::tRenderableEntity::cEffectSpatialSetIndex );
		}
	}

	void tSceneGraph::fCaptureLogicCountByTypeDeltaReferenceFrame( )
	{
#ifdef sig_logging
		gReferenceInSg.fClear( );
		gReferenceOutSg.fClear( );
		fCountLogicsByType( gReferenceInSg, gReferenceOutSg );
#endif // sig_logging
	}

	u32 tSceneGraph::fNextSpatialSetIndex( )
	{
		static u32 gNext = 1;
		return gNext++;
	}

	//------------------------------------------------------------------------------
	b32 tSceneGraph::fForceRunListsST( )
	{
		return Debug_SceneGraph_ForceRunListsST;
	}

	tSceneGraph::tSceneGraph( const tLogicThreadPoolPtr& threadPool )
		: mLogicThreadPool( threadPool )
	{
		if( !mLogicThreadPool )
			mLogicThreadPool.fReset( NEW tLogicThreadPool( ) );
		fCommonCtor( );
	}

	void tSceneGraph::fCommonCtor( )
	{
		fResetLogicGuids( );
		mElapsedTime = 0.f;
		mPausableTime = 0.f;
		mFrameDeltaTime = 0.f;
		mPaused = false;
		mPauseNextFrame = false;
		mNumDups = 0;
		mInMTRunList = false;
		mLastCoRenderTime = 0.f;

		// insert the root entity; it will never get removed
		fInsert( mRootEntity );
		fInsert( mPreserveRoot );

		// to ensure the root entity never gets deleted,
		// we add ref it here (we might pass out smart pointers to it)
		mRootEntity.fAddRef( );
		mPreserveRoot.fAddRef( );

		mSpatialSets.fPushBack( tSpatialSetPtr( NEW tSpatialSet ) );
		mRenderCulling.fReset( NEW tLinearFrustumCulling( ) );
		mPotentialVisibilitySet.fReset( NEW Gfx::tPotentialVisibilitySet( ) );
		mPhysics.fReset( NEW Physics::tPhysicsWorld( ) );
		mScreen = NULL;
	}

	tSceneGraph::~tSceneGraph( )
	{
		fClear( );

		mPreserveRoot.fDecRef( );
		mRootEntity.fDecRef( );
		
		fRemove( mRootEntity );
		fRemove( mPreserveRoot );
	}

	void tSceneGraph::fSetSpatialBounds( const Math::tAabbf& bounds, u32 maxDepth )
	{
		for( u32 i = 0; i < mSpatialSets.fCount( ); ++i )
			mSpatialSets[ i ]->fCreate( bounds, maxDepth );
	}

	void tSceneGraph::fSetSpatialBounds( f32 worldHalfAxisLength, f32 xAxisOffset, f32 yAxisOffset, f32 zAxisOffset, u32 maxDepth )
	{
		fSetSpatialBounds( Math::tAabbf(
			Math::tVec3f( -worldHalfAxisLength + xAxisOffset, -worldHalfAxisLength + yAxisOffset, -worldHalfAxisLength + zAxisOffset ), 
			Math::tVec3f( +worldHalfAxisLength + xAxisOffset, +worldHalfAxisLength + yAxisOffset, +worldHalfAxisLength + zAxisOffset ) ), maxDepth );
	}

	void tSceneGraph::fClear( )
	{
		while( mStandAloneLogic.fCount( ) > 0 )
			mStandAloneLogic.fFront( )->fRemoveStandAloneFromSceneGraph( );

		mRootEntity.fClearChildren( );
		for( u32 i = 0; i < mRunLists.fCount( ); ++i )
			sigassert( mRunLists[ i ].fCount( ) == 0 );
		mDeletionList.fDeleteArray( );
		mSpawnList.fDeleteArray( );
		mElapsedTime = 0.f;
		mPausableTime = 0.f;
		mFrameDeltaTime = 0.f;
		mLogicThreadPool->fClear( );

		fCleanSpatialSet( );

		mPauseNextFrame = false;
		mPaused = false;
	}

	void tSceneGraph::fRemoveUnpreserved( )
	{
		// transfer all entities in the "preserved" list to the preserved root
		sigassert( mPreserveRoot.fChildCount( ) == 0 );
		for( u32 i = 0; i < mPreserveList.fCount( ); ++i )
			mPreserveRoot.fAddChild( *mPreserveList[ i ] );
		mPreserveList.fDeleteArray( );

		while( mStandAloneLogic.fCount( ) > 0 )
			mStandAloneLogic.fFront( )->fRemoveStandAloneFromSceneGraph( );

		mRootEntity.fClearChildren( );
		mDeletionList.fDeleteArray( );
		mSpawnList.fDeleteArray( );
		mPreserveRoot.fTransferChildren( mRootEntity );

		sigassert( mPreserveRoot.fChildCount( ) == 0 );
		sigassert( mPreserveList.fCount( ) == 0 );

		mLogicThreadPool->fClear( );

		fCleanSpatialSet( );
		mPotentialVisibilitySet->fClear( );

		mPauseNextFrame = false;
		mPaused = false;
	}

	void tSceneGraph::fDistributeForLoop( Threads::tDistributedForLoopCallback& cb, u32 loopCount )
	{
		sigassert( !mInMTRunList );
		mLogicThreadPool->fDistributeForLoop( cb, loopCount );
	}

	void tSceneGraph::fWaitForLogicRunListsToComplete( )
	{
		profile_pix( "tSceneGraph::fWaitForLogicRunListsToComplete" );
		profile( cProfilePerfRunListCoRender );
		mLogicThreadPool->fWaitForWorkToComplete( );
		mInMTRunList = false;
		//mLastCoRenderTime = mLogicThreadPool->fLastJobTimeMs( );
		//profile_time( cProfilePerfRunListCoRender, mLastCoRenderTime );
	}

	void tSceneGraph::fDisallowLogicThread( u32 hwThreadId )
	{
		mLogicThreadPool->fDisallowHwThread( hwThreadId );
	}

	void tSceneGraph::fAllowLogicThread( u32 hwThreadId )
	{
		mLogicThreadPool->fAllowHwThread( hwThreadId );
	}

	void tSceneGraph::fAdvanceTime( f32 dt )
	{
		profile_pix("tSceneGraph::fAdvanceTime");

		// stop things from getting out of control if a non-fixed timestep app has a huge stall.
		if( dt > 1.f )
		{
			log_warning( "SceneGraph dt was: " << dt << " seconds" );
			dt = 1.f;
		}
		
		// clear scene graph's debug geometry
		if( !fWillPauseOnNextFrame( ) )
			fDebugGeometry( ).fClear( );

		fUpdatePausedState( );

		mFrameDeltaTime = dt;
		fIncrementTimeCounters( dt );

		fProcessSpawns( );
		fCleanSpatialSet( );
		fProcessDeletions( );

		fProcessRunListST( tLogic::cRunListActST,		cProfilePerfRunListAct, dt );
		fProcessRunListMT( tLogic::cRunListAnimateMT,	cProfilePerfRunListAnimate, dt );
		fProcessRunListMT( tLogic::cRunListCollideMT,	cProfilePerfRunListCollide, dt );
#ifdef target_game
		if( !mPaused ) mPhysics->fStep( dt );
#endif//target_game
		fProcessRunListMT( tLogic::cRunListPhysicsMT,	cProfilePerfRunListPhysics, dt );
		fProcessRunListST( tLogic::cRunListMoveST,		cProfilePerfRunListMove, dt );
		fProcessRunListMT( tLogic::cRunListEffectsMT,	cProfilePerfRunListEffects, dt );
		fProcessRunListST( tLogic::cRunListThinkST,		cProfilePerfRunListThink, dt );
		fProcessRunListST( tLogic::cRunListCameraST,	cProfilePerfRunListCamera, dt );
		fProcessRunListST( tLogic::cRunListPreRenderST, cProfilePerfRunListPreRender, dt );

		if( Debug_SceneGraph_CaptureLCBTDeltaReferenceFrame )
		{
			fCaptureLogicCountByTypeDeltaReferenceFrame( );
			devvar_update_value( Debug_SceneGraph_CaptureLCBTDeltaReferenceFrame, 0.f );
		}

		if( Debug_SceneGraph_DumpLogicCountsByType )
		{
			fLogLogicCountsByType( );
			devvar_update_value( Debug_SceneGraph_DumpLogicCountsByType, 0.f );
		}
	}

	void tSceneGraph::fKickCoRenderMTRunList( )
	{
		fProcessRunListMT( tLogic::cRunListCoRenderMT, cProfilerCategoryNull, mFrameDeltaTime );
	}

	void tSceneGraph::fIncrementTimeCounters( f32 dt )
	{
		// track time
		mFrameDeltaTime = dt;
		mElapsedTime += dt;
		if( !fIsPaused( ) )
			mPausableTime += dt;
	}


	void tSceneGraph::fPauseNextFrame( b32 pause )
	{
		sync_event_v_c( pause, tSync::cSCSceneGraph );
		mPauseNextFrame = pause;
	}

	void tSceneGraph::fPrepareEntityClouds( const Gfx::tCamera & camera )
	{
		profile_pix( "tSceneGraph::fPrepareEntityClouds" );
		const u32 cloudCount = mCloudList.fCount( );
		for( u32 c = 0; c < cloudCount; ++c )
			mCloudList[ c ]->fPrepareRenderables( camera );
	}

	void tSceneGraph::tDefaultRayCastCallback::operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
	{
		if( i->fQuickRejectByFlags( ) )
			return;

		tSpatialEntity* spatial = fCullAgainstIgnoreList( i );
		if( !spatial || spatial->fIsHelper( ) )
			return;

		if( i->fQuickRejectByBox( ray ) )
			return;

		Math::tRayCastHit hit;
		spatial->fRayCast( ray, hit );
		if( hit.fHit( ) && hit.mT < mHit.mT )
		{
			mHit			= hit;
			mFirstEntity	= spatial;
		}
	}

	tSpatialEntity* tSceneGraph::tDefaultRayCastCallback::fCullAgainstIgnoreList( tEntityBVH::tObjectPtr i ) const
	{
		tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i->fOwner( ) );

		// check to ignore entity before performing actual ray cast
		for( u32 iignore = 0; iignore < mNumToIgnore; ++iignore )
		{
			if( spatial == mIgnoreList[ iignore ] )
				return 0;
			if( spatial->fIsAncestorOfMine( *mIgnoreList[ iignore ] ) )
				return 0;
			//if( mIgnoreList[ iignore ]->fIsAncestor( *spatial ) )
			//	return 0;
		}

		return spatial;
	}

	void tSceneGraph::fVisualizeSpatialSetDebug( s32 targetDepth, b32 objectBoxesOnly ) const
	{
		for( u32 i = 0; i < mSpatialSets.fCount( ); ++i )
			mSpatialSets[ i ]->fRenderDebug( fMin<s32>( targetDepth, mSpatialSets[ i ]->fMaxDepth( ) ), objectBoxesOnly, mDebugGeometry );
	}

	namespace
	{
		template<class tVolume>
		struct tRenderDebugSubObject : public tEntityBVH::tIntersectVolumeCallback<tVolume>
		{
			const tSceneGraph* mSG;
			s32 mDepth;

			tRenderDebugSubObject( const tSceneGraph* sg, s32 depth ) 
				: mSG( sg ), mDepth( depth )
			{ }

			inline void operator()( const tVolume& v, tEntityBVH::tObjectPtr i, b32 aabbWhollyContained ) const
			{
				tMeshEntity* test = static_cast< tSpatialEntity* >( i->fOwner( ) )->fDynamicCast<tMeshEntity>( );
				if( test )
				{
					//if( fQuickAabbTest( v, i, aabbWhollyContained ) )
						test->fSubMesh( )->mPolySoupKDTree.fDraw( test->fObjectToWorld( ), mSG->fDebugGeometry( ), mDepth );
				}
			}
			inline void operator()( tEntityBVH::tObjectPtr i ) const
			{
				tMeshEntity* test = static_cast< tSpatialEntity* >( i->fOwner( ) )->fDynamicCast<tMeshEntity>( );
				if( test )
				{
					test->fSubMesh( )->mPolySoupKDTree.fDraw( test->fObjectToWorld( ), mSG->fDebugGeometry( ), mDepth );
				}
			}
		};
	}

	void tSceneGraph::fVisualizeSubObjectSpatialDebug( s32 targetDepth ) const
	{
		fCollect( tRenderDebugSubObject<Math::tAabbf>( this, targetDepth ), Gfx::tRenderableEntity::cSpatialSetIndex );
	}

	void tSceneGraph::fUpdatePausedState( )
	{
		if( mPauseNextFrame == mPaused )
			return;
		mPaused = mPauseNextFrame;

		// notify all entities
		mRootEntity.fOnPause( mPaused );

		// notify all stand alone logics
		for( u32 i = 0; i < mStandAloneLogic.fCount( ); ++i )
			mStandAloneLogic[ i ]->fOnPause( mPaused );
	}

	void tSceneGraph::fSyncSpatialSet( u32 spatialSetIndex ) const
	{
#ifndef spatial_correctness
		tSpatialSet& spatialSet = *mSpatialSets[ spatialSetIndex ];

		for( u32 i = 0; i < spatialSet.mDirtyObjects.fCount( ); ++i )
		{
			spatialSet.mDirtyObjects[ i ]->fResetLazyUpdateListIndex( );
			spatialSet.fMove( spatialSet.mDirtyObjects[ i ]->fToSpatialSetObject( ) );
		}

		if( mRenderCulling && fRenderCullSpatialSet( spatialSetIndex ) )
		{
			for( u32 i = 0; i < spatialSet.mDirtyObjects.fCount( ); ++i )
			{
				spatialSet.mDirtyObjects[ i ]->fResetLazyUpdateListIndex( );
				Gfx::tRenderableEntity* re = static_cast<Gfx::tRenderableEntity*>( spatialSet.mDirtyObjects[ i ] );
				re->fUpdateRenderCulling( *mRenderCulling );
			}
		}

		spatialSet.mDirtyObjects.fSetCount( 0 );
#endif//spatial_correctness
	}

	void tSceneGraph::fSyncAllSpatialSets( ) const
	{
		for( u32 i = 0; i < mSpatialSets.fCount( ); ++i )
			fSyncSpatialSet( i );
	}

	void tSceneGraph::fCleanSpatialSet( )
	{
		profile_pix("tSceneGraph::fCleanSpatialSet");
		for( u32 i = 0; i < mSpatialSets.fCount( ); ++i )
		{
			fSyncSpatialSet( i );
			mSpatialSets[ i ]->fClean( );
		}
	}
	void tSceneGraph::fProcessSpawns( )
	{
		profile_pix("tSceneGraph::fProcessSpawns");
		sync_event_v_c( mSpawnList.fCount( ), tSync::cSCSceneGraph );

		for( u32 i = 0; i < mSpawnList.fCount( ); ++i )
		{
			sigassert( mSpawnList[ i ].mChild );
			sigassert( mSpawnList[ i ].mParent );
			if( mSpawnList[ i ].mParent->fSceneGraph( ) == this )
			{
				mSpawnList[ i ].mChild->mInSpawnList = false;

				if( mSpawnList[ i ].mChild->fSceneGraph( ) == this ) // reparenting
					mSpawnList[ i ].mParent->fAddChild( *mSpawnList[ i ].mChild );
				else
					mSpawnList[ i ].mChild->fSpawnImmediate( *mSpawnList[ i ].mParent );

				if( mSpawnList[ i ].mChild->mDeleteAfterSpawn )
				{
					mSpawnList[ i ].mChild->mDeleteAfterSpawn = false;
					mSpawnList[ i ].mChild->fDelete( );
				}
			}
			else
				log_warning( "An entity tried to spawn to an entity that was removed from the scene graph." << std::endl << " You likely spawned something as a child then deleted the parent with delete immediate." );
		}

		// reset spawn list
		mSpawnList.fSetCount( 0 );
	}

	void tSceneGraph::fProcessDeletions( )
	{
		profile_pix("tSceneGraph::fProcessDeletions");
		// Give the clouds a chance to delete their objects
		for( u32 i = 0; i < mCloudList.fCount( ); ++i )
			mCloudList[ i ]->fCleanRenderables( );

		for( u32 i = 0; i < mDeletionList.fCount( ); ++i )
		{
			if( mDeletionList[ i ]->fReadyForDeletion( ) )
				mDeletionList[ i ]->fDeleteImmediate( );
			else
				mRootEntity.fAddChild( *mDeletionList[ i ] );
		}

		// reset deletion list
		mDeletionList.fSetCount( 0 );

#ifdef sig_logging
		if( Debug_SceneGraph_DumpLogicsMissingResource )
			fLogEntitiesMissingResource( );
#endif

	}

	void tSceneGraph::fInsertStandAloneLogic( tLogic& l )
	{
		sigassert( !mInMTRunList );
		mStandAloneLogic.fFindOrAdd( &l );
	}

	void tSceneGraph::fRemoveStandAloneLogic( tLogic& l )
	{
		sigassert( !mInMTRunList );
		mStandAloneLogic.fFindAndErase( &l );
	}

	void tSceneGraph::fInsert( tEntity& e )
	{
		sigassert( !mInMTRunList );

		// recursively insert all children
		for( u32 i = 0; i < e.fChildCount( ); ++i )
			fInsert( *e.fChild( i ) );

		if( e.fSceneGraph( ) == this )
			return; // already inserted
		else if( e.fSceneGraph( ) )
			e.fSceneGraph( )->fRemove( e ); // in a different scene graph, remove before inserting here

		// set scene graph pointer
		e.fSetSceneGraph( this );
	}

	void tSceneGraph::fRemove( tEntity& e )
	{
		sigassert( !mInMTRunList );

		// recursively remove all children
		for( u32 i = 0; i < e.fChildCount( ); ++i )
			fRemove( *e.fChild( i ) );

		if( e.fSceneGraph( ) != this )
			return; // doesn't exist in this scene graph

		// remove from all run lists
		e.fRemoveFromRunLists( );

		// clear scene graph pointer
		e.fSetSceneGraph( 0 );
	}
	void tSceneGraph::fProcessRunListST( tLogic::tRunListId runListId, u32 profilerId, f32 dt )
	{
		profile_pix( tLogic::fRunListString( runListId ) );
		profile( profilerId );

		const tLogic::tRunList& runList = mRunLists[ runListId ];

		sync_event_v_c( Math::tVec2u( runListId, runList.fCount( ) ), tSync::cSCSceneGraph );

		// output run list size
		if( Debug_SceneGraph_DumpLogicCountsByType )
			log_line( 0, "Run List [" << tLogic::fRunListString( runListId ) << "] has [" << runList.fCount( ) << "] logics." );
		
		// tick all logic objects serializedly
		for( u32 i = 0; i < runList.fCount( ); ++i )
			runList[ i ]->fOnTick( runListId, dt );
	}
	void tSceneGraph::fProcessRunListMT( tLogic::tRunListId runListId, u32 profilerId, f32 dt )
	{
		profile_pix( tLogic::fRunListString( runListId ) );

		b32 runSt = Debug_SceneGraph_ForceRunListsST;
#ifdef target_tools
		runSt = true;
#endif
		if( runSt )
		{
			fProcessRunListST( runListId, profilerId, dt );
			return;
		}

		profile( profilerId );

		const tLogic::tRunList& runList = mRunLists[ runListId ];

		sync_event_v_c( Math::tVec2u( runListId, runList.fCount( ) ), tSync::cSCSceneGraph );

		// output run list size
		if( Debug_SceneGraph_DumpLogicCountsByType )
			log_line( 0, "Run List [" << tLogic::fRunListString( runListId ) << "] has [" << runList.fCount( ) << "] logics." );

		const b32 secondaryThreadsOnly	= ( runListId == tLogic::cRunListCoRenderMT );
		const b32 waitForWorkToComplete	= ( runListId != tLogic::cRunListCoRenderMT );

		// tick logic objects in threads
		mInMTRunList = true;
		mLogicThreadPool->fBeginWork( runListId, dt, runList, secondaryThreadsOnly, waitForWorkToComplete );
		if( waitForWorkToComplete )
			mInMTRunList = false;
	}
	void tSceneGraph::fSpawn( tEntity& e, tEntity& parent  )
	{
		sync_event_v_c( Math::tVec2u( e.fGuid( ), parent.fGuid( ) ), tSync::cSCSceneGraph );

		sigassert( !mInMTRunList );

		const tSpawnData spawnData = tSpawnData( tEntityPtr( &e ), tEntityPtr( &parent ) );

		//sigassert( !e.fSceneGraph( ) );
		sigassert( !parent.fSceneGraph( ) || parent.fSceneGraph( ) == this );
		sigassert( !mSpawnList.fFind( spawnData ) );

		e.mInSpawnList = true;
		mSpawnList.fPushBack( spawnData );
	}

	void tSceneGraph::fDelete(	tEntity& e )
	{
		sync_event_v_c( e.fGuid( ), tSync::cSCSceneGraph );
		sigassert( !mInMTRunList );
		mDeletionList.fFindOrAdd( tEntityPtr( &e ) );
	}

	b32 tSceneGraph::fInDeletionList( const tEntity& e ) const
	{
		return mDeletionList.fFind( &e ) != NULL;
	}

	void tSceneGraph::fPreserve( tEntity& e )
	{
		sigassert( !mInMTRunList );
		mPreserveList.fFindOrAdd( tEntityPtr( &e ) );
	}

	void tSceneGraph::fSpatialMove( tSpatialEntity& e )
	{
		sigassert( !mInMTRunList );

		// N.B., we explicitly DO NOT recursively move all children, as 
		// movement is not necessarily an inherited property; i.e., a parent
		// can move without its children moving... it's up to the derived
		// entity to call this method on its children

		const u32 spatialIndex = e.fSpatialSetIndex( );
		if( spatialIndex == ~0 )
			return;

		tSpatialSet& spatialSet = *mSpatialSets[ spatialIndex ];
		log_assert( !e.fToSpatialSetObject( )->mCellKey.fNull( ), "entities should be inserted into a spatial set before being moved. Most likely this entity stuck around past a level load. Figure that out and you'll probably solve this crash. " << ((void*)&e) );

#ifdef spatial_correctness
		spatialSet.fMove( e.fToSpatialSetObject( ) );
#else//spatial_correctness
		if( e.fLazyUpdateListIndexInvalid( ) )
		{
			sigassert( spatialSet.mDirtyObjects.fCount( ) < 0xFFFF );
			e.fSetLazyUpdateListIndex( spatialSet.mDirtyObjects.fCount( ) );
			spatialSet.mDirtyObjects.fPushBack( &e );
		}
		else
			++mNumDups;
#endif//spatial_correctness
	}

	void tSceneGraph::fSpatialInsert(	tSpatialEntity& e )
	{
		sigassert( !mInMTRunList );

		const u32 spatialIndex = e.fSpatialSetIndex( );
		if( spatialIndex == ~0 )
			return;

		if( spatialIndex >= mSpatialSets.fCount( ) )
		{
			const u32 oldCount = mSpatialSets.fCount( );
			mSpatialSets.fSetCount( spatialIndex + 1 );
			for( u32 i = oldCount; i < mSpatialSets.fCount( ); ++i )
			{
				if( !mSpatialSets[ i ] )
				{
					mSpatialSets[ i ].fReset( NEW tSpatialSet );
					mSpatialSets[ i ]->fCreate( mSpatialSets.fFront( )->fBounds( ), mSpatialSets.fFront( )->fMaxDepth( ) );
				}
			}
		}

#ifndef spatial_correctness
		sigassert( !mSpatialSets[ spatialIndex ]->mDirtyObjects.fFind( &e ) );
		e.fResetLazyUpdateListIndex( );
#endif//spatial_correctness

		mSpatialSets[ spatialIndex ]->fInsert( e.fToSpatialSetObject( ) );

		if( mRenderCulling && fRenderCullSpatialSet( spatialIndex ) )
		{
			Gfx::tRenderableEntity& re = static_cast<Gfx::tRenderableEntity&>( e );
			re.fAddToRenderCulling( *mRenderCulling );
		}
	}

	void tSceneGraph::fSpatialRemove( tSpatialEntity& e )
	{
		sigassert( !mInMTRunList );

		const u32 spatialIndex = e.fSpatialSetIndex( );
		if( spatialIndex == ~0 )
			return;

		tSpatialSet& spatialSet = *mSpatialSets[ spatialIndex ];

#ifndef spatial_correctness
		if( !e.fLazyUpdateListIndexInvalid( ) )
		{
			spatialSet.mDirtyObjects.fErase( e.fLazyUpdateListIndex( ) );
			if( e.fLazyUpdateListIndex( ) < spatialSet.mDirtyObjects.fCount( ) )
				spatialSet.mDirtyObjects[ e.fLazyUpdateListIndex( ) ]->fSetLazyUpdateListIndex( e.fLazyUpdateListIndex( ) );
			e.fResetLazyUpdateListIndex( );
		}
#endif//spatial_correctness

		spatialSet.fRemove( e.fToSpatialSetObject( ) );

		if( mRenderCulling && fRenderCullSpatialSet( spatialIndex ) )
		{
			Gfx::tRenderableEntity& re = static_cast<Gfx::tRenderableEntity&>( e );
			re.fRemoveFromRenderCulling( *mRenderCulling );
		}
	}

	//------------------------------------------------------------------------------
	void tSceneGraph::fCloudInsert( tEntityCloud & e )
	{
		sigassert( !mCloudList.fFind( &e ) );
		mCloudList.fPushBack( &e );
	}

	//------------------------------------------------------------------------------
	void tSceneGraph::fCloudRemove( tEntityCloud & e )
	{
		b32 found = mCloudList.fFindAndErase( &e );
		sigassert( found );
	}

	//------------------------------------------------------------------------------
	b32 tSceneGraph::fInSpawnList( tEntity* e )
	{
		for(u32 i = 0; i < mSpawnList.fCount( ); ++i)
		{
			if( mSpawnList[ i ].mChild == e || mSpawnList[ i ].mParent == e )
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------
	void tSceneGraph::fInitGCInstancing( )
	{
		for( u32 i = 0; i < mCloudList.fCount( ); ++i )
			mCloudList[ i ]->fInitGCInstancing( );
	}

}

//--------------------------------------------------------------------------------------------------------------
//
//    Script-Specific Implementation
//
//--------------------------------------------------------------------------------------------------------------

namespace Sig
{
	void tSceneGraph::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tSceneGraph, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("GameTime"),		&tSceneGraph::fPausableTime)
			.Prop(_SC("Physics"),		&tSceneGraph::fPhysics)
			;

		vm.fRootTable( ).Bind( _SC("SceneGraph"), classDesc );
	}

}
