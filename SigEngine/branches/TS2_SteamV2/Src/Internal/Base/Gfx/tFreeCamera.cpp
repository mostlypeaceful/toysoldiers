#include "BasePch.hpp"
#include "tFreeCamera.hpp"
#include "tUser.hpp"

namespace Sig { namespace Gfx
{
	devvar( bool, Cameras_FreeCam_ExclusiveInput, true );
	devvar_clamp( f32, Cameras_FreeCam_LeftStickScale, 1.f, 0.f, 100.f, 2 );
	devvar_clamp( f32, Cameras_FreeCam_RightStickScale, 1.f, 0.f, 100.f, 2 );

	tFreeCamera::tFreeCamera( const tUserPtr& user )
		: tCameraController( user->fViewport( ) )
		, mLockInXZPlane( false )
		, mGamepadFilterIndex( ~0 )
	{
		fAddUser( user );
	}

	const Input::tGamepad& tFreeCamera::fGamepad( ) const
	{
		return Cameras_FreeCam_ExclusiveInput ? mUsers.fFront( )->fFilteredGamepad( mGamepadFilterIndex ) : mUsers.fFront( )->fRawGamepad( );
	}

	void tFreeCamera::fOnTick( f32 dt )
	{
		if( mUsers.fCount( ) != 1 || ( Cameras_FreeCam_ExclusiveInput && mGamepadFilterIndex == ~0 ) )
			return; // couldn't evaluate target
		Gfx::tCamera camera = fViewport( )->fLogicCamera( );
		const Input::tGamepad& gamepad = fGamepad( );



		camera.fSetUp( Math::tVec3f::cYAxis ); // don't get off axis

		if( gamepad.fButtonDown( Input::tGamepad::cButtonA ) )
			mLockInXZPlane = !mLockInXZPlane;

		f32 length;
		Math::tVec3f toLookAt = ( camera.fGetTripod( ).mLookAt - camera.fGetTripod( ).mEye ).fNormalizeSafe( length );

		// now rotate
		const Math::tVec2f rstick = Cameras_FreeCam_RightStickScale * gamepad.fRightStick( );
		if( !rstick.fIsZero( ) )
			mYawSpeed.fAddA( -15.f * rstick.x );
		mYawSpeed.fStep( dt, 0.15f );
		const Math::tQuatf ry( Math::tAxisAnglef( Math::tVec3f::cYAxis, mYawSpeed.fV( ) * dt ) );
		toLookAt = ry.fRotate( toLookAt );

		if( !rstick.fIsZero( ) )
			mPitchSpeed.fAddA( -15.f * rstick.y );
		mPitchSpeed.fStep( dt, 0.15f );
		const Math::tQuatf rx( Math::tAxisAnglef( camera.fXAxis( ), mPitchSpeed.fV( ) * dt ) );
		const Math::tVec3f toLookAtNorm = Math::tVec3f( toLookAt ).fNormalizeSafe( Math::tVec3f::cZAxis );
		if( ( toLookAtNorm.y < 0.999f || rstick.y < 0 ) && ( toLookAtNorm.y > -0.999f || rstick.y > 0.f ) )
			toLookAt = rx.fRotate( toLookAt );

		camera.fSetLookAt( camera.fGetTripod( ).mEye + toLookAt * length );

		const Math::tVec2f lstick = Cameras_FreeCam_LeftStickScale * gamepad.fLeftStick( );
		const f32 moveMagnitude = 15.f + 950.0f * gamepad.fLeftTriggerPressure( );
		const Math::tVec3f moveX = camera.fXAxis( );
		const Math::tVec3f moveY = camera.fYAxis( );
		const Math::tVec3f moveZ = mLockInXZPlane ? camera.fZAxis( ).fProjectToXZAndNormalize( ) : camera.fZAxis( );

		f32 yVel = 0.f;
		if( !gamepad.fButtonHeld( Input::tGamepad::cButtonSelect ) )
		{
			if( gamepad.fButtonHeld( Input::tGamepad::cButtonLShoulder ) ) yVel -= 1.f;
			if( gamepad.fButtonHeld( Input::tGamepad::cButtonRShoulder ) ) yVel += 1.f;
		}

		const Math::tVec3f moveAcc = moveMagnitude * ( -lstick.x * moveX + lstick.y * moveZ + yVel * moveY );
		mPanVelocity.fAddA( moveAcc );
		mPanVelocity.fStep( dt, 0.125f );
		camera.fMoveGlobal( mPanVelocity.fV( ) * dt );

		fViewport( )->fSetCameras( camera );
	}
	void tFreeCamera::fOnActivate( b32 active )
	{
		if( mUsers.fCount( ) == 1 )
		{
			if( !fIsActive( ) && active )
			{
				// activating
				if( Cameras_FreeCam_ExclusiveInput )
					mGamepadFilterIndex = mUsers.fFront( )->fIncInputFilterLevel( );
			}
			else if( fIsActive( ) && !active )
			{
				// de-activating
				if( mGamepadFilterIndex != ~0 )
				{
					mUsers.fFront( )->fDecInputFilterLevel( mGamepadFilterIndex );
					mGamepadFilterIndex = ~0;
				}
			}
		}

		tCameraController::fOnActivate( active );
	}

}}

