#include "BasePch.hpp"
#include "tGoalDriven.hpp"
#include "tProfiler.hpp"



namespace Sig { namespace Logic
{
	tGoalDriven::tGoalDriven( )
		: mLogic( 0 )
	{
	}
	void tGoalDriven::fOnDelete( tLogic* logic )
	{
		fClearGoals( logic );
		//mMasterGoal.fOnDelete( ); //Un comment this if you ever need to clean up entity ptrs. or cyclic goal references. not needed as of now.
	}
	void tGoalDriven::fClearGoals( tLogic* logic )
	{
		mMasterGoal.fSuspendIfActive( logic );
		mMasterGoal = AI::tGoalPtr( );
	}
	void tGoalDriven::fOnSpawn( tLogic* logic )
	{
		AI::tGoal* masterGoal = mMasterGoal.fGoal( );
		if( masterGoal )
			mMasterGoal.fActivateIfNotActive( logic ); 
	}
	void tGoalDriven::fActST( tLogic* logic, f32 dt )
	{
		AI::tGoal* masterGoal = mMasterGoal.fGoal( );
		if( masterGoal )
		{
			mMasterGoal.fActivateIfNotActive( logic ); 
			masterGoal->fProcess( mMasterGoal, logic, dt );
		}
	}
	b32 tGoalDriven::fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e )
	{
		profile( cProfilePerfScriptEventHandlers );
		return mMasterGoal.fHandleLogicEvent( logic, e );
	}
	void tGoalDriven::fSetScriptGoalObject( const Sqrat::Object& obj )
	{
		mMasterGoal = AI::tGoalPtr( obj, 1.f );
		if( mLogic )
			mMasterGoal.fOnInsertion( mLogic );
	}
	void tGoalDriven::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tGoalDriven, Sqrat::NoConstructor > classDesc( vm.fSq( ) );

		classDesc
			.Prop(_SC("MasterGoal"), &tGoalDriven::fScriptGoalObject, &tGoalDriven::fSetScriptGoalObject)
			.Func(_SC("OnSpawn"), &tGoalDriven::fOnSpawn)
			;

		vm.fRootTable( ).Bind(_SC("GoalDriven"), classDesc);
	}
}}
