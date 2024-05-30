#include "GameAppPch.hpp"
#include "tTurretLogic.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tUseTurretCamera.hpp"
#include "tSgFileRefEntity.hpp"
#include "AI/tSigAIGoal.hpp"
#include "tSaveGame.hpp"
#include "tRtsCursorLogic.hpp"
#include "tVehicleLogic.hpp"
#include "tReferenceFrameEntity.hpp"
#include "tSync.hpp"
#include "tBuildSiteLogic.hpp"

#include "Wwise_IDs.h"

using namespace Sig::Math;

namespace Sig
{
	struct tTurretSaveData : public tEntitySaveData
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tTurretSaveData, 0x262918C4 );
	public:
		tTurretSaveData( )
			: mPosition( tMat3f::cIdentity )
			, mUnitID( ~0 )
			, mUnitCountry( 0 )
			, mYawConstraint( 0 )
			, mCreationType( 0 )
			, mSelectable( false )
		{
		}

		tTurretSaveData( tTurretLogic* turret, tSaveGameRewindPreview& preview )
		{
			mUnitID = turret->fUnitIDConsideringUpgrade( );
			mUnitCountry = (u8)turret->fCountry( );
			mYawConstraint = (s8)turret->fYawConstraintQuadrant( );
			mCreationType = (u8)turret->fCreationType( );
			mSelectable = turret->fHasSelectionFlag( );

			mDeleted = !turret->fOwnerEntity( )->fSceneGraph( );
			if( !mDeleted )
			{
				mPosition = turret->fUnflippedXform( ); //can only acquire position correctly if we're in scene graph
				preview.mTurrets.fPushBack( tTurretRewindPreview( mUnitID, mPosition.fGetTranslation( ).fXZ( ), (u32)turret->fCountry( ) ) );
			}
		}

		virtual void fSpawnSavedEntity( )
		{
			if( mUnitID == ~0 )
			{
				log_warning( 0, "Invalid tTurretSaveData" );
				return;
			}

			const tFilePathPtr path = tGameApp::fInstance( ).fUnitResourcePath( (GameFlags::tUNIT_ID)mUnitID, ( GameFlags::tCOUNTRY )mUnitCountry );
			const tResourceId rid = tResourceId::fMake< tSceneGraphFile >( path );

			tSgFileRefEntity* gameTurret = NEW tSgFileRefEntity( tGameApp::fInstance( ).fResourceDepot( )->fQuery( rid ) );
			gameTurret->fSetLockedToParent( false );
			gameTurret->fSpawn( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );

			tTurretLogic* unitLogic = gameTurret->fLogicDerived< tTurretLogic >( );
			sigassert( unitLogic );
			unitLogic->fEnableSelection( true );

			unitLogic->fSetCreationType( (tUnitLogic::tCreationType)mCreationType );
			unitLogic->fQueryEnums( );
			
			if( unitLogic->fUnitAttributeConstrainToQuadrant( ) )
				unitLogic->fSetYawConstraintQuadrant( (s32)mYawConstraint );

			unitLogic->fSetupMotionBase( mPosition );

			if( mSelectable )
				gameTurret->fAddGameTags( GameFlags::cFLAG_SELECTABLE );

			unitLogic->fFindAppropriateBuildSite( );
		}
		virtual void fRestoreSavedEntity( tEntity* entity ) const
		{
			// TODO not sure if this is necessary, but basically if we need to modify the state of a pre-placed turret, then do it here
		}
		virtual void fSaveLoadDerived( tGameArchive& archive ) { fSaveLoad( archive ); }
		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			tEntitySaveData::fSaveLoadDerived( archive );
			archive.fSaveLoad( mUnitID );
			archive.fSaveLoad( mUnitCountry );
			archive.fSaveLoad( mYawConstraint );
			archive.fSaveLoad( mCreationType );
			archive.fSaveLoad( mSelectable );

			archive.fSaveLoad( mPosition );
		}

	public:
		u32 mUnitID;

		u8 mUnitCountry;
		s8 mYawConstraint;
		u8 mCreationType;
		b8 mSelectable;

		tMat3f mPosition;
	};
	register_rtti_factory( tTurretSaveData, false );

}

namespace Sig
{

	devvar( f32, Gameplay_Weapon_Turret_PowerUpSpeedUp, 2.0f );
	devvar( f32, Gameplay_Weapon_Turret_QuadrantLerpSpeed, 2.0f );
	devvar( f32, Gameplay_Turrets_UpgradeFlipLerpBegin, 0.12f );
	devvar( f32, Gameplay_Turrets_UpgradeFlipLerpEnd, 0.23f );
	devvar( f32, Gameplay_Turrets_UpgradeFlipTime, 0.1f );

	devvar( f32, Gameplay_Turrets_RandomAnimMin, 2.0f );
	devvar( f32, Gameplay_Turrets_RandomAnimMax, 5.0f );

	namespace 
	{
		f32 fRandomCharacterAnimTime( )
		{
			return sync_rand( fFloatInRange( Gameplay_Turrets_RandomAnimMin, Gameplay_Turrets_RandomAnimMax ) );
		}
	}


	tTurretLogic::tTurretLogic( )
		: mUserDirection( tVec3f::cZeroVector )
		, mUserDirectionLerp( 1.f )
		, mUpgradeID( GameFlags::cUNIT_ID_NONE )
		, mUpgrading( false )
		, mRepairing( false )
		, mFlipTurret( false )
		, mTurretFlipping( false )
		, mOnVehicle( false )
		, mDisableYawConstraintAdjust( false )
		, mQuickSwitchCamera( false )
		, mDisableSelling( false )
		, mMotionBaseInitialized( false )
		, mDontAlignOnBlendIn( false )
		, mQuickSwitchFlipIn( false )
		, mOnVehicleLogic( NULL )
		, mTimeTillNextRandomAnim( fRandomCharacterAnimTime( ) )
		, mFlipValue( 0.f, Gameplay_Turrets_UpgradeFlipLerpBegin, Gameplay_Turrets_UpgradeFlipLerpEnd, Gameplay_Turrets_UpgradeFlipTime )
		, mRepairStartPercent( 0.0 )
		, mPreviousTurretForQuickSwitch( NULL )
		, mSlaveParent( NULL )
		, mWorldLookDir( tVec3f::cZeroVector )
		, mDamageParent( false )
		, mDeployed( false )
	{
		tAnimatable::fSetLogic( this ); 
		fSetLogicType( GameFlags::cLOGIC_TYPE_TURRET );
	}
	tTurretLogic::~tTurretLogic( )
	{
	}
	void tTurretLogic::fOnSkeletonPropagated( )
	{
		tUnitLogic::fOnSkeletonPropagated( );
		tAnimatable::fListenForAnimEvents( *this );
	}
	void tTurretLogic::fOnSpawn( )
	{
		fQueryEnums( );

		if( fCreationType( ) == cCreationTypeFromLevel || fCreationType( ) == cCreationTypeFromBuildSite && !fOwnerEntity( )->fHasGameTagsAny( GameFlags::cMAP_TYPE_MINIGAME ) )
			fOwnerEntity( )->fAddGameTags( GameFlags::cFLAG_SAVEABLE ); // do this before tUnitLogic::fOnSpawn

		fCollectSoldiers( mArtillerySoldiers );

		tUnitLogic::fOnSpawn( );
		fComputeCollisionShapeIfItDoesntExist( );
		fSetupMotionBase( );

		tAnimatable::fOnSpawn( );
		fOnPause( false );

		// We are now a child of the build site, so we don't have to reserve our spot
		tBuildSiteLogic* buildSite = fOwnerEntity( )->fFirstAncestorWithLogicOfType<tBuildSiteLogic>( );
		if( buildSite )
			buildSite->fSetReserved( false );

		if( fCreationType( ) != cCreationTypeGhost )
		{
			if( fHasSelectionFlag( ) )
				tGameApp::fInstance( ).fCurrentLevel( )->fRegisterUseableUnit( fOwnerEntity( ) );

			fAddHealthBar( );
			fAddToMiniMap( );
			fAddUpgradeAndRepairIndicators( );
			if( !mOnVehicle )
				fAddEnemyTurrentIndicator( );

			for( u32 i = 0; i < mWeaponStations.fCount( ); ++i )
				mWeaponStations[ i ]->fSetAcquireTargets( !mQuickSwitchFlipIn );

			for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
			{
				const tEntityPtr& e = fOwnerEntity( )->fChild( i );
				if( e->fHasGameTagsAll( tEntityTagMask( GameFlags::cFLAG_TURRET_BASE ) ) )
				{
					mBaseObjects.fPushBack( e );
					
					if( mTurretFlipping ) // new upgrade turret, we're upside down so flip the base
					{
						tMat3f newRelXform;
						newRelXform.fSetTranslation( tVec3f( 0 ) );
						newRelXform.fOrientZAxis( tVec3f::cZAxis, -tVec3f::cYAxis );
						e->fSetParentRelativeXform( e->fParentRelative( ) * newRelXform );
					}

					e->fSetLockedToParent( false );						
				}
			}

			if( fCreationType( ) == cCreationTypeFromLevel )
				fFindAppropriateBuildSite( );
		}
		else
		{
			// preview first weapon ui
			if( fTeamPlayer( ) && fHasWeapon(0,0,0) )
				fWeaponRawPtr( 0,0,0 )->fBeginRendering( fTeamPlayer( ) );
		}

		if( fConstrainYaw( ) )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			sigassert( level );
			if( level->fIsDisplayCase( ) )
				mConstrainYaw = -1;
			else
				fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 )->fSetYawConstraint( fUserToWorld( mConstraintAxis ), mConstraintAngle );
		}

		if( fHasWeapon( 0,0,0 ) && !fWeaponRawPtr( 0,0,0 )->fInst( ).mTurretEntity )
			fWeaponRawPtr( 0,0,0 )->fSetTurretEntity( fOwnerEntity( ) );

		if( mIdleTarget )
			mUserDirection = fIdleTargetUserDir( );
		else
			mUserDirection = mConstraintAxis;

		fUpdateWorldLookDir( );

		mUpgradeID = (GameFlags::tUNIT_ID)GameFlags::fUNIT_IDValueStringToEnum( fUnitSharedTable( ).fIndexByRowCol<tStringPtr>( mUnitIDString, cUnitSharedUpgradesTo ) );

		if( mUpgradeClock ) mUpgradeClock->fReparent( *fOwnerEntity( ) );

		if( (fHasSelectionFlag( ) && (tGameApp::fInstance( ).fDifficulty( ) == GameFlags::cDIFFICULTY_ELITE))
			|| (mOnVehicleLogic && mOnVehicleLogic->fWaveDisabledAIFire( ))
			|| !tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).fChallengeTurretAI( )
			|| tGameApp::fInstance( ).fCurrentLevelDemand( )->fIsDisplayCase( ) ) 
			fWeaponStation( 0 )->fSetAcquireTargets( false );

		fConfigureAudio( );

		tGameApp& app = tGameApp::fInstance( );
		if( app.fCurrentLevelDemand( )->fIsDisplayCase( ) )
			mTintStack.fStack( ).fSetCount( 0 );
	}
	void tTurretLogic::fSetupMotionBase( )
	{
		if( !mMotionBaseInitialized )
		{
			mMotionBaseInitialized = true;

			tVehicleLogic* logicParent = fOwnerEntity( )->fFirstAncestorWithLogicOfType<tVehicleLogic>( );
			if( logicParent )
			{
				mMotionParent.fReset( logicParent->fOwnerEntity( ) );
				mOnVehicle = true;
				mOnVehicleLogic = logicParent;
				mCreationType = logicParent->fCreationType( );

				if( fCreationType( ) != cCreationTypeFromLevel && fCreationType( ) != cCreationTypeFromBuildSite )
				{
					fRegisterForSaveGame( false );
				}
			}
			else
			{
				mMotionParent.fReset( tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
			}

			tMat3f flipMat = tMat3f::cIdentity;
			if( mTurretFlipping )
			{
				f32 angle = mFlipValue.fValue( ) * cPi;
				tQuatf rotate( tAxisAnglef( tVec3f::cZAxis, angle ) );
				flipMat = tMat3f( rotate );
			}

			mMotionParentRelative = mMotionParent->fWorldToObject( ) * fOwnerEntity( )->fObjectToWorld( );
			mMotionParentRelative *= flipMat;

			const tMat3f* teamOrient = tGameApp::fInstance( ).fTeamOrientation( fTeam( ) );
			if( teamOrient )
			{
				tMat3f unRotate = *teamOrient;
				unRotate.fSetTranslation( tVec3f( 0 ) );
				unRotate = unRotate.fInverse( );
				mMotionParentRelative *= unRotate;
			}

			if( fConstrainYaw( ) )
			{
				tMat3f unRotate( tQuatf( tAxisAnglef( tVec3f::cYAxis, -fConstraintAxisAngle( ) ) ) );
				mMotionParentRelative *= unRotate;
			}	
		}
	}
	void tTurretLogic::fSetupMotionBase( const Math::tMat3f& motionParentRelative )
	{
		mMotionBaseInitialized = true;
		mMotionParent.fReset( tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
		mMotionParentRelative = motionParentRelative;
		fOwnerEntity( )->fMoveTo( fUnflippedXform( ) );

		if( fConstrainYaw( ) )
			fIncrementYawConstraint( 0, true, NULL );
	}
	void tTurretLogic::fOnDelete( )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level )
		{
			level->fUnregisterUseableUnit( fOwnerEntity( ) );
			if( fUnderUserControl( ) )
				fEjectAllPlayers( );
		}

		if( !mDeleteScriptCallback.IsNull( ) )
			mDeleteScriptCallback.Execute( );
		mDeleteScriptCallback.Release( );

		fRegisterForSaveGame( false );

		mArtillerySoldiers.fSetCount( 0 );
		mBaseObjects.fSetCount( 0 );
		mMotionParent.fRelease( );
		mSlaveParentEnt.fRelease( );
		mPlayers.fSetCount( 0 );
		Logic::tAnimatable::fOnDelete( );
		tUnitLogic::fOnDelete( );

		mIdleTarget.fRelease( );
		mUpgradeIndicator.fRelease( );
		mRepairIndicator.fRelease( );
		mEnemyTurretIndicator.fRelease( );
		mUpgradeClock.fRelease( );
	}
	void tTurretLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListAnimateMT );
			fRunListRemove( cRunListMoveST );
			fRunListRemove( cRunListThinkST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListAnimateMT );
			fRunListInsert( cRunListMoveST );
			fRunListInsert( cRunListThinkST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}
	void tTurretLogic::fActST( f32 dt )
	{
		profile( cProfilePerfTurretLogicActST );

		tUnitLogic::fActST( dt );

		if( tGameApp::fInstance( ).fSingleScreenCoopEnabled( ) && mPlayers.fCount( ) && tGameApp::fInstance( ).fSingleScreenControlPlayer( ) != fWeaponStation( 0 )->fPlayer( ) )
		{
			fWeaponStation( 0 )->fPlayer( )->fSetStepCameras( false );
			fWeaponStation( 0 )->fEndUserControl( );
			fWeaponStation( 0 )->fBeginUserControl( tGameApp::fInstance( ).fSingleScreenControlPlayer( ) );
			tGameApp::fInstance( ).fSingleScreenControlPlayer( )->fSetStepCameras( true );
		}
	}
	void tTurretLogic::fAnimateMT( f32 dt )
	{
		dt *= fTimeScale( );

		{
			profile( cProfilePerfTurretLogicAnimateMT );
			Logic::tAnimatable::fAnimateMT( dt );
		}

		fStepTintStack( dt );
	}
	void tTurretLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfTurretLogicMoveST );
		dt *= fTimeScale( );

		Logic::tAnimatable::fMoveST( dt );

		tMat3f currentMotionBase = mMotionParent->fObjectToWorld( ) * mMotionParentRelative;
		tMat3f localMove = currentMotionBase.fInverse( ) * fOwnerEntity( )->fObjectToWorld( );
		localMove.fOrientYWithZAxis( tVec3f::cYAxis, localMove.fZAxis( ) ); //this cancels out previous flipping xform

		tMat3f animDelta = tMat3f::cIdentity;
		Logic::tAnimatable::fAnimatedSkeleton( )->fApplyRefFrameDelta( animDelta );

		localMove *= animDelta;

		// Handle turret flipping, this should go into animate.
		if( mTurretFlipping )
		{
			f32 targetFlipAngle = 0.f;
			if( mFlipTurret ) targetFlipAngle = 1.f;			
			mFlipValue.fStep( targetFlipAngle, dt );

			if( !mFlipTurret && mFlipValue.fValue( ) < 0.01f )
			{
				mTurretFlipping = false;
				mQuickSwitchFlipIn = false;
			
				if( mUpgradeClock )
					mUpgradeClock->fDelete( );
				mUpgradeClock.fRelease( );

				if( mAcquireThisPlayerOnSpawn.fCount( ) )
				{
					for( u32 i = 0; i < mAcquireThisPlayerOnSpawn.fCount( ); ++i )
					{
						mAcquireThisPlayerOnSpawn[ i ].mPlayer->fLockInUnitDirect( this );
						mAcquireThisPlayerOnSpawn[ i ].mPlayer->fSetDontResetComboOnExit( false );
					}
					mAcquireThisPlayerOnSpawn.fSetCount( 0 );

					if( mPreviousTurretForQuickSwitch )
						mPreviousTurretForQuickSwitch->fOwnerEntity( )->fDelete( );
					mPreviousTurretForQuickSwitch = NULL;
				}
			}
			else
			{
				f32 angle = mFlipValue.fValue( ) * cPi;
				if( mQuickSwitchFlipIn ) angle = -angle;
				tQuatf rotate( tAxisAnglef( tVec3f::cZAxis, angle ) );
				localMove *= tMat3f( rotate );
			}
		}

		fOwnerEntity( )->fMoveTo( currentMotionBase * localMove );
	}
	void tTurretLogic::fThinkST( f32 dt )
	{
		profile( cProfilePerfTurretLogicThinkST );
		dt *= fTimeScale( );

		for( u32 ws = 0; ws < mWeaponStations.fCount( ); ++ws )
		{
			if( mOnVehicle )
			{
				tVehicleLogic* veh = fOnVehicleLogic( );
				sigassert( veh );
				mWeaponStations[ ws ]->fSetParentVelocityMT( veh );
			}

			mWeaponStations[ ws ]->fProcessST( dt );
		}

	}
	void tTurretLogic::fCoRenderMT( f32 dt )
	{
		profile( cProfilePerfTurretLogicCoRenderMT );
		dt *= fTimeScale( );

		sigassert( dt == dt );

		for( u32 ws = 0; ws < mWeaponStations.fCount( ); ++ws )
			mWeaponStations[ ws ]->fProcessMT( dt );
	}
	void tTurretLogic::fStepRandomEvent( u32 context, f32 dt )
	{
		mTimeTillNextRandomAnim -= dt;

		if( mTimeTillNextRandomAnim < 0.f )
		{
			mTimeTillNextRandomAnim = fRandomCharacterAnimTime( );

			Logic::tEvent e( GameFlags::cEVENT_RANDOM_CHARACTER_ANIM, NEW Logic::tIntEventContext( context ) );
			for( u32 i = 0; i < mArtillerySoldiers.fCount( ); ++i )
				mArtillerySoldiers[ i ]->fLogic( )->fHandleLogicEvent( e );
		}
	}
	Gui::tRadialMenuPtr tTurretLogic::fCreateSelectionRadialMenu( tPlayer& player )
	{
		//tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		//sigassert( level );

		//if( level->fIsDisplayCase( ) )
		//{
		//	// jump straight in
		//	fTryToUse( &player );
		//	return Gui::tRadialMenuPtr( );
		//}
		Gui::tRadialMenuPtr radialMenu;
		if( fUnitID( ) == GameFlags::cUNIT_ID_SNIPER_TOWER )
		{
			radialMenu = Gui::tRadialMenuPtr( NEW Gui::tRadialMenu( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptSniperTowerOptions ), player.fUser( ), player.fGameController( ) ) );
		}
		else
		{
			radialMenu = Gui::tRadialMenuPtr( NEW Gui::tRadialMenu( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptTurretOptions ), player.fUser( ), player.fGameController( ) ) );
		}

		Sqrat::Object params( Sqrat::Table( ).SetValue( "Unit", this ).SetValue( "Player", &player ) );
		Sqrat::Function( radialMenu->fCanvas( ).fScriptObject( ), "DefaultSetup" ).Execute( params );

		return radialMenu;
	}
	void tTurretLogic::fApplyMotionStateToArtillerySoldiers( const char* motionState )
	{
		f32 timeScale = fPowerUpTimeScale( );

		for( u32 i = 0; i < mArtillerySoldiers.fCount( ); ++i )
		{
			tVehiclePassengerLogic* soldierLogic = mArtillerySoldiers[ i ]->fLogicDerived<tVehiclePassengerLogic>( );
			sigassert( soldierLogic );
			tAnimatable* anim = soldierLogic->fQueryAnimatable( );
			sigassert( anim );

			Sqrat::Table table;
			table.SetValue( "Turret", this );
			const u32 ammo = fHasWeapon( 0, 0, 0 ) ? fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 )->fCurrentAmmo( ) : 0;
			table.SetValue( "AmmoCount", ammo );
			table.SetValue( "TimeScale", timeScale );
			anim->fExecuteMotionState( motionState, Sqrat::Object( table ) );
		}
	}
	void tTurretLogic::fFindAppropriateBuildSite( )
	{
		// we were placed directly in the level, so we need to look for a build site just in case
		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( levelLogic );

		const u32 unitSize = fUnitAttributeSize( );
		tShapeEntityPtr buildSite;
		f32 closestBuildSiteDist = 1.5f; //threshold
		if( unitSize <= 1 )
			buildSite = fFindAppropriateBuildSite( levelLogic->fBuildSitesLarge( ), closestBuildSiteDist );
		if( !buildSite && unitSize == 0 )
			buildSite = fFindAppropriateBuildSite( levelLogic->fBuildSitesSmall( ), closestBuildSiteDist );
		if( buildSite )
		{
			fSetCreationType( cCreationTypeFromBuildSite );
			fOwnerEntity( )->fSetLockedToParent( false );
			fOwnerEntity( )->fReparent( *buildSite );
		}
	}
	void tTurretLogic::fPlayerSpecificAudioEvent( tPlayer* player, u32 event )
	{
		player->fSoundSource( )->fSetSwitch( tGameApp::cUnitIDSwitchGroup, fAudioID( ) );
		player->fSoundSource( )->fHandleEvent( event );
	}
	void tTurretLogic::fConfigureAudio( )
	{
		if( fCreationType( ) == cCreationTypeFromBuildSite )
		{
			if( fFlipping( ) )
			{
				//upgradeing, play upgrade finished for this unit
				mAudio->fHandleEvent( AK::EVENTS::PLAY_TURRET_UPGRADE_COMPLETE );
			}
			else
				mAudio->fHandleEvent( AK::EVENTS::PLAY_TURRET_PLACE );
		}
	}
	tVec3f tTurretLogic::fIdleTargetUserDir( )
	{
		sigassert( mIdleTarget );
		tVec3f userDir = fWorldToUser( mIdleTarget->fObjectToWorld( ).fGetTranslation( ) - fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) );

		log_assert( fHasWeapon( 0, 0, 0 ), "Turrets need at least one weapon." );
		tWeapon* weap = fWeaponRawPtr( 0,0,0 );
		weap->fSetPitchAngle( fToDegrees( atan( userDir.y / userDir.fLength( ) ) ) );

		return userDir;
	}
	tVec3f tTurretLogic::fDesiredFacingUserDirection( )
	{
		log_assert( fHasWeapon( 0, 0, 0 ), "Turrets need at least one weapon." );
		tWeapon* weap = fWeaponRawPtr( 0,0,0 );

		tVec3f direction;
		if( fUnderUserControl( ) || mQuickSwitchFlipIn )
			direction = mUserDirection;
		else if( weap->fAcquireTargets( ) && weap->fHasTarget( ) )
			direction = fWorldToUser( weap->fIdealFacingDirection( ) );
		else if( mSlaveParent )
		{
			direction = mSlaveParent->fDesiredFacingUserDirection( );
		}
		else if( mIdleTarget )
		{
			direction = fIdleTargetUserDir( );
			mUserDirection = direction;
			mUserDirection.fProjectToXZAndNormalize( );
		}
		else
		{
			direction = fConstrainYaw( ) ? mConstraintAxis 
				: (fOnVehicleLogic( ) ? tVec3f::cZAxis : fWorldToUser( fDefaultFacingDirection( ) ));
		}

		f32 lenSq = direction.fLengthSquared( );
		if( fEqual( lenSq, 0.f ) )
			direction = tVec3f::cZAxis;
		else
			direction.fNormalize(  );

		return fConstrainFacingDirection( direction );
	}
	Math::tVec3f tTurretLogic::fConstrainFacingDirection( const Math::tVec3f& dir ) const
	{
		if( !fConstrainYaw( ) ) return dir;
		else
		{
			sigassert( fEqual( dir.fLengthSquared( ), 1.f ) );
			sigassert( fEqual( mConstraintAxis.fLengthSquared( ), 1.f ) );
			f32 angle = fAcos( dir.fDot( mConstraintAxis ) );
			f32 delta = angle - mConstraintAngle;
			if( delta > 0 )
			{
				//violated constraint
				tVec3f axis = dir.fCross( mConstraintAxis ).fNormalizeSafe( tVec3f::cYAxis );
				tQuatf correct( tAxisAnglef(axis, delta) );
				return correct.fRotate( dir );
			}
			else
				return dir;
		}
	}
	b32 tTurretLogic::fUpgradeMaxed( tPlayer* player ) const
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level && level->fDisableUpgrade( ) )
			return true;

		return mUpgradeID == GameFlags::cUNIT_ID_NONE;
	}
	b32 tTurretLogic::fUpgradeLocked( tPlayer* player ) const
	{
		if( mUpgradeID == GameFlags::cUNIT_ID_NONE )
			return false;

		return player->fUnitLocked( GameFlags::fUNIT_IDEnumToValueString( mUpgradeID ) );
	}
	f32 tTurretLogic::fPitchBlendValue( ) const
	{
		const tWeaponPtr& weapon = fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 );
		return fRemapZeroToOne( weapon->fDesc( ).mMinPitch, weapon->fDesc( ).mMaxPitch, weapon->fPitchAngle( ) );
	}
	f32 tTurretLogic::fSpeedBlendValue( ) const
	{
		const tWeaponPtr& weapon = fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 );
		return fRemapZeroToOne( weapon->fMinimumProjectileSpeed( ), weapon->fDesc( ).mProjectileSpeed, weapon->fProjectileSpeed( ) );
	}
	tVec2f tTurretLogic::fTurretUseCamRotSpeed( ) const
	{
		tVec2f rotSpeed;

		const tPlayerData* player = fControllingPlayerData( );

		if( player && player->mCamera )
			rotSpeed = fLerp( fUnitAttributeUseCamRotSpeed( ).fXY( ), fUnitAttributeScopeRotSpeed( ).fXY( ), player->mCamera->fScopeBlend( ) );
		else
			rotSpeed = fUnitAttributeUseCamRotSpeed( ).fXY( );

		if( fUnderUserControl( ) && fHasWeapon( 0, 0, 0 ) )
		{
			// Dampen rotate speed for continuous fire weapons that are currently firing
			if( fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 )->fIsContinuousFire( ) && fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 )->fFiring( ) )
				rotSpeed *= 0.4f;
		}

		return rotSpeed;
	}
	tVec2f tTurretLogic::fTurretUseCamDamping( ) const
	{
		tVec2f damp;

		const tPlayerData* player = fControllingPlayerData( );

		if( player && player->mCamera )
			damp = fLerp( fUnitAttributeUseCamRotDamping( ).fXY( ), fUnitAttributeScopeRotDamping( ).fXY( ), player->mCamera->fScopeBlend( ) );
		else
			damp = fUnitAttributeUseCamRotDamping( ).fXY( );

		return damp;
	}
	tShapeEntityPtr tTurretLogic::fFindAppropriateBuildSite( const tGrowableArray<tShapeEntityPtr>& buildSites, f32& closestDistance ) const
	{
		tShapeEntityPtr best;
		for( u32 i = 0; i < buildSites.fCount( ); ++i )
		{
			const f32 dist = ( buildSites[ i ]->fObjectToWorld( ).fGetTranslation( ) - fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) ).fLength( );
			if( dist < closestDistance )
			{
				closestDistance = dist;
				best = buildSites[ i ];
			}
		}
		return best;
	}
	tRefCounterPtr<tEntitySaveData> tTurretLogic::fStoreSaveGameData( b32 entityIsPartOfLevelFile, tSaveGameRewindPreview& preview )
	{
		if( mCreationType == cCreationTypeFromBuildSite || mCreationType == cCreationTypeFromLevel )
		{
			tTurretSaveData* saveData = NEW tTurretSaveData( this, preview );
			return tRefCounterPtr<tEntitySaveData>( saveData );
		}
		else
			return tRefCounterPtr<tEntitySaveData>( );
	}
	b32 tTurretLogic::fTryToUse( tPlayer* player )
	{
		sigassert( player && !player->fCurrentUnit( ) );

		if( !tGameApp::fInstance( ).fCurrentLevel( )->fAllowCoopTurrets( ) && fUnderUserControl( ) )
			return false;

		if( mRepairing )
			return false;

		if( mUpgrading )
			return false;

		if( !fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_CONTROL_BEGIN ) ) )
			return false;
		
		tUseTurretCameraPtr cam( NEW tUseTurretCamera( *player, *this ) );
		player->fSetCurrentUnitLogic( this );
		player->fPushCamera( Gfx::tCameraControllerPtr( cam.fGetRawPtr( ) ) );
		mPlayers.fPushBack( tPlayerData( player, cam ) );
		player->fSetStepCameras( !tGameApp::fInstance( ).fSingleScreenCoopEnabled( ) || tGameApp::fInstance( ).fSingleScreenControlPlayer( ) == player );


		if( mPlayers.fCount( ) == 1 )
			fBeginEndControl( true );

		if( !player->fCurrentBarrage( ).fIsNull( ) )
			player->fCurrentBarrage( ).fCodeObject( )->fEnteredBarrageUnit( this, true, player );
		
		return true;
	}
	void tTurretLogic::fBeginEndControl( b32 begin )
	{
		if( begin )
		{
			if( fOnVehicle( ) )
				fUpdateWorldLookDir( );
			fWeaponStation( 0 )->fBeginUserControl( mPlayers[ 0 ].mPlayer );
			fSetUnderUserControl( true );
			fEnableSelection( false );
			//fResetCamera( );
		}
		else
		{
			fSetUnderUserControl( false );
			fEnableSelection( true );
			fWeaponStation( 0 )->fEndUserControl( );
		}

		if( fOnVehicle( ) && fOnVehicleLogic( )->fSlaveLinkTurrentChildren( ) )
		{
			tEntity* veh = fOnVehicleLogic( )->fOwnerEntity( );
			for( u32 i = 0; i < veh->fChildCount( ); ++i )
			{
				tEntity* e = veh->fChild( i ).fGetRawPtr( );
				if( e->fDynamicCast<tReferenceFrameEntity>( ) )
					e = e->fChild( 0 ).fGetRawPtr( );

				tTurretLogic* turret = e->fLogicDerived<tTurretLogic>( );
				if( turret && turret != this )
					turret->fSetSlaveParent( begin ? this : NULL );
			}
		}
	}
	b32 tTurretLogic::fEndUse( tPlayer& player )
	{
		if( fUnderUserControl( ) )
		{
			// Set player, and then camera
			player.fSetCurrentUnitLogic( NULL );
			player.fCameraStack( ).fPopCamerasOfType<tUseTurretCamera>( );
			mPlayers.fFindAndEraseOrdered( &player );

			if( !player.fCurrentBarrage( ).fIsNull( ) )
				player.fCurrentBarrage( ).fCodeObject( )->fEnteredBarrageUnit( this, false, &player );
			
			player.fSetStepCameras( !tGameApp::fInstance( ).fSingleScreenCoopEnabled( ) || tGameApp::fInstance( ).fSingleScreenControlPlayer( ) == &player );

			if( mPlayers.fCount( ) == 0 )
			{
				fBeginEndControl( false );
				return true;
			}
		}
		else
		{
			log_warning( 0, "calling tTurretLogic::fEndUse on a turret that is not under user control" );
		}

		return false;
	}
	void tTurretLogic::fEjectAllPlayers( )
	{
		for( s32 i = mPlayers.fCount( ) -1; i >= 0; --i )
			fEndUse( *mPlayers[ i ].mPlayer );
	}
	tTurretLogic::tPlayerData* tTurretLogic::fControllingPlayerData( ) 
	{ 
		if( mPlayers.fCount( ) )
		{
			tPlayerData* p = &mPlayers[ 0 ];

			tGameApp& app = tGameApp::fInstance( );
			if( app.fSingleScreenCoopEnabled( ) )
			{
				for( u32 i = 0; i < mPlayers.fCount( ); ++i )
					if( mPlayers[ i ].mPlayer == app.fSingleScreenControlPlayer( ) )
					{
						p = &mPlayers[ i ];
						break;
					}
			}
			return p;
		}

		return NULL;
	}
	tPlayer* tTurretLogic::fControllingPlayer( )
	{ 	
		tPlayerData* data = fControllingPlayerData( );
		if( data )
			return data->mPlayer;
		return NULL;
	}
	const tTurretLogic::tPlayerData* tTurretLogic::fControllingPlayerData( ) const
	{ 
		if( mPlayers.fCount( ) )
		{
			const tPlayerData* p = &mPlayers[ 0 ];

			tGameApp& app = tGameApp::fInstance( );
			if( app.fSingleScreenCoopEnabled( ) )
			{
				for( u32 i = 0; i < mPlayers.fCount( ); ++i )
					if( mPlayers[ i ].mPlayer == app.fSingleScreenControlPlayer( ) )
					{
						p = &mPlayers[ i ];
						break;
					}
			}
			return p;
		}

		return NULL;
	}
	const tPlayer* tTurretLogic::fControllingPlayer( ) const
	{ 
		const tPlayerData* data = fControllingPlayerData( );
		if( data )
			return data->mPlayer;
		return NULL;
	}

	b32 tTurretLogic::fTryToSell( tPlayer* player )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( mRepairing || mUpgrading || mDisableSelling || (level && level->fDisableSell( )) )
		{
			// Show no upgrade symbol
			if( player->fCursorLogic( )->fDisplay( ).fUI( ) )
				player->fCursorLogic( )->fDisplay( ).fUI( )->fSetNoMoney( true, true, 0 );
			return false;
		}

		player->fAddInGameMoney( fUnitAttributeSellValue( ) );
		fPlayerSpecificAudioEvent( player, AK::EVENTS::PLAY_TURRET_SELL );
		fOwnerEntity( )->fDelete( );

		if( level ) level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_TURRET_SELL, fUnitID( ), fUnitID( ), false, fUnitType( ), fOwnerEntity( ) ) );

		return true;
	}
	b32 tTurretLogic::fShouldRepair( ) const
	{
		return !fEqual( fHealthPercent( ), 1.f );
	}
	b32 tTurretLogic::fCanRepair( ) const
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		return !(mRepairing || mUpgrading || (level && level->fDisableRepair( )));
	}
	b32 tTurretLogic::fTryToRepair( tPlayer* player )
	{
		if( !fShouldRepair( ) || !fCanRepair( ) )
		{
			// Show no upgrade symbol
			if( player->fCursorLogic( )->fDisplay( ).fUI( ) )
				player->fCursorLogic( )->fDisplay( ).fUI( )->fSetNoRepair( true, true );
			return false;
		}

		if( !player->fAttemptPurchase( fUnitAttributeRepairCost( ) ) )
			return false;

		player->fStats( ).fIncStat( GameFlags::cSESSION_STATS_UNITS_REPAIRED, 1 );
		if( fHealthPercent( ) <= 0.05f )
		{
			player->fIncrementLocationalStat( GameFlags::cSESSION_STATS_RESCUED, 1.f, fOwnerEntity( ) );

			u32 previousTotal = (u32)player->fProfile( )->fAllTimeStat( GameFlags::cSESSION_STATS_RESCUED );
			u32 ingameTotal = (u32)player->fStats( ).fStat( GameFlags::cSESSION_STATS_RESCUED );
			if( previousTotal + ingameTotal == 5 )
				player->fAwardAchievement( GameFlags::cACHIEVEMENTS_CLUTCH_REPAIRS );
		}


		mRepairing = true;
		mTakesDamage = false;
		fEnableWeapons( false );
		fPlayerSpecificAudioEvent( player, AK::EVENTS::PLAY_TURRET_REPAIR );
		fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_REPAIR_BEGIN ) );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level ) level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_TURRET_REPAIR, fUnitID( ), fUnitID( ), false, fUnitType( ), fOwnerEntity( ) ) );


		// Turn on the indicator
		if( mRepairIndicator )
		{
			mRepairIndicator->fShow( );
			mRepairIndicator->fSetPercent( 1.0f );
		}

		return true;
	}

	void tTurretLogic::fQuickSwitchTo( u32 unitID )
	{
		{
			mTurretFlipping = true;
			mFlipTurret = true;

			const tFilePathPtr nextTurretPath = tGameApp::fInstance( ).fUnitResourcePath( (GameFlags::tUNIT_ID)unitID, fCountry( ) );
			const tResourcePtr nextTurretResource = tGameApp::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< tSceneGraphFile >( nextTurretPath ) );

			mAudio->fHandleEvent( AK::EVENTS::STOP_TURRET_UPGRADE );

			if( !nextTurretResource->fLoaded( ) )
			{
				log_warning( 0, "For some reason the next turret in the upgrade chain wasn't loaded [" << nextTurretPath << "] - aborting upgrade." );
				return;
			}

			tSgFileRefEntity* gameTurret = NEW tSgFileRefEntity( nextTurretResource );
			gameTurret->fSetLockedToParent( false );
			gameTurret->fSpawn( *fOwnerEntity( )->fParent( ) );


			tMat3f xform = fOwnerEntity( )->fObjectToWorld( );
			tMat3f newRelXform;
			newRelXform.fSetTranslation( tVec3f( 0 ) );
			newRelXform.fOrientZAxis( tVec3f::cZAxis, -tVec3f::cYAxis );
			gameTurret->fMoveTo( xform * newRelXform );

			tTurretLogic* logic = gameTurret->fLogicDerived< tTurretLogic >( );
			sigassert( logic );
			gameTurret->fAddGameTags( GameFlags::cFLAG_SELECTABLE );
			logic->fSetCreationType( tUnitLogic::cCreationTypeFromBuildSite );

			for( u32 i = 0; i < mPlayers.fCount( ); ++i )
				mPlayers[ i ].mPlayer->fSetDontResetComboOnExit( true );

			logic->fYoureFlipped( this );
			logic->mAcquireThisPlayerOnSpawn.fJoin( mPlayers );
			logic->mPreviousTurretForQuickSwitch = this;
			logic->mQuickSwitchFlipIn = true;

			if( fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_DOESNT_TAKE_DAMAGE ) )
				gameTurret->fAddGameTagsRecursive( GameFlags::cFLAG_DOESNT_TAKE_DAMAGE );
		}
	}

	Sig::u32 tTurretLogic::fUpgradeCost( )
	{
		return (u32)tGameApp::fInstance( ).fUnitsSharedTable( fCountry( ) ).fIndexByRowCol<f32>( GameFlags::fUNIT_IDEnumToValueString( mUpgradeID ), cUnitSharedPurchaseCost );
	}

	b32 tTurretLogic::fTryToUpgrade( tPlayer* player )
	{
		if( !fCanUpgrade( player ) )
		{
			// Show no upgrade symbol
			if( player->fCursorLogic( )->fDisplay( ).fUI( ) )
				player->fCursorLogic( )->fDisplay( ).fUI( )->fSetNoUpgrade( true, true );
			return false;
		}

		u32 nextID = mUpgradeID;
		u32 cost = fUpgradeCost( );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( level );
		b32 freeUpgrades = level->fFreeUpgrades( );

		if( !player->fAttemptPurchase( cost, freeUpgrades ) )
			return false;

		player->fStats( ).fIncStat( GameFlags::cSESSION_STATS_UNITS_UPGRADED, 1 );
		mPurchasedBy.fReset( player );

		fDoUpgrade( nextID );

		return true;
	}

	void tTurretLogic::fDoUpgrade( u32 nextID )
	{
		mUpgradeID = nextID;
		mUpgrading = true;
		fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_UPGRADE_BEGIN ) );

		mTurretFlipping = true;
		mFlipTurret = true;
		mTakesDamage = false;
		fEnableWeapons( false );

		// Spawn an upgrade clock
		sigassert( fUnitAttributeSize( ) >= 0 );
		mUpgradeClock.fReset( fOwnerEntity( )->fSpawnChild( tGameApp::fInstance( ).fUpgradeClockPath( (GameFlags::tBUILD_SITE)fUnitAttributeSize( ) ) ) );
		Gfx::tRenderableEntity::fSetDisallowIndirectColorControllers( *mUpgradeClock, true );

		tMat3f clockParRelXform;
		clockParRelXform.fSetTranslation( tVec3f( 0 ) );
		clockParRelXform.fOrientZAxis( tVec3f::cZAxis, -tVec3f::cYAxis );
		mUpgradeClock->fSetParentRelativeXform( clockParRelXform );

		mAudio->fHandleEvent( AK::EVENTS::PLAY_TURRET_UPGRADE );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level ) 
			level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_TURRET_UPGRADE, fUnitID( ), fUnitID( ) ) );

		// Turn on the indicator
		if( mUpgradeIndicator )
		{
			mUpgradeIndicator->fShow( );
			mUpgradeIndicator->fSetPercent( 1.0f );
		}
	}
	void tTurretLogic::fUpgradeFinished( )
	{
		u32 nextID = mUpgradeID;

		const tFilePathPtr nextTurretPath = tGameApp::fInstance( ).fUnitResourcePath( nextID, fCountry( ) );
		const tResourcePtr nextTurretResource = tGameApp::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< tSceneGraphFile >( nextTurretPath ) );

		mAudio->fHandleEvent( AK::EVENTS::STOP_TURRET_UPGRADE );

		if( !nextTurretResource->fLoaded( ) )
		{
			log_warning( 0, "For some reason the next turret in the upgrade chain wasn't loaded [" << nextTurretPath << "] - aborting upgrade." );
			return;
		}

		tSgFileRefEntity* gameTurret = NEW tSgFileRefEntity( nextTurretResource );
		gameTurret->fSetLockedToParent( false );
		gameTurret->fSpawn( *fOwnerEntity( )->fParent( ) );
		gameTurret->fMoveTo( fOwnerEntity( )->fObjectToWorld( ) );

		tBuildSiteLogic* buildSite = fOwnerEntity( )->fParent( )->fLogicDerived<tBuildSiteLogic>( );
		if( buildSite )
			buildSite->fSetReserved( true );

		tTurretLogic* logic = gameTurret->fLogicDerived< tTurretLogic >( );
		sigassert( logic );

		logic->fSetCreationType( fCreationType( ) );
		if( fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_SELECTABLE ) )
			gameTurret->fAddGameTags( GameFlags::cFLAG_SELECTABLE );

		logic->fYoureFlipped( this );
		logic->mPurchasedBy = mPurchasedBy;
		if( mUpgradeClock ) 
			mUpgradeClock->fReparent( *gameTurret );

		gameTurret->fSetName( fOwnerEntity( )->fName( ) );
		u32 slo = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_SPECIAL_LEVEL_OBJECT );
		if( slo != ~0 )
			gameTurret->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_SPECIAL_LEVEL_OBJECT, slo ) );

		fOwnerEntity( )->fDelete( );

		// Turn off the indicator
		if( mUpgradeIndicator )
			mUpgradeIndicator->fHide( );
	}

	void tTurretLogic::fYoureFlipped( tTurretLogic* logic )
	{ 
		mTurretFlipping = true; 
		mFlipValue.fSetValue( 1.f ); 
		mUpgradeClock = logic->mUpgradeClock; 
		if( logic->fConstrainYaw( ) ) 
			fSetYawConstraintQuadrant( logic->fYawConstraintQuadrant( ) );
		mMotionParent = logic->mMotionParent;
		mMotionParentRelative = logic->mMotionParentRelative;
		mUserDirection = logic->mUserDirection;
		mMotionBaseInitialized = true;
	}

	void tTurretLogic::fRepairFinished( )
	{
		mRepairing = false;
		mTakesDamage = true;
		fEnableWeapons( true );
		fResetHitPoints( );
		if( mRepairIndicator )
			mRepairIndicator->fHide( );
	}
	const char* tTurretLogic::fUpgradeUnitID( )
	{
		return GameFlags::fUNIT_IDEnumToValueString( mUpgradeID ).fCStr( );
	}
	void tTurretLogic::fShowInUseIndicator( Gui::tInUseIndicator* indicator )
	{
		if( indicator )
			indicator->fShow( );
	}
	void tTurretLogic::fAddUpgradeAndRepairIndicators( )
	{
		tUserArray users;
		tGameApp& gameApp = tGameApp::fInstance( );
		for( u32 i = 0; i < gameApp.fPlayers( ).fCount( ); ++i )
		{
			tPlayer& player = *gameApp.fPlayers( )[ i ];
			if( !player.fIsActive( ) )
				continue;
			if( !player.fUser( )->fIsViewportVirtual( ) )
				users.fPushBack( player.fUser( ) );
		}

		mUpgradeIndicator.fReset( NEW Gui::tTurretRadialIndicator( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptTurretUpgradeIndicator ), users ) );
		mRepairIndicator.fReset( NEW Gui::tTurretRadialIndicator( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptTurretRepairIndicator ), users ) );

		mUpgradeIndicator->fSetObjectSpaceOffset( Math::tVec3f( 0, -3, 0 ) ); // this one is upside down
		mUpgradeIndicator->fSpawn( *fOwnerEntity( ) );

		mRepairIndicator->fSetObjectSpaceOffset( Math::tVec3f( 0, 3, 0 ) );
		mRepairIndicator->fSpawn( *fOwnerEntity( ) );
	}
	void tTurretLogic::fAddEnemyTurrentIndicator( )
	{
		if( tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).mMapType == GameFlags::cMAP_TYPE_CAMPAIGN ||
			tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).mMapType == GameFlags::cMAP_TYPE_HEADTOHEAD )
		{
			tUserArray users;
			for( u32 i = 0; i < tGameApp::fInstance( ).fPlayerCount( ); ++i )
			{
				tPlayer* player = tGameApp::fInstance( ).fGetPlayer( i );
				if( player->fCountry( ) != mCountry && player->fUser( )->fIsLocal( ) )
					users.fPushBack( player->fUser( ) );
			}
			mEnemyTurretIndicator.fReset( NEW Gui::tInUseIndicator( users ) );
			mEnemyTurretIndicator->fSetObjectSpaceOffset( Math::tVec3f( 0, 5, 0 ) );
			mEnemyTurretIndicator->fSpawn( *fOwnerEntity( ) );
			mEnemyTurretIndicator->fSetIndicator( NULL, mCountry );
			mEnemyTurretIndicator->fShow( );
		}
	}
	void tTurretLogic::fSetUpgradeProgress( f32 percent )
	{
		if( mUpgradeIndicator )
			mUpgradeIndicator->fSetPercent( percent );
	}
	void tTurretLogic::fStartRepair( )
	{
		mRepairStartPercent = fHealthPercent( );
	}
	void tTurretLogic::fSetRepairProgress( f32 percent )
	{
		mHitPoints = ( mRepairStartPercent + ( ( 1.0f - mRepairStartPercent ) * percent ) ) * fUnitAttributeMaxHitPoints( );
		if( mRepairIndicator )
			mRepairIndicator->fSetPercent( percent );
	}
	void tTurretLogic::fResetCamera( )
	{
		mCameraMovement.fReset( tVec3f::cZAxis );
	}
	void tTurretLogic::fApplyUserControl( f32 dt )
	{
		/*// kick player from turret under victory or defeat
		if( tGameApp::fInstance( ).fCurrentLevel( )->fVictoryOrDefeat( ) )
		{
			if( fTeamPlayer( ) )
			{
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_CONTROL_END ) );
				return;
			}
		}*/

		tPlayerData* data = fControllingPlayerData( );
		sigassert( data );
		tPlayer* controllingPlayer = data->mPlayer;
		const tGameControllerPtr gc = controllingPlayer->fGameController( );

		const tWeaponPtr& mainWeap = fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 );
		b32 blendedIn = data->mCamera && data->mCamera->fHasBlendedIn( );

		if( blendedIn )
		{
			if( fWeaponStation( 0 )->fWantsAdvanceTargeting( ) )
				fWeaponStation( 0 )->fProcessAdvancedTargetting( );
		}

		if( !mainWeap->fShellCaming( ) && !mDisableControl )
		{
			if( fConstrainYaw( ) && !mDisableYawConstraintAdjust )
			{
				if( gc->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_LEFT ) )
					fIncrementYawConstraint( 1, false, controllingPlayer );
				else if( gc->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_RIGHT ) )
					fIncrementYawConstraint( -1, false, controllingPlayer );
			}

			if( blendedIn )
			{

				// For non-artillery types, add the sticks together.
				tVec2f stick = gc->fAimStick( tUserProfile::cProfileTurrets );

				// Concenses is not to add sticks.
				//if( mainWeap->fDesc( ).mWeaponType == tGameApp::cWeaponDerivedTypeGun )
				//{
				//	// Move stick not automatically inverted
				//	tVec2f moveStick = controllingPlayer->fMoveStick( tUserProfile::cProfileTurrets );
				//	if( controllingPlayer->fProfile( )->fInversion( tUserProfile::cProfileTurrets ) )
				//		moveStick.y *= -1.f;
				//	stick += moveStick;
				//}

				// Clamp because of adding
				//f32 stickLen = stick.fLength( );
				//f32 clampedLen = fMin( stickLen, 1.f );

				//if( stickLen > 0.25f ) //anything > 0 and < 1.0 will work as a threshold. since the minimum for thresholding would be > 1.f
				//	stick *= clampedLen / stickLen;			

				// begin real update
				mCameraMovement.fSetSpeed( fTurretUseCamRotSpeed( ) );
				mCameraMovement.fSetDamping( fTurretUseCamDamping( ) );
				mCameraMovement.fUpdate( dt, stick );

				tWeapon* primaryWeapon = fWeaponRawPtr( 0,0,0 );
				f32 pitchAngle = fToRadians( primaryWeapon->fPitchAngle( ) );
				if( fOnVehicle( ) )
				{
					//world space update.
					mUserDirection = fWorldToUser( mWorldLookDir );
					pitchAngle = fAsin( mUserDirection.y );
					mUserDirection.y = 0.f;
				}
				
				tVec2f pitchYaw = mCameraMovement.fYawPitchVelocity( );
				f32 adjust = pitchYaw.y;
				if( primaryWeapon->fInvertPitch( ) ) //this is different than user preference y inversion
					adjust *= -1.f;
				pitchAngle += adjust;
				pitchAngle = fToDegrees( pitchAngle );

				for( u32 s = 0; s < fWeaponStationCount( ); ++s )
					for( u32 b = 0; b < fWeaponStation( s )->fBankCount( ); ++b )
						for( u32 w = 0; w < fWeaponStation( s )->fBank( b )->fWeaponCount( ); ++w )
						{
							tWeapon* weap = fWeaponRawPtr( s, b, w );
							if( weap == primaryWeapon )
								weap->fSetPitchAngle( pitchAngle );
							else
							{
								// increment addditional weapons
								f32 adjust = pitchYaw.y;
								if( weap->fInvertPitch( ) ) //this is different than user preference y inversion
									adjust *= -1.f;
								weap->fAdjustPitchAngle( fToDegrees( adjust ) );
							}
						}

				// Rotate turret entity
				const tQuatf qrot = tQuatf( tAxisAnglef( tVec3f::cYAxis, -pitchYaw.x ) );
				mUserDirection = qrot.fRotate( mUserDirection );
				mUserDirection.fNormalizeSafe( tVec3f::cZAxis );

				tVec3f newZ;
				if( fConstrainYaw( ) ) 
				{
					mUserDirection = fConstrainFacingDirection( mUserDirection );
					newZ = fNLerp( fWorldToUser( fOwnerEntity( )->fObjectToWorld( ).fZAxis( ) ), mUserDirection, mUserDirectionLerp );
					mUserDirectionLerp = fMin( mUserDirectionLerp + dt * Gameplay_Weapon_Turret_QuadrantLerpSpeed, 1.f );
				}
				else 
					newZ = mUserDirection;
				newZ.fNormalize( );

				if( fOnVehicle( ) )
					fUpdateWorldLookDir( );

				tMat3f xform = tMat3f::cIdentity;
				xform.fOrientZAxis( newZ );
				fOwnerEntity( )->fMoveTo( mMotionParent->fObjectToWorld( ) * mMotionParentRelative * xform );
			}
		}
		else
			mCameraMovement.fSetYawPitchVelocity( tVec2f::cZeroVector );

		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
			mPlayers[ i ].mPlayer->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_USING_TURRETS, dt );
	}

	void tTurretLogic::fUpdateWorldLookDir( )
	{
		tVec3f fullLocal = mUserDirection;
		fullLocal.y = 0;

		sigassert( fWeaponRawPtr( 0,0,0 ) );
		f32 weapAngle = fToRadians( fWeaponRawPtr( 0,0,0 )->fPitchAngle( ) );
		fullLocal.fSetLength( cos( weapAngle ) );
		fullLocal.y = sin( weapAngle );
		mWorldLookDir = fUserToWorld( fullLocal );
	}

	void tTurretLogic::fApplyAIControl( f32 dt )
	{
		tWeapon* weap = fWeaponRawPtr( 0,0,0 );
		sigassert( weap );

		// targetting takes precidence over slave parenting
		if( mSlaveParent && !(weap->fAcquireTargets( ) && weap->fHasTarget( )) )
		{
			const tWeaponPtr& myWeapon = fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 );
			const tWeaponPtr& theirWeapon = mSlaveParent->fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 );
			myWeapon->fSetPitchAngle( theirWeapon->fPitchAngle( ) );
		}
	}

	void tTurretLogic::fIncrementYawConstraint( s32 dir, b32 onSpawn, tPlayer* player )
	{
		tUnitLogic::fIncrementYawConstraint( dir, onSpawn, player );
		
		mUserDirection = mConstraintAxis;
		if( fEqual( mUserDirectionLerp, 1.f ) )
			mUserDirectionLerp = 0.f;

		if( !onSpawn )
		{
			fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 )->fSetYawConstraint( fUserToWorld( mConstraintAxis ), mConstraintAngle );

			if( player )
				fPlayerSpecificAudioEvent( player, AK::EVENTS::PLAY_TURRET_TURN_TOGGLE );
			else
				tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( )->fHandleEvent(AK::EVENTS::PLAY_TURRET_TURN_TOGGLE );
		}
	}

	f32 tTurretLogic::fPowerUpTimeScale( ) const
	{
		f32 timeScale = 1.0f;
		const tPlayer *tp = fControllingPlayer( );
		if( tp && tp->fOverChargeActive( ) )
			timeScale = Gameplay_Weapon_Turret_PowerUpSpeedUp;
		return timeScale;
	}

	b32 tTurretLogic::fShouldPushAimGoal( ) const
	{
		const tWeaponStationPtr& station = fWeaponStation( 0 );
		return fUnderUserControl( ) || ( station->fShouldAcquire( ) && station->fHasTarget( ) );
	}

	b32 tTurretLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_UNIT_DESTROYED:
			{
				if( fUnderUserControl( ) )
					fEjectAllPlayers( );
				if( mDamageParent )
				{
					tVehicleLogic* logicParent = fOwnerEntity( )->fFirstAncestorWithLogicOfType<tVehicleLogic>( );
					if( logicParent )
					{
						tDamageContext damageContext;	
						damageContext.fSetAttacker( GameFlags::cTEAM_NONE, GameFlags::cDAMAGE_TYPE_NONE );
						damageContext.mWorldPosition = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
						damageContext.fSetExplicit( logicParent->fUnitAttributeMaxHitPoints( ) / 4 );
						logicParent->fDealDamage( damageContext );
					}
				}
				if( mPurchasedBy )
					mPurchasedBy->fStats( ).fIncStat( GameFlags::cSESSION_STATS_TURRETS_LOST, 1.f );
			}
			break;
		case GameFlags::cEVENT_WEAPON_ACTION:
			{
				const Logic::tIntEventContext* event = e.fContext<Logic::tIntEventContext>( );
				if( event )
				{
					if( event->fInt( ) == GameFlags::cWEAPON_ACTION_FIRE )
						fWeaponStation( 0 )->fSetAIFireOverride( true );
					else if( event->fInt( ) == GameFlags::cWEAPON_ACTION_END_FIRE )
						fWeaponStation( 0 )->fSetAIFireOverride( false );
					else if( event->fInt( ) == GameFlags::cWEAPON_ACTION_ENABLE )
						fWeaponStation( 0 )->fEnable( true );
					else if( event->fInt( ) == GameFlags::cWEAPON_ACTION_DISABLE )
						fWeaponStation( 0 )->fEnable( false );
				}
			}
			break;
		case GameFlags::cEVENT_ANIMATION:
			{
				const tKeyFrameEventContext* event = e.fContext<tKeyFrameEventContext>( );
				if( event )
				{
					if( event->mEventTypeCppValue == GameFlags::cKEYFRAME_EVENT_FIRE_WEAPON )
					{
						tWeapon* weap = fWeaponRawPtr( 0, 0, 0 );
						if( weap )
						{
							if( event->mTag.fExists( ) )
							{
								char c = *event->mTag.fCStr( );
								sigassert( c >= '0' && c <= '9' );
								
								tWeaponTarget target;
								target.mMuzzleIndex = c - '0';
								weap->fFire( &target );
							}
							else
								weap->fFire( );

							weap->fEndFire( );
						}
					}
				}
			}
			break;
		}

		return tUnitLogic::fHandleLogicEvent( e );
	}

	void tTurretLogic::fDeployBegin( )
	{
		const tWeaponPtr& myWeapon = fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 );
		fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_DEPLOY_BEGIN ) );
	}
}

namespace Sig
{
	typedef AI::tDerivedLogicGoalHelper<tTurretLogic> tTurretGoalHelper;

	namespace
	{
		void fCheckUserFire( tTurretLogic* logic )
		{
			if( !logic->fFlipping( ) )
			{
				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				if( level && level->fDisableVehicleInput( ) )
					return;

				const tGameControllerPtr gc = logic->fControllingPlayer( )->fGameController( );
				tWeaponBankPtr& bank = logic->fWeaponStation( 0 )->fBank( 0 );

				b32 continuousFire = bank->fIsContinuousFire( );
				b32 fire = continuousFire ?  gc->fButtonHeld( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY ) : 
					(bank->fWantsLocks( ) ? (bank->fAcquiringLocks( ) && gc->fButtonUp( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY ) ) : gc->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY ) );

				if( !logic->fDisableControl( ) && fire && bank->fShouldFire( ) )
					logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_FIRE_BEGIN ) );
			}
		}
	}

	class tTurretReloadGoal : public AI::tSigAIGoal, public tTurretGoalHelper
	{
		define_dynamic_cast(tTurretReloadGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );
			fSetReloading( true );
		}
		virtual void fOnSuspend( tLogic* logic )
		{
			AI::tSigAIGoal::fOnSuspend( logic );
			fSetReloading( false );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
			fLogic( )->fStepRandomEvent( GameFlags::cWEAPON_STATE_RELOADING, dt );

			tWeapon* weapon = fLogic( )->fWeaponRawPtr( 0, 0, 0 );
			if( weapon )
				weapon->fSetReloadProgress( fProgress( ) );
		}
		void fSetReloading( b32 reloading )
		{
			if( fLogic( ) )
				fLogic( )->fWeaponStation( 0 )->fBank( 0 )->fSetReloadOverride( reloading );
		}
	};
	class tTurretUpgradeGoal : public AI::tSigAIGoal, public tTurretGoalHelper
	{
		define_dynamic_cast(tTurretUpgradeGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
			fLogic( )->fSetUpgradeProgress( fProgress( ) );
		}
	};
	class tTurretRepairGoal : public AI::tSigAIGoal, public tTurretGoalHelper
	{
		define_dynamic_cast(tTurretRepairGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );
			fLogic( )->fStartRepair( );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
			fLogic( )->fSetRepairProgress( fProgress( ) );
		}
	};
	class tTurretUseGoal : public AI::tSigAIGoal, public tTurretGoalHelper
	{
		define_dynamic_cast(tTurretUseGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );
		}

		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );

			tWeaponStationPtr& station = fLogic( )->fWeaponStation( 0 );
			if( !fLogic( )->fUnderUserControl( ) && !station->fHasTarget( ) )
				fLogic( )->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_TARGET_LOST ) );
		}
	};
	class tTurretIdleBaseGoal : public AI::tSigAIGoal, public tTurretGoalHelper
	{
		define_dynamic_cast(tTurretIdleBaseGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );

			tWeaponBankPtr& bank = fLogic( )->fWeaponStation( 0 )->fBank( 0 );
			if( fLogic( )->fUnderUserControl( ) )
			{
				const tGameControllerPtr gc = fLogic( )->fControllingPlayer( )->fGameController( );
				if( !fLogic( )->fDisableControl( ) && bank->fWantsLocks( ) && gc->fButtonHeld( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY ) )
					bank->fFire( ); //already wants to acquire locks
			}
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );

			tWeaponBankPtr& bank = fLogic( )->fWeaponStation( 0 )->fBank( 0 );

			if( fLogic( )->fUnderUserControl( ) )
			{
				const tGameControllerPtr gc = fLogic( )->fControllingPlayer( )->fGameController( );
				if( !fLogic( )->fDisableControl( ) && bank->fWantsLocks( ) && gc->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY ) )
				{
					// locks will be "acquiring" right now.
					bank->fFire( );
				}
				else
					fCheckUserFire( fLogic( ) );
			}
			else
			{
				if( bank->fShouldFire( ) )
					logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_FIRE_BEGIN ) );
			}

			if( bank->fNeedsReload( ) )
				logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_RELOAD_START ) );
		}
	};
	class tTurretIdleGoal : public tTurretIdleBaseGoal
	{
		define_dynamic_cast(tTurretIdleGoal, tTurretIdleBaseGoal);
	public:
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			tTurretIdleBaseGoal::fOnProcess( goalPtr, logic, dt );
			fLogic( )->fStepRandomEvent( GameFlags::cWEAPON_STATE_IDLE, dt );

			tWeaponStationPtr& station = fLogic( )->fWeaponStation( 0 );

			if( fLogic( )->fShouldPushAimGoal( ) )
				fLogic( )->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_TARGET_ACQUIRED ) );
		}
	};
	class tTurretAimGoal : public tTurretIdleBaseGoal
	{
		define_dynamic_cast(tTurretAimGoal, tTurretIdleBaseGoal);
	public:
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			tTurretIdleBaseGoal::fOnProcess( goalPtr, logic, dt );
			fLogic( )->fStepRandomEvent( GameFlags::cWEAPON_STATE_AIMING, dt );
		}
	};
	class tTurretOneShotFireGoal : public AI::tSigAIGoal, public tTurretGoalHelper
	{
		define_dynamic_cast(tTurretOneShotFireGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			tWeaponBankPtr& bank = fLogic( )->fWeaponStation( 0 )->fBank( 0 );

			if( !bank->fShouldFire( ) )
			{
				fMarkAsComplete( );
				return;
			}

			AI::tSigAIGoal::fOnActivate( logic );

			fLogic( )->fApplyMotionStateToArtillerySoldiers( "FireOneShot" );

			if( !fLogic( )->fUnderUserControl( ) )
			{
				bank->fFire( );

				if( bank->fWantsLocks( ) )
				{
					//if we want locks and have targets, add a lock
					const tEntityPtr& target = bank->fWeapon( 0 )->fAITarget( );
					if( target ) 
						bank->fAddLock( tWeaponTargetPtr( NEW tWeaponTarget( (u32)target, tEntityPtr( target ), NULL, GameFlags::cUNIT_TYPE_NONE, tVec2f::cZeroVector ) ) );
				}

				bank->fEndFire( );
			}
			else 
			{
				if( bank->fWantsLocks( ) )
				{
					// Fire already called, acquiring locks
					bank->fEndFire( );
				}
				else
				{
					bank->fFire( );
					//Endfire is called from main control goal when gamepad trigger is released
				}
			}
		}
		virtual void fOnSuspend( tLogic* logic )
		{
			if( fLogic( ) && fLogic( )->fWeaponStation( 0 )->fBank( 0 )->fNeedsReload( ) )
				fLogic( )->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_RELOAD_START ) );

			AI::tSigAIGoal::fOnSuspend( logic );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
			fLogic( )->fStepRandomEvent( GameFlags::cWEAPON_STATE_FIRING, dt );

			if( fLogic( )->fUnderUserControl( ) && fLogic( )->fWeaponRawPtr( 0, 0, 0 )->fRapidFire( ) )
			{
				// allow interrupt
				fCheckUserFire( fLogic( ) );
			}
		}
	};
	class tTurretContinuousFireGoal : public AI::tSigAIGoal, public tTurretGoalHelper
	{
		define_dynamic_cast(tTurretContinuousFireGoal, AI::tSigAIGoal);
	public:
		b32 mSpinningUp;

		tTurretContinuousFireGoal( )
			: mSpinningUp( false )
		{ }

		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );

			if( !mSpinningUp )
				fLogic( )->fWeaponRawPtr( 0,0,0 )->fSetSpinUpPercentage( 1.0f );
		}
		virtual void fOnSuspend( tLogic* logic )
		{
			AI::tSigAIGoal::fOnSuspend( logic );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );

			fLogic( )->fStepRandomEvent( GameFlags::cWEAPON_STATE_FIRING, dt );

			if( !fStillShouldFire( logic ) )
				fMarkAsComplete( );

			if( mSpinningUp )
				fLogic( )->fWeaponRawPtr( 0,0,0 )->fSetSpinUpPercentage( fProgress( ) );

			if( fLogic( )->fWeaponStation( 0 )->fBank( 0 )->fNeedsReload( ) )
			{
				fMarkAsComplete( );
				logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_RELOAD_START ) );
			}
		}

		b32 fStillShouldFire( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );

			if( fLogic( )->fFlipping( ) )
				return false;

			if( fLogic( )->fUnderUserControl( ) )
			{
				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				if( level && level->fDisableVehicleInput( ) )
					return false;

				const tGameControllerPtr gc = fLogic( )->fControllingPlayer( )->fGameController( );
				if( !gc->fButtonHeld( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY ) )
					return false;
			}
			else
			{
				if( !fLogic( )->fWeaponStation( 0 )->fBank( 0 )->fShouldFire( ) )
					return false;
			}

			return true;
		}
	};
	class tTurretContinuousFireActiveGoal : public tTurretContinuousFireGoal
	{
		define_dynamic_cast(tTurretContinuousFireActiveGoal, tTurretContinuousFireGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );

			if( fStillShouldFire( logic ) )
			{
				tWeaponBankPtr& bank = fLogic( )->fWeaponStation( 0 )->fBank( 0 );
				if( !bank->fAnimationDriven( ) )
					bank->fFire( );
			}
			else
				fClearMotionState( );

			tTurretContinuousFireGoal::fOnActivate( logic );
		}
		virtual void fOnSuspend( tLogic* logic )
		{
			tTurretContinuousFireGoal::fOnSuspend( logic );

			if( fLogic( ) && fLogic( )->fWeaponStationCount( ) ) 
			{
				tWeaponBankPtr& bank = fLogic( )->fWeaponStation( 0 )->fBank( 0 );
				if( !bank->fAnimationDriven( ) )
					bank->fEndFire( );
			}
		}
	};
	class tTurretControlGoal : public AI::tSigAIGoal, public tTurretGoalHelper
	{
		define_dynamic_cast(tTurretControlGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			tWeaponStationPtr& station = fLogic( )->fWeaponStation( 0 );

			if( fLogic( )->fUnderUserControl( ) )
			{
				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				tPlayer* player = fLogic( )->fControllingPlayer( );
				const tGameControllerPtr gc = player->fGameController( );
				const tWeaponPtr& weap = station->fBank( 0 )->fWeapon( 0 );
				
				log_assert( fLogic( )->fHasWeapon( 0, 0, 0 ), "Turret is missing its weapon." );

				if( !level->fDisableVehicleInput( ) )
				{
					b32 overCharge = player->fOverChargeActive( );
					if( level && level->fForceWeaponOvercharge( ) )
						overCharge = true;

					weap->fInst( ).mOverCharged = overCharge; 

					if( station->fHasBank( 1 ) && !fLogic( )->fDisableControl( ) )
					{
						const tWeaponBankPtr& weap2 = station->fBank( 1 );
						for( u32 i = 0; i < weap2->fWeaponCount( ); ++i )
							weap2->fWeapon( i )->fInst( ).mOverCharged = player->fOverChargeActive( );

						weap2->fProcessUserInput( );
					}

					if( !weap->fIsContinuousFire( ) && !gc->fButtonHeld( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY ) && !station->fBank( 0 )->fWantsLocks( ) )
						station->fEndFire( ); //End fire is used to end shell cam.
				}
				else
					fLogic( )->fCancelAllWeaponFire( );


				fLogic( )->fApplyUserControl( dt );

				tFireEventList events;
				station->fAccumulateFireEvents( events );
				for( u32 e = 0; e < events.fCount( ); ++e )
					tWeaponStation::fApplyRumbleEvent( events[ e ], *fLogic( )->fControllingPlayer( ) );
			}
			else
			{
				tWeaponPtr weap = station->fBank( 0 )->fWeapon( 0 );
				weap->fInst( ).mOverCharged = false;
				fLogic( )->fApplyAIControl( dt );
			}

			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}
	};

	static void fExportTurretGoalsScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tTurretUseGoal, AI::tSigAIGoal, Sqrat::NoCopy<tTurretUseGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("TurretUseGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tTurretIdleGoal, AI::tSigAIGoal, Sqrat::NoCopy<tTurretIdleGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("TurretIdleGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tTurretAimGoal, AI::tSigAIGoal, Sqrat::NoCopy<tTurretAimGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("TurretAimGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tTurretReloadGoal, AI::tSigAIGoal, Sqrat::NoCopy<tTurretReloadGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("TurretReloadGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tTurretUpgradeGoal, AI::tSigAIGoal, Sqrat::NoCopy<tTurretUpgradeGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("TurretUpgradeGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tTurretRepairGoal, AI::tSigAIGoal, Sqrat::NoCopy<tTurretRepairGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("TurretRepairGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tTurretControlGoal, AI::tSigAIGoal, Sqrat::NoCopy<tTurretControlGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("TurretControlGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tTurretContinuousFireGoal, AI::tSigAIGoal, Sqrat::NoCopy<tTurretContinuousFireGoal> > classDesc( vm.fSq( ) );
			classDesc
				.Func(_SC("StillShouldFire"),	&tTurretContinuousFireGoal::fStillShouldFire)
				.Var(_SC("SpinningUp"),			&tTurretContinuousFireGoal::mSpinningUp)
				;
			vm.fRootTable( ).Bind(_SC("TurretContinuousFireGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tTurretContinuousFireActiveGoal, tTurretContinuousFireGoal, Sqrat::NoCopy<tTurretContinuousFireActiveGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("tTurretContinuousFireActiveGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tTurretOneShotFireGoal, AI::tSigAIGoal, Sqrat::NoCopy<tTurretOneShotFireGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("TurretOneShotFireGoal"), classDesc);
		}
	}
}


namespace Sig
{
	void tTurretLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tTurretLogic, tUnitLogic, Sqrat::NoCopy<tTurretLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("UpgradeMaxed"),		&tTurretLogic::fUpgradeMaxed)
			.Func(_SC("UpgradeLocked"),		&tTurretLogic::fUpgradeLocked)
			.Func(_SC("EndUse"),			&tTurretLogic::fEndUse)
			.Func(_SC("QuickSwitchTo"),		&tTurretLogic::fQuickSwitchTo)
			.Func(_SC("TryToUse"),			&tTurretLogic::fTryToUse)
			.Func(_SC("TryToSell"),			&tTurretLogic::fTryToSell)
			.Func(_SC("TryToRepair"),		&tTurretLogic::fTryToRepair)
			.Func(_SC("TryToUpgrade"),		&tTurretLogic::fTryToUpgrade)
			.Func(_SC("DoUpgrade"),			&tTurretLogic::fDoUpgrade)
			.Func(_SC("DeployBegin"),		&tTurretLogic::fDeployBegin)
			.Prop(_SC("SellingDisabled"),	&tTurretLogic::fSellingDisabled, &tTurretLogic::fSetSellingDisabled)
			.Prop(_SC("CanRepair"),			&tTurretLogic::fCanRepair)
			.Prop(_SC("ShouldRepair"),		&tTurretLogic::fShouldRepair)
			.Prop(_SC("UpgradeCost"),		&tTurretLogic::fUpgradeCost)
			.Prop(_SC("UpgradeID"),			&tTurretLogic::fUpgradeUnitID)
			.Var(_SC("UpgradeIDVar"),		&tTurretLogic::mUpgradeID)
			.Func(_SC("UpgradeFinished"),	&tTurretLogic::fUpgradeFinished)
			.Func(_SC("RepairFinished"),	&tTurretLogic::fRepairFinished)
			.Prop(_SC("PowerUpTimeScale"),  &tTurretLogic::fPowerUpTimeScale)
			.Func(_SC("ApplyMotionStateToSoldiers"), &tTurretLogic::fApplyMotionStateToArtillerySoldiers)
			.Prop(_SC("DisableYawConstraintAdjust"), &tTurretLogic::fDisableYawConstraintAdjust, &tTurretLogic::fSetDisableYawConstraintAdjust)
			.Prop(_SC("QuickSwitchCamera"), &tTurretLogic::fQuickSwitchCamera, &tTurretLogic::fSetQuickSwitchCamera)
			.Prop(_SC("ShouldPushAimGoal"), &tTurretLogic::fShouldPushAimGoal)
			.Prop(_SC("IdleTarget"),		&tTurretLogic::fIdleTarget, &tTurretLogic::fSetIdleTarget)
			.Prop(_SC("DeleteCallback"),	&tTurretLogic::fDeleteCallback, &tTurretLogic::fSetDeleteCallback)
			.Prop(_SC("DamageParent"),		&tTurretLogic::fGetDamageParent, &tTurretLogic::fSetDamageParent)
			.Prop(_SC("Deployed"),			&tTurretLogic::fGetDeployed, &tTurretLogic::fSetDeployed)
			;
		vm.fRootTable( ).Bind(_SC("TurretLogic"), classDesc);

		fExportTurretGoalsScriptInterface( vm );
	}

}

