#include "BasePch.hpp"
#include "tFreeCamera.hpp"
#include "tUser.hpp"
#include "tGameAppBase.hpp"

namespace Sig { namespace Gfx
{
	devvar_clamp( f32, Cameras_FreeCam_LeftStickScale, 1.f, 0.f, 100.f, 2 );
	devvar_clamp( f32, Cameras_FreeCam_RightStickScale, 1.f, 0.f, 100.f, 2 );
	devvar( bool, Debug_Camera_AllowInputToggle, false );

	tFreeCamera::tFreeCamera( const tUserPtr& user )
		: tCameraController( user->fViewport( ) )
		, mLockInXZPlane( false )
		, mAllowInput( false )
	{
		fAddUser( user );
	}

	const Input::tGamepad& tFreeCamera::fGamepad( ) const
	{
		return fGetAllowInput( ) ? mUsers.fFront( )->fRawGamepad( )
								 : Input::tGamepad::cNullGamepad;
	}

	void tFreeCamera::fOnTick( f32 dt )
	{	
		if( Debug_Camera_AllowInputToggle )
		{
			std::stringstream ss;
			ss << "FreeCam Input " << ( mAllowInput ? "ENABLED" : "DISABLED" );
			Math::tVec4f color = mAllowInput ? Math::tVec4f( 0, 0.7f, 0, 1 ) : Math::tVec4f( 0.7f, 0, 0, 1 );
			tGameAppBase::fInstance( ).fDebugOutput( ss.str( ), 240.0f, 16.0f, color );
			tGameAppBase::fInstance( ).fDebugOutput( "Press B-button to toggle", 240.0f, 28.0f, Math::tVec4f( 1, 1, 1, 1 ) );
		}

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
		const f32 moveMagnitude = 15.f + 950.0f * gamepad.fLeftTriggerPressure( ) + 950.0f * gamepad.fRightTriggerPressure( );
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

	b32 tFreeCamera::fGetAllowInput( ) const
	{
		if( Debug_Camera_AllowInputToggle )
			return mAllowInput;
		return true;
	}

	b32 tFreeCamera::fAllowInputToggle( )
	{
		return Debug_Camera_AllowInputToggle;
	}

}}

