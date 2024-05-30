#include "GameAppPch.hpp"
#include "tUseVehicleCamera.hpp"
#include "tGameApp.hpp"
#include "tWheeledVehicleLogic.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( bool, Gameplay_Vehicle_Camera_EnableDynamics, true );



	tUseVehicleCamera::tUseVehicleCamera( tPlayer& player, u32 seat, tWheeledVehicleLogic& vehicleLogic )
		: tUseUnitCamera( player, &vehicleLogic, false )
		, mVehicle( vehicleLogic )
		, mSeat( seat )
		, mZoomDist( 0.0f, 10.0f, 5.f )
		, mLookIn( 0.0f, 7.5f, 6.f )
		, mInitialized( false )
	{
		mTargetZoom = vehicleLogic.fUnitAttributeCameraFOV( );
		mRaycastTerrainPenetration = true;
	}

	void tUseVehicleCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
		fUserTick( dt, tripod );
	}

	void tUseVehicleCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
		const Physics::tVehiclePhysics& phys = mVehicle.fPhysics( );
		const tWheeledVehicleLogic::tCameraData& cameraData = mVehicle.fCameraData( );

		// integrate dynamics
		f32 speed = phys.fSpeed( );
		f32 speedVal = speed / phys.fMaxSpeed( );
		f32 idealZoomDist = fAbs( speedVal ) * -cameraData.mCameraZoomOut;
		mZoomDist.fStep( idealZoomDist, dt );
		if( !Gameplay_Vehicle_Camera_EnableDynamics ) mZoomDist.fSetValue( 0.0f );

		tUseUnitCamera::mZoomDist = mZoomDist.fValue( );
		Gfx::tTripod newTripod;

		if( mVehicle.fIsTank( ) )
		{
			fKeepCameraAlignedToUnit( newTripod, mVehicle.fCameraMovement( mSeat ).fCameraBasis( ) );
		}
		else
		{
			const tGameControllerPtr gc = fPlayer( ).fGameController( );
			f32 idealLookIn = phys.fSteeringInput( ) * cameraData.mCameraTurnIn;

			//const b32 southPawMode = fPlayer( ).fProfile( )->fSouthPaw( tUserProfile::cProfileCamera );
			if( mVehicle.fExtraMode( ) && gc->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_AIM_MINMAG ) )
			{
				idealLookIn = gc->fAimStick( tUserProfile::cProfileCamera ).fAngle( ) - Math::cPiOver2;
			}

			idealLookIn = mLookIn.fValue( ) + fShortestWayAround( mLookIn.fValue( ), idealLookIn );
			mLookIn.fStep( idealLookIn, dt );
			if( !Gameplay_Vehicle_Camera_EnableDynamics ) mLookIn.fSetValue( 0.0f );

			f32 realLookIn = mLookIn.fValue( );
			//realLookIn *= realLookIn * realLookIn;



			// keep the camera aligned to unit
			const tMat3f& unitXform = fUnitEntity( )->fObjectToWorld( );
			tVec3f zAxis = unitXform.fZAxis( );
			zAxis.y = 0;
			zAxis.fNormalizeSafe( );

			tVec3f xAxis = tVec3f::cYAxis.fCross( zAxis );
			tMat3f leveledXform( xAxis, tVec3f::cYAxis, zAxis, unitXform.fGetTranslation( ) );
			tMat3f rotation( tQuatf( tAxisAnglef( tVec3f::cYAxis, realLookIn ) ) );

			// calculate new tripod
			fKeepCameraAlignedToUnit( newTripod, leveledXform * rotation);
		}

		if( !mInitialized )
		{
			mInitialized = true;
			mOldTripod = newTripod;
		}

		// blend with old tripod
		const f32 frameRateCompensation = dt / (1.f/30.f);
		const f32 camBlendT = frameRateCompensation * cameraData.mCameraLerp;
		fBlendTripods( camBlendT, newTripod, mOldTripod );
		tripod = mOldTripod;
	}

}

