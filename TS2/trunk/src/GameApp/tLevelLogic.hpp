#ifndef __tLevelLogic__
#define __tLevelLogic__
#include "Gui/tCanvas.hpp"
#include "tShapeEntity.hpp"
#include "tWaveManager.hpp"
#include "tVersusWaveManager.hpp"
#include "tScreenSpaceHealthBarList.hpp"
#include "tDialogBox.hpp"
#include "tPathEntity.hpp"
#include "Logic/tGoalDriven.hpp"
#include "tRationTicketUI.hpp"
#include "tUnitLogic.hpp"

namespace Sig
{
	struct tSaveGameData;
	class tGoalBoxLogic;
	class tTurretLogic;

	namespace Gfx
	{
		class tFollowPathCamera;
	}
	class tTutorialEvent : public Logic::tEventContext
	{
		define_dynamic_cast( tTutorialEvent, Logic::tEventContext );
		define_class_pool_new_delete( tTutorialEvent, 32 )
	public:
		u32 mEventID;
		// Set these if they require a specific value in fCompare
		u32 mEventValue;
		u32 mCurrentUnitID;
		u32 mEventUnitType;
		b32 mShellCaming;
		tEntity* mEntity;
		tPlayer* mPlayer;
		tStringPtr mPlatformName;
		tStringPtr mWeaponID;
		u32 mCombo;
		f32 mExtra;
		u32 mFlags;
		tUnitLogic* mPlayerKillerLogic; //if this is set then the player's current unit is the killer

		tTutorialEvent( u32 id = ~0, u32 value = ~0, u32 currentUnitID = ~0, b32 shellCaming = false, u32 eventUnitType = ~0, tEntity* ent = NULL, tPlayer* player = NULL )
			: mEventID( id )
			, mEventValue( value )
			, mCurrentUnitID( currentUnitID )
			, mEventUnitType( eventUnitType )
			, mShellCaming( shellCaming )
			, mEntity( ent )
			, mPlayer( player )
			, mCombo( 1 )
			, mExtra( 0.0f )
			, mFlags( 0 )
			, mPlayerKillerLogic( NULL )
		{ }

		static tTutorialEvent fConstruct( u32 id, u32 value ) { return tTutorialEvent( id, value ); }

		// Expected to be called on the fired event, and other = the stored event
		b32 fCompare( const tTutorialEvent* other ) const 
		{ 
			return other 
				&& mEventID == other->mEventID 
				&& (other->mEventValue == ~0 || mEventValue == other->mEventValue)
				&& (other->mCurrentUnitID == ~0 || mCurrentUnitID == other->mCurrentUnitID)
				&& (other->mEventUnitType == ~0 || mEventUnitType == other->mEventUnitType)
				&& (!other->mShellCaming || mShellCaming == other->mShellCaming)
				&& (!other->mPlatformName.fExists( ) || mPlatformName == other->mPlatformName); 
		}

		void fSetFlag( u32 flag ) { mFlags = fSetBits( mFlags, 1 << flag ); }
		b32 fCheckFlag( u32 flag ) const { return fTestBits( mFlags, 1 << flag ); }

		static tTutorialEvent* fConvert( Logic::tEventContext* context ) { return context->fDynamicCast<tTutorialEvent>( ); }
	};

	class tLevelLogic : public tLogic, public Logic::tGoalDriven
	{
		define_dynamic_cast( tLevelLogic, tLogic );

	public:
		tLevelLogic( );
		virtual ~tLevelLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual Logic::tGoalDriven* fQueryGoalDriven( ) { return this; }
		virtual b32 fHandleLogicEvent( const Logic::tEvent& e );
	public:

		enum tLevelProgression
		{
			cLevelProgressionIntro,
			cLevelProgressionPlaying,
			cLevelProgressionMinigame,
			cLevelProgressionStageBoss,
			cLevelProgressionStageVictory,
			cLevelProgressionStageDefeat
		};


		b32 fGatherEntitySaveData( tSaveGameData& data, tSaveGameRewindPreview& preview );
		tSaveGameRewindPreview* fGetAllTurretData( );

		void fSetRootMenuFromScript( const Sqrat::Object& rootMenuScriptObj );
		const Sqrat::Object& fGetRootMenuFromScript( ) const { return mRootMenuScriptObject; }

		Gui::tCanvasPtr& fRootMenu( ) { return mRootMenu; }
		const Gui::tCanvasPtr& fRootMenu( ) const { return mRootMenu; }

		void fRegisterSaveableEntity( tEntity* entity );
		void fUnRegisterSaveableEntity( tEntity* entity );

		void fRegisterCameraPoint( tEntity* entity );
		u32 fCameraPointCount( ) const { return mCameraPoints.fCount( ); }
		tEntity* fCameraPoint( u32 i ) { return i < mCameraPoints.fCount( ) ? mCameraPoints[ i ].fGetRawPtr( ) : 0; }

		void fRegisterBombDropperBox( tEntity* entity );
		tShapeEntity* fBombDropperBox( u32 team ) { return mBombDropperBox[ team ].fGetRawPtr( ); }

		void fRegisterCameraBox( tEntity* entity );
		tShapeEntity* fCameraBox( u32 team ) { return mCameraBoxes[ team ].fGetRawPtr( ); }

		void fRegisterCameraBlocker( tEntity* entity );
		u32 fCameraBlockerCount( ) const { return mCameraBlockers.fCount( ); }
		tShapeEntity* fCameraBlocker( u32 i ) { return i < mCameraBlockers.fCount( ) ? mCameraBlockers[ i ].fGetRawPtr( ) : 0; }

		b32 fWaypointsProcessed( ) const { return mWaypointsProcessed; }
		void fRegisterCameraGround( tEntity* entity );
		u32 fCameraGroundCount( ) const { return mCameraGrounds.fCount( ); }
		tShapeEntity* fCameraGround( u32 i ) { return i < mCameraGrounds.fCount( ) ? mCameraGrounds[ i ].fGetRawPtr( ) : 0; }

		void fRegisterPathStart( tEntity* startPoint );
		const tGrowableArray<tPathEntityPtr>&	fPathStarts( ) const { return mPathStarts; }
		const tGrowableArray<b32>&				fPathEndsInGoal( ) const { return mPathEndsInGoal; }
		const tGrowableArray<b32>&				fPathHasDropPoints( ) const { return mPathHasDropPoints; }

		void fRegisterTrenchStart( tEntity* startPoint );
		const tGrowableArray<tPathEntityPtr>& fTrenchStarts( ) const { return mTrenchStarts; }
		f32 fTrenchLength( u32 index );

		void fRegisterExitGenStart( tEntity* startPoint );
		const tGrowableArray<tPathEntityPtr>& fExitGenStarts( ) const { return mExitGenStarts; }

		void fRegisterContextPathStart( tEntity* startPoint );
		const tGrowableArray<tPathEntityPtr>& fContextPathStarts( ) const { return mContextPathStarts; }

		void fRegisterFlightPathStart( tEntity* startPoint );
		const tGrowableArray<tPathEntityPtr>& fFlightPathStarts( ) const { return mFlightPathStarts; }

		void fRegisterNamedWaypoint( tEntity* waypoint );
		const tGrowableArray<tPathEntityPtr>& fNamedWayPoints( ) const { return mNamedWaypoints; }
		tEntity* fNamedPathPoint( const tStringPtr& name ) const;

		void fRegisterNamedObject( tEntity* obj );
		b32 fRemoveNamedObject( const tStringPtr& name );
		const tGrowableArray<tEntityPtr>& fNamedObjects( ) const { return mNamedObjects; }
		tEntity* fNamedObject( const tStringPtr& name ) const;

		void fRegisterGoalBox( tEntity* goalBox );
		u32 fGoalBoxCount( ) const { return mGoalBoxes.fCount( ); }
		tEntity* fGoalBox( u32 ithGoalBox ) { return ithGoalBox < mGoalBoxes.fCount( ) ? mGoalBoxes[ ithGoalBox ].fGetRawPtr( ) : 0; }
		
		void fRegisterBuildSiteSmall( tEntity* entity );
		void fRegisterBuildSiteLarge( tEntity* entity );
		const tGrowableArray<tShapeEntityPtr>& fBuildSitesSmall( ) const { return mBuildSitesSmall; }
		const tGrowableArray<tShapeEntityPtr>& fBuildSitesLarge( ) const { return mBuildSitesLarge; }
		const tGrowableArray<tEntityPtr>& fBuildSiteRootsSmall( ) const { return mBuildSiteRootsSmall; }
		const tGrowableArray<tEntityPtr>& fBuildSiteRootsLarge( ) const { return mBuildSiteRootsLarge; }
		tBuildSiteLogic* fBuildSiteNamed( const tStringPtr& name ) const;

		void fRegisterGenerator( tEntity* generator );
		u32 fGeneratorCount( ) const { return mGenerators.fCount( ); }
		tEntity* fGenerator( u32 ithGenerator ) { return ithGenerator < mGenerators.fCount( ) ? mGenerators[ ithGenerator ].fGetRawPtr( ) : 0; }

		void fRegisterTeleporter( tEntity* teleporter ) { mTeleporters.fPushBack( tEntityPtr( teleporter ) ); }
		const tGrowableArray<tEntityPtr>& fTeleporters( ) const { return mTeleporters; }

		u32 fSpecialLevelObjectCount( ) const { return mSpecialLevelObjects.fCount( ); }
		tEntity* fSpecialLevelObject( u32 ithSpecialLevel ) { return ithSpecialLevel < mSpecialLevelObjects.fCount( ) ? mSpecialLevelObjects[ ithSpecialLevel ].fGetRawPtr( ) : 0; }

		void fAddArtilleryBarrageSpawnPt( tEntity* entity );
		void fAddArtilleryBarragePt( tEntity* entity );
		const tGrowableArray<tEntityPtr>& fArtilleryBarragePtsForTeam( u32 team ) { return mArtilleryPtsByTeam[ team ]; }
		tEntity* fArtilleryBarrageSpawnPt( const tStringPtr& name );

		void fAddBarrageDropPt( tEntity* entity );
		tEntity* fBarrageDropPt( u32 team, const Math::tVec3f& pos, const tStringPtr& name );

		void fRegisterToyBoxAlarm( tEntity* entity );
		const tGrowableArray<tProximityLogic*>& fToyBoxAlarms( u32 team );

		void fRegisterCameraPathStart( tEntity* entity );
		const tGrowableArray<tPathEntityPtr>& fCameraPathStarts( ) const { return mCameraPathStarts; }
		
		void fRegisterUseableUnit( tEntity* entity );
		void fUnregisterUseableUnit( tEntity* entity );
		u32 fUseableUnitCount( ) const { return mUseableUnits.fCount( ); }
		tEntity* fUseableUnit( u32 i ) { return i < mUseableUnits.fCount( ) ? mUseableUnits[ i ].mEntity.fGetRawPtr( ) : 0; }
		tEntity* fUseableUnit( const tStringPtr& name ) const;
		const Math::tMat3f* fUsableUnitInitialTransform( tEntity* ent ) const;

		void fRegisterUnit( const tUnitLogic& unitLogic );
		void fUnregisterUnit( const tUnitLogic& unitLogic );
		const tGrowableArray<tEntityPtr>& fUnitList( GameFlags::tUNIT_TYPE unitType ) const { return mUnitsByType[ unitType ]; }
		const tGrowableArray<tEntityPtr>& fAllUnitList( ) const { return mAllUnits; }


		void fRegisterAirSpace( tEntity* entity );
		const tShapeEntityPtr& fAirSpace( u32 team ) { return mAirSpace[ team ]; }

		void fRegisterWaterLevel( tEntity* entity ) { sigassert( entity ); mWaterLevel = entity->fObjectToWorld( ).fGetTranslation( ).y; }
		f32 fWaterLevel( ) const { return mWaterLevel; }

		void fRegisterSpecialLevelObject( tEntity* entity );

		void fOnSimulationBegin( );
		void fOnLevelLoadEnd( );
		void fOnLevelUnloadBegin( );
		void fPushStandardCameras( );
		void fSetPlayerMoney( u32 money );

		const Math::tAabbf& fLevelBounds( ) const { return mLevelBounds; }
		f32 fMaxLevelDimension( ) const { return mMaxLevelDimension; } //the cached diagonal of the level bounds.

		void fOnWaveListFinished( tWaveList* waveList );
		void fOnWaveKilled( tWaveDesc* wave );
		void fOnNoWavesOrEnemiesLeft( );
		void fOnPathCameraFinished( Gfx::tFollowPathCamera& camera );
		tWaveList* fAddWaveList( const tStringPtr& waveListName );
		tWaveList* fWaveList( u32 i );
		u32 fWaveListCount( ) const;
		void fSetUIWaveList( const tStringPtr& listName );
		void fShowEnemiesAliveList( b32 show );
		void fShowWaveListUI( b32 show );

		const Gui::tScreenSpaceHealthBarListPtr& fScreenSpaceHealthBarList( ) const { return mScreenSpaceHealthBarList; }

		void fOnTicketsCountChanged( u32 currentTickets, tPlayer* player );
		void fWaveListScriptCall( const tStringPtr& function );

		void fSetCurrentLevelProgression( u32 progress );
		u32 fCurrentLevelProgrssion( ) const { return mLevelProgression; }
		
		//set this true when the final game state has been determined, but not necessarily set yet (Delayed defeat/victory)
		b32 fVictoryOrDefeat( ) const { return mMatchEnded; }

		void	 fQuitNetGameEarly( );
		void     fSetDefeatedPlayer( tPlayer* player );
		void     fSetVictoriousPlayer( tPlayer* player );
		void	 fMatchEnded( bool endedEarly = false );
		void	 fLevelExited( );
		void	 fSetMiniGameScore( u32 score, u32 extraMiniIndex, u32 leaderBoard );
		tPlayer* fDefeatedPlayer( ) const { return mDefeatedPlayer.fGetRawPtr( ); }
		tPlayer* fVictoriousPlayer( ) const { return mVictoriousPlayer.fGetRawPtr( ); }
		b32		 fIsVictorious( tPlayer* player ) { return !mVictoriousPlayer.fNull( ) && player != NULL && mVictoriousPlayer == player; }

		f32 fUnitTimeMultiplier( ) const { return mUnitTimeMultiplier; }
		void fSetUnitTimeMultiplier( f32 mult ) { mUnitTimeMultiplier = mult; }
		f32 fQueryCurrentUnitTimeScale( tUnitLogic& unitLogic ) const;

		f32 fGroundHeight( ) const { return mGroundHeight; }
		void fSetGroundHeight( f32 height ) { mGroundHeight = height; }

		void fAddOffensiveWave( const tOffensiveWaveDesc& waveDesc );
		b32  fLaunchOffensiveWave( tOffensiveWaveDesc& waveDesc, tPlayer* player );
		tWaveManagerPtr& fWaveManager( ) { return mWaveManager; }
		tVersusWaveManagerPtr& fVersusWaveManager( ) { return mVersusWaveManager; }
		tWaveList* fDisplayedWaveList( ) const { return mWaveManager ? mWaveManager->fDisplayedWaveList( ): NULL; }
		tWaveList* fCurrentOrLastDisplayedWaveList( ) const { return mWaveManager ? mWaveManager->fCurrentOrLastDisplayedWaveList( ): NULL; }
		Gui::tRationTicketUIPtr& fRationTicketUI( ) { return mRationTicketUI; }
		Sqrat::Object fRationTicketUIScript( ) { return mRationTicketUI->fCanvas( ).fScriptObject( ); }

		u32 fLevelNumber( ) const { return mLevelNumber; }
		u32 fDlcNumber( ) const { return mDlcNumber; }
		u32 fMapType( ) const { return mMapType; }

		// Tutorial stuff
		void fHandleTutorialEvent( const tTutorialEvent& event );
		void fHandleTutorialEventScript( u32 event );
		void fSuspendTutorial( b32 suspend );
		void fAddMiniGameTime( f32 time );

		b32 fDisableVehicleRespawn( ) const { return mDisableVehicleRespawn; }
		b32 fDisableUpgrade( ) const { return mDisableUpgrade; }
		b32 fDisableRepair( ) const { return mDisableRepair; }
		b32 fDisableSell( ) const { return mDisableSell; }
		b32 fDisableUse( ) const { return mDisableUse; }
		b32 fFreeUpgrades( ) const { return mFreeUpgrades; }  //upgrades dont require you to have enough money
		b32 fDisableBarrage( ) const { return mDisableBarrage; } //no barrage
		b32 fDisableOvercharge( ) const { return mDisableOvercharge; }
		b32 fForceWeaponOvercharge( ) const { return mForceWeaponOvercharge; }
		b32 fHideKillHoverText( ) const { return mHideKillHoverText; } //hide text that floats up after kills
		b32 fDisableRewind( ) const { return mDisableRewind; }
		void fSetDisableRewind( b32 set );
		b32 fAllowCoopTurrets( ) const { return mAllowCoopTurrets; }
		f32 fPlatformPrice( ) const { return mPlatformPrice; }
		void fPlatformPurchased( tEntity* platform );
		b32  fEnablePlatformLocking( ) const { return mEnablePlatformLocking; }
		void fUpdatePlatformGlow( u32 size );
		b32 fAllowTutorialDebugSkip( ) const;
		b32 fDisableLaunchArrows( ) const { return mDisableLaunchArrows; }

		void fSetDisableSell( b32 set ) { mDisableSell = set; mTurretOptionsChanged = 2; }
		void fSetDisableRepair( b32 set ) { mDisableRepair = set; mTurretOptionsChanged = 2; }
		void fSetDisableUpgrade( b32 set ) { mDisableUpgrade = set; mTurretOptionsChanged = 2; }
		void fSetDisableUse( b32 set ) { mDisableUse = set; mTurretOptionsChanged = 2; }
		b32 fTurretMenuChanged( ) const { return mTurretOptionsChanged > 0; }	

		void fNukeGloablAudioState( ); // call this when a nuke goes off
		void fExtraMiniGamePoints( f32 points );

		b32 fLockPlaceMenuUpUntilBuild( ) const { return mLockPlaceMenuUpUntilBuild; }
		void fResetLockPlaceMenuUpUntilBuild( ) { mLockPlaceMenuUpUntilBuild = false; }
		b32 fDisablePlaceMenu( ) const { return mDisablePlaceMenu; }
		b32 fDisableVehicleInput( ) const { return mDisableVehicleInput; }
		b32 fDisableRandomPickups( ) const { return mDisableRandomPickups; }
		b32 fContinuousCombo( ) const { return mContinuousCombo; }
		b32 fDisablePropMoney( ) const { return mDisablePropMoney; }
		b32 fDisableOverkills( ) const { return mDisableOverkills; }
		b32 fUseMinigameScoring( ) const { return mUseMinigameScoring; }
		b32 fDisableQuickSwitch( ) const { return mDisableQuickSwitch; }
		b32 fIsDisplayCase( ) const { return mIsDisplayCase; }
		b32 fDisableBonusHoverText( ) const { return mDisableBonusHoverText; }
		b32 fDisableComboText( ) const { return mDisableComboText; }
		b32 fAllowSpeedBonus( ) const { return mAllowSpeedBonus; }
		b32 fAllyLaunchArrows( ) const { return mAllyLaunchArrows; }
		b32 fDisableRTSRings( ) const { return mDisableRTSRings; }

		void fImpactText( const tStringPtr& locID, tPlayer& player );
		

		b32 fSaveFirstWave( ) const { return mSaveFirstWave; }
		

		void fFocusTarget( tEntity* entity, const tStringPtr& key );

		b32 fIsTrial( ) const { return fLevelNumber( ) == 0 && fMapType( ) == GameFlags::cMAP_TYPE_CAMPAIGN; }

		f32 fSurvivalTimer( ) const { return mSurvivalLevelTime; }

		f32 fMiniGameTime( ) const { return mMiniGameTime; }
		void fSetMiniGameTime( f32 val ) { mMiniGameTime = val; }
		f32 fMiniGameCurrentScore( ) const { return mMiniGameCurrentScore; }
		void fSetMiniGameCurrentScore( f32 val ) { mMiniGameCurrentScore = val; }

		// Extra stuff
		tGrowableArray<tEntityPtr> mPlayerSpawn;
		void fRegisterPlayerSpawn( tEntity* e ) { mPlayerSpawn.fPushBack( tEntityPtr( e ) ); mExtraMode = true; }
		tEntity* fPlayerSpawn( const tStringPtr& name ) const;
		b32 mExtraMode; //registering a player spawn point will set this
		b32 fExtraMode( ) const { return mExtraMode; }

		tLevelScores* fLevelScores( const tPlayer* player ) const { return player->fProfile( )->fGetLevelScores( fMapType( ), fLevelNumber( ) ); }

		void fAwardDecoration( u32 index ); //level specific ration ticket index
		void fFailDecoration( u32 index);
		b32 fDecorationActive( u32 index );
		void fSetRationTicketHasProgress( u32 index ); //call this if you're going to call the next function
		b32 fDecorationHasProgress( u32 index, tUser* user ) const;
		void fDecorationProgress( u32 index, f32 progress, f32 max );
		Sqrat::Object fDecorationStatus( u32 index, tUser* user );

		// Display case
		void fIncCameraAngle( f32 angle );
		void fIncWorldAngle( f32 angle );
		void fApplyAngles( );
		b32 fOverDisplayCaseTurret( ) const;
		b32 fCanExitDisplayCase( );
		void fToggleDisplayCaseCountry( );

		void fIncRankProgress( tPlayer* player, s32 val );

		void fIncCoopAssists( );

		u32  fRewindCount( ) const { return mRewindCount; }
		void fSetRewindCount( u32 count ) { mRewindCount = count; }
		void fSetPlatformPrice( f32 price ) { mPlatformPrice = price; }

	protected:
		virtual void fActST( f32 dt );
		virtual void fThinkST( f32 dt );
		virtual void fCoRenderMT( f32 dt );
	private:
		void fRegisterBuildSite( tEntity* entity, tGrowableArray<tShapeEntityPtr>& buildSites, tGrowableArray<tEntityPtr>& buildSiteRoots, GameFlags::tBUILD_SITE buildSiteType );
		void fRenderPath( const tEntity* pathEnt );
		void fComputeLevelBounds( );
		void fOnRegisterSpecialLevelObject( tEntity* levelObj );
		void fOnRegisterBoss( tEntity* levelObj );
		void fProcessWaypoints( );
		void fAttachWaypointLogics( );
		void fLinkGeneratorParents( );
		void fAttachLogicRecursively( tPathEntity* path );
		void fFindGoalPathEnds( );
		b32  fFindAndRegisterPathEndWithGoalLogic( tPathEntity* path, b32& hasDropPoints );
		void fCheckAchievements( tPlayer& player );

	public:	
		void fGamePurchased( );

	private:
		void fSpawnShowcaseTurrets( );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		struct tUsableUnit
		{
			tEntityPtr mEntity;
			Math::tMat3f mInitialTransform;
			b32 operator == ( tEntity* ent ) const { return mEntity == ent; }
		};

		Gui::tCanvasPtr mRootMenu;
		Sqrat::Object	mRootMenuScriptObject;
		tGrowableArray<tEntityPtr> mSaveableEntsFromLevel;
		tGrowableArray<tEntityPtr> mSaveableEntsDynamic;
		tFixedArray<tShapeEntityPtr, GameFlags::cTEAM_COUNT>	mCameraBoxes;
		tFixedArray<tShapeEntityPtr, GameFlags::cTEAM_COUNT>	mBombDropperBox;		
		tGrowableArray<tEntityPtr> mCameraPoints;
		tGrowableArray<tShapeEntityPtr> mCameraBlockers;
		tGrowableArray<tShapeEntityPtr> mCameraGrounds;
		tGrowableArray<tShapeEntityPtr> mBuildSitesSmall;
		tGrowableArray<tShapeEntityPtr> mBuildSitesLarge;
		tGrowableArray<tEntityPtr> mBuildSiteRootsSmall;
		tGrowableArray<tEntityPtr> mBuildSiteRootsLarge;
		tGrowableArray<tPathEntityPtr>	mPathStarts;
		tGrowableArray<b32>				mPathEndsInGoal;
		tGrowableArray<b32>				mPathHasDropPoints;
		tGrowableArray<tPathEntityPtr>	mTrenchStarts;
		tGrowableArray<f32>				mTrenchLengths;
		tGrowableArray<tPathEntityPtr>	mExitGenStarts;
		tGrowableArray<tPathEntityPtr>	mContextPathStarts;
		tGrowableArray<tPathEntityPtr>	mFlightPathStarts;
		tGrowableArray<tPathEntityPtr>	mCameraPathStarts;
		tGrowableArray<tPathEntityPtr>  mNamedWaypoints;
		tGrowableArray<tEntityPtr>		mNamedObjects;
		tGrowableArray<tEntityPtr> mGoalBoxes;
		tGrowableArray<tEntityPtr> mGenerators;
		tGrowableArray<tEntityPtr> mTeleporters;
		tGrowableArray<tUsableUnit> mUseableUnits;
		tGrowableArray<tEntityPtr> mLevelChunks;
		tGrowableArray<tEntityPtr> mSpecialLevelObjects;
		tFixedArray<tGrowableArray<tEntityPtr>, GameFlags::cUNIT_TYPE_COUNT> mUnitsByType;
		tGrowableArray<tEntityPtr> mAllUnits;
		tFixedArray<tGrowableArray<tEntityPtr>, GameFlags::cTEAM_COUNT> mArtilleryPtsByTeam;
		tFixedArray<tGrowableArray<tEntityPtr>, GameFlags::cTEAM_COUNT> mBarrageDropPtsByTeam;
		tGrowableArray<tEntityPtr> mArtillerySpawnPts;
		tFixedArray<tGrowableArray<tEntityPtr>, GameFlags::cTEAM_COUNT> mToyBoxAlarmEnts;
		tFixedArray<tGrowableArray<tProximityLogic*>, GameFlags::cTEAM_COUNT> mToyBoxAlarms;
		tFixedArray<tShapeEntityPtr, GameFlags::cTEAM_COUNT> mAirSpace;
		tPlayerPtr mVictoriousPlayer;
		tPlayerPtr mDefeatedPlayer;

		tWaveManagerPtr mWaveManager;
		tVersusWaveManagerPtr mVersusWaveManager;
		Gui::tScreenSpaceHealthBarListPtr mScreenSpaceHealthBarList;
		Gui::tRationTicketUIPtr mRationTicketUI;


		struct tRationTicketData
		{
			b32 mHasProgress;
			b32 mFailed;
			f32 mCurrent;
			f32 mMax;

			tRationTicketData( )
				: mHasProgress( 0 )
				, mFailed( 0 )
				, mCurrent( 0.f )
				, mMax( 0.f )
			{ }
		};

		tFixedArray< tFixedArray<tRationTicketData, 2>, 2> mRationTickets;

		Math::tAabbf mLevelBounds;
		f32 mMaxLevelDimension;
		u32 mLevelProgression;

		b8 mMatchEnded;
		b8 mSlowMoUnits;
		b8 mWaypointsProcessed;
		b8 pad0;

		f32 mGroundHeight;
		f32 mWaterLevel;

		u32 mLevelNumber;
		u32 mMapType;
		u32 mPreviousGlow;
		u32 mDlcNumber;

		// Tutorial shiz
		u32 mOnlyPlaceThisUnit;
		b32 mOnlyPlaceSmallUnits; 
		b32 mDisableSell;	
		b32 mDisableRepair; 
		b32 mDisableUpgrade;
		b32 mDisableUse;
		u32 mTurretOptionsChanged;
		b32 mFreeUpgrades;  //upgrades dont require you to have enough money
		b32 mDisableOvercharge;
		b32 mForceWeaponOvercharge;
		b32 mHideKillHoverText; //no floating text over kills
		b32 mDisableBarrage; //no overchage or barrage
		b32 mDisableVehicleRespawn; //no overchage or barrage
		b32 mEnablePlatformLocking;
		b32 mDisableLaunchArrows;
		b32 mLockPlaceMenuUpUntilBuild;
		b32 mDisableRewind;
		b32 mAllowCoopTurrets;
		b32 mDisablePlaceMenu;
		b32 mDisableVehicleInput; //for the count down in vehicle minigames
		b32 mDisableRandomPickups;
		b32 mSaveFirstWave;
		b32 mContinuousCombo;
		b32 mDisablePropMoney;
		b32 mDisableOverkills;
		b32 mUseMinigameScoring;
		b32 mDisableQuickSwitch;
		b32 mIsDisplayCase;
		b32 mLightUpOnlyTutorialPlatforms;
		b32 mAlwaysShowUnitControls;
		b32 mDisableBonusHoverText;
		b32 mDisableComboText;
		b32 mDisableRTSRings;
		b32 mAllowSpeedBonus;
		b32 mAllyLaunchArrows;

		f32 mPlatformPrice;
		f32 mSurvivalLevelTime;
		f32 mUnitTimeMultiplier;

		f32 mNukeMixNeutralTimer;
		f32 mNeutralTimer;
		f32 mNeutralTimerTime;
		b32 mTutorialSuspended;
		f32 mMiniGameTime;
		f32 mMiniGameCurrentScore;
		tRefCounterPtr<tSaveGameRewindPreview> mTurretData;

		u32 mCoOpAssists;
		u32 mRewindCount;
		f32 mStopWavesTimer;

		// Showcase stuff
		tEntityPtr mBackDrop;
		tEntityPtr mUSAFlag;
		tEntityPtr mUSSRFlag;
		tRtsCamera* mBackDropCamera;
		f32 mCameraAngle;
		f32 mWorldAngleRate;
		s32 mWorldAngle;
		u32 mDisplayCaseCountry;
		u32 mWaitOneFrameToSwitchCountry;
		Math::tVec3f mOriginalDir;
		Math::tMat3f mPivotToSpawn;
		Math::tVelocityDamped mAngleDamper;
		tGrowableArray< tTurretLogic* > mTurrets;
		tGrowableArray< tEntityPtr >	mTurretTargets;
		Math::tMat3f fMakeTurretMatrix( u32 index ) const;
	};

}

#endif//__tLevelLogic__
