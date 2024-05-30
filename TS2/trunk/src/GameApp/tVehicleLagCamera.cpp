#include "GameAppPch.hpp"
#include "tVehicleLagCamera.hpp"
#include "tGameApp.hpp"
#include "tWheeledVehicleLogic.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( bool, Gameplay_Vehicle_LagCamera_EnableDynamics, true );
	devvar( f32, Gameplay_Vehicle_LagCamera_LookInScale, 1.0f );
	devvar( f32, Gameplay_Vehicle_LagCamera_WSubtract, 1.0f );
	devvar( f32, Gameplay_Vehicle_LagCamera_WScale, 2.0f );
	devvar( f32, Gameplay_Vehicle_LagCamera_VDamp, 1.0f );
	devvar( f32, Gameplay_Vehicle_LagCamera_VScale, 1.0f );
	devvar( f32, Gameplay_Vehicle_LagCamera_DistLerp, 0.0f );

	devvar( f32, Gameplay_Vehicle_LagCamera_DirectionTorque, 1.0f );
	devvar( f32, Gameplay_Vehicle_LagCamera_DirectionTorqueUser, 300.0f );
	devvar( f32, Gameplay_Vehicle_LagCamera_DirectionTorqueCentering, 600.f );
	devvar( f32, Gameplay_Vehicle_LagCamera_BoostZoom, 2.4f );



	tVehicleLagCamera::tVehicleLagCamera( tPlayer& player, u32 seat, tWheeledVehicleLogic& vehicleLogic )
		: tUseUnitCamera( player, &vehicleLogic, false )
		, mVehicle( vehicleLogic )
		, mSeat( seat )
		, mZoomDist( 0.0f, 10.0f, 5.f )
		, mZoomBlend( 0.0f, 10.0f, 5.f )
		, mLook( 0.0f, 7.5f, 6.f )
		, mLookTorque( 0.f, 10.f, 5.f )
		, mInitialized( false )
		, mCenterLook( false )
	{
		mInitialZoom = mTargetZoom;
		mRaycastTerrainPenetration = true;
	}

	void tVehicleLagCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
		fUserTick( dt, tripod );
	}

	void tVehicleLagCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
		mVehicle.fSetUseCamZoom( Gameplay_Vehicle_LagCamera_DistLerp );
		//mVehicle.fUseCamZoomFromGamepad( dt );

		const Physics::tVehiclePhysics& phys = mVehicle.fPhysics( );
		const tWheeledVehicleLogic::tCameraData& cameraData = mVehicle.fCameraData( );
		const tMat3f& unitXform = fUnitEntity( )->fObjectToWorld( );

		tMat3f flatXform;
		flatXform.fOrientYWithZAxis( tVec3f::cYAxis, unitXform.fZAxis( ) );
		flatXform.fSetTranslation( unitXform.fGetTranslation( ) );

		// integrate dynamics
		f32 idealLookAhead = fClamp( 1.0f - (phys.mW.fLength( ) - Gameplay_Vehicle_LagCamera_WSubtract) / Gameplay_Vehicle_LagCamera_WScale, 0.f, 1.f );

		// If we're sliding sideways, this will go to zero, going backwards will go to -1.0f, prefer 1.f in zero velocity cases )
		f32 directionFactor = tVec3f( phys.fVelocity( ) ).fProjectToXZAndNormalize( ).fDot( flatXform.fZAxis( ) );
		if( directionFactor < -0.25f ) 
			idealLookAhead *= 0.f;

		if( !mVehicle.fPhysics( ).fOnGround( ) )
			idealLookAhead *= 0.25f;

		idealLookAhead *= cameraData.mCameraZoomOut;

		if( Gameplay_Vehicle_LagCamera_EnableDynamics )
		{
			mZoomDist.fStep( idealLookAhead, dt );
			mZoomBlend.fStep( mVehicle.fBoostProgress( ) * phys.fSpeedPercent( ), dt );
			mTargetZoom = fLerp( mInitialZoom, (f32)Gameplay_Vehicle_LagCamera_BoostZoom, mZoomBlend.fValue( ) );
		}

		//tUseUnitCamera::mZoomDist = -mZoomDist.fValue( );
		Gfx::tTripod newTripod;

		if( mVehicle.fIsTank( ) )
		{
			fKeepCameraAlignedToUnit( newTripod, mVehicle.fCameraMovement( mSeat ).fCameraBasis( ) );
		}
		else
		{
			const Input::tGamepad& gp = fPlayer( ).fGamepad( );

			// These start in local space
			tVec3f target = fUnitLogic( ).fUseCamLookTarget( );
			target += tVec3f::cZAxis * mZoomDist.fValue( ) * Gameplay_Vehicle_LagCamera_LookInScale;

			tVec3f from = fUnitLogic( ).fUseCamOffset( );
			tVec3f lookDir = target - from;
			
			f32 eyeHeight = lookDir.y;
			f32 distance = lookDir.fXZ( ).fLength( );

			// Convert to world space
			newTripod.mLookAt = flatXform.fXformPoint( target );

			tVec3f currentDelta;

			if( !mInitialized )
			{
				currentDelta = fViewport( )->fLogicCamera( ).fLocalToWorld( ).fZAxis( );
				currentDelta.y = 0.f;
				currentDelta.fNormalizeSafe( );
			}
			else
			{
				currentDelta = newTripod.mLookAt - (mOldTripod.mEye + mVel * dt * Gameplay_Vehicle_LagCamera_VScale);
				currentDelta.y = 0.f;
				currentDelta.fNormalizeSafe( );
			}

			// Torque camera around to target dir
			f32 idealLookIn = 0.0f;
			tVec3f targetDir = flatXform.fZAxis( );
			f32 lookTorque = Gameplay_Vehicle_LagCamera_DirectionTorque;
			b32 userLook = false;

			const b32 southPawMode = fPlayer( ).fProfile( )->fSouthPaw( tUserProfile::cProfileCamera );
			if( gp.fButtonDown( southPawMode ? Input::tGamepad::cButtonLThumb : Input::tGamepad::cButtonRThumb ) )
				mCenterLook = true;

			if( gp.fButtonDown( southPawMode ? Input::tGamepad::cButtonLThumbMinMag : Input::tGamepad::cButtonRThumbMinMag ) )
			{
				mCenterLook = false;
				// intialize look on button press
				mLook.fSetValue( flatXform.fInverseXformVector( currentDelta ).fXZHeading( ) );
			}

			while( mLook.fValue( ) > cPi ) mLook.fSetValue( mLook.fValue( ) - c2Pi );
			while( mLook.fValue( ) < -cPi ) mLook.fSetValue( mLook.fValue( ) + c2Pi );

			if( mCenterLook )
				lookTorque = Gameplay_Vehicle_LagCamera_DirectionTorqueCentering;

			if( gp.fButtonHeld( southPawMode ? Input::tGamepad::cButtonLThumbMinMag : Input::tGamepad::cButtonRThumbMinMag ) )
			{
				mCenterLook = false;
				idealLookIn = (southPawMode ? gp.fLeftStickAngle( ) : gp.fRightStickAngle( ))  - Math::cPiOver2;
				lookTorque = Gameplay_Vehicle_LagCamera_DirectionTorqueUser;
				userLook = true;
			}

			idealLookIn = mLook.fValue( ) + fShortestWayAround( mLook.fValue( ), idealLookIn );
			if( Gameplay_Vehicle_LagCamera_EnableDynamics )
				mLook.fStep( idealLookIn, dt );

			targetDir.fSetXZHeading( mLook.fValue( ) );
			targetDir = flatXform.fXformVector( targetDir );

			tAxisAnglef aa( currentDelta, targetDir );
			f32 absAngleFromPole = fMin( fAbs( aa.mAngle ), cPi - fAbs( aa.mAngle ) );

			// Slow down as we approach target
			f32 slowDown = 1.0f;
			if( !userLook )
			{
				f32 slowDownStart = cPiOver4;
				slowDown = fMin( absAngleFromPole / slowDownStart, 1.f );
			}

			if( fEqual( aa.mAngle, 0.f, 0.3f ) )
				mCenterLook = false;

			mLookTorque.fStep( lookTorque, dt );
			f32 maxDA = mLookTorque.fValue( ) * slowDown * dt;
			aa.mAngle = fClamp( aa.mAngle, -maxDA, maxDA );
			currentDelta = tQuatf( aa ).fRotate( currentDelta );

			currentDelta.y = eyeHeight;
			currentDelta.x *= distance;
			currentDelta.z *= distance;
			newTripod.mEye = newTripod.mLookAt - currentDelta;
			newTripod.mUp = tVec3f::cYAxis;
		}

		if( !mInitialized )
		{
			mInitialized = true;
			mOldTripod = newTripod;
		}

		// blend with old tripod
		const f32 frameRateCompensation = dt / (1.f/30.f);
		const f32 camBlendT = frameRateCompensation * cameraData.mCameraLerp;
		const tVec3f oldPos = mOldTripod.mEye;
		fBlendTripods( camBlendT, newTripod, mOldTripod );
		tripod = mOldTripod;

		mVel = (mOldTripod.mEye - oldPos) / dt;
		mVel *= Gameplay_Vehicle_LagCamera_VDamp;
	}

}

