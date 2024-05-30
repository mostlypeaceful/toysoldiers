#ifndef __tGameSimulationAppState__
#define __tGameSimulationAppState__
#include "tAppStateDefault.hpp"

namespace Sig
{

	class tGameSimulationAppState : public tAppStateDefault
	{
	public:
		tGameSimulationAppState( );
		virtual void fOnBecomingCurrent( );
		virtual void fOnTick( );
	private:
		Time::tStopWatch mTimer;
	};
}

#endif//__tGameSimulationAppState__

