#ifndef __tGameApp__
#define __tGameApp__
#include "tGameAppBase.hpp"
#include "tGameAppSession.hpp"
#include "tCachedScoreWriterSession.hpp"
#include "tProfiler.hpp"
#include "tPlayer.hpp"
#include "tDataTableFile.hpp"
#include "FX/tTracerTrailDef.hpp"
#include "tDebrisLogicDef.hpp"
#include "tSaveGame.hpp"
#include "Gui/tSaveUI.hpp"
#include "tTimedCallback.hpp"
#include "tLevelLoadInfo.hpp"
#include "Scripts/tScript64.hpp"
#include "tDLCEnumerator.hpp"
#if defined( platform_pcdx9 )
#include "tAppGraphicsSettings.hpp"
#endif

namespace Sig
{
	enum tProfilerGameCategoryName
	{
		cProfilePerfCharacterLogicActST = cProfilePerfCategoryCount,
		cProfilePerfCharacterLogicAnimateMT,
		cProfilePerfCharacterLogicPhysicsMT,
		cProfilePerfCharacterLogicMoveST,
		cProfilePerfCharacterLogicThinkST,
		cProfilePerfCharacterLogicCoRenderMT,

		cProfilePerfVehicleLogicCollisionQuery,
		cProfilePerfVehicleLogicCollision,
		cProfilePerfVehicleLogicAIST,
		cProfilePerfVehicleLogicUserST,
		cProfilePerfVehicleLogicActST,
		cProfilePerfVehicleLogicAnimateMT,
		cProfilePerfVehicleLogicPhysicsMT,
		cProfilePerfVehicleLogicMoveST,
		cProfilePerfVehicleLogicBaseMoveST,
		cProfilePerfVehicleLogicThinkST,
		cProfilePerfVehicleLogicBaseThinkST,
		cProfilePerfVehicleLogicBaseThinkSTWeapons,
		cProfilePerfVehicleLogicCoRenderMT,

		
		cProfilePerfTurretLogicActST,
		cProfilePerfTurretLogicAnimateMT,
		cProfilePerfTurretLogicPhysicsMT,
		cProfilePerfTurretLogicMoveST,
		cProfilePerfTurretLogicThinkST,
		cProfilePerfTurretLogicCoRenderMT,

		cProfilePerfProjectileLogicActST,
		cProfilePerfProjectileLogicAnimateMT,
		cProfilePerfProjectileLogicPhysicsMT,
		cProfilePerfProjectileLogicMoveST,
		cProfilePerfProjectileLogicThinkST,
		cProfilePerfProjectileLogicCoRenderMT,

		cProfilePerfPassengerAnimateMT,
		cProfilePerfPassengerMoveST,

		cProfilePerfBreakableAnimateMT,
		cProfilePerfBreakableMoveST,

		cProfilePerfExplosionLogicActST,
		cProfilePerfExplosionLogicThinkST,
		cProfilePerfExplosionLogicCoRenderMT,

		cProfilePerfProximityLogicActST,
		cProfilePerfProximityLogicCoRenderMT,

		cProfilePerfArcQuery,
		cProfilePerfArcRay,
		cProfilePerfArcGfx,

		cProfilePerfRTSCam,
		cProfilePerfWeaponUI,
		cProfilePerfTintStack,
		cProfilePerfSurfaceLookup,
		cProfilePerfTutorialEvents,

		cProfilePerfGameCategoryCount
	};

	namespace Log
	{
#include "GameLogFlags.hpp"
	}
}

namespace Sig
{
	class tLevelLogic;
	class tSaveGame;
	class tGamePostEffectManager;
	class tWaveList;

	class tGameApp : public tGameAppBase
	{
		friend class tGameAppSession;
		define_dynamic_cast( tGameApp, tGameAppBase );
	public:
		static inline tGameApp& fInstance( ) { sigassert( gGameApp ); return *gGameApp; }
	public:
		static Math::tVec4f cColorDirtyWhite;
		static Math::tVec4f cColorCleanWhite;
		static Math::tVec4f cColorLockedGreen;
		static Math::tVec4f cColorSuspendedBlue;

		static const tStringPtr cUnitIDSwitchGroup;
		static const tStringPtr cWeaponTypeSwitchGroup;
		static const tStringPtr cPlayerControlRTPC;
		static const tStringPtr cSurfaceTypeSwitchGroup;
		static const tStringPtr cLevelSwitchGroup;
		static const tStringPtr cPersonalityTypeSwitchGroup;
		static const tStringPtr cCrewmanSwitchGroup;
		static const tStringPtr cBarrageFactionSwitchGroup;

		static const tFilePathPtr & fDefaultLoadScriptPath( );

#if defined( platform_pcdx9 )
		// Content
		static tFilePathPtr cLocalAppData;
#endif

	public:
		enum tDataTableType
		{
			cDataTableLevelResources,
			cDataTableWaveList,
			cDataTableCommonWaveList,
			cDataTableBreakables,
			cDataTableCombos,
			cDataTableComboEnumValues,
			cDataTableContextAnims,
			cDataTableUnitUpgradeProgression,
			cDataTableSessionStats,
			cDataTableEffectsCombos,
			cDataTableCount
		};
		enum tGlobalScriptType
		{
			cGlobalScriptPlaceTurret,
			cGlobalScriptTurretOptions,
			cGlobalScriptVehicleOptions,
			cGlobalScriptAirborneOptions,
			cGlobalScriptInfantryOptions,
			cGlobalScriptBuildSiteOptions,
			cGlobalScriptPurchaseVehicleOptions,
			cGlobalScriptPowerUpOptions,
			cGlobalScriptWaveLaunch,
			cGlobalScriptSPWaveList,
			cGlobalScriptVersusWaveList,
			cGlobalScriptEnemiesAliveList,
			cGlobalScriptScoreUI,
			cGlobalScriptBarrageIndicator,
			cGlobalScriptBombDropOverlay,
			cGlobalScriptRationTicketUI,
			cGlobalScriptPersonalBestUI,
			cGlobalScriptDialogBox,
			cGlobalScriptMiniMap,
			cGlobalScriptEnemyHealthBar,
			cGlobalScriptInUseIndicator,
			cGlobalScriptTurretUpgradeIndicator,
			cGlobalScriptTurretRepairIndicator,
			cGlobalScriptUnitPickupIndicator,
			cGlobalScriptScreenSpaceHealthBarList,
			cGlobalScriptRtsCursorUI,
			cGlobalScriptRtsHoverText,
			cGlobalScriptWaveLaunchArrowUI,
			cGlobalScriptPowerPoolUI,
			cGlobalScriptHoverTimer,
			cGlobalScriptAnimatingCanvas,
			cGlobalScriptBatteryMeter,
			cGlobalScriptScreenSpaceNotification,
			cGlobalScriptFocalPrompt,
			cGlobalScriptWorldSpaceFlyingText,
			cGlobalScriptOutOfBoundsIndicator,
			cGlobalScriptPointCaptureUI,
			cGlobalScriptAchievementBuyNotification,
			cGlobalScriptAvatarAwardBuyNotification,
			cGlobalScriptNetUI,
			cGlobalScriptSniperTowerOptions,
			cGlobalScriptCount
		};
		enum tFontType
		{
			cFontFixedSmall,
			cFontSimpleSmall,
			cFontSimpleMed,
			cFontFancyMed,
			cFontFancyLarge,
			cFontCount
		};
		enum tWeaponDerivedType
		{
			cWeaponDerivedTypeGun,
			cWeaponDerivedTypeCannon,
			cWeaponDerivedTypeMortar,
			cWeaponDerivedTypeLightning,
			cWeaponDerivedTypeLaser,
			cWeaponDerivedTypeCount
		};
		enum tDebrisPiecesType
		{
			cDebrisPiecesDefault,
			cDebrisPiecesCount
		};
		enum tRtsCursorTextureID
		{
			cRtsCursorTextureIDArrow,
			cRtsCursorTextureIDRange,
			cRtsCursorTextureIDCount
		};
		enum tCharacterPropsColumns
		{
			cCharacterPropsLeftHandResource,
			cCharacterPropsRightHandResource,
			cCharacterPropsHelmetResource,
			cCharacterPropsCount
		};
		
	public:
		tGameApp( );
		virtual ~tGameApp( );
		void fGetResourceIDs( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolder[], u32 resourceFolderCount );
	
	public: // game modes
		//tLevelLoadInfoPtr fLevelLoadInfo( u32 front, u32 index ) const;
		tLevelLoadInfo fLevelLoadInfo( u32 front, u32 index ) const;
		const tLevelLoadInfo& fCurrentLevelLoadInfo( ) const { return mCurrentLevelLoadInfo; }
		const tLevelLoadInfo& fNextLevelLoadInfo( ) const { return mNextLevelLoadInfo; }
		const tLocalizedString& fCurrentLevelDisplayName( ) const { return mCurrentLevelLoadInfo.mMapDisplayName; }
		const tGameMode& fGameMode( ) const { return mCurrentLevelLoadInfo.mGameMode; }

	public: // misc
		u32 fDifficulty( ) const { return mCurrentLevelLoadInfo.mDifficulty; }
		u32 fChallengeMode( ) const { return mCurrentLevelLoadInfo.mChallengeMode; }
		u32 fMaxTickets( ) const;

		// Use the difficulty override during minigames, only spot used in the game to have minimal impact (in title update)
		u32 fDiffcultyOverride( ) const { return mDifficultyOverride; }
		void fSetDifficultyOverride( u32 dOverride ) { mDifficultyOverride = dOverride; }

		virtual void fAddToStatText( std::stringstream& statsText );
		virtual void fPause( bool pause, Audio::tSource* source );
		virtual bool fPaused( ) const;
		virtual void fSetIngameSimulationState( b32 sim );
		virtual b32 fUseLowDetailTextures( ) { return !mGraphicsSettings.fTextureDetail(); }
		b32 fAAACantLose( ) const;

		static b32 fExtraDemoMode( );
		static b32 fPAXDemoMode( );
		static b32 fE3Mode( );
		b32 fPAXDemoModeScript( ) const { return fPAXDemoMode( ); }
		b32 fE3ModeScript( ) const { return fE3Mode( ); }

		b32 fHasEverReachedFrontEnd( ) { return mHasEverReachedFrontEnd; }

		// this is called from the level, in the run lists
		void fStepTimedCallbacks( f32 dt );
	
	public: // level stuff
		void fOnLevelLoadBegin( );
		void fOnLevelLoadEnd( );
		void fOnLevelUnloadBegin( ); //Happens while game state is preserved, before destruction begins
		void fOnLevelUnloadEnd( );  //Happens after game state is destroyed
		void fOnSimulationBegin( );
		void fAddLevelResource( const tResourcePtr& levelResource );
		tLevelLoadInfo fQueryLevelLoadInfoFromTable( u32 mapType, u32 levelIndex ) const;
	private:
		tLevelLoadInfo & fLoadingLevelLoadInfo( b32 levelUnloaded ) { return levelUnloaded ? mCurrentLevelLoadInfo : mNextLevelLoadInfo; }
		tLevelLoadInfo fQueryLevelLoadInfoFromPath( const tFilePathPtr& mapPath ) const;
		void fFillLevelLoadInfo( tLevelLoadInfo& levelLoadInfo, const tDataTable& table, u32 mapType, u32 levelIndex ) const;
		void fSnapshotMemoryConsumption( );
	public:
		void fSetLevelLoadState( class tGameLoadAppState * state );
		void fLoadLevelDelayed( );
		void fLoadLevelNowThatSessionIsReady( );
		void fLoadLevel( const tLevelLoadInfo& levelLoadInfo );
		void fLoadLevelFromScript( u32 mapType, u32 levelIndex, Sqrat::Function& levelInfoCallback );
		void fLoadLevelCoOpFromScript( u32 mapType, u32 levelIndex, Sqrat::Function& levelInfoCallback );
		void fLoadFrontEnd( ) { fLoadFrontEnd( false ); }
		void fLoadFrontEnd( b32 firstBootUp );
		void fReloadCurrentLevel( );
		void fConfigureAppForGameMode( const tLevelLoadInfo& loadLevelInfo );
		void fConfigureViewportsForGameMode( const tGameMode& gameMode );
		void fAssignViewportsToPlayers( const tGameMode& gameMode );
		void fSetViewportProportions( const tGameMode& gameMode );
		virtual void fResetPlayerCameraAspectRatios();
		
		tLevelLogic* fCurrentLevel( ) const { return mCurrentLevel; }
		tLevelLogic* fCurrentLevelDemand( ) { sigassert( mCurrentLevel ); return mCurrentLevel; }
		Sqrat::Object fCurrentLevelForScript( );
		void fSetCurrentLevelForScript( Sqrat::Object obj );
		void fSetCurrentLevel( tLevelLogic* level );
		
		const tSaveGame* fCurrentSaveGame( ) const { return mCurrentSaveGame.fGetRawPtr( ); }
		b32 fFromSaveGame( ) const { return (fCurrentSaveGame( ) != NULL); }
		b32 fSpawningCurrentLevel( ) const { return mSpawningCurrentLevel; }
		void fOnLevelSpawnBegin( );
		void fOnLevelSpawnEnd( );

		// access level info from LevelResources.xls
		u32 fNumLevelsInTable( u32 mapType ) const;
		u32 fNumCampaignLevels( u32 dlc ) const;
		u32 fNextCampaignLevel( u32 index, u32 dlc ) const;
		b32 fIsCampaignComplete( u32 dlc, tPlayer* player ) const;
		const char* fLevelNameInTable( u32 mapType, u32 ithLevel ) const;
		u32 fLevelDLCInTable( u32 mapType, u32 ithLevel ) const;
		u32 fRandomLevelIndex( tRandom& rand, GameFlags::tMAP_TYPE mapType, u32 legalAddOnFlags ) const;

		// debug level stuff
		u32 fNumDebugLevels( ) const;
		const char* fDebugLevelName( u32 ithLevel ) const;

		// Saving/Loading
		void fRewind( tUser* user, u32 waveIndex );
		void fSaveRewind( u32 waveIndex, b32 justPreview ); //calls fAddSaveInstance
		b32  fBuildSaveData( tSaveGameData & data, tSaveGameRewindPreview & preview );
		void fAddSaveInstance( const Gui::tSaveUI::tSaveInstancePtr& save );
		tLevelLoadInfo fGetSaveGameLevelLoadInfo( tGameArchive & archive );

		void fApplyRichPresence( );
		void fFullLicenseAcquired( );

		void fSetBrightness( f32 brightness );
		void fSetMusicVolume( f32 vol );
		void fSetSfxVolume( f32 vol );
		void fSetHUDVolume( f32 vol );
		void fSetListeningMode( u32 mode );

		virtual void fOnDeviceReset( );

		b32 fIsDisplayCase( ) const;
		b32 fLoadingLevel( ) const { return mGameAppSession.fLoadingLevel( ); }

		void fWriteCachedLeaderboards( );

	public: // global resources/assets
		void fSetWaveListTable( const tResourcePtr& waveListResource );
		b32 fHasWaveListTable( ) const { return fDataTableResource( cDataTableWaveList ) && fDataTableResource( cDataTableWaveList )->fLoaded( ); }
		
		const tResourcePtr& fDataTableResource( tDataTableType i ) const { return mDataTables[ i ]; }
		const tDataTableFile& fDataTable( tDataTableType i ) const { sigassert( mDataTables[ i ] && "Table doesnt exist" ); return *mDataTables[ i ]->fCast< tDataTableFile >( ); }
		const tStringHashDataTable& fUnitsSharedTable( u32 country ) const { return mUnitsShared.fIndexTable( country ); }
		const tStringHashDataTable& fWeaponsTable( ) const { return mWeapons.fIndexTable( 0 ); }
		const tStringHashDataTable& fCargoTable( ) const { return mCargo.fIndexTable( 0 ); }
		const tStringHashDataTableFile& fUnitsPhysicsTable( u32 country ) const { sigassert( mUnitsPhysics[ country ].fIsSet( ) ); return mUnitsPhysics[ country ]; }
		const tStringHashDataTable& fTracersTable( ) const { return mTracers.fIndexTable( 0 ); }
		const tStringHashDataTable& fCharacterPropsTable( u32 country ) const { return mCharacterProps.fIndexTable( country ); }
		tStringPtr fSurfaceLookup( const tStringPtr& tableName, u32 surfaceType ) const;
		const tResourcePtr& fGlobalScriptResource( tGlobalScriptType i ) const { return mGlobalScripts[ i ]; }
		const tStringHashDataTable& fFocusItemsTable( ) const { return mFocusItems.fIndexTable( 0 ); }
		
		b32			 fLevelLockedByDefault( u32 mapType, u32 level ) const;
		tFilePathPtr fUnitResourcePath( u32 unitID, u32 country ) const;
		tFilePathPtr fUnitWaveIconPath( u32 unitID, u32 country ) const;
		u32			 fUnitType( u32 unitID, u32 country );
		u32			 fUnitIDAlias( u32 unitID, u32 country ) const;
		u32			 fOverChargeCost( u32 unitID, u32 country ) const;
		tStringPtr	 fUnitWaveAudioSwitch( u32 unitID, u32 country ) const;
		tFilePathPtr fUpgradeClockPath( GameFlags::tBUILD_SITE size ) const { return mUpgradeClockPath[ size ]; }
		const FX::tTracerTrailDef& fTracerTrailDef( u32 type );
		tDebrisLogicDef& fDebrisLogicDef( GameFlags::tDEBRIS_TYPE type );
		tResourcePtr fCannonAimingTexture( ) { return mCannonAimingTexture; }
		tResourcePtr fDefaultDecalTexture( ) { return mDefaultDecalTexture; }
		tResourcePtr fRtsCursorTexture( tRtsCursorTextureID id ) { return mRtsCursorTexture[ id ]; }
		const tRefCounterPtr<tGamePostEffectManager>& fPostEffectsManager( ) const { return mPostEffectMgr; }
		f32 fComboEnumValue( u32 comboEnum ) const { return fDataTable( cDataTableComboEnumValues ).fIndexTable( 0 ).fIndexByRowCol<f32>( comboEnum, 0 ); }
	
		void fSetUnitUpgradeProgressionTable( const tFilePathPtr& path );
		const tResourcePtr& fTrajectoryTargetVisual( ) const { return mTrajectoryTargetVisual; }

		void fAddTimedCallback( f32 time, const Sqrat::Function& func );
		void fAddPausableTimedCallback( f32 time, const Sqrat::Function& func );

		void fScriptAudioEvent( const char *event );
		void fSetAudioParam( const char *param, f32 val );

		b32 fRewindMenuDisabled( ) const;
		s32 fHighestRewindableWaveIndex( ) const { return mCurrentLevelLoadInfo.mHighestWaveReached; }

		Sqrat::Object fAC130BarrageData( u32 vpIndex ) const;
		Math::tRect fComputeScreenSafeRect( ) const;

		tCachedScoreWriterSession& fCachedScoreWriter( ) { return mCachedScoreWriterSession; };
		virtual void fOnStorageDeviceChanged( );

		u32& fDeathLimiter( ) { return mDeathLimiter; }

	public: // players
		tPlayer* fFrontEndPlayer( ) { return mFrontEndPlayer.fGetRawPtr( ); }
		tPlayer* fSecondaryPlayer( ) { return fOtherPlayer( fFrontEndPlayer( ) ); }
		u32 fPlayerCount( ) const { return mPlayers.fCount( ); }
		tPlayer * fGetPlayer( u32 index ) { return mPlayers.fCount( ) > index ? mPlayers[ index ].fGetRawPtr( ) : NULL; }
		tPlayer * fGetPlayerByTeam( u32 team );
		tPlayer * fGetPlayerByCountry( u32 country );
		tPlayer * fGetEnemyPlayerByCountry( u32 country );
		tPlayer * fGetPlayerByUser( tUser* user );
		tPlayer * fGetPlayerByHwIndex( u32 hwIndex );

		bool fSetPlayerLocalUserFromScript( u32 player, u32 user );
		tUser * fGetLocalUser( u32 idx ) { return mLocalUsers[ idx ].fGetRawPtr( ); }
		u32	fLocalUserCount( ) const { return mLocalUsers.fCount( ); }
		u32 fWhichUser( tUser* user );
		u32 fWhichPlayer( tUser* user );
		
		tDynamicArray< tPlayerPtr >& fPlayers( ) { return mPlayers; }
		tPlayer* fOtherPlayer( const tPlayer* me );
		tPlayer* fAnyPlayerWithButtonsDown( const u32* pressedButtons, u32 pressedButtonsCount, const u32* notPressedButtons, u32 notPressedButtonsCount, b32 localOnly );
		tPlayer* fAnyLocalPlayerWithButtonsDown(  const u32* pressedButtons, u32 pressedButtonsCount, const u32* notPressedButtons, u32 notPressedButtonsCount );
		tPlayer* fAnyPlayerOpeningPauseMenu( );
		tPlayer* fAnyPlayerOpeningRewindMenu( );
		tPlayer* fHostPlayerOpeningRewindMenu( ) const;
		void fFindAllTeamPlayers( u32 team, tGrowableArray< tPlayerPtr > & out );
		void fFindAllEnemyPlayers( u32 team, tGrowableArray< tPlayerPtr > & out );
		u32 fFindLocalUserWithGameControlPressed( u32 gameControlId );
		u32 fFindLocalUserWithAnyPressStartInputPressed( ) const;

		void fAwardAchievementToAllPlayers( u32 achievement );
		void fAwardDeferredAchievementToAllPlayers( u32 achievement );
		void fAwardAvatarAwardToAllPlayers( u32 award );

		u32 fSingleScreenControlIndex( ) const { return mSingleScreenControl; }
		void fSetSingleScreenControlIndex( u32 index );
		tPlayer* fSingleScreenControlPlayer( ) const;
		b32 fSingleScreenCoopEnabled( ) const { return mSingleScreenCoop; }
		void fSetSingleScreenCoop( b32 enable );
		void fResetSingleScreenViewports( );

		tGameAppSession* fGameAppSession( ) { return &mGameAppSession; }

		void fRequestUserStatsById( tScript64 userId );
		tGameSessionStats* fGetRequestedUserStats( );

		void fAskPlayerToBuyGame( );
		void fFadeThroughBlack( f32 inTime, f32 holdTime, f32 outTime );
		void fShowErrorDialogBox( const char* locId, b32 cancelAble, tUser* user, u32 data, b32 pause, b32 acceptAnyInput );
		b32 fErrorDialogConfirm( u32 data, b32 confirmOrCancel, tUser* user );


		enum tErrorTypes
		{
			cNoError, //zero is reserved for success
			cCouldntSaveProfile,
			cCouldntLoadLevel,
			cCouldntLoadMenu,
			cCorruptSaveFile,
			cMissingSaveFile,
			cCouldntSaveFile,
			cNoSaveDevice,
			cProfileDeviceRemoved,
			cRewindListDeviceRemoved,
			cCorruptDLC,
			cUnLicensedDLC,
			cDLCInstalled,
			cTitleUpdateWarning
		};
		void fShowErrorDialog( tErrorTypes error, tUser* user );
		void fReallyShowErrorDialog( u32 error, tPlatformUserId userId );
		virtual void fResourceLoadFailure( tResource* resource );

		u32 fLanguage( ) const { return mLanguage; }
		u32 fRegion( ) const { return mRegion; }
		u32 fLocale( ) const { return mLocale; }
		b32 fInMultiplayerLobby( ) const { return mInMultiplayerLobby; }
		void fSetInMultiplayerLobby( b32 inLobby );

		virtual	void fOnDlcInstalled( );
		virtual void fOnFullVersionInstalled( );
		virtual void fSetAddonFlags( );

	public: // team/country conversion utility
		u32 fDefaultTeamFromCountry( u32 country ) const;
		u32 fDefaultCountryFromTeam( u32 team ) const;
		Math::tVec3f fDefaultTurretDirection( u32 team ) const;

		const Math::tMat3f* fTeamOrientation( u32 team ) const { return (mTeamOrientationSet[ team ]) ? &mTeamOrientation[ team ] : NULL; }
		void fSetTeamOrientation( tEntity* orient );

		u8 fGetLocalizedThousandsSeparator( ) const;
		u8 fGetLocalizedDecimalSeparator( ) const;

		b32 fIsOneManArmy( ) const;
		b32 fPlayerIncompatibilityCheck( ); // This should be const, but I can't because of the GameAppSession accessor

#if defined( platform_pcdx9 )
	public:
		tAppGraphicsSettings &fAppGraphicsSettings( ) { return mGraphicsSettings; }
		const tAppGraphicsSettings &fAppGraphicsSettings( ) const { return mGraphicsSettings; }
		tAppGraphicsSettings *fAppGraphicsSettingsForScript( ) { return &mGraphicsSettings; }
		virtual float fGetShadowRange() { return mGraphicsSettings.fShadowRange(); }

	protected:
		tAppGraphicsSettings mGraphicsSettings;

	private:
		bool fGetProductAndVersionFromResource( );
		const tLocalizedString &fGetProductName( ) const;
		const tLocalizedString &fGetProductVersion( ) const;

		tLocalizedString mProductName;
		tLocalizedString mProductVersion;
#endif

	private:
		static void fExportScriptInterface( tScriptVm& vm );
		virtual void fConfigureApp( tStartupOptions& opts, tPlatformStartupOptions& platformOpts );
		virtual void fFillOutScreenCreateOpts( Gfx::tScreenCreationOptions& screenCreateOpts );
		virtual void fStartupApp( );
		virtual void fExportGameScriptInterfaces( tScriptVm& vm );
		virtual void fLoadPermaLoadedGameResources( );
		void fLoadLocResources( );
		virtual void fSetupPostEffects( );
		virtual void fStartupUsers( );
		virtual void fPreview( const tFilePathPtr& previewPath );
		virtual void fReplay( const tFilePathPtr& replayPath );
		virtual void fStartGame( );
		virtual void fTickApp( );
		virtual void fOnUserSignInChange( u32 userMask );
		virtual void fOnSystemUiChange( b32 sysUiVisible );
		virtual void fOnGameInviteAccepted( u32 localHwIndex );
		virtual void fOnMuteListChanged( );
		virtual void fOnPartyMembersChanged( );
		virtual void fOnInputDevicesChanged( u32 connected );
		virtual void fTickCameras( f32 dt );
		virtual void fTickCanvas( f32 dt );
		virtual void fShutdownApp( );
		virtual void fPollInputDevices( );
		virtual void fPreLoadFilePackages( );
		void fConfigureAppPlatform( tStartupOptions& opts, tPlatformStartupOptions& platformOptsOut );

		void fSetupTracers( );
		void fSetupDebris( );
		void fSetupLeaderboardData( );
		virtual void fStartupSfx( );
		
		void fTickSaveUI( float dt );

		void fForEachPlayerFromScript( Sqrat::Function& func, Sqrat::Object& obj );
		b32  fJoinInviteSession( );
		void fFireInviteEvent( );
		bool fHasInviteFromScript( ) { return !mGameInvite.fNull( ); }
		void fClearInvite( );

		void fGatherUserSignInStates( );
		void fDrawStats( ) const;

		void fSetSecondLocalUserFromScript( tUser* user );
		void fSetPlayerFromUser( u32 playerIndex, tUser* user );
		tLocalizedString fGetLocalLevelName( u32 mapType, u32 index );
		bool fIsUserHostFromScript( tUser* user ) const;
		bool fIsRewindEnabledFromScript( ) const;
		bool fReportBug( );

	private:
		static tGameApp* gGameApp;
		b32 mGamePauseOverride; //Set true if the game itself wants to be paused, even when not paused
	private:
		tRefCounterPtr<tGamePostEffectManager>	mPostEffectMgr;
		tDynamicArray<tPlayerPtr>	mPlayers;
		tPlayerPtr					mFrontEndPlayer;
		//tUserPtr					mSecondLocalUser;
		tLevelLogic*				mCurrentLevel;
		tRefCounterPtr< tSaveGame > mCurrentSaveGame;
		b32							mSpawningCurrentLevel;
		b32							mNextLevelQueued;
		u32							mDifficultyOverride;
		tLevelLoadInfo				mCurrentLevelLoadInfo, mNextLevelLoadInfo;
		tFixedArray<tResourcePtr,cDataTableCount> mDataTables;
		tStringHashDataTableFile	mUnitsShared;
		tFixedArray<tStringHashDataTableFile,GameFlags::cCOUNTRY_COUNT>	mUnitsPhysics;
		tStringHashDataTableFile	mFocusItems;
		tStringHashDataTableFile	mTracers;
		tStringHashDataTableFile	mCharacterProps;
		tStringHashDataTableFile	mWeapons;
		tStringHashDataTableFile	mCargo;
		tFixedArray<tResourcePtr,cGlobalScriptCount> mGlobalScripts;
		tGrowableArray<FX::tTracerTrailDef> mTracerTrailDefs;
		tFixedArray<tResourcePtr,cDebrisPiecesCount> mDebrisPieces;
		tFixedArray<tDebrisLogicDef,GameFlags::cDEBRIS_TYPE_COUNT> mDebrisLogicDefs;		
		tResourcePtr				mCannonAimingTexture;
		tResourcePtr				mDefaultDecalTexture;
		tFixedArray<tResourcePtr,cRtsCursorTextureIDCount> mRtsCursorTexture;
		tFixedArray<tFilePathPtr,GameFlags::cBUILD_SITE_COUNT> mUpgradeClockPath;
		tGrowableArray<tResourcePtr> mLevelResources;
		Gui::tSaveUIPtr				mSaveUI;
		tGameAppSession				mGameAppSession;
		tCachedScoreWriterSession   mCachedScoreWriterSession;
		tFixedArray<Math::tMat3f, GameFlags::cTEAM_COUNT>	mTeamOrientation;
		tFixedArray<b32, GameFlags::cTEAM_COUNT>			mTeamOrientationSet;
		tGrowableArray<tTimedCallback> mCallbacks;
		tResourcePtr				mTrajectoryTargetVisual;
		Gui::tCanvasPtr				mBlackBar;

		tApplicationStatePtr		mDelayedLoadScreen;
		Logic::tEventContextPtr		mSysUiEventContext;

		tFixedArray<tUserSigninInfo, tUser::cMaxLocalUsers> mUserSignInStates;

		b32							mAskPlayerToBuy;
		b32							mSingleScreenCoop;
		u32							mSingleScreenControl; //0 or 1

		tGrowableArray<tRefCounterPtr<tSaveGameRewindPreview>> mRewindSaves;
		tSaveGameRewindPreview* fRewindPreview( u32 waveIndex );
		u32							mLanguage;
		u32							mRegion;
		u32							mLocale;
		b32							mInMultiplayerLobby;
		b32							mLoadingFrontEndBecauseProfileCouldntSave;
		b32							fUserProfileConfirm( tUser* user, b32 confirm, b32 saveOnSuccess );
		b32							mLaunchedDeviceSelectionUI;
		b32							mHasEverReachedFrontEnd;
		b32							mLevelNameReadyToRead;
		u32							mDeathLimiter;

		tGrowableArray< tPlayerPtr > mPlayersToRelease;

		tDLCEnumerator				mDLCEnumerator;

		Sqrat::Function				mAskPlayerToBuyCallback;
		u32							mDLCMusicPushed; //keep track of which version of the music we're playing

		b32							mSimulationBegan;
	};

}

#endif//__tGameApp__
