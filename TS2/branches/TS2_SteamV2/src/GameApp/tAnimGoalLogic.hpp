#ifndef __tAnimGoalLogic__
#define __tAnimGoalLogic__
#include "Logic/tGoalDriven.hpp"
#include "Logic/tAnimatable.hpp"

namespace Sig
{
	class tAnimGoalLogic : public tLogic
		, public Logic::tGoalDriven
		, public Logic::tAnimatable
	{
		define_dynamic_cast( tAnimGoalLogic, tLogic );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tAnimGoalLogic( );
		virtual ~tAnimGoalLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
	public: // query specific components
		virtual Logic::tGoalDriven* fQueryGoalDriven( ) { return this; }
		virtual Logic::tAnimatable* fQueryAnimatable( ) { return this; }
	protected:
		virtual void fActST( f32 dt );
		virtual void fAnimateMT( f32 dt );
		virtual void fMoveST( f32 dt );
	};

}

#endif//__tAnimGoalLogic__
