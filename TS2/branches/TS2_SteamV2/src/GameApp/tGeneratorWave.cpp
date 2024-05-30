#include "GameAppPch.hpp"
#include "tGeneratorWave.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tCharacterLogic.hpp"
#include "tUnitPath.hpp"
#include "tDropLogic.hpp"
#include "tSync.hpp"
#include "tWaveLaunchArrowUI.hpp"

namespace Sig
{
	namespace
	{
		static const tStringPtr cAudioSmokeDrop( "Play_SmokeSignal" );
		static const tStringPtr cAudioSmokeLand( "" );
		static const tStringPtr cAudioSmokeDelete( "Stop_SmokeSignal" );
		devvar( f32, Gameplay_Gui_WaveLaunchArrow_Duration, 8.0f );
	}


	tGeneratorWave::tGeneratorWave( tWaveDesc* desc, u32 spawnCount, tGeneratorLogic* generator )
		: mDesc( desc )
		, mUnits( desc->mUnitDescs )
		, mPathName( desc->mPathName )
		, mCurrentUnitIndex( 0 )
		, mCurrentSpawnCount( 0 )
		, mCurrentLaunchCount( 0 )
		, mTotalSpawnCount( spawnCount )
		, mSpawnInterval( desc->mSpawnInterval )
		, mSpawnTimer( 0 )
		, mGenerator( generator )
		, mGenerating( true )
		, mLaunched( false )
		, mKilled( false )
		, mPrompted( false )
		, mKillingAll( false )
		, mSpawnedArrow( false )
		, mValue( 0.f )
	{
		mKillsPerPlayer.fFill( 0 );
		sigassert( mDesc && mDesc->mList );
	}

	b32 tGeneratorWave::fDisableAIFire( ) const
	{
		sigassert( mDesc );
		sigassert( mDesc->mList );
		return mDesc->mList->fDisableAIFire( );
	}

	void tGeneratorWave::fSpawnUnit( const Math::tMat3f& spawnXform )
	{
		sigassert( mGenerator );

		if( mCurrentSpawnCount >= mTotalSpawnCount )
		{
			log_warning( 0, "too many spawned: " << mCurrentSpawnCount << " " << mTotalSpawnCount );
			return;
		}

		// Someone's in my way!
		if( mGenerator->fProximityBlocked( ) )
			return;


		// Actual spawning
		const tWaveList& list = *mDesc->mList;
		const tUnitDesc& spawnDesc = mUnits[ mCurrentUnitIndex ];
		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
		tFilePathPtr unitPath = tGameApp::fInstance( ).fUnitResourcePath( spawnDesc.mUnitID, mGenerator->fCountry( ) );
		tEntity* unit = levelLogic->fOwnerEntity( )->fSpawnChild( unitPath );
		log_assert( unit, "Could not spawn unit: " << GameFlags::fUNIT_IDEnumToValueString( spawnDesc.mUnitID ) << " path: " << unitPath );

		tUnitLogic* unitLogic = unit->fLogicDerived< tUnitLogic >( );
		log_assert( unitLogic, "No logic attached: " << unitPath );
		
		unitLogic->fQueryEnums( );
		if( list.fMakeSelectableUnits( ) )
		{
			unitLogic->fEnableSelection( false );
			unit->fAddGameTags( GameFlags::cFLAG_SELECTABLE );
		}

		unitLogic->fSetCreationType( tUnitLogic::cCreationTypeFromGenerator );
		unitLogic->fSetHitPointModifier( spawnDesc.mHealthModifier );
		unitLogic->fSetTimeScale( spawnDesc.mTimeMultiplier );
		unitLogic->fSetWave( this );
		unit->fSetName( spawnDesc.mName );

		sync_event_v( spawnDesc.mTimeMultiplier );

		if( list.fAssignPickup( ) != ~0 )
			unitLogic->fSetPickup( list.fAssignPickup( ) );

		log_assert( !spawnXform.fIsNan( ), "Nan transform!" );
		unit->fMoveTo( spawnXform );

		// Slotting
		tSpawnedUnitsPtr* spawnedUnit = fFindOrAddSpawnedUnits( spawnDesc.mUIUnitID, mTotalSpawnCount );
		sigassert( spawnedUnit );
		tSpawnedUnits& units = *(*spawnedUnit);

		u32 slot = 0;
		if( (*spawnedUnit)->mSlots.fCount( ) > 0 )
		{
			//choose random place to stand
			u32 index = sync_rand( fIntInRange( 0, units.mSlots.fCount( ) -1 ) );
			slot = units.mSlots[ index ];
			units.mSlots.fErase( index );
		}

		if( mLaunched )
		{
			unitLogic->fAddToAliveList( );
			mValue += unitLogic->fUnitAttributeDestroyValue( );
			++mCurrentLaunchCount;
			units.mLaunchedUnits.fPushBack( tEntityPtr( unit ) );
			if( !mSpawnedArrow )
				fSpawnArrow( unitLogic );
		}
		else
		{
			// in minigames, where we want to see ally arrows. we want it right away rather than after they trench
			if( !mSpawnedArrow && tGameApp::fInstance( ).fCurrentLevelDemand( )->fAllyLaunchArrows( ) )
				fSpawnArrow( unitLogic );
			units.mSpawningUnits.fPushBack( tSpawnPair( tEntityPtr( unit ), slot ) );
		}


		// Pathing
		log_assert( mTotalSpawnCount != 0, "Zero spawn count but spawning!" );
		f32 slotPosition = slot / (f32)mTotalSpawnCount;
		fConfigureUnitPath( unitLogic, spawnXform.fGetTranslation( ), slotPosition, !mLaunched, mPathName );

		++mCurrentSpawnCount;

		if( ++mCurrentUnitIndex >= mUnits.fCount( ) )
		{
			mCurrentUnitIndex = 0;
			mSpawnTimer = mSpawnInterval + mUnits.fBack( ).mSpawnInterval; // Remove this last part if we want to ignore the last interval in a group
		}
		else
			mSpawnTimer = mUnits[ mCurrentUnitIndex - 1 ].mSpawnInterval;
	}

	void tGeneratorWave::fSpawnArrow( tUnitLogic* logic )
	{
		mSpawnedArrow = true;

		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) 
			&& !tGameApp::fInstance( ).fCurrentLevelDemand( )->fDisableLaunchArrows( ) )
		{
			const tDynamicArray<tPlayerPtr>& players = tGameApp::fInstance( ).fPlayers( );
			for( u32 i = 0; i < players.fCount( ); ++i )
			{
				b32 show = mGenerator->fTeam( ) == players[ i ]->fEnemyTeam( );

				if( tGameApp::fInstance( ).fCurrentLevelDemand( )->fAllyLaunchArrows( ) )
					show = mGenerator->fTeam( ) == players[ i ]->fTeam( );

				if( show )
				{
					sigassert( players[ i ]->fUser( ) );
					Gui::tWaveLaunchArrowUI* arrow = NEW Gui::tWaveLaunchArrowUI( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptWaveLaunchArrowUI ), players[ i ]->fUser( ), Gameplay_Gui_WaveLaunchArrow_Duration );
					arrow->fSpawn( *logic->fOwnerEntity( ) );
					arrow->fSetParentRelativeXform( Math::tMat3f::cIdentity );
				}
			}
		}
	}

	void tGeneratorWave::fConfigureUnitPath( tUnitLogic* logic, const Math::tVec3f& startPt, f32 slotPosition, b32 wait, const tStringPtr& pathName )
	{
		tUnitPath* up = logic->fUnitPath( );

		if( up )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			u32 ignoreAny = ~0;

			// The owner doesn't get set until fOnSpawn, so we just set it now
			up->fSetOwnerEntity( logic->fOwnerEntity( ) );

			// find all our paths
			Math::tVec3f trenchPos = startPt;

			s32 exitGen = tUnitPath::fFindClosestStartPoint( startPt, level->fExitGenStarts( ), pathName, &ignoreAny );
			if( exitGen > -1 ) up->fAddPathStartEntry( tUnitPath::tPathStartEntry( level->fExitGenStarts( )[ exitGen ] ) );

			if( wait )
			{
				// totally skip trench path if we dont want to trench

				s32 trenchStart = tUnitPath::fFindClosestStartPoint( startPt, level->fTrenchStarts( ), pathName, &ignoreAny );
				if( trenchStart > -1 ) 
				{
					f32 trenchLen = level->fTrenchLength( trenchStart );
					const tPathEntityPtr& path = level->fTrenchStarts( )[ trenchStart ];
					path->fTraversePath( trenchLen * slotPosition, trenchPos );

					up->fAddPathStartEntry( tUnitPath::tPathStartEntry( path, trenchLen * slotPosition, wait ) );
				}
			}

			b32 addGoal = true;
			if( logic->fUnitType( ) == GameFlags::cUNIT_TYPE_AIR
				&& (tUnitPath::fFindClosestStartPoint( trenchPos, level->fFlightPathStarts( ), pathName ) != -1) )
				addGoal = false; //we will be navigating around for a bit first.

			if( addGoal )
			{
				s32 pathStart = tUnitPath::fFindClosestStartPoint( trenchPos, level->fPathStarts( ), pathName, &ignoreAny );
				if( pathStart > -1 )
				{
					if( !level->fPathEndsInGoal( )[ pathStart ] )
						logic->fSetWillNotEndInGoal( );
					up->fAddPathStartEntry( tUnitPath::tPathStartEntry( level->fPathStarts( )[ pathStart ] ) );

					if( level->fPathHasDropPoints( )[ pathStart ]
						&& logic->fUnitType( ) != GameFlags::cUNIT_TYPE_INFANTRY
						&& logic->fUnitType( ) != GameFlags::cUNIT_TYPE_BOSS
						&& mGenerator->fSmokeSigml( ).fExists( ) )
						fDropSmoke( logic, level->fPathStarts( )[ pathStart ].fGetRawPtr( ) );
				}
				else
					log_warning( 0, "No goal path found with name " << pathName );
			}

			up->fStartPathSequence( );
		}
	}

	void tGeneratorWave::fDropSmoke( tUnitLogic* logic, const tPathEntity* pathStart )
	{
		if( pathStart->fHasGameTagsAny( GameFlags::cFLAG_DROP_CARGO ) )
		{
			if( !mDroppedSmokes.fFind( pathStart ) )
			{
				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				if( level )
				{
					tEntity* ent = level->fRootEntity( )->fSpawnChild( mGenerator->fSmokeSigml( ) );
					if( ent )
					{
						tDropLogic* drop = ent->fLogicDerived<tDropLogic>( );
						if( drop )
						{
							drop->mSpawnAudioEvent = cAudioSmokeDrop;
							drop->mDeleteAudioEvent = cAudioSmokeDelete;
							drop->mLandAudioEvent = cAudioSmokeLand;
						}

						mDroppedSmokes.fPushBack( pathStart );
						mSmokePtrs.fPushBack( tSmokeDestroyerPtr( NEW tSmokeDestroyer( ent, pathStart ) ) );
						logic->fAddCargoDropSmokePtr( mSmokePtrs.fBack( ) );
						ent->fMoveTo( pathStart->fObjectToWorld( ).fGetTranslation( ) );
					}
				}
			}
		}

		for( u32 i = 0; i < pathStart->fNextPointCount( ); ++i )
			fDropSmoke( logic, pathStart->fNextPoint( i ) );
	}

	namespace
	{
		b32 fUnitDead( tEntityPtr& p )
		{
			tUnitLogic* unit = p->fLogicDerived< tUnitLogic >( );

			return !unit || unit->fIsDestroyed( ) || !p->fSceneGraph( );
		}	
	}

	void tGeneratorWave::fPrompt( tUnitLogic* unit )
	{
		if( !mPrompted )
		{
			mPrompted = true;
			unit->fShowFocalPrompt( unit->fUnitAttributeFocalPrompt( ) );
		}
	}

	void tGeneratorWave::fStep( f32 dt, const Math::tMat3f& spawnXform )
	{
		if( mGenerating )
		{
			mSpawnTimer -= dt;

			b32 spawnedEnough = fSpawnedEnough( );
			if( !spawnedEnough && mSpawnTimer <= 0 )
				fSpawnUnit( spawnXform );
		}
	}

	void tGeneratorWave::fRefreshActiveUnitCounts( )
	{
		for( tSpawnedUnitsMap::tIterator spawnedUnits = mSpawnedUnits.fBegin( ); spawnedUnits != mSpawnedUnits.fEnd( ); ++spawnedUnits )
		{
			if( spawnedUnits->fNullOrRemoved( ) ) continue;

			tSpawnedUnits& units = *spawnedUnits->mValue;

			//update spawning guys (trenches ), if they die, recreate their slot
			for( s32 i = units.mSpawningUnits.fCount( ) - 1; i >= 0; --i )
			{
				if( fUnitDead( units.mSpawningUnits[ i ].mEnt ) )
				{
					sigassert( !mLaunched );
					units.mSlots.fPushBack( units.mSpawningUnits[ i ].mSlot );
					units.mSpawningUnits.fErase( i );
					if( mCurrentSpawnCount > 0 ) 
						--mCurrentSpawnCount;
				}
			}

			// Remove any dead launched guys
			for( s32 i = units.mLaunchedUnits.fCount( ) - 1; i >= 0; --i )
			{
				if( fUnitDead( units.mLaunchedUnits[i] ) )
				{
					if( mCurrentLaunchCount > 0 )
					{
						--mCurrentLaunchCount;
						if( !mKilled && mCurrentLaunchCount == 0 && fSpawnedEnough( ) )
							mKilled = true;
					}
					units.mLaunchedUnits.fErase( i );
				}
			}
		}

	}


	void tGeneratorWave::fEnemyKilled( tUnitLogic* logic, tPlayer* player )
	{
		if( !mKillingAll )
		{
			tSpawnedUnitsPtr* plist = mSpawnedUnits.fFind( logic->fUnitIDAlias( ) );
			if( !plist )
			{
				log_warning( 0, "I'm not sure how this happened! tGeneratorWave::fEnemyKilled" );
				return;
			}

			sigassert( plist );
			tSpawnedUnits* list = plist->fGetRawPtr( );

			if( !mLaunched )
			{
				if( mCurrentSpawnCount > 0 )
					--mCurrentSpawnCount;
				else
					log_warning( 0, GameFlags::fUNIT_IDEnumToValueString( logic->fUnitIDAlias( ) ) << " zero spawn count." );


				u32 index = list->mSpawningUnits.fIndexOf( logic->fOwnerEntity( ) );
				if( index != ~0 )
				{
					list->mSlots.fPushBack( list->mSpawningUnits[ index ].mSlot );
					list->mSpawningUnits.fErase( index );
				}
				else
					log_warning( 0, GameFlags::fUNIT_IDEnumToValueString( logic->fUnitIDAlias( ) ) << " not found in spawned list." );

			}
			else
			{
				if( mCurrentLaunchCount > 0 )
					--mCurrentLaunchCount;
				else
					log_warning( 0, GameFlags::fUNIT_IDEnumToValueString( logic->fUnitIDAlias( ) ) << " zero launch count." );


				u32 index = list->mLaunchedUnits.fIndexOf( logic->fOwnerEntity( ) );
				if( index != ~0 )
				{
					list->mLaunchedUnits.fErase( index );
				}
				else
					log_warning( 0, GameFlags::fUNIT_IDEnumToValueString( logic->fUnitIDAlias( ) ) << " not found in launched list." );

				if( player && mGenerator->fTeam( ) != player->fTeam( ) )
					++mKillsPerPlayer[ player->fPlayerIndex( ) ];

				if( !mKilled && mCurrentLaunchCount == 0 && fSpawnedEnough( ) )
				{
					mKilled = true;

					tPlayer* killedWholeWave = NULL;
					if( player && mKillsPerPlayer[ player->fPlayerIndex( ) ] == mCurrentSpawnCount )
					{
						killedWholeWave = player;
						killedWholeWave->fIncrementWaveChain( logic->fOwnerEntity( ), mValue );
					}

					// inform those who didnt
					const tDynamicArray<tPlayerPtr>& players = tGameApp::fInstance( ).fPlayers( );
					for( u32 i = 0; i < players.fCount( ); ++i )
					{
						if( players[ i ] != killedWholeWave && players[ i ]->fTeam( ) != mGenerator->fTeam( ) )
							players[ i ]->fResetWaveChain( );
					}

					tGameApp::fInstance( ).fCurrentLevelDemand( )->fOnWaveKilled( mDesc.fGetRawPtr( ) );
				}			
			}
		}
	}

	void tGeneratorWave::fAddCargoUnit( tUnitLogic* unit )
	{
		mValue += unit->fUnitAttributeDestroyValue( );
		++mCurrentLaunchCount;

		tSpawnedUnitsPtr* plist = mSpawnedUnits.fFind( unit->fUnitID( ) );
		if( !plist )
			plist = mSpawnedUnits.fInsert( unit->fUnitID( ), tSpawnedUnitsPtr( NEW tSpawnedUnits( 0 ) ) );

		(*plist)->mLaunchedUnits.fPushBack( tEntityPtr( unit->fOwnerEntity( ) ) );
	}


	void tGeneratorWave::fAddClosestPathStart( tUnitLogic& unitLogic )
	{
		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
		if( !levelLogic )
			return;

		// Use the units current position as the point to search from.
		const Math::tVec3f startPt = unitLogic.fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		u32 ignoreAny = ~0;
		tUnitPath* unitPath = unitLogic.fUnitPath( );
		// Find the closest path start from here.
		const s32 goal = unitPath->fFindClosestStartPoint( startPt, levelLogic->fPathStarts( ), unitPath->fLastStartedPathName( ), &ignoreAny );
		if( goal > -1 ) 
		{
			// Check if the path ends in goal and mark accordingly.
			if( !levelLogic->fPathEndsInGoal( )[ goal] )
				unitLogic.fSetWillNotEndInGoal( );
			unitPath->fAddPathStartEntry( tUnitPath::tPathStartEntry( levelLogic->fPathStarts( )[ goal ] ) );
		}
	}

	void tGeneratorWave::fLaunch( )
	{
		for( tSpawnedUnitsMap::tIterator spawnedUnits = mSpawnedUnits.fBegin( ); spawnedUnits != mSpawnedUnits.fEnd( ); ++spawnedUnits )
		{
			if( spawnedUnits->fNullOrRemoved( ) ) 
				continue;

			tSpawnedUnits& units = *spawnedUnits->mValue;

			for( u32 i = 0; i < units.mSpawningUnits.fCount( ); ++i )
			{
				tUnitLogic* logic = units.mSpawningUnits[ i ].mEnt->fLogicDerived< tUnitLogic >( );
				tUnitPath* path = logic->fUnitPath( );
				if( path )
				{
					// Throw away any path start we had before.  That was chosen when we spawned, but we could have
					// trenched and be in a different location now.
					path->fClearPath( );
					path->fClearPathStarts( );
					// Find a closest path start at this point.
					fAddClosestPathStart( *logic );
				}

				logic->fSetFirstWaveLaunch( true );
				units.mLaunchedUnits.fPushBack( tEntityPtr( logic->fOwnerEntity( ) ) );

				if( path )
				{
					// Since we just cleared our path, we need to start and not resume from here.
					path->fStartPathSequence( );
				}

				logic->fAddToAliveList( );
				mValue += logic->fUnitAttributeDestroyValue( );
				++mCurrentLaunchCount;

				if( !mSpawnedArrow )
					fSpawnArrow( logic );
			}

			units.mSpawningUnits.fSetCount( 0 );
		}

		mLaunched = true;
	}

	void tGeneratorWave::fKillEveryone( )
	{
		mKillingAll = true;
		mLaunched = true;
		mKilled = true;
		mCurrentSpawnCount = mTotalSpawnCount;

		for( tSpawnedUnitsMap::tIterator spawnedUnits = mSpawnedUnits.fBegin( ); spawnedUnits != mSpawnedUnits.fEnd( ); ++spawnedUnits )
		{
			if( spawnedUnits->fNullOrRemoved( ) ) 
				continue;

			tSpawnedUnits& units = *spawnedUnits->mValue;

			for( u32 i = 0; i < units.mSpawningUnits.fCount( ); ++i )
			{
				tUnitLogic* logic = units.mSpawningUnits[ i ].mEnt->fLogicDerived< tUnitLogic >( );
				logic->fDestroyDontJib( );
			}
			for( u32 i = 0; i < units.mLaunchedUnits.fCount( ); ++i )
			{
				tUnitLogic* logic = units.mLaunchedUnits[ i ]->fLogicDerived< tUnitLogic >( );
				logic->fDestroyDontJib( );
			}
			units.mSpawningUnits.fSetCount( 0 );
			units.mLaunchedUnits.fSetCount( 0 );
		}
	}

	void tGeneratorWave::fReleaseTrenched( )
	{
		fLaunch( );
	}


	
	tSpawnedUnitsPtr* tGeneratorWave::fFindOrAddSpawnedUnits( u32 unitID, u32 slotCount )
	{
		tSpawnedUnitsPtr* spawnedUnit = mSpawnedUnits.fFind( unitID );
		if( !spawnedUnit )
			spawnedUnit = mSpawnedUnits.fInsert( unitID, tSpawnedUnitsPtr( NEW tSpawnedUnits( slotCount ) ) );

		return spawnedUnit;
	}
}

