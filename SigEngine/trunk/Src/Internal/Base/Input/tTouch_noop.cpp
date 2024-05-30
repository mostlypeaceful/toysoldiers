#include "BasePch.hpp"
#if defined( platform_xbox360 ) || (defined( platform_pcdx ) && defined( target_tools ))
#include "tTouch.hpp"

namespace Sig { namespace Input
{
	devvar( bool, Input_Multitouch, true );

	void fTouchWndProc( HWND hwnd, WPARAM wParam, LPARAM lParam )
	{
		log_warning_unimplemented( );
	}

#if defined( platform_pcdx )
	void tTouch::fTouchWndProc( HWND hwnd, WPARAM wParam, LPARAM lParam )
	{
		log_warning_unimplemented( );
	}
#endif

	void tTouch::fCaptureStateUnbuffered( tStateData& stateData, f32 dt )
	{
		log_warning_unimplemented( );
	}

	void tTouch::fStartup( tGenericWindowHandle winHandle )
	{
		log_warning_unimplemented( );
	}

	void tTouch::fShutdown( )
	{
	}

}}

#endif // defined( platform_xbox360 ) || (defined( platform_pcdx ) && defined( target_tools ))
