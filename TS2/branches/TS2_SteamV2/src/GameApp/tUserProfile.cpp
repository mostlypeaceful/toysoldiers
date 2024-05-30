#include "GameAppPch.hpp"
#include "tUserProfile.hpp"
#include "tGameApp.hpp"

namespace Sig
{

	devvar( bool, Gameplay_Profile_UseHighestLevelReachedOverride, false );
	devvar( u32, Gameplay_Profile_HighestLevelReachedOverride, 2 );
	devvar( bool, Gameplay_Profile_YInvert, false );
	devvar( bool, Gameplay_Profile_UnlockAllLevels, false );

	// Level Scores
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	tLevelScores::tLevelScores( u32 mapType, u32 index )
		: mIndex( index )
		, mMapType( mapType )
		, mMiscFlags( 0 )
		, mGoalsComplete( 0 )
		, mRankProgress( 0 )
		, mRankCount( -1 )
	{
		fLock( );
		mHighScoresPerDifficulty.fFill( tDifficultyData( ) );
	}

	void tLevelScores::fLock( ) 
	{ 
		mMiscFlags = fSetBits( mMiscFlags, cLocked );
	}

	void tLevelScores::fUnlock( ) 
	{ 
		mMiscFlags = fClearBits( mMiscFlags, cLocked );
	}

	b32 tLevelScores::fIsLocked( ) const 
	{ 
		return !Gameplay_Profile_UnlockAllLevels && fTestBits( mMiscFlags, cLocked );
	}

	b32 tLevelScores::fHasWon( u32 difficulty ) const
	{ 
		return (mHighScoresPerDifficulty[ difficulty ].mHighScore > -1); 
	}

	void tLevelScores::fSetHighScore( u32 difficulty, u32 score )
	{
		sync_event_c( "(diff,score)", Math::tVec2u( difficulty, score ), tSync::cSCUser );
		sigassert( difficulty < GameFlags::cDIFFICULTY_COUNT );
		sigassert( score != ~0 );

		u32& scoreInOut = mHighScoresPerDifficulty[ difficulty ].mHighScore;
		if( scoreInOut == ~0 || score > scoreInOut )
			scoreInOut = score;
	}

	u32 tLevelScores::fGetHighScore( u32 difficulty ) const
	{
		sigassert( difficulty < GameFlags::cDIFFICULTY_COUNT );
		return mHighScoresPerDifficulty[ difficulty ].mHighScore;
	}

	void tLevelScores::fSetGoalComplete( u32 goalIndex, b32 complete )
	{
		sync_event_c( "(goal,complete)", Math::tVec2u( goalIndex, (u32)complete ), tSync::cSCUser );

		sigassert( goalIndex < 8 );
		if( complete )
			mGoalsComplete = fSetBits( mGoalsComplete, (1<<goalIndex) );
		else
			mGoalsComplete = fClearBits( mGoalsComplete, (1<<goalIndex) );
	}

	b32 tLevelScores::fIsGoalComplete( u32 goalIndex ) const
	{
		sigassert( goalIndex < 8 );
		return fTestBits( mGoalsComplete, (1<<goalIndex) );
	}

	void tLevelScores::fSetMedalProgress( u32 difficulty, u32 medalIndex, u32 progress )
	{
		sync_event_c( "(medal,progress)", Math::tVec2u( medalIndex, progress ), tSync::cSCUser );

		sigassert( progress < cMedalProgressCount );
		u8& medalInOut = mHighScoresPerDifficulty[ difficulty ].mMedals[ medalIndex ];
		medalInOut = fMax<u8>( medalInOut, progress );
	}

	u32 tLevelScores::fMedalProgress( u32 difficulty, u32 medalIndex ) const
	{
		return mHighScoresPerDifficulty[ difficulty ].mMedals[ medalIndex ];
	}

	void tLevelScores::fSetFoundGoldenArcade( )
	{
		mMiscFlags = fSetBits( mMiscFlags, cGoldenArcadeFound );
	}

	b32 tLevelScores::fHasFoundGoldenArcade( ) const
	{
		return fTestBits( mMiscFlags, cGoldenArcadeFound );
	}

	s32 tLevelScores::fIncRankProgress( s32 val ) 
	{ 
		const u32 prevRankProgress = fRankProgress( );
		mRankProgress += val; 

		// Check if passed threshold
		const tLevelLoadInfo& levelInfo = tGameApp::fInstance( ).fLevelLoadInfo( mMapType, mIndex );
		s32 earnedRank = -1;
		for( u32 i = 0; i < levelInfo.fRankThresholdCount( ); ++i )
		{
			const u32 threshold = levelInfo.fRankThreshold( i );
			if( prevRankProgress < threshold && mRankProgress >= threshold )
				earnedRank = i;
		}

		if( earnedRank > -1 )
			mRankCount = earnedRank;

		return earnedRank;
	}

	void tLevelScores::fSetFoundAllGoldenBabushkas( )
	{
		mMiscFlags = fSetBits( mMiscFlags, cGoldenBabushkasFound );
	}

	b32 tLevelScores::fHasFoundAllGoldenBabushkas() const
	{
		return fTestBits( mMiscFlags, cGoldenBabushkasFound );
	}

	void tLevelScores::fSetFoundAllGoldenDogTags()
	{
		mMiscFlags = fSetBits( mMiscFlags, cGoldenDogTagsFound );
	}

	Sig::b32 tLevelScores::fHasFoundAllGoldenDogTags() const
	{
		return fTestBits( mMiscFlags, cGoldenDogTagsFound );
	}

	// User Profile
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	const f32 tUserProfile::cMaxSurvivalTime = 10.0f * 60.0f;
	const u32 tUserProfile::cMaxMinigameTries = 5;

	namespace
	{
		static const tStringPtr cControlProfileEnumAsStrings[ tUserProfile::cProfileCount ] =
		{
			tStringPtr( "ProfileCharacters" ),
			tStringPtr( "ProfileCamera" ),
			tStringPtr( "ProfileTurrets" ),
			tStringPtr( "ProfilePlanes" ),
			tStringPtr( "ProfileVehicles" ),
			tStringPtr( "ProfileShellCam" ),
			tStringPtr( "ProfileUI" )
		};
	}

	tUserProfile::tUserProfile( )
		: mAchievementMask( 0 )
		, mSettingsMask( 0 )
		, mMusicVolume( 100 )
		, mSfxVolume( 100 )
		, mBrightness( 30 )
		, mAvatarAwardMask( 0 )
		, mLastCampaignLevelPlayed( 0 )
		, mLastPlayedRewinedWave( 0 )
		, mListeningMode( Audio::tSystem::cListeningModeMedium )
		, mSurvivalTimer( 0.f )
		, mMiniGameCounter( 0 )
		, mSaveDeviceId( 0 )
	{
		// Set usable Defaults, just to let the game run
		const u32 mapTypeLevelCounts[ GameFlags::cMAP_TYPE_COUNT ] = { 0, 17, 5, 5, 0 };
		mHighestLevelIndexReached.fSetCount( GameFlags::cDLC_COUNT );
		mHighestLevelIndexReached.fFill( 0 );
		mAllTimeStats.fFill( 0.f );

		for( u32 mapType = 0; mapType < GameFlags::cMAP_TYPE_COUNT ; ++mapType )
		{
			tLevelScoresTable* table = &mLevelScores[ mapType ];
			if( mapType == GameFlags::cMAP_TYPE_DEVSINGLEPLAYER )
				table = &mDevScores; //fake scores

			table->fSetCount( mapTypeLevelCounts[ mapType ] );

			for( u32 i = 0; i < mapTypeLevelCounts[ mapType ]; ++i )
				(*table)[ i ] = tLevelScores( mapType, i );

			if( mapTypeLevelCounts[ mapType ] > 0 )
				(*table)[ 0 ].fUnlock( ); // All first levels are unlocked
		}
	}

	// Set real defaults, after we have loaded stuff and we know who we're setting them for
	//  This is not a reset function and is only complimentary to the default constructor
	void tUserProfile::fSetRealDefaults( )
	{
		// Set Defaults
		u32 mapTypeLevelCounts[ GameFlags::cMAP_TYPE_COUNT ];
		for( u32 i = 0; i < GameFlags::cMAP_TYPE_COUNT; ++i )
			mapTypeLevelCounts[ i ] = tGameApp::fInstance( ).fNumLevelsInTable( i );
		
		for( u32 mapType = 0; mapType < GameFlags::cMAP_TYPE_COUNT ; ++mapType )
		{
			tLevelScoresTable* table = &mLevelScores[ mapType ];
			if( mapType == GameFlags::cMAP_TYPE_DEVSINGLEPLAYER )
				table = &mDevScores; //fake scores

			table->fSetCount( mapTypeLevelCounts[ mapType ] );

			for( u32 i = 0; i < mapTypeLevelCounts[ mapType ]; ++i )
			{
				(*table)[ i ] = tLevelScores( mapType, i );

				if( !tGameApp::fInstance( ).fLevelLockedByDefault( mapType, i ) )
					(*table)[ i ].fUnlock( );
			}
		}

		// find the highest unlocked level for each dlc.
		mHighestLevelIndexReached.fFill( -1 );
		for( u32 dlc = 0; dlc < GameFlags::cDLC_COUNT; ++dlc )
		{
			for( u32 i = 0; i < mapTypeLevelCounts[ GameFlags::cMAP_TYPE_CAMPAIGN ]; ++i )
			{
				const tLevelLoadInfo& info = tGameApp::fInstance( ).fLevelLoadInfo( GameFlags::cMAP_TYPE_CAMPAIGN, i );

				if( info.mDlcNumber == dlc )
				{
					mHighestLevelIndexReached[ dlc ] = info.mLevelIndex;
					break;
				}
			}
		}

		sigassert( mHighestLevelIndexReached[ GameFlags::cDLC_COLD_WAR ] != ~0 && "Could not set default starting level. No unlocked levels found for Cold War." );
		sigassert( mHighestLevelIndexReached[ GameFlags::cDLC_NAPALM ] != ~0 && "Could not set default starting level. No unlocked levels found for Napalm DLC." );
		sigassert( mHighestLevelIndexReached[ GameFlags::cDLC_EVIL_EMPIRE ] != ~0 && "Could not set default starting level. No unlocked levels found for Spec Ops DLC." );
    //Adding asserts for TS1 content
		sigassert( mHighestLevelIndexReached[ GameFlags::cDLC_BRITISH ] != ~0 && "Could not set default starting level. No unlocked levels found for TS1 British campaign." );
		sigassert( mHighestLevelIndexReached[ GameFlags::cDLC_GERMAN ] != ~0 && "Could not set default starting level. No unlocked levels found for TS1 German campaign." );
		sigassert( mHighestLevelIndexReached[ GameFlags::cDLC_INVASION ] != ~0 && "Could not set default starting level. No unlocked levels found for TS1 Invasion DLC." );
		sigassert( mHighestLevelIndexReached[ GameFlags::cDLC_KAISER ] != ~0 && "Could not set default starting level. No unlocked levels found for TS1 Kaiser DLC." );
	}

	b32 tUserProfile::fIsFirstLevelInDlc( u32 index, u32 dlc )
	{
		u32 levelCount = tGameApp::fInstance( ).fNumLevelsInTable( GameFlags::cMAP_TYPE_CAMPAIGN );
		for( u32 i = 0; i < levelCount; ++i )
		{
			const tLevelLoadInfo& info = tGameApp::fInstance( ).fLevelLoadInfo( GameFlags::cMAP_TYPE_CAMPAIGN, i );
			if( info.mDlcNumber == dlc )
			{
				if( index == info.mLevelIndex )
					return true;
				else
					return false;
			}
		}
		return false;
	}

	void tUserProfile::fSetLastPlayedWave( u32 levelIndex, u32 waveIndex ) 
	{ 
		sigassert( levelIndex < std::numeric_limits<u8>::max( ) && waveIndex < std::numeric_limits<u8>::max( ) );
		mLastCampaignLevelPlayed = levelIndex;
		mLastPlayedRewinedWave = waveIndex;
	}

	b32 tUserProfile::fIncrementSurvivalTimer( f32 dt )
	{
		if( mSurvivalTimer > cMaxSurvivalTime )
			return false;
		
		mSurvivalTimer += dt;
		return true;
	}

	b32 tUserProfile::fIncrementMinigameCount( )
	{
		if( mMiniGameCounter >= cMaxMinigameTries )
			return false;

		++mMiniGameCounter;
		return true;
	}

	void tUserProfile::fSetInversionDefaults( b32 userInversionSetting )
	{
		fSetSetting( cSettingsCharacterInversion, userInversionSetting );
		fSetSetting( cSettingsCameraInversion, userInversionSetting );
		fSetSetting( cSettingsTurretsInversion, userInversionSetting );
		fSetSetting( cSettingsPlanesInversion, userInversionSetting );
		fSetSetting( cSettingsVehiclesInversion, userInversionSetting );
		fSetSetting( cSettingsShellCamInversion, userInversionSetting );
	}

	void tUserProfile::fSetSouthpawDefaults( b32 userSouthpawSetting )
	{
		fSetSetting( cSettingsCharacterInversion, userSouthpawSetting );
		fSetSetting( cSettingsCameraSouthpaw, userSouthpawSetting );
		fSetSetting( cSettingsTurretsSouthpaw, userSouthpawSetting );
		fSetSetting( cSettingsPlanesSouthpaw, userSouthpawSetting );
		fSetSetting( cSettingsVehiclesSouthpaw, userSouthpawSetting );
		fSetSetting( cSettingsShellCamSouthpaw, userSouthpawSetting );
	}

	void tUserProfile::fSetHighestLevelReached( u32 levelIndex, u32 dlc )
	{
		sync_event_v_c( levelIndex, tSync::cSCUser );
		mHighestLevelIndexReached[ dlc ] = fMax( levelIndex, mHighestLevelIndexReached[ dlc ] );

		tLevelScoresTable& scores = mLevelScores[ GameFlags::cMAP_TYPE_CAMPAIGN ];

		// set them all to unlocked incase we added new levels
		u32 countToUnlock = fMin( mHighestLevelIndexReached[ dlc ] + 1, tGameApp::fInstance( ).fNumLevelsInTable( GameFlags::cMAP_TYPE_CAMPAIGN ) );
		for( u32 i = 0; i < countToUnlock; ++i )
		{
			if( tGameApp::fInstance( ).fLevelDLCInTable( GameFlags::cMAP_TYPE_CAMPAIGN, i )  == dlc )
				mLevelScores[ GameFlags::cMAP_TYPE_CAMPAIGN ][ i ].fUnlock( );
		}

	}

	u32 tUserProfile::fGetHighestLevelReached( u32 dlc ) const
	{
		const tLevelLoadInfo& info = tGameApp::fInstance( ).fCurrentLevelLoadInfo( );
		if( info.mMapType == GameFlags::cMAP_TYPE_HEADTOHEAD || info.mMapType == GameFlags::cMAP_TYPE_SURVIVAL )
			return tGameApp::fInstance( ).fNumLevelsInTable( GameFlags::cMAP_TYPE_CAMPAIGN );
		else
			return Gameplay_Profile_UseHighestLevelReachedOverride ? Gameplay_Profile_HighestLevelReachedOverride : mHighestLevelIndexReached[ dlc ];
	}

	b32 tUserProfile::fAchievementAwarded( u32 index ) const 
	{ 
		sigassert( index < 32 ); 
		return fTestBits( mAchievementMask, (1<<index) ); 
	}

	void tUserProfile::fAwardAchievement( u32 index )  
	{ 
		sigassert( index < 32 ); 
		sync_event_v_c( index, tSync::cSCUser );
		mAchievementMask = fSetBits( mAchievementMask, (1<<index) ); 
	}

	b32 tUserProfile::fAvatarAwarded( u32 index ) const
	{
		sigassert( index < 8 ); 
		return fTestBits( mAvatarAwardMask, (1<<index) ); 
	}

	void tUserProfile::fAwardAvatar( u32 index )
	{
		sigassert( index < 8 ); 
		sync_event_v_c( index, tSync::cSCUser );
		mAvatarAwardMask = fSetBits( mAvatarAwardMask, (1<<index) ); 
	}

	b32  tUserProfile::fSetting( u32 setting ) const 
	{ 
		sigassert( setting < 32 );
		if( !this )
		{
			tScriptVm::fDumpCallstack( );
			return false;
		}
		return fTestBits( mSettingsMask, (1<<setting) ); 
	}

	void tUserProfile::fSetSetting( u32 setting, b32 value ) 
	{ 
		sigassert( setting < 32 ); 
		sync_event_c( "(setting,value)", Math::tVec2u( setting, value ), tSync::cSCUser );
		if( value )
			mSettingsMask = fSetBits( mSettingsMask, (1<<setting) ); 
		else
			mSettingsMask = fClearBits( mSettingsMask, (1<<setting) ); 
	}

	u32 tUserProfile::fGetLevelCount( u32 mapType )
	{
		sigassert( mapType < GameFlags::cMAP_TYPE_COUNT );

		tLevelScoresTable& scoresTable = mLevelScores[ mapType ];
		return scoresTable.fCount( );
	}

	u32 tUserProfile::fGetTotalLevelScore( ) const
	{
		u32 totalScore = 0u;

		// For every map type
		for( u32 mapType = 0; mapType < GameFlags::cMAP_TYPE_COUNT; ++mapType )
		{
			if( mapType != GameFlags::cMAP_TYPE_CAMPAIGN && mapType != GameFlags::cMAP_TYPE_SURVIVAL && mapType != GameFlags::cMAP_TYPE_MINIGAME )
				continue;

			const tLevelScoresTable& scoresTable = mLevelScores[ mapType ];

			// For ever level
			for( u32 level = 0; level < scoresTable.fCount( ); ++level )
			{
				// at every difficulty
				for( u32 diff = 0; diff < GameFlags::cDIFFICULTY_COUNT; ++diff )
				{
					if( scoresTable[ level ].fHasScore( diff ) )
						totalScore += scoresTable[ level ].fGetHighScore( diff );
				}
			}
		}

		return totalScore;
	}

	tLevelScores* tUserProfile::fGetLevelScores( u32 mapType, u32 levelIndex )
	{
		sigassert( mapType < GameFlags::cMAP_TYPE_COUNT );
		
		tLevelScoresTable* table = &mLevelScores[ mapType ];
		if( mapType == GameFlags::cMAP_TYPE_DEVSINGLEPLAYER )
			table = &mDevScores;

		if( levelIndex >= (*table).fCount( ) )
		{
			log_warning( 0, "Scores could not be found for map type and index: " << mapType << ", " << levelIndex << ". Reseting to defaults!" );
			fSetRealDefaults( );
		}

		if( levelIndex < (*table).fCount( ) )
			return &(*table)[ levelIndex ];
		else
		{
			sigassert( 0 && "Level scores do not exist for this maptype and index" );
			return NULL;
		}
	}

	void tUserProfile::fUnlockAllLevels()
	{
		for( u32 mapType = 0; mapType < GameFlags::cMAP_TYPE_COUNT; ++mapType )
		{
			tLevelScoresTable& scoresTable = mLevelScores[ mapType ];
			for( u32 i = 0; i < scoresTable.fCount( ); ++i )
			{
				scoresTable[ i ].fUnlock( );
			}
		}

		// set highest level reached so that decorations and challenges still work.
		u32 numCampaignLevels = tGameApp::fInstance( ).fNumLevelsInTable( GameFlags::cMAP_TYPE_CAMPAIGN );
		for( u32 dlc = 0; dlc < GameFlags::cDLC_COUNT; ++dlc )
		{
			for( u32 i = 0; i < numCampaignLevels; ++i )
			{
				const tLevelLoadInfo& info = tGameApp::fInstance( ).fLevelLoadInfo( GameFlags::cMAP_TYPE_CAMPAIGN, i );

				if( ( info.mDlcNumber == dlc ) && ( mHighestLevelIndexReached[ dlc ] < info.mLevelIndex ) )
				{
					mHighestLevelIndexReached[ dlc ] = info.mLevelIndex;
				}
			}
		}
	}

	f32 tUserProfile::fSurvivalTimeRemaining( )
	{
		return cMaxSurvivalTime - mSurvivalTimer;
	}

	u32 tUserProfile::fMinigameTriesRemaining( )
	{
		return cMaxMinigameTries - mMiniGameCounter;
	}

	void tUserProfile::fSetHasEarnedJetPack( )
	{
		fSetSetting( cSettingEarnedJetpack, true );
	}

	b32 tUserProfile::fEarnedJetPack( )
	{
		return fSetting( cSettingEarnedJetpack );

		//u32 levelCnt = tGameApp::fInstance( ).fNumCampaignLevels( 0 );

		//// on any difficulty
		//for( u32 d = GameFlags::cDIFFICULTY_NORMAL; d < GameFlags::cDIFFICULTY_COUNT; ++d )
		//{
		//	b32 failed = false;

		//	// all base levels
		//	for( u32 l = 0; l < levelCnt; ++l )
		//	{
		//		const tLevelScores* scores = fGetLevelScores( GameFlags::cMAP_TYPE_CAMPAIGN, l );
		//		if( scores->fMedalProgress( d, tLevelScores::cOverallMedal ) != tLevelScores::cPlatinum )
		//		{
		//			// must have platinum medal
		//			failed = true;
		//			break;
		//		}
		//	}

		//	if( !failed )
		//		return true;
		//}

		//return false;
	}

	void tUserProfile::fCompleteEverything( )
	{
		for( u32 i = 0; i < mHighestLevelIndexReached.fCount( ); ++i )
		{
			u32 highest = tGameApp::fInstance( ).fNumCampaignLevels( i );
			mHighestLevelIndexReached[ i ] = highest;
		}

		for( u32 i = 0; i < mLevelScores.fCount( ); ++i )
		{
			for( u32 x = 0; x < mLevelScores[ i ].fCount( ); ++x )
			{
				tLevelScores& scores = mLevelScores[ i ][ x ];
				for( u32 d = 0; d < GameFlags::cDIFFICULTY_COUNT; ++d )
					scores.fSetHighScore( d, 100 );

				for( u32 g = 0; g < 3; ++g )
					scores.fSetGoalComplete( g, true );

				for( u32 d = 0; d < GameFlags::cDIFFICULTY_COUNT; ++d )
					for( u32 g = 0; g < tLevelScores::cMedalCount; ++g )
						scores.fSetMedalProgress( d, g, fMin<u32>( tLevelScores::cBronze + g, tLevelScores::cPlatinum ) );

				scores.fUnlock( );
				scores.fSetFoundGoldenArcade( );
				scores.fSetFoundAllGoldenBabushkas( );
				scores.fSetFoundAllGoldenDogTags( );
				scores.fSetRankProgress( 100 );
			}
		}
	}

	const tStringPtr& tUserProfile::fControlProfileEnumToString( u32 enumValue )
	{
		sigassert( ( enumValue >= cProfileBegin ) && ( enumValue < cProfileEnd ) );
		
		if( ( enumValue >= cProfileBegin ) && ( enumValue < cProfileEnd ) )
		{
			return cControlProfileEnumAsStrings[ enumValue - cProfileBegin ];
		}

		return tStringPtr::cNullPtr;
	}

	u32 tUserProfile::fControlProfileStringToEnum( const tStringPtr& enumName )
	{
		for( int i = cProfileBegin; i < cProfileEnd; ++i )
		{
			if( cControlProfileEnumAsStrings[ i - cProfileBegin ] == enumName )
			{
				return i;
			}
		}

		return cProfileEnd;
	}
}

namespace Sig
{

	void tLevelScores::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tLevelScores, Sqrat::DefaultAllocator<tLevelScores> > classDesc( vm.fSq( ) );
		classDesc
			.Prop( _SC( "Index"),			&tLevelScores::fGetIndex ) 
			.Prop( _SC( "MapType"),			&tLevelScores::fGetMapType ) 
			.Prop( _SC( "Locked"),			&tLevelScores::fIsLocked )
			.Func( _SC( "GetHighScore" ),	&tLevelScores::fGetHighScore )
			.Func( _SC( "IsGoalComplete" ), &tLevelScores::fIsGoalComplete )
			.Func( _SC( "MedalProgress" ),	&tLevelScores::fMedalProgress )
			.Prop( _SC( "HasFoundGoldenArcade" ),	&tLevelScores::fHasFoundGoldenArcade )
			.Func( _SC( "SetFoundGoldenArcade" ),	&tLevelScores::fSetFoundGoldenArcade )
			.Prop( _SC( "RankProgress" ),			&tLevelScores::fRankProgress, &tLevelScores::fSetRankProgress )
			.Prop( _SC( "HasFoundAllGoldenBabushkas" ), &tLevelScores::fHasFoundAllGoldenBabushkas )
			.Func( _SC( "SetFoundAllGoldenBabushkas" ), &tLevelScores::fSetFoundAllGoldenBabushkas )
			.Prop( _SC( "HasFoundAllGoldenDogTags" ), &tLevelScores::fHasFoundAllGoldenDogTags)
			.Func( _SC( "SetFoundAllGoldenDogTags" ), &tLevelScores::fSetFoundAllGoldenDogTags )
			.StaticFunc( _SC( "GoldenObjectsPerLevel" ), &tLevelScores::fGoldenObjectsPerLevel )
			;
		vm.fRootTable( ).Bind(_SC("LevelScores"), classDesc);

		vm.fConstTable( ).Const(_SC("MEDAL_TYPE_DEFENSE"), (int)tLevelScores::cDefenceMedal);
		vm.fConstTable( ).Const(_SC("MEDAL_TYPE_AGGRESSION"), (int)tLevelScores::cTimeMedal);
		vm.fConstTable( ).Const(_SC("MEDAL_TYPE_PROFIT"), (int)tLevelScores::cMoneyMedal);
		vm.fConstTable( ).Const(_SC("MEDAL_TYPE_OVERALL"), (int)tLevelScores::cOverallMedal);
		vm.fConstTable( ).Const(_SC("MEDAL_TYPE_COUNT"), (int)tLevelScores::cMedalCount);

		vm.fConstTable( ).Const(_SC("MEDAL_RANK_NONE"), (int)tLevelScores::cNone);
		vm.fConstTable( ).Const(_SC("MEDAL_RANK_BRONZE"), (int)tLevelScores::cBronze);
		vm.fConstTable( ).Const(_SC("MEDAL_RANK_SILVER"), (int)tLevelScores::cSilver);
		vm.fConstTable( ).Const(_SC("MEDAL_RANK_GOLD"), (int)tLevelScores::cGold);
		vm.fConstTable( ).Const(_SC("MEDAL_RANK_PLATINUM"), (int)tLevelScores::cPlatinum);
		vm.fConstTable( ).Const(_SC("MEDAL_RANK_COUNT"), (int)tLevelScores::cMedalProgressCount);
	}

	void tUserProfile::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tUserProfile, Sqrat::NoCopy<tUserProfile> > classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc( _SC( "IsFirstLevelInDlc" ), &tUserProfile::fIsFirstLevelInDlc )
			.Func( _SC( "HighestLevelReached"),	&tUserProfile::fGetHighestLevelReached )
			.Func( _SC( "GetLevelScores"),		&tUserProfile::fGetLevelScores )
			.Func( _SC( "GetSetting"),			&tUserProfile::fSetting )
			.Func( _SC( "SetSetting"),			&tUserProfile::fSetSetting )
			.Prop( _SC( "MusicVolume" ),		&tUserProfile::fMusicVolume, &tUserProfile::fSetMusicVolume )
			.Prop( _SC( "SfxVolume" ),			&tUserProfile::fSfxVolume, &tUserProfile::fSetSfxVolume )
			.Prop( _SC( "Brightness" ),			&tUserProfile::fBrightness, &tUserProfile::fSetBrightness )
			.Prop( _SC( "ListeningMode" ),		&tUserProfile::fListeningMode, &tUserProfile::fSetListeningMode )
			.Prop( _SC( "LastPlayedRewindValid" ), &tUserProfile::fLastPlayedRewindValid )
			.Prop( _SC( "LastPlayedRewindLevel" ), &tUserProfile::fLastPlayedRewindLevel )
			.Prop( _SC( "LastPlayedRewindWave" ),  &tUserProfile::fLastPlayedRewindWave )
			.Func( _SC( "Stat"),				&tUserProfile::fAllTimeStat )
			.Func( _SC( "IncrementMinigameCount"), &tUserProfile::fIncrementMinigameCount )
			.Prop( _SC( "SurvivalTimeRemaining" ), &tUserProfile::fSurvivalTimeRemaining )
			.Prop( _SC( "MinigameTriesRemaining" ), &tUserProfile::fMinigameTriesRemaining )
			;
		vm.fRootTable( ).Bind(_SC("UserProfile"), classDesc);

		vm.fConstTable( ).Const( _SC( "USERPROFILE_CONTROLPROFILE_CHARACTERS" ), ( int )cProfileCharacters );
		vm.fConstTable( ).Const( _SC( "USERPROFILE_CONTROLPROFILE_CAMERA" ), ( int )cProfileCamera );
		vm.fConstTable( ).Const( _SC( "USERPROFILE_CONTROLPROFILE_TURRETS" ), ( int )cProfileTurrets );
		vm.fConstTable( ).Const( _SC( "USERPROFILE_CONTROLPROFILE_PLANES" ), ( int )cProfilePlanes );
		vm.fConstTable( ).Const( _SC( "USERPROFILE_CONTROLPROFILE_VEHICLES" ), ( int )cProfileVehicles );
		vm.fConstTable( ).Const( _SC( "USERPROFILE_CONTROLPROFILE_SHELL_CAM" ), ( int )cProfileShellCam );
		vm.fConstTable( ).Const( _SC( "USERPROFILE_CONTROLPROFILE_UI" ), ( int )cProfileUI );

		// Some values to help iteration
		vm.fConstTable( ).Const( _SC( "USERPROFILE_CONTROLPROFILE_END" ), ( int )cProfileEnd );
		vm.fConstTable( ).Const( _SC( "USERPROFILE_CONTROLPROFILE_BEGIN" ), ( int )cProfileBegin );
		vm.fConstTable( ).Const( _SC( "USERPROFILE_CONTROLPROFILE_COUNT" ), ( int )cProfileCount );

		vm.fConstTable( ).Const(_SC("SETTINGS_CHARACTERINVERSION"), (int)cSettingsCharacterInversion );
		vm.fConstTable( ).Const(_SC("SETTINGS_CHARACTERSOUTHPAW"), (int)cSettingsCharacterSouthpaw );
		vm.fConstTable( ).Const(_SC("SETTINGS_CAMERAINVERSION"), (int)cSettingsCameraInversion );
		vm.fConstTable( ).Const(_SC("SETTINGS_CAMERASOUTHPAW"), (int)cSettingsCameraSouthpaw );
		vm.fConstTable( ).Const(_SC("SETTINGS_TURRETSINVERSION"), (int)cSettingsTurretsInversion );
		vm.fConstTable( ).Const(_SC("SETTINGS_TURRETSSOUTHPAW"), (int)cSettingsTurretsSouthpaw );
		vm.fConstTable( ).Const(_SC("SETTINGS_PLANESINVERSION"), (int)cSettingsPlanesInversion );
		vm.fConstTable( ).Const(_SC("SETTINGS_PLANESSOUTHPAW"), (int)cSettingsPlanesSouthpaw );
		vm.fConstTable( ).Const(_SC("SETTINGS_VEHICLESINVERSION"), (int)cSettingsVehiclesInversion );
		vm.fConstTable( ).Const(_SC("SETTINGS_VEHICLESSOUTHPAW"), (int)cSettingsVehiclesSouthpaw );
		vm.fConstTable( ).Const(_SC("SETTINGS_SHELLCAMINVERSION"), (int)cSettingsShellCamInversion );
		vm.fConstTable( ).Const(_SC("SETTINGS_SHELLCAMSOUTHPAW"), (int)cSettingsShellCamSouthpaw );
		vm.fConstTable( ).Const(_SC("SETTINGS_DISABLE_CONTROLLER_VIBE"), (int)cSettingsDisableControllerVibe );

		vm.fConstTable( ).Const(_SC("TRIAL_MAX_MINIGAME_PLAYS"), (float)tUserProfile::cMaxMinigameTries );
		vm.fConstTable( ).Const(_SC("TRIAL_MAX_SURVIVAL_TIME"), (int)tUserProfile::cMaxSurvivalTime );

		tLevelScores::fExportScriptInterface( vm );
	}

}

