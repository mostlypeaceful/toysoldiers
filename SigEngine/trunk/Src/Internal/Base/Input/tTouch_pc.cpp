#include "BasePch.hpp"
#if defined( platform_pcdx ) && !defined( target_tools )
#include "tTouch.hpp"
#include "Win32Include.hpp"

namespace Sig { namespace Input
{
	devvar( bool, Input_Multitouch, true );

	namespace
	{
		tHashTable<HWND,tGrowableArray<tTouch*>> gTouchInstances;
	}

	void fTouchWndProc( HWND hwnd, WPARAM wParam, LPARAM lParam )
	{
		tTouch::fTouchWndProc( hwnd, wParam, lParam );
	}

	void tTouch::fTouchWndProc( HWND hwnd, WPARAM wParam, LPARAM lParam )
	{
		const tGrowableArray<tTouch*>* windowTouchInstances = gTouchInstances.fFind(hwnd);
		if ( !windowTouchInstances || windowTouchInstances->fCount()==0 )
			return; // ignore WM_TOUCH, no registered tTouch wants to handle it.

		BOOL bHandled = FALSE;
		const UINT cInputs = LOWORD(wParam);
		tDynamicArray<TOUCHINPUT> pInputs(cInputs);

		POINT clientOffset = { 0, 0 };
		ClientToScreen( hwnd, &clientOffset );

		if (!GetTouchInputInfo((HTOUCHINPUT)lParam, cInputs, pInputs.fBegin( ), sizeof(TOUCHINPUT)))
			return;

		for ( u32 t=0 ; t<windowTouchInstances->fCount( ) ; ++t )
		{
			tTouch* touch = (*windowTouchInstances)[t];
			tTouch::tStateData& raw = touch->mRawState;

			for ( u32 i=0 ; i<tTouch::cFingerCount ; ++i )
			{
				if ( tTouch::fCanReuseSlot(raw[i]) )
				{
					raw[i] = tTouch::tTouchStateData(); // clear any previous position / fingerid info
				}
			}

			for (UINT i=0; i < cInputs; i++)
			{
				TOUCHINPUT ti = pInputs[i];
				const b32 primary = fTestBits( ti.dwFlags, TOUCHEVENTF_PRIMARY );
				const b32 down    = fTestBits( ti.dwFlags, TOUCHEVENTF_DOWN );
				const b32 move    = fTestBits( ti.dwFlags, TOUCHEVENTF_MOVE );
				const b32 up      = fTestBits( ti.dwFlags, TOUCHEVENTF_UP   );

				// TOUCHINPUT x and y are stored in 100th's of a pixel, in physical coordinates
				const Math::tVec2f fingerPos( ti.x / 100.f - clientOffset.x, ti.y / 100.f - clientOffset.y );

				if ( down && touch->fRecoverBadFingers(ti.dwID,fingerPos) )
				{
					// already handled by fRecoverBadFingers
				}
				else if (!Input_Multitouch) // Monotouch
				{
					//log_line( 0, "TI " << i << "  " << ti.dwID << " " << ti.x << " " << ti.y << (primary?"P":" ") << (down?"D":" ") << (move?"M":" ") << (up?"U":" ") );
					if (!primary)
						continue;

					else if ( down )
					{
						touch->fHandleRawDown( tTouch::cFinger1, fingerPos );
					}
					else if ( up )
						touch->fHandleRawUp( tTouch::cFinger1, fingerPos );

					raw[ tTouch::cFinger1 ].mPosition = fingerPos;
				}
				else
				{
					sigassert( Input_Multitouch );

					// Find existing finger or new slot.
					const u32 invalid = ~0u;
					u32 fingerI = invalid;
					for ( u32 i = 0 ; i < raw.fCount( ) ; ++i )
					{
						if ( raw[i].mHwFingerId == ti.dwID )
						{
							fingerI = i; // use this existing finger
							break;
						}
						else if ( tTouch::fCanReuseSlot(raw[i]) && fingerI == invalid )
						{
							fingerI = i; // maybe use this first unused finger slot
						}
					}

					if ( fingerI==invalid )
					{
						log_warning( "OUT OF FINGER SLOTS" );
						break; // no matching slots and out of fingers.
					}

					const tTouch::tFinger finger = (tTouch::tFinger)fingerI;

					if ( down || move )
						raw[finger].mHwFingerId  = ti.dwID;

					if ( down )
					{
						touch->fHandleRawDown( finger, fingerPos );
					}

					if ( up )
						touch->fHandleRawUp( finger, fingerPos );

					raw[finger].mPosition = fingerPos;
				}
			}
		}

		// if you handled the message, close the touch input handle and return
		CloseTouchInputHandle((HTOUCHINPUT)lParam);
	}


	void tTouch::fCaptureStateUnbuffered( tStateData& stateData, f32 dt )
	{
		stateData = mRawState;
	}

	void tTouch::fStartup( tGenericWindowHandle winHandle )
	{
		fShutdown( );

		mWindowHandle = winHandle;
		HWND hwnd = ( HWND )( winHandle );
		if( !hwnd )
		{
			log_warning( "Failure to create Touch input device, no HWND window handle." );
			return;
		}
		gTouchInstances[hwnd].fPushBack( this );

		RegisterTouchWindow( hwnd, 0 );
	}

	void tTouch::fShutdown( )
	{
		if ( mWindowHandle )
			gTouchInstances[(HWND)mWindowHandle].fErase(gTouchInstances[(HWND)mWindowHandle].fIndexOf(this));
		mWindowHandle = 0;
	}

}}

#endif // defined( platform_pcdx ) && !defined( target_tools )
