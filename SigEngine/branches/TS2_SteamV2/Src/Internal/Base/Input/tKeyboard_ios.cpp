#include "BasePch.hpp"
#if defined( platform_ios )
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
			// TODOIOS need to figure out how to do keyboard input
			if( false )//if( GetAsyncKeyState( i ) & 0x8000 )
			{
				stateData.mKeysOn[i] = 1;
				stateData.mFramesHeld[i] = ( u8 )fMin<u32>( 255, stateData.mFramesHeld[i] + 1 ); // don't wrap to zero, we want to clamp at 255
			}
			else
			{
				stateData.mKeysOn[i] = 0;
				stateData.mFramesHeld[i] = 0;
			}
		}

	}

	void tKeyboard::fStartup( )
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

	const tKeyboard::tButton tKeyboard::cButtonUp 			= 0; //VK_UP;				/* UpArrow on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonLeft			= 0; //VK_LEFT;				/* LeftArrow on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonRight		= 0; //VK_RIGHT;			/* RightArrow on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonDown			= 0; //VK_DOWN;				/* DownArrow on arrow keypad */

	const tKeyboard::tButton tKeyboard::cButtonEscape		= 0; //VK_ESCAPE;
	const tKeyboard::tButton tKeyboard::cButtonMinus		= 0; //VK_OEM_MINUS;			/* -_ on main keyboard */
	const tKeyboard::tButton tKeyboard::cButtonEquals		= 0; //VK_OEM_PLUS;			/* =+ on main keyboard */
	const tKeyboard::tButton tKeyboard::cButtonBackspace	= 0; //VK_BACK;				/* backspace */
	const tKeyboard::tButton tKeyboard::cButtonTab			= 0; //VK_TAB;
	const tKeyboard::tButton tKeyboard::cButtonLBracket		= 0; //VK_OEM_4;
	const tKeyboard::tButton tKeyboard::cButtonRBracket		= 0; //VK_OEM_6;
	const tKeyboard::tButton tKeyboard::cButtonEnter		= 0; //VK_RETURN;			/* Enter on main keyboard */
	const tKeyboard::tButton tKeyboard::cButtonLCtrl		= 0; //VK_LCONTROL;
	const tKeyboard::tButton tKeyboard::cButtonRCtrl		= 0; //VK_RCONTROL;
	const tKeyboard::tButton tKeyboard::cButtonLAlt			= 0; //VK_LMENU;			/* left Alt */
	const tKeyboard::tButton tKeyboard::cButtonRAlt 		= 0; //VK_RMENU;			/* right Alt */
	const tKeyboard::tButton tKeyboard::cButtonLShift		= 0; //VK_LSHIFT;
	const tKeyboard::tButton tKeyboard::cButtonRShift		= 0; //VK_RSHIFT;
	const tKeyboard::tButton tKeyboard::cButtonSpace		= 0; //VK_SPACE;
	const tKeyboard::tButton tKeyboard::cButtonCapsLock		= 0; //VK_CAPITAL;
	const tKeyboard::tButton tKeyboard::cButtonNumLock		= 0; //VK_NUMLOCK;
	const tKeyboard::tButton tKeyboard::cButtonScrollLock	= 0; //VK_SCROLL;			/* Scroll Lock */
	const tKeyboard::tButton tKeyboard::cButtonHome			= 0; //VK_HOME;				/* Home on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonPrior		= 0; //VK_PRIOR;			/* PgUp on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonEnd 			= 0; //VK_END;				/* End on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonNext 		= 0; //VK_NEXT;				/* PgDn on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonInsert		= 0; //VK_INSERT;			/* Insert on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonDelete		= 0; //VK_DELETE;			/* Delete on arrow keypad */
	const tKeyboard::tButton tKeyboard::cButtonLWin			= ~0; //VK_LWIN;				/* Left Windows key */
	const tKeyboard::tButton tKeyboard::cButtonRWin			= ~0; //VK_RWIN;				/* Right Windows key */
	const tKeyboard::tButton tKeyboard::cButtonPause		= 0; //VK_PAUSE;				/* Pause/Break key */
	const tKeyboard::tButton tKeyboard::cButtonComma		= 0; //VK_OEM_COMMA;
	const tKeyboard::tButton tKeyboard::cButtonPeriod		= 0; //VK_OEM_PERIOD;
	const tKeyboard::tButton tKeyboard::cButtonSemiColon	= 0; //VK_OEM_1;
	const tKeyboard::tButton tKeyboard::cButtonQuote		= 0; //VK_OEM_7;
	const tKeyboard::tButton tKeyboard::cButtonBackslash	= 0; //VK_OEM_5;
	const tKeyboard::tButton tKeyboard::cButtonTilde		= 0; //VK_OEM_3;
	const tKeyboard::tButton tKeyboard::cButtonQuestionMark	= 0; //VK_OEM_2;


	const tKeyboard::tButton tKeyboard::cButtonNumPad0		= 0; //VK_NUMPAD0;
	const tKeyboard::tButton tKeyboard::cButtonNumPad1		= 0; //VK_NUMPAD1;
	const tKeyboard::tButton tKeyboard::cButtonNumPad2		= 0; //VK_NUMPAD2;
	const tKeyboard::tButton tKeyboard::cButtonNumPad3		= 0; //VK_NUMPAD3;
	const tKeyboard::tButton tKeyboard::cButtonNumPad4		= 0; //VK_NUMPAD4;
	const tKeyboard::tButton tKeyboard::cButtonNumPad5		= 0; //VK_NUMPAD5;
	const tKeyboard::tButton tKeyboard::cButtonNumPad6		= 0; //VK_NUMPAD6;
	const tKeyboard::tButton tKeyboard::cButtonNumPad7		= 0; //VK_NUMPAD7;
	const tKeyboard::tButton tKeyboard::cButtonNumPad8		= 0; //VK_NUMPAD8;
	const tKeyboard::tButton tKeyboard::cButtonNumPad9		= 0; //VK_NUMPAD9;
	const tKeyboard::tButton tKeyboard::cButtonNumPadDec	= 0; //VK_DECIMAL;			/* . on numeric keypad */
	const tKeyboard::tButton tKeyboard::cButtonNumPadSub	= 0; //VK_SUBTRACT;			/* - on numeric keypad */
	const tKeyboard::tButton tKeyboard::cButtonNumPadAdd	= 0; //VK_ADD;				/* + on numeric keypad */
	const tKeyboard::tButton tKeyboard::cButtonNumPadMul	= 0; //VK_MULTIPLY;			/* * on numeric keypad */
	const tKeyboard::tButton tKeyboard::cButtonNumPadDiv	= 0; //VK_DIVIDE;			/* / on numeric keypad */

	const tKeyboard::tButton tKeyboard::cButtonF1 			= 0; //VK_F1;
	const tKeyboard::tButton tKeyboard::cButtonF2 			= 0; //VK_F2;
	const tKeyboard::tButton tKeyboard::cButtonF3 			= 0; //VK_F3;
	const tKeyboard::tButton tKeyboard::cButtonF4 			= 0; //VK_F4;
	const tKeyboard::tButton tKeyboard::cButtonF5 			= 0; //VK_F5;
	const tKeyboard::tButton tKeyboard::cButtonF6 			= 0; //VK_F6;
	const tKeyboard::tButton tKeyboard::cButtonF7 			= 0; //VK_F7;
	const tKeyboard::tButton tKeyboard::cButtonF8 			= 0; //VK_F8;
	const tKeyboard::tButton tKeyboard::cButtonF9 			= 0; //VK_F9;
	const tKeyboard::tButton tKeyboard::cButtonF10 			= 0; //VK_F10;
	const tKeyboard::tButton tKeyboard::cButtonF11 			= 0; //VK_F11;
	const tKeyboard::tButton tKeyboard::cButtonF12 			= 0; //VK_F12;
	const tKeyboard::tButton tKeyboard::cButtonF13 			= 0; //VK_F13;
	const tKeyboard::tButton tKeyboard::cButtonF14 			= 0; //VK_F14;
	const tKeyboard::tButton tKeyboard::cButtonF15 			= 0; //VK_F15;

}}
#endif//#if defined( platform_ios )

