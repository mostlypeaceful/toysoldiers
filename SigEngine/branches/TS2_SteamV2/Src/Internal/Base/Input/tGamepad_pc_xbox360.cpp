#include "BasePch.hpp"
#if defined( platform_msft )
#include "tGamepad.hpp"
#include "tUser.hpp"
#include "tApplication.hpp"

#if !(defined( platform_metro ) && defined( build_release)) // Compile out XInput use for CPX build
#define use_xinput
#endif

#if !defined( platform_xbox360 )
#	include <XInput.h>
#endif


namespace Sig { namespace Input
{
	namespace
	{
		const f32 cMaxStickValue = f32( 0x7fff );
		devvar( bool, Debug_Gamepad_NoRumble, false );
		devvar_clamp( f32, Gamepad_Config_StickDeadzoneMin, 0.24f, 0.f, 1.f, 2 ); // threshold percentage below which we say no input happened
		devvar_clamp( f32, Gamepad_Config_StickDeadzoneMax, 0.97f, 0.f, 1.f, 2 );
		devvar_clamp( u32, Gamepad_Config_TriggerThresh, 0, 0, 255, 1 );
		devvar( bool, Gamepad_Enabled, true );

		inline f32 fConvertStickValue( SHORT input )
		{
			return ( input / cMaxStickValue );
		}

		Math::tVec2f fConvertStick( SHORT x, SHORT y )
		{
			const f32 deadZone = Gamepad_Config_StickDeadzoneMin;
			Math::tVec2f o( fConvertStickValue( x ), fConvertStickValue( y ) );
			
			if( fAbs( o.x ) < deadZone )
				o.x = 0;
			if( fAbs( o.y ) < deadZone )
				o.y = 0;

			return o;
		}

		inline u8 fConvertTriggerValue( u8 value )
		{
			return (value > Gamepad_Config_TriggerThresh) ? value : 0;
		}
	}

	const tGamepad::tButton tGamepad::cButtonStart					= XINPUT_GAMEPAD_START;
	const tGamepad::tButton tGamepad::cButtonSelect					= XINPUT_GAMEPAD_BACK;
	const tGamepad::tButton tGamepad::cButtonA						= XINPUT_GAMEPAD_A;
	const tGamepad::tButton tGamepad::cButtonB						= XINPUT_GAMEPAD_B;
	const tGamepad::tButton tGamepad::cButtonX						= XINPUT_GAMEPAD_X;
	const tGamepad::tButton tGamepad::cButtonY						= XINPUT_GAMEPAD_Y;
	const tGamepad::tButton tGamepad::cButtonDPadRight				= XINPUT_GAMEPAD_DPAD_RIGHT;
	const tGamepad::tButton tGamepad::cButtonDPadUp					= XINPUT_GAMEPAD_DPAD_UP;
	const tGamepad::tButton tGamepad::cButtonDPadLeft				= XINPUT_GAMEPAD_DPAD_LEFT;
	const tGamepad::tButton tGamepad::cButtonDPadDown				= XINPUT_GAMEPAD_DPAD_DOWN;
	const tGamepad::tButton tGamepad::cButtonLShoulder				= XINPUT_GAMEPAD_LEFT_SHOULDER;
	const tGamepad::tButton tGamepad::cButtonLThumb					= XINPUT_GAMEPAD_LEFT_THUMB;
	const tGamepad::tButton tGamepad::cButtonRShoulder				= XINPUT_GAMEPAD_RIGHT_SHOULDER;
	const tGamepad::tButton tGamepad::cButtonRThumb					= XINPUT_GAMEPAD_RIGHT_THUMB;
	const tGamepad::tButton tGamepad::cButtonRTrigger				= 1 << 16; // safe because the XINPUT_*** ones only go up to 1 << 15 (WORD-size)
	const tGamepad::tButton tGamepad::cButtonLTrigger				= 1 << 17; 
	const tGamepad::tButton tGamepad::cButtonRThumbMaxMag			= 1 << 18; 
	const tGamepad::tButton tGamepad::cButtonLThumbMaxMag			= 1 << 19; 
	const tGamepad::tButton tGamepad::cButtonRThumbMinMag			= 1 << 20; 
	const tGamepad::tButton tGamepad::cButtonLThumbMinMag			= 1 << 21; 
	const tGamepad::tButton tGamepad::cButtonLStickRight			= 1 << 22; 
	const tGamepad::tButton tGamepad::cButtonLStickUp				= 1 << 23; 
	const tGamepad::tButton tGamepad::cButtonLStickLeft				= 1 << 24; 
	const tGamepad::tButton tGamepad::cButtonLStickDown				= 1 << 25; 
	const tGamepad::tButton tGamepad::cButtonRStickRight			= 1 << 26; 
	const tGamepad::tButton tGamepad::cButtonRStickUp				= 1 << 27; 
	const tGamepad::tButton tGamepad::cButtonRStickLeft				= 1 << 28; 
	const tGamepad::tButton tGamepad::cButtonRStickDown				= 1 << 29; 

	f32 tGamepad::fComputeStickMagnitude( const Math::tVec2f& stick, f32 rawMag, f32 angle )
	{
		const f32 effectiveMag = fMin( 1.f, rawMag / Gamepad_Config_StickDeadzoneMax );
		return effectiveMag;
	}

	void tGamepad::fCaptureStateUnbuffered( tStateData& stateData, u32 userIndex, f32 dt )
	{
		// start by zeroing out data
		fZeroOut( stateData );

#if defined(use_xinput)
		// TODOHACK support multiple pads
		XINPUT_STATE xis={0};
		if( userIndex < tUser::cMaxLocalUsers )
		{
			const DWORD result = XInputGetState( userIndex, &xis );
			if( result == ERROR_SUCCESS )
				mConnected = true;
			else
				mConnected = false;
		}
		else
			mConnected = false;

		if( !mConnected || !Gamepad_Enabled
#if defined( platform_pcdx ) && defined( target_game )
			|| tApplication::fInstance( ).fSystemUiShowing( )
#endif // defined( platform_pcdx ) && defined( target_game )
			)
		{
			// not connected, bail
			return;
		}

		XINPUT_VIBRATION xVibes = {0};

		if( mRumble.fStep( dt ) )
		{
			Math::tVec2f currentRumble = mRumble.fCurrentRumble( );
			currentRumble = fClamp( currentRumble, Math::tVec2f::cZeroVector, Math::tVec2f::cOnesVector );
			
			if( !Debug_Gamepad_NoRumble )
			{
				xVibes.wLeftMotorSpeed	= fRound<WORD>( 65535 * currentRumble.x );
				xVibes.wRightMotorSpeed = fRound<WORD>( 65535 * currentRumble.y );
			}
		}			

		XInputSetState( userIndex, &xVibes );

		// convert xinput state to our platform-independent representation...

		// copy over button flags and trigger pressures
		stateData.mButtonsDown	= xis.Gamepad.wButtons;
		stateData.mLeftTrigger	= fConvertTriggerValue( xis.Gamepad.bLeftTrigger );
		stateData.mRightTrigger = fConvertTriggerValue( xis.Gamepad.bRightTrigger );

		// convert trigger values to virtual buttons
		stateData.mButtonsDown = stateData.mLeftTrigger > 0 ? fSetBits( stateData.mButtonsDown, cButtonLTrigger ) : fClearBits( stateData.mButtonsDown, cButtonLTrigger );
		stateData.mButtonsDown = stateData.mRightTrigger > 0 ? fSetBits( stateData.mButtonsDown, cButtonRTrigger ) : fClearBits( stateData.mButtonsDown, cButtonRTrigger );
		
		// convert raw stick directions/strengths
		const Math::tVec2f lstick = fConvertStick( xis.Gamepad.sThumbLX, xis.Gamepad.sThumbLY );
		const Math::tVec2f rstick = fConvertStick( xis.Gamepad.sThumbRX, xis.Gamepad.sThumbRY );

		// copy and normalize
		f32 lstickMag, rstickMag;
		stateData.mLeftStick = lstick;
		stateData.mLeftStick.fNormalizeSafe( lstickMag );
		stateData.mRightStick = rstick;
		stateData.mRightStick.fNormalizeSafe( rstickMag );
		// compute angle
		stateData.mLeftStickAngle = lstick.fAngle( );
		stateData.mRightStickAngle = rstick.fAngle( );

		// given angle, compute magnitudes
		stateData.mLeftStickStrength = fComputeStickMagnitude( lstick, lstickMag, stateData.mLeftStickAngle );
		stateData.mRightStickStrength = fComputeStickMagnitude( rstick, rstickMag, stateData.mRightStickAngle );

		// convert stick displacement to virtual buttons
		const f32 cMaxMag = 0.95f;
		const f32 cMinMag = 0.05f;
		stateData.mButtonsDown = stateData.mLeftStickStrength > cMaxMag ? fSetBits( stateData.mButtonsDown, cButtonLThumbMaxMag ) : fClearBits( stateData.mButtonsDown, cButtonLThumbMaxMag );
		stateData.mButtonsDown = stateData.mRightStickStrength > cMaxMag ? fSetBits( stateData.mButtonsDown, cButtonRThumbMaxMag ) : fClearBits( stateData.mButtonsDown, cButtonRThumbMaxMag );
		stateData.mButtonsDown = stateData.mLeftStickStrength > cMinMag ? fSetBits( stateData.mButtonsDown, cButtonLThumbMinMag ) : fClearBits( stateData.mButtonsDown, cButtonLThumbMinMag );
		stateData.mButtonsDown = stateData.mRightStickStrength > cMinMag ? fSetBits( stateData.mButtonsDown, cButtonRThumbMinMag ) : fClearBits( stateData.mButtonsDown, cButtonRThumbMinMag );

		// Left stick
		stateData.mButtonsDown = ( stateData.mLeftStick.y < 0 && stateData.mLeftStickStrength > cMinMag ) ? fSetBits( stateData.mButtonsDown, cButtonLStickDown ) : fClearBits( stateData.mButtonsDown, cButtonLStickDown );
		stateData.mButtonsDown = ( stateData.mLeftStick.y > 0 && stateData.mLeftStickStrength > cMinMag ) ? fSetBits( stateData.mButtonsDown, cButtonLStickUp ) : fClearBits( stateData.mButtonsDown, cButtonLStickUp );
		stateData.mButtonsDown = ( stateData.mLeftStick.x < 0 && stateData.mLeftStickStrength > cMinMag ) ? fSetBits( stateData.mButtonsDown, cButtonLStickLeft ) : fClearBits( stateData.mButtonsDown, cButtonLStickLeft );
		stateData.mButtonsDown = ( stateData.mLeftStick.x > 0 && stateData.mLeftStickStrength > cMinMag ) ? fSetBits( stateData.mButtonsDown, cButtonLStickRight ) : fClearBits( stateData.mButtonsDown, cButtonLStickRight );

		// Right stick
		stateData.mButtonsDown = ( stateData.mRightStick.y < 0 && stateData.mRightStickStrength > cMinMag ) ? fSetBits( stateData.mButtonsDown, cButtonRStickDown ) : fClearBits( stateData.mButtonsDown, cButtonRStickDown );
		stateData.mButtonsDown = ( stateData.mRightStick.y > 0 && stateData.mRightStickStrength > cMinMag ) ? fSetBits( stateData.mButtonsDown, cButtonRStickUp ) : fClearBits( stateData.mButtonsDown, cButtonRStickUp );
		stateData.mButtonsDown = ( stateData.mRightStick.x < 0 && stateData.mRightStickStrength > cMinMag ) ? fSetBits( stateData.mButtonsDown, cButtonRStickLeft ) : fClearBits( stateData.mButtonsDown, cButtonRStickLeft );
		stateData.mButtonsDown = ( stateData.mRightStick.x > 0 && stateData.mRightStickStrength > cMinMag ) ? fSetBits( stateData.mButtonsDown, cButtonRStickRight ) : fClearBits( stateData.mButtonsDown, cButtonRStickRight );

		// modify stick values to have proper adjusted magnitude
		stateData.mLeftStick *= stateData.mLeftStickStrength;
		stateData.mRightStick *= stateData.mRightStickStrength;
#endif // use_xinput
	}

	void tGamepad::fStartup( )
	{
	}

	void tGamepad::fShutdown( )
	{
	}

}}
#endif//#if defined( platform_msft )

