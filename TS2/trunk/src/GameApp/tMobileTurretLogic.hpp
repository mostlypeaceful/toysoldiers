#ifndef __tMobileTurretLogic__
#define __tMobileTurretLogic__
#include "tBreakableLogic.hpp"

namespace Sig
{
	
	class tVehicleLogic;

	class tMobileTurretLogic : public tAnimatedBreakableLogic
	{
		define_dynamic_cast( tMobileTurretLogic, tAnimatedBreakableLogic );
	public:

		tMobileTurretLogic( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );

		tVehicleLogic* fOwnerVehicle( ) const { return mVehicleLogic; }

	private:
		tEntityPtr		mVehicle;
		tVehicleLogic*	mVehicleLogic;

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	};

}

#endif//__tMobileTurretLogic__
