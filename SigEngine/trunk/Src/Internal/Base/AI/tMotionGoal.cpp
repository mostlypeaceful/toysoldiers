#include "BasePch.hpp"
#include "tMotionGoal.hpp"
#include "Logic/tAnimatable.hpp"

namespace Sig { namespace AI
{
	tMotionGoal::tMotionGoal( )
	{
	}
	tMotionGoal::tMotionGoal( const char* motionStateName, const Sqrat::Object& motionStateTable )
	{
		fSetMotionState( motionStateName, motionStateTable );
	}
	void tMotionGoal::fOnActivate( )
	{
		fExecuteMotionState( );
	}
	void tMotionGoal::fSetMotionState( const char* motionStateName, const Sqrat::Object& motionStateTable )
	{
		sigassert( fOldLogic( ) && fOldLogic( )->fQueryAnimatable( ) );
		mApplyMotionState = fOldLogic( )->fQueryAnimatable( )->fMotionMap( ).fMapState( motionStateName );
		mMotionStateParams = motionStateTable.IsNull( ) ? Sqrat::Object( tScriptVm::fInstance( ).fEmptyTable( ) ) : motionStateTable;
		if_devmenu( mStateName = tStringPtr( motionStateName ) );
	}
	void tMotionGoal::fExecuteMotionState( )
	{
		sigassert( fOldLogic( ) && fOldLogic( )->fQueryAnimatable( ) );
		if_devmenu( fOldLogic( )->fQueryAnimatable( )->fSetCurrentMotionStateName( mStateName ) );
		mApplyMotionState.Execute( mMotionStateParams );
	}
	f32 tMotionGoal::fEvaluateMotionState( )
	{
		sigassert( fOldLogic( ) && fOldLogic( )->fQueryAnimatable( ) );
		if_devmenu( fOldLogic( )->fQueryAnimatable( )->fSetCurrentMotionStateName( mStateName ) );
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
	tTimedMotionGoal::tTimedMotionGoal( const char* motionStateName, const Sqrat::Object& motionStateTable )
		: mTimer( 0.f ), mTimeToLive( 1.f )
	{
		fSetMotionState( motionStateName, motionStateTable );
	}
	void tTimedMotionGoal::fOnActivate( )
	{
		tMotionGoal::fExecuteMotionState( );
		mTimer = 0.f;
	}
	void tTimedMotionGoal::fOnProcess( tGoalPtr& goalPtr, f32 dt )
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
	tOneShotMotionGoal::tOneShotMotionGoal( const char* motionStateName, const Sqrat::Object& motionStateTable )
	{
		fSetMotionState( motionStateName, motionStateTable );
	}
	void tOneShotMotionGoal::fOnActivate( )
	{
		mTimeToLive = tMotionGoal::fEvaluateMotionState( );
		mTimer = 0.f;
	}
	void tOneShotMotionGoal::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tOneShotMotionGoal, tTimedMotionGoal, Sqrat::NoCopy<tOneShotMotionGoal> > classDesc( vm.fSq( ) );
		vm.fNamespace(_SC("AI")).Bind(_SC("OneShotMotionGoal"), classDesc);
	}
}}


