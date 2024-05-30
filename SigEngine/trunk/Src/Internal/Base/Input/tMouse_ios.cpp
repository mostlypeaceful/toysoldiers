#include "BasePch.hpp"
#if defined( platform_ios )
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
		log_warning_unimplemented(0); // probably for the best on metro as we're targeting it a touch platform for now
	}
	
	b32 tMouse::fCursorHidden( )
	{
		return false;
	}
}}

#endif//#if defined( platform_ios )
