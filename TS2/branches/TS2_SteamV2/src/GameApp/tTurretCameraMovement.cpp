#include "GameAppPch.hpp"
#include "tTurretCameraMovement.hpp"
#include "Input/tGamepad.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( u32, Gameplay_Turrets_Roundness, 3 );

	tTurretCameraMovement::tTurretCameraMovement( )
		: mSpeed( 0.f )
		, mDamping( 0.f )
		, mFlatCamera( tMat3f::cIdentity )
		, mCurrentCameraPitch( 0.f )
		, mPitchMax( 0.f )
		, mPitchMin( 0.f )
	{ 
		fReset( tVec3f::cZAxis );
	}

	void tTurretCameraMovement::fReset( const tVec3f& direction )
	{	
		mYawPitchVel = tVec2f::cZeroVector;
		mCameraBasis.fOrientZAxis( direction );
		mFlatCamera = mCameraBasis;
		mCurrentCameraPitch = 0.f;
	}

	void tTurretCameraMovement::fUpdate( f32 dt, Math::tVec2f stickInput )
	{
		stickInput = Input::tGamepad::fMapStickCircleToRectangle( stickInput, Gameplay_Turrets_Roundness );

		mYawPitchVel = stickInput * mSpeed * dt;

		const f32 frameRateCompensation = dt / ( 1.f / 30.f );
		mYawPitchVel *= mDamping / frameRateCompensation;

		tQuatf cameraYaw( tAxisAnglef( tVec3f::cYAxis, -mYawPitchVel.x ) );
		mFlatCamera.fOrientXAxis( cameraYaw.fRotate( mCameraBasis.fXAxis( ) ) );

		// compute pitched camera
		mCurrentCameraPitch = fClamp( mCurrentCameraPitch + mYawPitchVel.y, mPitchMin, mPitchMax );
		tQuatf cameraPitch( tAxisAnglef( mCameraBasis.fXAxis( ), -mCurrentCameraPitch ) );
		tVec3f newZ = cameraPitch.fRotate( mFlatCamera.fZAxis( ) ).fNormalizeSafe( tVec3f::cZAxis );
		mCameraBasis.fOrientZAxis( newZ );
	}

}

