#include "GameAppPch.hpp"
#include "tGameSimulationAppState.hpp"
#include "tGameApp.hpp"

namespace Sig
{
	tGameSimulationAppState::tGameSimulationAppState( )
	{
		tGameApp::fInstance( ).fSetIngameSimulationState( true );
	}

	void tGameSimulationAppState::fOnBecomingCurrent( )
	{
		tGameApp::fInstance( ).fOnSimulationBegin( );
		mTimer.fResetElapsedS( );
	}
	void tGameSimulationAppState::fOnTick( )
	{
		profile_pix("tGameSim*::fOnTick");
		if( mTimer.fGetElapsedS( ) > 10.f )
		{
			mTimer.fStop( );
			mTimer.fResetElapsedS( );
			log_line( 0, "@LEVEL_START_SUCCESS@" );
		}
	}
}
