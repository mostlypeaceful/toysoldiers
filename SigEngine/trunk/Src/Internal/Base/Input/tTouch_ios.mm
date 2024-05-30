#include "BasePch.hpp"
#if defined( platform_ios )
#include "tTouch.hpp"
#import "GameAppView_ios.hpp"

// Do we really want to even have gTouchEnabled?  Might be better to warn the dev somehow if it's not enabled on iOS...

namespace Sig { namespace Input
{
	namespace
	{
		tTouch::tStateData gTouchData;
		bool gTouchEnabled = false;
	}
	
	void fHandleIosTouch( UIView* view, NSSet* touches, b32 down )
	{
		using namespace Sig::Input;

		// TODO: Fingers currently aren't stable, make them stable.
		
		if (!gTouchEnabled)
			return;

		// Clear out existing fingers
		for ( u32 i=0 ; i < tTouch::cFingerCount ; ++i )
			gTouchData[i] = tTouch::tTouchStateData();

		// Read new finger positions
		u32 finger = 0;
		for ( UITouch* touch in touches )
		{
			if ( finger >= tTouch::cFingerCount )
				return;
				
			CGPoint location = [ touch locationInView:view ];
			gTouchData[ finger ].mPosition.x = location.x;
			gTouchData[ finger ].mPosition.y = location.y;
			gTouchData[ finger ].mFlags.fSetBit( tTouch::cFlagFingerHeld, down );
			++finger;
		}
	}

	void tTouch::fCaptureStateUnbuffered( tStateData& stateData, f32 dt )
	{
		stateData = gTouchData;
	}

	void tTouch::fStartup( tGenericWindowHandle winHandle )
	{
		gTouchEnabled = true;
	}
	
	void tTouch::fShutdown( )
	{
		gTouchEnabled = false;
	}

}}
#endif//#if defined( platform_ios )
