#include "GameAppPch.hpp"
#include "tGeneratorLogic.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tCharacterLogic.hpp"
#include "tVerreyLightLogic.hpp"
#include "tWaveLaunchArrowUI.hpp"
#include "tDropLogic.hpp"
#include "tSync.hpp"

namespace Sig
{
	namespace
	{
		enum tStartPoints
		{
			cStartPointExitGen = 0,
			cStartPointTrench,
			cStartPointGoalPath,
			cStartPointCount
		};

		static const tStringPtr cSpawnName( "spawnPoint" );
		static const tStringPtr cVerreyName( "verreyPoint" );
	}



	tGeneratorLogic::tGeneratorLogic( ) 
		: mActive( true )
		, mOpenDoors( false )
		, mReadying( false )
		, mWasReadying( false )
		, mSpawnPoint( NULL )
		, mVeryLightPt( NULL )
		, mRefreshActiveUnitTimer( 0.5f )
	{
	}

	tGeneratorLogic::~tGeneratorLogic( )
	{
	}
	void tGeneratorLogic::fOnSpawn( )
	{
		fOnPause( false );

		tAnimatedBreakableLogic::fOnSpawn( );

		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			tEntity* ent = fOwnerEntity( )->fChild( i ).fGetRawPtr( );
			if( ent->fName( ) == cSpawnName )
				mSpawnPoint = ent;
			else if( ent->fName( ) == cVerreyName )
				mVeryLightPt = ent;
		}

		if( !mVeryLightPt )
			mVeryLightPt = fOwnerEntity( );

		if( !mSpawnPoint )
			mSpawnPoint = fOwnerEntity( );

		fSetupProximity( );

		fAddHealthBar( );
		fAddToMiniMap( );
	}
	void tGeneratorLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListAnimateMT );
			fRunListRemove( cRunListMoveST );
		}
		else
		{
			fRunListInsert( cRunListAnimateMT );
			fRunListInsert( cRunListMoveST );
		}
	}
	void tGeneratorLogic::fOnDelete( )
	{
		mProximity.fReleaseAllEntityRefsST( );
		mWaves.fSetCount( 0 );
		mParentGenerator.fRelease( );
		tAnimatedBreakableLogic::fOnDelete( );
	}
	void tGeneratorLogic::fMoveST( f32 dt )
	{
		tAnimatedBreakableLogic::fMoveST( dt );

		// see how many guys are still out there
		mRefreshActiveUnitTimer -= dt;
		if( mRefreshActiveUnitTimer <= 0 )
		{
			mRefreshActiveUnitTimer = 0.5f;
			fRefreshActiveUnitCounts( );
		}

		Math::tMat3f spawnXform = mSpawnPoint->fObjectToWorld( );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level && level->fWaypointsProcessed( ) )
		{
			for( u32 i = 0; i < mWaves.fCount( ); ++i )
			{
				mWaves[ i ]->fStep( dt, spawnXform );

				if( mWaves[ i ]->fDone( ) )
				{
					//give em one last launch for good measure :)
					mWaves[ i ]->fLaunch( );
					mWaves.fEraseOrdered( i );
					--i;
				}
			}
		}

		mProximity.fCleanST( );

		// TODO move to CoRenderMT
		mProximity.fRefreshMT( dt, fSceneGraph( )->fRootEntity( ) );

		fStepDoors( );
	}

	void tGeneratorLogic::fStepDoors( )
	{
		b32 shouldReady = mReadying || mReadyRequests.fCount( );
		b32 shouldOpen = !fSpawnedEnough( ) || mOpenRequests.fCount( );
		if( shouldOpen != mOpenDoors )
		{
			mOpenDoors = shouldOpen;
			if( mParentGenerator )
				fParentGenerator( )->fRequestOpen( fOwnerEntity( ), mOpenDoors );

			fHandleLogicEvent( Logic::tEvent::fConstructDefault( GameFlags::cEVENT_REAPPLY_ONESHOT_MOTION_STATE ) );
		}

		if( shouldReady != mWasReadying )
		{
			mWasReadying = shouldReady;
			if( mParentGenerator )
				fParentGenerator( )->fRequestReady( fOwnerEntity( ), shouldReady );

			if( !mOpenDoors )
				fHandleLogicEvent( Logic::tEvent::fConstructDefault( GameFlags::cEVENT_REAPPLY_MOTION_STATE ) );
		}

	}

	void tGeneratorLogic::fRequestOpen( tEntity* from, b32 open )
	{
		if( open )
		{
			sigassert( mOpenRequests.fIndexOf( from ) == ~0 );
			mOpenRequests.fPushBack( from );
		}
		else
		{
			b32 found = mOpenRequests.fFindAndErase( from );
			sigassert( found );
		}
		fStepDoors( );
	}

	void tGeneratorLogic::fRequestReady( tEntity* from, b32 ready )
	{
		if( ready )
		{
			sigassert( mReadyRequests.fIndexOf( from ) == ~0 );
			mReadyRequests.fPushBack( from );
		}
		else
		{
			b32 found = mReadyRequests.fFindAndErase( from );
			sigassert( found );
		}
		fStepDoors( );
	}

	tGeneratorWave* tGeneratorLogic::fStartGenerating( tWaveDesc& desc, u32 spawnCount )
	{
		mReadying = false;

		if( !mActive )
		{
			log_warning( 0, "Skipping generating for generator ( " << fOwnerEntity( )->fName( ) << " ) because generator is inactive." );
			return NULL;
		}

		if( fIsDestroyed( ) )
		{
			log_warning( 0, "Generator is destroyed" );
			return NULL;
		}

		if( desc.mUnitDescs.fCount( ) == 0 )
		{
			log_warning( 0, "fStartGenerating being called on generator ( " << fOwnerEntity( )->fName( ) << " ) with no units added to tUnitDesc array." );
			return NULL;
		}
		
		tGeneratorWave* newWave = NEW tGeneratorWave( &desc, spawnCount, this );
		mWaves.fPushBack( tWavePtr( newWave ) );

		return newWave;
	}

	void tGeneratorLogic::fStopGenerating( )
	{ 
		mReadying = false;

		for( u32 i = 0; i < mWaves.fCount( ); ++i )
			if( !mWaves[ i ]->fSpawnedEnough( ) )
				log_warning( 0, mWaves[ i ]->fPathName( ) << " wave had more units to spawn when told to stop generating." );

		mWaves.fSetCount( 0 );
	}

	b32 tGeneratorLogic::fSpawnedEnough( ) const 
	{
		if( fIsDestroyed( ) ) 
			return true;

		for( u32 i = 0; i < mWaves.fCount( ); ++i )
			if( !mWaves[ i ]->fSpawnedEnough( ) )
				return false;

		return true;
	}

	void tGeneratorLogic::fKillEveryone( )
	{
		for( u32 i = 0; i < mWaves.fCount( ); ++i )
			mWaves[ i ]->fKillEveryone( );
	}

	void tGeneratorLogic::fReleaseTrenched( )
	{
		for( u32 i = 0; i < mWaves.fCount( ); ++i )
			mWaves[ i ]->fReleaseTrenched( );
	}

	
	void tGeneratorLogic::fRefreshActiveUnitCounts( )
	{
		for( u32 i = 0; i < mWaves.fCount( ); ++i )
			mWaves[ i ]->fRefreshActiveUnitCounts( );
	}

	

	void tGeneratorLogic::fLaunch( tWaveDesc& desc, u32 spawnCount )
	{
		mReadying = false;

		tGeneratorWave* wave = NULL;

		// See if we've already started generating this (trenching)
		for( u32 i = 0; i < mWaves.fCount( ); ++i )
		{
			if( mWaves[ i ]->fEquals( desc ) && !mWaves[ i ]->fLaunched( ) )
			{
				log_line( Log::cFlagWaveList, " WaveList: Already trenched, skipping generation for whole group: " << fOwnerEntity( )->fName( ) << " path: " << mWaves[ i ]->fPathName( ) );

				wave = mWaves[ i ].fGetRawPtr( );
			}
		}

		if( !wave )
			wave = fStartGenerating( desc, spawnCount );

		if( wave )
			wave->fLaunch( );

		if( !fIsDestroyed( ) && mActive )
		{
			if( fSpawnLightAndSound( ) )
				tVerreyLightLogic::fSpawn( fVerryLightPoint( )->fObjectToWorld( ).fGetTranslation( ) );
		}
	}

	namespace{ static const tStringPtr cSpawnProximity("spawnProximity"); }
	void tGeneratorLogic::fSetupProximity( )
	{		
		tDynamicArray<u32> spatialSetIndices;
		Gfx::tRenderableEntity::fAddRenderableSpatialSetIndices( spatialSetIndices );
		mProximity.fSetSpatialSetIndices( spatialSetIndices );
		
		mProximity.fSetRefreshFrequency( 0.5f, 0.01f );
		mProximity.fSetFilterByLogicEnts( true );
		mProximity.fSetQueryInheritedProperties( true );
		mProximity.fLogicFilter( ).fAddProperty( tEntityEnumProperty( GameFlags::cENUM_UNIT_TYPE, GameFlags::cUNIT_TYPE_VEHICLE ) );

		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			tShapeEntity* shape = fOwnerEntity( )->fChild( i )->fDynamicCast< tShapeEntity >( );
			if( shape && shape->fName( ) == cSpawnProximity )
			{				
				switch( shape->fShapeType( ) )
				{
				case tShapeEntityDef::cShapeTypeSphere:
					mProximity.fAddSphere( shape->fSphere( ) );
					break;
				case tShapeEntityDef::cShapeTypeBox:
					mProximity.fAddAabb( Math::tAabbf( shape->fBox( ) ) );
					break;
				}
			}
		}
	}

	void tGeneratorLogic::fFindAndLinkParent( tGrowableArray<tEntityPtr>& gens )
	{
		for( u32 i = 0; i < gens.fCount( ); ++i )
		{
			if( gens[ i ] == fOwnerEntity( ) )
				continue;

			Math::tObbf shape( gens[ i ]->fCombinedObjectSpaceBox( ), gens[ i ]->fObjectToWorld( ) );
			if( shape.fContains( fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) ) )
			{
				mParentGenerator = gens[ i ];
				break;
			}
		}
	}

}

namespace Sig
{
	void tGeneratorLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tGeneratorLogic, tUnitLogic, Sqrat::NoCopy<tGeneratorLogic> > classDesc( vm.fSq( ) );

		classDesc
			.Var(_SC("DropSmokeSigml"),		&tGeneratorLogic::mDropSmokeSigml)
			.Prop(_SC("OpenDoors"),			&tGeneratorLogic::fOpenDoors)
			.Prop(_SC("Readying"),			&tGeneratorLogic::fReadying)
			;

		vm.fRootTable( ).Bind(_SC("GeneratorLogic"), classDesc);
	}
}

