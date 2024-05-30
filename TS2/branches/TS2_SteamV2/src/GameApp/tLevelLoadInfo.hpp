#ifndef __tLoadLevelInfo__
#define __tLoadLevelInfo__

namespace Sig
{
	
	class tLevelLoadInfo
	{
	public:
		static const u32 cInvalidMapFront = ~0;
		static const u32 cRankCount = 4;

		tLevelLoadInfo( ) 
		{
			fCommonInit( );
		}

		explicit tLevelLoadInfo( const tFilePathPtr& mapPath ) 
			: mMapPath( mapPath )
		{
			fCommonInit( );
		}

		void fCommonInit( )
		{
			mMapType = cInvalidMapFront;
			mLevelIndex = 0;
			mHighestWaveReached = -1;
			mChallengeMode = 0;
			mChallengeFlags = ~0; //all flags on by default
			mMoney = -1; // not set
			mCountry = GameFlags::cCOUNTRY_DEFAULT;
			mCountry2 = GameFlags::cCOUNTRY_DEFAULT;
			mDlcNumber = 0; 
			mTickets = ~0;
			mDifficulty = GameFlags::cDIFFICULTY_CASUAL;
			mPotentialTickets.fFill( 10 );
			mPreview = false;
			mSkipBriefing = false;
			mRankThresholds.fFill( 0 );
		}

		void fLog( ) const;
		void fFillEmptyWithDefaults( );
		b32 fIsAssetEquivalent( const tLevelLoadInfo& prevLevel ) const;

		template<class tArchive>
		void fSaveLoad( tArchive & archive )
		{
			archive.fSaveLoad( mGameMode );
			archive.fSaveLoad( mMapType );
			archive.fSaveLoad( mLevelIndex );
			archive.fSaveLoad( mHighestWaveReached );
			archive.fSaveLoad( mChallengeMode );
			archive.fSaveLoad( mChallengeFlags );
			archive.fSaveLoad( mMoney );
			archive.fSaveLoad( mTickets );
			archive.fSaveLoad( mDifficulty );
			archive.fSaveLoad( mMapPath );
			archive.fSaveLoad( mLoadScript );
			archive.fSaveLoad( mWaveList );
			archive.fSaveLoad( mDescriptionLocKey );
			archive.fSaveLoad( mPreviewImage );
			archive.fSaveLoad( mMapDisplayName );
			archive.fSaveLoad( mAudioID );
			archive.fSaveLoad( mCountry );
			archive.fSaveLoad( mDlcNumber );
			archive.fSaveLoad( mAvailableInTrial );
			archive.fSaveLoad( mPreview );
			archive.fSaveLoad( mRankDescLocKey );
			archive.fSaveLoad( mRankThresholds );
			archive.fSaveLoad( mPotentialTickets );
			archive.fSaveLoad( mPotentialMoneys );
			archive.fSaveLoad( mRankDescLocKey );
			archive.fSaveLoad( mCountry2 );
		}

		// ALL OF THESE VARIABLES MUST BE ARCHIVED
		tGameMode mGameMode;
		u32 mMapType; //GameFlags::tMAP_TYPE
		u32 mLevelIndex;
		s32 mHighestWaveReached;
		u32 mChallengeMode;
		u32 mChallengeFlags;
		s32 mMoney;
		u32 mTickets;
		u32 mDifficulty;
		tFilePathPtr mMapPath;
		tFilePathPtr mLoadScript;
		tFilePathPtr mWaveList;
		tStringPtr mDescriptionLocKey;
		tFilePathPtr mPreviewImage;
		tLocalizedString mMapDisplayName;
		tStringPtr mAudioID;
		u32 mCountry;
		u32 mCountry2;
		u32 mDlcNumber; // 0 if part of main game release, 1 for first dlc, etc.
		b32 mAvailableInTrial;
		b32 mPreview; // not saved
		b32 mSkipBriefing;
		tStringPtr mRankDescLocKey;
		tFixedArray<u32, cRankCount> mRankThresholds;
		tFixedArray<u32, GameFlags::cDIFFICULTY_COUNT> mPotentialTickets;
		tFixedArray<u32, GameFlags::cDIFFICULTY_COUNT> mPotentialMoneys;

		// All flags on by default
		enum tChallengeFlags
		{
			cChallengeFlagVehicles,
			cChallengeFlagBarrages,
			cChallengeFlagTurretAI,
		};

		const char* fMapPath( ) const { return mMapPath.fCStr( ); }
		const char* fLoadScript( ) const { return mLoadScript.fCStr( ); }
		const char* fWaveList( ) const { return mWaveList.fCStr( ); }
		const char* fDescriptionLocId( ) const { return mDescriptionLocKey.fCStr( ); }
		const char* fPreviewImage( ) const { return mPreviewImage.fCStr( ); }
		u32 fTickets( ) const { return mTickets; }
		u32 fMoney( ) const { return mMoney; }
		u32 fChallengeMode( ) const { return mChallengeMode; }
		b32 fChallengeVehicles( ) const { return fTestBits( mChallengeFlags, (1<<cChallengeFlagVehicles) ); }
		b32 fChallengeBarrages( ) const { return fTestBits( mChallengeFlags, (1<<cChallengeFlagBarrages) ); }
		b32 fChallengeTurretAI( ) const { return fTestBits( mChallengeFlags, (1<<cChallengeFlagTurretAI) ); }

		void fSetTickets( u32 tickets ) { mTickets = tickets; }
		void fSetMoney( u32 money ) { mMoney = money; }
		void fSetChallengeMode( u32 mode ) { mChallengeMode = mode; }
		void fSetChallengeVehicles( b32 enable ) { mChallengeFlags = fSetClearBits( mChallengeFlags, (1<<cChallengeFlagVehicles), enable ); }
		void fSetChallengeBarrages( b32 enable ) { mChallengeFlags = fSetClearBits( mChallengeFlags, (1<<cChallengeFlagBarrages), enable ); }
		void fSetChallengeTurretAI( b32 enable ) { mChallengeFlags = fSetClearBits( mChallengeFlags, (1<<cChallengeFlagTurretAI), enable ); }

		b32 fIsDisplayCase( ) const { return mMapType == GameFlags::cMAP_TYPE_DEVSINGLEPLAYER && mLevelIndex == 0; }
		b32 fIsFrontEnd( ) const { return mMapType == GameFlags::cMAP_TYPE_FRONTEND; }

		tGameMode* fGameModeForScript( ) { return &mGameMode; }

		const char* fRankDescLocKey( ) const { return mRankDescLocKey.fCStr( ); }
		u32 fRankThreshold( u32 rankIndex ) const { return mRankThresholds[ rankIndex ]; }
		u32 fRankThresholdCount( ) const { return mRankThresholds.fCount( ); }
		// To script
		static void fExportScriptInterface( tScriptVm& vm );

	};
	

}

#endif//__tLoadLevelInfo__
