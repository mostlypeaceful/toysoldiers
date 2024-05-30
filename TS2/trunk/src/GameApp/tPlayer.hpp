#ifndef __tPlayer__
#define __tPlayer__
#include "Gfx/tCameraControllerStack.hpp"
#include "tBarrage.hpp"
#include "tUserProfile.hpp"
#include "tScreenSpaceNotification.hpp"
#include "tFocalPrompt.hpp"
#include "tBarrageController.hpp"
#include "Gfx/tFollowPathCamera.hpp"
#include "tInUseIndicator.hpp"
#include "tGameEffects.hpp"
#include "tRadialMenu.hpp"
#include "tAchievements.hpp"
#include "tPointCaptureUI.hpp"
#include "Audio/tSystem.hpp"
#include "tDeviceSelector.hpp"
#include "xlsp/xlsp.h"

namespace Sig
{
	namespace Gui
	{
		class tScoreUI;
		class tMiniMap;
		class tWorldSpaceFloatingText;
		class tComboTimerUI;
		class tPowerPoolUI;
		class tBatteryMeter;
		class tScoreUI;
		class tPersonalBestUI;
		class tOutOfBoundsIndicator;
		class tAchievementBuyNotification;
	}

	namespace Audio
	{
		class tListener;
	}

	class tGameSessionStats;
	class tUnitLogic;
	class tBarrage;
	class tRtsCursorLogic;
	class tOffensiveWaveDesc;
	class tProximityLogic;
	class tTurretLogic;
	class tBuildSiteLogic;
	struct tDamageContext;

	class tPlayer;
	typedef tRefCounterPtr<tPlayer> tPlayerPtr;


	struct tFocusItem
	{
		f32 mDelay;
		f32 mDuration;
		f32 mBlendDist;
		tStringPtr mMessage;
		tEntityPtr mEntity;

		tFocusItem( tEntity* entity = NULL
			, const tStringPtr& message = tStringPtr::cNullPtr
			, f32 delay = 1.f
			, f32 duration = 3.f
			, f32 blendDist = 10.f )
			: mEntity( entity )
			, mMessage( message )
			, mDelay( delay )
			, mDuration( duration )
			, mBlendDist( blendDist )
		{ }

	};

	struct tEarnedItemData
	{
	public:
		tEarnedItemData( u32 type = ~0, u32 val = ~0, b32 deferred = false ) 
			: mType( type )
			, mValue( val )
			, mDeferred( deferred )
		{ }

		enum 
		{ 
			cDecoration, 
			cRank, 
			cAchievement,
			cAvatarAward,
			cGoldenArcade,
			cJetPack,
		};

		u32 mType;
		u32 mValue;
		b32 mDeferred;

		b32 operator == ( const tEarnedItemData& right )
		{
			return mType == right.mType && mValue == right.mValue;
		}
	};

	class tPlayer : public tEffectPlayer
	{
		define_dynamic_cast( tPlayer, tEffectPlayer );
	public:
		enum tState
		{
			cStateLoading,
			cStatePlaying,
			cStateCount
		};
	public:
		static void fExportScriptInterface( tScriptVm& vm );

		void fResetProfile( );

	public:
		explicit tPlayer( const tUserPtr& user );
		~tPlayer( );

		virtual void fOnPause( b32 paused );

		b32  fIsActive( ) const { return true; }
		void fSetIsActive( b32 ) { }
		void fDisableStats( ) { mStatsEnabled = false; }
		b32  fIsLoading( ) const { return mState == cStateLoading; }

		void fOnLevelLoadBegin( );
		void fOnLevelUnloadBegin( );
		void fOnLevelUnloadEnd( );
		void fOnSimulationBegin( );
		void fOnTick( f32 dt );
		void fLevelOnSpawn( );

		void fReadProfile( );
		void fSaveProfile( );
		void fSaveDefaultProfile( );
		void fSaveFullyCompleteProfile( );
		tUserProfile* fProfile( ) const { return mProfile.fGetRawPtr( ); }
		void fSetProfile( tUserProfile * profile );
		b32  fCanWriteProfile( );

		b32 fAchievementAwarded( u32 index ) const { return mProfile->fAchievementAwarded( index ); }
		void fAwardAchievement( u32 index );
		void fAwardAchievementDeferred( u32 index );

		b32 fHasDLC( u32 dlcIndex ) const;
		b32 fInstalledDLC( u32 dlcIndex ) const;

		void fScriptAudioEvent( const char *event );
		u32  fListenerIndex( ) const;
		const Audio::tSourcePtr& fSoundSource( ) const { return mSoundSource; }
		Audio::tSource* fScriptSoundSource( ) const { return mSoundSource.fGetRawPtr( ); }

		b32 fVictorious( ) const;

		static const Math::tVec3f cInUseIndicatorOffset;

	public: // user stuff

		const tUserPtr&	fUser( ) const { return mUser; }
		tUser*			fUserFromScript( ) { return mUser.fGetRawPtr( ); }
		void			fSetUser( const tUserPtr & user );
		void			fApplyProfileSettings( );
		void			fKickUser( ); //incase something went horribly wrong


		void fRefreshUserIds( );
		b32 fHasValidUserId( ) const { return mUserId != tUser::cInvalidUserId; }
		b32 fUserIdsMatch( ) const { return mUser->fPlatformId( ) == mUserId; }
		b32 fOfflineUserIdsMatch( ) const { return mUser->fOfflinePlatformId( ) == mOfflineUserId; }

		const Input::tGamepad& fGamepad( ) const { return mUser->fFilteredGamepad( 0 ); }
		const Input::tGamepad& fRawGamepad( ) const { return mUser->fRawGamepad( ); }
		const Input::tGamepad& fFilteredGamepad( u32 level ) const { return mUser->fFilteredGamepad( level ); }
		Math::tRect fViewportRect( ) const { return mUser->fComputeViewportRect( ); }
		Math::tRect fViewportSafeRect( ) const { return mUser->fComputeViewportSafeRect( ); }
		std::string fHudLayerName( ) const;

		u32 fPlayerIndex( ) const;
		b32 fUserKicked( ) const { return mUserKicked; }

		b32 fNeedsToChooseSaveDevice( );
		void fChooseSaveDeviceId( b32 forceShow );
		b32 fDidChooseSaveDeviceId( );

	public: // cameras
		void fStepCameraStack( f32 dt );
		void fClearCameraStack( );
		void fPushCamera( const Gfx::tCameraControllerPtr& camera );
		void fPopCamera( );

		virtual void fActST( f32 dt ) { mDisableQuickSwitch = false; }
		virtual void fCameraST( f32 dt );

		const Gfx::tCameraControllerPtr& fCameraStackTop( ) const { return mCameraStack.fTop( ); }
		u32 fCameraStackCount( ) const { return mCameraStack.fCount( ); }
		Gfx::tCameraControllerStack& fCameraStack( ) { return mCameraStack; }
		const Gfx::tCameraControllerStack& fCameraStack( ) const { return mCameraStack; }
		
		void fPushFreeCamera( tEntity* startPoint );
		void fPushFrontEndCamera( tEntity* startPoint );
		void fPushRTSCamera( tEntity* startPoint );
		void fPushTransitionCamera( tEntity* endPoint );
		void fSetDefaultCameraParameters( const Gfx::tTripod& cameraTripod );
		void fDebugOnlyToggleFreeCam( );

		void fForceFilmGrainActive( b32 b ) { mForceFilmGrainActive = b; }

		void fCreateScreenSpaceNotifications( );
		void fReleaseScreenSpaceNotifications( );

		virtual tEffectData fGetEffectsData( tEntity* owner ) const;

	public: // Gameplay
		void fResetGameplayRelatedData( );

		void		fSetSelectedUnitLogic( tUnitLogic *unitLogic );
		tUnitLogic* fSelectedUnitLogic( ) const { return mHoveredUnit; }
		void		fSetCurrentUnitLogic( tUnitLogic *unitLogic );
		tUnitLogic* fCurrentUnit( ) const { return mCurrentUnit; }
		Sqrat::Object fCurrentUnitScript( ) const;

		static u32	fDefaultEnemyTeam( u32 team );
		u32			fTeam( ) const { return mTeam; }
		u32			fEnemyTeam( ) const { return mEnemyTeam; }
		void		fSetTeam( u32 team );
		void		fSetCountry( u32 country );
	private:
		void		fActuallySetTeam( u32 team );
		void		fActuallySetCountry( u32 country );
		b32			fActuallyReadProfile( );
	public:
		u32			fCountry( ) const { return mCountry; }
		u32			fEnemyCountry( ) const { return mEnemyCountry; }
		static u32	fEnemyCountry( u32 country );

		s32  fInGameMoney( ) const { return mInGameMoney; }
		void fSetInGameMoney( s32 money );
		s32  fTicketsLeft( ) const;
		void fSetTicketsLeft( s32 tickets );

		void fSubtractTickets( s32 tickets );
		void fOnTicketsChanged( );

	private:
		void fRawSetMoney( s32 money );
		void fConfigureAudio( );
	public:
		void fAddInGameMoney( s32 moneyToAdd );
		void fSpendInGameMoney( s32 moneyToSpend );
		b32  fAttemptPurchase( s32 cost, b32 force = false ); // will subtract cost from money if cost <= money, will use up to current money and allow purchase
		b32  fCouldPurchase( s32 cost ) const { return mInGameMoney >= cost; }

		b32 fHasPowerPoolShown( ) const;
		void fComboLost( );
		b32  fOverChargeActive( ) const;
		b32  fOverChargeAvailable( ) const;
		inline tGameSessionStats& fStats( ) const { sigassert( mStats ); return *mStats.fGetRawPtr( ); }
		inline tGameSessionStats* fStatsScript( ) const { sigassert( mStats ); return mStats.fGetRawPtr( ); }
		b32 fIsNotAllowedToSaveStats( ) const { return mIsNotAllowedToSaveStats; }
		void fSetProfileDataIsCached( ) { mCachedProfileInfo = true; }

		static void fLockScoreAndStats( b32 extraMiniGame, const tDynamicArray< tPlayerPtr >& players );
		Sqrat::Object fLevelScoreAndStats( );
		void fResetMiniGameStats( );

		void fCreateCachedStatProps( tGrowableArray<tUserProperty>& props, u32 tableIndex );
		void fCreateStatPropsAndWriteToProfile( u32& propCount, tFixedArray<tUserProperty, GameFlags::cSESSION_STATS_COUNT>& props, u32 tableIndex );

		void fEnemyReachedGoal( tUnitLogic* unit );
		void fEnemyKilled( const tDamageContext& dc, f32 damageAmount, tUnitLogic& killedLogic, u32 shareIndex, u32 shareCount );
		void fPauseComboTimers( b32 pause );

		void fIncrementLocationalStat( u32 stat, f32 value, tEntity* pos );
		void fIncrementWaveChain( tEntity* pos, f32 waveValue );
		void fResetWaveChain( );

		void fIncrementStarShatter( f32 val, tEntity* pos );

		void fAddBarrage( Sqrat::Object& barrage );
		const tBarragePtr& fCurrentBarrage( ) const { return mBarrageController ? mBarrageController->fCurrentBarrage( ) : tBarrageController::cNullBarrage; }
		const tBarrageControllerPtr& fBarrageController( ) const { return mBarrageController; }

		void fSetCursorLogic( const tRefCounterPtr<tRtsCursorLogic>& logic );
		const tRefCounterPtr<tRtsCursorLogic>& fCursorLogic( ) const { return mCursorLogic; }

		void fCloseRadialMenus( );

		b32 fUnitLocked( const tStringPtr& unitIDValueString ) const;
		Sqrat::Object fCollectUnlockedUnitIDs( u32 levelNumberBeat ) const;

		const tRefCounterPtr< Audio::tListener >& fAudioListener( ) const { return mAudioListener; }

		void fSetBarrageMeterValue( f32 newValue );

		void fLockInCurrentUnit( );
		tUnitLogic* fLockInUnit( const tStringPtr& name );
		void fLockInUnitDirect( tUnitLogic* unit ); //unique name for script
		void fUnlockFromUnit( b32 kickOut );
		void fAllowDPadSwitch( ) { mAllowDPadSwitchWhileLocked = true; } //when they're locked into a turret on a vehicle
		b32 fLockedInUnit( ) const { return mLockedInUnit; }

		tUnitLogic* fBuildTurret( tBuildSiteLogic* site, u32 unitID );
		b32 fQuickSwitchTurret( ) const { return mQuickSwitchTurret; }
		void fResetQuickSwitchTurret( ) { mQuickSwitchTurret = false; }

		void fShowScoreUI( b32 scoreVisible );
		const Gui::tScreenSpaceNotificationPtr& fScreenSpaceNotification( ) const { return mScreenSpaceNotification; }

		void fOnMatchEnded( );

		void fResetAmmo( ); //of the current player if it is set
		void fCancelOvercharge( );
		void fGiveOverCharge( );
		void fGiveBarrage( b32 forTutorial );
		void fSetBarrageSkip( tEntity* target );
		void fRestrictBarrage( const tStringPtr& name );
		void fGiveInstantBarrage( const tStringPtr& name, tEntity* target );
		void fForceUseBarrage( );
		void fSetUniqueBarrages( b32 unique );

		void fFocusTarget( const tFocusItem& item );
		void fSpawnIndicatorArrow( tEntity* attachTo, f32 duration );

		const tRefCounterPtr< Gui::tPersonalBestUI >& fPersonalBestUI( ) const { return mPersonalBestUI; }
		
		void fPushTiltShiftCamera( );
		void fPopTiltShiftCamera( );
		void fSetStepCameras( b32 enable ) { mStepCameras = enable; }

		void fBeginCameraPath( const tStringPtr& name, bool skipable );
		void fOnPathCameraFinished( Gfx::tFollowPathCamera& camera );

		// Extra Stuff
		void fSpawnCharacter( tFilePathPtr& path, const tStringPtr& name );
		void fRespawn( );
		tFilePathPtr mCharacterPath;
		tStringPtr mLastSpawnPt;
		Sqrat::Object fGetDataForDisplayCase( u32 unitID );

		void fAddSkippableSeconds( f32 seconds ) { mSkippableSeconds += seconds; }
		f32 fSkipableSeconds( ) const { return mSkippableSeconds; }

		b32 fOffensiveMenuOpen( ) const { return mWaveLaunchMenu; }
		b32 fDisableYAccessDueToFocus( ) const;
		f32 fDefaultZoom( ) const;
		f32 fDefaultTranstionCamZoom( ) const;
		Gfx::tLens fDefaultLens( f32 zoom ) const;

		void fSetDontResetComboOnExit( b32 value ) { mDontResetComboOnExit = value; }

		u32 fMoveThumbButton( u32 profile ) const
		{
			return fProfile( )->fSouthPaw( profile ) ? Input::tGamepad::cButtonRThumb : Input::tGamepad::cButtonLThumb;
		}
		u32 fAimThumbButton( u32 profile ) const
		{
			return fProfile( )->fSouthPaw( profile ) ? Input::tGamepad::cButtonLThumb : Input::tGamepad::cButtonRThumb;
		}
		Math::tVec2f fAimStick( u32 profile ) const 
		{ 
			Math::tVec2f result =  fProfile( )->fSouthPaw( profile ) ?  fGamepad( ).fLeftStick( ) : fGamepad( ).fRightStick( ); 
			if( fProfile( )->fInversion( profile ) )
				result.y *= -1.f;
			return result;
		}
		Math::tVec2f fMoveStick( u32 profile ) const { return fProfile( )->fSouthPaw( profile ) ? fGamepad( ).fRightStick( ) : fGamepad( ).fLeftStick( ); }


		struct tWaveKill
		{
			tWaveKill( f32 v = 0.0f, u32 c = 0 ) 
				: mValue( v ), mCombo( c ) 
			{ }

			f32 mValue; 
			u32 mCombo;

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				archive.fSaveLoad( mValue );
				archive.fSaveLoad( mCombo );
			}
		};
		struct tSpeedKill
		{
			tSpeedKill( u32 v = 0.0f, f32 p = 1.f ) 
				: mValue( v ), mPercentage( p ) 
			{ }

			u32 mValue; 
			f32 mPercentage;

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				archive.fSaveLoad( mValue );
				archive.fSaveLoad( mPercentage );
			}
		};

		typedef tGrowableArray< tWaveKill >  tWaveKillList;
		typedef tGrowableArray< tSpeedKill > tSpeedKillList;
		typedef tGrowableArray< u32 > tBombingRunList;
		

		const tWaveKillList& fWaveKillList( ) const { return mWaveBonusValues; }
		void fSetWaveKillList( const tWaveKillList& list ) { mWaveBonusValues = list; }

		const tSpeedKillList& fSpeedKillList( ) const { return mSpeedBonusValues; }
		void fSetSpeedKillList( const tSpeedKillList& list ) { mSpeedBonusValues = list; }

		const tBombingRunList& fBombingRunList( ) const { return mBombingRunValues; }
		void fSetBombingRunList( const tBombingRunList& list ) { mBombingRunValues = list; }

		void fDisableQuickSwitch( ) { mDisableQuickSwitch = true; }
		b32	 fQuickSwitchDisabled( ) const { return mDisableQuickSwitch; }
		b32	 fSwitchingUseTurret( ) const { return mChangeUseTurret; }

		b32 fBeatLevelTheFirstTime( ) const { return mBeatLevelTheFirstTime; }
		void fSetComboUI( u32 combo );
		void fBarrageHasEnded( );

		u32 fTotalGoldenArcadeCount( ) const { return 11; } //lol
		u32 fGoldenArcadeFoundCount( ) const;

		b32 fFoundAllGoldenBabushkas( ) const;
		b32 fFoundAllGoldenDogTags( ) const;

		void fAddEarnedItem( const tEarnedItemData& data );
		Sqrat::Object fEarnedItemTable( ) const;

		const tAchievementData& fGetAchievementData( u32 index );
		tRefCounterPtr< Gui::tPointCaptureUI > fPointCaptureUI( ) { return mPointCaptureUI; }

		void fAwardAvatarAward( u32 index );
		
		void fAwardSessionAwards( ); // only after first save

		const tRefCounterPtr< Gui::tOutOfBoundsIndicator >& fBToExitIndicator( ) const { return mOutOfBoundsIndicator; }
		
		static b32 fDebugRandyMode( );

		u32 fSaveDeviceId( ) { return mDeviceSelector.fSaveDeviceId( ); }
		tDeviceSelector& fDeviceSelector( ) { return mDeviceSelector; }
		void fOnStorageDeviceChanged( );
		void fDeviceSelectorLogic( );
		void fOnStorageDeviceSelected( );

		// they've chosen not choose a save device
		b32 fDoesntWantRewind( ) { return mDoesntWantRewind; }

		b32 fHasJetPack( ) const { return mHasJetPack; }

	private:
		void fOnTickLoading( f32 dt );
		void fOnTickPlaying( f32 dt );
		b32	 fInUseMode( ) const;
		void fTransitionUseTurrets( );
		void fUseBarrageCheck( f32 dt );
		void fUseOverChargeCheck( f32 dt );
		tEntity* fFindTurretInDirection( const Math::tVec3f& dir, tEntity* owner ) const;
		void fCheckToyBoxAlarm( f32 dt );
		void fStepTutorial( f32 dt );
		void fUpdateWaveLaunchMenu( );

		void fCheckKillStats( const tDamageContext& dc, tUnitLogic& killedLogic, f32 damageAmount, b32 barrageKill, b32 userUnit, b32& outOverkill, b32& outAssist, b32& outProne );

	private: //UI
		void fCreateScoreUI( );
		void fReleaseScoreUI( );
		void fInitScoreUI( );

		void fShowMoney( b32 show );
		void fShowTickets( b32 show );
		void fShowBarrageIndicator( b32 show );

		void fCreateFocalPrompt( );
		void fReleaseFocalPrompt( );

		void fStepFocus( f32 dt );
		void fShowFocus( b32 show );
		void fHideFocus( b32 hide );
		void fPushFocusCamera( b32 push );

		void fCreatePersonalBestUI( );
		void fReleasePersonalBestUI( );

		void fCreateMiniMap( );
		void fReleaseMiniMap( );
		void fInitMiniMap( );
		void fShowMiniMap( b32 show );

		void fCreatePowerPoolUI( );
		void fReleasePowerPoolUI( );
		void fShowOverchargeMeter( b32 show );

		void fCreatePointCaptureUI( );
		void fReleasePointCaptureUI( );

		void fCreateAchievementBuyNotification( u32 index );
		void fCreateAvatarAwardBuyNotification( u32 index );
		void fReleaseAchievementBuyNotification( );

		void fSpawnMoneyText( u32 amount, const Math::tVec3f& pos );

		void fCreateInUseIndicator( );
		void fReleaseInUseIndicator( );
		void fAddInUseIndicator( tUnitLogic* unit );
		void fShowInUseIndicator( b32 show, tUnitLogic* unit );
		void fCreateOutOfBoundsIndicator( );
		void fReleaseOutOfBoundsIndicator( );

		u32 fCurrentLevelHighScore( ) const;

		tLevelScores* fLevelScores( );

		f32 fApplyDamageEffect( ); //full screen effect
		void fProcessQuickSwitch( );
		void fQuickSwitchToUnitImp( tTurretLogic* logic );

		b32 fFullScreenOverlayActive( ) const { return mFullScreenOverlayActive; }
		void fSetFullScreenOverlayActive( b32 active );

		void fHideShowOffensiveWaveMenu( b32 show );
		void fCreateBonusText( const tLocalizedString& text, const Math::tVec3f& pos, u32 statID );
		void fStepJetpackQuery( );

	public: // UI public
		const tRefCounterPtr< Gui::tMiniMap >& fMiniMap( ) const { return mMiniMap; }
		Gui::tWorldSpaceFloatingText* fSpawnText( const char* text, const Math::tVec3f& pos, const Math::tVec4f& tint = Math::tVec4f( 1.f, 1.f, 1.f, 0.f ), f32 scale = 1.f, f32 timeToLive = 1.f, b32 flyToScreenCorner = false );
		Gui::tWorldSpaceFloatingText* fSpawnText( const tLocalizedString& locText, const Math::tVec3f& pos, const Math::tVec4f& tint = Math::tVec4f( 1.f, 1.f, 1.f, 0.f ), f32 scale = 1.f, f32 timeToLive = 1.f, b32 flyToScreenCorner = false );
		Gui::tWorldSpaceFloatingText* fSpawnLocText( tLocalizedString text, const Math::tVec3f& pos, const Math::tVec4f& tint, f32 scale );
		void fSpawnBonusText( u32 statID, const Math::tVec3f& pos );
		void fSpawnBonusTextWithCSuffix( u32 statID, const char* suffix, const Math::tVec3f& pos );
		void fSpawnBonusTextWithLocSuffix( u32 statID, const tLocalizedString& suffix, const Math::tVec3f& pos );

	public: // Stats public
		static b32 fIsTurret( u32 unitID );
		static b32 fIsMachineGun( u32 unitID );
		static b32 fIsArtillery( u32 unitID );
		static b32 fIsMortar( u32 unitID );
		static b32 fIsAntiTank( u32 unitID );
		static b32 fIsMakeshift( u32 unitID );
		static b32 fIsAntiAir( u32 unitID );
		static b32 fIsBasicInfantry( u32 unitID );
		static b32 fIsEliteInfantry( u32 unitID );

		static b32 fIsTransportCopter( u32 unitID );
		static b32 fIsGunship( u32 unitID );
		static b32 fIsAttackCopter( u32 unitID );

		static b32 fIsFighterPlane( u32 unitID );
		static b32 fIsTransportPlane( u32 unitID );
		static b32 fIsBomber( u32 unitID );

		static b32 fIsIFV( u32 unitID );
		static b32 fIsMedTank( u32 unitID );
		static b32 fIsHeavyTank( u32 unitID );

		static b32 fIsCar( u32 unitID );
		static b32 fIsATV( u32 unitID );
		static b32 fIsAPC( u32 unitID );

		static b32 fIsTank( u32 unitID );
		static b32 fIsPlane( u32 unitID );
		static b32 fIsCopter( u32 unitID );

	private:

		tState mState;
		tUserPtr mUser;

		// Used to determine if the user signed in is the user that was set on us
		tPlatformUserId mUserId;
		tPlatformUserId mOfflineUserId;

		tRefCounterPtr<tRtsCursorLogic> mCursorLogic;
		Gfx::tCameraControllerStack mCameraStack;
		u32 mTeam;
		u32 mEnemyTeam;
		u32 mCountry;
		u32 mEnemyCountry;
		s32 mInGameMoney;
		tRefCounterPtr< tGameSessionStats > mStats;
		tRefCounterPtr< Gui::tScoreUI >	mScoreUI;
		tRefCounterPtr< Gui::tMiniMap >	mMiniMap;
		tRefCounterPtr< Gui::tPowerPoolUI >	mPowerPool;
		tRefCounterPtr< Gui::tBatteryMeter > mBatteryMeter;
		tGrowableArray< tRefCounterPtr< Gui::tComboTimerUI > >	mComboTimers;
		tRefCounterPtr< Gui::tInUseIndicator > mInUseIndicator;
		tRefCounterPtr< Gui::tOutOfBoundsIndicator > mOutOfBoundsIndicator;
		tRefCounterPtr< Gui::tPointCaptureUI > mPointCaptureUI;
		tRefCounterPtr< Gui::tAchievementBuyNotification > mAchievementBuyNotification;
		Audio::tSourcePtr mSoundSource;

		tBarrageControllerPtr mBarrageController;

		b8 mDontResetComboOnExit;
		b8 mForceFilmGrainActive;
		b8 mOverChargeActive;
		b8 mChangeUseTurret;
		tEntityPtr mNewUseUnit;

		b8 mWasFailing;
		b8 mLockedInUnit;
		b8 mQuickSwitchTurret;
		b8 mAllowDPadSwitchWhileLocked; //when mLockedInUnit is set

		b8 mFocusShown;
		b8 mFocusCameraPushed;
		b8 mBeatLevelTheFirstTime; //true after Locking score and having beat level the first time
		b8 mIsNotAllowedToSaveStats; //due to being a co-op guy who has not unlocked this level.

		b8 mCachedProfileInfo;
		b8 mDisableQuickSwitch;
		b8 mFullScreenOverlayActive;
		b8 mUserKicked;
		b32 mStatsEnabled;

		f32 mSkippableSeconds;
		u32 mCurrentWaveChain;

		u32 mBombingRunCount;
		f32 mBombingRunTimer;
		Math::tVec3f mBombingRunLastKillPos;

		u32 mQuickSwitchesWhileOvercharged;

		u32 mDisableTiltShiftCount;
		b32 fDisableTiltShift( ) const { return mDisableTiltShiftCount; }
		void fSetDisableTiltShift( b32 disable );

		tUnitLogic* mCurrentUnit; // !NULL, if we're actually using a unit
		tUnitLogic* mHoveredUnit;
		tEntityPtr mHoverUnitEnt;

		tRefCounterPtr< Audio::tListener > mAudioListener;

		tUserProfilePtr mProfile;

		f32				mToyBoxAlarmTimer;

		Gui::tRadialMenuPtr mWaveLaunchMenu;
		Gui::tScreenSpaceNotificationPtr mScreenSpaceNotification;

		u32 mLastKillValue;
		f32 mFocusShowTimer;
		f32 mFocusCameraTimer;
		tUnitLogic* mCurrentFocusUnit;
		tFocusItem mCurrentItem;
		tRingBuffer<tFocusItem> mFocusItems;
		Gui::tFocalPromptPtr mFocalPrompt;
		tRefCounterPtr< Gui::tPersonalBestUI > mPersonalBestUI;

		tWaveKillList mWaveBonusValues; //for each wave bonus increment there will be a value here
		tSpeedKillList mSpeedBonusValues;
		tBombingRunList mBombingRunValues;
		Sqrat::Object mLevelScoreAndStats;

		// the follow path camera needs special lifetime considerations
		Gfx::tCameraControllerPtr mFollowPathCam;
		b32 mFollowPathCamActive;
		b32 mStepCameras;

		// For display case:
		Sqrat::Function mSelectedUnitChangedCallback;

		tGrowableArray< tEarnedItemData > mEarnedItems;
		tFixedArray< tAchievementData, GameFlags::cACHIEVEMENTS_COUNT > mAchievementData;
		f32 mStarPoints;

		// trial shiz
		u32 mSessionAchievementMask; //waiting to be written on purchase
		u32	mSessionAvatarMask; //waiting to be written on purchase

		tDeviceSelector mDeviceSelector;
		b32 mProfileDeviceAvailable;
		b32 mShowRemovedOncePerLevel;
		b32 mDoesntWantRewind;
		Sqrat::Function mStorageDeviceSelectionCallback;

		b32 mHasJetPack;
		tXLSPLeaderboard mJetpackQueryBoard;
	};
}

#endif//__tPlayer__
