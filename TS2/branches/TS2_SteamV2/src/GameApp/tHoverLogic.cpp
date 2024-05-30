#include "GameAppPch.hpp"
#include "tHoverLogic.hpp"
#include "tGameApp.hpp"
#include "tSceneGraph.hpp"
#include "tUseHoverCamera.hpp"
#include "tHoverPathFollowing.hpp"
#include "tLevelLogic.hpp"
#include "Physics/tGroundRayCastCallback.hpp"

#include "Wwise_IDs.h"

using namespace Sig::Math;
using namespace Sig::Physics;

namespace Sig
{	
	namespace 
	{
		
		enum tHoverTableParams
		{
			cHoverTableParamMaxRoll,
			cHoverTableParamMaxSpeed,
			cHoverTableParamAIMaxThrottle, 
			cHoverTableParamAIThrottleBlend,
			cHoverTableParamSpeedAcc,
			cHoverTableParamSpeedDamping,
			cHoverTableParamYawAcc,
			cHoverTableParamYawDamping,
			cHoverTableParamYawAccAI,
			cHoverTableParamYawDampingAI,
			cHoverTableParamRollP,
			cHoverTableParamRollD,
			cHoverTableParamYawRoll,
			cHoverTableParamPreYaw,
			cHoverTableParamMinStrafeInfluence,
			cHoverTableParamCameraLerp,
			cHoverTableParamCameraPitchMin,
			cHoverTableParamCameraPitchMax,
			cHoverTableParamMinGroundHeight,
			cHoverTableParamElevationVelMax,
			cHoverTableParamElevationVelAcc,
			cHoverTableParamElevationVelDamping
		};

		const tStringPtr cPhysicsTableName( "HELICOPTER" );

		static const tStringPtr cVehicleShape( "vehicleShape" ); 	
		static const tStringPtr cCollisionProbe( "collisionProbe" ); 	

		devvar( f32, Gameplay_Hover_FlightBoxSpring, 0.589f );
		devvar( f32, Gameplay_Hover_FlightBoxDamper, 1.95f );
	}

	tHoverLogic::tHoverLogic( )
	{
		mCameraLerp = 0.f;
		mThrottleScaleTarget = 0.5f;
	}
	void tHoverLogic::fOnDelete( )
	{
		mCollisionProbe.fRelease( );
		tVehicleLogic::fOnDelete( );
	}

	void tHoverLogic::fSetupVehicle( )
	{
		Physics::tHoverProperties props;
		props.mGroundMask = GameFlags::cFLAG_GROUND;

		fApplyTableProperties( props );
		mPhysics.fSetProperties( props );
		mPhysics.fReset( fOwnerEntity( )->fObjectToWorld( ) );
		mCollisionBounds = tAabbf( tVec3f(-1), tVec3f(1) );

		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			const tEntityPtr& child = fOwnerEntity( )->fChild( i );
			
			if( child->fName( ) == cVehicleShape )
			{
				tShapeEntity* shapeEnt = child->fDynamicCast<tShapeEntity>( );
				sigassert( shapeEnt );
				mDoCollisionTest = true;
				mCollisionBounds = tAabbf( child->fParentRelative( ).fXformPoint( tVec3f(-1,-1,-1) ), child->fParentRelative( ).fXformPoint( tVec3f(1,1,1) ) );
				mCollisionShape.fReset( shapeEnt );
			}
			else if( child->fName( ) == cCollisionProbe )
			{
				mCollisionProbe = child;
			}
		}		

		// make a sphere inside of the collision bounds. It is this sphere that will actually collide and slide along obstacles
		tSpheref collisionshape( mCollisionBounds.fComputeCenter( ), mCollisionBounds.fComputeDiagonal( ).fMinMagnitude( ) * 0.5f );
		mPhysics.fSetCollisionShape( collisionshape );
	}

	void tHoverLogic::fApplyTableProperties( Physics::tHoverProperties &props )
	{
		const tStringHashDataTable* params = tGameApp::fInstance( ).fUnitsPhysicsTable( mCountry ).fFindTable( cPhysicsTableName );
		log_assert( params, "No hover properties loaded!" );

		u32 row = params->fRowIndex( GameFlags::fUNIT_IDEnumToValueString( fUnitID( ) ) );
		log_assert( row != ~0, "No Helicopter physics properties found for: " << GameFlags::fUNIT_IDEnumToValueString( fUnitID( ) ) );

		props.mGroundMask		= GameFlags::cFLAG_GROUND;
		props.mMaxRoll			= fToRadians( params->fIndexByRowCol<f32>( row, cHoverTableParamMaxRoll ) );
		props.mMaxSpeed			= params->fIndexByRowCol<f32>( row, cHoverTableParamMaxSpeed );
		props.mAIMaxThrottle	= params->fIndexByRowCol<f32>( row, cHoverTableParamAIMaxThrottle );
		props.mSpeedAcc			= params->fIndexByRowCol<f32>( row, cHoverTableParamSpeedAcc );
		props.mSpeedDamping		= params->fIndexByRowCol<f32>( row, cHoverTableParamSpeedDamping );
		props.mYawAcc			= fToRadians( params->fIndexByRowCol<f32>( row, cHoverTableParamYawAcc ) );
		props.mYawDamping		= params->fIndexByRowCol<f32>( row, cHoverTableParamYawDamping );
		props.mYawAccAI			= fToRadians( params->fIndexByRowCol<f32>( row, cHoverTableParamYawAccAI ) );
		props.mYawDampingAI		= params->fIndexByRowCol<f32>( row, cHoverTableParamYawDampingAI );
		props.mLandingHeight	= -tMeshEntity::fCombinedObjectSpaceBox( *fOwnerEntity( ) ).mMin.y;
		props.mRollP			= params->fIndexByRowCol<f32>( row, cHoverTableParamRollP );
		props.mRollD			= params->fIndexByRowCol<f32>( row, cHoverTableParamRollD );
		props.mYawRoll			= params->fIndexByRowCol<f32>( row, cHoverTableParamYawRoll );
		props.mPreYaw			= params->fIndexByRowCol<f32>( row, cHoverTableParamPreYaw );
		props.mMinStrafeInfluence = params->fIndexByRowCol<f32>( row, cHoverTableParamMinStrafeInfluence );
		props.mElevationVelMax = params->fIndexByRowCol<f32>( row, cHoverTableParamElevationVelMax );
		props.mElevationVelAcc = params->fIndexByRowCol<f32>( row, cHoverTableParamElevationVelAcc );
		props.mElevationVelDamping = params->fIndexByRowCol<f32>( row, cHoverTableParamElevationVelDamping );
		props.mMinGroundHeight	= params->fIndexByRowCol<f32>( row, cHoverTableParamMinGroundHeight );

		mCameraLerp				= params->fIndexByRowCol<f32>( row, cHoverTableParamCameraLerp );
		f32 camPitchMin			= fToRadians( params->fIndexByRowCol<f32>( row, cHoverTableParamCameraPitchMin ) );
		f32 camPitchMax			= fToRadians( params->fIndexByRowCol<f32>( row, cHoverTableParamCameraPitchMax ) );
		mCameraMovement.fSetMinMaxPitch( camPitchMin, camPitchMax );

		mThrottleBlend = params->fIndexByRowCol<f32>( row, cHoverTableParamAIThrottleBlend );

		mUnitPath->fSetDistanceTolerance( 2.f );
	}

	void tHoverLogic::fReapplyTable( )
	{
		Physics::tHoverProperties props = mPhysics.fProperties( );
		fApplyTableProperties( props );
		mPhysics.fSetProperties( props );
	}

	void tHoverLogic::fComputeUserInput( f32 dt )
	{
		profile( cProfilePerfVehicleLogicUserST );

		if( fSeatOccupied( cVehicleDriverSeat ) )
		{
			tPlayer* player = fGetPlayer( cVehicleDriverSeat );
			Physics::tHoverInput input;
			input.fZero( );
			input.mActive = !mStartingUp;
			input.mStartingUp = mStartingUp;
			input.mUserControl = true;

			if( !fHasWeaponStation( cVehicleDriverSeat ) || !fWeaponStation( cVehicleDriverSeat )->fShellCaming( ) )
			{
				const tGameControllerPtr gc = fGetPlayer( cVehicleDriverSeat )->fGameController( );

				tVec2f aimStick = gc->fAimStick( tUserProfile::cProfileVehicles );
				if( gc->fMode( ) == tGameController::KeyboardMouse )
				{
					aimStick *= 0.9f;
				}
				const tVec2f moveStick = gc->fMoveStick( tUserProfile::cProfileVehicles );

				// Rotate camera
				mCameraMovement.fSetSpeed( fUseCamRotSpeed( ) );
				mCameraMovement.fSetDamping( fUnitAttributeUseCamRotDamping( ).fXY( ) );
				mCameraMovement.fUpdate( dt, aimStick );

				// hover input is relative to flat camera, this code is a bit sketchy, it should not be doing an inverse transform, and there should not be a need to negate
				//  see tWheeledVehicleLogic: tvec3f stick3d; to see how this should work :(
				tVec3f stick3d( moveStick.x, 0, moveStick.y );
				tVec3f strafe3d = mCameraMovement.fFlatCamera( ).fInverseXformVector( stick3d );
				strafe3d.x *= -1.f;

				if( gc->fButtonHeld( tUserProfile::cProfileVehicles, GameFlags::cGAME_CONTROLS_DECREASE_ALTITUDE ) ) input.mHeightAdjustment -= 1.f;
				if( gc->fButtonHeld( tUserProfile::cProfileVehicles, GameFlags::cGAME_CONTROLS_INCREASE_ALTITUDE ) ) input.mHeightAdjustment += 1.f;

				// rectify by airspace
				const tShapeEntityPtr& airSpace = tGameApp::fInstance( ).fCurrentLevelDemand( )->fAirSpace( fTeam( ) );
				if( airSpace )
				{
					tVec3f pos = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) + mPhysics.fVelocity( ) * (dt * Gameplay_Hover_FlightBoxDamper);
					if( !airSpace->fContains( pos ) )
					{
						tVec3f closest = airSpace->fBox( ).fClosestPoint( pos );
						tVec3f delta = closest - pos;
						f32 mag;
						delta.y = 0;
						delta.fNormalizeSafe( tVec3f::cZeroVector, mag );

						//f32 invalid = delta.fDot( strafe3d );
						//if( invalid < 0.f )
						strafe3d += delta * mag * Gameplay_Hover_FlightBoxSpring;
						strafe3d.fNormalizeSafe( tVec3f::cZeroVector );

						if( pos.y > closest.y )
							input.mHeightAdjustment = fMin( 0.f, input.mHeightAdjustment );
					}
				}

				input.mWorldStrafe = strafe3d.fXZ( );
			}

			input.mIntendedHeading = mCameraMovement.fFlatCamera( ).fZAxis( ).fXZHeading( );
			mPhysics.fSetInput( input );
		}

		tVehicleLogic::fComputeUserInput( dt );
	}

	void tHoverLogic::fComputeAIInput( f32 dt )
	{
		profile( cProfilePerfVehicleLogicAIST );

		tHoverInput input;
		input.fZero( );

		if( mUnitPath->fHasWaypoints( ) )
		{
			tGrowableArray<tVehicleOffender> offenders;
			tHoverPathFollower::fComputeInput( input, *this, *mUnitPath.fGetRawPtr( ), offenders, dt, &fSceneGraph( )->fDebugGeometry( ) );
		}

		mPhysics.fSetInput( input );
		tVehicleLogic::fComputeAIInput( dt );
	}

	void tHoverLogic::fPushCamera( tPlayer* player, u32 seat )
	{
		fResetCameraBasis( player );
		player->fPushCamera( Gfx::tCameraControllerPtr( NEW tUseHoverCamera( *player, *this ) ) );
	}

	void tHoverLogic::fPopCamera( tPlayer* player )
	{
		player->fCameraStack( ).fPopCamerasOfType<tUseHoverCamera>( );
	}

	void tHoverLogic::fResetCameraBasis( tPlayer* player )
	{
		sigassert( player );
		tVec3f dir = player->fUser( )->fViewport( )->fRenderCamera( ).fGetTripod( ).fLookDelta( ).fProjectToXZ( );
		dir.fNormalizeSafe( fOwnerEntity( )->fObjectToWorld( ).fZAxis( ) );
		mCameraMovement.fReset( dir );
	}

	void tHoverLogic::fRespawn( const Math::tMat3f& tm )
	{
		mPhysics.fReset( tm );
		fOwnerEntity( )->fMoveTo( tm );
		tVehicleLogic::fRespawn( tm );
	}
	
	tVec3f tHoverLogic::fComputeContactResponse( tUnitLogic* theirUnit, const Physics::tContactPoint& cp, f32 mass, b32& ignore ) 
	{
		if( theirUnit && theirUnit->fUnitType( ) == GameFlags::cUNIT_TYPE_AIR )
		{
			ignore = true;
			return tVec3f::cZeroVector;
		}

		return mPhysics.fComputeContactResponse( cp, mass );
	}

	void tHoverLogic::fAnimateMT( f32 dt )
	{
		profile( cProfilePerfVehicleLogicAnimateMT );

		dt *= fTimeScale( );
		mAnimatable.fAnimateMT( dt );

		tMat3f xform = fOwnerEntity( )->fObjectToWorld( );

		if( mStartingUp && mAnimatable.fHasSkeleton( ) )
		{
			tPRSXformf delta = mAnimatable.fAnimatedSkeleton( )->fRefFrameDelta( );

			tVec3f vel = delta.mP / dt;
			mPhysics.fSetVelocity( vel );

			delta.fApplyAsRefFrameDelta( xform, 1.f );
		}

		mPhysics.fSetTransform( xform );
	}

	void tHoverLogic::fPhysicsMT( f32 dt )
	{
		profile( cProfilePerfVehicleLogicPhysicsMT );

		dt *= fTimeScale( );
		mPhysics.fPhysicsMT( this, dt );

		fUpdateCharactersMT( dt );

		tVehicleLogic::fPhysicsMT( dt );
	}

	void tHoverLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfVehicleLogicMoveST );

		dt *= fTimeScale( );
		mAnimatable.fMoveST( dt );

		const tMat3f &transform = mPhysics.fTransform( );
		fOwnerEntity( )->fMoveTo( transform );

		mCameraMovement.fCameraBasis( ).fSetTranslation( transform.fGetTranslation( ) );

		//fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tObbf( f OwnerEntity( )->fCombinedObjectSpaceBox( ), fOwnerEntity( )->fObjectToWorld( ) ), tVec4f( 1,0,0,0.5f ) );

		tVehicleLogic::fMoveST( dt );
	}

	void tHoverLogic::fThinkST( f32 dt )
	{
		profile( cProfilePerfVehicleLogicThinkST );
		dt *= fTimeScale( );

		f32 audioSpeed = mPhysics.fGetSpeed( ) / mPhysics.fGetMaxSpeed( );
		f32 audioDrive = mPhysics.fLoad( );
		mAudio->fSetGameParam( AK::GAME_PARAMETERS::SPEED, audioSpeed );
		mAudio->fSetGameParamSmooth( AK::GAME_PARAMETERS::DRIVE, audioDrive, fAudioDriveBlend( ) );
		mAudio->fSetGameParam( AK::GAME_PARAMETERS::VEHICLE_ROLL, fAbs( mPhysics.fRollPos( ).x ) );
		mAudio->fSetGameParam( AK::GAME_PARAMETERS::VEHICLE_PITCH, fAbs( mPhysics.fRollPos( ).y ) );

		if( fUnitType( ) == GameFlags::cUNIT_TYPE_BOSS )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			if( level )
			{
				f32 groundHeight = level->fGroundHeight( );
				f32 height = mPhysics.fTransform( ).fGetTranslation( ).y - groundHeight;
				mAudio->fSetGameParam( AK::GAME_PARAMETERS::HEIGHT, height );
			}
		}

		if( fUnderUserControl( ) && fHasWeaponStation( 0 ) && fShowStats( ) )
		{
			//DEBUGGING
			const Gui::tWeaponUIPtr& ui = fWeaponStation( 0 )->fUI( );
			if( ui ) ui->fSetVehicleStats( audioSpeed, 0.0f, audioDrive * 0.5f );
		}

		if( fUnderUserControl( ) )
		{
			if( tPlayer::fIsAttackCopter( fUnitID( ) ) )
				fGetPlayer( cVehicleDriverSeat )->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_USING_ATTACK_HELICOPTER, dt );
			else if( tPlayer::fIsGunship( fUnitID( ) ) )
				fGetPlayer( cVehicleDriverSeat )->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_USING_HELICOPTER_GUNSHIP, dt );
		}

		tVehicleLogic::fThinkST( dt );
	}

	void tHoverLogic::fCoRenderMT( f32 dt )
	{
		profile( cProfilePerfVehicleLogicCoRenderMT );
		dt *= fTimeScale( );

		//tHoverPhysics::fCoRenderMT( this, dt );

		//test for ground pen
		if( mCollisionProbe )
		{
			const f32 probeLength = 100.0f;

			tRayf ray;
			ray.mExtent = tVec3f::cYAxis * -probeLength;
			ray.mOrigin = mCollisionProbe->fObjectToWorld( ).fGetTranslation( ) - ray.mExtent;

			tGroundRayCastCallback cb( *fOwnerEntity( ), GameFlags::cFLAG_GROUND );
			fSceneGraph( )->fRayCastAgainstRenderable( ray, cb );
			if( cb.mHit.fHit( ) )
			{
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_LAND ) );
			}
		}

		tVehicleLogic::fCoRenderMT( dt );
	}

	b32 tHoverLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_UNIT_DESTROYED:
			{
				mPhysics.fDitch( true );
			}
			break;
		}
		return tVehicleLogic::fHandleLogicEvent( e );
	}
	
	void tHoverLogic::fReactToDamage( const tDamageContext& dc, const tDamageResult& dr )
	{
		if( dr.mDestroysYou ) 
			fEjectAllPassengers( mPhysics.fVelocity( ) + dr.mSpawnInfluence );

		tUnitLogic::fReactToDamage( dc, dr );
	}

	void tHoverLogic::fReactToWeaponFire( const tFireEvent& event )
	{

	}

}




namespace Sig
{
	void tHoverLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tHoverLogic, tVehicleLogic, Sqrat::NoCopy<tHoverLogic> > classDesc( vm.fSq( ) );
		classDesc
			;

		vm.fRootTable( ).Bind(_SC("HoverLogic"), classDesc);
	}
}

