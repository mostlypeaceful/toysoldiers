#include "GameAppPch.hpp"
#include "tAirborneLogic.hpp"
#include "tGameApp.hpp"
#include "tSceneGraph.hpp"
#include "tUseAirborneCamera.hpp"
#include "tAirbornePathFollowing.hpp"
#include "tLevelLogic.hpp"
#include "tWaypointLogic.hpp"
#include "tGameEffects.hpp"
#include "tSync.hpp"
#include "tFxFileRefEntity.hpp"
#include "Wwise_IDs.h"

using namespace Sig::Math;
using namespace Sig::Physics;

namespace Sig
{	
	namespace 
	{
		
		enum tAirborneTableParams
		{
			cAirborneTableParamMaxRoll,
			cAirborneTableParamRollP,
			cAirborneTableParamRollD,
			cAirborneTableParamElevationLerp,
			cAirborneTableParamElevationRate,
			cAirborneTableParamElevationDamping,
			cAirborneTableParamYawRate,
			cAirborneTableParamYawDamping,
			cAirborneTableParamCruiseSoeed,
			cAirborneTableParamMaxSpeed,
			cAirborneTableParamSlowSpeed,
			cAirborneTableParamAccelerationLerp,
			cAirborneTableParamZoomMax,
			cAirborneTableParamZoomSlow,
			cAirborneTableParamAIYawDamping,
			cAirborneTableParamAIThrottle,
			cAirborneTableParamAIThrottleBlend
		};

		static const tStringPtr cPhysicsTableName( "AIRBORNE" );
		static const tStringPtr cCollisionProbeName( "collisionProbe" );
		static const tStringPtr cConTrail( "conTrail" );
		static const tStringPtr cSonicBoomEffect( "airborne_sonic_boom" );
		static const tStringPtr cJetEngineAttachmentName( "jet_engine" );
		static const tStringPtr cEngineStartupEffect( "fighter_jet_takeoff_rumbles" );		

		static const u32 cPlayerCollisionMask = GameFlags::cFLAG_COLLISION | GameFlags::cFLAG_GROUND;
		static const u32 cAICollisionMask = GameFlags::cFLAG_GROUND;

		enum tSpecialAnims
		{
			cSpecialAnimRollLeft,
			cSpecialAnimRollRight,
			cSpecialAnim180,
			cSpecialAnimCount
		};

		devvar( f32, Gameplay_Airborne_NewPathTimeout, 1.5f );
		devvar( f32, Gameplay_Airborne_SpeedUpTimer, 4.0f );
		devvar( f32, Gameplay_Airborne_SpeedUpSpeed, 0.6f );
		devvar( f32, Gameplay_Airborne_SpecialMoveBlendSpeed, 0.2f );
		devvar( f32, Gameplay_Airborne_RandomFlyTime, 20.0f );
		devvar( f32, Gameplay_Airborne_SpecialMoveTiming, 0.05f );	

		devvar( f32, Gameplay_Airborne_FlightBoxSpring, 0.589f );
		devvar( f32, Gameplay_Airborne_FlightBoxDamper, 1.95f );	

	}

	tAirborneLogic::tAirborneLogic( )
		: mInBombCam( false )
		, mSpecialMove( false )
		, mSpecialMoveInputLock( false )
		, mSpecialMoveInputDisabled( false )
		, mOnFinalGoalPath( false )
		, mDisableEvasion( false )
		, mLockedInBombCam( false )
		, mDontDitchOnExit( false )
		, mWasFullSpeed( false )
		, mEnginesStarted( false )
		, mRandomFlyTimeRemaining( Gameplay_Airborne_RandomFlyTime )
		, mSpeedUpTimer( -1.f )
		, mNewPathTimeout( -1.f )
		, mConTrailIntensity( 0.f )
		, mDeathState( -1 )
		, mSpecialMoveInputTimer( 0.f )
	{
		mDoCollisionTest = true;
		mThrottleScaleTarget = 0.5f;
		mDontInstaDestroyOutsideOfFlyBox = true;
		mBoostEffect = cSonicBoomEffect;	// this isn't the actual used boost fx, that's handled thru tJetEngineFx, this just needs to be set so the boost-system will work still...gah, so bad.
	}

	void tAirborneLogic::fOnDelete( )
	{
		for( u32 i = 0; i < mConTrails.fCount( ); ++i )
			mConTrails[ i ]->fStopTrackingParent( ) ;
		mConTrails.fSetCount( 0 );

		mAudio->fHandleEvent( AK::EVENTS::STOP_AIRPLANE_DIVE );

		tVehicleLogic::fOnDelete( );
	}

	void tAirborneLogic::fSetupVehicle( )
	{
		if( fHasSelectionFlag( ) )
		{
			f32 heightShift = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ).y - tMeshEntity::fCombinedWorldSpaceBox( *fOwnerEntity( ) ).mMin.y;
			fOwnerEntity( )->fTranslate( Math::tVec3f( 0, heightShift, 0 ) );
		}

		Physics::tAirborneProperties props;
		props.mGroundMask = GameFlags::cFLAG_GROUND;

		fApplyTableProperties( props );

		tGrowableArray<tEntityPtr> conTrails;

		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			const tEntityPtr& child = fOwnerEntity( )->fChild( i );
						
			if( child->fName( ) == cCollisionProbeName )
				props.mCollisionProbes.fPushBack( child->fParentRelative( ).fGetTranslation( ) );
			else if( child->fName( ) == cConTrail )
				conTrails.fPushBack( child );
		}

		if( !props.mCollisionProbes.fCount( ) )
		{
			log_warning( 0, "No collision probe on airborne: " << GameFlags::fUNIT_IDEnumToValueString( fUnitID( ) ) );
		}

		mPhysics.fSetProperties( props );
		mPhysics.fReset( fOwnerEntity( )->fObjectToWorld( ) );

		fEnableBombCam( false );

		u32 conTracer = tGameApp::fInstance( ).fTracersTable( ).fRowIndex( mConTrailTracerName );
		if( conTracer != ~0 )
		{
			const FX::tTracerTrailDef &def = tGameApp::fInstance( ).fTracerTrailDef( conTracer );

			for( u32 i = 0; i < conTrails.fCount( ); ++i )
			{
				FX::tTracerTrailEntity* trail = NEW FX::tTracerTrailEntity( *conTrails[ i ], def );
				trail->fSpawn( *conTrails[ i ] );
				mConTrails.fPushBack( FX::tTracerTrailEntityPtr( trail ) );
				mConTrailPosLastFrame = conTrails.fFront( )->fObjectToWorld( ).fGetTranslation( ); //always the front
			}
		}

		fComputeCollisionShapeIfItDoesntExist( );
		sigassert( mCollisionShape );
		mCollisionBounds = tAabbf( mCollisionShape->fParentRelativeBox( ) );
	}

	void tAirborneLogic::fApplyTableProperties( Physics::tAirborneProperties &props )
	{
		const tStringHashDataTable* params = tGameApp::fInstance( ).fUnitsPhysicsTable( mCountry ).fFindTable( cPhysicsTableName );
		log_assert( params, "No airborne properties loaded!" );

		u32 row = params->fRowIndex( GameFlags::fUNIT_IDEnumToValueString( fUnitID( ) ) );
		log_assert( row != ~0, "No Airborne physics properties found for: " << GameFlags::fUNIT_IDEnumToValueString( fUnitID( ) ) );

		props.mMaxPitch			= cPiOver4;
		props.mMaxRoll			= fToRadians( params->fIndexByRowCol<f32>( row, cAirborneTableParamMaxRoll ) );
		props.mRollP			= params->fIndexByRowCol<f32>( row, cAirborneTableParamRollP );
		props.mRollD			= params->fIndexByRowCol<f32>( row, cAirborneTableParamRollD );
		props.mElevationLerp	= params->fIndexByRowCol<f32>( row, cAirborneTableParamElevationLerp );
		props.mElevationRate	= fToRadians( params->fIndexByRowCol<f32>( row, cAirborneTableParamElevationRate ) );
		props.mElevationDamping	= params->fIndexByRowCol<f32>( row, cAirborneTableParamElevationDamping );
		props.mYawRate			= fToRadians( params->fIndexByRowCol<f32>( row, cAirborneTableParamYawRate ) );
		props.mYawDamping		= params->fIndexByRowCol<f32>( row, cAirborneTableParamYawDamping );
		props.mCruiseSpeed		= params->fIndexByRowCol<f32>( row, cAirborneTableParamCruiseSoeed );
		props.mMaxSpeed			= params->fIndexByRowCol<f32>( row, cAirborneTableParamMaxSpeed );
		props.mSlowSpeed		= params->fIndexByRowCol<f32>( row, cAirborneTableParamSlowSpeed );
		props.mAccelerationLerp		= params->fIndexByRowCol<f32>( row, cAirborneTableParamAccelerationLerp );
		props.mZoomMax			= params->fIndexByRowCol<f32>( row, cAirborneTableParamZoomMax );
		props.mZoomSlow			= params->fIndexByRowCol<f32>( row, cAirborneTableParamZoomSlow );
		props.mAIYawDamping		= params->fIndexByRowCol<f32>( row, cAirborneTableParamAIYawDamping );
		props.mAIThrottle		= params->fIndexByRowCol<f32>( row, cAirborneTableParamAIThrottle );

		mThrottleBlend = params->fIndexByRowCol<f32>( row, cAirborneTableParamAIThrottleBlend );
	}

	void tAirborneLogic::fReapplyTable( )
	{
		Physics::tAirborneProperties props = mPhysics.fProperties( );
		fApplyTableProperties( props );
		mPhysics.fSetProperties( props );
	}

	devvar_clamp( f32, Gameplay_Airborne_ZeppelinDitchHealth, 0.10f, 0.f, Math::cInfinity, 2 );

	s32 tAirborneLogic::fComputeChangeState( const tDamageContext& dc, const tDamageResult& dr )
	{
		// Special case for zeppelin boss.
		// State 0 = normal
		// State 1 = ditching
		// State 0 = destroyed
		if( fUnitID( ) == GameFlags::cUNIT_ID_BOSS_ZEPPELIN )
		{
			// Check if we haven't started our death ditch yet and we're damaged enough.
			if( mDeathState == -1 )
			{
				if( dr.mHealthPercentEnd <= Gameplay_Airborne_ZeppelinDitchHealth )
				{
					if( dc.fAttackerPlayer( ) )
						mDestroyedByPlayer = dc.fAttackerPlayer( );

					fDitch( );
					mDeathState = 1;
					return 1;
				}

				return 0;
			}
			// If we have done our death ditch and we're completely out of hp, time to explode.
			else if( dr.mHealthPercentEnd <= 0.f )
			{
				return 2;
			}	

			return 0;
		}

		const s32 cPlusXWingOff = mLastState - 2;	// Left wing
		const s32 cMinusXWingOff = mLastState - 1;	// Right Wing
		const s32 cLastWholeState = mLastState - 3;
		sigassert( cLastWholeState >= 0 && "Airborne does not have enough states." );

		if( dr.mHealthPercentEnd <= 0.f )
		{
			return mLastState;
		}
		else if( dr.mHealthPercentEnd <= 0.2f )
		{
			if( mDeathState == -1 )
			{
				if( dr.mAttackerDirection.fDot( fOwnerEntity( )->fObjectToWorld( ).fXAxis( ) ) > 0.f )
					mDeathState = cPlusXWingOff;
				else
					mDeathState = cMinusXWingOff;

				if( dc.fAttackerPlayer( ) )
					mDestroyedByPlayer = dc.fAttackerPlayer( );

				fDitch( );
			}

			return mDeathState;
		}
		else
			return s32( cLastWholeState * (1.0f - dr.mHealthPercentEnd) );
	}

	void tAirborneLogic::fComputeUserInput( f32 dt )
	{
		profile( cProfilePerfVehicleLogicUserST );

		if( fSeatOccupied( cVehicleDriverSeat ) )
		{
			tPlayer* player = fGetPlayer( cVehicleDriverSeat );
			const tGameControllerPtr gc = player->fGameController( );
			Physics::tAirborneInput input;
			input.fZero( );

			input.mActive = !mStartingUp;

			if( input.mActive )
			{
				if( !mSpecialMoveInputLock )
				{
					const tVec2f speedStick = gc->fMoveStick( tUserProfile::cProfilePlanes );
					const tVec2f aimStick = gc->fAimStick( tUserProfile::cProfilePlanes );
					
					input.mThrottle = 1.f + speedStick.y;
					input.mStick = aimStick;
					input.mStick = Input::tGamepad::fMapStickCircleToRectangle( aimStick );

					// planes behave like turrets by default
					input.mStick.y *= -1.f;

					if( !mLockedInBombCam && gc->fButtonDown( tUserProfile::cProfilePlanes, GameFlags::cGAME_CONTROLS_CAMERA_CYCLE ) )
						fEnableBombCam( !mInBombCam );

					if( fBoostInput( true ) )
						mJetEngineFx.fEngineBoost( );

					if( mBoostTimer > 0.f )
					{
						input.mBoost = fBoostProgress( );
						input.mThrottle = 2.f;
					}
				}

				if( mInBombCam || mSpecialMoveInputLock )
				{
					//Level the plane out
					b32 discard;
					tVec2f newStick = mPhysics.fComputeStickToReachHeading( mPhysics.fTransform( ).fZAxis( ).fProjectToXZ( ), discard );
					input.mStick.y = newStick.y;
				}

				if( !mSpecialMove && !mSpecialMoveInputDisabled )
				{
					//special anim
					u32 id = ~0;
					b32 lShoulder = gc->fButtonHeld( tUserProfile::cProfilePlanes, GameFlags::cGAME_CONTROLS_DECREASE_ALTITUDE );
					b32 rShoulder = gc->fButtonHeld( tUserProfile::cProfilePlanes, GameFlags::cGAME_CONTROLS_INCREASE_ALTITUDE );

					if( lShoulder && rShoulder ) 
						id = cSpecialAnim180;	
					else if( lShoulder || rShoulder )
					{
						if( mSpecialMoveInputTimer > Gameplay_Airborne_SpecialMoveTiming )
						{
							if( lShoulder )		
								id = cSpecialAnimRollLeft;
							else if( rShoulder ) 
								id = cSpecialAnimRollRight;
						}

						mSpecialMoveInputTimer += dt;
					}
					else
						mSpecialMoveInputTimer = 0.f;

					if( id != ~0 )
					{
						// kicks the anim
						fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_DO_SPECIAL_MOVE, NEW Logic::tIntEventContext( id ) ) );

						tTutorialEvent tutEvent( GameFlags::cTUTORIAL_EVENT_SPECIAL_MOVE );
						tutEvent.mCurrentUnitID = fUnitID( );
						tutEvent.mEventValue = id;
						tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevelDemand( );
						level->fHandleTutorialEvent( tutEvent );
					}
				}
				else
					mSpecialMoveInputTimer = 0.f;


				// rectify by airspace
				const tShapeEntityPtr& airSpace = tGameApp::fInstance( ).fCurrentLevelDemand( )->fAirSpace( fTeam( ) );
				if( airSpace )
				{
					tAabbf box( airSpace->fBox( ) );
					tVec3f pos = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) + mPhysics.fVelocity( ) * (dt * Gameplay_Airborne_FlightBoxDamper);
					if( !box.fContains( pos ) )
					{
						tVec3f closest = box.fClosestPoint( pos );
						
						if( !fEqual( closest.x, pos.x ) || !fEqual( closest.z, pos.z ) ) //flew out the side
						{
							if( !mSpecialMove )
							{
								tVec3f delta = closest - pos;
								delta.fNormalizeSafe( tVec3f::cZeroVector );

								//check that we're flying out
								if( fOwnerEntity( )->fObjectToWorld( ).fZAxis( ).fDot( delta ) < 0.f )
									fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_DO_SPECIAL_MOVE, NEW Logic::tIntEventContext( cSpecialAnim180 ) ) );	
							}
						}
						
						if( pos.y > closest.y + 0.1f )
						{
							//flew out the top
							b32 discard;
							tVec3f target = mPhysics.fTransform( ).fZAxis( );
							target.y = closest.y - pos.y;
							tVec2f newStick = mPhysics.fComputeStickToReachHeading( target, discard );
							input.mStick.y = fMax( input.mStick.y, newStick.y );
						}
					}
				}
			}

			mPhysics.fSetInput( input );
		}

		tVehicleLogic::fComputeUserInput( dt );
	}

	void tAirborneLogic::fComputeAIInput( f32 dt )
	{
		tAirborneInput input;
		input.fZero( );

		if( mUnitPath->fHasWaypoints( ) )
		{
			tGrowableArray<tVehicleOffender> offenders;
			tAirbornePathFollower::fComputeInput( input, *this, *mUnitPath.fGetRawPtr( ), offenders, dt, &fSceneGraph( )->fDebugGeometry( ) );
		}

		mPhysics.fSetInput( input );
		tVehicleLogic::fComputeAIInput( dt );
	}

	void tAirborneLogic::fDitch( )
	{
		if( mStartingUp || mPhysics.fInput( ).mActive )
		{
			mEjectUser = true;
			mPhysics.fDitch( true );
			mAudio->fHandleEvent( AK::EVENTS::PLAY_AIRPLANE_DIVE );
		}
	}

	void tAirborneLogic::fEnableBombCam( b32 enable )
	{
		mInBombCam = enable;
		
		// Turn on the alternate money/combo notifications for the player
		if( fSeatOccupied( cVehicleDriverSeat ) )
		{
			tPlayer* player = fGetPlayer( cVehicleDriverSeat );
			if( player )
			{
				if( player->fScreenSpaceNotification( ) )
					player->fScreenSpaceNotification( )->fEnable( enable );
			}
		}

		// Show the alternate controls
		if( enable )
		{
			for( u32 i = 0; i < mWeaponStations.fCount( ); ++i )
			{
				 if( mWeaponStations[ i ]->fUI( ) )
				 {
					 mWeaponStations[ i ]->fUI( )->fHideControls( Gui::tWeaponUI::cMainControls );
					 mWeaponStations[ i ]->fUI( )->fShowControls( Gui::tWeaponUI::cAltControls );
				 }
			}
		}
		else
		{
			for( u32 i = 0; i < mWeaponStations.fCount( ); ++i )
			{
				if( mWeaponStations[ i ]->fUI( ) )
				{
					mWeaponStations[ i ]->fUI( )->fHideControls( Gui::tWeaponUI::cAltControls );
					mWeaponStations[ i ]->fUI( )->fShowControls( Gui::tWeaponUI::cMainControls );
				}
			}
		}

		fApplyUserRestrictedBanks( );
	}

	void tAirborneLogic::fApplyUserRestrictedBanks( )
	{
		b32 aiControl = !fUnderUserControl( );

		for( u32 i = 0; i < mEnableBombBank.fCount( ); ++i )
			fWeaponStation( 0 )->fBank( mEnableBombBank[ i ] )->fEnable( aiControl || mInBombCam );

		for( u32 i = 0; i < mDisableBombBank.fCount( ); ++i )
			fWeaponStation( 0 )->fBank( mDisableBombBank[ i ] )->fEnable( aiControl || !mInBombCam );
	}

	void tAirborneLogic::fStopSpecialMove( )
	{
		mSpecialMove = false;
		mSpecialMoveInputLock = false;
	}

	void tAirborneLogic::fStartSpecialMove( u32 move )
	{
		mSpecialMove = true;
		if( move == cSpecialAnim180 )
			mSpecialMoveInputLock = true;
	}

	void tAirborneLogic::fPushCamera( tPlayer* player, u32 seat )
	{
		player->fPushCamera( Gfx::tCameraControllerPtr( NEW tUseAirborneCamera( *player, *this ) ) );
		fEnableBombCam( false );
	}

	void tAirborneLogic::fPopCamera( tPlayer* player )
	{
		fEnableBombCam( false );
		player->fCameraStack( ).fPopCamerasOfType<tUseAirborneCamera>( );

		if( !mDontDitchOnExit )
			fDitch( );

		fSetHitPoints( 0.f );
		mStartingUp = false;
	}

	void tAirborneLogic::fRespawn( const Math::tMat3f& tm )
	{
		mPhysics.fReset( tm );
		fOwnerEntity( )->fMoveTo( tm );
		tVehicleLogic::fRespawn( tm );
	}

	void tAirborneLogic::fAnimateMT( f32 dt )
	{
		profile( cProfilePerfVehicleLogicAnimateMT );
		dt *= fTimeScale( );

		mAnimatable.fAnimateMT( dt );

		tMat3f xform = fOwnerEntity( )->fObjectToWorld( );

		if( (mStartingUp || mSpecialMove) && mAnimatable.fHasSkeleton( ) )
		{
			tPRSXformf delta = mAnimatable.fAnimatedSkeleton( )->fRefFrameDelta( );

			mAnimationBlend.fSetBlends( Gameplay_Airborne_SpecialMoveBlendSpeed );
			mAnimationBlend.fStep( 1.0f, dt );

			tVec3f vel = mPhysics.fTransform( ).fXformVector( delta.mP ) / dt;
			vel = fLerp( mPhysics.fVelocity( ), vel, mAnimationBlend.fValue( ) );
			mPhysics.fSetVelocity( vel );

			delta.fApplyAsRefFrameDelta( xform, 1.f );
		}
		else
			mAnimationBlend.fSetValue( 0.f );

		mPhysics.fSetTransform( xform );
	}

	void tAirborneLogic::fPhysicsMT( f32 dt )
	{
		profile( cProfilePerfVehicleLogicPhysicsMT );
		dt *= fTimeScale( );

		mSpeedUpTimer -= dt;
		mNewPathTimeout -= dt;

		if( mSpeedUpTimer > 0.f )
			mThrottleScaleTargetOverride = Gameplay_Airborne_SpeedUpSpeed;
		else
			mThrottleScaleTargetOverride = -1.f;

		mPhysics.fPhysicsMT( this, dt );

		fUpdateCharactersMT( dt );

		if( mConTrails.fCount( ) && mConTrails[ 0 ]->fParent( ) )
		{
			tVec3f newConTrailPos = mPhysics.fTransform( ).fXformPoint( mConTrails.fFront( )->fParent( )->fParentRelative( ).fGetTranslation( ) );
			tVec3f vel = (newConTrailPos - mConTrailPosLastFrame) / dt;
			mConTrailIntensity = mPhysics.fSpeedBlendImp( vel.fLength( ) );

			if( mConTrailIntensity < 0.2f )
				mConTrailIntensity = 0.f; //low alphas look bad.
			else 
				mConTrailIntensity = fMin( mConTrailIntensity, 1.f );

			mConTrailPosLastFrame = newConTrailPos;
		}

		tVehicleLogic::fPhysicsMT( dt );
	}

	void tAirborneLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfVehicleLogicMoveST );
		dt *= fTimeScale( );

		mAnimatable.fMoveST( dt );

		const tMat3f &transform = mPhysics.fGetTransform( );
		fOwnerEntity( )->fMoveTo( transform );

		tVehicleLogic::fMoveST( dt );

		if( mPhysics.fCollided( ) )
		{
			if( mDestroyedByPlayer )
				fDestroy( mDestroyedByPlayer, true );
			else
				fDestroy( true );
		}

		for( u32 i = 0; i < mConTrails.fCount( ); ++i )
			mConTrails[ i ]->fSetIntensity( mConTrailIntensity );
	}

	void tAirborneLogic::fThinkST( f32 dt )
	{
		profile( cProfilePerfVehicleLogicThinkST );
		dt *= fTimeScale( );

		f32 audioSpeed = mPhysics.fSpeedBlend( );
		f32 audioRPM = 0.f; //mPhysics.fSignedSpeedBlend( );
		f32 audioDrive = mPhysics.fLoad( );
		tVec2f rollPos = mPhysics.fRollPos( );
		mAudio->fSetGameParam( AK::GAME_PARAMETERS::SPEED, audioSpeed );
		mAudio->fSetGameParamSmooth( AK::GAME_PARAMETERS::DRIVE, audioDrive, fAudioDriveBlend( ) );
		mAudio->fSetGameParam( AK::GAME_PARAMETERS::VEHICLE_ROLL, fAbs( rollPos.x ) );
		mAudio->fSetGameParam( AK::GAME_PARAMETERS::VEHICLE_PITCH, fAbs( rollPos.y ) );
		
		if( fUnderUserControl( ) && fHasWeaponStation( 0 ) && fShowStats( ) )
		{
			//DEBUGGING
			const Gui::tWeaponUIPtr& ui = fWeaponStation( 0 )->fUI( );
			if( ui ) ui->fSetVehicleStats( audioSpeed, audioRPM * 0.5f + 0.5f, audioDrive * 0.5f );
		}

		mRandomFlyTimeRemaining -= dt;
		mOnFinalGoalPath = (mUnitPath->fLastStartedPathType( ) == tWaypointLogic::cGoalPath);

		const f32 cFullSpeedThresh = 0.95f;
		if( mWasFullSpeed )
		{
			if( audioSpeed < cFullSpeedThresh )
				mWasFullSpeed = false;
		}
		else
		{
			if( audioSpeed >= cFullSpeedThresh )
			{
				mWasFullSpeed = true;
				tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), cSonicBoomEffect );
			}
		}

		if( fUnderUserControl( ) )
		{
			mPhysics.fSetCollisionMask( cPlayerCollisionMask );
			if( tPlayer::fIsFighterPlane( fUnitID( ) ) )
				fGetPlayer( cVehicleDriverSeat )->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_USING_FIGHTER_PLANE, dt );
		}
		else
			mPhysics.fSetCollisionMask( cAICollisionMask );

		tVehicleLogic::fThinkST( dt );
	}

	//void tAirborneLogic::fCoRenderMT( f32 dt )
	//{
	//	profile( cProfilePerfVehicleLogicCoRenderMT );

	//	tAirbornePhysics::fCoRenderMT( this, dt );

	//	tVehicleLogic::fCoRenderMT( dt );
	//}
	
	void tAirborneLogic::fFindCollisionMT( f32 dt )
	{
	}

	b32 tAirborneLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_UNIT_DESTROYED:
			{
				mAudio->fHandleEvent( AK::EVENTS::STOP_AIRPLANE_DIVE );
			}
			break;
		case GameFlags::cEVENT_REACHED_END_OF_PATH:
			{
				if( mOnFinalGoalPath )
					return tVehicleLogic::fHandleLogicEvent( e );
				else if( !fNewPath( true ) && !fGoToGoal( ) )
					fOwnerEntity( )->fDelete( );

				// we have special end of path behavior
				return true;
			}
			break;
		case GameFlags::cEVENT_ANIMATION:
			{
				if( !mEnginesStarted )
				{
					const tKeyFrameEventContext* context = e.fContext<tKeyFrameEventContext>( );
					if( context && context->mEventTypeCppValue == GameFlags::cKEYFRAME_EVENT_ENGINE_START )
					{
						mEnginesStarted = true;
						mJetEngineFx.fEngineBoost( );
						mJetEngineFx.fEngineIgnition( );
						tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), cEngineStartupEffect );	// performs the rumbles!!!
					}
				}
			}
			break;
		}
		return tVehicleLogic::fHandleLogicEvent( e );

	}
	
	void tAirborneLogic::fReactToDamage( const tDamageContext& dc, const tDamageResult& dr )
	{
		if( dc.fAttackingTeam( ) != fTeam( )
			&& !mDisableEvasion
			&& !fUnderUserControl( )
			&& ( fUnitPath( ) && fUnitPath( )->fPathMode( ) == tUnitPath::cPathModeFollow )
			)
		{
			fAvert( );
		}

		if( dr.mDestroysYou
			&& dc.fEffectDamageType( ) == GameFlags::cDAMAGE_TYPE_BULLET
			&& dc.fAttackerPlayer( )
			&& dc.fAttacker( )
			&& tPlayer::fIsFighterPlane( dc.fAttacker( )->fUnitID( ) )
			&& tPlayer::fIsFighterPlane( fUnitID( ) )
			)
		{
			// editing the achievement so it can only be done by USA vs USSR
			if( dc.fAttackerPlayer( )->fCountry( ) == GameFlags::cCOUNTRY_USA )
			{
				dc.fAttackerPlayer( )->fAwardAchievement( GameFlags::cACHIEVEMENTS_I_CAN_T_GET_A_TONE );
			}
		}

		tUnitLogic::fReactToDamage( dc, dr );
	}

	void tAirborneLogic::fReactToWeaponFire( const tFireEvent& event )
	{
	}

	void tAirborneLogic::fAvert( )
	{
		if( !mOnFinalGoalPath )
		{
			mSpeedUpTimer = Gameplay_Airborne_SpeedUpTimer;
			if( /*fHealthPercent( ) < 0.5f || */!fNewPath( false ) )
				fGoToGoal( );
		}
	}

	void tAirborneLogic::fStartup( b32 startup )
	{
		if( !startup && !mEnginesStarted )
		{
			mEnginesStarted = true;
			mJetEngineFx.fEngineIgnition( );
		}

		tVehicleLogic::fStartup( startup );
	}

	b32 tAirborneLogic::fNewPath( b32 needNewPath )
	{
		if( !needNewPath && mNewPathTimeout > 0 )
			return true; //stay on same path;

		if( mRandomFlyTimeRemaining > 0 && !mOnFinalGoalPath )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );

			tGrowableArray<tPathEntityPtr> potentialPaths;
			for( u32 i = 0; i < level->fFlightPathStarts( ).fCount( ); ++i )
			{
				if( mUnitPath->fLastStartedPathPt( ) == level->fFlightPathStarts( )[ i ]
					|| level->fFlightPathStarts( )[ i ]->fName( ) != mUnitPath->fLastStartedPathName( ) )
					continue;

				potentialPaths.fPushBack( level->fFlightPathStarts( )[ i ] );
			}

			// if there were no new random paths, consider our current path.
			if( !potentialPaths.fCount( ) && mUnitPath->fLastStartedPathPt( ) )
				potentialPaths.fPushBack( mUnitPath->fLastStartedPathPt( ) );

			if( potentialPaths.fCount( ) )
			{
				mNewPathTimeout = Gameplay_Airborne_NewPathTimeout;

				s32 newPath = sync_rand( fIntInRange( 0, potentialPaths.fCount( ) - 1 ) ); 
				mUnitPath->fStartSimplePath( potentialPaths[ newPath ] );
				fCancelAllWeaponFire( );

				return true;
			}			
		}

		// no path to switch to
		return false;
	}

	b32 tAirborneLogic::fGoToGoal( )
	{
		if( !mOnFinalGoalPath )
		{
			mOnFinalGoalPath = true;

			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			u32 ignoreAny = ~0;
			s32 newPath = tUnitPath::fFindClosestStartPoint( fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ), level->fPathStarts( ), mUnitPath->fLastStartedPathName( ), &ignoreAny );
			if( newPath > -1 )
			{
				const tPathEntityPtr& path = level->fPathStarts( )[ newPath ];
				mUnitPath->fStartSimplePath( path );
				fCancelAllWeaponFire( );

				return true;
			}
			else
			{
				log_warning( 0, "Random flying airplane could not find a goal path with the same name: " << mUnitPath->fLastStartedPathName( ) );
				return false;
			}
		}

		return true;
	}

	void tAirborneLogic::fSetRandomFlyTimeRemaining( f32 time )
	{
		mRandomFlyTimeRemaining = time;

		if( mRandomFlyTimeRemaining <= 0.f )
			fGoToGoal( );
		else
		{
			if( mOnFinalGoalPath )
			{
				mOnFinalGoalPath = false;
				fNewPath( true );
			}
		}
	}


	class tJetEngineAttachmentSearch
	{
	public:
		tGrowableArray< tAttachmentEntityPtr >& mAttachments;
		u32 mTagToNOThave;

		tJetEngineAttachmentSearch( tGrowableArray< Sig::tAttachmentEntityPtr >& attachments )
		: mAttachments( attachments )
		{	}
		b32 operator( ) ( tEntity& e ) const
		{
			tAttachmentEntity *attach = e.fDynamicCast< tAttachmentEntity >( );
			if( attach )
			{
				if( attach->fName( ) == cJetEngineAttachmentName )
					mAttachments.fPushBack( tAttachmentEntityPtr( attach ) );
			}
			return false;
		}
	};


	void tAirborneLogic::tJetEngineFx::fInit( tEntity* airplane, const tFilePathPtr& ignition, const tFilePathPtr& boost )
	{
		mIgnitionFx = ignition;
		mBoostFx = boost;
		airplane->fForEachDescendent( tJetEngineAttachmentSearch( mEngineAttachments ) );
	}

	void tAirborneLogic::tJetEngineFx::fEngineIgnition( )
	{
		for( u32 i = 0; i < mEngineAttachments.fCount( ); ++i )
			mEngineAttachments[ i ]->fSpawnFxChild( mIgnitionFx, -1, true );
	}
	void tAirborneLogic::tJetEngineFx::fEngineBoost( )
	{
		if( mCurrentBoost && !mCurrentBoost->fReadyForDeletion( ) )
			return;
		for( u32 i = 0; i < mEngineAttachments.fCount( ); ++i )
			mCurrentBoost.fReset( mEngineAttachments[ i ]->fSpawnFxChild( mBoostFx, 1, true ) );
	}

	void tAirborneLogic::fInitializeJetEngineFx( const tFilePathPtr& ignitionFx, const tFilePathPtr& boostFx )
	{
		mJetEngineFx.fInit( fOwnerEntity( ), ignitionFx, boostFx );
	}
}




namespace Sig
{
	void tAirborneLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tAirborneLogic, tVehicleLogic, Sqrat::NoCopy<tAirborneLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("AddEnableBombBank"),			&tAirborneLogic::fAddEnableBombBank)
			.Func(_SC("AddDisableBombBank"),		&tAirborneLogic::fAddDisableBombBank)
			.Func(_SC("SetRandomFlyTimeRemaining"),	&tAirborneLogic::fSetRandomFlyTimeRemaining)
			.Func(_SC("GoToGoal"),					&tAirborneLogic::fGoToGoal)
			.Func(_SC("InitializeJetEngineFx"),		&tAirborneLogic::fInitializeJetEngineFx)
			.Prop(_SC("DisableEvasion"),			&tAirborneLogic::fDisableEvasion, &tAirborneLogic::fSetDisableEvasion)
			.Prop(_SC("SpecialMoveInputDisabled"),	&tAirborneLogic::fSpecialMoveInputDisabled, &tAirborneLogic::fSetSpecialMoveInputDisabled)
			.Var(_SC("ConTrailTracerName"),			&tAirborneLogic::mConTrailTracerName)
			
			;

		vm.fRootTable( ).Bind(_SC("AirborneLogic"), classDesc);

		vm.fConstTable( ).Const(_SC("SPECIAL_ANIM_ROLL_LEFT"), cSpecialAnimRollLeft);
		vm.fConstTable( ).Const(_SC("SPECIAL_ANIM_ROLL_RIGHT"), cSpecialAnimRollRight);
		vm.fConstTable( ).Const(_SC("SPECIAL_ANIM_180"), cSpecialAnim180);
	}
}

