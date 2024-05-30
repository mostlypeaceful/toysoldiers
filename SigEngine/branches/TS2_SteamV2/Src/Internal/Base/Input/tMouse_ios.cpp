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

}}
#endif//#if defined( platform_ios )
