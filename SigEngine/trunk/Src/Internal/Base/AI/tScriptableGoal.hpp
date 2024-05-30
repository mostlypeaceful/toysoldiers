#ifndef __tScriptableGoal__
#define __tScriptableGoal__
#include "tGoal.hpp"

namespace Sig { namespace AI
{

	class tScriptableGoal : public tGoal
	{
		define_dynamic_cast( tScriptableGoal, tGoal );
	private:
		f32 mTimer;
		f32 mProcessFreq;
	public:
		tScriptableGoal( );
		virtual void fOnProcess( tGoalPtr& goalPtr, f32 dt );
	private:
		void fSetCompleted( );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

}}

#endif//__tScriptableGoal__
