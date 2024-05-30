#include "BasePch.hpp"
#include "tGamepad.hpp"

namespace Sig { namespace Input
{
	namespace
	{
		struct tButtonString
		{ 
			tButtonString( ) : mButton( 0 ) { }
			tButtonString( const tStringPtr& name, tGamepad::tButton button )
				: mName( name )
				, mButton( button )
			{
			}

			b32 operator==( const tStringPtr& name ) const { return mName == name; }
			b32 operator==( tGamepad::tButton button ) const { return mButton == button; }

			tStringPtr mName; 
			tGamepad::tButton mButton; 
		};

		static tGrowableArray< tButtonString > gButtonStrings;

		define_static_function( fBuildButtonStrings )
		{
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "START" ), tGamepad::cButtonStart ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "SELECT" ), tGamepad::cButtonSelect ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "A" ), tGamepad::cButtonA ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "B" ), tGamepad::cButtonB ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "X" ), tGamepad::cButtonX ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "Y" ), tGamepad::cButtonY ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "DPADRIGHT" ), tGamepad::cButtonDPadRight ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "DPADUP" ), tGamepad::cButtonDPadUp ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "DPADDOWN" ), tGamepad::cButtonDPadDown ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "DPADLEFT" ), tGamepad::cButtonDPadLeft ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LSHOULDER" ), tGamepad::cButtonLShoulder ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LTHUMB" ), tGamepad::cButtonLThumb ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LTRIGGER" ), tGamepad::cButtonLTrigger ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LTHUMBMAXMAG" ), tGamepad::cButtonLThumbMaxMag ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LTHUMBMINMAG" ), tGamepad::cButtonLThumbMinMag ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RSHOULDER" ), tGamepad::cButtonRShoulder ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RTHUMB" ), tGamepad::cButtonRThumb ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RTRIGGER" ), tGamepad::cButtonRTrigger ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RTHUMBMAXMAG" ), tGamepad::cButtonRThumbMaxMag ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RTHUMBMINMAG" ), tGamepad::cButtonRThumbMinMag ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LSTICKRIGHT" ), tGamepad::cButtonLStickRight ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LSTICKUP" ), tGamepad::cButtonLStickUp ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LSTICKLEFT" ), tGamepad::cButtonLStickLeft ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LSTICKDOWN" ), tGamepad::cButtonLStickDown ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RSTICKRIGHT" ), tGamepad::cButtonRStickRight ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RSTICKUP" ), tGamepad::cButtonRStickUp ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RSTICKLEFT" ), tGamepad::cButtonRStickLeft ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RSTICKDOWN" ), tGamepad::cButtonRStickDown ) );
		}
	}

	tGamepad::tButton tGamepad::fStringToButton( const tStringPtr& name )
	{
		if( const tButtonString* map = gButtonStrings.fFind( name ) )
			return map->mButton;
		return 0;
	}

	const tStringPtr& tGamepad::fButtonToString( tButton button )
	{
		if( const tButtonString* map = gButtonStrings.fFind( button ) )
			return map->mName;
		return tStringPtr::cNullPtr;
	}

	const tGamepad tGamepad::cNullGamepad;

	tGamepad::tGamepad( u32 historyBufferSize )
		: mStateHistory( historyBufferSize )
		, mInputDevicePtr( 0 )
		, mConnected( false )
	{
		tStateData dummyStateData;
		mStateHistory.fFill( dummyStateData );
		fStartup( );
	}

	tGamepad::~tGamepad( )
	{
		fShutdown( );
	}

	void tGamepad::fCaptureState( u32 userIndex, f32 dt )
	{
		tStateData stateData;
		fCaptureStateUnbuffered( stateData, userIndex, dt );
		mStateHistory.fPushLast( stateData );
	}

	void tGamepad::fSetHistorySize( u32 newSize )
	{
		mStateHistory.fResize( fMax( 2u, newSize ) );
	}

	b32 tGamepad::fButtonHeld( tButton button ) const
	{
		return ( mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mButtonsDown & button );
	}

	b32 tGamepad::fButtonHeldRepeat( tButton button, u32 repeat ) const
	{
		u32 count = 0;
		for( u32 i = 0; i < repeat + 1; ++i )
		{
			if( mStateHistory[ mStateHistory.fNumItems( ) - i - 1 ].mButtonsDown & button )
				++count;
		}
		return count == repeat + 1;
	}

	b32 tGamepad::fButtonDown( tButton button ) const
	{ 
		return  ( mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mButtonsDown & button )
			&& !( mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mButtonsDown & button );
	}

	void tGamepad::fClearButtonDown( tButton button ) const
	{
		// This assert ensures you dont call this on a cNullGamepad
		sigassert( fButtonHeld( button ) && "Button is not even down this frame." );

		// Act like the button was down last frame.
		u32& buttonsDown = const_cast<u32&>( mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mButtonsDown );
		buttonsDown |= button;
	}

	b32 tGamepad::fButtonUp( tButton button ) const
	{ 
		return !( mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mButtonsDown & button )
			&&  ( mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mButtonsDown & button );
	}

	f32 tGamepad::fLeftTriggerPressure( ) const
	{
		return mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mLeftTrigger / 255.f;
	}

	b32 tGamepad::fLeftTriggerHeld( ) const
	{
		return mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mLeftTrigger > 0;
	}

	b32 tGamepad::fLeftTriggerDown( ) const
	{
		return	 ( mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mLeftTrigger > 0 )
			 &&	!( mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mLeftTrigger > 0 );
	}

	b32 tGamepad::fLeftTriggerUp( ) const
	{
		return	!( mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mLeftTrigger > 0 )
			 &&	 ( mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mLeftTrigger > 0 );
	}

	f32 tGamepad::fRightTriggerPressure( ) const
	{
		return mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mRightTrigger / 255.f;
	}

	b32 tGamepad::fRightTriggerHeld( ) const
	{
		return mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mRightTrigger > 0;
	}

	b32 tGamepad::fRightTriggerDown( ) const
	{
		return	 ( mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mRightTrigger > 0 )
			 &&	!( mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mRightTrigger > 0 );
	}

	b32 tGamepad::fRightTriggerUp( ) const
	{
		return	!( mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mRightTrigger > 0 )
			 &&	 ( mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mRightTrigger > 0 );
	}

	Math::tVec2f tGamepad::fLeftStick( u32 numFramesBack ) const
	{
		const tStateData& s = mStateHistory[ mStateHistory.fNumItems( ) - 1 - numFramesBack ];
		return Math::tVec2f( s.mLeftStick.x, s.mLeftStick.y );
	}

	f32 tGamepad::fLeftStickMagnitude( u32 numFramesBack ) const
	{
		const tStateData& s = mStateHistory[ mStateHistory.fNumItems( ) - 1 - numFramesBack ];
		return s.mLeftStickStrength;
	}

	f32 tGamepad::fLeftStickAngle( u32 numFramesBack ) const
	{
		const tStateData& s = mStateHistory[ mStateHistory.fNumItems( ) - 1 - numFramesBack ];
		return s.mLeftStickAngle;
	}

	Math::tVec2f tGamepad::fRightStick( u32 numFramesBack ) const
	{
		const tStateData& s = mStateHistory[ mStateHistory.fNumItems( ) - 1 - numFramesBack ];
		return Math::tVec2f( s.mRightStick.x, s.mRightStick.y );
	}

	f32 tGamepad::fRightStickMagnitude( u32 numFramesBack ) const
	{
		const tStateData& s = mStateHistory[ mStateHistory.fNumItems( ) - 1 - numFramesBack ];
		return s.mRightStickStrength;
	}

	/*static*/ Math::tVec2f tGamepad::fMapStickCircleToRectangle( const Math::tVec2f &stick, u32 innerRoundness )
	{
		//from: http://theinstructionlimit.com/?p=531
	 
		// Determine the theta angle
		f32 angle = Math::fAtan2( stick.y, stick.x ) + Math::cPi;
	 
		Math::tVec2f squared;
		// Scale according to which wall we're clamping to
		// X+ wall
		if( angle <= Math::cPiOver4 || angle > 7 * Math::cPiOver4 )
			squared = stick / Math::fCos(angle);
		// Y+ wall
		else if( angle > Math::cPiOver4 && angle <= 3 * Math::cPiOver4 )
			squared = stick / Math::fSin(angle);
		// X- wall
		else if( angle > 3 * Math::cPiOver4 && angle <= 5 * Math::cPiOver4 )
			squared = stick / -Math::fCos(angle);
		// Y- wall
		else if( angle > 5 * Math::cPiOver4 && angle <= 7 * Math::cPiOver4 )
			squared = stick / -Math::fSin(angle);
		else 
			log_assert( false, "Invalid angle!" );
	 
		// Early-out for a perfect square output
		if (innerRoundness == 0)
			return squared;
	 
		// Find the inner-roundness scaling factor and LERP
		f32 length = stick.fLength();
		f32 factor = pow( length, (f32)innerRoundness );
		return fLerp( stick, squared, factor );
	}

	f32 tGamepad::fRightStickAngle( u32 numFramesBack ) const
	{
		const tStateData& s = mStateHistory[ mStateHistory.fNumItems( ) - 1 - numFramesBack ];
		return s.mRightStickAngle;
	}

	//------------------------------------------------------------------------------
	u32 tGamepad::fGetDirection( ) const
	{
		if( fButtonDown( cButtonDPadLeft ) )
			return cDirectionLeft;
		if( fButtonDown( cButtonDPadRight ) )
			return cDirectionRight;
		if( fButtonDown( cButtonDPadUp ) )
			return cDirectionUp;
		if( fButtonDown( cButtonDPadDown ) )
			return cDirectionDown;

		Math::tVec2f leftStick = fLeftStick( );

		if( fAbs( leftStick.y ) > fAbs( leftStick.x ) )
		{
			if( leftStick.y > 0 )
				return cDirectionUp;
			else
				return cDirectionDown;
		}
		else if( fAbs( leftStick.x ) > fAbs( leftStick.y ) )
		{
			if( leftStick.x > 0 )
				return cDirectionRight;
			else
				return cDirectionLeft;
		}

		return cDirectionNone;
	}
		
	void tGamepad::fPutStateData( const tStateData& data )
	{
		mStateHistory.fPushLast( data );
	}

	const tGamepad::tStateData& tGamepad::fGetStateData( ) const
	{
		return  mStateHistory[ mStateHistory.fNumItems( ) - 1 ];
	}

	b32 tGamepad::fUpdated( ) const
	{
		const tStateData& now  = mStateHistory[ mStateHistory.fNumItems( ) - 1 ];
		const tStateData& prev = mStateHistory[ mStateHistory.fNumItems( ) - 2 ];

		return ( now.mButtonsDown  != prev.mButtonsDown )
			|| ( now.mLeftStick    != prev.mLeftStick   )
			|| ( now.mLeftTrigger  != prev.mLeftTrigger )
			|| ( now.mRightStick   != prev.mRightStick  )
			|| ( now.mRightTrigger != prev.mRightTrigger )
			;
	}

	b32 tGamepad::fActive( ) const
	{
		const tStateData& now  = mStateHistory[ mStateHistory.fNumItems( ) - 1 ];
		return fUpdated( ) || now.mButtonsDown;
	}

	//------------------------------------------------------------------------------
	tGamepad * tGamepad::fNullGamepadFromScript( )
	{
		return (tGamepad *)&cNullGamepad;
	}

	namespace
	{
		void fBuzz( tGamepad* gp, f32 strength )
		{
			sigassert( gp );
			gp->fRumble( ).fBuzz( strength );
		}
	}

	//------------------------------------------------------------------------------
	void tGamepad::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tGamepad, Sqrat::DefaultAllocator<tGamepad> > classDesc( vm.fSq( ) );

		classDesc
			.Prop(_SC("LeftStick"), &tGamepad::fLeftStick)
			.Prop(_SC("RightStick"), &tGamepad::fRightStick)
			.Prop(_SC("Direction"), &tGamepad::fGetDirection)
			.Func(_SC("ButtonDown"), &tGamepad::fButtonDown)
			.Func(_SC("ButtonUp"), &tGamepad::fButtonUp)
			.Func(_SC("ButtonHeld"), &tGamepad::fButtonHeld)
			.StaticFunc(_SC("NullGamepad"), &tGamepad::fNullGamepadFromScript)
			.GlobalFunc(_SC("Buzz"), &fBuzz)
			;

		vm.fNamespace(_SC("Input")).Bind( _SC("Gamepad"), classDesc );

		vm.fConstTable( ).Const( _SC("GAMEPAD_DIRECTION_NONE"), ( int )cDirectionNone );
		vm.fConstTable( ).Const( _SC("GAMEPAD_DIRECTION_LEFT"), ( int )cDirectionLeft );
		vm.fConstTable( ).Const( _SC("GAMEPAD_DIRECTION_RIGHT"), ( int )cDirectionRight );
		vm.fConstTable( ).Const( _SC("GAMEPAD_DIRECTION_UP"), ( int )cDirectionUp );
		vm.fConstTable( ).Const( _SC("GAMEPAD_DIRECTION_DOWN"), ( int )cDirectionDown );

		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_START"), ( int )cButtonStart );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_SELECT"), ( int )cButtonSelect );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_A"), ( int )cButtonA );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_B"), ( int )cButtonB );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_X"), ( int )cButtonX );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_Y"), ( int )cButtonY );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_DPAD_RIGHT"), ( int )cButtonDPadRight );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_DPAD_UP"), ( int )cButtonDPadUp );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_DPAD_LEFT"), ( int )cButtonDPadLeft );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_DPAD_DOWN"), ( int )cButtonDPadDown );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_LSHOULDER"), ( int )cButtonLShoulder );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_LTHUMB"), ( int )cButtonLThumb );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_LTRIGGER"), ( int )cButtonLTrigger );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_LTHUMB_MAXMAG"), ( int )cButtonLThumbMaxMag );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_LTHUMB_MINMAG"), ( int )cButtonLThumbMinMag );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_RSHOULDER"), ( int )cButtonRShoulder );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_RTHUMB"), ( int )cButtonRThumb );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_RTRIGGER"), ( int )cButtonRTrigger );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_RTHUMB_MAXMAG"), ( int )cButtonRThumbMaxMag );
		vm.fConstTable( ).Const( _SC("GAMEPAD_BUTTON_RTHUMB_MINMAG"), ( int )cButtonRThumbMinMag );
	}

}}
