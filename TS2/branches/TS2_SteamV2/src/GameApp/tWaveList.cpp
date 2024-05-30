#include "GameAppPch.hpp"
#include "tWaveList.hpp"
#include "tGeneratorLogic.hpp"
#include "tLevelLogic.hpp"
#include "tSync.hpp"

#include "Wwise_IDs.h"

namespace Sig
{
	namespace
	{
		const tStringPtr cWaveTypeSwitch( "Wave_Type" );

		b32 fConsiderUIWave( tWaveDesc* desc )
		{
			return !desc->fConsiderPreviousWave( ) && !desc->mFunctionCallName.fExists( );
		}
	}

	devvar( f32, Gameplay_Survival_TimeAdd, 0.05f );
	devvar( f32, Gameplay_Survival_TimeMax, 3.0f );
	devvar( f32, Gameplay_Survival_HealthAdd, 0.3f );
	devvar( f32, Gameplay_Survival_HealthMax, 4.0f );
	devvar( bool, Gameplay_Survival_BonusEveryTime, false );


	tWaveDesc::tWaveDesc( tWaveList* list, u32 count, f32 readyTime, f32 countdown, f32 spawnInterval, f32 launchTime, u32 waveID )
		: mList( list )
		, mCount( count )
		, mReadyTime( readyTime )
		, mCountdown( countdown )
		, mSpawnInterval( spawnInterval )
		, mTotalLaunchTime( launchTime )
		, mWaveID( waveID )
		, mPopWhenFinished( false )
	{
	}

	const tUnitDesc& tWaveDesc::fUnitDesc( u32 index ) const
	{
		sigassert( index < mUnitDescs.fCount( ) );
		return mUnitDescs[ index ];
	}

	u32	tWaveDesc::fUnitID( u32 index ) const
	{
		sigassert( index < mUnitDescs.fCount( ) );
		return mUnitDescs[ index ].mUnitID;
	}

	u32	tWaveDesc::fUIUnitID( u32 index ) const
	{
		sigassert( index < mUnitDescs.fCount( ) );
		return mUnitDescs[ index ].mUIUnitID;
	}

	u32 tWaveDesc::fCountry( u32 index ) const
	{
		sigassert( index < mUnitDescs.fCount( ) );
		return mUnitDescs[ index ].mCountry;
	}

	b32 tWaveDesc::fIsBarrage( tWaveDesc* desc )
	{
		return (desc->fUnitDescCount( ) && desc->fUnitDesc( 0 ).mUnitID == GameFlags::cUNIT_ID_BARRAGE_SPIN);
	}

	tWaveList::tWaveList( )
		: mCountry( GameFlags::cCOUNTRY_USSR )
		, mCurrentWaveIndex( 0 )
		, mTotalWaveIndex( 0 )
		, mState( cInactive )
		, mTimer( 0.f )
		, mLooping( false )
		, mAllowReadySkip( true )
		, mVeryFirstWave( true )
		, mMakeSelectableUnits( false )
		, mSaveable( true )
		, mSurvivalMode( false )
		, mBonusWave( false )
		, mBonusWavesActive( false )
		, mDisableAIFire( false )
		, mAssignPickup( ~0 )
		, mSurvivalRound( 0 )
		, mSurvivalWaveCount( 1 )
	{
	}

	tWaveList::~tWaveList( )
	{
	}

	void tWaveList::fSetTotalWaveIndex( u32 index ) 
	{ 
		if( fWaveCount( ) > 0 )
		{
			log_line( Log::cFlagRewind, " Setting Wavelist to "  << index );
			mTotalWaveIndex = index; 
			mCurrentWaveIndex = Math::fModulus( index, fWaveCount( ) );

			if( mWaveListUI )
			{
				mWaveListUI->fShow( true );

				// Go to that wave in the list
				for( u32 i = 1; i <= index; ++i )
				{
					u32 wave = Math::fModulus( i, fWaveCount( ) );
					if( fConsiderUIWave( mWaveList[ wave ].fGetRawPtr( ) ) )
						mWaveListUI->fNextWave( );
				}
			}
		}
	}

	void tWaveList::fUpdate( f32 dt )
	{
		if( !fIsActive( ) )
			return;

		tGameApp& gameApp = tGameApp::fInstance( );
		tWaveDesc* wave = fCurrentWave( );

		if( !wave )
			return;

		if( gameApp.fCurrentLevel( ) )
			dt *= gameApp.fCurrentLevel( )->fUnitTimeMultiplier( );

		mTimer -= dt;

		// Allow for state to transition completely through a wave in one frame if the times are 0

		if( mState == cReadying && mTimer <= 0.f )
		{
			mState = cWaiting;
			mTimer = wave->mCountdown;

			if( gameApp.fGameMode( ).fIsSinglePlayerOrCoop( ) )
			{
				const tDynamicArray< tPlayerPtr > & players = gameApp.fPlayers( );
				const u32 playerCount = players.fCount( );
				for( u32 p = 0; p < playerCount; ++p )
				{
					b32 enemyWave = players[ p ]->fEnemyCountry( ) == fCountry( );
					if( enemyWave )
						players[ p ]->fAddSkippableSeconds( wave->mCountdown );
				}
			}
		}

		if( mState == cWaiting )
		{
			b32 xButtonPressed = false;

			const b32 waveListVisible = ( mWaveListUI && mWaveListUI->fIsVisible( ) );
			if( gameApp.fGameMode( ).fIsSinglePlayerOrCoop( ) && waveListVisible )
			{
				const tDynamicArray< tPlayerPtr > & players = gameApp.fPlayers( );
				const u32 playerCount = players.fCount( );
				for( u32 p = 0; p < playerCount; ++p )
				{
					b32 enemyWave = players[ p ]->fEnemyCountry( ) == fCountry( );
					if( enemyWave && players[ p ]->fGameController( )->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_LAUNCH_WAVE ) )
					{
						xButtonPressed = true;
						players[ p ]->fStats( ).fIncStat( GameFlags::cSESSION_STATS_SECONDS_SKIPPED, fMax( mTimer, 0.f ) );
	
						if( players[ p ]->fUser( )->fIsLocal( ) )
							players[ p ]->fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_UI_SELECT_FORWARD );
					}
				}
			}

			sync_event_v_c( xButtonPressed, tSync::cSCInput );

			// clearing this later allows us to award multiple players the bonus if they press it at the same time
			if( xButtonPressed )
			{
				// Set timer to be finished so it goes on to the next wave
				mTimer = 0.f;
			}

			if( mWaveListUI )
				mWaveListUI->fCountdownTimer( mTimer );

			if( mTimer <= 0.f )
			{
				mState = cLaunching;
				mTimer = wave->mTotalLaunchTime;

				log_line( Log::cFlagWaveList, " WaveList: Launching pathname: " << wave->mPathName );
				fStartLaunching( );
			}
		}

		if( mState == cLaunching )
		{
			if( mWaveListUI )
				mWaveListUI->fLaunching( dt );

			if( mTimer <= 0.f || fFinishedLaunching( ) )
				fNextWave( );
		}
	}

	namespace
	{
		void fSetHighestSurvivalRound( tPlayer* player, u32 round )
		{
			sigassert( player );
			switch( tGameApp::fInstance( ).fChallengeMode( ) )
			{
			case GameFlags::cCHALLENGE_MODE_SURVIVAL:	player->fStats( ).fMaxStat( GameFlags::cSESSION_STATS_HIGHEST_SURVIVAL_ROUND, (f32)round ); break;
			case GameFlags::cCHALLENGE_MODE_LOCKDOWN:	player->fStats( ).fMaxStat( GameFlags::cSESSION_STATS_HIGHEST_LOCKDOWN_ROUND, (f32)round ); break;
			case GameFlags::cCHALLENGE_MODE_HARDCORE:	player->fStats( ).fMaxStat( GameFlags::cSESSION_STATS_HIGHEST_HARDCORE_ROUND, (f32)round ); break;
			}
		}
	}

	void tWaveList::fNextWave( ) 
	{ 
		if( !fIsActive( ) )
			return;

		mVeryFirstWave = false;

		do
		{
			if( mWaveList[ mCurrentWaveIndex ]->mPopWhenFinished )
			{
				mWaveList.fEraseOrdered( mCurrentWaveIndex );
			}
			else
			{
				++mTotalWaveIndex;
				++mCurrentWaveIndex;
			}
		} 
		while( mCurrentWaveIndex < fSurvivalWaveCount( ) && mWaveList[ mCurrentWaveIndex ]->fConsiderPreviousWave( ) );

		if( mCurrentWaveIndex >= fSurvivalWaveCount( ) )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			sigassert( level );

			// Just in case don't fire the level event until index and state and what not are set
			if( mLooping || mSurvivalMode )
			{
				mCurrentWaveIndex = 0;
				level->fOnWaveListFinished( this );

				if( mSurvivalMode )
				{
					fRampUpCurrentWaves( );

					++mSurvivalRound;
					mSurvivalWaveCount = fMin( mSurvivalWaveCount + 1, mWaveList.fCount( ) );

					if( Gameplay_Survival_BonusEveryTime || mBonusWavesActive )
					{
						fPause( ); // starts bonus wave
						return;
					}
				}
			}
			else
			{
				mState = cInactive;
				mTimer = 0.f;
				level->fOnWaveListFinished( this );
				if( mWaveListUI )
					mWaveListUI->fFinalEnemyWave( );

				return;
			}
		}

		if( mWaveListUI && !mWaveList[ mCurrentWaveIndex ]->mFunctionCallName.fExists( ) )
			mWaveListUI->fNextWave( );

		if( fWaveCount( ) > 0 )
			fBeginReadying( );
	}

	void tWaveList::fRampUpCurrentWaves( )
	{
		sync_line( );
		for( u32 i = 0; i < fSurvivalWaveCount( ); ++i )
		{
			tWaveDesc* desc = mWaveList[ i ].fGetRawPtr( );
			for( u32 u = 0; u < desc->mUnitDescs.fCount( ); ++u )
			{
				tUnitDesc& unit = desc->mUnitDescs[ u ];
				unit.mHealthModifier += Gameplay_Survival_HealthAdd;
				unit.mTimeMultiplier += Gameplay_Survival_TimeAdd;
				unit.mHealthModifier = fMin( (f32)Gameplay_Survival_HealthMax, unit.mHealthModifier );
				unit.mTimeMultiplier = fMin( (f32)Gameplay_Survival_TimeMax, unit.mTimeMultiplier );
			}
		}
	}

	void tWaveList::fSetActive( bool activate )
	{
		if( activate && fWaveCount( ) > 0 && mCurrentWaveIndex < fWaveCount( ) )
		{
			if( mState != cInactive )
				fSetActive( false );

			if( mWaveListUI )
			{
				mWaveListUI->fShow( true );

				//survival mode the main wave list
				mSurvivalMode = (tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).mMapType == GameFlags::cMAP_TYPE_SURVIVAL);
			}
			else
				mSurvivalMode = false;

			mVeryFirstWave = true;

			fBeginReadying( );
		}
		else
		{
			//stop generation
			for( u32 i = 0; i < fWaveCount( ); ++i )
			{
				for( u32 j = 0; j < mWaveList[ i ]->mGenerators.fCount( ); ++j )
					mWaveList[ i ]->mGenerators[ j ]->fStopGenerating( );
			}

			mState = cInactive;
			mTimer = 0.f;
			if( mWaveListUI )
				mWaveListUI->fShow( false );
		}
	}

	void tWaveList::fSetLooping( bool looping )
	{
		mLooping = looping;
		if( mWaveListUI )
			mWaveListUI->fLooping( mLooping );
	}

	void tWaveList::fSetHealthModifier( f32 modifier )
	{
		for( u32 i = 0; i < mWaveList.fCount( );  ++i )
			for( u32 u = 0; u < mWaveList[ i ]->mUnitDescs.fCount( ); ++u )
				mWaveList[ i ]->mUnitDescs[ u ].mHealthModifier = modifier;
	}

	void tWaveList::fStartLaunching( )
	{
		if( fWaveCount( ) == 0 || mCurrentWaveIndex >= fSurvivalWaveCount( ) )
			return;

		if( mWaveListUI )
		{
			mWaveListUI->fLaunchStart( );
			if( mSurvivalMode )
			{
				mWaveListUI->fSurvivalRound( mSurvivalRound + 1 ); // UI is one based

				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				sigassert( level );

				if( !level->fVictoryOrDefeat( ) && mCurrentWaveIndex == 0 )
				{
					tPlayer* player = tGameApp::fInstance( ).fGetEnemyPlayerByCountry( mCountry );
					sigassert( player );

					fSetHighestSurvivalRound( player, mSurvivalRound );
					if( tGameApp::fInstance( ).fGameMode( ).fIsCoOp( ) )
						fSetHighestSurvivalRound( tGameApp::fInstance( ).fOtherPlayer( player ), mSurvivalRound );

					if( mSurvivalRound == 9 )
					{
						tGameApp::fInstance( ).fAwardDeferredAchievementToAllPlayers( GameFlags::cACHIEVEMENTS_RESOLUTE );
						
						if( tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).mChallengeMode == GameFlags::cCHALLENGE_MODE_COMMANDO )
						{
							//earned jetpack
							for( u32 i = 0; i < tGameApp::fInstance( ).fPlayerCount( ); ++i )
							{
								tPlayer* p = tGameApp::fInstance( ).fGetPlayer( i );
								if( !p->fProfile( )->fEarnedJetPack( ) )
								{
									p->fProfile( )->fSetHasEarnedJetPack( );
									p->fAddEarnedItem( tEarnedItemData( tEarnedItemData::cJetPack ) );
								}
							}
						}
					}
					else if( mSurvivalRound == 4 )
						tGameApp::fInstance( ).fAwardAvatarAwardToAllPlayers( GameFlags::cAVATAR_AWARDS_T_SHIRT );
				}
			}

			// Save rewind:
			//  state needs to be set to readying before we get here

			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			sigassert( level->fWaveManager( )->fDisplayedWaveList( ) == this 
				|| (level->fVersusWaveManager( ) && level->fVersusWaveManager( )->fWaveLists( ).fIndexOf( this ) != ~0) );

			tGameApp::fInstance( ).fSaveRewind( fCurrentUIWaveListID( ), mVeryFirstWave );
		}

		b32 wasBarrage = false;
	
		tStringPtr audioSwitch;
		b32 makeSound = false;

		for( u32 i = mCurrentWaveIndex; i < fSurvivalWaveCount( ); ++i )
		{
			if( i == mCurrentWaveIndex || mWaveList[ i ]->fConsiderPreviousWave( ) )
			{
				if( tWaveDesc::fIsBarrage( mWaveList[ i ].fGetRawPtr( ) ) )
				{
					wasBarrage = true;
					tDynamicArray<tPlayerPtr>& players = tGameApp::fInstance( ).fPlayers( );
					for( u32 i = 0; i < players.fCount( ); ++i )
						if( players[ i ]->fCountry( ) == mCountry )
							players[ i ]->fGiveBarrage( false );
				}

				const u32 genCount = mWaveList[ i ]->mGenerators.fCount( );
				if( genCount > 0 )
				{
					if( !audioSwitch.fExists( ) && mWaveList[ i ]->mUnitDescs.fCount( ) > 0 )
						audioSwitch = tGameApp::fInstance( ).fUnitWaveAudioSwitch( mWaveList[ i ]->mUnitDescs[ 0 ].mUnitID, mWaveList[ i ]->mUnitDescs[ 0 ].mCountry );
					
					const u32 spawnCount = mWaveList[ i ]->mCount / genCount;
					const u32 spawnRemainder = mWaveList[ i ]->mCount % genCount;

					for( u32 j = 0; j < genCount; ++j )
					{
						u32 count = spawnCount + ( j < spawnRemainder ? 1 : 0 );

						log_line( Log::cFlagWaveList, " WaveList: Launching genName: " << mWaveList[ i ]->mGenerators[ j ]->fOwnerEntity( )->fName( ) << " path: " << mWaveList[ i ]->mPathName << " count: " << count );

						sigassert( mWaveList[ i ]->mList == this );
						mWaveList[ i ]->mGenerators[ j ]->fLaunch( *mWaveList[ i ], count );
						if( mWaveList[ i ]->mGenerators[ j ]->fSpawnLightAndSound( ) )
							makeSound = true;
					}
				}

				if( mWaveList[ i ]->mFunctionCallName.fExists( ) )
					tGameApp::fInstance( ).fCurrentLevel( )->fWaveListScriptCall( mWaveList[ i ]->mFunctionCallName );
			}
			else
				break;
		}

		if( mVersusPlayer && !wasBarrage )
		{
			mVersusPlayer->fStats( ).fIncStat( GameFlags::cSESSION_STATS_VERSUS_WAVES_LAUNCHED, 1.f );

			//achievement
			{
				//u32 previousTotal = (u32)mVersusPlayer->fProfile( )->fAllTimeStat( GameFlags::cSESSION_STATS_VERSUS_WAVES_LAUNCHED );
				u32 ingameTotal = (u32)mVersusPlayer->fStats( ).fStat( GameFlags::cSESSION_STATS_VERSUS_WAVES_LAUNCHED );
				if( ingameTotal >= 4 )
					mVersusPlayer->fAwardAchievementDeferred( GameFlags::cACHIEVEMENTS_AGGRESSIVE_INVESTMENT_STRATEGY );
			}
		}


		if( makeSound && audioSwitch.fExists( ) )
		{
			tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( )->fSetSwitch( cWaveTypeSwitch, audioSwitch );
			tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( )->fHandleEvent( AK::EVENTS::PLAY_UI_WAVELAUNCH );
		}
	}

	b32 tWaveList::fFinishedLaunching( ) const
	{
		if( fWaveCount( ) == 0 || mCurrentWaveIndex >= fSurvivalWaveCount( ) )
			return true;

		for( u32 i = mCurrentWaveIndex; i < fSurvivalWaveCount( ); ++i )
		{
			if( i == mCurrentWaveIndex || mWaveList[ i ]->fConsiderPreviousWave( ) )
			{
				const u32 genCount = mWaveList[ i ]->mGenerators.fCount( );
				if( genCount > 0 )
				{		 
					for( u32 j = 0; j < genCount; ++j )
						if( !mWaveList[ i ]->mGenerators[ j ]->fSpawnedEnough( ) )
							return false;
				}
			}
			else
				break;
		}

		return true;
	}

	void tWaveList::fBeginReadying( )
	{
		if( fWaveCount( ) == 0 || mCurrentWaveIndex >= fSurvivalWaveCount( ) )
			return;

		mState = cReadying;
		mTimer = fCurrentWave( )->mReadyTime;

		if( mWaveListUI && mTimer > 0.f )
			mWaveListUI->fReadying( );

		const tGrowableArray<tPathEntityPtr>& trenches = tGameApp::fInstance( ).fCurrentLevel( )->fTrenchStarts( );

		for( u32 i = mCurrentWaveIndex; i < fSurvivalWaveCount( ); ++i )
		{
			if( i == mCurrentWaveIndex || mWaveList[ i ]->fConsiderPreviousWave( ) )
			{
				const u32 genCount = mWaveList[ i ]->mGenerators.fCount( );
				if( genCount > 0 )
				{			
					for( u32 j = 0; j < mWaveList[ i ]->mGenerators.fCount( ); ++j )
					{
						mWaveList[ i ]->mGenerators[ j ]->fBeginReadying( );

						b32 trench = false;

						for( u32 trenchIndex = 0; trenchIndex < trenches.fCount( ); ++trenchIndex )
						{
							const tStringPtr& name = trenches[trenchIndex]->fName( );
							if( name.fExists( ) && name == mWaveList[ i ]->mPathName 
								&& trenches[ trenchIndex ]->fQueryEnumValue( GameFlags::cENUM_UNIT_TYPE ) == ~0 ) //dont trench to paths with unit types, those are for cargo
							{
								trench = true;
								break;
							}
						}

						if( trench )
						{
							// Spawn an equal amount on each, + 1 on remainder number of generators
							const u32 spawnCount = mWaveList[ i ]->mCount / genCount;
							const u32 spawnRemainder = mWaveList[ i ]->mCount % genCount;
							u32 count = spawnCount + ( j < spawnRemainder ? 1 : 0 );

							log_line( Log::cFlagWaveList, " WaveList: Generating genName: " << mWaveList[ i ]->mGenerators[ j ]->fOwnerEntity( )->fName( ) << " path: " << mWaveList[ i ]->mPathName << " count: " << count );

							sigassert( mWaveList[ i ]->mList == this );
							mWaveList[ i ]->mGenerators[ j ]->fStartGenerating( *mWaveList[ i ], count );
						}
					}

				}
			}
			else
				break;
		}
	}

	void tWaveList::fFinishReadying( )
	{
		if( mState != cReadying || !mAllowReadySkip || mVeryFirstWave )
			return;

		mTimer = 0.f;
	}

	// This will force the skip, regardless of internal state. very explicit
	void tWaveList::fFinishReadyingForScript( )
	{
		if( mState == cReadying )
			mTimer = 0.f;
	}

	void tWaveList::fReset( )
	{
		fSetActive( false );

		mCurrentWaveIndex = 0;
		mTotalWaveIndex = 0;
		mState = cInactive;
		mTimer = 0.f;
		mVeryFirstWave = true;

		if( mWaveListUI )
			mWaveListUI->fShow( false );
	}

	void tWaveList::fKillEveryone( )
	{
		for( u32 i = 0; i < mWaveList.fCount( ); ++i )
		{
			const u32 genCount = mWaveList[ i ]->mGenerators.fCount( );
			for( u32 j = 0; j < genCount; ++j )
				mWaveList[ i ]->mGenerators[ j ]->fKillEveryone( );
		}
	}

	void tWaveList::fReleaseTrenched( )
	{
		for( u32 i = 0; i < mWaveList.fCount( ); ++i )
		{
			const u32 genCount = mWaveList[ i ]->mGenerators.fCount( );
			for( u32 j = 0; j < genCount; ++j )
				mWaveList[ i ]->mGenerators[ j ]->fReleaseTrenched( );
		}
	}

	

	void tWaveList::fSetUI( const Gui::tSinglePlayerWaveListPtr& ui )
	{
		mWaveListUI = ui;
		if( mWaveListUI )
		{
			mWaveListUI->fShow( true );
		}
		fFillUIWaves( );
	}

	void tWaveList::fFillUIWaves( )
	{
		if( mWaveListUI.fNull( ) )
			return;

		mWaveListUI->fSetup( this );
	}

	u32 tWaveList::fNonSimultaneousWaveCount() const
	{
		u32 count = 0;

		for( u32 i = 0; i < mWaveList.fCount( ); ++i )
		{
			if( !mWaveList[ i ]->fConsiderPreviousWave( ) )
				count++;
		}

		return count;
	}

	u32 tWaveList::fUIWaveIDToTotalWaveIndex( u32 index ) const
	{
		if( index == 0 )
			return 0;

		u32 counter = 0;
		u32 i = 1;
		for( ; i < fWaveCount( ); ++i )
		{
			if( fConsiderUIWave( mWaveList[ i ].fGetRawPtr( ) ) )
				++counter;

			if( counter == index )
				break;
		}

		return i;
	}

	u32 tWaveList::fCurrentUIWaveListID( ) const
	{
		u32 wave = 0;
		for( u32 i = 1; i <= mTotalWaveIndex; ++i )
		{
			tWaveDescPtr wavePtr = mWaveList[ Math::fModulus( i, fWaveCount( ) ) ];
			if( fConsiderUIWave( wavePtr.fGetRawPtr( ) ) )
				++wave;
		}
		return wave;
	}

	void tWaveList::fDontWaitAtAll( )
	{
		mTimer = 0.f;
	}

	f32 tWaveList::fTimeRemaining( ) const
	{
		return mTimer;
	}

	void tWaveList::fReactivateReadying( f32 time )
	{
		mState = cWaiting;
		mTimer = time;
	}
}

namespace Sig
{
	void tWaveDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tWaveDesc, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("UnitDescCount"), &tWaveDesc::fUnitDescCount)
			.Func(_SC("UnitID"), &tWaveDesc::fUnitID)
			.Func(_SC("UIUnitID"), &tWaveDesc::fUIUnitID)
			.Func(_SC("Country"),&tWaveDesc::fCountry)
			.Func(_SC("WaveID"), &tWaveDesc::fWaveID)
			.Func(_SC("IsFunction"), &tWaveDesc::fIsFunction)
			;

		vm.fRootTable( ).Bind( _SC("WaveDesc"), classDesc );
	}

	void tWaveList::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tWaveList,Sqrat::DefaultAllocator<tWaveList> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("Activate"),		&tWaveList::fActivate)
			.Func(_SC("Pause"),			&tWaveList::fPause)
			.Func(_SC("IsActive"),		&tWaveList::fIsActive)
			.Func(_SC("SetLooping"),	&tWaveList::fSetLooping)
			.Func(_SC("IsLooping"),		&tWaveList::fIsLooping)
			.Func(_SC("Reset"),			&tWaveList::fReset)
			.Func(_SC("Name"),			&tWaveList::fName)
			.Func(_SC("FinishReadying"),&tWaveList::fFinishReadyingForScript)
			.Func(_SC("SetHealthModifier"),			&tWaveList::fSetHealthModifier)
			.Func(_SC("SetMakeSelectableUnits"),	&tWaveList::fSetMakeSelectableUnits)
			.Prop(_SC("WaveCount"),		&tWaveList::fWaveCount)
			.Func(_SC("Wave"),			&tWaveList::fWaveForScript)
			.Func(_SC("CurrentWave"),	&tWaveList::fCurrentWaveForScript)
			.Func(_SC("TotalWaveIndex"), &tWaveList::fTotalWaveID)
			.Prop(_SC("RoundCount"),	&tWaveList::fNonSimultaneousWaveCount)
			.Prop(_SC("CurrentUIWaveListID"),	&tWaveList::fCurrentUIWaveListID)	
			.Prop(_SC("Saveable"),		&tWaveList::fSaveable, &tWaveList::fSetSaveable)	
			.Prop(_SC("BonusWave"),		&tWaveList::fBonusWave, &tWaveList::fSetBonusWave)	
			.Prop(_SC("BonusWaveActive"),		&tWaveList::fBonusWaveActive, &tWaveList::fSetBonusWaveActive)	
			.Prop(_SC("DisableAIFire"),	&tWaveList::fDisableAIFire, &tWaveList::fSetDisableAIFire)	
			.Prop(_SC("AssignPickup"),	&tWaveList::fAssignPickup, &tWaveList::fSetAssignPickup)	
			.Func(_SC("KillEveryone"),	&tWaveList::fKillEveryone)
			.Func(_SC("ReleaseTrenched"),	&tWaveList::fReleaseTrenched)
			.Prop(_SC("SurvivalMode"),	&tWaveList::fSurvivalMode)
			;

		vm.fRootTable( ).Bind( _SC("WaveList"), classDesc );

		tWaveDesc::fExportScriptInterface( vm );
	}

}

