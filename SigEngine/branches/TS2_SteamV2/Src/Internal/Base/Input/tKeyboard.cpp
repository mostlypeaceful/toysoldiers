#include "BasePch.hpp"
#include "tKeyboard.hpp"

namespace Sig { namespace Input
{
	const tKeyboard tKeyboard::cNullKeyboard;

	namespace
	{
		struct tButtonString
		{ 
			tButtonString( ) : mButton( 0 ) { }
			tButtonString( const tStringPtr& name, tKeyboard::tButton button )
				: mName( name )
				, mButton( button )
			{
			}

			b32 operator==( const tStringPtr& name ) const { return mName == name; }
			b32 operator==( tKeyboard::tButton button ) const { return mButton == button; }

			tStringPtr mName; 
			tKeyboard::tButton mButton; 
		};

		static tGrowableArray< tButtonString > gButtonStrings;

		define_static_function( fBuildButtonStrings )
		{
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "0" ), tKeyboard::cButton0 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "1" ), tKeyboard::cButton1 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "2" ), tKeyboard::cButton2 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "3" ), tKeyboard::cButton3 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "4" ), tKeyboard::cButton4 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "5" ), tKeyboard::cButton5 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "6" ), tKeyboard::cButton6 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "7" ), tKeyboard::cButton7 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "8" ), tKeyboard::cButton8 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "9" ), tKeyboard::cButton9 ) );

			gButtonStrings.fPushBack( tButtonString( tStringPtr( "A" ), tKeyboard::cButtonA ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "B" ), tKeyboard::cButtonB ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "C" ), tKeyboard::cButtonC ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "D" ), tKeyboard::cButtonD ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "E" ), tKeyboard::cButtonE ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F" ), tKeyboard::cButtonF ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "G" ), tKeyboard::cButtonG ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "H" ), tKeyboard::cButtonH ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "I" ), tKeyboard::cButtonI ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "J" ), tKeyboard::cButtonJ ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "K" ), tKeyboard::cButtonK ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "L" ), tKeyboard::cButtonL ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "M" ), tKeyboard::cButtonM ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "N" ), tKeyboard::cButtonN ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "O" ), tKeyboard::cButtonO ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "P" ), tKeyboard::cButtonP ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "Q" ), tKeyboard::cButtonQ ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "R" ), tKeyboard::cButtonR ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "S" ), tKeyboard::cButtonS ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "T" ), tKeyboard::cButtonT ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "U" ), tKeyboard::cButtonU ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "V" ), tKeyboard::cButtonV ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "W" ), tKeyboard::cButtonW ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "X" ), tKeyboard::cButtonX ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "Y" ), tKeyboard::cButtonY ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "Z" ), tKeyboard::cButtonZ ) );

			gButtonStrings.fPushBack( tButtonString( tStringPtr( "UP" ), tKeyboard::cButtonUp ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LEFT" ), tKeyboard::cButtonLeft ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RIGHT" ), tKeyboard::cButtonRight ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "DOWN" ), tKeyboard::cButtonDown ) );
			
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "ESCAPE" ), tKeyboard::cButtonEscape ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "SUBTRACT" ), tKeyboard::cButtonMinus ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "ADD" ), tKeyboard::cButtonEquals ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "BACKSPACE" ), tKeyboard::cButtonBackspace ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "TAB" ), tKeyboard::cButtonTab ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LBRACKET" ), tKeyboard::cButtonLBracket ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RBRACKET" ), tKeyboard::cButtonRBracket ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RETURN" ), tKeyboard::cButtonEnter ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LCTRL" ), tKeyboard::cButtonLCtrl ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RCTRL" ), tKeyboard::cButtonRCtrl ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LALT" ), tKeyboard::cButtonLAlt ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RALT" ), tKeyboard::cButtonRAlt ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LSHIFT" ), tKeyboard::cButtonLShift ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RSHIFT" ), tKeyboard::cButtonRShift ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "SPACE" ), tKeyboard::cButtonSpace ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "CAPSLOCK" ), tKeyboard::cButtonCapsLock ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMLOCK" ), tKeyboard::cButtonNumLock ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "SCROLL" ), tKeyboard::cButtonScrollLock ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "HOME" ), tKeyboard::cButtonHome ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "PRIOR" ), tKeyboard::cButtonPrior ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "END" ), tKeyboard::cButtonEnd ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NEXT" ), tKeyboard::cButtonNext ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "INSERT" ), tKeyboard::cButtonInsert ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "DELETE" ), tKeyboard::cButtonDelete ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LWIN" ), tKeyboard::cButtonLWin ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RWIN" ), tKeyboard::cButtonRWin ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "PAUSE" ), tKeyboard::cButtonPause ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "COMMA" ), tKeyboard::cButtonComma ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "PERIOD" ), tKeyboard::cButtonPeriod ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "SEMICOLON" ), tKeyboard::cButtonSemiColon ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "QUOTE" ), tKeyboard::cButtonQuote ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "BACKSLASH" ), tKeyboard::cButtonBackslash ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "TILDE" ), tKeyboard::cButtonTilde ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "QUESTIONMARK" ), tKeyboard::cButtonQuestionMark ) );

			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPAD0" ), tKeyboard::cButtonNumPad0 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPAD1" ), tKeyboard::cButtonNumPad1 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPAD2" ), tKeyboard::cButtonNumPad2 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPAD3" ), tKeyboard::cButtonNumPad3 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPAD4" ), tKeyboard::cButtonNumPad4 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPAD5" ), tKeyboard::cButtonNumPad5 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPAD6" ), tKeyboard::cButtonNumPad6 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPAD7" ), tKeyboard::cButtonNumPad7 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPAD8" ), tKeyboard::cButtonNumPad8 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPAD9" ), tKeyboard::cButtonNumPad9 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPADDECIMAL" ), tKeyboard::cButtonNumPadDec ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPADSUBTRACT" ), tKeyboard::cButtonNumPadSub ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPADADD" ), tKeyboard::cButtonNumPadAdd ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPADMULTIPLY" ), tKeyboard::cButtonNumPadMul ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "NUMPADDIVIDE" ), tKeyboard::cButtonNumPadDiv ) );
			
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F1" ), tKeyboard::cButtonF1 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F2" ), tKeyboard::cButtonF2 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F3" ), tKeyboard::cButtonF3 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F4" ), tKeyboard::cButtonF4 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F5" ), tKeyboard::cButtonF5 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F6" ), tKeyboard::cButtonF6 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F7" ), tKeyboard::cButtonF7 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F8" ), tKeyboard::cButtonF8 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F9" ), tKeyboard::cButtonF9 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F10" ), tKeyboard::cButtonF10 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F11" ), tKeyboard::cButtonF11 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F12" ), tKeyboard::cButtonF12 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F13" ), tKeyboard::cButtonF13 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F14" ), tKeyboard::cButtonF14 ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "F15" ), tKeyboard::cButtonF15 ) );
		}

		static tKeyboard::tStateData gStateData;

		define_static_function( fInitStateData )
		{
			fZeroOut( gStateData );
		}
	}

	tKeyboard::tKeyboard( )
		: mWindow(0)
	{
	}

	tKeyboard::~tKeyboard( )
	{
		fShutdown( );
	}

	void tKeyboard::fCaptureGlobalState( f32 dt )
	{
		fCaptureGlobalStateUnbuffered( gStateData, dt );
	}

	void tKeyboard::fCaptureStateUnbuffered( tStateData& stateData, f32 dt )
	{
		stateData = gStateData;
	}

	void tKeyboard::fCaptureState( f32 dt )
	{
		mStateData = gStateData;
	}

	const tKeyboard::tStateData& tKeyboard::fGetStateData( ) const
	{
		return mStateData;
	}

	void tKeyboard::fPutStateData( const tStateData& data )
	{
		mStateData = data;
	}

	//u8 tKeyboard::fNumFramesHeld( tButton button ) const
	//{
	//	return mStateData.mFramesHeld[ button ];
	//}

	b32 tKeyboard::fButtonHeld( tButton button ) const
	{
		return mStateData.mKeysOn.fGetBit( button );
	}

	b32 tKeyboard::fButtonDown( tButton button ) const
	{
		return mStateData.mKeysOn.fGetBit( button ) && !mStateData.mPrevKeysOn.fGetBit( button );
	}

	b32 tKeyboard::fButtonUp( tButton button ) const
	{
		return !mStateData.mKeysOn.fGetBit( button ) && mStateData.mPrevKeysOn.fGetBit( button );
	}

	tKeyboard::tButton tKeyboard::fStringToButton( const tStringPtr& name )
	{
		if( const tButtonString* map = gButtonStrings.fFind( name ) )
			return map->mButton;
		return 0;
	}

	const tStringPtr& tKeyboard::fButtonToString( tButton button )
	{
		if( const tButtonString* map = gButtonStrings.fFind( button ) )
			return map->mName;
		return tStringPtr::cNullPtr;
	}

	b32 tKeyboard::fActive( ) const
	{
		u32 supportedKeyCount = gButtonStrings.fCount( );
		for( u32 buttonIndex = 0; buttonIndex < supportedKeyCount; ++buttonIndex )
		{
			if( mStateData.mKeysOn.fGetBit( gButtonStrings[ buttonIndex ].mButton ) )
			{
				return true;
			}
		}

		return false;
	}


	void tKeyboard::fExportScriptInterface( tScriptVm& vm )
	{
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_0" ), ( int )cButton0 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_1" ), ( int )cButton1 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_2" ), ( int )cButton2 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_3" ), ( int )cButton3 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_4" ), ( int )cButton4 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_5" ), ( int )cButton5 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_6" ), ( int )cButton6 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_7" ), ( int )cButton7 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_8" ), ( int )cButton8 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_9" ), ( int )cButton9 );

		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_A" ), ( int )cButtonA );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_B" ), ( int )cButtonB );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_C" ), ( int )cButtonC );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_D" ), ( int )cButtonD );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_E" ), ( int )cButtonE );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F" ), ( int )cButtonF );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_G" ), ( int )cButtonG );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_H" ), ( int )cButtonH );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_I" ), ( int )cButtonI );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_J" ), ( int )cButtonJ );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_K" ), ( int )cButtonK );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_L" ), ( int )cButtonL );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_M" ), ( int )cButtonM );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_N" ), ( int )cButtonN );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_O" ), ( int )cButtonO );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_P" ), ( int )cButtonP );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_Q" ), ( int )cButtonQ );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_R" ), ( int )cButtonR );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_S" ), ( int )cButtonS );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_T" ), ( int )cButtonT );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_U" ), ( int )cButtonU );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_V" ), ( int )cButtonV );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_W" ), ( int )cButtonW );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_X" ), ( int )cButtonX );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_Y" ), ( int )cButtonY );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_Z" ), ( int )cButtonZ );

		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_UP" ), ( int )cButtonUp );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_DOWN" ), ( int )cButtonDown );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_LEFT" ), ( int )cButtonLeft );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_RIGHT" ), ( int )cButtonRight );

		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_ESCAPE" ), ( int )cButtonEscape );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_MINUS" ), ( int )cButtonMinus );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_EQUALS" ), ( int )cButtonEquals );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_BACKSPACE" ), ( int )cButtonBackspace );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_TAB" ), ( int )cButtonTab );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_LBRACKET" ), ( int )cButtonLBracket );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_RBRACKET" ), ( int )cButtonRBracket );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_ENTER" ), ( int )cButtonEnter );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_LCTRL" ), ( int )cButtonLCtrl );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_RCTRL" ), ( int )cButtonRCtrl );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_LALT" ), ( int )cButtonLAlt );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_RALT" ), ( int )cButtonRAlt );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_LSHIFT" ), ( int )cButtonLShift );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_RSHIFT" ), ( int )cButtonRShift );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_SPACE" ), ( int )cButtonSpace );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_CAPSLOCK" ), ( int )cButtonCapsLock );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMLOCK" ), ( int )cButtonNumLock );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_SCROLL" ), ( int )cButtonScrollLock );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_HOME" ), ( int )cButtonHome );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_PRIOR" ), ( int )cButtonPrior );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_END" ), ( int )cButtonEnd );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NEXT" ), ( int )cButtonNext );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_INSERT" ), ( int )cButtonInsert );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_DELETE" ), ( int )cButtonDelete );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_LWIN" ), ( int )cButtonLWin );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_RWIN" ), ( int )cButtonRWin );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_PAUSE" ), ( int )cButtonPause );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_COMMA" ), ( int )cButtonComma );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_PERIOD" ), ( int )cButtonPeriod );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_SEMICOLON" ), ( int )cButtonSemiColon );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_QUOTE" ), ( int )cButtonQuote );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_BACKSLASH" ), ( int )cButtonBackslash );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_TILDE" ), ( int )cButtonTilde );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_QUESTIONMARK" ), ( int )cButtonQuestionMark );

		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPAD0" ), ( int )cButtonNumPad0 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPAD1" ), ( int )cButtonNumPad1 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPAD2" ), ( int )cButtonNumPad2 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPAD3" ), ( int )cButtonNumPad3 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPAD4" ), ( int )cButtonNumPad4 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPAD5" ), ( int )cButtonNumPad5 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPAD6" ), ( int )cButtonNumPad6 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPAD7" ), ( int )cButtonNumPad7 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPAD8" ), ( int )cButtonNumPad8 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPAD9" ), ( int )cButtonNumPad9 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPADDECIMAL" ), ( int )cButtonNumPadDec );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPADSUBTRACT" ), ( int )cButtonNumPadSub );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPADADD" ), ( int )cButtonNumPadAdd );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPADMULTIPLY" ), ( int )cButtonNumPadMul );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_NUMPADDIVIDE" ), ( int )cButtonNumPadDiv );

		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F1" ), ( int )cButtonF1 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F2" ), ( int )cButtonF2 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F3" ), ( int )cButtonF3 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F4" ), ( int )cButtonF4 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F5" ), ( int )cButtonF5 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F6" ), ( int )cButtonF6 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F7" ), ( int )cButtonF7 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F8" ), ( int )cButtonF8 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F9" ), ( int )cButtonF9 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F10" ), ( int )cButtonF10 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F11" ), ( int )cButtonF11 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F12" ), ( int )cButtonF12 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F13" ), ( int )cButtonF13 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F14" ), ( int )cButtonF14 );
		vm.fConstTable( ).Const( _SC( "KEYBOARD_BUTTON_F15" ), ( int )cButtonF15 );
	}
}}

