#include "GameAppPch.hpp"
#include "tGameController.hpp"

#include "Input/tGamepad.hpp"
#include "Input/tKeyboard.hpp"
#include "Input/tMouse.hpp"

#include "tApplication.hpp"
#include "tGameApp.hpp"
#include "tPlayer.hpp"
#include "tSync.hpp"

#include "tXmlFile.hpp"
#include "tXmlSerializer.hpp"
#include "tXmlDeserializer.hpp"

namespace Sig
{
	namespace
	{
		const tFilePathPtr	cKeyBindingsFileName( "keybindings.xml" );
		const tStringPtr	cKeyBindingsXmlRoot( "SigKeyBindings" );
		const f32			cSimulatedStickMinMag = FLT_EPSILON;
		const f32			cSimulatedStickMaxMag = 0.95f;

		//static u32			gDisplayWidth = ~0;
		//static u32			gDisplayHeight = ~0;

		devvar( f32, GameController_MouseMaxDeltaDefault, 50.0f );
		devvar( f32, GameController_MouseMaxDeltaCamera, 100.0f );
		devvar( f32, GameController_MouseSensFloor, 1.0f );
		devvar( f32, GameController_MouseSensRange, 18.0f );

		struct tButtonMap
        {
			tStringPtr	mControlProfile;
            tStringPtr	mName;
            tStringPtr	mKey;
            tButtonMap( ) { }
            tButtonMap( const tStringPtr& controlProfile, const tStringPtr& name, const tStringPtr& key ) : mControlProfile( controlProfile ), mName( name ), mKey( key ) { }
        };

		struct tMouseSettingMap
		{
			tStringPtr	mControlProfile;
			f32			mSensitivity;
			b32			mInverted;
			tMouseSettingMap( ) { };
			tMouseSettingMap( const tStringPtr& controlProfile, f32 sensitivity, b32 inverted ) : mControlProfile( controlProfile ), mSensitivity( sensitivity ), mInverted( inverted ) { }
		};

#if defined( platform_pcdx9 )
		const Input::tKeyboard::tButton AllowableGameControlMappings[ ] = 
		{
			Input::tKeyboard::cButton0,
			Input::tKeyboard::cButton1,
			Input::tKeyboard::cButton2,
			Input::tKeyboard::cButton3,
			Input::tKeyboard::cButton4,
			Input::tKeyboard::cButton5,
			Input::tKeyboard::cButton6,
			Input::tKeyboard::cButton7,
			Input::tKeyboard::cButton8,
			Input::tKeyboard::cButton9,

			Input::tKeyboard::cButtonA,
			Input::tKeyboard::cButtonB,
			Input::tKeyboard::cButtonC,
			Input::tKeyboard::cButtonD,
			Input::tKeyboard::cButtonE,
			Input::tKeyboard::cButtonF,
			Input::tKeyboard::cButtonG,
			Input::tKeyboard::cButtonH,
			Input::tKeyboard::cButtonI,
			Input::tKeyboard::cButtonJ,
			Input::tKeyboard::cButtonK,
			Input::tKeyboard::cButtonL,
			Input::tKeyboard::cButtonM,
			Input::tKeyboard::cButtonN,
			Input::tKeyboard::cButtonO,
			Input::tKeyboard::cButtonP,
			Input::tKeyboard::cButtonQ,
			Input::tKeyboard::cButtonR,
			Input::tKeyboard::cButtonS,
			Input::tKeyboard::cButtonT,
			Input::tKeyboard::cButtonU,
			Input::tKeyboard::cButtonV,
			Input::tKeyboard::cButtonW,
			Input::tKeyboard::cButtonX,
			Input::tKeyboard::cButtonY,
			Input::tKeyboard::cButtonZ,

			Input::tKeyboard::cButtonUp,
			Input::tKeyboard::cButtonLeft,
			Input::tKeyboard::cButtonRight,
			Input::tKeyboard::cButtonDown,

			Input::tKeyboard::cButtonMinus,
			Input::tKeyboard::cButtonEquals,
			Input::tKeyboard::cButtonTab,
			Input::tKeyboard::cButtonLBracket,
			Input::tKeyboard::cButtonRBracket,
			Input::tKeyboard::cButtonLCtrl,
			Input::tKeyboard::cButtonRCtrl,
			Input::tKeyboard::cButtonLAlt,
			Input::tKeyboard::cButtonRAlt,
			Input::tKeyboard::cButtonLShift,
			Input::tKeyboard::cButtonRShift,
			Input::tKeyboard::cButtonSpace,
			Input::tKeyboard::cButtonHome,
			Input::tKeyboard::cButtonPrior,
			Input::tKeyboard::cButtonEnd,
			Input::tKeyboard::cButtonNext,
			Input::tKeyboard::cButtonInsert,
			Input::tKeyboard::cButtonDelete,
			//Input::tKeyboard::cButtonComma,
			//Input::tKeyboard::cButtonPeriod,
			//Input::tKeyboard::cButtonSemiColon,
			//Input::tKeyboard::cButtonQuote,
			Input::tKeyboard::cButtonEnter,

			Input::tKeyboard::cButtonNumPad0,
			Input::tKeyboard::cButtonNumPad1,
			Input::tKeyboard::cButtonNumPad2,
			Input::tKeyboard::cButtonNumPad3,
			Input::tKeyboard::cButtonNumPad4,
			Input::tKeyboard::cButtonNumPad5,
			Input::tKeyboard::cButtonNumPad6,
			Input::tKeyboard::cButtonNumPad7,
			Input::tKeyboard::cButtonNumPad8,
			Input::tKeyboard::cButtonNumPad9,
			Input::tKeyboard::cButtonNumPadDec,
			Input::tKeyboard::cButtonNumPadSub,
			Input::tKeyboard::cButtonNumPadAdd,
			Input::tKeyboard::cButtonNumPadMul,
			Input::tKeyboard::cButtonNumPadDiv,

			Input::tKeyboard::cButtonF1,
			Input::tKeyboard::cButtonF2,
			Input::tKeyboard::cButtonF3,
			Input::tKeyboard::cButtonF4,
			Input::tKeyboard::cButtonF5,
			Input::tKeyboard::cButtonF6,
			Input::tKeyboard::cButtonF7,
			Input::tKeyboard::cButtonF8,
			Input::tKeyboard::cButtonF9,
			Input::tKeyboard::cButtonF10,
			Input::tKeyboard::cButtonF11,
			Input::tKeyboard::cButtonF12
			//Input::tKeyboard::cButtonF13,
			//Input::tKeyboard::cButtonF14,
			//Input::tKeyboard::cButtonF15
		};
#endif
    }

	const tGameController tGameController::cNullGameController;

	tGameController::tGameController( void )
		: mUser( *( NEW tUserPtr( ) ) )
		, mProfile( *( NEW tUserProfilePtr( ) ) )
		, mDisableMouseCursorAutoRestrict( false )
		, mCaptureNextKeyPress( false )
		, mCaptureNextKeyPressResult( 0 )
		, mLastGamepadConnected( false )
		, mBindingsChanged( false )
		, mMode(KeyboardMouse)
		, mMouseSensitivity( 0.5f )
		, mMouseInverted( 0 )
		, mMouseWheelDeltaAccumulated( 0.0f )
		, mCurrentBindingsVersion( 0 )
	{
		mMouseCursorRestrictInputFilterStack.fPushBack( 0 );
	}

	tGameController::tGameController( const tUserPtr& user, const tUserProfilePtr& profile )
		: mUser( user )
		, mProfile( profile )
		, mDisableMouseCursorAutoRestrict( false )
		, mCaptureNextKeyPress( false )
		, mCaptureNextKeyPressResult( 0 )
		, mBindingsChanged( false )
		, mMode(KeyboardMouse)
		, mMouseSensitivity( 0.5f )
		, mMouseInverted( 0 )
		, mMouseWheelDeltaAccumulated( 0.0f )
		, mCurrentBindingsVersion( 0 )
	{
		mMouseCursorRestrictInputFilterStack.fPushBack( 0 );

		//fSetUser( user );
		//fSetProfile( profile );

		fClearBindings( );

		tResourcePtr keybindingsXml = tApplication::fInstance( ).fResourceDepot( )->fQueryLoadBlock( tResourceId::fMake<tXmlFile>( cKeyBindingsFileName ), this );

		// Load default keybindings
        tXmlFile* file = keybindingsXml->fCast<tXmlFile>( );
        fLoadKeyBindingsFromFile( file );

		keybindingsXml->fUnload( this );
#if defined( platform_pcdx9 )

		// Load user set keybindings
		if( !tGameApp::cLocalAppData.fNull( ) )
		{
			tXmlDeserializer des;
			des.fLoad( tFilePathPtr::fConstructPath( tGameApp::cLocalAppData, cKeyBindingsFileName ), cKeyBindingsXmlRoot.fCStr( ), *this );
		}

		fSaveKeyBindingsLocalAppData( );

#endif
		//b32 fullscreen;
		//b32 vsync;

		//tGameApp::fInstance( ).fDevice( )->fGetDisplayMode( gDisplayWidth, gDisplayHeight, fullscreen, vsync );

		mLastGamepadConnected = fGamepadConnected( );
	}


	tGameController::~tGameController( void )
	{
	}

	void tGameController::fOnTick( f32 dt, u32 inputFilter )
	{
		if( !mUser )
		{
			return;
		}




#if defined( platform_pcdx )
		if( mCaptureNextKeyPress )
		{
			const Input::tKeyboard& kb = mUser->fRawKeyboard( );
			for( u32 i = 0; i < array_length( AllowableGameControlMappings ); ++i )
			{
				if( kb.fButtonDown( AllowableGameControlMappings[ i ] ) )
				{
					mCaptureNextKeyPressResult = AllowableGameControlMappings[ i ];
					mCaptureNextKeyPress = false;
					break;
				}
			}
		}
#endif

		u32 dummyParamUnused = 0;
		if( fCanUseKeyboardMouse( ) && mUser->fIsLocal( ) )
		{
			bool restrictMouse =  ( mUser->fInputFilterLevel( ) == fMouseCursorRestrictFilterLevel( ) ) 
				&& ( mUser->fWarningFilterLevel( ) == 0 ) 
				&& !mDisableMouseCursorAutoRestrict 
				&& ( !tGameApp::fInstance( ).fGetPlayer( 1 ) || ( tGameApp::fInstance( ).fGetPlayer( 1 ) && !tGameApp::fInstance( ).fGameMode( ).fIsLocal( ) ) || fCanUseKeyboardMouse( ) );

			mUser->fRawMouseFromScript( dummyParamUnused )->fRestrictToClientWindow( restrictMouse );
		}

		if( mBindingsChanged || ( mLastGamepadConnected != fGamepadConnected( ) ) )
		{
			mBindingsChanged = false;
			mLastGamepadConnected = fGamepadConnected( );

			tGameApp::fInstance( ).fRootCanvas( ).fHandleCanvasEvent( Logic::tEvent( GameFlags::cEVENT_INPUT_CONFIG_CHANGED ) );
		}

		//no switching with second player
		if( !tGameApp::fInstance( ).fGetPlayer( 1 ) || ( tGameApp::fInstance( ).fGetPlayer( 1 ) && !tGameApp::fInstance( ).fGameMode( ).fIsLocal( ) ) )
		{
			if(mUser->fRawGamepad( ).fActive( ))
			{
				fSetMode( GamePad );
			}
			else if(mUser->fRawMouse( ).fActive( ) || mUser->fRawKeyboard( ).fActive( ))
			{
				fSetMode( KeyboardMouse );
			}
		}

		// Per Microsoft docs, standard mouse wheel delta = 120 per notch on the wheel.
		// Some mice may implement more notches on the wheel, in which case they will return delta in partial amounts of 120
		// EG twice the notches = deltas of 60 more frequently.
		// It is recommended to accumulate the delta, and then divide by WHEEL_DELTA constant to find how many multiples of the standard
		// notches you have moved.
		// We don't do the divide here though, which allows functions to check for finer grained mouse wheel movement.
		f32 mouseWheelDelta = mUser->fRawMouse( ).fWheelDelta( );
		if( fSign( mouseWheelDelta ) != fSign( mMouseWheelDeltaAccumulated ) )
		{
			mMouseWheelDeltaAccumulated = 0.0f;
		}
		mMouseWheelDeltaAccumulated += mouseWheelDelta;

	}


	void tGameController::fSetMode( u32 mode ) 
	{

		Mode oldMode = mMode;

		mMode = (Mode)mode; //this must stay before cEVENT_INPUT_CONFIG_CHANGED is sent

		if( (Mode)mode != oldMode )
		{
			tGameApp::fInstance( ).fRootCanvas( ).fHandleCanvasEvent( Logic::tEvent( GameFlags::cEVENT_INPUT_CONFIG_CHANGED ) );
		}

	};


    template<class tSerializer>
    static void fSerializeXmlObject( tSerializer& s, tButtonMap& o )
    {
        s( "ControlProfile", o.mControlProfile );
        s( "Name", o.mName );
        s( "Key", o.mKey );
    }

    template<class tSerializer>
	static void fSerializeXmlObject( tSerializer& s, tMouseSettingMap& o )
    {
        s( "ControlProfile", o.mControlProfile );
		s( "Sensitivity", o.mSensitivity );
		s( "Inverted", o.mInverted );
    }

    template<class tSerializer>
    void fSerializeXmlObject( tSerializer& s, tGameController& o )
    {
		tGrowableArray< tButtonMap > keyboardBindings;
		tGrowableArray< tButtonMap > mouseBindings;
		tGrowableArray< tButtonMap > gamepadBindings;
		tGrowableArray< tMouseSettingMap > mouseSettings;

		u32 loadedVersion = ~0;
		if( s.fIn( ) )
		{
			s( "Version", loadedVersion );

			if( loadedVersion < o.fCurrentBindingsVersion( ) )
			{
				// old bindings version, don't load them.
				return;
			}

			o.fSetCurrentBindingsVersion( loadedVersion );

			// Start out by marking all keys as invalid
			o.fClearBindings( );
		}
		else
		{
			loadedVersion = o.fCurrentBindingsVersion( );  // copy cBindingsVersionNumber because serializer.
			s( "Version", loadedVersion );
		}

		// Take the textual values from the XML file and convert them
        // to the proper enums
		if( s.fOut( ) )
		{
			for( u32 controlProfile = tUserProfile::cProfileBegin; controlProfile < tUserProfile::cProfileEnd; ++controlProfile )
			{
				const tStringPtr& controlProfileString = tUserProfile::fControlProfileEnumToString( controlProfile );
				for( u32 i = 0; i < GameFlags::cGAME_CONTROLS_COUNT; ++i )
				{
					u32 gamepadBinding = o.fGetGamepadBinding( controlProfile, i );
					if( gamepadBinding != 0 )
					{
						const tStringPtr& valueString = GameFlags::fGAME_CONTROLSEnumToValueString( i );
						const tStringPtr& buttonString = Input::tGamepad::fButtonToString( gamepadBinding );
						tButtonMap buttonMap( controlProfileString, valueString, buttonString );
						gamepadBindings.fPushBack( buttonMap );
					}
				}
			}
		}
        s( "Gamepad", gamepadBindings );
		if( s.fIn( ) )
		{
			for( u32 i = 0; i < gamepadBindings.fCount( ); ++i )
			{
				u32 controlProfile = tUserProfile::fControlProfileStringToEnum( gamepadBindings[ i ].mControlProfile );
				if( controlProfile < tUserProfile::cProfileEnd )
				{
					u32 index = GameFlags::fGAME_CONTROLSValueStringToEnum( gamepadBindings[ i ].mName );
					if( index < GameFlags::cGAME_CONTROLS_COUNT )
						o.fSetGamepadBinding( controlProfile, index, Input::tGamepad::fStringToButton( gamepadBindings[ i ].mKey ) );
					else
						log_warning( Log::cFlagInput, "Could not convert gamepad binding to game control: " << gamepadBindings[ i ].mName );
				}
				else
				{
					log_warning( Log::cFlagInput, "Could not convert gamepad binding to control profile: " << gamepadBindings[ i ].mControlProfile );
				}
			}
		}

		if( s.fOut( ) )
		{
			for( u32 controlProfile = tUserProfile::cProfileBegin; controlProfile < tUserProfile::cProfileEnd; ++controlProfile )
			{
				const tStringPtr& controlProfileString = tUserProfile::fControlProfileEnumToString( controlProfile );
				for( u32 i = 0; i < GameFlags::cGAME_CONTROLS_COUNT; ++i )
				{
					u32 keyboardBinding = o.fGetKeyboardBinding( controlProfile, i );
					if( keyboardBinding != 0 )
					{
						const tStringPtr& valueString = GameFlags::fGAME_CONTROLSEnumToValueString( i );
						const tStringPtr& buttonString = Input::tKeyboard::fButtonToString( keyboardBinding );
						tButtonMap buttonMap( controlProfileString, valueString, buttonString );
						keyboardBindings.fPushBack( buttonMap );
					}
				}
			}
		}
        s( "Keyboard", keyboardBindings );
		if( s.fIn( ) )
		{
			for( u32 i = 0; i < keyboardBindings.fCount( ); ++i )
			{
				u32 controlProfile = tUserProfile::fControlProfileStringToEnum( keyboardBindings[ i ].mControlProfile );
				if( controlProfile < tUserProfile::cProfileEnd )
				{
					u32 index = GameFlags::fGAME_CONTROLSValueStringToEnum( keyboardBindings[ i ].mName );
					if( index < GameFlags::cGAME_CONTROLS_COUNT )
						o.fSetKeyboardBinding( controlProfile, index, Input::tKeyboard::fStringToButton( keyboardBindings[ i ].mKey ) );
					else
						log_warning( Log::cFlagInput, "Could not convert keyboard binding to game control: " << keyboardBindings[ i ].mName );
				}
				else
				{
					log_warning( Log::cFlagInput, "Could not convert keyboard binding to control profile: " << keyboardBindings[ i ].mControlProfile );
				}
			}
		}

		if( s.fOut( ) )
		{
			for( u32 controlProfile = tUserProfile::cProfileBegin; controlProfile < tUserProfile::cProfileEnd; ++controlProfile )
			{
				const tStringPtr& controlProfileString = tUserProfile::fControlProfileEnumToString( controlProfile );
				for( u32 i = 0; i < GameFlags::cGAME_CONTROLS_COUNT; ++i )
				{
					u32 mouseBinding = o.fGetMouseBinding( controlProfile, i );
					if( mouseBinding != Input::tMouse::cButtonCount )
					{
						const tStringPtr& valueString = GameFlags::fGAME_CONTROLSEnumToValueString( i );
						const tStringPtr& buttonString = Input::tMouse::fButtonToString( mouseBinding );
						tButtonMap buttonMap( controlProfileString, valueString, buttonString );
						mouseBindings.fPushBack( buttonMap );
					}
				}
			}
		}
		s( "Mouse", mouseBindings );
		if( s.fIn( ) )
		{
			for( u32 i = 0; i < mouseBindings.fCount( ); ++i )
			{
				u32 controlProfile = tUserProfile::fControlProfileStringToEnum( mouseBindings[ i ].mControlProfile );
				if( controlProfile < tUserProfile::cProfileEnd )
				{
					u32 index = GameFlags::fGAME_CONTROLSValueStringToEnum( mouseBindings[ i ].mName );
					if( index < GameFlags::cGAME_CONTROLS_COUNT )
						o.fSetMouseBinding( controlProfile, index, Input::tMouse::fStringToButton( mouseBindings[ i ].mKey ) );
					else
						log_warning( Log::cFlagInput, "Could not convert mouse binding to game control: " << mouseBindings[ i ].mName );
				}
				else
				{
					log_warning( Log::cFlagInput, "Could not convert mouse binding to control profile: " << mouseBindings[ i ].mControlProfile );
				}
			}
		}

		if( s.fOut( ) )
		{
			for( u32 controlProfile = tUserProfile::cProfileBegin; controlProfile < tUserProfile::cProfileEnd; ++controlProfile )
			{
				const tStringPtr& controlProfileString = tUserProfile::fControlProfileEnumToString( controlProfile );
				tMouseSettingMap mouseSettingMap( controlProfileString, o.fMouseSensitivity( controlProfile ), o.fMouseInverted( controlProfile ) );
				mouseSettings.fPushBack( mouseSettingMap );
			}
		}
		s( "MouseSettings", mouseSettings );
		if( s.fIn( ) )
		{
			for( u32 i = 0; i < mouseSettings.fCount( ); ++i )
			{
				u32 controlProfile = tUserProfile::fControlProfileStringToEnum( mouseSettings[ i ].mControlProfile );
				if( controlProfile < tUserProfile::cProfileEnd )
				{
					o.fSetMouseSensitivity( controlProfile, mouseSettings[ i ].mSensitivity );
					o.fSetMouseInverted( controlProfile, mouseSettings[ i ].mInverted );
				}
				else
				{
					log_warning( Log::cFlagInput, "Could not convert mouse setting to control profile: " << mouseSettings[ i ].mControlProfile );
				}
			}
		}
	}

	b32 tGameController::fLoadKeyBindingsFromFile( tXmlFile* xmlFile )
	{
		const char* data = xmlFile->fBegin( );

		tXmlDeserializer des;
		b32 result = des.fParse( data, cKeyBindingsXmlRoot.fCStr( ), *this );
		return result;
	}

	b32 tGameController::fSaveKeyBindingsLocalAppData( )
	{
		b32 result = false;

#if defined( platform_pcdx9 )
		if( !tGameApp::cLocalAppData.fNull( ) ) 
		{
			tXmlSerializer des;
			result = des.fSave( tFilePathPtr::fConstructPath( tGameApp::cLocalAppData, cKeyBindingsFileName ), cKeyBindingsXmlRoot.fCStr( ), *this, false, false );
		}
#endif

		return result;
	}

	b32 tGameController::fIsActive( ) const
	{
		if( mMode == GamePad )
		{
			return ( mUser->fRawGamepad( ).fActive( ) );
		}
		else
		{
			return ( mUser->fRawMouse( ).fActive( ) || mUser->fRawKeyboard( ).fActive( ) );
		}
	}

	b32 tGameController::fButtonHeld( u32 controlProfile, u32 gameControlsFlag ) const
	{
		return fButtonHeld( controlProfile, gameControlsFlag, 0 );
	}

	b32 tGameController::fButtonHeld( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const
	{
		if( !mUser )
		{
			return false;
		}


		u32 gamepadBinding = fGetGamepadBinding( controlProfile, gameControlsFlag );
		if( ( fCanUseGamePad( ) && gamepadBinding != 0 ) && ( mUser->fFilteredGamepad( inputFilter ).fButtonHeld( gamepadBinding ) ) )
		{
			return true;
		}

		u32 keyboardBinding = fGetKeyboardBinding( controlProfile, gameControlsFlag );
		if( ( fCanUseKeyboardMouse( ) && keyboardBinding != 0 ) && ( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( keyboardBinding ) ) )
		{
			return true;
		}

		u32 mouseBinding = fGetMouseBinding( controlProfile, gameControlsFlag );
		if( ( fCanUseKeyboardMouse( ) && mouseBinding != Input::tMouse::cButtonCount ) && ( mUser->fFilteredMouse( inputFilter ).fButtonHeld( mouseBinding ) ) )
		{
			return true;
		}
		/*
		// Special case handling, for keyboard/mouse "gamepad stick" replacements
		switch( controlProfile )
		{
		case tUserProfile::cProfileCamera:
			{

				if( fMouseAsMoveStickHeld( controlProfile, gameControlsFlag, inputFilter ) )
				{
					return true;
				}
			}
			break;

		default:
			{
				if( fMouseAsAimStickHeld( controlProfile, gameControlsFlag, inputFilter ) )
				{
					return true;
				}
			}
			break;
		}
		*/
		return false;
	}

	b32 tGameController::fButtonDown( u32 controlProfile, u32 gameControlsFlag ) const
	{
		return fButtonDown( controlProfile, gameControlsFlag, 0 );
	}

    b32 tGameController::fButtonDown( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const
	{
		if( !mUser )
		{
			return false;
		}

		u32 gamepadBinding = fGetGamepadBinding( controlProfile, gameControlsFlag );
		if( ( fCanUseGamePad( ) && gamepadBinding != 0 ) && ( mUser->fFilteredGamepad( inputFilter ).fButtonDown( gamepadBinding ) ) )
		{
			return true;
		}

		u32 keyboardBinding = fGetKeyboardBinding( controlProfile, gameControlsFlag );
		if( ( fCanUseKeyboardMouse( ) && keyboardBinding != 0 ) && ( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( keyboardBinding ) ) )
		{
			return true;
		}

		u32 mouseBinding = fGetMouseBinding( controlProfile, gameControlsFlag );
		if( ( fCanUseKeyboardMouse( ) && mouseBinding != Input::tMouse::cButtonCount ) && ( mUser->fFilteredMouse( inputFilter ).fButtonDown( mouseBinding ) ) )
		{
			return true;
		}

		// Special case handling, for keyboard/mouse "gamepad stick" replacements
		switch( controlProfile )
		{
		case tUserProfile::cProfileCamera:
			{
			}
			break;

		case tUserProfile::cProfileTurrets:
			{
				if( gameControlsFlag == GameFlags::cGAME_CONTROLS_CAMERA_MOVE_IN )
				{
					if( ( mMouseWheelDeltaAccumulated / WHEEL_DELTA ) > 0.0f )
					{
						return true;
					}
				}
				else if( gameControlsFlag == GameFlags::cGAME_CONTROLS_CAMERA_MOVE_OUT )
				{
					if( ( mMouseWheelDeltaAccumulated / WHEEL_DELTA ) < 0.0f )
					{
						return true;
					}
				}
			}
			// Drop through to default

		default:
			{
				if( fMouseAsAimStickDown( controlProfile, gameControlsFlag, inputFilter ) )
				{
					return true;
				}
			}
			break;
		}
		return false;
	}

    b32 tGameController::fButtonUp( u32 controlProfile, u32 gameControlsFlag ) const
	{
		return fButtonUp( controlProfile, gameControlsFlag, 0 );
	}

    b32 tGameController::fButtonUp( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const
	{
		if( !mUser )
		{
			return false;
		}

		u32 gamepadBinding = fGetGamepadBinding( controlProfile, gameControlsFlag );
		if( ( fCanUseGamePad() && gamepadBinding != 0 ) && ( mUser->fFilteredGamepad( inputFilter ).fButtonUp( gamepadBinding ) ) )
		{
			return true;
		}

		u32 keyboardBinding = fGetKeyboardBinding( controlProfile, gameControlsFlag );
		if( ( fCanUseKeyboardMouse( ) && keyboardBinding != 0 ) && ( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( keyboardBinding ) ) )
		{
			return true;
		}

		u32 mouseBinding = fGetMouseBinding( controlProfile, gameControlsFlag );
		if( ( fCanUseKeyboardMouse( ) && mouseBinding != Input::tMouse::cButtonCount ) && ( mUser->fFilteredMouse( inputFilter ).fButtonUp( mouseBinding ) ) )
		{
			return true;
		}
	
		// Special case handling, for keyboard/mouse "gamepad stick" replacements
		switch( controlProfile )
		{
		case tUserProfile::cProfileCamera:
			{

			}
			break;

		default:
			{
				if( fMouseAsAimStickUp( controlProfile, gameControlsFlag, inputFilter ) )
				{
					return true;
				}
			}
			break;
		}
		
		return false;
	}

	Math::tVec2f tGameController::fAimStick( u32 controlProfile ) const 
	{ 
		return fAimStick( controlProfile, 0 );
	}

	Math::tVec2f tGameController::fAimStick( u32 controlProfile, u32 inputFilter ) const 
	{ 
		if( !mProfile || !mUser )
		{
			return Math::tVec2f::cZeroVector;
		}

		Math::tVec2f result = mProfile->fSouthPaw(controlProfile) ? mUser->fFilteredGamepad(inputFilter).fLeftStick( ) : mUser->fFilteredGamepad(inputFilter).fRightStick( );
		if( mProfile->fInversion( controlProfile ) )
		{
			result.y *= -1.f;
		}

		if( !fCanUseGamePad( ) )
		{
			result = Math::tVec2f::cZeroVector;
		}

		if( fCanUseKeyboardMouse( ) )
		{
			Math::tVec2f aimDir = Math::tVec2f::cZeroVector;
			switch ( controlProfile )
			{
				case tUserProfile::cProfileCamera:
				{
					aimDir = fMouseToStick(controlProfile, inputFilter);
					aimDir.y = mUser->fFilteredMouse(inputFilter).fWheelDelta();
					break;
				}

				default:
				{
					aimDir = fMouseToStick(controlProfile, inputFilter);
					break;
				}

			}

			if( aimDir.fLengthSquared( ) > cSimulatedStickMinMag )
			{
				result = aimDir;
				if( fMouseInverted( controlProfile ) )
				{
					result.y *= -1.f;
				}
			}
		}

		return result;
	}



	f32 tGameController::fAimStickMagnitude(u32 controlProfile, u32 inputFilter) const
	{
		if (!mProfile || !mUser)
		{
			return 0.0f;
		}

		f32 magnitude = mProfile->fSouthPaw(controlProfile) ? mUser->fFilteredGamepad(inputFilter).fLeftStickMagnitude() : mUser->fFilteredGamepad(inputFilter).fRightStickMagnitude();

		if (!fCanUseGamePad())
		{
			magnitude = 0.0f;
		}

		if (magnitude <= cSimulatedStickMinMag)
		{

			if (fCanUseKeyboardMouse())
			{

				Math::tVec2f aimDir = Math::tVec2f::cZeroVector;
				switch (controlProfile)
				{
					case tUserProfile::cProfileCamera:
					{
						aimDir = fMouseToStick(controlProfile, inputFilter);
						aimDir.y = mUser->fFilteredMouse(inputFilter).fWheelDelta();
						break;
					}

					default:
					{
						aimDir = fMouseToStick(controlProfile, inputFilter);
						break;
					}
				}
				magnitude = aimDir.fLength();
			}

		}

		return magnitude;
	}

	//u32 tGameController::fAimThumbButton( u32 profileControlType ) const
	//{
	//	return mProfile->fSouthPaw( profileControlType ) ? GameFlags::cGAME_CONTROLS_GAME_LEFT_THUMB : GameFlags::cGAME_CONTROLS_GAME_RIGHT_THUMB;
	//}
	
	Math::tVec2f tGameController::fMoveStick( u32 controlProfile ) const
	{
		return fMoveStick( controlProfile, 0 );
	}

	Math::tVec2f tGameController::fMoveStick( u32 controlProfile, u32 inputFilter ) const
	{
		if( !mProfile || !mUser )
		{
			return Math::tVec2f::cZeroVector;
		}


		Math::tVec2f moveDir = Math::tVec2f::cZeroVector;
		moveDir = fKeysToStick( controlProfile, inputFilter );

		switch( controlProfile )
		{
		case tUserProfile::cProfileCamera:
			{

//				moveDir = fMouseToStick( controlProfile, inputFilter );
				moveDir = fKeysToStick( controlProfile, inputFilter );
				if( fCanUseKeyboardMouse( ) )
				{
					fMouseTouchScreenEdgeToMoveStick(controlProfile, inputFilter, moveDir);
				}

			}
			break;

		default:
			{
				moveDir = fKeysToStick( controlProfile, inputFilter );
			}
			break;

		}

		if( moveDir.fLengthSquared( ) > cSimulatedStickMinMag )
		{
			return moveDir;
		}

		if( !fCanUseGamePad( ) )
			return Math::tVec2f::cZeroVector;

		return mProfile->fSouthPaw( controlProfile ) ? mUser->fFilteredGamepad( inputFilter ).fRightStick( ) : mUser->fFilteredGamepad( inputFilter ).fLeftStick( );
	}

	f32 tGameController::fMoveStickMagnitude( u32 controlProfile, u32 inputFilter ) const
	{
		if( !mProfile || !mUser )
		{
			return 0.0f;
		}

		Math::tVec2f moveDir = Math::tVec2f::cZeroVector;
		switch( controlProfile )
		{
		case tUserProfile::cProfileCamera:
			{

//				moveDir = fMouseToStick( controlProfile, inputFilter );
				moveDir = fKeysToStick( controlProfile, inputFilter );

				if( fCanUseKeyboardMouse( ) )
				{
					fMouseTouchScreenEdgeToMoveStick(controlProfile, inputFilter, moveDir);
				}

			}
			break;

		default:
			{
				moveDir = fKeysToStick( controlProfile, inputFilter );
			}
			break;

		}

		if( moveDir.fLengthSquared( ) > cSimulatedStickMinMag )
		{
			return moveDir.fLength( );
		}

		if( !fCanUseGamePad( ) )
			return 0.0f;


		return mProfile->fSouthPaw( controlProfile ) ? mUser->fFilteredGamepad( inputFilter ).fRightStickMagnitude( ) : mUser->fFilteredGamepad( inputFilter ).fLeftStickMagnitude( );
	}
	
	//u32 tGameController::fMoveThumbButton( u32 profileControlType ) const
	//{
	//	return mProfile->fSouthPaw( profileControlType ) ? GameFlags::cGAME_CONTROLS_GAME_RIGHT_THUMB : GameFlags::cGAME_CONTROLS_GAME_LEFT_THUMB;
	//}

	f32 tGameController::fGetAcceleration( u32 controlProfile, u32 inputFilter ) const
	{
		if( !mProfile || !mUser )
		{
			return 0.0f;
		}

		f32 kbAccel = 0.0f;
		u32 kbBinding = fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_ACCELERATE );
		if( fCanUseKeyboardMouse( ) && (mUser->fFilteredKeyboard( inputFilter ).fButtonDown( kbBinding ) || mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( kbBinding )) )
		{
			kbAccel = 1.0f;
		}

		if( kbAccel > 0.0f )
		{
			return kbAccel;
		}

		if( !fCanUseGamePad( ) )
			return 0.0f;


		return mUser->fFilteredGamepad( inputFilter ).fRightTriggerPressure( );
	}

	f32 tGameController::fGetDeceleration( u32 controlProfile, u32 inputFilter ) const
	{
		if( !mProfile || !mUser )
		{
			return 0.0f;
		}

		f32 kbDecel = 0.0f;
		u32 kbBinding = fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_DECCELERATE );
		if(  fCanUseKeyboardMouse( ) &&  (mUser->fFilteredKeyboard( inputFilter ).fButtonDown( kbBinding ) || mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( kbBinding )) )
		{
			kbDecel = 1.0f;
		}

		if( kbDecel > 0.0f )
		{
			return kbDecel;
		}

		if( !fCanUseGamePad( ) )
			return 0.0f;

		return mUser->fFilteredGamepad( inputFilter ).fLeftTriggerPressure( );
	}

	Math::tVec2f tGameController::fMenuStick( ) const
	{
		return fMenuStick( 0 );
	}

	Math::tVec2f tGameController::fMenuStick( u32 inputFilter ) const
	{
		if( !mUser )
		{
			return Math::tVec2f::cZeroVector;
		}

		if( fCanUseKeyboardMouse( ) )
		{
			Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
			if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( tUserProfile::cProfileUI, GameFlags::cGAME_CONTROLS_MENU_UP ) ) || mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( tUserProfile::cProfileUI, GameFlags::cGAME_CONTROLS_MENU_UP ) ) )
			{
				kbDir.y = 1.0f;
			}
			if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( tUserProfile::cProfileUI, GameFlags::cGAME_CONTROLS_MENU_DOWN ) ) || mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( tUserProfile::cProfileUI, GameFlags::cGAME_CONTROLS_MENU_DOWN ) ) )
			{
				kbDir.y = -1.0f;
			}
			if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( tUserProfile::cProfileUI, GameFlags::cGAME_CONTROLS_MENU_LEFT ) ) || mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( tUserProfile::cProfileUI, GameFlags::cGAME_CONTROLS_MENU_LEFT ) ) )
			{
				kbDir.x = -1.0f;
			}
			if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( tUserProfile::cProfileUI, GameFlags::cGAME_CONTROLS_MENU_RIGHT ) ) || mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( tUserProfile::cProfileUI, GameFlags::cGAME_CONTROLS_MENU_RIGHT ) ) )
			{
				kbDir.x = 1.0f;
			}

			kbDir = kbDir.fNormalizeSafe( );

			if( kbDir.fLengthSquared( ) > 0.0f )
			{
				return kbDir;
			}

//			kbDir.x = mUser->fFilteredMouse( inputFilter ).fWheelDelta();

			if( kbDir.fLengthSquared( ) > 0.0f )
			{
				return kbDir;
			}

		}


		return mUser->fFilteredGamepad( inputFilter ).fLeftStick( );
	}

	Math::tVec2f tGameController::fMouseToStick( u32 controlProfile, u32 inputFilter ) const
	{

		if( !fCanUseKeyboardMouse( ) )
		{
			return Math::tVec2f::cZeroVector;
		}

		Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );

		// TS1 style
		f32 maxDeltaLen = GameController_MouseMaxDeltaDefault;
		if( controlProfile == tUserProfile::cProfileCamera )
		{
			maxDeltaLen = GameController_MouseMaxDeltaCamera;
		}

		f32 deltaMag = mouseDelta.fLength( );
		if( deltaMag >= maxDeltaLen )
		{
			mouseDelta /= deltaMag;
		}
		else
		{
			mouseDelta /= maxDeltaLen;
		}
		mouseDelta.y = -mouseDelta.y;

		const f32 slider = fMouseSensitivity( controlProfile );
		mouseDelta *= GameController_MouseSensFloor + ( GameController_MouseSensRange * slider );

		return mouseDelta;
	}

	b32 tGameController::fMouseTouchScreenEdgeToMoveStick( u32 controlProfile, u32 inputFilter, Math::tVec2f &stick ) const
	{
		if( tGameApp::fInstance( ).fIsDisplayCase( ) )
			return false;

		Math::tVec2f mousePosition = mUser->fFilteredMouse( inputFilter ).fPosition( );
		Math::tVec2f mouseCoordRange = mUser->fFilteredMouse( inputFilter ).fCoordRange( );

		// bounds check of the window
		if( ( mousePosition.x > mouseCoordRange.x ) || ( mousePosition.y > mouseCoordRange.y )
			|| ( mousePosition.x < 0.0f ) || ( mousePosition.y < 0.0f ) )
			return false;

		f32 borderWidth = ( f32 )mouseCoordRange.x * 0.01f;
		f32 borderHeight = ( f32 )mouseCoordRange.y * 0.01f;

		if( ( mousePosition.x >= ( mouseCoordRange.x - borderWidth ) && mousePosition.x <= mouseCoordRange.x ) ||
			( mousePosition.y >= ( mouseCoordRange.y - borderHeight ) && mousePosition.y <= mouseCoordRange.y ) ||
			( mousePosition.x <= borderWidth && mousePosition.x >= 0.0f ) ||
			( mousePosition.y <= borderHeight && mousePosition.y >= 0.0f ) )
		{
			Math::tVec2f screenCenter( ( f32 )mouseCoordRange.x * 0.5f, ( f32 )mouseCoordRange.y * 0.5f );
			Math::tVec2f stickDir = mousePosition - screenCenter;
			stickDir.y = -stickDir.y;
			stickDir = stickDir.fNormalize( );

			stick += stickDir;
			stick = stick.fNormalize( );

			return true;
		}

		return false;
	}


	b32 tGameController::fMouseAsAimStickHeld( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const
	{
		if( !fCanUseKeyboardMouse( ) )
		{
			return false;
		}

		switch( gameControlsFlag )
		{
		case GameFlags::cGAME_CONTROLS_MOVE_MINMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) )
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) )
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) )
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) )
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMinMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_MOVE_MAXMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) )
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) )
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) )
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) )
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMaxMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MINMAG:
			{
				// Was the mouse idle last frame but moved at all this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f previousMouseDelta = Math::tVec2f::fConstruct( previousFrameStateData.mCursorDeltaX, previousFrameStateData.mCursorDeltaY );
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousMouseDelta.fLengthSquared( ) > cSimulatedStickMinMag ) &&
					( mouseDelta.fLengthSquared( ) > cSimulatedStickMinMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MAXMAG:
			{
				// Was the mouse moving slowly last frame but quickly this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f previousMouseDelta = Math::tVec2f::fConstruct( previousFrameStateData.mCursorDeltaX, previousFrameStateData.mCursorDeltaY );
				previousMouseDelta.fNormalizeSafe( );
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				mouseDelta = mouseDelta.fNormalizeSafe( );
				if( ( previousMouseDelta.fLengthSquared( ) > cSimulatedStickMaxMag ) && ( mouseDelta.fLengthSquared( ) > cSimulatedStickMaxMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_LEFT:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaX < 0 ) && ( mouseDelta.x < 0.0f ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_RIGHT:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaX > 0 ) && ( mouseDelta.x > 0.0f ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_UP:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaY < 0 ) && ( mouseDelta.y < 0.0f ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_DOWN:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaY > 0 ) && ( mouseDelta.y > 0.0f ) )
				{
					return true;
				}
			}
			break;

		}

		return false;
	}

	b32 tGameController::fMouseAsAimStickDown( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const
	{
		if( !fCanUseKeyboardMouse( ) )
		{
			return false;
		}


		switch( gameControlsFlag )
		{
		case GameFlags::cGAME_CONTROLS_MOVE_MINMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) )
					)
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMinMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_MOVE_MAXMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) )
					)
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMaxMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MINMAG:
			{
				// Was the mouse idle last frame but moved at all this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( ( previousFrameStateData.mCursorDeltaX == 0 ) && ( previousFrameStateData.mCursorDeltaY == 0 ) ) &&
					( mouseDelta.fLengthSquared( ) > cSimulatedStickMinMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MAXMAG:
			{
				// Was the mouse moving slowly last frame but quickly this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f previousMouseDelta = Math::tVec2f::fConstruct( previousFrameStateData.mCursorDeltaX, previousFrameStateData.mCursorDeltaY );
				previousMouseDelta.fNormalizeSafe( );
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				mouseDelta = mouseDelta.fNormalizeSafe( );
				if( ( previousMouseDelta.fLengthSquared( ) < cSimulatedStickMaxMag ) && ( mouseDelta.fLengthSquared( ) > cSimulatedStickMaxMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_LEFT:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaX >= 0 ) && ( mouseDelta.x < 0.0f ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_RIGHT:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaX <= 0 ) && ( mouseDelta.x > 0.0f ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_UP:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaY >= 0 ) && ( mouseDelta.y < 0.0f ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_DOWN:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaY <= 0 ) && ( mouseDelta.y > 0.0f ) )
				{
					return true;
				}
			}
			break;

		}

		return false;
	}

	b32 tGameController::fMouseAsAimStickUp( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const
	{
		if( !fCanUseKeyboardMouse( ) )
		{
			return false;
		}


		switch( gameControlsFlag )
		{
		case GameFlags::cGAME_CONTROLS_MOVE_MINMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) )
					)
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMinMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_MOVE_MAXMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) )
					)
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMaxMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MINMAG:
			{
				// Was the mouse idle last frame but moved at all this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( ( previousFrameStateData.mCursorDeltaX != 0 ) && ( previousFrameStateData.mCursorDeltaY != 0 ) ) &&
					( mouseDelta.fLengthSquared( ) <= cSimulatedStickMinMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MAXMAG:
			{
				// Was the mouse moving slowly last frame but quickly this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f previousMouseDelta = Math::tVec2f::fConstruct( previousFrameStateData.mCursorDeltaX, previousFrameStateData.mCursorDeltaY );
				previousMouseDelta.fNormalizeSafe( );
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				mouseDelta = mouseDelta.fNormalizeSafe( );
				if( ( previousMouseDelta.fLengthSquared( ) > cSimulatedStickMaxMag ) && ( mouseDelta.fLengthSquared( ) <= cSimulatedStickMaxMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_LEFT:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaX < 0 ) && ( mouseDelta.x >= 0.0f ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_RIGHT:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaX > 0 ) && ( mouseDelta.x <= 0.0f ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_UP:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaY < 0 ) && ( mouseDelta.y >= 0.0f ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_DOWN:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousFrameStateData.mCursorDeltaY > 0 ) && ( mouseDelta.y <= 0.0f ) )
				{
					return true;
				}
			}
			break;

		}

		return false;
	}

	b32 tGameController::fMouseAsMoveStickHeld( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const
	{

		if( !fCanUseKeyboardMouse( ) )
		{
			return false;
		}


		switch( gameControlsFlag )
		{
		case GameFlags::cGAME_CONTROLS_MOVE_MINMAG:
			{
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f previousMouseDelta = Math::tVec2f::fConstruct( previousFrameStateData.mCursorDeltaX, previousFrameStateData.mCursorDeltaY );
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( previousMouseDelta.fLengthSquared( ) > cSimulatedStickMinMag ) &&
					( mouseDelta.fLengthSquared( ) > cSimulatedStickMinMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_MOVE_MAXMAG:
			{
				// Was the mouse moving slowly last frame but quickly this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f previousMouseDelta = Math::tVec2f::fConstruct( previousFrameStateData.mCursorDeltaX, previousFrameStateData.mCursorDeltaY );
				previousMouseDelta.fNormalizeSafe( );
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				mouseDelta = mouseDelta.fNormalizeSafe( );
				if( ( previousMouseDelta.fLengthSquared( ) > cSimulatedStickMaxMag ) && ( mouseDelta.fLengthSquared( ) > cSimulatedStickMaxMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MINMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) )
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) )
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) )
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) )
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMinMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MAXMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) )
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) )
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) )
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) )
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMaxMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_LEFT:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_RIGHT:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_UP:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_DOWN:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) )
				{
					return true;
				}
			}
			break;

		}

		return false;
	}

	b32 tGameController::fMouseAsMoveStickDown( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const
	{

		if( !fCanUseKeyboardMouse( ) )
		{
			return false;
		}


		switch( gameControlsFlag )
		{
		case GameFlags::cGAME_CONTROLS_MOVE_MINMAG:
			{
				// Was the mouse idle last frame but moved at all this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( ( previousFrameStateData.mCursorDeltaX == 0 ) && ( previousFrameStateData.mCursorDeltaY == 0 ) ) &&
					( mouseDelta.fLengthSquared( ) > cSimulatedStickMinMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_MOVE_MAXMAG:
			{
				// Was the mouse moving slowly last frame but quickly this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f previousMouseDelta = Math::tVec2f::fConstruct( previousFrameStateData.mCursorDeltaX, previousFrameStateData.mCursorDeltaY );
				previousMouseDelta.fNormalizeSafe( );
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				mouseDelta = mouseDelta.fNormalizeSafe( );
				if( ( previousMouseDelta.fLengthSquared( ) < cSimulatedStickMaxMag ) && ( mouseDelta.fLengthSquared( ) > cSimulatedStickMaxMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MINMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) )
					)
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMinMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MAXMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) )
					)
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMaxMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_LEFT:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_RIGHT:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_UP:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_DOWN:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) )
				{
					return true;
				}
			}
			break;

		}

		return false;
	}

	b32 tGameController::fMouseAsMoveStickUp( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const
	{

		if( !fCanUseKeyboardMouse( ) )
		{
			return false;
		}


		switch( gameControlsFlag )
		{
		case GameFlags::cGAME_CONTROLS_MOVE_MINMAG:
			{
				// Was the mouse idle last frame but moved at all this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				if( ( ( previousFrameStateData.mCursorDeltaX != 0 ) && ( previousFrameStateData.mCursorDeltaY != 0 ) ) &&
					( mouseDelta.fLengthSquared( ) <= cSimulatedStickMinMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_MOVE_MAXMAG:
			{
				// Was the mouse moving slowly last frame but quickly this frame?
				const Input::tMouse::tStateDataBuffer& stateHistory = mUser->fFilteredMouse( inputFilter ).fGetStateHistory( );
				const Input::tMouse::tStateData& previousFrameStateData = stateHistory[ stateHistory.fNumItems( ) - 2 ];
				Math::tVec2f previousMouseDelta = Math::tVec2f::fConstruct( previousFrameStateData.mCursorDeltaX, previousFrameStateData.mCursorDeltaY );
				previousMouseDelta.fNormalizeSafe( );
				Math::tVec2f mouseDelta = mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
				mouseDelta = mouseDelta.fNormalizeSafe( );
				if( ( previousMouseDelta.fLengthSquared( ) > cSimulatedStickMaxMag ) && ( mouseDelta.fLengthSquared( ) <= cSimulatedStickMaxMag ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MINMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) )
					)
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMinMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_MAXMAG:
			{
				Math::tVec2f kbDir = Math::tVec2f::cZeroVector;
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = 1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.y = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) )
					)
				{
					kbDir.x = -1.0f;
				}
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) &&
					!mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) )
					)
				{
					kbDir.x = 1.0f;
				}

				kbDir = kbDir.fNormalizeSafe( );

				if( kbDir.fLengthSquared( ) > cSimulatedStickMaxMag )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_LEFT:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_RIGHT:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_UP:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) )
				{
					return true;
				}
			}
			break;

		case GameFlags::cGAME_CONTROLS_AIM_DOWN:
			{
				if( mUser->fFilteredKeyboard( inputFilter ).fButtonUp( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) )
				{
					return true;
				}
			}
			break;

		}

		return false;
	}

	Math::tVec2f tGameController::fKeysToStick( u32 controlProfile, u32 inputFilter ) const
	{
		
		Math::tVec2f kbDir = Math::tVec2f::cZeroVector;

		if( !fCanUseKeyboardMouse( ) )
		{
			return kbDir;
		}

		if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) || mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP ) ) )
		{
			kbDir.y = 1.0f;
		}
		if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) || mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN ) ) )
		{
			kbDir.y = -1.0f;
		}
		if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) || mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT ) ) )
		{
			kbDir.x = -1.0f;
		}
		if( mUser->fFilteredKeyboard( inputFilter ).fButtonDown( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) || mUser->fFilteredKeyboard( inputFilter ).fButtonHeld( fGetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT ) ) )
		{
			kbDir.x = 1.0f;
		}

		kbDir = kbDir.fNormalizeSafe( );
		
		return kbDir;
	}

	void tGameController::fClearBindings( )
	{
		for( int controlProfile = tUserProfile::cProfileBegin; controlProfile < tUserProfile::cProfileEnd; ++controlProfile )
		{
			mBindings[ controlProfile - tUserProfile::cProfileBegin ].mKeyboardBindings.fFill( 0 );
			mBindings[ controlProfile - tUserProfile::cProfileBegin ].mMouseBindings.fFill( Input::tMouse::cButtonCount );
			mBindings[ controlProfile - tUserProfile::cProfileBegin ].mGamepadBindings.fFill( 0 );

			//// TEMP: hardcoded mapping until xml loading is implemented
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_ACCEPT, Input::tGamepad::cButtonA );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_CANCEL, Input::tGamepad::cButtonB );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_GAMERCARD, Input::tGamepad::cButtonSelect );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_PAUSE, Input::tGamepad::cButtonStart );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_REWIND, Input::tGamepad::cButtonSelect );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_UP, Input::tGamepad::cButtonDPadUp );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_DOWN, Input::tGamepad::cButtonDPadDown );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_LEFT, Input::tGamepad::cButtonDPadLeft );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_RIGHT, Input::tGamepad::cButtonDPadRight );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_CYCLE_LEADERBOARD_FILTER, Input::tGamepad::cButtonX );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_SHOW_INVITE_UI, Input::tGamepad::cButtonX );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_SHOW_LEADERBOARDS, Input::tGamepad::cButtonY );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_PREVIOUS_PAGE, Input::tGamepad::cButtonLShoulder );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_NEXT_PAGE, Input::tGamepad::cButtonRShoulder );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_SELECT, Input::tGamepad::cButtonA );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_CANCEL, Input::tGamepad::cButtonB );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_PLACEMENT_MENU, Input::tGamepad::cButtonRTrigger );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_LAUNCH_WAVE, Input::tGamepad::cButtonX );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_LAUNCH_BARRAGE, Input::tGamepad::cButtonY );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_CAMERA_CYCLE, Input::tGamepad::cButtonLThumb );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_CAMERA_CENTER, Input::tGamepad::cButtonLThumb );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_CAMERA_SPEED, Input::tGamepad::cButtonLTrigger );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_CAMERA_FOCUS_EVENT, Input::tGamepad::cButtonY );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_JUMP, Input::tGamepad::cButtonA );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MELEE, Input::tGamepad::cButtonLShoulder );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_LEFT, Input::tGamepad::cButtonLShoulder );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_RIGHT, Input::tGamepad::cButtonRShoulder );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_ENTER_EXIT_UNIT, Input::tGamepad::cButtonY );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_ACCELERATE, Input::tGamepad::cButtonRTrigger );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_DECCELERATE, Input::tGamepad::cButtonLTrigger );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_BOOST, Input::tGamepad::cButtonA );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_HANDBRAKE, Input::tGamepad::cButtonX );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_DECREASE_ALTITUDE, Input::tGamepad::cButtonLShoulder );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_INCREASE_ALTITUDE, Input::tGamepad::cButtonRShoulder );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY, Input::tGamepad::cButtonRTrigger );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_TRIGGER_SECONDARY, Input::tGamepad::cButtonLTrigger );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_FORWARD, Input::tGamepad::cButtonDPadUp );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_BEHIND, Input::tGamepad::cButtonDPadDown );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_LEFT, Input::tGamepad::cButtonDPadLeft );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_RIGHT, Input::tGamepad::cButtonDPadRight );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_QUICK_USE, Input::tGamepad::cButtonDPadUp );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_QUICK_SELL, Input::tGamepad::cButtonDPadLeft );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_QUICK_UPGRADE, Input::tGamepad::cButtonDPadRight );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_QUICK_REPAIR, Input::tGamepad::cButtonDPadDown );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT, Input::tGamepad::cButtonLStickLeft );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT, Input::tGamepad::cButtonLStickRight );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP, Input::tGamepad::cButtonLStickUp );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN, Input::tGamepad::cButtonLStickDown );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_MINMAG, Input::tGamepad::cButtonLThumbMinMag );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_MAXMAG, Input::tGamepad::cButtonLThumbMaxMag );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_MINMAG, Input::tGamepad::cButtonRThumbMinMag );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_MAXMAG, Input::tGamepad::cButtonRThumbMaxMag );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_LEFT, Input::tGamepad::cButtonRStickLeft );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_RIGHT, Input::tGamepad::cButtonRStickRight );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_UP, Input::tGamepad::cButtonRStickUp );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_DOWN, Input::tGamepad::cButtonRStickDown );
			//fSetGamepadBinding( controlProfile, GameFlags::cGAME_CONTROLS_BARBED_WIRE, Input::tGamepad::cButtonLThumb );

			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_ACCEPT, Input::tKeyboard::cButtonEnter );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_CANCEL, Input::tKeyboard::cButtonEscape );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_GAMERCARD, Input::tKeyboard::cButtonSelect );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_PAUSE, Input::tKeyboard::cButtonEscape );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_REWIND, Input::tKeyboard::cButtonLCtrl );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_UP, Input::tKeyboard::cButtonUp );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_DOWN, Input::tKeyboard::cButtonDown );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_LEFT, Input::tKeyboard::cButtonLeft );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_RIGHT, Input::tKeyboard::cButtonRight );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_CYCLE_LEADERBOARD_FILTER, Input::tKeyboard::cButtonSpace );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_SHOW_INVITE_UI, Input::tKeyboard::cButtonI );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_SHOW_LEADERBOARDS, Input::tKeyboard::cButtonTab );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_PREVIOUS_PAGE, Input::tKeyboard::cButtonMinus );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MENU_NEXT_PAGE, Input::tKeyboard::cButtonEquals );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_SELECT, Input::tKeyboard::cButtonEnter );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_CANCEL, Input::tKeyboard::cButtonEscape );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_PLACEMENT_MENU, Input::tKeyboard::cButtonRTrigger );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_LAUNCH_WAVE, Input::tKeyboard::cButtonEnter );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_LAUNCH_BARRAGE, Input::tKeyboard::cButtonSpace );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_CAMERA_CYCLE, Input::tKeyboard::cButtonSpace );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_CAMERA_CENTER, Input::tKeyboard::cButtonNumPad5 );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_CAMERA_SPEED, Input::tKeyboard::cButtonLShift );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_CAMERA_FOCUS_EVENT, Input::tKeyboard::cButtonF );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_JUMP, Input::tKeyboard::cButtonSpace );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MELEE, Input::tKeyboard::cButtonE );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_LEFT, Input::tKeyboard::cButtonQ );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_RIGHT, Input::tKeyboard::cButtonE );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_ENTER_EXIT_UNIT, Input::tKeyboard::cButtonR );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_ACCELERATE, Input::tKeyboard::cButtonW );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_DECCELERATE, Input::tKeyboard::cButtonS );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_BOOST, Input::tKeyboard::cButtonLShift );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_HANDBRAKE, Input::tKeyboard::cButtonSpace );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_DECREASE_ALTITUDE, Input::tKeyboard::cButtonSpace );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_INCREASE_ALTITUDE, Input::tKeyboard::cButtonLShift );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY, Input::tKeyboard::cButtonRTrigger );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_TRIGGER_SECONDARY, Input::tKeyboard::cButtonLTrigger );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_FORWARD, Input::tKeyboard::cButtonNumPad8 );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_BEHIND, Input::tKeyboard::cButtonNumPad2 );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_LEFT, Input::tKeyboard::cButtonNumPad4 );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_RIGHT, Input::tKeyboard::cButtonNumPad6 );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_QUICK_USE, Input::tKeyboard::cButton1 );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_QUICK_SELL, Input::tKeyboard::cButton2 );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_QUICK_UPGRADE, Input::tKeyboard::cButton3 );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_QUICK_REPAIR, Input::tKeyboard::cButton4 );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_LEFT, Input::tKeyboard::cButtonA );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_RIGHT, Input::tKeyboard::cButtonD );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_UP, Input::tKeyboard::cButtonW );
			//fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_DOWN, Input::tKeyboard::cButtonS );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_MINMAG, Input::tKeyboard::cButtonLThumbMinMag );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_MOVE_MAXMAG, Input::tKeyboard::cButtonLThumbMaxMag );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_MINMAG, Input::tKeyboard::cButtonRThumbMinMag );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_MAXMAG, Input::tKeyboard::cButtonRThumbMaxMag );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_LEFT, Input::tKeyboard::cButtonRStickLeft );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_RIGHT, Input::tKeyboard::cButtonRStickRight );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_UP, Input::tKeyboard::cButtonRStickUp );
			////fSetKeyboardBinding( controlProfile, GameFlags::cGAME_CONTROLS_AIM_DOWN, Input::tKeyboard::cButtonRStickDown );

			//fSetMouseBinding( controlProfile, GameFlags::cGAME_CONTROLS_SELECT, Input::tMouse::cButtonLeft );
			//fSetMouseBinding( controlProfile, GameFlags::cGAME_CONTROLS_PLACEMENT_MENU, Input::tMouse::cButtonRight );
			//fSetMouseBinding( controlProfile, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY, Input::tMouse::cButtonLeft );
			//fSetMouseBinding( controlProfile, GameFlags::cGAME_CONTROLS_TRIGGER_SECONDARY, Input::tMouse::cButtonRight );
		}
	}

	void tGameController::fSetGamepadBinding( u32 controlProfile, u32 gameControlsFlag, Input::tGamepad::tButton button )
	{
		sigassert( ( controlProfile >= tUserProfile::cProfileBegin ) && ( controlProfile < tUserProfile::cProfileEnd ) );
		sigassert( ( gameControlsFlag < GameFlags::cGAME_CONTROLS_COUNT ) );
		if( ( ( controlProfile < tUserProfile::cProfileBegin ) || ( controlProfile >= tUserProfile::cProfileEnd ) ) || ( gameControlsFlag >= GameFlags::cGAME_CONTROLS_COUNT ) )
		{
			return;
		}

		mBindings[ controlProfile - tUserProfile::cProfileBegin ].mGamepadBindings[ gameControlsFlag ] = button;
	}

	void tGameController::fSetKeyboardBinding( u32 controlProfile, u32 gameControlsFlag, Input::tKeyboard::tButton button )
	{
		sigassert( ( controlProfile >= tUserProfile::cProfileBegin ) && ( controlProfile < tUserProfile::cProfileEnd ) );
		sigassert( ( gameControlsFlag < GameFlags::cGAME_CONTROLS_COUNT ) );
		if( ( ( controlProfile < tUserProfile::cProfileBegin ) || ( controlProfile >= tUserProfile::cProfileEnd ) ) || ( gameControlsFlag >= GameFlags::cGAME_CONTROLS_COUNT ) )
		{
			return;
		}

		mBindings[ controlProfile - tUserProfile::cProfileBegin ].mKeyboardBindings[ gameControlsFlag ] = button;
	}

	void tGameController::fSetMouseBinding( u32 controlProfile, u32 gameControlsFlag, Input::tMouse::tButton button )
	{
		sigassert( ( controlProfile >= tUserProfile::cProfileBegin ) && ( controlProfile < tUserProfile::cProfileEnd ) );
		sigassert( ( gameControlsFlag < GameFlags::cGAME_CONTROLS_COUNT ) );
		if( ( ( controlProfile < tUserProfile::cProfileBegin ) || ( controlProfile >= tUserProfile::cProfileEnd ) ) || ( gameControlsFlag >= GameFlags::cGAME_CONTROLS_COUNT ) )
		{
			return;
		}

		mBindings[ controlProfile - tUserProfile::cProfileBegin ].mMouseBindings[ gameControlsFlag ] = button;
	}


	Input::tGamepad::tButton tGameController::fGetGamepadBinding( u32 controlProfile, u32 gameControlsFlag ) const
	{
		sigassert( ( controlProfile >= tUserProfile::cProfileBegin ) && ( controlProfile < tUserProfile::cProfileEnd ) );
		sigassert( ( gameControlsFlag < GameFlags::cGAME_CONTROLS_COUNT ) );
		if( ( ( controlProfile < tUserProfile::cProfileBegin ) || ( controlProfile >= tUserProfile::cProfileEnd ) ) || ( gameControlsFlag >= GameFlags::cGAME_CONTROLS_COUNT ) )
		{
			return 0;
		}

		return mBindings[ controlProfile - tUserProfile::cProfileBegin ].mGamepadBindings[ gameControlsFlag ];
	}

    Input::tKeyboard::tButton tGameController::fGetKeyboardBinding( u32 controlProfile, u32 gameControlsFlag ) const
	{
		sigassert( ( controlProfile >= tUserProfile::cProfileBegin ) && ( controlProfile < tUserProfile::cProfileEnd ) );
		sigassert( ( gameControlsFlag < GameFlags::cGAME_CONTROLS_COUNT ) );
		if( ( ( controlProfile < tUserProfile::cProfileBegin ) || ( controlProfile >= tUserProfile::cProfileEnd ) ) || ( gameControlsFlag >= GameFlags::cGAME_CONTROLS_COUNT ) )
		{
			return 0;
		}

		return mBindings[ controlProfile - tUserProfile::cProfileBegin ].mKeyboardBindings[ gameControlsFlag ];
	}

    Input::tMouse::tButton tGameController::fGetMouseBinding( u32 controlProfile, u32 gameControlsFlag ) const
	{
		sigassert( ( controlProfile >= tUserProfile::cProfileBegin ) && ( controlProfile < tUserProfile::cProfileEnd ) );
		sigassert( ( gameControlsFlag < GameFlags::cGAME_CONTROLS_COUNT ) );
		if( ( ( controlProfile < tUserProfile::cProfileBegin ) || ( controlProfile >= tUserProfile::cProfileEnd ) ) || ( gameControlsFlag >= GameFlags::cGAME_CONTROLS_COUNT ) )
		{
			return Input::tMouse::cButtonCount;
		}

		return mBindings[ controlProfile - tUserProfile::cProfileBegin ].mMouseBindings[ gameControlsFlag ];
	}

	b32 tGameController::fGamepadConnected() const
	{
		if( !mUser )
			return false;

		const Input::tGamepad& gp = mUser->fRawGamepad( );
		return gp.fConnected( );
	}

	void tGameController::fDisableMouseCursorAutoRestrict( b32 restrictDisabled )
	{
		if( restrictDisabled == false && mDisableMouseCursorAutoRestrict != restrictDisabled )
		{
			const Input::tMouse &mouse = mUser->fRawMouse( );
			const Input::tMouse::tStateData& mouseState = mouse.fGetState( );

			Input::tMouse::sRestrictCursorPosX = mouseState.mCursorPosX;
			Input::tMouse::sRestrictCursorPosY = mouseState.mCursorPosY;
		}

		mDisableMouseCursorAutoRestrict = restrictDisabled;
	}

	void tGameController::fPushMouseCursorRestrictFilterLevel( u32 filterLevel )
	{
		mMouseCursorRestrictInputFilterStack.fPushBack( filterLevel );
	}

	void tGameController::fPopMouseCursorRestrictFilterLevel( u32 filterLevel )
	{
		b32 found = mMouseCursorRestrictInputFilterStack.fFindAndEraseOrdered( filterLevel );
		sigassert( found );
	}

	u32 tGameController::fMouseCursorRestrictFilterLevel( )
	{
		return mMouseCursorRestrictInputFilterStack.fBack( );
	}

	Math::tVec2f tGameController::fUIMousePos( u32 inputFilter ) const
	{
		if( !mUser )
		{
			return Math::tVec2f::cZeroVector;
		}

		Math::tVec2f mousePosition = mUser->fFilteredMouse( inputFilter ).fPosition( );
		Math::tVec2f mouseCoordRange = mUser->fFilteredMouse( inputFilter ).fCoordRange( );

		// calculate the gui rect because we are letterboxed/pillarboxed 16:9 720p on non 16:9 displays.
		Math::tRect guiRect;

		f32 ratio = 16.f / 9.f;
		if( mouseCoordRange.y * ratio < mouseCoordRange.x )
		{
			guiRect.mL = ( f32 )( ( s32 )( ( mouseCoordRange.x - ( mouseCoordRange.y * ratio ) ) * 0.5f ) );
			guiRect.mR = ( f32 )( ( s32 )( guiRect.mL + ( mouseCoordRange.y * ratio ) ) );
			guiRect.mT = 0.0f;
			guiRect.mB = ( f32 )mouseCoordRange.y;
		}
		else
		{
			guiRect.mL = 0.0f;
			guiRect.mR = ( f32 )mouseCoordRange.x;
			guiRect.mT = ( f32 )( ( s32 )( ( mouseCoordRange.y - ( mouseCoordRange.x / ratio ) ) * 0.5f ) );
			guiRect.mB = ( f32 )( ( s32 )( guiRect.mT + ( mouseCoordRange.x / ratio ) ) );
		}

		// convert display size mouse coords to gui rect mouse coords.
		mousePosition.x = ( /*fClamp(*/ mousePosition.x/*, guiRect.mL, guiRect.mR )*/ - guiRect.mL ) * ( 1280.0f / guiRect.fWidth( ) );
		mousePosition.y = ( /*fClamp(*/ mousePosition.y/*, guiRect.mT, guiRect.mB )*/ - guiRect.mT ) * ( 720.0f / guiRect.fHeight( ) );

		//log_line( 0, "Gui Mouse Position: " << mousePosition );

		return mousePosition;
	}

	b32 tGameController::fUIMouseClicked( u32 button, u32 inputFilter ) const
	{
		if( !mUser )
		{
			return false;
		}

		return mUser->fFilteredMouse( inputFilter ).fButtonUp( button );
	}

	Math::tVec2f tGameController::fUIMouseDelta( u32 inputFilter ) const
	{
		if( !mUser )
		{
			return Math::tVec2f::cZeroVector;
		}

		return mUser->fFilteredMouse( inputFilter ).fDeltaPosition( );
	}

	b32 tGameController::fUIMouseHeld( u32 button, u32 inputFilter ) const
	{
		if( !mUser )
		{
			return false;
		}

		return mUser->fFilteredMouse( inputFilter ).fButtonHeld( button );
	}

	s32 tGameController::fUIMouseWheelDelta( u32 inputFilter ) const
	{
#if defined( platform_pcdx )
		return ( s32 )( mMouseWheelDeltaAccumulated / WHEEL_DELTA ) * -1;
#else
		return 0.0f;
#endif
	}

	void tGameController::fUISetMouseCursorPos( const Math::tVec2f &uiCoordsCursorPos )
	{
		if( !mUser )
		{
			return;
		}

		Math::tVec2f mouseCoordRange = mUser->fRawMouse( ).fCoordRange( );

		// calculate the gui rect because we are letterboxed/pillarboxed 16:9 720p on non 16:9 displays.
		Math::tRect guiRect;

		f32 ratio = 16.f / 9.f;
		if( mouseCoordRange.y * ratio < mouseCoordRange.x )
		{
			guiRect.mL = ( f32 )( ( s32 )( ( mouseCoordRange.x - ( mouseCoordRange.y * ratio ) ) * 0.5f ) );
			guiRect.mR = ( f32 )( ( s32 )( guiRect.mL + ( mouseCoordRange.y * ratio ) ) );
			guiRect.mT = 0.0f;
			guiRect.mB = ( f32 )mouseCoordRange.y;
		}
		else
		{
			guiRect.mL = 0.0f;
			guiRect.mR = ( f32 )mouseCoordRange.x;
			guiRect.mT = ( f32 )( ( s32 )( ( mouseCoordRange.y - ( mouseCoordRange.x / ratio ) ) * 0.5f ) );
			guiRect.mB = ( f32 )( ( s32 )( guiRect.mT + ( mouseCoordRange.x / ratio ) ) );
		}

		// convert from gui coords to display size coords
		Math::tVec2f cursorPos;
		//cursorPos.x = ( /*fClamp(*/ uiCoordsCursorPos.x/*, guiRect.mL, guiRect.mR )*/ - guiRect.mL ) * ( 1280.0f / guiRect.fWidth( ) );
		//cursorPos.y = ( /*fClamp(*/ uiCoordsCursorPos.y/*, guiRect.mT, guiRect.mB )*/ - guiRect.mT ) * ( 720.0f / guiRect.fHeight( ) );
		cursorPos.x = guiRect.mL + ( uiCoordsCursorPos.x * ( guiRect.fWidth( ) / 1280.0f ) );
		cursorPos.y = guiRect.mT + ( uiCoordsCursorPos.y * ( guiRect.fHeight( ) / 720.0f ) );

		fSetMouseCursorPos( cursorPos );
	}

	tGameController* tGameController::fNullGameControllerFromScript( )
	{
		return (tGameController *)&cNullGameController;
	}

	//------------------------------------------------------------------------------
	const tLocalizedString& tGameController::fGetGameControlsLocString( u32 controlProfile, u32 gameControl ) const
	{
		std::string gameControlLocName = "KeyBinding_";
		gameControlLocName.append( tUserProfile::fControlProfileEnumToString( controlProfile ).fCStr( ) );
		gameControlLocName.append( "_" );
		gameControlLocName.append( GameFlags::fGAME_CONTROLSEnumToString( gameControl ).fCStr( ) );
		tStringPtr gameControlStringId( gameControlLocName );

		if( tGameApp::fInstance( ).fLocStringExists( gameControlStringId ) )
		{
			return tGameApp::fInstance( ).fLocString( gameControlStringId );
		}

		return tLocalizedString::fEmptyString( );
	}

	//------------------------------------------------------------------------------
	const tLocalizedString& tGameController::fGetGameControlsKeyboardBinding( u32 controlProfile, u32 gameControl )
	{
#if defined( platform_pcdx )
		Input::tKeyboard::tButton vkCode = fGetKeyboardBinding( controlProfile, gameControl );

		return fKeyboardtButtonToAscii( vkCode );
#else
		return tLocalizedString::fEmptyString( );
#endif
	}

	//------------------------------------------------------------------------------
	const tLocalizedString& tGameController::fKeyboardtButtonToAscii( Input::tKeyboard::tButton button )
	{
#if defined( platform_pcdx )
		if( button == 0 )
		{
			return tLocalizedString::fEmptyString( );
		}

		// Convert the VK to a scancode
		uint32 scanCode = MapVirtualKey( button, 0 /* MAPVK_VK_TO_VSC */ );

		// because MapVirtualKey strips the extended bit for some keys
		switch( button )
		{
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN: // arrow keys
		case VK_PRIOR:
		case VK_NEXT: // page up and page down
		case VK_END:
		case VK_HOME:
		case VK_INSERT:
		case VK_DELETE:
		case VK_DIVIDE: // numpad slash
		case VK_NUMLOCK:
		case VK_RCONTROL:
		case VK_RMENU:
			scanCode |= 0x100; // set extended bit
			break;
		}

		WCHAR text[64];
		if( GetKeyNameTextW( scanCode << 16, text, 64 ) != 0 )
		{
			mControlBindingLocalizedString.fFromCStr( text );
			return mControlBindingLocalizedString;
		}
#endif

		return tLocalizedString::fEmptyString( );
	}

	//------------------------------------------------------------------------------
	const tLocalizedString& tGameController::fGetGameControlsMouseBinding( u32 controlProfile, u32 gameControl )
	{
#if defined( platform_pcdx )
		Input::tMouse::tButton vkCodeMouse = fGetMouseBinding( controlProfile, gameControl );

		if( vkCodeMouse == Input::tMouse::cButtonCount )
		{
			return tLocalizedString::fEmptyString( );
		}

		switch( vkCodeMouse )
		{
		case Input::tMouse::cButtonLeft:
			{
				return tGameApp::fInstance( ).fLocString( tStringPtr( "KeyBinding_Mouse_LMB" ) );
			}
			break;

		case Input::tMouse::cButtonMiddle:
			{
				return tGameApp::fInstance( ).fLocString( tStringPtr( "KeyBinding_Mouse_MMB" ) );
			}
			break;

		case Input::tMouse::cButtonRight:
			{
				return tGameApp::fInstance( ).fLocString( tStringPtr( "KeyBinding_Mouse_RMB" ) );
			}
			break;
		}
#endif

		return tLocalizedString::fEmptyString( );
	}

    //------------------------------------------------------------------------------
	b32 tGameController::fCaptureNextKeyPress( )
	{
#if defined( platform_pcdx )
		mCaptureNextKeyPress = true;
		mCaptureNextKeyPressResult = 0;
#endif
		return true;
	}

    //------------------------------------------------------------------------------
	b32 tGameController::fCancelCaptureNextKeyPress( )
	{
#if defined( platform_pcdx )
		mCaptureNextKeyPress = false;
		mCaptureNextKeyPressResult = 0;
#endif
		return true;
	}

    //------------------------------------------------------------------------------
	u32 tGameController::fGetKeyPressCaptured( )
	{
		return mCaptureNextKeyPressResult;
	}

    //------------------------------------------------------------------------------
	b32 tGameController::fLoadDefaultGameControlsBindings( )
	{
		tResourcePtr keybindingsXml = tApplication::fInstance( ).fResourceDepot( )->fQueryLoadBlock( tResourceId::fMake<tXmlFile>( cKeyBindingsFileName ), this );

		// Load default keybindings
        tXmlFile* file = keybindingsXml->fCast<tXmlFile>( );
        fLoadKeyBindingsFromFile( file );

		keybindingsXml->fUnload( this );

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameController::fRevertGameControlsBindingsChanges( )
	{
#if defined( platform_pcdx9 )

		// Load user set keybindings
		if( !tGameApp::cLocalAppData.fNull( ) )
		{
			tXmlDeserializer des;
			des.fLoad( tFilePathPtr::fConstructPath( tGameApp::cLocalAppData, cKeyBindingsFileName ), cKeyBindingsXmlRoot.fCStr( ), *this );
		}
#endif

		return true;
	}


	//------------------------------------------------------------------------------
	b32 tGameController::fCanUseGamePad( ) const
	{
		return tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) || tGameApp::fInstance( ).fIsDisplayCase( ) || mMode == GamePad;
	}

	//------------------------------------------------------------------------------
	b32 tGameController::fCanUseKeyboardMouse( ) const
	{
		return tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) || tGameApp::fInstance( ).fIsDisplayCase( ) || mMode == KeyboardMouse;
	}


	//------------------------------------------------------------------------------
	void tGameController::fSetMouseCursorPos( const Math::tVec2f &cursorPos )
	{
		if( mUser->fIsLocal( ) )
		{
			const_cast<Input::tMouse &>( mUser->fRawMouse( ) ).fSetPosition( ( u32 )cursorPos.x, ( u32 )cursorPos.y );
		}
	}

	f32 tGameController::fMouseSensitivity( u32 controlProfile ) const
	{
		if( controlProfile == tUserProfile::cProfileShellCam )
		{
			return mMouseSensitivity * 0.2f;
		}

		return mMouseSensitivity;
	}

	void tGameController::fSetMouseSensitivity( u32 controlProfile, f32 sensitivity )
	{
		mMouseSensitivity = sensitivity;
	}

	b32 tGameController::fMouseInverted( u32 controlProfile ) const
	{
		return ( mMouseInverted & ( 1 << controlProfile ) ) != 0;
	}

	void tGameController::fSetMouseInverted( u32 controlProfile, b32 inverted )
	{
		if( inverted )
		{
			mMouseInverted |= ( 1 << controlProfile );
		}
		else
		{
			mMouseInverted &= ~( 1 << controlProfile );
		}
	}

	void tGameController::fExportScriptInterface( tScriptVm& vm )
	{
        Sqrat::Class< tGameController, Sqrat::NoConstructor > classDesc( vm.fSq( ) );

        classDesc
			.Overload< b32 ( tGameController::* )( u32, u32 )		const >( _SC( "ButtonHeld" ),	&tGameController::fButtonHeld )
			.Overload< b32 ( tGameController::* )( u32, u32, u32 )	const >( _SC( "ButtonHeld" ),	&tGameController::fButtonHeld )
			.Overload< b32 ( tGameController::* )( u32, u32 )		const >( _SC( "ButtonDown" ),	&tGameController::fButtonDown )
			.Overload< b32 ( tGameController::* )( u32, u32, u32 )	const >( _SC( "ButtonDown" ),	&tGameController::fButtonDown )
			.Overload< b32 ( tGameController::* )( u32, u32 )		const >( _SC( "ButtonUp" ),		&tGameController::fButtonUp )
			.Overload< b32 ( tGameController::* )( u32, u32, u32 )	const >( _SC( "ButtonUp" ),		&tGameController::fButtonUp )
			.Overload< Math::tVec2f ( tGameController::* )( u32 )		const >( _SC( "MoveStick" ),	&tGameController::fMoveStick )
			.Overload< Math::tVec2f ( tGameController::* )( u32, u32 )	const >( _SC( "MoveStick" ),	&tGameController::fMoveStick )
			.Overload< Math::tVec2f ( tGameController::* )( u32 )		const >( _SC( "AimStick" ),		&tGameController::fAimStick )
			.Overload< Math::tVec2f ( tGameController::* )( u32, u32 )	const >( _SC( "AimStick" ),		&tGameController::fAimStick )
			.Overload< Math::tVec2f ( tGameController::* )( )			const >( _SC( "MenuStick" ),	&tGameController::fMenuStick )
			.Overload< Math::tVec2f ( tGameController::* )( u32 )		const >( _SC( "MenuStick" ),	&tGameController::fMenuStick )
			.StaticFunc( _SC( "NullGameController" ),		&tGameController::fNullGameControllerFromScript )
 			.Func( _SC( "GameControlsLocString" ),			&tGameController::fGetGameControlsLocString )
 			.Func( _SC( "GameControlsKeyboardBinding" ),	&tGameController::fGetGameControlsKeyboardBinding )
 			.Func( _SC( "KeyboardtButtonToAscii" ),			&tGameController::fKeyboardtButtonToAscii )
 			.Func( _SC( "GameControlsMouseBinding" ),		&tGameController::fGetGameControlsMouseBinding )
			.Func( _SC( "CaptureNextKeyPress" ),			&tGameController::fCaptureNextKeyPress )
			.Func( _SC( "CancelCaptureNextKeyPress" ),		&tGameController::fCancelCaptureNextKeyPress )
			.Func( _SC( "GetKeyPressCaptured" ),			&tGameController::fGetKeyPressCaptured )
			.Func( _SC( "LoadDefaultGameControlsBindings" ),	&tGameController::fLoadDefaultGameControlsBindings )
			.Func( _SC( "RevertGameControlsBindingsChanges" ),	&tGameController::fRevertGameControlsBindingsChanges )
			.Func( _SC( "SetKeyboardBinding" ),				&tGameController::fSetKeyboardBinding )
			.Func( _SC( "SaveKeyBindingsLocalAppData" ),	&tGameController::fSaveKeyBindingsLocalAppData )
			.Func( _SC( "UIMousePos" ),						&tGameController::fUIMousePos )
			.Func( _SC( "UIMouseClicked" ),					&tGameController::fUIMouseClicked )
			.Func( _SC( "UIMouseDelta" ),					&tGameController::fUIMouseDelta )
			.Func( _SC( "UIMouseHeld" ),					&tGameController::fUIMouseHeld )
			.Func( _SC( "UIMouseWheelDelta" ),				&tGameController::fUIMouseWheelDelta )
			.Func( _SC( "UISetMouseCursorPos" ),			&tGameController::fUISetMouseCursorPos )
			.Func( _SC( "GamepadConnected" ),				&tGameController::fGamepadConnected )
			.Func( _SC( "GetKeyboardBinding" ),				&tGameController::fGetKeyboardBinding )
			.Func( _SC( "GetMouseBinding" ),				&tGameController::fGetMouseBinding )
			.Func( _SC( "GetGamepadBinding" ),				&tGameController::fGetGamepadBinding )
			.Func( _SC( "SetMode" ),						&tGameController::fSetMode )
			.Func( _SC( "GetMode" ),						&tGameController::fMode )
			.Func( _SC( "MouseSensitivity" ),				&tGameController::fMouseSensitivity )
			.Func( _SC( "SetMouseSensitivity" ),			&tGameController::fSetMouseSensitivity )
			.Func( _SC( "MouseInverted" ),					&tGameController::fMouseInverted )
			.Func( _SC( "SetMouseInverted" ),				&tGameController::fSetMouseInverted )
			;

        vm.fRootTable( ).Bind( _SC( "GameController" ), classDesc );

				vm.fConstTable( ).Const( _SC( "GAMECONTROLLER_MODE_KEYBOARDMOUSE" ), ( int )KeyboardMouse );
				vm.fConstTable( ).Const( _SC( "GAMECONTROLLER_MODE_GAMEPAD" ), ( int )GamePad );


	}



} // namespace Sig
