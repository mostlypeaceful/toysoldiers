#ifndef __tUserProfile__
#define __tUserProfile__

#include "tGameSessionStats.hpp"

namespace Sig
{

	struct tLevelScores
	{
	public:
		tLevelScores( u32 mapType = ~0, u32 index = ~0 );

		u32  fGetIndex( ) const { return mIndex; }
		u32  fGetMapType( ) const { return mMapType; }

		void fLock( );
		void fUnlock( );
		b32  fIsLocked( ) const;
		b32  fHasWon( u32 difficulty ) const;

		void fSetHighScore( u32 difficulty, u32 score );
		u32  fGetHighScore( u32 difficulty ) const;

		void fSetGoalComplete( u32 goalIndex, b32 complete );
		u32  fIsGoalComplete( u32 goalIndex ) const;

		void fSetMedalProgress( u32 difficulty, u32 medalIndex, u32 progress );
		u32  fMedalProgress( u32 difficulty, u32 medalIndex ) const;

		void fSetRankProgress( u32 progress ) { mRankProgress = progress; }
		s32 fIncRankProgress( s32 val ); // returns > -1 when fRankCountchanges, (returns fRankCount )
		u32 fRankProgress( ) const { return mRankProgress; }
		s32 fRankCount( ) const { return mRankCount; }

		void fSetFoundGoldenArcade( );
		b32  fHasFoundGoldenArcade( ) const;

		void fSetFoundAllGoldenBabushkas( );
		b32 fHasFoundAllGoldenBabushkas( ) const;
		void fSetFoundAllGoldenDogTags( );
		b32 fHasFoundAllGoldenDogTags( ) const;
		static u32 fGoldenObjectsPerLevel( ) { return cGoldenObjectTotal; }

		b32 fHasScore( u32 difficulty ) const { return mHighScoresPerDifficulty[ difficulty ].fScoreIsSet( ); }

	public:
		enum tMedalProgress
		{
			cNone,
			cBronze,
			cSilver,
			cGold,
			cPlatinum,
			cMedalProgressCount
		};
		enum tMedals
		{
			cDefenceMedal,
			cTimeMedal,
			cMoneyMedal,
			cOverallMedal,
			cMedalCount,
		};

		enum tMiscFlagValues
		{
			cLocked					= (1 << 0),
			cGoldenArcadeFound		= (1 << 1),
			cGoldenBabushkasFound	= (1 << 2),
			cGoldenDogTagsFound		= (1 << 3),
			// this can only store up to 8 flags!

			cGoldenObjectTotal    = 5
		};

		struct tDifficultyData
		{
			u32 mHighScore;
			tFixedArray< u8, cMedalCount > mMedals;

			tDifficultyData( ) 
				: mHighScore( -1 )
			{
				mMedals.fFill( 0 );
			}

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				archive.fSaveLoad( mHighScore );
				archive.fSaveLoad( mMedals );
			}

			b32 fScoreIsSet( ) const { return mHighScore != -1; }
		};

		u8 mIndex;
		u8 mMapType;
		u8 mGoalsComplete;
		b8 mMiscFlags;
		tFixedArray< tDifficultyData, GameFlags::cDIFFICULTY_COUNT > mHighScoresPerDifficulty;
		u16 mRankProgress;
		s8 mRankCount;

	public:
		static void fExportScriptInterface( tScriptVm& vm );

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			sigassert( GameFlags::cDIFFICULTY_COUNT == 5 && cMedalCount == 4 && cMedalProgressCount == 5 && "User profile version changed!" );

			archive.fSaveLoad( mIndex );
			archive.fSaveLoad( mMapType );
			archive.fSaveLoad( mMiscFlags );
			archive.fSaveLoad( mGoalsComplete );
			archive.fSaveLoad( mHighScoresPerDifficulty );
			archive.fSaveLoad( mRankProgress );
			archive.fSaveLoad( mRankCount );
		}
	};

	typedef tGrowableArray< tLevelScores > tLevelScoresTable;

	// When saved, the memory will first contain the header, followed by the profile.
	struct tUserProfileHeader
	{
		u32 mSize; //size of the user profile, not including the header
		u32 mVersion;

		tUserProfileHeader( )
			: mSize( 0 ), mVersion( 0 )
		{ }

		b32 fValid( ) const { return mSize != 0 && mVersion != 0; }
	};

	class tUserProfile : public tRefCounter
	{
	public:
		//static const u32 cCertVersion = 13;
		static const u32 cVersion = 13;
		static const f32 cMaxSurvivalTime;
		static const u32 cMaxMinigameTries;

		enum tControlProfile
		{
			cProfileCharacters,
			cProfileCamera,
			cProfileTurrets,
			cProfilePlanes,
			cProfileVehicles,
			cProfileShellCam,
		};

		enum tSettings
		{
			cSettingsCameraInversion,
			cSettingsCameraSouthpaw,
			cSettingsTurretsInversion,
			cSettingsTurretsSouthpaw,
			cSettingsShellCamInversion,
			cSettingsShellCamSouthpaw,
			cSettingsPlanesInversion,
			cSettingsPlanesSouthpaw,
			cSettingsVehiclesInversion,
			cSettingsVehiclesSouthpaw,
			cSettingsCharacterInversion,
			cSettingsCharacterSouthpaw,
			cSettingsDisableControllerVibe,
			cSettingsCount,

			// Extra settings here not included in the main UI settings, not serialized, are just generic flags.
			cSettingsScoresCached,
			cSettingEarnedJetpack,
			cSettingShownTUWarning
		};

		tUserProfile( );

		void fSetRealDefaults( );
		void fSetInversionDefaults( b32 userInversionSetting );
		void fSetSouthpawDefaults( b32 userSouthpawSetting );

		void fSetHighestLevelReached( u32 levelIndex, u32 dlc );
		u32	 fGetHighestLevelReached( u32 dlc ) const;

		b32 fAchievementAwarded( u32 index ) const;
		void fAwardAchievement( u32 index );
		void fSetAchievementMask( u32 mask ) { mAchievementMask = mask; }

		// this handles both avatar awards and gamer pics.
		b32 fAvatarAwarded( u32 index ) const;
		void fAwardAvatar( u32 index );
		
		b32  fSetting( u32 setting ) const;
		void fSetSetting( u32 setting, b32 value );

		b32 fSouthPaw( u32 profile )
		{
			switch( profile )
			{
			case cProfileCharacters: return fSetting( cSettingsCharacterSouthpaw ); break;
			case cProfileCamera: return fSetting( cSettingsCameraSouthpaw ); break;
			case cProfileTurrets: return fSetting( cSettingsTurretsSouthpaw ); break;
			case cProfilePlanes: return fSetting( cSettingsPlanesSouthpaw ); break;
			case cProfileVehicles: return fSetting( cSettingsVehiclesSouthpaw ); break;
			case cProfileShellCam: return fSetting( cSettingsShellCamSouthpaw ); break;
			default: sigassert( 0 && "Invalid profile!" );
			}
			return false;
		}

		b32 fInversion( u32 profile )
		{
			switch( profile )
			{
			case cProfileCharacters: return fSetting( cSettingsCharacterInversion ); break;
			case cProfileCamera: return fSetting( cSettingsCameraInversion ); break;
			case cProfileTurrets: return fSetting( cSettingsTurretsInversion ); break;
			case cProfilePlanes: return fSetting( cSettingsPlanesInversion ); break;
			case cProfileVehicles: return fSetting( cSettingsVehiclesInversion ); break;
			case cProfileShellCam: return fSetting( cSettingsShellCamInversion ); break;
			default: sigassert( 0 && "Invalid profile!" );
			}
			return false;
		}

		tLevelScores* fGetLevelScores( u32 mapType, u32 levelIndex );
		u32 fGetLevelCount( u32 mapType );

		// for survival + campaign + minigames, all levels, all difficulties
		u32 fGetTotalLevelScore( ) const;

		void fUnlockAllLevels( );

		f32 fIncAllTimeStat( u32 statType, f32 value ) { mAllTimeStats[ statType ] += value; return mAllTimeStats[ statType ]; }
		f32 fAllTimeStat( u32 statType ) { return mAllTimeStats[ statType ]; }
		void fSetAllTimeStat( u32 statType, f32 val ) { mAllTimeStats[ statType ] = val; }

		f32 fMusicVolume( ) const { return fByteToFloat( mMusicVolume ); }
		void fSetMusicVolume( f32 val ) { mMusicVolume = fFloatToByte( val ); }

		f32 fSfxVolume( ) const { return fByteToFloat( mSfxVolume ); }
		void fSetSfxVolume( f32 val ) { mSfxVolume = fFloatToByte( val ); }

		f32 fBrightness( ) const { return fByteToFloat( mBrightness ); }
		void fSetBrightness( f32 val ) { mBrightness = fFloatToByte( val ); }

		u32 fListeningMode( ) const { return mListeningMode; }
		void fSetListeningMode( u32 val ) { mListeningMode = val; }

		// Preserving last played levels rewinds
		void fSetLastPlayedWave( u32 levelIndex, u32 waveIndex );

		b32 fLastPlayedRewindValid( ) { return mLastPlayedRewinedWave != 0; }
		u32 fLastPlayedRewindLevel( ) const { return mLastCampaignLevelPlayed; }
		u32 fLastPlayedRewindWave( ) const { return mLastPlayedRewinedWave; }
		void fInvalidateLastRewind( ) { mLastPlayedRewinedWave = 0; }

		b32 fIncrementSurvivalTimer( f32 dt );
		b32 fIncrementMinigameCount( );

		f32 fSurvivalTimeRemaining( );
		u32 fMinigameTriesRemaining( );

		u8& fSaveDeviceId( ) { return mSaveDeviceId; }

		b32 fEarnedJetPack( );
		void fSetHasEarnedJetPack( );

		// debugging, earn everything possible
		void fCompleteEverything( );

	private:
		static u8  fFloatToByte( f32 v ) { return u8(v * 100.f); }
		static f32 fByteToFloat( u8 v ) { return v * 0.01f; }
		static b32 fIsFirstLevelInDlc( u32 index, u32 dlc );

		tUserProfile( const tUserProfile & ) { }
		tUserProfile & operator= (tUserProfile & ) { }

		u32						mAchievementMask; // this is serialized, but is overwritten from the profile info
		tGrowableArray< u32 >	mHighestLevelIndexReached; //stored per dlc
		u32						mSettingsMask;
		tFixedArray< tLevelScoresTable, GameFlags::cMAP_TYPE_COUNT > mLevelScores;
		tFixedArray< f32, GameFlags::cSESSION_STATS_COUNT > mAllTimeStats;
		u8						mMusicVolume;
		u8						mSfxVolume;
		u8						mBrightness;
		u8						mAvatarAwardMask;
		
		u8						mLastCampaignLevelPlayed;
		u8						mLastPlayedRewinedWave; //for last campaign level played
		u8						mListeningMode;

		// for trial mode
		f32						mSurvivalTimer;
		u8						mMiniGameCounter;

		tLevelScoresTable		mDevScores; //fake scores, not serialized!

		u8						mSaveDeviceId;
	
	public:
		static void fExportScriptInterface( tScriptVm& vm );

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			sigassert( cSettingsCount == 13 && "User profile version changed!" );
			sigassert( GameFlags::cMAP_TYPE_COUNT == 6 && "User profile version changed!" );
			sigassert( GameFlags::cSESSION_STATS_COUNT == 122 && "User profile version changed! Make sure to add the new enum value to cLeaderboardPlayerStatsColumnIdByStat and cPopertyPlayerStats in GameSession_xbox360.cpp and session stat data table AND the genLeaderBoard files. Then increment this number" );

			archive.fSaveLoad( mAchievementMask );
			archive.fSaveLoad( mHighestLevelIndexReached );
			archive.fSaveLoad( mSettingsMask );
			archive.fSaveLoad( mLevelScores );
			archive.fSaveLoad( mAllTimeStats );
			archive.fSaveLoad( mMusicVolume );
			archive.fSaveLoad( mSfxVolume );
			archive.fSaveLoad( mBrightness );
			archive.fSaveLoad( mAvatarAwardMask );
			archive.fSaveLoad( mLastCampaignLevelPlayed );
			archive.fSaveLoad( mLastPlayedRewinedWave );
			archive.fSaveLoad( mListeningMode );
			archive.fSaveLoad( mSaveDeviceId );
			//archive.fSaveLoad( mSurvivalTimer ); //this aren't supposed to persist since they restart after you restart the demo anyways
			//archive.fSaveLoad( mMiniGameCounter );
		}
	};

	typedef tRefCounterPtr<tUserProfile> tUserProfilePtr;
}

#endif//__tUserProfile__
