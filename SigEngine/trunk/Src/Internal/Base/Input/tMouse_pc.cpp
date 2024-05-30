#include "BasePch.hpp"
#if defined( platform_pcdx )
#include "tMouse.hpp"
#include "tGameAppBase.hpp"
#include "Win32Include.hpp"

#if defined( target_game ) // no fScreen( ) available on tools?
//#define RENDER_SPACE_MOUSE_COORDS // Disabled for merge: Letterboxing not (yet?) merged over.
#endif

namespace Sig { namespace Input
{
	namespace
	{
		static s32 gWheelPos = 0;
	}

	void fMouseWheelWndProc( HWND hwnd, WPARAM wParam, LPARAM lParam )
	{
		const s32 mouseDelta = GET_WHEEL_DELTA_WPARAM( wParam );
		gWheelPos += mouseDelta;
	}

	void tMouse::fSetPosition( u32 x, u32 y )
	{
		HWND hwnd = ( HWND )mWindowHandle;
		POINT pos = { x, y };
		ClientToScreen( hwnd, &pos );
		SetCursorPos( pos.x, pos.y );
	}

	void tMouse::fShowCursor( b32 show )
	{
		ShowCursor( show );
	}

	b32 tMouse::fCursorHidden( )
	{
		CURSORINFO cursorInfo;
		cursorInfo.cbSize = sizeof(CURSORINFO);
		GetCursorInfo(&cursorInfo);
		return cursorInfo.flags == 0;
	}

	void tMouse::fCaptureStateUnbuffered( tStateData& stateData, f32 dt )
	{
		HWND hwnd = ( HWND )mWindowHandle;
		POINT win32CursorPos={0};
		GetCursorPos( &win32CursorPos );
		ScreenToClient( hwnd, &win32CursorPos );
		RECT clientRect={0};
		GetClientRect( hwnd, &clientRect );
		s32 clientWidth = clientRect.right - clientRect.left;
		s32 clientHeight = clientRect.bottom - clientRect.top;
		const b32 outsideClient = (		win32CursorPos.x < 0 
									||	win32CursorPos.y < 0 
									||	win32CursorPos.x >= clientWidth
		
                                    ||	win32CursorPos.y >= clientHeight );

		if( GetForegroundWindow( ) != tApplication::fInstance( ).fGetHwnd( ) /*|| outsideClient /*|| !tApplication::fInstance( ).fHasFocus( )*/ )
		{
			fZeroOut( stateData );

			// Mouse wheel needs to stay in the same position or you get weird pops on re-focus.
			// Note: if a control actually relies on the literal mWheelPos, this is probably going
			// to make the control act weird.
			stateData.mWheelDelta = 0;
			stateData.mWheelPos = gWheelPos;

			stateData.mCursorPosX = ( s16 )( clientRect.right - clientRect.left ) / 2;
			stateData.mCursorPosY = ( s16 )( clientRect.bottom - clientRect.top ) / 2;
			return;
		}

#ifdef RENDER_SPACE_MOUSE_COORDS
		const Gfx::tScreenPtr& screen = tGameAppBase::fInstance( ).fScreen( );
		const Gfx::tScreenCreationOptions& opts = screen->fCreateOpts( );

		if( clientWidth<=0 || clientHeight<=0 )
		{
			win32CursorPos.x = -1;
			win32CursorPos.y = -1;
		}
		else if( screen->fLetterbox1280x720() )
		{
			Math::tRectf fit = Math::tRectf::fConstructEx(0,0,(f32)clientWidth,(f32)clientHeight).fMakeCenterFittedRect(1280,720);
			win32CursorPos.x -= (s32)fit.mL;
			win32CursorPos.y -= (s32)fit.mT;

			// Mutliply win32CursorPos by the larger of these two ratios:
			//if( opts.mBackBufferWidth / clientWidth > opts.mBackBufferHeight / clientHeight )

			// In the actual conditional we multiply both sides by clientWidth*clientHeight to avoid integer divs and the resulting rounding:
			if( opts.mBackBufferWidth * clientHeight > opts.mBackBufferHeight * clientWidth )
			{
				win32CursorPos.x = win32CursorPos.x * (s32)opts.mBackBufferWidth / clientWidth;
				win32CursorPos.y = win32CursorPos.y * (s32)opts.mBackBufferWidth / clientWidth;
			}
			else
			{
				win32CursorPos.x = win32CursorPos.x * (s32)opts.mBackBufferHeight / clientHeight;
				win32CursorPos.y = win32CursorPos.y * (s32)opts.mBackBufferHeight / clientHeight;
			}
		}
		else // stretch
		{
			win32CursorPos.x = win32CursorPos.x * (s32)opts.mBackBufferWidth / clientWidth;
			win32CursorPos.y = win32CursorPos.y * (s32)opts.mBackBufferHeight / clientHeight;
		}
#endif

		stateData.mCursorDeltaX		= ( s16 )win32CursorPos.x - fGetState( ).mCursorPosX;
		stateData.mCursorDeltaY		= ( s16 )win32CursorPos.y - fGetState( ).mCursorPosY;
		stateData.mWheelDelta		= ( s16 )gWheelPos - fGetState( ).mWheelPos;
		stateData.mCursorPosX		= ( s16 )win32CursorPos.x;
		stateData.mCursorPosY		= ( s16 )win32CursorPos.y;
		stateData.mWheelPos			= ( s16 )gWheelPos;
		  
		const b32 swapLandRbuttons = GetSystemMetrics( SM_SWAPBUTTON );
		stateData.mFlags.fSetBit( cButtonLeft, GetAsyncKeyState( swapLandRbuttons ? VK_RBUTTON : VK_LBUTTON ) & 0x8000 );
		stateData.mFlags.fSetBit( cButtonRight, GetAsyncKeyState( swapLandRbuttons ? VK_LBUTTON : VK_RBUTTON ) & 0x8000 );
		stateData.mFlags.fSetBit( cButtonMiddle, GetAsyncKeyState( VK_MBUTTON ) & 0x8000 );
		stateData.mFlags.fSetBit( cFlagCursorInClient, !outsideClient );
	}

	void tMouse::fStartup( tGenericWindowHandle winHandle )
	{
		fShutdown( );

		mWindowHandle = winHandle;
		HWND hwnd = ( HWND )( winHandle );
		if( !hwnd )
		{
			log_warning( "Failure to create Mouse input device, no HWND window handle." );
			return;
		}
	}

	void tMouse::fShutdown( )
	{
		mWindowHandle = 0;
	}

}}
#endif // #if defined( platform_pcdx )

