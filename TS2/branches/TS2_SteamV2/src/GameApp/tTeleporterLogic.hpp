#ifndef __tTeleporterLogic__
#define __tTeleporterLogic__
#include "tUnitLogic.hpp"
#include "tBreakableLogic.hpp"

namespace Sig
{
	class tTeleporterLogic : public tBreakableLogic
	{
		define_dynamic_cast( tTeleporterLogic, tBreakableLogic );
	public:
		static void fExportScriptInterface( tScriptVm& vm );

		virtual void fOnSpawn( );
		virtual void fOnStateChanged( );

		tStringPtr mExitPathName;	   //The path point to resume when this teleporter is used as a destination.
		tStringPtr mDestroyedPathNameDisable; //The path point name to disable when this teleporter is destroyed
		tStringPtr mDestroyedPathNameEnable; //The path point name to enable when this teleporter is destroyed
	};

}

#endif//__tTeleporterLogic__
