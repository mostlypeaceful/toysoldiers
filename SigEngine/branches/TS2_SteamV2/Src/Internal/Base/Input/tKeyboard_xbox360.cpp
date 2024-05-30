#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tKeyboard.hpp"

namespace Sig { namespace Input
{
	void tKeyboard::fCaptureGlobalStateUnbuffered( tStateData& stateData, f32 dt )
	{
		// before retrieving new state, move current state to last
		stateData.mPrevKeysOn = stateData.mKeysOn;

		// translate keys info to be purely boolean and track keys pressed info
		for( u32 i = 0; i < stateData.mKeysOn.fCount( ); ++i )
		{
			// TODOXBOX360 need to figure out how to do keyboard input
			if( false )//if( GetAsyncKeyState( i ) & 0x8000 )
			{
				stateData.mKeysOn[i] = 1;
				//stateData.mFramesHeld[i] = ( u8 )fMin<u32>( 255, stateData.mFramesHeld[i] + 1 ); // don't wrap to zero, we want to clamp at 255
			}
			else
			{
				stateData.mKeysOn[i] = 0;
				//stateData.mFramesHeld[i] = 0;
			}
		}

	}

	void tKeyboard::fStartup( tGenericWindowHandle )
	{
	}

	void tKeyboard::fShutdown( )
	{
	}

	const tKeyboard::tButton tKeyboard::cButton0 			= '0';
	const tKeyboard::tButton tKeyboard::cButton1 			= '1';
	const tKeyboard::tButton tKeyboard::cButton2 			= '2';
	const tKeyboard::tButton tKeyboard::cButton3 			= '3';
	const tKeyboard::tButton tKeyboard::cButton4 			= '4';
	const tKeyboard::tButton tKeyboard::cButton5 			= '5';
	const tKeyboard::tButton tKeyboard::cButton6 			= '6';
	const tKeyboard::tButton tKeyboard::cButton7 			= '7';
	const tKeyboard::tButton tKeyboard::cButton8 			= '8';
	const tKeyboard::tButton tKeyboard::cButton9 			= '9';

	const tKeyboard::tButton tKeyboard::cButtonA 			= 'A';
	const tKeyboard::tButton tKeyboard::cButtonB 			= 'B';
	const tKeyboard::tButton tKeyboard::cButtonC 			= 'C';
	const tKeyboard::tButton tKeyboard::cButtonD 			= 'D';
	const tKeyboard::tButton tKeyboard::cButtonE 			= 'E';
	const tKeyboard::tButton tKeyboard::cButtonF 			= 'F';
	const tKeyboard::tButton tKeyboard::cButtonG 			= 'G';
	const tKeyboard::tButton tKeyboard::cButtonH 			= 'H';
	const tKeyboard::tButton tKeyboard::cButtonI 			= 'I';
	const tKeyboard::tButton tKeyboard::cButtonJ 			= 'J';
	const tKeyboard::tButton tKeyboard::cButtonK 			= 'K';
	const tKeyboard::tButton tKeyboard::cButtonL 			= 'L';
	const tKeyboard::tButton tKeyboard::cButtonM 			= 'M';
	const tKeyboard::tButton tKeyboard::cButtonN 			= 'N';
	const tKeyboard::tButton tKeyboard::cButtonO 			= 'O';
	const tKeyboard::tButton tKeyboard::cButtonP 			= 'P';
	const tKeyboard::tButton tKeyboard::cButtonQ 			= 'Q';
	const tKeyboard::tButton tKeyboard::cButtonR 			= 'R';
	const tKeyboard::tButton tKeyboard::cButtonS 			= 'S';
	const tKeyboard::tButton tKeyboard::cButtonT 			= 'T';
	const tKeyboard::tButton tKeyboard::cButtonU 			= 'U';
	const tKeyboard::tButton tKeyboard::cButtonV 			= 'V';
	const tKeyboard::tButton tKeyboard::cButtonW 			= 'W';
	const tKeyboard::tButton tKeyboard::cButtonX 			= 'X';
	const tKeyboard::tButton tKeyboard::cButtonY 			= 'Y';
	const tKeyboard::tButton tKeyboard::cButtonZ 			= 'Z';

	const tKeyboard::tButton tKeyboard::cButtonUp 			= VK_UP;				/* UpArrow on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonLeft			= VK_LEFT;				/* LeftArrow on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonRight		= VK_RIGHT;			/* RightArrow on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonDown			= VK_DOWN;				/* DownArrow on arrow keypad */

	const tKeyboard::tButton tKeyboard::cButtonEscape		= VK_ESCAPE;
	const tKeyboard::tButton tKeyboard::cButtonMinus		= VK_OEM_MINUS;			/* -_ on main keyboard */
	const tKeyboard::tButton tKeyboard::cButtonEquals		= VK_OEM_PLUS;			/* =+ on main keyboard */
	const tKeyboard::tButton tKeyboard::cButtonBackspace	= VK_BACK;				/* backspace */
	const tKeyboard::tButton tKeyboard::cButtonTab			= VK_TAB;
	const tKeyboard::tButton tKeyboard::cButtonLBracket		= VK_OEM_4;
	const tKeyboard::tButton tKeyboard::cButtonRBracket		= VK_OEM_6;
	const tKeyboard::tButton tKeyboard::cButtonEnter		= VK_RETURN;			/* Enter on main keyboard */
	const tKeyboard::tButton tKeyboard::cButtonLCtrl		= VK_LCONTROL;
	const tKeyboard::tButton tKeyboard::cButtonRCtrl		= VK_RCONTROL;
	const tKeyboard::tButton tKeyboard::cButtonLAlt			= VK_LMENU;			/* left Alt */
	const tKeyboard::tButton tKeyboard::cButtonRAlt 		= VK_RMENU;			/* right Alt */
	const tKeyboard::tButton tKeyboard::cButtonLShift		= VK_LSHIFT;
	const tKeyboard::tButton tKeyboard::cButtonRShift		= VK_RSHIFT;
	const tKeyboard::tButton tKeyboard::cButtonSpace		= VK_SPACE;
	const tKeyboard::tButton tKeyboard::cButtonCapsLock		= VK_CAPITAL;
	const tKeyboard::tButton tKeyboard::cButtonNumLock		= VK_NUMLOCK;
	const tKeyboard::tButton tKeyboard::cButtonScrollLock	= VK_SCROLL;			/* Scroll Lock */
	const tKeyboard::tButton tKeyboard::cButtonHome			= VK_HOME;				/* Home on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonPrior		= VK_PRIOR;			/* PgUp on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonEnd 			= VK_END;				/* End on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonNext 		= VK_NEXT;				/* PgDn on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonInsert		= VK_INSERT;			/* Insert on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonDelete		= VK_DELETE;			/* Delete on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonLWin			= ~0;	//VK_LWIN;				/* Left Windows key */
	const tKeyboard::tButton tKeyboard::cButtonRWin			= ~0;	//VK_RWIN;				/* Right Windows key */
	const tKeyboard::tButton tKeyboard::cButtonPause		= VK_PAUSE;				/* Pause/Break key */
	const tKeyboard::tButton tKeyboard::cButtonComma		= VK_OEM_COMMA;
	const tKeyboard::tButton tKeyboard::cButtonPeriod		= VK_OEM_PERIOD;
	const tKeyboard::tButton tKeyboard::cButtonSemiColon	= VK_OEM_1;
	const tKeyboard::tButton tKeyboard::cButtonQuote		= VK_OEM_7;
	const tKeyboard::tButton tKeyboard::cButtonBackslash	= VK_OEM_5;
	const tKeyboard::tButton tKeyboard::cButtonTilde		= VK_OEM_3;
	const tKeyboard::tButton tKeyboard::cButtonQuestionMark	= VK_OEM_2;


	const tKeyboard::tButton tKeyboard::cButtonNumPad0		= VK_NUMPAD0;
	const tKeyboard::tButton tKeyboard::cButtonNumPad1		= VK_NUMPAD1;
	const tKeyboard::tButton tKeyboard::cButtonNumPad2		= VK_NUMPAD2;
	const tKeyboard::tButton tKeyboard::cButtonNumPad3		= VK_NUMPAD3;
	const tKeyboard::tButton tKeyboard::cButtonNumPad4		= VK_NUMPAD4;
	const tKeyboard::tButton tKeyboard::cButtonNumPad5		= VK_NUMPAD5;
	const tKeyboard::tButton tKeyboard::cButtonNumPad6		= VK_NUMPAD6;
	const tKeyboard::tButton tKeyboard::cButtonNumPad7		= VK_NUMPAD7;
	const tKeyboard::tButton tKeyboard::cButtonNumPad8		= VK_NUMPAD8;
	const tKeyboard::tButton tKeyboard::cButtonNumPad9		= VK_NUMPAD9;
	const tKeyboard::tButton tKeyboard::cButtonNumPadDec	= VK_DECIMAL;			/* . on numeric keypad */
	const tKeyboard::tButton tKeyboard::cButtonNumPadSub	= VK_SUBTRACT;			/* - on numeric keypad */
	const tKeyboard::tButton tKeyboard::cButtonNumPadAdd	= VK_ADD;				/* + on numeric keypad */
	const tKeyboard::tButton tKeyboard::cButtonNumPadMul	= VK_MULTIPLY;			/* * on numeric keypad */
	const tKeyboard::tButton tKeyboard::cButtonNumPadDiv	= VK_DIVIDE;			/* / on numeric keypad */

	const tKeyboard::tButton tKeyboard::cButtonF1 			= VK_F1;
	const tKeyboard::tButton tKeyboard::cButtonF2 			= VK_F2;
	const tKeyboard::tButton tKeyboard::cButtonF3 			= VK_F3;
	const tKeyboard::tButton tKeyboard::cButtonF4 			= VK_F4;
	const tKeyboard::tButton tKeyboard::cButtonF5 			= VK_F5;
	const tKeyboard::tButton tKeyboard::cButtonF6 			= VK_F6;
	const tKeyboard::tButton tKeyboard::cButtonF7 			= VK_F7;
	const tKeyboard::tButton tKeyboard::cButtonF8 			= VK_F8;
	const tKeyboard::tButton tKeyboard::cButtonF9 			= VK_F9;
	const tKeyboard::tButton tKeyboard::cButtonF10 			= VK_F10;
	const tKeyboard::tButton tKeyboard::cButtonF11 			= VK_F11;
	const tKeyboard::tButton tKeyboard::cButtonF12 			= VK_F12;
	const tKeyboard::tButton tKeyboard::cButtonF13 			= VK_F13;
	const tKeyboard::tButton tKeyboard::cButtonF14 			= VK_F14;
	const tKeyboard::tButton tKeyboard::cButtonF15 			= VK_F15;

}}
#endif//#if defined( platform_xbox360 )

