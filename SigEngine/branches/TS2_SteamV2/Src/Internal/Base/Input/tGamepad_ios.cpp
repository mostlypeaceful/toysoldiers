#include "BasePch.hpp"
#if defined( platform_apple )
#include "tGamepad.hpp"

namespace Sig { namespace Input
{
	const tGamepad::tButton tGamepad::cButtonStart			= 1 << 0;
	const tGamepad::tButton tGamepad::cButtonSelect			= 1 << 1;
	const tGamepad::tButton tGamepad::cButtonA				= 1 << 2;
	const tGamepad::tButton tGamepad::cButtonB				= 1 << 3;
	const tGamepad::tButton tGamepad::cButtonX				= 1 << 4;
	const tGamepad::tButton tGamepad::cButtonY				= 1 << 5;
	const tGamepad::tButton tGamepad::cButtonDPadRight		= 1 << 6;
	const tGamepad::tButton tGamepad::cButtonDPadUp			= 1 << 7;
	const tGamepad::tButton tGamepad::cButtonDPadLeft		= 1 << 8;
	const tGamepad::tButton tGamepad::cButtonDPadDown		= 1 << 9;
	const tGamepad::tButton tGamepad::cButtonLShoulder		= 1 << 10;
	const tGamepad::tButton tGamepad::cButtonLThumb			= 1 << 11;
	const tGamepad::tButton tGamepad::cButtonRShoulder		= 1 << 12;
	const tGamepad::tButton tGamepad::cButtonRThumb			= 1 << 13;
	const tGamepad::tButton tGamepad::cButtonRTrigger		= 1 << 16;
	const tGamepad::tButton tGamepad::cButtonLTrigger		= 1 << 17; 
	const tGamepad::tButton tGamepad::cButtonRThumbMaxMag	= 1 << 18; 
	const tGamepad::tButton tGamepad::cButtonLThumbMaxMag	= 1 << 19; 
	const tGamepad::tButton tGamepad::cButtonRThumbMinMag	= 1 << 20; 
	const tGamepad::tButton tGamepad::cButtonLThumbMinMag	= 1 << 21; 

	void tGamepad::fCaptureStateUnbuffered( tStateData& stateData, u32 userIndex, f32 dt )
	{
	}

	void tGamepad::fStartup( )
	{
	}

	void tGamepad::fShutdown( )
	{
	}

}}
#endif//#if defined( platform_apple )

