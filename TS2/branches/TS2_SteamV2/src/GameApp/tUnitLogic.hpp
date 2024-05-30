#ifndef __tUnitLogic__
#define __tUnitLogic__
#include "Logic/tGoalDriven.hpp"
#include "tDataTableFile.hpp"
#include "tShapeEntity.hpp"
#include "tRadialMenu.hpp"
#include "tGameApp.hpp"
#include "Audio/tSource.hpp"
#include "tTurretUpgradeIndicator.hpp"
#include "tWeaponStation.hpp"
#include "tLevelEventHandler.hpp"
#include "Gfx/tTintStack.hpp"
#include "tInUseIndicator.hpp"
#include "tGeneratorWave.hpp"

namespace Sig
{
	class tPlayer;
	class tUnitPath;
	class tMeshEntity;
	class tUnitLogic;
	class tPathEntity;
	class tSmokeDestroyer;

	namespace Gui
	{
		class tHealthBar;
	}


	struct tEntitySaveData;

	///
	/// \brief Represents and provides base functionality for most any individual actor in the game, 
	/// including infanctry, vehicles, turrets, bosses, generators, etc. Think "unit" in the most general RTS sense.
	class tUnitLogic : public tLogic
		, public Logic::tGoalDriven
	{
		define_dynamic_cast( tUnitLogic, tLogic );
	public:
		enum tCreationType
		{
			cCreationTypeFromLevel,
			cCreationTypeGhost,
			cCreationTypeFromBuildSite,
			cCreationTypeFromGenerator,
			cCreationTypeCount
		};

		enum tTintTypes
		{
			cTintTypeRTSCursorFlash,
			cTintTypeSelected,
			cTintTypeDamage,
			cTintTypeCount
		};

	public:
		static void fExportScriptInterface( tScriptVm& vm );
		tFilePathPtr fResourcePath( ) const;
	public:
		tUnitLogic( );
		virtual ~tUnitLogic( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual b32  fHandleLogicEvent( const Logic::tEvent& e );
		virtual void fActST( f32 dt );
		virtual Logic::tGoalDriven* fQueryGoalDriven( ) { return this; }
		virtual Math::tVec4f fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const;

	public: // misc dynamic flags
		virtual b32 fShouldSelect( ) const { return fSelectionEnabled( ); }
		b32  fHasSelectionFlag( ) const { return fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_SELECTABLE ); }
		b32  fSelectionEnabled( ) const;
		void fEnableSelection( b32 enable );
		void fSetSelectionOverride( b32 override ) { mSelectionOverride = override; fUpdateSelectionTint( ); }
		
		void fDontStopBullets( );
		
		b32  fUnderUserControl( ) const { return mUnderUserControl; }
		b32  fUnderUserControlScript( ) { return mUnderUserControl; }
		void fSetUnderUserControl( b32 userControl );

		void fAddCanvasObject( const Gui::tScriptedControlPtr& obj );

	public:
		const tStringHashDataTable& fUnitSharedTable( ) const;

		void				fQueryEnums( );
		u32					fTeam( ) const { return mTeam; }
		u32					fCountry( ) const { return mCountry; }
		u32					fUnitCountry( ) const { return mUnitCountry; }
		void				fSetUnitCountry( u32 country ) { mUnitCountry = country; }
		u32					fUnitType( ) const { return mUnitType; }
		u32					fUnitID( ) const { return mUnitID; }
		const tStringPtr&	fUnitIDString( ) const { return mUnitIDString; }
		u32					fUnitIDAlias( ) const { return mUnitIDAlias; }
		void				fSetUnitIDAlias( u32 alias ) { mUnitIDAlias = (GameFlags::tUNIT_ID)alias; }
		void				fSetPersonalityType( u32 type );
		
		
		const tStringPtr&		fAudioID( ) const;
		const Audio::tSourcePtr& fAudioSource( ) const { return mAudio; }

		u32						fLogicType( ) const { return mLogicType; }
		u32						fPickup( ) const { return mPickup; }
		void					fSetPickup( u32 pickUp );
		void					fPickedUp( );

		virtual void			fRegisterUnit( );
		void					fSetWave( tGeneratorWave* wave ) { mWave.fReset( wave ); }
		const tWavePtr&			fWave( ) const { return mWave; }
		b32						fWaveDisabledAIFire( ) const { return mWave && mWave->fDisableAIFire( ); }
		
	protected:
		void	fSetTeam( u32 team );
		void	fSetCountry( u32 country );
		void	fSetUnitID( u32 unitID );
		void	fSetLogicType( u32 logicType );

	public:

		f32  fTimeScale( ) const { return mTimeScale * mLevelTimeScale; }
		void fSetTimeScale( f32 timeScale ) { if( !mDisableTimeScale ) mTimeScale = timeScale; }
		b32  fDisableTimeScale( ) const { return mDisableTimeScale; }
		void fSetDisableTimeScale( b32 disable ) { mDisableTimeScale = disable; if( mDisableTimeScale ) mTimeScale = 1.f; }
		f32  fCurrentHitPoints( ) const { return mHitPoints; }
		void fSetHitPoints( f32 hp ) { mHitPoints = hp;  }
		void fSetHitPointModifier( f32 modifier ) { mHitPointsModifier = modifier; }
		void fResetHitPoints( );
		f32	 fHealthPercent( ) const;
		void fSetTakesDamage( b32 val ) { mTakesDamage = val; }
		b32	 fTakesDamage( ) const { return mTakesDamage; }
		
		enum tUnitSharedColumns
		{
			cUnitSharedResourcePath,
			cUnitSharedWaveIconPath,
			cUnitSharedUnitType,
			cUnitSharedSize,
			cUnitSharedSellValue,
			cUnitSharedRepairCost,
			cUnitSharedPurchaseCost,
			cUnitSharedDestroyValueCasual,
			cUnitSharedDestroyValueNormal,
			cUnitSharedDestroyValueHard,
			cUnitSharedDestroyValueElite,
			cUnitSharedDestroyValueGeneral,
			cUnitSharedDestroyValueVersus,
			cUnitSharedDestroyComboMeterValue,
			cUnitSharedHitPointsCasual,
			cUnitSharedHitPointsNormal,
			cUnitSharedHitPointsHard,
			cUnitSharedHitPointsElite,
			cUnitSharedHitPointsGeneral,
			cUnitSharedHitPointsVersus,
			cUnitSharedOverkillDamage,
			cUnitSharedHealthBarSize,
			cUnitSharedUserModeDamageMultiplier,
			cUnitSharedUserModeDamageMultiplierVersus,
			cUnitSharedUseCamRotateSpeed,
			cUnitSharedUseCamRotateDamping,
			cUnitSharedScopeRotateSpeed,
			cUnitSharedScopeRotateDamping,
			cUnitSharedUseCamFOV,
			cUnitSharedScopeFOV,
			cUnitSharedAIRotateSpeed,
			cUnitSharedConstrainToQuadrant,
			cUnitSharedAutoNearFarInterpolate,
			cUnitSharedOverChargeComboCost,
			cUnitSharedOverChargeDuration,
			cUnitSharedBatteryRechargeTime,
			cUnitSharedUpgradesTo,
			cUnitSharedUnitIDAlias,
			cUnitSharedWaveLaunchAudioSwitch,
			cUnitSharedUserHitAudio,
			cUnitSharedUserKillAudio,
			cUnitSharedOverKillAudio,
			cUnitSharedDestroyedExplosionSize,
			cUnitSharedDestroyedExplosionRate,
			cUnitSharedDestroyedExplosionPoints,
			cUnitSharedFocalPrompt,
		};

		s32 fUnitAttributeSize( ) const { return ( s32 )fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedSize ); }
		f32 fUnitAttributeMaxHitPoints( ) const;
		f32 fUnitAttributeUserModeDamageMultiplier( ) const;
		u32 fUnitAttributeSellValue( ) const { return ( u32 )fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedSellValue ); }
		u32 fUnitAttributeRepairCost( ) const { return ( u32 )fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedRepairCost ); }
		u32 fUnitAttributePurchaseCost( ) const { return ( u32 )fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedPurchaseCost ); }
		u32 fUnitAttributeDestroyValue( ) const;
		u32 fUnitAttributeOverkillDamage( ) const { return ( u32 )fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedOverkillDamage ); }
		Math::tVec4f fUnitAttributeUseCamRotSpeed( ) const { return fUnitSharedTable( ).fIndexByRowCol<Math::tVec4f>( mUnitIDString, cUnitSharedUseCamRotateSpeed ); }
		Math::tVec4f fUnitAttributeUseCamRotDamping( ) const { return fUnitSharedTable( ).fIndexByRowCol<Math::tVec4f>( mUnitIDString, cUnitSharedUseCamRotateDamping ); }
		Math::tVec4f fUnitAttributeScopeRotSpeed( ) const { return fUnitSharedTable( ).fIndexByRowCol<Math::tVec4f>( mUnitIDString, cUnitSharedScopeRotateSpeed ); }
		Math::tVec4f fUnitAttributeScopeRotDamping( ) const { return fUnitSharedTable( ).fIndexByRowCol<Math::tVec4f>( mUnitIDString, cUnitSharedScopeRotateDamping ); }
		f32 fUnitAttributeCameraFOV( ) const { return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedUseCamFOV ); }
		f32 fUnitAttributeScopeZoom( ) const { return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedScopeFOV ); }
		f32 fUnitAttributeAIRotateSpeed( ) const { return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedAIRotateSpeed ); }
		f32 fUnitAttributeOverChargeComboCost( ) const { return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedOverChargeComboCost ); }
		f32 fUnitAttributeOverChargeDuration( ) const { return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedOverChargeDuration ); }
		f32 fUnitAttributeBatteryRechargeTime( ) const { return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedBatteryRechargeTime ); }
		b32 fUnitAttributeConstrainToQuadrant( ) const { return (b32)fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedConstrainToQuadrant ); }
		f32 fUnitAttributeQuadrantConstraintAngle( ) const { return Math::fToRadians( fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedConstrainToQuadrant ) ); }
		b32 fUnitAttributeAutoNearFarInterpolate( ) const { return (b32)fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedAutoNearFarInterpolate ); }
		u32 fUnitAttributeDestroyedComboMeterValue( ) const { return (u32)fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedDestroyComboMeterValue ); }
		f32 fUnitAttributeDestroyedExplosionSize( ) const { return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedDestroyedExplosionSize ); }
		f32 fUnitAttributeDestroyedExplosionRate( ) const { return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedDestroyedExplosionRate ); }
		f32 fUnitAttributeDestroyedExplosionPoints( ) const { return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedDestroyedExplosionPoints ); }
		tStringPtr fUnitAttributeFocalPrompt( ) const { return fUnitSharedTable( ).fIndexByRowCol<tStringPtr>( mUnitIDString, cUnitSharedFocalPrompt ); }
		tStringPtr fUnitAttributeUserHitAudio( ) const { return fUnitSharedTable( ).fIndexByRowCol<tStringPtr>( mUnitIDString, cUnitSharedUserHitAudio ); }
		tStringPtr fUnitAttributeUserKillAudio( ) const { return fUnitSharedTable( ).fIndexByRowCol<tStringPtr>( mUnitIDString, cUnitSharedUserKillAudio ); }
		tStringPtr fUnitAttributeOverKillAudio( ) const { return fUnitSharedTable( ).fIndexByRowCol<tStringPtr>( mUnitIDString, cUnitSharedOverKillAudio ); }

	public: // new base methods to be implemented by derived types
		virtual void fOnRtsCursorHoverBeginEnd( b32 startStopFlag ); // called once on hover start, once on hover end
		virtual Gui::tRadialMenuPtr fCreateSelectionRadialMenu( tPlayer& player );
		virtual tLocalizedString fHoverText( ) const;
		virtual tLocalizedString fPurchaseText( ) const;
		std::string fLocKey( ) const { return fLocKey( fCountry( ), fUnitID( ) ); }
		static std::string fLocKey( u32 country, u32 unitID );
		virtual tLocalizedString fUnitName( ) const;
		virtual tLocalizedString fClassText( ) const;
		virtual tLocalizedString fPurchaseDescription( ) const;
		static tLocalizedString fUnitLocNameScript( u32 country, u32 unitID );
		static tLocalizedString fUnitLocClassScript( u32 country, u32 unitID );

		virtual b32 fCanBeUsed( ) { return false; }
		virtual b32 fTryToUse( tPlayer* player ) { return false; }
		virtual b32 fEndUse( tPlayer& player ) { return true; } //returns true if totally vacant
		virtual tRefCounterPtr<tEntitySaveData> fStoreSaveGameData( b32 entityIsPartOfLevelFile, tSaveGameRewindPreview& preview );
	
	public:
		tPlayer*		fTeamPlayer( ) { return mTeamPlayer; }
		const tPlayer*	fTeamPlayer( ) const { return mTeamPlayer; }
		void			fSetTeamPlayer( tPlayer* player, b32 setCurrent = true );
		
		b32				fIsDestroyed( ) const { return mHitPoints <= 0.f; }
		b32				fIsValidToReactToDamage( ) const { return fTakesDamage( ) && !fIsDestroyed( ) && (fCreationType( ) != cCreationTypeGhost); }

		// always shoot at turrets. otherwise only shoot at selectable stuff that is under user control.
		b32				fIsValidInTermsOfUserControl( ) const { return (fLogicType( ) == GameFlags::cLOGIC_TYPE_TURRET) || (!fHasSelectionFlag( ) || fUnderUserControl( )); }
		b32				fIsValidTarget( ) const { return fIsValidToReactToDamage( ) && fIsValidInTermsOfUserControl( ); }
		
		void			fDealDamage( const tDamageContext& damage, b32 force = false ); //force will deal the damage even if it's not a valid target
		void			fDestroy( ) { fDestroy( false ); }
		void			fDestroy( b32 force ) { tDamageContext dc; dc.fDestroyDamage( this ); fDealDamage( dc, force ); }
		void			fDestroyDontJib( ) { tDamageContext dc; dc.fDestroyDamage( this ); dc.fSetDontJib( true ); fDealDamage( dc ); }
		void			fDestroy( tUnitLogic* attacker, b32 force = false, tPlayer* player = NULL ) { tDamageContext dc; dc.fDestroyDamage( this ); dc.fSetAttacker( tDamageID( attacker, player, attacker->fTeam( ) ), GameFlags::cDAMAGE_TYPE_NONE ); fDealDamage( dc, force ); }
		void			fDestroy( GameFlags::tTEAM attackingTeam ) { tDamageContext dc; dc.fDestroyDamage( this ); dc.fSetAttacker( tDamageID( attackingTeam ) ); fDealDamage( dc ); }
		void			fDestroy( tPlayer* player, b32 force = false ) { tDamageContext dc; dc.fDestroyDamage( this ); dc.fSetAttacker( tDamageID( NULL, player, player->fTeam( ) ) ); fDealDamage( dc, force ); }

		void			fForceDestroyForScript( ) { fDestroy( true ); }

		tPlayer*		fDestroyedByPlayer( ) const { return mDestroyedByPlayer; }
		
		void			fActivateDamageTint( f32 strength = 1.f );
		f32				fAcquireTarget( ); //add one second to how long someone would need to shoot if they were shooting me

		u32				fCurrentPersistentEffect( ) const { return mPersistentEffect; }
		const tDamageContext* fCurrentPersistentDC( ) const { return mPersistentDamageContext.fGetRawPtr( ); }

	protected:
		// The way damage works:
		//  User calls deal damage. It redirects as per hitpoint linking.
		//  For each of the real damage takers compute and handle is called. This does physics stuff.
		//  If there are points to be removed from the damage, fApplyDamage is called.
		void			fComputeAndHandleDamage( f32 transferModifier, const tDamageContext& damageContext, b32 fromChild, b32 force );
		void			fApplyDamage( f32 damageAmount, const tDamageContext& damageContext, tDamageResult& result, b32 fromChild, b32 force );

	public:
		
		tCreationType	fCreationType( ) const { return mCreationType; }
		s32				fCreationTypeInt( ) const { return ( s32 ) mCreationType; }
		void			fSetCreationType( tCreationType type ) { mCreationType = type; }
		
		void			fAddToAliveList( );
		void			fRemoveFromAliveList( );

		void			fShowFocalPrompt( const tStringPtr& prompt );
		
		f32 fSelectionRadius( ) const;
		b32 fIsSelectionShape( tEntity& shape ) const;
		const tGrowableArray<tShapeEntityPtr>& fSelectionShapes( ) const { return mSelectionShapes; }
		void fComputeCollisionShapeIfItDoesntExist( );
		
		Math::tVec3f fUseCamOffset( ) const;
		Math::tVec3f fUseCamLookDir( ) const;
		Math::tVec3f fUseCamLookTarget( ) const;
		const tEntityPtr& fUseCamScopePt( ) const { return mUseCamScope; }

		f32			fUseCamZoom( ) const { return mUseCamBlendValue; }
		void		fSetUseCamZoom( f32 blendVal ) { mUseCamBlendValue = blendVal; }
		void		fUseCamZoomFromGamepad( f32 dt, f32 stickYAxis );
		Math::tVec2f fUseCamRotSpeed( ) const { return fUnitAttributeUseCamRotSpeed( ).fXY( ); }


		Math::tVec3f fDefaultFacingDirection( ) const { return tGameApp::fInstance( ).fDefaultTurretDirection( fTeam( ) ); }
		b32			fConstrainYaw( ) const { return mConstrainYaw != -1; }
		s32			fYawConstraintQuadrant( ) const { return mConstrainYaw; }
		void		fSetYawConstraintQuadrant( s32 quadrant ) { mConstrainYaw = quadrant; } //This is only meant to be used pre fOnSpawn. If used after, you must call fIncrementYawConstraint( 0 )
		virtual void fIncrementYawConstraint( s32 dir, b32 onSpawn, tPlayer* player );
		f32			fConstraintAxisAngle( ) const { sigassert( fConstrainYaw( ) ); const f32 angleStep = Math::c2Pi / mConstraintQuadrantCnt; return angleStep * mConstrainYaw; }


		f32						fDistToUnit( const Math::tVec3f& offset, const Math::tVec3f& lookDir  ) const;
		virtual Math::tVec3f	fLinearVelocity( const Math::tVec3f& localOffset ) { return Math::tVec3f::cZeroVector; }
		const Math::tVec3f&		fTargetOffset( ) const;

		virtual tUnitPath*	fUnitPath( ) { return NULL; }
		virtual b32 fPathPointReached( tPathEntity& point, const Logic::tEventContextPtr& context ) { return false; }
		void fSetWillNotEndInGoal( ) { mWillNotEndInGoal = true; }

		b32 fFirstWaveLaunch( ) const { return mFirstWaveLaunch; }
		void fSetFirstWaveLaunch( b32 val ) { mFirstWaveLaunch = val; }

		b32		fGoalBoxCheck( );
		b32		fInCameraBox( ) const;
		void	fReachedEnemyGoal( );
		b32		fHasPath( );
		void	fExplodeIntoParts( );
		void	fExplodeIntoAllParts( );
		static void fExplodeIntoPartsImp( tEntity* entity, const tFilePathPtr& debrisPath, s32 needed ); //-1 for all
		
		bool fUseDefaultEndTransition( ) { return ( mUseDefaultEndTransition != 0 ); }
		void fSetUseDefaultEndTransition( bool val ) { mUseDefaultEndTransition = ( val != 0 );  }
		void fSetAlternateDebrisMesh( const tFilePathPtr& path ) { mAlternateDebrisMesh = path; }
		void fSetAlternateDebrisMeshParent( const tStringPtr& name ) { mAlternateDebrisMeshSpawnParent = name; }
		void fSetDestroyedEffect( const tStringPtr& effectName ) { mDestroyedEffect = effectName; }

		void fSetDamageTintColor( const Math::tVec4f& tint );

		virtual void fAddCargoDropSmokePtr( tRefCounterPtr<tSmokeDestroyer>& ptr ) { }

		void fCancelAllWeaponFire( );
		void fDisableAIWeaponFire( b32 disable );
		void fSetWeaponIgnoreParent( tUnitLogic* p );

		b32  fSetOnFire( ); // Return true if first time
		void fSpawnDefaultDestroyedExplosion( );

		void fSetDisableControl( b32 disable );
		b32  fDisableControl( ) const { return mDisableControl; }

		void fSetDestroyedEnemyUnit( b32 has ) { mHasDestroyedAnEnemy = has; }
		b32  fDestroyedEnemyUnit( ) const { return mHasDestroyedAnEnemy; }

		void fSetInAlarmZone( b32 inAZ );
		b32  fInAlarmZone( ) const { return mInAlarmZone; }

		// character prop enum has given us a head
		b32 fHasHelmetProp( );

		// Replace parent with the new entity, inherit the name
		static tEntity* fSpawnReplace( tEntity* parent, const tFilePathPtr& path );

		// Extra stuff
		void fSetExtraMode( b32 mode ) { mExtraMode = mode; gExtraMode = mode; }
		b32 fExtraMode( ) const { return mExtraMode; }
		b32 mExtraMode;
		static b32 gExtraMode;
		// End extra stuff

	public: 
		b32							fHasWeaponStation( u32 index ) const { return index < fWeaponStationCount( ); }
		b32							fHasWeapon( u32 station, u32 bank ) const;
		b32							fHasWeapon( u32 station, u32 bank, u32 weapon ) const;
		tWeaponStationPtr&			fWeaponStation( u32 index );
		const tWeaponStationPtr&	fWeaponStation( u32 index ) const;
		tWeaponStation*				fWeaponStationRawPtr( u32 index );
		tWeapon*					fWeaponRawPtr( u32 station, u32 bank, u32 weapon );
		void						fCheckWeaponStationCount( u32 index );
		u32							fWeaponStationCount( ) const { return mWeaponStations.fCount( ); }
		f32							fWeaponMaxRange( ) const;
		f32							fWeaponMinRange( ) const;
		void						fEnableWeapons( b32 enable );
		void						fResetAmmo( );

		void fSetCurrentBreakState( u32 stateIndex );
		void fSetCurrentBreakStateAndSpawnDebris( u32 stateIndex, const Math::tVec3f& spawnInfluence );

		s32 fCurrentState( ) const { return mCurrentState; }
		s32 fLastState( ) const { return mLastState; }
		b32 fDeleteAfterStates( ) const { return mDeleteAfterStates; }
		b32 fNextStateIsDelete( ) const { return mDeleteAfterStates && mCurrentState == mDestroyState-1; }

		void fRegisterForLevelEvent( u32 type, Sqrat::Function func );
		void fFireLevelEvent( u32 type );
		void fCopyLevelEvents( tUnitLogic* from ) { mLevelEvents = from->mLevelEvents; }
		virtual void fShowInUseIndicator( Gui::tInUseIndicator* indicator );

		void fSetHasScreenSpaceHealthBar( b32 has ) { mHasScreenSpaceHealthBar = has; }

		void fVocDeath( );

	public:
		tUnitLogic* fHitpointLinkedUnitLogic( ) { return mHitpointLinkedUnitLogic; }
		void fSetHitpointLinkedUnitLogic( tUnitLogic* unitLogic );
		void fFindHitpointLinkedChildren( );
		u32 fHitPointLinkedChildrenCount( ) const { return mAllHitPointLinkedChildren.fCount( ); }
		tEntity* fHitPointLinkedChild( u32 index ) const { return mAllHitPointLinkedChildren[ index ].fGetRawPtr( ); }

	protected:
		void fCollectSoldiers( tGrowableArray<tEntityPtr>& entitiesOut );
		void fCollectSelectionShapes( );
		void fConfigureBreakStates( );

		void fLinkChildAudioLogics( b32 linkRTPC, b32 linkEvents );
		void fLinkChildAudioLogicsRecursive( tEntity* e, b32 linkRTPC, b32 linkEvents );
		void fScriptAudioEvent( const char *event );

		virtual void fAddHealthBar( );
		virtual void fAddToMiniMap( );
		virtual void fRegisterForSaveGame( b32 addTo );
		virtual void fReactToDamage( const tDamageContext& dc, const tDamageResult& dr );
		virtual s32 fComputeChangeState( const tDamageContext& dc, const tDamageResult& dr );
		void fChangeStatesAndSpawnDebris( const tDamageContext& dc, const tDamageResult& dr );
		virtual void fOnStateChanged( );
		void  fReapplyChangeState( );

		void fSetHealthBarColor( const Math::tVec4f& color );
		void fSetHealthBarFlashAndFill( f32 flash, f32 fill );

		void fAddPickup( u32 pickup );
		void fAddRandomPickup( );
		void fCreatePickupIcon( u32 pickup );
		void fReleasePickupIcon( );
		
		//tGrowableArray<tWeaponInstance> mWeapons;
		tGrowableArray<tWeaponStationPtr> mWeaponStations;

		tStringPtr	 mDestroyedEffect;
		tFilePathPtr mAlternateDebrisMesh;
		tStringPtr   mAlternateDebrisMeshSpawnParent;
		tEntityPtr	 mAlternateDebrisMeshSpawnParentEnt;

	protected:
		tPlayer* mTeamPlayer;
		u32 mTeam;
		u32 mCountry;
		u32 mUnitCountry;
		u32 mUnitType;
		u32 mLogicType; //to group all kinds of vehicles together regardless of their unit type
		u32 mUnitID;
		u32 mUnitIDAlias;
		tStringPtr mUnitIDString;
		tStringPtr mPersonalityTypeValue;
		tCreationType mCreationType;
		f32 mTimeScale;
		f32 mLevelTimeScale;
		f32 mHitPoints;
		f32 mHitPointsModifier;
		f32 mDamageModifier;
		f32 mDamageTransferModifier;
		Math::tVec3f mHealthBarOffset;

		u32 mPickup;
		
		b8 mUnderUserControl;
		b8 mUseDefaultEndTransition;
		b8 mDeleteAfterStates;
		s8 mConstrainYaw; //-1 means no constraint, + values are the quadrant, Set from derived types, so their type doesnt need to be dynamic casted in some cases

		b8 mTakesDamage; //override to disable taking damage, turrets use this when upgrading
		b8 mUnderCursor;
		b8 mEnumsQueried;
		b8 mAddedToAliveList;

		b8 mFirstWaveLaunch;
		b8 mWillNotEndInGoal;
		b8 mInAlarmZone;
		b8 mHasDestroyedAnEnemy; //set true if this unit has destroyed someone.

		b8 mSelectionEnable;
		b8 mSelectionOverride;
		b8 mDisableControl;
		b8 mDontLightUpChildren; //set this true if you dont want your tint to influence children with logic types

		b8 mDontJib;
		b8 mHasScreenSpaceHealthBar;
		b8 mDisableTimeScale;
		b8 mOnFire;

		tWavePtr mWave;

		// Constraints for yaw orientation.
		Math::tVec3f	mConstraintAxis;
		s32				mConstraintQuadrantCnt;
		f32				mConstraintAngle; //Max deviation from mConstraintAxis

		tGrowableArray<tShapeEntityPtr> mSelectionShapes;
		f32 mSelectionRadius;
		
		tEntityPtr	mUseCamNear;
		tEntityPtr	mUseCamFar;
		tEntityPtr	mUseCamScope;
		tEntityPtr  mUseCamTarget;
		f32			mUseCamBlendValue;

		u32 mPersistentEffect;
		f32 mPersistentTimer;
		tDamageContextPtr mPersistentDamageContext;
		FX::tFxSystemsArray mOnFireEffects;
		tStringPtr mFireEffectOverride; //optionally overridden fire effect per unit

		f32 mTargetCoolOff;
		tPlayer* mDestroyedByPlayer; //this needs to persist after OnDelete. so we know who killed it

		// Will need an array of these if multiple offsets are required
		Math::tVec3f mTargetOffset;
		tShapeEntityPtr mCollisionShape; //use for proximity to explosion computation.

		// Pickup icon
		Gui::tTurretRadialIndicatorPtr mPickupIcon;

		Gfx::tTintStack mTintStack;
		void fSetupTintStack( );
		void fStepTintStack( f32 dt );
		void fUpdateSelectionTint( );
		void fEnableRTSCursorPulse( );

	protected:// State changing
		s16	mLastState;
		s16 mDestroyState;
		s16 mCurrentState;
		s16 mFirstState; //this can be incremented, skipping states, as damage is taken, a state between first and last will be applied.
		s16 mStateOverride;
		static const s32 cInvalidStateOverride = 0xFF;

		//tMeshEntityPtr mAlternateDebrisMesh;
		void fSetStateOverride( u32 state, b32 defer );

	protected:
		tLevelEventHandlerPtr mLevelEvents;
		Audio::tSourcePtr mAudio;
		void fConfigureAudio( );
		void fConfigureBasicCharacterAudio( ); //call this in addition to the above, to set teh audio source in the middle of character and set its character switches

		// tUnitLogic that has its health linked to this one.  ie, units comprised of multiple sigmls where damaging a "child" sigml damages the "main" sigml
	protected:
		tUnitLogic*	mHitpointLinkedUnitLogic;
		tGrowableArray<tGrowableArray<tEntityPtr>> mHitPointLinkedChildren; //a list of children, sorted by weapon index.
		tGrowableArray<tEntityPtr> mAllHitPointLinkedChildren;
		void fRemoveHitPointLinkedChild( tEntity* ent ) { for( u32 i = 0; i < mHitPointLinkedChildren.fCount( ); ++i ) mHitPointLinkedChildren[ i ].fFindAndErase( ent ); }

		f32 mChildDamageTintBlend;

	private:
		// Icons for the minimap
		tGrowableArray< Gui::tScriptedControlPtr >	mCanvasObjs;

		tRefCounterPtr< Gui::tHealthBar >			mHealthBar;

	};

	// Super light object for just assigning unit tint
	class tUnitTintLogic : public tLogic
	{
		define_dynamic_cast( tUnitTintLogic, tLogic );
	public:
		virtual Math::tVec4f fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const;

		static void fAddTo( tEntityPtr& e );

	};

}

#endif//__tUnitLogic__

