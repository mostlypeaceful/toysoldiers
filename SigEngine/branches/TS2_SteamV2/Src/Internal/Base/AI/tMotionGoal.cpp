#include "BasePch.hpp"
#include "tMotionGoal.hpp"
#include "Logic/tAnimatable.hpp"

namespace Sig { namespace AI
{
	tMotionGoal::tMotionGoal( )
	{
	}
	tMotionGoal::tMotionGoal( tLogic* logic, const char* motionStateName, const Sqrat::Object& motionStateTable )
	{
		fSetMotionState( logic, motionStateName, motionStateTable );
	}
	void tMotionGoal::fOnActivate( tLogic* logic )
	{
		fExecuteMotionState( logic );
	}
	void tMotionGoal::fSetMotionState( tLogic* logic, const char* motionStateName, const Sqrat::Object& motionStateTable )
	{
		sigassert( logic && logic->fQueryAnimatable( ) );
		mApplyMotionState = logic->fQueryAnimatable( )->fMotionMap( ).fMapState( motionStateName );
		mMotionStateParams = motionStateTable.IsNull( ) ? Sqrat::Object( tScriptVm::fInstance( ).fEmptyTable( ) ) : motionStateTable;
		if_devmenu( mStateName = tStringPtr( motionStateName ) );
	}
	void tMotionGoal::fExecuteMotionState( tLogic* logic )
	{
		sigassert( logic && logic->fQueryAnimatable( ) );
		if_devmenu( logic->fQueryAnimatable( )->fSetCurrentMotionStateName( mStateName ) );
		mApplyMotionState.Execute( mMotionStateParams );
	}
	f32 tMotionGoal::fEvaluateMotionState( tLogic* logic )
	{
		sigassert( logic && logic->fQueryAnimatable( ) );
		if_devmenu( logic->fQueryAnimatable( )->fSetCurrentMotionStateName( mStateName ) );
		return mApplyMotionState.Evaluate<f32>( mMotionStateParams );
	}
	void tMotionGoal::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tMotionGoal, tGoal, Sqrat::NoCopy<tMotionGoal> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("SetMotionState"), &tMotionGoal::fSetMotionState)
			;
		vm.fNamespace(_SC("AI")).Bind(_SC("MotionGoal"), classDesc);
	}




	tTimedMotionGoal::tTimedMotionGoal( ) 
		: mTimer( 0.f ), mTimeToLive( 1.f )
	{
	}
	tTimedMotionGoal::tTimedMotionGoal( tLogic* logic, const char* motionStateName, const Sqrat::Object& motionStateTable )
		: mTimer( 0.f ), mTimeToLive( 1.f )
	{
		fSetMotionState( logic, motionStateName, motionStateTable );
	}
	void tTimedMotionGoal::fOnActivate( tLogic* logic )
	{
		tMotionGoal::fExecuteMotionState( logic );
		mTimer = 0.f;
	}
	void tTimedMotionGoal::fOnProcess( tGoalPtr& goalPtr, tLogic* logic, f32 dt )
	{
		if( mTimer >= mTimeToLive )
			fMarkAsComplete( );
		else
			mTimer += dt;
	}
#ifdef sig_devmenu
	std::string tTimedMotionGoal::fExtraWorldDebugText( ) const
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision( 3 ) << "TimedMotionGoal: (time = " << mTimer << ") (ttl = " << mTimeToLive << ")";
		return ss.str( );
	}
#endif//sig_devmenu
	void tTimedMotionGoal::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tTimedMotionGoal, tMotionGoal, Sqrat::NoCopy<tTimedMotionGoal> > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("TimeToLive"), &tTimedMotionGoal::mTimeToLive)
			;
		vm.fNamespace(_SC("AI")).Bind(_SC("TimedMotionGoal"), classDesc);
	}




	tOneShotMotionGoal::tOneShotMotionGoal( )
	{
	}
	tOneShotMotionGoal::tOneShotMotionGoal( tLogic* logic, const char* motionStateName, const Sqrat::Object& motionStateTable )
	{
		fSetMotionState( logic, motionStateName, motionStateTable );
	}
	void tOneShotMotionGoal::fOnActivate( tLogic* logic )
	{
		mTimeToLive = tMotionGoal::fEvaluateMotionState( logic );
		mTimer = 0.f;
	}
	void tOneShotMotionGoal::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tOneShotMotionGoal, tTimedMotionGoal, Sqrat::NoCopy<tOneShotMotionGoal> > classDesc( vm.fSq( ) );
		vm.fNamespace(_SC("AI")).Bind(_SC("OneShotMotionGoal"), classDesc);
	}
}}


