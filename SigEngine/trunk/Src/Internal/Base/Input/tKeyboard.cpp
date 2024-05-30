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
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "Z" ), tKeyboard::cButtonY ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "UP" ), tKeyboard::cButtonUp ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "DOWN" ), tKeyboard::cButtonDown ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "LEFT" ), tKeyboard::cButtonLeft ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "RIGHT" ), tKeyboard::cButtonRight ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "ESCAPE" ), tKeyboard::cButtonEscape ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "SUBTRACT" ), tKeyboard::cButtonMinus ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "ADD" ), tKeyboard::cButtonEquals ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "BACKSPACE" ), tKeyboard::cButtonBackspace ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "TAB" ), tKeyboard::cButtonTab ) );
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
	}

	tKeyboard::tKeyboard( )
		: mWindow(0)
	{
		fZeroOut( mStateData );
	}

	tKeyboard::~tKeyboard( )
	{
		fShutdown( );
	}

	void tKeyboard::fCaptureState( f32 dt )
	{
		fCaptureStateUnbuffered( mStateData, dt );
	}

	const tKeyboard::tStateData& tKeyboard::fGetStateData( ) const
	{
		return mStateData;
	}

	u8 tKeyboard::fNumFramesHeld( tButton button ) const
	{
		return mStateData.mFramesHeld[button];
	}

	b32 tKeyboard::fButtonHeld( tButton button ) const
	{
		return mStateData.mKeysOn[button] != 0;
	}

	b32 tKeyboard::fButtonDown( tButton button ) const
	{
		return mStateData.mKeysOn[button] && !mStateData.mPrevKeysOn[button];
	}

	b32 tKeyboard::fButtonUp( tButton button ) const
	{
		return !mStateData.mKeysOn[button] && mStateData.mPrevKeysOn[button];
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

}}

