#include "BasePch.hpp"
#if defined( platform_metro )
#include "tTouch.hpp"
#include "Win32Include.hpp"
#include "tImmersiveWindow.hpp"
#include "Gfx/tScreen.hpp"
#include "tGameAppBase.hpp"
#include "Debug/CrashDump.hpp"

#define RENDER_SPACE_TOUCH_COORDS
// (e.g. not true screen coords)

// #define MAP_COREWINDOWS
//
// Apparently 1 unique CoreWindow^ != one actual window due to threading/apartments bullshit.
// There's a "Platform::Agile" mentioned in the resulting warnings, but this doesn't exist.
// For now, we're just assuming there's only one window ever.  If that changes, we'll really
// need to figure out that multi-apartment bullshit.

namespace Sig { namespace Input
{
	devvar( bool, Input_Multitouch, true );

	namespace
	{
		using namespace Windows::Devices::Input;
		using namespace Windows::UI::Core;

		typedef tGrowableArray<tTouch*> tTouchInstances;

		struct tPerWindow
		{
			tTouchInstances mTouchInstances;

			/// \brief Used to unregister events when we're done with everything.
			Windows::Foundation::EventRegistrationToken
				mPointerMovedCookie,
				mPointerPressedCookie,
				mPointerReleasedCookie,
				
				mPointerEnteredCookie,
				mPointerExitedCookie,
				mPointerCaptureLostCookie;
		};

		tHashTable<tImmersiveWindow*,tPerWindow> gWindowData;
#ifdef MAP_COREWINDOWS
		tHashTable<CoreWindow^,tImmersiveWindow*> gCoreToWindow;
#else
		tImmersiveWindow* gWindow;
#endif

		tImmersiveWindow* fImmersiveWindowFrom( CoreWindow^ window )
		{
#ifdef MAP_COREWINDOWS
			sigassert( gCoreToWindow.fFind(window) );
			return gCoreToWindow[window];
#else
			return gWindow;
#endif
		}

		Math::tVec2f fToLocalFromWindow( CoreWindow^ window, Windows::Foundation::Point position )
		{
			Math::tVec2f v( position.X, position.Y );
#ifdef RENDER_SPACE_TOUCH_COORDS
			const Gfx::tScreenPtr& screen = tGameAppBase::fInstance( ).fScreen( );
			const Gfx::tScreenCreationOptions& opts = screen->fCreateOpts( );

			// Remap from window -> back buffer, in case there's stretching/shrinking
			v.x = v.x * opts.mBackBufferWidth  / window->Bounds.Width;
			v.y = v.y * opts.mBackBufferHeight / window->Bounds.Height;

			if( screen->fLetterbox1280x720( ) )
			{
				// Remap from back buffer -> viewport
				log_warning( 0, "Not properly implemented: letterbox touch" );
			}
#endif
			return v;
		}

		const u32 cInvalidFingerSlot = ~0u;
	}

	class tTouch::tPlatformInternal
	{
	public:
		static u32 fGetFingerSlot( tTouch* touch, u32 hwId )
		{
			tTouch::tStateData& raw = touch->mRawState;

			// Find existing finger or new slot.
			u32 fingerI = cInvalidFingerSlot;
			for( u32 i = 0 ; i < raw.fCount( ) ; ++i )
			{
				if( raw[i].mHwFingerId == hwId )
				{
					fingerI = i; // use this existing finger
					return fingerI;
				}
				else if( tTouch::fCanReuseSlot(raw[i]) && fingerI == cInvalidFingerSlot )
				{
					fingerI = i; // maybe use this first unused finger slot
				}
			}

			if( fingerI == cInvalidFingerSlot )
			{
				log_warning( 0, "OUT OF FINGER SLOTS" );
			}

			return fingerI;
		}

		static b32 fCareAbout( PointerEventArgs^ args )
		{
			switch( args->CurrentPoint->PointerDevice->PointerDeviceType )
			{
			case PointerDeviceType::Touch:
			case PointerDeviceType::Pen:
				// A stylus ("Pen") is more "touchlike" (can be placed in new, arbitrary absolute positions)
				// than "mouselike" (a relative motion only device, unless you've got a crazy tablet mouse)
				// so we'll handle it as "touch" too.
				return true;
			default:
				return false;
			}
		}

		/// \brief Callback registered with metro to capture multitouch information.
		static void fOnPointerMoved( CoreWindow^ window, PointerEventArgs^ args )
		{
			if( !fCareAbout(args) )
				return;

			if( args->CurrentPoint->Properties->IsCanceled )
			{
				fOnPointerReleased(window,args);
				return;
			}

			const Math::tVec2f pos = fToLocalFromWindow( window, args->CurrentPoint->Position );

			tTouchInstances& touches = gWindowData[fImmersiveWindowFrom(window)].mTouchInstances;
			for( u32 i=0 ; i<touches.fCount( ); ++i )
			{
				tTouch* touch = touches[i];
				const u32 slot = fGetFingerSlot( touch, args->CurrentPoint->PointerId );
				if( slot == cInvalidFingerSlot )
					continue; // not in this tTouch

				tTouch::tTouchStateData& raw = touch->mRawState[slot];
				raw.mHwFingerId = args->CurrentPoint->PointerId;
				raw.mPosition = pos;
			}
		}

		/// \brief Callback registered with metro to capture multitouch information.
		static void fOnPointerPressed( CoreWindow^ window, PointerEventArgs^ args )
		{
			if( !fCareAbout(args) )
				return;

			const Math::tVec2f pos = fToLocalFromWindow( window, args->CurrentPoint->Position );

			tTouchInstances& touches = gWindowData[fImmersiveWindowFrom(window)].mTouchInstances;
			for( u32 i=0 ; i<touches.fCount( ); ++i )
			{
				tTouch* touch = touches[i];
				const u32 slot = fGetFingerSlot( touch, args->CurrentPoint->PointerId );
				if( slot == cInvalidFingerSlot )
					continue; // not in this tTouch

				tTouch::tTouchStateData& raw = touch->mRawState[slot];
				raw.mHwFingerId = args->CurrentPoint->PointerId;
				touch->fHandleRawDown( (tTouch::tFinger)slot, pos );
				raw.mPosition = pos;
			}
		}

		/// \brief Callback registered with metro to capture multitouch information.
		static void fOnPointerReleased( CoreWindow^ window, PointerEventArgs^ args )
		{
			if( !fCareAbout(args) )
				return;

			const Math::tVec2f pos = fToLocalFromWindow( window, args->CurrentPoint->Position );

			tTouchInstances& touches = gWindowData[fImmersiveWindowFrom(window)].mTouchInstances;
			for( u32 i=0 ; i<touches.fCount( ); ++i )
			{
				tTouch* touch = touches[i];
				const u32 slot = fGetFingerSlot( touch, args->CurrentPoint->PointerId );
				if( slot == cInvalidFingerSlot )
					continue; // not in this tTouch

				tTouch::tTouchStateData& raw = touch->mRawState[slot];
				touch->fHandleRawUp( (tTouch::tFinger)slot, pos );
				raw.mPosition = pos;
			}
		}
	};

	void tTouch::fCaptureStateUnbuffered( tStateData& stateData, f32 dt )
	{
		if( mWindowHandle )
		{
			// Deal with unreported ups to help avoid 'finger forever held down at this spot' deadlocks
			for( u32 i=0; i<mRawState.fCount( ); ++i )
			{
				if( !fFingerHeld((tFinger)i) )
					continue;

				auto point = Windows::UI::Input::PointerPoint::GetCurrentPoint(mRawState[i].mHwFingerId);
				if( point==nullptr || !point->IsInContact )
					fHandleRawUp( (tFinger)i, mRawState[i].mPosition );
			}
		}

		// Actually copy the data
		stateData = mRawState;
	}

	void tTouch::fStartup( tGenericWindowHandle winHandle )
	{
		fShutdown( );

		sigassert(!mWindowHandle);
		tImmersiveWindow* window = ( tImmersiveWindow* )( winHandle );
		if( !window )
		{
			log_warning( Log::cFlagInput, "Failure to create Touch input device, no immersive window handle." );
			return;
		}

		using namespace Windows::Foundation;

		CoreWindow^ core = window->fGetCoreWindow();
#ifdef MAP_COREWINDOWS
		gCoreToWindow[core] = window;
#else
		sigassert( gWindow==nullptr || gWindow==window );
		gWindow = window;
#endif
		if( gWindowData[window].mTouchInstances.fCount( ) == 0 )
		{
			// No callbacks were registered, register them.
			gWindowData[window].mPointerMovedCookie			= core->PointerMoved		::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&tPlatformInternal::fOnPointerMoved		) );
			gWindowData[window].mPointerPressedCookie		= core->PointerPressed		::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&tPlatformInternal::fOnPointerPressed	) );
			gWindowData[window].mPointerReleasedCookie		= core->PointerReleased		::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&tPlatformInternal::fOnPointerReleased	) );
			//gWindowData[window].mPointerEnteredCookie		= core->PointerEntered		::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&tPlatformInternal::fOnPointerReleased	) );
			gWindowData[window].mPointerExitedCookie		= core->PointerExited		::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&tPlatformInternal::fOnPointerReleased	) );
			//gWindowData[window].mPointerCaptureLostCookie	= core->PointerCaptureLost	::add( Debug::fInferCallbackTypeAndDumpCrashSEH(&tPlatformInternal::fOnPointerReleased	) );
		}
		gWindowData[window].mTouchInstances.fPushBack( this );
		mWindowHandle = winHandle;
	}

	void tTouch::fShutdown( )
	{
		if( mWindowHandle )
		{
			tImmersiveWindow* immersive = (tImmersiveWindow*)mWindowHandle;
			tPerWindow& pwi = gWindowData[immersive];
			pwi.mTouchInstances.fErase( pwi.mTouchInstances.fIndexOf(this) );
			if( pwi.mTouchInstances.fCount() == 0 )
			{
				 // No tTouches interested in the events anymore, unregister them.
				CoreWindow^ core = immersive->fGetCoreWindow();
				core->PointerMoved		::remove( pwi.mPointerMovedCookie );
				core->PointerPressed	::remove( pwi.mPointerPressedCookie );
				core->PointerReleased	::remove( pwi.mPointerReleasedCookie );
				//core->PointerEntered	::remove( pwi.mPointerEnteredCookie );
				core->PointerExited		::remove( pwi.mPointerExitedCookie );
				//core->PointerCaptureLost::remove( pwi.mPointerCaptureLostCookie );
			}
		}
		mWindowHandle = 0;
	}

}}
#endif//#if defined( platform_metro )
