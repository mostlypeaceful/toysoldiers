#include "GameAppPch.hpp"
#include "tUserControllableCharacterLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tUseInfantryCamera.hpp"
#include "AI\tSigAIGoal.hpp"
#include "AI\tSigAIGoal.hpp"
#include "tSync.hpp"

#include "tGunWeapon.hpp"
#include "tLandMineLogic.hpp"
#include "tVehicleLogic.hpp"
#include "IK/tIK.hpp"
#include "ContextAnim.hpp"
#include "tTeleporterLogic.hpp"
#include "tReferenceFrameEntity.hpp"
#include "Math/ProjectileUtility.hpp"
#include "tGameEffects.hpp"

#include "Wwise_IDs.h"

// extra stuff
#include "tRPGCamera.hpp"
#include "tKeyFrameAnimation.hpp"
#include "Physics/tGroundRayCastCallback.hpp"

//dlc hacks
#include "tHoverLogic.hpp"
#include "tVehiclePassengerAnimTrack.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( f32, Gameplay_Character_UserControl_PitchMax, 45.0f );
	devvar( bool, Gameplay_Character_UserControl_PitchInvert, false );
	devvar( f32, Gameplay_Character_UserControl_JumpVel, 5.8f ); 
	devvar( f32, Gameplay_Character_UserControl_AimBlend, 0.25f ); 
	devvar( f32, Gameplay_Character_UserControl_AimBlendOutTime, 0.5f );
	devvar( f32, Gameplay_Character_UserControl_YawRate, 1.5f ); 
	devvar( f32, Gameplay_Character_UserControl_PitchRate, 1.5f ); 
	devvar( f32, Gameplay_Character_UserControl_Damping, 0.25f );
	devvar( f32, Gameplay_Character_UserControl_TurnRate, 0.2f );
	devvar( f32, Gameplay_Character_UserControl_SprintStrafeScale, 10.0f );
	devvar( f32, Gameplay_Character_AITurnRate, 0.1f );

	devvar( f32, Gameplay_Character_Commando_TimerHeight, 6.0f );

	devvar( f32, Gameplay_Character_MeleeDistance, 5.f );
	devvar( f32, Gameplay_Character_IK_Squat, 0.1f );

	devvar( f32, Gameplay_Character_Vehicle_ExitPreRay, 0.5f );
	devvar( f32, Gameplay_Character_Vehicle_ExitRay, 2.f ); 
	devvar( f32, Gameplay_Character_ExpireTime, 15.0f ); 
	devvar( bool, Gameplay_Character_ForceJetpack, false ); 

	namespace
	{
		struct tMeleeRayCastCallback
		{
			mutable Math::tRayCastHit	mHit;
			mutable tEntity*			mFirstEntity;
			tEntity*					mIgnoreEntity;
			//const tEntityTagMask		mGroundMask;

			explicit tMeleeRayCastCallback( tEntity& ignore/*, tEntityTagMask groundMask*/ ) : mFirstEntity( 0 ), mIgnoreEntity( &ignore )/*, mGroundMask( groundMask )*/ { }
			inline void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;
				tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i );
				//if( !spatial->fHasGameTagsAny( mGroundMask ) )
				//	return;
				if( spatial->fHasGameTagsAny( GameFlags::cFLAG_DUMMY ) )
					return;
				if( spatial == mIgnoreEntity || spatial->fIsAncestorOfMine( *mIgnoreEntity ) )
					return;
				if( i->fQuickRejectByBox( ray ) )
					return;

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( hit.fHit( ) && hit.mT < mHit.mT )
				{
					mHit			= hit;
					mFirstEntity	= spatial;
				}
			}
		};

		enum tIKChannels
		{
			cIKChannelLeftFoot,
			cIKChannelRightFoot,
			cIKChannelCount
		};

		static const tStringPtr cWaitLocKey( "Vehicle_Wait" );
		static const tStringPtr cExpiresLocKey( "Vehicle_Expires" );

		static const tStringPtr cCommandoStartSprintingEffect( "Voc_RunInto" );
		static const tStringPtr cCommandoStopSprintingEffect( "Voc_RunOut" );
		static const tStringPtr cCommandoJumpEffect( "Voc_Jump" );

		//Extra stuff
		static const tStringPtr cLProp( "l_prop" );
		static const tStringPtr cRProp( "r_prop" );
	}

	tUserControllableCharacterLogic::tUserControllableCharacterLogic( )
		: mLookVec( tVec3f::cZAxis )
		, mRotateVel( 0.f )
		, mNearestVehicleSeatIndex( ~0 )
		, mSpawnLocation( Math::tVec3f::cZeroVector )
		, mMoveVec( Math::tVec3f::cZeroVector )
		, mSpeed( 0.0f )
		, mDoAdvancedMovement( false )
		, mWantsJump( false )
		, mFirstUserInput( true )
		, mLookToggle( false )
		, mSprinting( false )
		, mVehicleBaseStarted( false )
		, mForBarrage( NULL )
		, mAimBlendDownUp( 0.0f )
		, mAimBlendMain( 0.5f, Gameplay_Character_UserControl_AimBlend, Gameplay_Character_UserControl_AimBlend )
		, mAimTimer( -1.0f )
		//, mSeatRelativeXform( tMat3f::cIdentity ) //safely late initialized
		, mVehicleBaseLogic( NULL )
		, mVehicleBaseAcc( tVec3f::cZeroVector )
		, mOriginalXform( tMat3f::cIdentity )
	{ }

	void tUserControllableCharacterLogic::fOnSpawn( )
	{
		tCharacterLogic::fOnSpawn( );

		mSpawnLocation = fOwnerEntity( )->fObjectToWorld( );
		mLookVec = mSpawnLocation.fZAxis( );

		if( tGameApp::fInstance( ).fIsOneManArmy( ) )
		{
			tPlayer* player = tGameApp::fInstance( ).fGameMode( ).fIsNet( ) ? tGameApp::fInstance( ).fGameAppSession( )->fGetHostPlayer( ) : tGameApp::fInstance( ).fFrontEndPlayer( );
			sigassert( player );

			if( player->fHasJetPack( ) || Gameplay_Character_ForceJetpack )
				fBindToVehicle( true );
		}

		mOriginalXform = fOwnerEntity( )->fObjectToWorld( );
	}

	void tUserControllableCharacterLogic::fOnDelete( )
	{
		tCharacterLogic::fOnDelete( );

		fBindToVehicle( false );

		mSpawnLocation = fOwnerEntity( )->fObjectToWorld( );
		mLookVec = mSpawnLocation.fZAxis( );

		mNearestVehicle.fRelease( );
		mRidingVehicle.fRelease( );
		mIKLegs.fRelease( );
		mWaitTimer.fRelease( );
		mExpireTimer.fRelease( );

		for( u32 e = 0; e < mEquip.fCount( ); ++e)
			mEquip[e].fRelease( );
	}


	b32 tUserControllableCharacterLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_UNIT_DESTROYED:
			{
				if( mForBarrage )
				{
					if( fHasSelectionFlag( ) )
					{
						fOwnerEntity( )->fRemoveGameTags( GameFlags::cFLAG_SELECTABLE );
						mForBarrage->fBarrageController( )->fEndBarrage( );
					}
				}
			}
			break;
		}

		return tCharacterLogic::fHandleLogicEvent( e );
	}

	void tUserControllableCharacterLogic::fActST( f32 dt )
	{
		if( mFellOutofLevel )
		{
			mFellOutofLevel = false;

			if( tGameApp::fInstance( ).fIsOneManArmy( ) )
			{
				fOwnerEntity( )->fMoveTo( mOriginalXform );
			}
			else if( fCreationType( ) == cCreationTypeFromLevel )
			{
				fDestroy( true ); //will cause respawn
				fOwnerEntity( )->fDelete( );
			}
			else
			{
				if( mForBarrage )
					mForBarrage->fBarrageController( )->fEndBarrage( );
				fOwnerEntity( )->fDelete( );
			}
		}
		else if( mExtraModeRespawn ) // only for extra mode
		{
			tPlayer* player = fTeamPlayer( );
			fOwnerEntity( )->fDelete( );
			fEndUse( *player );
			player->fRespawn( );
			mExtraModeRespawn = false;
		}

		tCharacterLogic::fActST( dt );

		if( mForBarrage )
			mLevelTimeScale = 1.f;
	}

	void tUserControllableCharacterLogic::fPhysicsMT( f32 dt )
	{
		if( mWantsJump )
		{
			if( !mVehicleBaseLogic )
			{
				mWantsJump = false;
				mPhysics.fJump( mPhysics.fVelocity( ) + tVec3f( 0, Gameplay_Character_UserControl_JumpVel, 0 ), 0.1f );
			}
		}

		tCharacterLogic::fPhysicsMT( dt );
	}

	void tUserControllableCharacterLogic::fThinkST( f32 dt )
	{
		tCharacterLogic::fThinkST( dt );

		if( mWaitTimer )
		{
			mWaitTimer->fThinkST( dt );
			if( mWaitTimer->fFinished( ) && mWaitTimer->fFadeOut( ) )
			{
				mWaitTimer->fDelete( );
				mWaitTimer.fRelease( );
				mTakesDamage = true;
				if( mPackage )
				{
					tUnitLogic* l = mPackage->fLogicDerivedStaticCast<tUnitLogic>( );
					l->fDestroy( true );
					mPackage.fRelease( );
					mHasPackage = false;
				}
			}
		}

		if( fHasSelectionFlag( ) )
		{
			if( fUnderUserControl( ) )
			{
				if( fIsCommando( ) )
				{
					sigassert( fTeamPlayer( ) );
					if( fCountry( ) == GameFlags::cCOUNTRY_USA )
						fTeamPlayer( )->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_USING_COMMANDO, dt );
					else
						fTeamPlayer( )->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_USING_IVAN, dt );
				}
			}
			else
			{
				if( fSelectionEnabled( ) && !fInCameraBox( ) && !mDontInstaDestroy )
				{
					fDestroy( true );
					fEnableSelection( false );
				}
			}
		}

		if( mExpireTimer )
		{
			mExpireTimer->fThinkST( dt );

			if( mExpireTimer->fFinished( ) )
			{
				// kill ourselves
				fDestroy( true );
				mExpireTimer->fDelete( );
				mExpireTimer.fRelease( );
			}
		}

		if( mVehicleBaseLogic )
		{
			// dont let it self destruct
			if( mVehicleBaseLogic->fExitedTheMap( ) )
				mVehicleBaseLogic->fSetExitedTheMap( false );

			if( mWantsJump )
			{
				mWantsJump = false;

				if( !mVehicleBaseStarted )
				{
					tPlayer* player = fTeamPlayer( );
					mParentRelative = true;
					mVehicleBaseStarted = true;

					mVehicleBase->fMoveTo( mVehicleBaseAttachPt->fObjectToWorld( ) );
					mVehicleBaseLogic->fResetPhysics( mVehicleBase->fObjectToWorld( ) );
					mVehicleBaseLogic->fOverrideOccupy( player, true );

					mVehicleBaseLogic->fPushCamera( player, 0 );
					tTurretCameraMovement* camMove = mVehicleBaseLogic->fRequestCameraMovement( );
					sigassert( camMove );
					camMove->fReset( fOwnerEntity( )->fObjectToWorld( ).fZAxis( ) );

					mVehicleBaseLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CONTROL_OVERRIDE_BEGIN ) );
					fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CONTROL_OVERRIDE_BEGIN ) );

					fOwnerEntity( )->fReparent( *mVehicleBase );
					mParentRelativeXformMT = mVehicleBaseAttachPt->fParentRelative( ).fInverse( );
					fOwnerEntity( )->fSetParentRelativeXform( mParentRelativeXformMT );

					fDisablePhysics( true );
					mPhysics.fForceFall( );
					fSetWeaponIgnoreParent( mVehicleBaseLogic );
				}
				else
				{
					tPlayer* player = fTeamPlayer( );
					mParentRelative = false;
					mVehicleBaseStarted = false;

					tTurretCameraMovement* camMove = mVehicleBaseLogic->fRequestCameraMovement( );
					sigassert( camMove );
					mCameraMovement = *camMove;
					mVehicleBaseLogic->fPopCamera( player );

					mVehicleBaseLogic->fOverrideOccupy( fTeamPlayer( ), false );
					mVehicleBaseLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CONTROL_OVERRIDE_END ) );
					fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CONTROL_OVERRIDE_END ) );
					fOwnerEntity( )->fReparentImmediate( *tGameApp::fInstance( ).fCurrentLevelDemand( )->fOwnerEntity( ) );
					mLookVec = mVehicleBase->fObjectToWorld( ).fZAxis( ).fProjectToXZAndNormalize( );

					fDisablePhysics( false );
					fSetWeaponIgnoreParent( this );
				}
			}
			else
			{
				if( !mVehicleBaseStarted )
				{
					mVehicleBase->fMoveTo( mVehicleBaseAttachPt->fObjectToWorld( ) );
				}
				else
				{
					mParentRelativeXformMT = mVehicleBaseAttachPt->fParentRelative( ).fInverse( );

					//keep the aim in sync witht the view
					f32 maxP = fGetAimPitchMax( );
					tVec3f lookVec = mVehicleBaseLogic->fRequestCameraMovement( )->fCameraBasis( ).fZAxis( );
					lookVec = mVehicleBase->fObjectToWorld( ).fInverseXformVector( lookVec );

					f32 downUpAngle = -asin( lookVec.y / lookVec.fLength( ) );
					downUpAngle = fClamp( downUpAngle, -maxP, maxP );
					mAimBlendDownUp = downUpAngle / maxP;

					mPhysics.fSetVelocity( mVehicleBaseLogic->fLinearVelocity( tVec3f::cZeroVector ) * 0.5f );

					tHoverLogic* hover = mVehicleBaseLogic->fDynamicCast<tHoverLogic>( );
					if( hover )
						mVehicleBaseAcc = fLerp( mVehicleBaseAcc, fOwnerEntity( )->fObjectToWorld( ).fInverseXformVector( hover->fPhysics( ).fDeltaV( ) / (hover->fPhysics( ).fProperties( ).mSpeedAcc * dt) ), 0.2f );
				}
			}
		}
	}

	void tUserControllableCharacterLogic::fMoveST( f32 dt )
	{
		tCharacterLogic::fMoveST( dt );

		mLookVec = fOwnerEntity( )->fObjectToWorld( ).fZAxis( );
	}

	void tUserControllableCharacterLogic::fCoRenderMT( f32 dt )
	{
		tCharacterLogic::fCoRenderMT( dt );

		for( u32 i = 0; i < fWeaponStationCount( ); ++i )
			fWeaponStation( i )->fProcessMT( dt );

		if( mExtraMode && !mRidingVehicle )
		{
			fFindNearestVehicleMT( );
		}
	}

	void tUserControllableCharacterLogic::fShowWaitTimer( )
	{
		fQueryEnums( );
		if( fUnitAttributeBatteryRechargeTime( ) > 0 )
		{
			mWaitTimer.fReset( NEW Gui::tHoverTimer( cWaitLocKey, fUnitAttributeBatteryRechargeTime( ), false, fTeam( ) ) );
			mWaitTimer->fSpawn( *fOwnerEntity( ) );
			tMat3f xform = tMat3f::cIdentity;
			xform.fSetTranslation( tVec3f( 0, Gameplay_Character_Commando_TimerHeight, 0 ) );
			mWaitTimer->fSetParentRelativeXform( xform );
			mTakesDamage = false;

			if( mExpireTimer )
			{
				mExpireTimer->fDelete( );
				mExpireTimer.fRelease( );
			}
		}
	}

	void tUserControllableCharacterLogic::fRespawn( )
	{
		if( fTeam( ) == GameFlags::cTEAM_BLUE )
			tUnitLogic::fSetHitPoints( tUnitLogic::fUnitAttributeMaxHitPoints( ) );

		fOwnerEntity( )->fMoveTo( mSpawnLocation );
	}

	Gui::tRadialMenuPtr tUserControllableCharacterLogic::fCreateSelectionRadialMenu( tPlayer& player )
	{
		Gui::tRadialMenuPtr radialMenu = Gui::tRadialMenuPtr( NEW Gui::tRadialMenu( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptInfantryOptions ), player.fUser( ) ) );

		Sqrat::Object params( Sqrat::Table( ).SetValue( "Unit", this ).SetValue( "Player", &player ) );
		Sqrat::Function( radialMenu->fCanvas( ).fScriptObject( ), "DefaultSetup" ).Execute( params );

		return radialMenu;
	}

	b32 tUserControllableCharacterLogic::fCanBeUsed( ) const
	{
		return !fUnderUserControl( ) && !fIsDestroyed( ) && !mPackage;
	}

	b32 tUserControllableCharacterLogic::fTryToUse( tPlayer* player )
	{
		if( fCanBeUsed( ) && fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_CONTROL_BEGIN ) ) )
		{
			// Set player, and then camera
			fSetTeamPlayer( player );

			if( mExtraMode )
			{
				tRPGCamera* cam = NEW tRPGCamera( *fTeamPlayer( ), *this );
				cam->fSkipBlendIn( );
				fTeamPlayer( )->fPushCamera( Gfx::tCameraControllerPtr( cam ) );
			}
			else
				fTeamPlayer( )->fPushCamera( Gfx::tCameraControllerPtr( NEW tUseInfantryCamera( *fTeamPlayer( ), *this ) ) );

			if( !player->fCurrentBarrage( ).fIsNull( ) )
				player->fCurrentBarrage( ).fCodeObject( )->fEnteredBarrageUnit( this, true, player );

			if( fIsCommando( ) )
				fCommandoAudio( cCommandoAudioSelect );

			if( mExpireTimer )
			{
				mExpireTimer->fDelete( );
				mExpireTimer.fRelease( );
			}

			return true;
		}
		return false;
	}

	b32 tUserControllableCharacterLogic::fEndUse( tPlayer& player )
	{
		if( fTeamPlayer( ) && fUnderUserControl( ) )
		{
			// Set player, and then camera
			fSetTeamPlayer( NULL );

			if( mExtraMode )
				player.fCameraStack( ).fPopCamerasOfType<tRPGCamera>( );
			player.fCameraStack( ).fPopCamerasOfType<tUseInfantryCamera>( );

			fActivateWeapons( false );

			mFirstUserInput = true;

			if( !player.fCurrentBarrage( ).fIsNull( ) )
				player.fCurrentBarrage( ).fCodeObject( )->fEnteredBarrageUnit( this, false, &player );

			if( !mForBarrage && !mExpireTimer && !mWaitTimer && !fIsDestroyed( ) )
			{
				mExpireTimer.fReset( NEW Gui::tHoverTimer( cExpiresLocKey, Gameplay_Character_ExpireTime, false, fTeam( ) ) ); 
				mExpireTimer->fSpawn( *fOwnerEntity( ) );
			}
		}
		return true;
	}

	b32 tUserControllableCharacterLogic::fEndUseScript( )
	{
		return fEndUse( *fTeamPlayer( ) );
	}

	namespace
	{
		b32 fSprintStickInvalidated( const Math::tVec2f& stick )
		{
			f32 strafeThreshSqr = 0.35f;
			strafeThreshSqr *= strafeThreshSqr;
			return (stick.fLengthSquared( ) < strafeThreshSqr || fAbs( fShortestWayAround( stick.fAngle( ) - cPiOver2, 0.f ) ) > cPiOver2);
		}
	}

	void tUserControllableCharacterLogic::fComputeUserInput( f32 dt, const Input::tGamepad& gamepad )
	{
		u32 moveButton = fTeamPlayer( )->fMoveThumbButton( tUserProfile::cProfileCharacters );
		tVec2f moveStick = fTeamPlayer( )->fMoveStick( tUserProfile::cProfileCharacters );
		tVec2f aimStick = fTeamPlayer( )->fAimStick( tUserProfile::cProfileCharacters );
		moveStick.x = -moveStick.x;

		if( mExtraMode )
		{
			fComputeUserInputExtra( moveStick, aimStick, dt, gamepad );
		}
		else
		{
	/*		if( mVehicleBaseStarted )
			{
				sigassert( mVehicleBaseLogic );
				mPhysics.fSetVelocity( mVehicleBaseLogic->fLinearVelocity( tVec3f::cZeroVector ) );
				moveStick = tVec2f::cZeroVector;
				aimStick.x = 0.f;
			}*/

			if( !mVehicleBaseStarted )
				fFirstPersonMovement( moveStick, aimStick, dt );
		}

		if( !mExtraMode || !fInVehicle( ) )
		{
			// shooting
			f32 aimWeight = 1.0f;

			if( mAimTimer > 0.0f )
			{
				mAimTimer -= dt;
				aimWeight = 1.0f;
			}
			else if( mExtraMode )
				aimWeight = 0.0f;

			mAimBlendMain.fSetBlends( Gameplay_Character_UserControl_AimBlend );
			mAimBlendMain.fStep( aimWeight, dt );

			// jumping
			if( !mFirstUserInput && gamepad.fButtonDown( Input::tGamepad::cButtonA ) )
			{
				if( !mPhysics.fFalling( ) || mVehicleBaseStarted )
				{
					mWantsJump = true;
					tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), cCommandoJumpEffect );
				}

			}

			if( !mSprinting )
			{
				if( gamepad.fButtonDown( moveButton )
					&& !fSprintStickInvalidated( moveStick ) )
					fSetSprinting( true );
			}
			else
			{
				if( gamepad.fButtonDown( moveButton ) 
					|| fSprintStickInvalidated( moveStick ) )
					fSetSprinting( false );
			}
		}

		mFirstUserInput = false;

		if( fHasWeaponStation( 0 ) )
		{
			const tWeaponStationPtr& station = fWeaponStation( 0 );
			tWeaponBankPtr& rocketBank = station->fBank( 1 );

			if( gamepad.fButtonDown( rocketBank->fTriggerButton( ) ) && rocketBank->fShouldFire( ) ) 
				rocketBank->fFire( );
			else if( gamepad.fButtonUp( rocketBank->fTriggerButton( ) ) ) 
				rocketBank->fEndFire( );

			tFireEventList events;
			station->fAccumulateFireEvents( events );
			for( u32 e = 0; e < events.fCount( ); ++e )
				tWeaponStation::fApplyRumbleEvent( events[ e ], *fTeamPlayer( ) );

			if( events.fCount( ) ) 
			{
				fSetSprinting( false );
				if( fIsCommando( ) )
					fCommandoAudio( tUserControllableCharacterLogic::cCommandoAudioFire );
			}

			if( station->fNeedsReload( ) )
				station->fReload( );
		}
	}

	void tUserControllableCharacterLogic::fComputeAIInput( f32 dt )
	{	
		mMoveVec = tVec3f::cZeroVector;
		mSpeed = 0.f;

		if( mAITargeting )
		{
			f32 aimWeight = 0.0f;

			if( mAimTimer > 0.0f )
			{
				mAimTimer -= dt;
				aimWeight = 1.0f;
			}
			mAimBlendMain.fSetBlends( Gameplay_Character_UserControl_AimBlend );
			mAimBlendMain.fStep( aimWeight, dt );

			if( fHasWeapon( 0, 0, 0 ) )
			{
				tWeapon* weap = fWeaponRawPtr( 0, 0, 0 );
				if( weap->fHasTarget( ) )
				{
					fRaiseGun( );

					tVec3f fireVec = weap->fComputeIdealLaunchVector( );

					//rotation
					tVec3f lookVec = fireVec;
					lookVec.fProjectToXZ( );
					tMat3f xForm = fOwnerEntity( )->fObjectToWorld( );
					xForm.fTurnToFaceZ( lookVec, Gameplay_Character_AITurnRate, dt );
					sigassert( !xForm.fIsNan( ) );
					fOwnerEntity( )->fMoveTo( xForm );

					//aiming
					f32 maxP = fGetAimPitchMax( );
					f32 downUpAngle = asin( fireVec.y / fireVec.fLength( ) ); //SOH
					downUpAngle = fClamp( downUpAngle, -maxP, maxP );
					mAimBlendDownUp = downUpAngle / maxP;
				}
			}
		}
	}

	void tUserControllableCharacterLogic::fSetSprinting( b32 value )
	{
		if( value )
			tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), cCommandoStartSprintingEffect );
		else if( mSprinting && !mPhysics.fWantsJump( ) && !mPhysics.fFalling( ) )
			tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), cCommandoStopSprintingEffect );

		mSprinting = value;
	}

	void tUserControllableCharacterLogic::fComputeUserInputExtra( const Math::tVec2f& moveStick, const Math::tVec2f& aimStick, f32 dt, const Input::tGamepad& gamepad )
	{
		if( !fInVehicle( ) )
		{
			if( mLookToggle ) 
				fFirstPersonMovement( moveStick, aimStick, dt );
			else
				f3rdPersonMovement( moveStick, aimStick, dt );

			if( gamepad.fButtonDown( fTeamPlayer( )->fAimThumbButton( tUserProfile::cProfileCharacters ) ) )
			{
				if( mLookToggle )
				{
					mAimBlendDownUp = 0.f;
					fTeamPlayer( )->fCameraStack( ).fPopCamerasOfType<tUseInfantryCamera>( );
					mLookToggle = false;
				}
				else
				{
					fTeamPlayer( )->fPushCamera( Gfx::tCameraControllerPtr( NEW tUseInfantryCamera( *fTeamPlayer( ), *this ) ) );
					mLookToggle = true;
				}
			}

			if( fHasWeaponStation( 0 ) )
				fWeaponStation( 0 )->fUI( )->fShowHideReticle( mLookToggle );
		}
	}

	void tUserControllableCharacterLogic::fFirstPersonMovement( const Math::tVec2f& moveStick, const Math::tVec2f& aimStick, f32 dt )
	{
		// strafing
		mMoveVec = tVec3f( moveStick.x, 0, moveStick.y );

		mSpeed = mMoveVec.fLength( );
		if( mIKLegs ) mIKLegs->fSetIdle( mSpeed < 0.05f );

		if( mMoveVec.z < 0.0f )
			mSpeed *= -1.0f;

		// view pitch
		mCameraMovement.fSetSpeed( fUseCamRotSpeed( ) );
		mCameraMovement.fSetDamping( fUnitAttributeUseCamRotDamping( ).fXY( ) );
		mCameraMovement.fUpdate( dt, aimStick );

		f32 maxP = fGetAimPitchMax( );
		f32 downUpAngle = mAimBlendDownUp * maxP;

		downUpAngle -= mCameraMovement.fYawPitchVelocity( ).y;
		downUpAngle = fClamp( downUpAngle, -maxP, maxP );

		mAimBlendDownUp = downUpAngle / maxP;

		// view yaw
		tQuatf rotation( tAxisAnglef(tVec3f::cYAxis, -mCameraMovement.fYawPitchVelocity( ).x) );
		mLookVec = rotation.fRotate( mLookVec );
		mLookVec.fProjectToXZAndNormalize( );

		tMat3f xForm = fOwnerEntity( )->fObjectToWorld( );
		xForm.fOrientZAxis( mLookVec, tVec3f::cYAxis );

		if( mSprinting )
			xForm.fTranslateLocal( tVec3f( moveStick.x * dt * Gameplay_Character_UserControl_SprintStrafeScale, 0, 0 ) );


		sigassert( !xForm.fIsNan( ) );
		fOwnerEntity( )->fMoveTo( xForm );

		mPhysics.fSetVelocity( rotation.fRotate( mPhysics.fVelocity( ) ) );
	}

	void tUserControllableCharacterLogic::f3rdPersonMovement( const Math::tVec2f& moveStick, Math::tVec2f aimStick, f32 dt )
	{
		aimStick *= aimStick * aimStick;

		// strafing
		const Math::tVec3f lookX = Math::tVec3f::cYAxis.fCross( mLookVec );
		tVec3f worldMoveVec = lookX * moveStick.x + mLookVec * moveStick.y;
		worldMoveVec.fNormalizeSafe( mSpeed );

		tVec3f facing;

		b32 aim = mAimBlendMain.fValue( ) > 0.1f;
		if( aim )
			facing = mLookVec;
		else
			facing = worldMoveVec;

		if( mIKLegs ) mIKLegs->fSetIdle( mSpeed < 0.05f );

		// rotate to move direction
		if( mSpeed > 0.01f || aim )
		{
			tMat3f xForm = fOwnerEntity( )->fObjectToWorld( );
			xForm.fTurnToFaceZ( facing, Gameplay_Character_UserControl_TurnRate, dt );
			fOwnerEntity( )->fMoveTo( xForm );
		}

		mMoveVec = fOwnerEntity( )->fObjectToWorld( ).fInverseXformVector( worldMoveVec );

		// view pitch
		f32 maxP = fGetAimPitchMax( );

		Math::tVec2f rate( -Gameplay_Character_UserControl_YawRate, Gameplay_Character_UserControl_PitchInvert ? Gameplay_Character_UserControl_PitchRate : -Gameplay_Character_UserControl_PitchRate );
		rate *= aimStick * dt;

		mRotateVel *= Gameplay_Character_UserControl_Damping;
		mRotateVel += rate;

		Math::tQuatf rotate( Math::tAxisAnglef( Math::tVec3f::cYAxis, mRotateVel.x ) );
		mLookVec = rotate.fRotate( mLookVec );
		mLookVec.fProjectToXZAndNormalize( );
	}

	tVehicleLogic* tUserControllableCharacterLogic::fRidingVehicleLogic( ) const 
	{ 
		sigassert( mRidingVehicle ); 
		return mRidingVehicle->fLogicDerivedStaticCast<tVehicleLogic>( ); 
	}

	namespace
	{
		void fProcessVehicle( const tVec3f& myPos, const tEntityPtr& ent, tEntityPtr& vehicleOut, u32& indexOut )
		{
			tVehicleLogic* veh = ent->fLogicDerived<tVehicleLogic>( );
			sigassert( veh );

			u32 availSeatIndex = veh->fFindEmptySeat( myPos );
			if( availSeatIndex != ~0 )
			{
				indexOut = availSeatIndex;
				vehicleOut = ent;
			}
		}
	}

	void tUserControllableCharacterLogic::fFindNearestVehicleMT( )
	{
		sigassert( !fInVehicle( ) );
		const tVec3f myPos = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );

		mNearestVehicle.fRelease( );
		mNearestVehicleSeatIndex = ~0;

		const tGrowableArray<tEntityPtr>& vehicles = tGameApp::fInstance( ).fCurrentLevel( )->fUnitList( GameFlags::cUNIT_TYPE_VEHICLE );
		for( u32 i = 0; i < vehicles.fCount( ); ++i )
			fProcessVehicle( myPos, vehicles[ i ], mNearestVehicle, mNearestVehicleSeatIndex );

		const tGrowableArray<tEntityPtr>& airborne = tGameApp::fInstance( ).fCurrentLevel( )->fUnitList( GameFlags::cUNIT_TYPE_AIR );
		for( u32 i = 0; i < airborne.fCount( ); ++i )
			fProcessVehicle( myPos, airborne[ i ], mNearestVehicle, mNearestVehicleSeatIndex );
	}

	void tUserControllableCharacterLogic::fHandleEnterExitVehicle( const Input::tGamepad& gamepad )
	{
		if( !fInVehicle( ) )
		{
			if( mNearestVehicle && gamepad.fButtonDown( Input::tGamepad::cButtonY ) )
			{
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_ENTER_VEHICLE, NEW Logic::tObjectEventContext( Sqrat::Object( Sqrat::Table( ).SetValue( "SeatIndex", mNearestVehicleSeatIndex ) ) ) ) );
			}
		}
		else
		{
			sigassert( mRidingVehicle );
			if( gamepad.fButtonDown( Input::tGamepad::cButtonY ) )
			{
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_EXIT_VEHICLE ) );
			}
		}
	}

	void tUserControllableCharacterLogic::fSwitchSeats( u32 newSeat )
	{
		sigassert( mRidingVehicle );
		mNearestVehicleSeatIndex = newSeat;
		fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_ENTER_VEHICLE, NEW Logic::tObjectEventContext( Sqrat::Object( Sqrat::Table( ).SetValue( "SeatIndex", mNearestVehicleSeatIndex ) ) ) ) );
	}

	void tUserControllableCharacterLogic::fAcquireExitVehicleXform( Math::tMat3f xform, tVehicleLogic* vehicle )
	{
		xform.fOrientYWithZAxis( tVec3f::cYAxis, xform.fZAxis( ) );
		fOwnerEntity( )->fMoveTo( xform );
		mPhysics.fSetTransform( xform );
		mPhysics.fSetVelocity( vehicle->fCurrentVelocityMT( ) );
	}

	void tUserControllableCharacterLogic::fShowEquip( u32 equip, b32 show )
	{
		u32 start = equip;
		u32 end = equip;

		if( equip == cAllEquip )
		{
			start = 0;
			end = cEquipCount-1;
		}

		for( u32 i = start; i <= end; ++i )
			if( mEquip[ i ] )
				Gfx::tRenderableEntity::fSetDisabled( *mEquip[ i ], !show );
	}

	IK::tCharacterLegTargets* tUserControllableCharacterLogic::fGetIKLegs( ) 
	{ 
		if( !mIKLegs ) mIKLegs.fReset( NEW IK::tCharacterLegTargets( this, GameFlags::cFLAG_DUMMY, GameFlags::cFLAG_GROUND, false ) );

		return mIKLegs.fGetRawPtr( ); 
	}

	void tUserControllableCharacterLogic::fRaiseGun( )
	{
		mAimTimer = Gameplay_Character_UserControl_AimBlendOutTime;
	}


	f32 tUserControllableCharacterLogic::fGetAimPitchMax( ) const
	{
		return fToRadians( (f32)Gameplay_Character_UserControl_PitchMax );
	}

	void tUserControllableCharacterLogic::fActivateWeapons( b32 activate )
	{
		if( activate )
		{
			if( fHasWeaponStation( 0 ) && fTeamPlayer( ) )
				fWeaponStation( 0 )->fBeginUserControl( fTeamPlayer( ) );
		}
		else
		{
			if( fHasWeaponStation( 0 ) )
				fWeaponStation( 0 )->fEndUserControl( );
		}
	}


	namespace
	{
		static const char* cRamboActivate = "Play_VO_Commando_Activate";
		static const char* cRamboUse = "Play_VO_Commando_Use";
		static const char* cRamboExit = "Play_VO_Commando_Exit";
	}

	const Audio::tSourcePtr& tUserControllableCharacterLogic::fPlayerOrLocalSource( ) const
	{
		return fTeamPlayer( ) ? fTeamPlayer( )->fSoundSource( ) : fAudioSource( );
	}

	void tUserControllableCharacterLogic::fCommandoAudio( u32 commandoEvent )
	{
		sigassert( fAudioSource( ) );
		switch( commandoEvent )
		{
		case cCommandoAudioSelect:	fPlayerOrLocalSource( )->fHandleEvent( cRamboActivate );	break;
		case cCommandoAudioFire:	fAudioSource( )->fHandleEvent( cRamboUse );			break;
		case cCommandoAudioDeath:	fPlayerOrLocalSource( )->fHandleEvent( cRamboExit );		break;
		}
	}

	void tUserControllableCharacterLogic::fBindToVehicle( b32 bind )
	{
		if( bind )
		{
			if( mVehicleBase )
				fBindToVehicle( false );

			tEntity* parent = tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( );
			mVehicleBase.fReset( parent->fSpawnChild( mVehicleBasePath ) );

			mVehicleBaseAttachPt.fReset( fOwnerEntity( )->fFirstDescendentWithName( tStringPtr( "targetOffset" ), false ) );
			sigassert( mVehicleBaseAttachPt && "Missing vehicle attach pt 'targetOffset'" );

			if( mVehicleBase )
			{
				mVehicleBaseLogic = mVehicleBase->fLogicDerived<tVehicleLogic>( );
				mVehicleBase->fMoveTo( mVehicleBaseAttachPt->fObjectToWorld( ) );
				mVehicleBaseLogic->fSetDontInstaDestroyOFB( true );
			}
		}
		else
		{
			if( mVehicleBaseLogic )
			{
				mVehicleBaseLogic = NULL;
				mVehicleBase.fRelease( );
			}
		}
	}


	// ---------- GOALS --------------------
	typedef AI::tDerivedLogicGoalHelper<tUserControllableCharacterLogic> tUserCharacterGoalHelper;

	class tInfantryOneShotFireGoal : public AI::tSigAIGoal, public tUserCharacterGoalHelper
	{
		define_dynamic_cast(tInfantryOneShotFireGoal, AI::tSigAIGoal);
	public:
		b32 mShotFired;

		tInfantryOneShotFireGoal( )
			: mShotFired( false )
		{ 
		}

		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );

			fSetMotionState( logic, "FireOneShot", Sqrat::Object(), true );
			AI::tSigAIGoal::fOnActivate( logic );

			fLogic( )->fRaiseGun( );
		}

		virtual void fOnSuspend( tLogic* logic )
		{
			if( !mShotFired )
				fFireShot( );

			AI::tSigAIGoal::fOnSuspend( logic );

			if( fLogic( )->fWeaponStation( 0 )->fNeedsReload( ) )
				logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_RELOAD_START ) );
		}

		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			//wait till gun comes up
			if( !mShotFired && fLogic( )->fGetAimBlendMain( ) > 0.9f )
				fFireShot( );

			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}

		void fFireShot( )
		{
			mShotFired = true;
			fLogic( )->fWeaponStation( 0 )->fFire( );
		}
	};

	class tInfantryContinuousFireGoal : public AI::tSigAIGoal, public tUserCharacterGoalHelper
	{
		define_dynamic_cast(tInfantryContinuousFireGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );

			fLogic( )->fBlendOutFireLooping( );

			fPersist( );
			AI::tSigAIGoal::fOnActivate( logic );

			fLogic( )->fRaiseGun( );
		}
		virtual void fOnSuspend( tLogic* logic )
		{
			log_assert( logic, "Suspending goal after logic is lost!" );

			AI::tSigAIGoal::fOnSuspend( logic );

			if( fExitMode( ) == cExitCompleted )
			{
				fLogic( )->fBlendOutFireLooping( );
			}
		}

		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			fLogic( )->fRaiseGun( );

			tWeaponBankPtr& gunBank = fLogic( )->fWeaponStation( 0 )->fBank( 0 );

			if( !gunBank->fShouldFire( ) || gunBank->fNeedsReload( ) )
			{
				if( gunBank->fNeedsReload( ) )
					logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_RELOAD_START ) );

				fMarkAsComplete( );
			}
			else
			{
				if( !gunBank->fFiring( ) && fLogic( )->fGetAimBlendMain( ) > 0.75f )
				{
					fSetMotionState( logic, "FireLooping", Sqrat::Object(), false );
					fExecuteMotionState( logic );

					gunBank->fFire( );
				}

				//tPlayer* tp = fLogic( )->fTeamPlayer( );
				//if( tp )
				//{
				//	if( tp->fGamepad( ).fRightTriggerDown( ) )
				//		logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_THROW_GRENADE ) );
				//	if( tp->fGamepad( ).fButtonDown( Input::tGamepad::cButtonLShoulder ) )
				//		logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_MELEE ) );
				//}
			}

			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}
	};

	class tInfantryContinuousFireUserGoal : public tInfantryContinuousFireGoal
	{
		define_dynamic_cast(tInfantryContinuousFireUserGoal, tInfantryContinuousFireGoal);

		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			tInfantryContinuousFireGoal::fOnProcess( goalPtr, logic, dt );

			const Input::tGamepad& gp = fLogic( )->fTeamPlayer( )->fGamepad( );
			const tWeaponBankPtr& gunBank = fLogic( )->fWeaponStation( 0 )->fBank( 0 );

			if( !gp.fButtonHeld( gunBank->fTriggerButton( ) ) )
			{
				fMarkAsComplete( );
			}
			else if( !gunBank->fShouldFire( ) )
			{
				logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_RELOAD_START ) );
			}
		}
	};

	class tInfantryReloadGoal : public AI::tSigAIGoal, public tUserCharacterGoalHelper
	{
		define_dynamic_cast(tInfantryReloadGoal, AI::tSigAIGoal);
	public:

		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );

			fSetMotionState( logic, "Reload", Sqrat::Object(), true );
			AI::tSigAIGoal::fOnActivate( logic );

			fLogic( )->fWeaponStation( 0 )->fReload( );
		}
		virtual void fOnSuspend( tLogic* logic )
		{
			AI::tSigAIGoal::fOnSuspend( logic );
		}

		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}
	};




	namespace { const tStringPtr cReleaseTag( "release" ); }

	class tInfantryThrowGrenadeGoal : public AI::tSigAIGoal, public tUserCharacterGoalHelper
	{
		define_dynamic_cast(tInfantryThrowGrenadeGoal, AI::tSigAIGoal);
	public:

		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );

			fSetMotionState( logic, "Throw", Sqrat::Object(), true );

			AI::tSigAIGoal::fOnActivate( logic );
		}

		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}
		virtual b32	fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e )
		{
			switch( e.fEventId( ) )
			{
			case GameFlags::cEVENT_ANIMATION:
				{
					const tKeyFrameEventContext* context = e.fContext< tKeyFrameEventContext >( );
					sigassert( context && fLogic( )->fHasWeapon( 0,0,0 ) );
					if( context->mTag == cReleaseTag )
					{
						const tWeaponPtr& weapon = fLogic( )->fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 );
						const tMat3f& xform = weapon->fFirstAnchorPoint( )->fObjectToWorld( );

						tVec3f target;
						if ( fLogic( )->fWeaponStation( 0 )->fComputeRealTargetThroughRetical( xform.fGetTranslation( ), target ) )
							tLandMineLogic::fThrow( xform, target, tVec3f::cYAxis, fLogic( )->fOwnerEntity( ) );
					}
				}
				break;
			default:
				return AI::tSigAIGoal::fHandleLogicEvent( logic, e );
			}
			return true;
		}
	};

	namespace { const tStringPtr cStabTag( "stab" ); }

	class tInfantryMeleeGoal : public AI::tSigAIGoal, public tUserCharacterGoalHelper
	{
		define_dynamic_cast(tInfantryMeleeGoal, AI::tSigAIGoal);
	public:

		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );

			fLogic( )->fRaiseGun( );

			fSetMotionState( logic, "Melee", Sqrat::Object(), true );

			AI::tSigAIGoal::fOnActivate( logic );
		}

		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			fLogic( )->fRaiseGun( );
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}
		virtual b32	fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e )
		{
			//switch( e.fEventId( ) )
			//{
			//case GameFlags::cEVENT_ANIMATION:
			//	{
			//		const tKeyFrameEventContext* context = e.fContext< tKeyFrameEventContext >( );
			//		sigassert( context );
			//		log_assert( fLogic( )->fHasWeaponStation( 0 ), "Need a weapon station and ui to stab." );
			//		if( context->mTag == cStabTag )
			//		{
			//			tVec3f myPos = fLogic( )->fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
			//			tMeleeRayCastCallback rayCastCallback( *fLogic( )->fOwnerEntity( ) );
			//			tRayf ray = fLogic( )->fWeaponStation( 0 )->fRayThroughRetical( myPos );
			//			fLogic( )->fSceneGraph( )->fRayCast( ray, rayCastCallback );

			//			if( rayCastCallback.mHit.fHit( ) && rayCastCallback.mFirstEntity )
			//			{
			//				tEntity *le = rayCastCallback.mFirstEntity->fFirstAncestorWithLogic( );
			//				if( le )
			//				{
			//					tUnitLogic *ul = le->fLogicDerived<tUnitLogic>( );
			//					if( ul )
			//					{
			//						tVec3f theirPos = ray.fEvaluate( rayCastCallback.mHit.mT );
			//						tVec3f delta = theirPos - myPos;
			//						delta.y = 0;

			//						f32 maxRangeSqr = Gameplay_Character_MeleeDistance * Gameplay_Character_MeleeDistance;
			//						if( delta.fLengthSquared( ) <= maxRangeSqr )
			//						{
			//							tDamageContext dc;
			//							dc.fSetAttacker( fLogic( ) );
			//							dc.fSetWeapon( GameFlags::cDAMAGE_TYPE_STAB, 10.0f );
			//							dc.mWorldPosition = myPos;
			//							dc.mWorldEffectorVector = delta;

			//							ul->fDealDamage( dc );
			//						}

			//					}
			//				}
			//			}
			//		}
			//	}
			//	break;
			//default:
			//	return AI::tSigAIGoal::fHandleLogicEvent( logic, e );
			//}
			return true;
		}
	};

	class tInfantryUserIdleFireGoal : public AI::tSigAIGoal, public tUserCharacterGoalHelper
	{
		define_dynamic_cast(tInfantryUserIdleFireGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );
		}

		//virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		//{

		//	AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		//}
	};

	class tInfantryUnderUserControlGoal : public AI::tSigAIGoal, public tUserCharacterGoalHelper
	{
		define_dynamic_cast(tInfantryUnderUserControlGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );

			fLogic( )->fSetUnderUserControl( true );
			fLogic( )->fEnableSelection( false );

			fLogic( )->fActivateWeapons( true );

			AI::tSigAIGoal::fOnActivate( logic );
		}
		virtual void fOnSuspend( tLogic* logic )
		{
			log_assert( logic, "Suspending goal after logic is lost!" );

			if( fLogic( )->fTeamPlayer( ) )
				fLogic( )->fEndUse( *fLogic( )->fTeamPlayer( ) );

			fLogic( )->fSetUnderUserControl( false );
			fLogic( )->fEnableSelection( true );

			AI::tSigAIGoal::fOnSuspend( logic );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			if( fLogic( )->fTeamPlayer( ) )
			{
				fLogic( )->fComputeUserInput( dt, fLogic( )->fTeamPlayer( )->fGamepad( ) );

				tPlayer* tp = fLogic( )->fTeamPlayer( );

				if( tp && fLogic( )->fHasWeaponStation( 0 ) )
				{
					tWeaponStationPtr& station = fLogic( )->fWeaponStation( 0 );

					const Input::tGamepad& gp = tp->fGamepad( );
					if( fLogic( )->fExtraMode( ) ) 
						fLogic( )->fHandleEnterExitVehicle( gp );

					tWeaponBankPtr& gunBank = station->fBank( 0 );
					if( gunBank->fIsContinuousFire( ) )
					{
						if( gp.fButtonHeld( gunBank->fTriggerButton( ) ) )
						{
							if( !gunBank->fFiring( ) && gunBank->fShouldFire( ) )
								logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_FIRE_BEGIN ) );
						}
						else
						{
							fLogic( )->fBlendOutFireLooping( );
						}
					}
					else
					{
						sigassert( !"Add support for tUserControllableCharacterLogic single shot weapons" );
					}

					//if( tp->fGamepad( ).fRightTriggerDown( ) )
					//	logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_THROW_GRENADE ) );

					if( tp->fGamepad( ).fButtonDown( Input::tGamepad::cButtonLShoulder ) )
						logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_MELEE ) );
				}
			}

			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}
		virtual b32	fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e )
		{
			switch( e.fEventId( ) )
			{
			case GameFlags::cEVENT_USER_FIRE_BEGIN:
				{
					const tWeaponStationPtr& station = fLogic( )->fWeaponStation( 0 );
					if( station->fShouldFire( ) )
					{
						if( station->fIsContinuousFire( ) )
						{
							fPushGoal( AI::tGoalPtr( NEW tInfantryContinuousFireUserGoal( ), 1.0f ), logic );
						}
						else
						{
							fPushGoal( AI::tGoalPtr( NEW tInfantryOneShotFireGoal( ), 1.0f ), logic );
						}
					}
				}
				return true;
			case GameFlags::cEVENT_RELOAD_START:
				{
					fPushGoal( AI::tGoalPtr( NEW tInfantryReloadGoal( ), 1.0f ), logic );
				}
				return true;
			//case GameFlags::cEVENT_USER_THROW_GRENADE:
			//	{
			//		fPushGoal( AI::tGoalPtr( NEW tInfantryOneShotFireGoal( ), 1.0f ), logic );
			//		//fPushGoal( AI::tGoalPtr( NEW tInfantryThrowGrenadeGoal( ), 1.0f ), logic  );
			//	}
			//	return true;
			case GameFlags::cEVENT_USER_MELEE:
				{
					fPushGoal( AI::tGoalPtr( NEW tInfantryMeleeGoal( ), 1.0f ), logic  );
				}
				return true;
			default:
				break;
			}

			return AI::tSigAIGoal::fHandleLogicEvent( logic, e );
		}
	};



	class tInfantryUnderAIControlGoal : public AI::tSigAIGoal, public tUserCharacterGoalHelper
	{
		define_dynamic_cast(tInfantryUnderAIControlGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );

			fLogic( )->fActivateWeapons( true );

			AI::tSigAIGoal::fOnActivate( logic );
		}
		virtual void fOnSuspend( tLogic* logic )
		{
			log_assert( logic, "Suspending goal after logic is lost!" );

			AI::tSigAIGoal::fOnSuspend( logic );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			sigassert( fLogic( )->fAITargeting( ) );

			if( !fLogic( )->fUnderUserControl( ) )
			{
				fLogic( )->fComputeAIInput( dt );
			}

			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}
		virtual b32	fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e )
		{
			switch( e.fEventId( ) )
			{
			case GameFlags::cEVENT_USER_FIRE_BEGIN:
				{
					const tWeaponStationPtr& station = fLogic( )->fWeaponStation( 0 );
					if( station->fShouldFire( ) )
					{
						if( station->fIsContinuousFire( ) )
						{
							fClearAndPushGoal( 1, AI::tGoalPtr( NEW tInfantryContinuousFireGoal( ), 1.0f ), logic );
						}
						else
						{
							fPushGoal( AI::tGoalPtr( NEW tInfantryOneShotFireGoal( ), 1.0f ), logic );
						}
					}
				}
				return true;
			case GameFlags::cEVENT_RELOAD_START:
				{
					fPushGoal( AI::tGoalPtr( NEW tInfantryReloadGoal( ), 1.0f ), logic );
				}
				return true;
			default:
				break;
			}

			return AI::tSigAIGoal::fHandleLogicEvent( logic, e );
		}
	};

	class tCharacterVehicleTransitionGoal : public AI::tSigAIGoal, public tUserCharacterGoalHelper
	{
		define_dynamic_cast(tCharacterVehicleTransitionGoal, AI::tSigAIGoal);
	public:
		b32 mIntoVehicle;
		tPRSXformf mCurrentPosition;
		tPRSXformf mEndPosition;
		Math::tVec3f mPositionCorrection;
		Math::tVec3f mAnimTranslationDir;
		Math::tVec3f mInitialPosition;
		f32 mAnimTranslationLen;
		f32 mProgress;

		b8 mAnimSet;
		b8 mPropsToggled;
		b8 mSwitchingSeats;
		b8 pad1;

		tCharacterVehicleTransitionGoal( ) 
			: mIntoVehicle( false )
			, mAnimSet( false )
			, mAnimTranslationDir( tVec3f::cZeroVector )
			, mPositionCorrection( tVec3f::cZeroVector )
			, mAnimTranslationLen( 0.f )
			, mProgress( 0.f )
			, mPropsToggled( false )
			, mSwitchingSeats( false )
		{ }


		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			tEntityPtr oldVehicle( fLogic( )->fRidingVehicle( ) );
			fLogic( )->fSetRidingVehicle( fLogic( )->fNearestVehicle( ).fGetRawPtr( ) );
			tVehicleLogic* vehicle = fLogic( )->fRidingVehicleLogic( );
			const tVehicleSeat* seat = &vehicle->fSeat( fLogic( )->fSeatIndex( ) );
			tEntity *entity = fLogic( )->fOwnerEntity( );

			if( mIntoVehicle )
			{
				if( oldVehicle && !mSwitchingSeats )
				{
					// reusing same vehicle
					mSwitchingSeats = true;
					tVehicleLogic* veh = oldVehicle->fLogicDerived<tVehicleLogic>( );
					sigassert( veh );
					veh->fSwitchSeats( veh->fSeatIndex( fLogic( )->fTeamPlayer( ) ), fLogic( )->fSeatIndex( ) );
				}
				else
				{
					// new vehicle
					b32 result = vehicle->fTryToUse( fLogic( )->fTeamPlayer( ), fLogic( )->fSeatIndex( ) );
					sigassert( result );
				}

				sigassert( seat->mSeatPoint );
				entity->fReparentImmediate( *seat->mSeatPoint );
			}
			else
			{
				if( vehicle->fEndUse( *fLogic( )->fTeamPlayer( ) ) )
					vehicle->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_CONTROL_END ) );
			}

			AI::tSigAIGoal::fOnActivate( logic );
			sigassert( mAnimSet );

			mCurrentPosition = tPRSXformf( entity->fParentRelative( ) );
			mInitialPosition = mCurrentPosition.mP;
			fLogic( )->fSeatRelativeXform( ) = entity->fParentRelative( );

			if( mIntoVehicle )
			{
				mEndPosition = tPRSXformf::cIdentity;
			}
			else
			{
				const tMat3f& exitLocation = seat->mEnterPoint->fObjectToWorld( );
				const tMat3f& seatInv = seat->mSeatPoint->fWorldToObject( );
				mEndPosition = tPRSXformf( seatInv * exitLocation );
			}
		}

		virtual void fOnSuspend( tLogic* logic )
		{
			AI::tSigAIGoal::fOnSuspend( logic );

			if( !mIntoVehicle )
			{
				tVehicleLogic* vehicle = fLogic( )->fRidingVehicleLogic( );
				const tVehicleSeat& seat = vehicle->fSeat( fLogic( )->fSeatIndex( ) );
				fLogic( )->fSetRidingVehicle( NULL );
				fLogic( )->fOwnerEntity( )->fReparentImmediate( *tGameApp::fInstance( ).fCurrentLevel( )->fRootEntity( ) );

				fLogic( )->fAcquireExitVehicleXform( fLogic( )->fOwnerEntity( )->fObjectToWorld( ), vehicle );
			}
			else
			{
				tVehicleLogic* vehicle = fLogic( )->fRidingVehicleLogic( );
				sigassert( vehicle );

				if( !vehicle->fSeatOccupied( cVehicleDriverSeat ) )
					fLogic( )->fSwitchSeats( cVehicleDriverSeat );
			}

			if( !mPropsToggled )
			{
				mPropsToggled = true;
				fLogic( )->fShowEquip( tUserControllableCharacterLogic::cAllEquip, !mIntoVehicle );
			}
		}

		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );

			if( !mIntoVehicle ) fComputeExitPositionCorrection( );

			tVec3f translation = fLogic( )->fQueryAnimatable( )->fAnimatedSkeleton( )->fRefFrameDelta( ).mP;
			f32 percent = translation.fDot( mAnimTranslationDir ) / mAnimTranslationLen;

			mProgress += percent * 1.25f;
			float clampedProgress = fClamp( mProgress, 0.f, 1.0f );

			mCurrentPosition.fBlendNLerpR( mEndPosition, 0.2f );
			mCurrentPosition.mP += translation;
			mCurrentPosition.fToMatrix( fLogic( )->fSeatRelativeXform( ) );
			fLogic( )->fSeatRelativeXform( ).fTranslateLocal( mPositionCorrection * clampedProgress );

			//logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( logic->fOwnerEntity( )->fObjectToWorld( ), 1.0f, 0.25f );
		}

		void fSetTransitionAnim( const tKeyFrameAnimation* anim )
		{
			tPRSXformf startXform, endXform;
			anim->mReferenceFrame.fFirstFrameXform( startXform );
			anim->mReferenceFrame.fLastFrameXform( endXform );
			mAnimTranslationDir = endXform.mP - startXform.mP;

			// Should be parented to seat by now.
			// This is only used for entrance, exit will override this, below.
			tVec3f currentDelta = fLogic( )->fOwnerEntity( )->fParentRelative( ).fGetTranslation( );
			mPositionCorrection = -(currentDelta + mAnimTranslationDir);

			mAnimTranslationDir.fNormalizeSafe( mAnimTranslationLen );
			mAnimSet = true;
		}

		void fComputeExitPositionCorrection( )
		{
			tVehicleLogic* vehicle = fLogic( )->fRidingVehicleLogic( );
			const tVehicleSeat& seat = vehicle->fSeat( fLogic( )->fSeatIndex( ) );
			const tMat3f& exitLocation = seat.mEnterPoint->fObjectToWorld( );
			const tMat3f& seatInv = seat.mSeatPoint->fWorldToObject( );

			tVec3f exitPosition = exitLocation.fGetTranslation( );
			tVec3f vehicleUp = vehicle->fOwnerEntity( )->fObjectToWorld( ).fYAxis( );
			tRayf ray( exitPosition + vehicleUp * Gameplay_Character_Vehicle_ExitPreRay, vehicleUp * -(Gameplay_Character_Vehicle_ExitRay + Gameplay_Character_Vehicle_ExitPreRay) );

			Physics::tGroundRayCastCallback cb( *fLogic( )->fOwnerEntity( ), GameFlags::cFLAG_GROUND );
			fLogic( )->fSceneGraph( )->fRayCast( ray, cb );

			tVec3f exitDelta;
			if( cb.mHit.fHit( ) )
			{
				tVec3f hitPoint = ray.fEvaluate( cb.mHit.mT );
				exitDelta = seatInv.fXformPoint( hitPoint );

				//{
				//	//debugging
				//	tMat3f xform;
				//	xform.fSetTranslation( hitPoint );
				//	xform.fOrientYWithZAxis( cb.mHit.mN, exitLocation.fZAxis( ) );
				//	fLogic( )->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( xform, 1.f, 1.f );
				//}
			}
			else
				exitDelta = seatInv.fXformPoint( exitLocation.fGetTranslation( ) );

			mPositionCorrection = exitDelta - mInitialPosition - mAnimTranslationDir * mAnimTranslationLen;
		}

		virtual b32	fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e )
		{
			switch( e.fEventId( ) )
			{
			case GameFlags::cEVENT_ANIMATION:
				{
					const tKeyFrameEventContext* context = e.fContext< tKeyFrameEventContext >( );
					sigassert( context );

					if( !mPropsToggled )
					{
						if( context->mEventTypeCppValue == GameFlags::cKEYFRAME_EVENT_TOGGLE_PROPS )
						{
							mPropsToggled = true;
							fLogic( )->fShowEquip( tUserControllableCharacterLogic::cAllEquip, !mIntoVehicle );
						}
					}

					sigassert( fLogic( )->fInVehicle( ) );
					if( !mSwitchingSeats && context->mEventTypeCppValue == GameFlags::cKEYFRAME_EVENT_DOOR_POP )
						fLogic( )->fRidingVehicleLogic( )->fSeat( fLogic( )->fSeatIndex( ) ).fOpenCloseDoor( true );
					else if( context->mEventTypeCppValue == GameFlags::cKEYFRAME_EVENT_DOOR_CLOSE )
						fLogic( )->fRidingVehicleLogic( )->fSeat( fLogic( )->fSeatIndex( ) ).fOpenCloseDoor( false );
				}
				break;
			default:
				return AI::tSigAIGoal::fHandleLogicEvent( logic, e );
			}
			return true;
		}
	};

	class tCharacterVehicleRidingGoal : public AI::tSigAIGoal, public tUserCharacterGoalHelper
	{
		define_dynamic_cast(tCharacterVehicleRidingGoal, AI::tSigAIGoal);
	public:
		tCharacterVehicleRidingGoal( ) 
		{ }

		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );

			// Should be close enough to not need this
			//fLogic( )->fSeatRelativeXform( ) = tMat3f::cIdentity;
		}

		virtual void fOnSuspend( tLogic* logic )
		{
			AI::tSigAIGoal::fOnSuspend( logic );
		}

		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			const Input::tGamepad& gp = fLogic( )->fTeamPlayer( )->fGamepad( );
			fLogic( )->fHandleEnterExitVehicle( gp );

			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}
	};


	static void fExportCharacterGoalsScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tInfantryUserIdleFireGoal, AI::tSigAIGoal, Sqrat::NoCopy<tInfantryUserIdleFireGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("InfantryUserIdleFireGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tInfantryUnderUserControlGoal, AI::tSigAIGoal, Sqrat::NoCopy<tInfantryUnderUserControlGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("InfantryUnderUserControlGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tInfantryUnderAIControlGoal, AI::tSigAIGoal, Sqrat::NoCopy<tInfantryUnderAIControlGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("InfantryUnderAIControlGoal"), classDesc);
		}

		// Extra stuff
		{
			Sqrat::DerivedClass<tCharacterVehicleTransitionGoal, AI::tSigAIGoal, Sqrat::NoCopy<tCharacterVehicleTransitionGoal> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("IntoVehicle"),		&tCharacterVehicleTransitionGoal::mIntoVehicle)
				.Func(_SC("SetTransitionAnim"),	&tCharacterVehicleTransitionGoal::fSetTransitionAnim)
				;
			vm.fRootTable( ).Bind(_SC("CharacterVehicleTransitionGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tCharacterVehicleRidingGoal, AI::tSigAIGoal, Sqrat::NoCopy<tCharacterVehicleRidingGoal> > classDesc( vm.fSq( ) );
			classDesc
				;
			vm.fRootTable( ).Bind(_SC("CharacterVehicleRidingGoal"), classDesc);
		}
	}
	
}

namespace Sig
{
	namespace
	{

		tSprungMassRef fGetParentSprungMass( tUserControllableCharacterLogic* ptr )
		{
			return tSprungMassRef( ptr->fGetBaseVehAccVec( ) );
		}
	}

	void tUserControllableCharacterLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tUserControllableCharacterLogic, tCharacterLogic, Sqrat::NoCopy<tUserControllableCharacterLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("TryToUse"),				&tUserControllableCharacterLogic::fTryToUse)
			.Func(_SC("EndUse"),				&tUserControllableCharacterLogic::fEndUseScript)
			.Func(_SC("ShowWaitTimer"),			&tUserControllableCharacterLogic::fShowWaitTimer)
			.Func(_SC("ActivateWeapons"),		&tUserControllableCharacterLogic::fActivateWeapons)
			.Prop(_SC("IKLegs"),				&tUserControllableCharacterLogic::fGetIKLegs)
			.Func(_SC("CommandoAudio" ),		&tUserControllableCharacterLogic::fCommandoAudio)
			.Prop(_SC("Sprinting"),				&tUserControllableCharacterLogic::fSprinting, &tUserControllableCharacterLogic::fSetSprinting)
			.Func(_SC("Respawn"),				&tUserControllableCharacterLogic::fRespawn)
			.Var(_SC("VehicleBasePath"),		&tUserControllableCharacterLogic::mVehicleBasePath)
			.Var(_SC("VehicleBaseStarted"),		&tUserControllableCharacterLogic::mVehicleBaseStarted)
			.GlobalFunc(_SC("ParentSprungMass"),&fGetParentSprungMass )
			;

		vm.fRootTable( ).Bind(_SC("UserControllableCharacterLogic"), classDesc);

		vm.fConstTable( ).Const(_SC("COMMANDO_AUDIO_DEATH"), (int)cCommandoAudioDeath);
		
		fExportCharacterGoalsScriptInterface( vm );
	}
}

