#include "BasePch.hpp"
#include "tScriptableGoal.hpp"

namespace Sig { namespace AI
{
	tScriptableGoal::tScriptableGoal( )
		: mTimer( 0.f )
		, mProcessFreq( 0.f )
	{
	}
	void tScriptableGoal::fOnProcess( tGoalPtr& goalPtr, f32 dt )
	{
		if( mTimer >= mProcessFreq )
		{
			mTimer = 0.f;

			Sqrat::Function f( goalPtr.fScriptObject( ), "Process" );
			if( !f.IsNull( ) ) f.Execute( dt );
		}
		else
			mTimer += dt;
	}
	void tScriptableGoal::fSetCompleted( )
	{
		mStatus = cStatusComplete;
	}

	void tScriptableGoal::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tScriptableGoal, tGoal, Sqrat::NoCopy<tScriptableGoal> > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("ProcessFrequency"),	&tScriptableGoal::mProcessFreq)
			.Func(_SC("SetCompleted"),		&tScriptableGoal::fSetCompleted)
			;
		vm.fNamespace(_SC("AI")).Bind(_SC("ScriptableGoal"), classDesc);
	}

}}


