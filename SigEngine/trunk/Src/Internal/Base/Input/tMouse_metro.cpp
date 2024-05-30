#include "BasePch.hpp"
#if defined( platform_metro )
#include "tMouse.hpp"
#include "Win32Include.hpp"
#include "tImmersiveWindow.hpp"
#include "tGameAppBase.hpp"
#include "Debug/CrashDump.hpp"

#define RENDER_SPACE_MOUSE_COORDS
// (e.g. not true screen coords)

namespace Sig { namespace Input
{
	namespace
	{
		using namespace Windows::UI::Core;

		u32 gMousePointerID = 0;
		s32 gWheelPos = 0;
		s32 gWheelDelta = 0;
		Math::tVec2f gMousePos(-1.001f,-1.001f);

		b32 gLeftMB, gMiddleMB, gRightMB;

		Windows::Foundation::EventRegistrationToken
			gMouseWheelCookie,
			gMouseDownCookie,
			gMouseUpCookie,
			gMouseMoveCookie,
			gMouseLostCookie,
			gMouseEnteredCookie,
			gMouseExitedCookie;

		void fMouseWheelCallback( CoreWindow^ window, PointerEventArgs^ args )
		{
			if ( args->CurrentPoint->PointerDevice->PointerDeviceType != Windows::Devices::Input::PointerDeviceType::Mouse )
				return; //we're only interested in mouse data

			gMousePointerID = args->CurrentPoint->PointerId;
			gWheelDelta = args->CurrentPoint->Properties->MouseWheelDelta;
			gWheelPos += gWheelDelta;
		}

		void fUpdatePos( CoreWindow^ window, PointerEventArgs^ args )
		{
			f32 x = args->CurrentPoint->Position.X;
			f32 y = args->CurrentPoint->Position.Y;

#ifdef RENDER_SPACE_MOUSE_COORDS
			const Gfx::tScreenPtr& screen = tGameAppBase::fInstance( ).fScreen( );
			const Gfx::tScreenCreationOptions& opts = screen->fCreateOpts( );

			if( screen->fLetterbox1280x720( ) )
			{
				Math::tRectf fit = Math::tRectf(0,0,window->Bounds.Height,window->Bounds.Width).fMakeCenterFittedRect(1280,720);
				f32 scale = fMax( (f32)opts.mBackBufferWidth / window->Bounds.Width, (f32)opts.mBackBufferHeight / window->Bounds.Height );
				x = (x - fit.mL) * scale;
				y = (y - fit.mT) * scale;
			}
			else // stretch
			{
				x = x * opts.mBackBufferWidth  / window->Bounds.Width;
				y = y * opts.mBackBufferHeight / window->Bounds.Height;
			}
#endif

			gMousePointerID = args->CurrentPoint->PointerId;
			gMousePos.x = x;
			gMousePos.y = y;
		}

		void fMouseGeneralCallback( CoreWindow^ window, PointerEventArgs^ args )
		{
			if ( args->CurrentPoint->PointerDevice->PointerDeviceType != Windows::Devices::Input::PointerDeviceType::Mouse )
				return; //we're only interested in mouse data

			fUpdatePos(window,args);

			if( args->CurrentPoint->Properties->IsCanceled )
			{
				gLeftMB		= false;
				gMiddleMB	= false;
				gRightMB	= false;
			}
			else
			{
				gLeftMB		= args->CurrentPoint->Properties->IsLeftButtonPressed;
				gMiddleMB	= args->CurrentPoint->Properties->IsMiddleButtonPressed;
				gRightMB	= args->CurrentPoint->Properties->IsRightButtonPressed;
			}
		}
	}

	void tMouse::fSetPosition( u32 x, u32 y )
	{
		// SetCursorPosition is probably also banned, don't use it either
		log_warning_unimplemented(0);
	}

	void tMouse::fShowCursor( b32 show )
	{
		// BANNED API: ShowCursor( show );
		log_warning_unimplemented(0); // probably for the best on metro as we're targeting it a touch platform for now
	}

	b32 tMouse::fCursorHidden( )
	{
		// GetCursorInfo is a banned API
		return false;
	}

	void tMouse::fCaptureStateUnbuffered( tStateData& stateData, f32 dt )
	{
		if( mWindowHandle && (gLeftMB || gMiddleMB || gRightMB) )
		{
			// Deal with unreported ups to help avoid 'mouse forever held down at this spot' deadlocks
			Windows::UI::Input::PointerPoint^ pointer = nullptr;

			try
			{
				pointer = Windows::UI::Input::PointerPoint::GetCurrentPoint(gMousePointerID);
			} catch( Platform::InvalidArgumentException^ ) {}

			if( pointer != nullptr )
			{
				if( pointer->Properties->IsCanceled )
				{
					gLeftMB		= false;
					gMiddleMB	= false;
					gRightMB	= false;
				}
				else
				{
					gLeftMB		= pointer->Properties->IsLeftButtonPressed;
					gMiddleMB	= pointer->Properties->IsMiddleButtonPressed;
					gRightMB	= pointer->Properties->IsRightButtonPressed;
				}
			}
		}

		// PointerPosition seems to be in screen-wide coordinates, but fortunately so is window->Bounds.
		Windows::UI::Core::CoreWindow^ window = ((tImmersiveWindow*)mWindowHandle)->fGetCoreWindow();
		Windows::Foundation::Rect bounds = window->Bounds;

		f32 curX = gMousePos.x;
		f32 curY = gMousePos.y;

		const Gfx::tScreenCreationOptions& opts = tGameAppBase::fInstance( ).fScreen( )->fCreateOpts( );
		const b32 outsideClient =
			curX < 0 ||
			curY < 0 ||
#ifdef RENDER_SPACE_MOUSE_COORDS
			curX >= opts.mBackBufferWidth ||
			curY >= opts.mBackBufferHeight;
#else
			curX >= bounds.Width ||
			curY >= bounds.Height ;
#endif

		stateData.mCursorDeltaX		= ( s16 )curX - fGetState( ).mCursorPosX;
		stateData.mCursorDeltaY		= ( s16 )curY - fGetState( ).mCursorPosY;
		stateData.mWheelDelta		= ( s16 )gWheelPos - fGetState( ).mWheelPos;
		//stateData.mWheelDelta		= 0;
		stateData.mCursorPosX		= ( s16 )curX;
		stateData.mCursorPosY		= ( s16 )curY;
		stateData.mWheelPos			= ( s16 )gWheelPos;
		//stateData.mWheelPos			= 0;

		//const b32 swapLandRbuttons = GetSystemMetrics( SM_SWAPBUTTON );

		using Windows::System::VirtualKey;
		using Windows::UI::Core::CoreVirtualKeyStates;

		const b32 swapLandRbuttons = false;
		stateData.mFlags.fSetBit( cButtonLeft  , swapLandRbuttons ? gRightMB : gLeftMB );
		stateData.mFlags.fSetBit( cButtonRight , swapLandRbuttons ? gLeftMB : gRightMB );
		stateData.mFlags.fSetBit( cButtonMiddle, gMiddleMB );
		stateData.mFlags.fSetBit( cFlagCursorInClient, !outsideClient );
	}

	void tMouse::fStartup( tGenericWindowHandle winHandle )
	{
		fShutdown( );

		mWindowHandle = winHandle;

		if( mWindowHandle )
		{
			using namespace Windows::UI::Core;
			using namespace Windows::Foundation;

			CoreWindow^ window = ((tImmersiveWindow*)mWindowHandle)->fGetCoreWindow();
			gMouseWheelCookie	= window->PointerWheelChanged	::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&fMouseWheelCallback	) );
			gMouseDownCookie	= window->PointerPressed		::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&fMouseGeneralCallback	) );
			gMouseMoveCookie	= window->PointerMoved			::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&fMouseGeneralCallback	) );
			gMouseUpCookie		= window->PointerReleased		::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&fMouseGeneralCallback	) );
			gMouseLostCookie	= window->PointerCaptureLost	::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&fMouseGeneralCallback	) );
			gMouseEnteredCookie	= window->PointerEntered		::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&fMouseGeneralCallback	) );
			gMouseExitedCookie	= window->PointerExited			::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&fMouseGeneralCallback	) );
		}
	}

	void tMouse::fShutdown( )
	{
		if( mWindowHandle )
		{
			Windows::UI::Core::CoreWindow^ window = ((tImmersiveWindow*)mWindowHandle)->fGetCoreWindow();
			window->PointerWheelChanged	::remove(gMouseWheelCookie);
			window->PointerPressed		::remove(gMouseDownCookie);
			window->PointerMoved		::remove(gMouseMoveCookie);
			window->PointerReleased		::remove(gMouseUpCookie);
			window->PointerCaptureLost	::remove(gMouseLostCookie);
			window->PointerEntered		::remove(gMouseEnteredCookie);
			window->PointerExited		::remove(gMouseExitedCookie);
		}

		mWindowHandle = 0;
	}

}}
#endif // #if defined( platform_metro )
