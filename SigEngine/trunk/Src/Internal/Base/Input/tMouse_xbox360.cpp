#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tMouse.hpp"

namespace Sig { namespace Input
{
	void tMouse::fCaptureStateUnbuffered( tStateData& stateData, f32 dt )
	{
		// TODO mouse supported?
		fZeroOut( stateData );
	}

	void tMouse::fStartup( tGenericWindowHandle winHandle )
	{
		fShutdown( );
	}

	void tMouse::fShutdown( )
	{
		mWindowHandle = 0;
	}

	void tMouse::fShowCursor( b32 show )
	{
		log_warning_unimplemented( );
	}

	b32 tMouse::fCursorHidden( )
	{
		log_warning_unimplemented( );
		return true;
	}

}}
#endif//#if defined( platform_xbox360 )
