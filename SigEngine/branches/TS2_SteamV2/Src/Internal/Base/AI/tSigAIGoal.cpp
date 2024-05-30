#include "BasePch.hpp"
#include "tSigAIGoal.hpp"
#include "Logic/tAnimatable.hpp"
#include "Logic/tGoalDriven.hpp"

namespace Sig { namespace AI
{

	///
	/// tSigAIGoal
	const char tSigAIGoal::cSigAIPotentialFunction[] = { "PotentialSigAI" };

	u32 tSigAIGoal::gCount = 0;

	tSigAIGoal::tSigAIGoal( )
	{
		fCommonInit( );
	}

	tSigAIGoal::~tSigAIGoal( )
	{
		sigassert( gCount > 0 );
		--gCount;
	}

	void tSigAIGoal::fCommonInit( )
	{
		mParent = NULL;
		mPriority = 0;
		mCurrentChildPriority = 0;
		mSwappedOnExit = false;
		mExitMode = cExitSuspended;
		mIsOneShot = false;
		mIsMotionState = false;
		mPersist = false;
		mWaitForEndEvent = false;
		mReceivedEndEvent = false;
		mTimeToLive = 0;
		mTimer = 0;
		sq_resetobject( &mScriptGoal );
		++gCount;

		sync_line_c( tSync::cSCAI );
	}

	b32 tSigAIGoal::fClearAndPushGoal( u32 maxClear, tGoalPtr& goal, tLogic* logic )
	{
		sync_event_v_c( maxClear, tSync::cSCAI );
		if( fCanClear( maxClear ) )
		{
			tCompositeGoal::fRemoveSubGoals( logic );
			fPushGoal( goal, logic );
			return true;
		}

		return false;
	}

	void tSigAIGoal::fPushGoal( tGoalPtr& goal, tLogic* logic )
	{
		sync_line_c( tSync::cSCAI );
		tSigAIGoal* aiGoal = fToSigAI( goal );
		aiGoal->fSetParent( this );

		tCompositeGoal::fAddImmediateGoal( goal, logic );
	}

	void tSigAIGoal::fSwitchToGoal( tGoalPtr& goal, tLogic* logic )
	{
		sync_line_c( tSync::cSCAI );
		log_assert( fParent( ), "Attempting to switch from goal [" << fDebugTypeName( ) << "] to [" << goal.fDebugTypeName( ) << "], but there is no parent goal (i.e., someone fucked up in SigAI)" );

		// Terminate ourselves
		fSetExitMode( cExitCompleted );
		tGoalPtr myGoal = fParent( )->fFindMe( this );
		myGoal.fSuspendIfActive( logic );
		fParent( )->fGoalStack( ).fFindAndEraseOrdered( myGoal );

		// Push new goal onto parent's stack
		fParent( )->fPushGoal( goal, logic );
	}

	void tSigAIGoal::fOnActivate( tLogic* logic )
	{
		if( mIsMotionState )
		{
			if( mIsOneShot )
			{
				mTimeToLive = fEvaluateMotionState( logic );
				if( mTimeToLive != mTimeToLive )
				{
					if_devmenu( log_warning( Log::cFlagAnimation, "No value was returned from MoMap state [" << mStateName << "]" ); )
					mTimeToLive = 0.0f;
				}
				mTimer = 0.f;
			}
			else
			{
				fExecuteMotionState( logic );
			}
		}

		if( !fHasSubGoals( ) && !sq_isnull( mScriptGoal ) )
		{
			tGoalPtr goalPtr = tGoalPtr( Sqrat::Object( mScriptGoal ), 1.f );
			fProcessPotentialGoals( goalPtr, logic, 0.f );
		}

		if( fHasSubGoals( ) )
		{
			tGoalPtr& currentGoal = tCompositeGoal::fCurrentGoal( );
			if( currentGoal.fActivateIfNotActive( logic ) )
				mCurrentChildPriority = fCurrentGoal( )->fPriority( );
		}
	}

	void tSigAIGoal::fOnProcess( tGoalPtr& goalPtr, tLogic* logic, f32 dt )
	{
		if( mIsOneShot )
		{
			if( mWaitForEndEvent )
			{
				if( mReceivedEndEvent )
				{
					mReceivedEndEvent = false;
					fMarkAsComplete( );
				}
			}
			else if( mTimer >= mTimeToLive )
				fMarkAsComplete( );
			else
				mTimer += dt;
		}

		fProcessSubGoals( goalPtr, logic, dt );
	}

	void tSigAIGoal::fProcessSubGoals( tGoalPtr& goalPtr, tLogic* logic, f32 dt )
	{
		// remove all completed goals, s32 due to the --
		for( s32 i = 0; i < (s32)fGoalStack( ).fCount( ); ++i )
		{
			//b32 onTop = (i == (s32)fGoalStack( ).fCount( ) - 1);
			tGoalPtr gp = fGoalStack( )[ i ];
			tSigAIGoal *goal = fToSigAI( gp );

			const tStatus status = goal->fStatus( );
			if( status == cStatusComplete || status == cStatusTerminated )
			{
				if( status != cStatusTerminated ) 
					goal->fSetExitMode( cExitCompleted );
				gp.fSuspendIfActive( logic ); //may cause the top goal to be swapped with another
				
				if( fGoalStack( ).fFindAndEraseOrdered( gp ) ) i--;
			}
		}

		if( fHasSubGoals( ) )
		{
			mCurrentChildPriority = fCurrentGoal( )->fPriority( );
		}
		else
		{
			mCurrentChildPriority = 0;

			if( mSuspendAfterSubGoals )
			{
				mStatus = cStatusComplete;
				mSuspendAfterSubGoals = false;
				return;
			}
		}

		// we're out of sub-goals, go through potentials and see if they want a turn
		if( !fHasSubGoals( ) )
			fProcessPotentialGoals( goalPtr, logic, dt );

		// ensure that any goals that are no longer at the top of the goal stack get suspended if necessary (BEFORE activating current goal)
		for( s32 i = fGoalStack( ).fCount( ) - 2; i >= 0; --i )
			fGoalStack( )[ i ].fSuspendIfActive( logic );

		// process sub-goals
		if( fHasSubGoals( ) )
		{
			tGoalPtr currentGoal = tCompositeGoal::fCurrentGoal( );
			sigassert( currentGoal.fGoal( )->fStatus( ) != cStatusTerminated );

			mCurrentChildPriority = fCurrentGoal( )->fPriority( );
			currentGoal.fActivateIfNotActive( logic );
			currentGoal.fGoal( )->fProcess( currentGoal, logic, dt );
			//currentGoal may very well not be the current goal anymore
		}
		else if( !mIsMotionState && !mIsOneShot && !mPersist )
			mStatus = cStatusComplete; // if still no sub-goals, then we're done (if not a motion goal, or one shot overridden )
	}

	void tSigAIGoal::fProcessPotentialGoals( tGoalPtr& goalPtr, tLogic* logic, f32 dt )
	{
		if( !goalPtr.fScriptObject( ).IsNull( ) )
		{
			Sqrat::Function f( goalPtr.fScriptObject( ), cSigAIPotentialFunction );
			if( !f.IsNull( ) )
				f.Execute( fGetLogicForScript( logic ) );
		}
		else
			fCheckAndPushPotentialGoals( fGetLogicForScript( logic ) );
	}

	b32 tSigAIGoal::fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e )
	{
		if( fActive( ) && mIsMotionState && mIsOneShot && mWaitForEndEvent )
		{
			if( tAnimatedSkeleton::fIsReachedEndOfOneShotEvent( e ) )
				mReceivedEndEvent = true;
		}

		if( fHasSubGoals( ) )
		{
			for( s32 i = fGoalStack( ).fCount( ) - 1; i >= 0; --i )
				if( fGoalStack( )[ i ].fHandleLogicEvent( logic, e ) ) 
					return true;
		}
		return false;
	}

	void tSigAIGoal::fOnDelete( )
	{
		for( u32 i = 0; i < mGoalStack.fCount( ); ++i )
			mGoalStack[ i ].fOnDelete( );
		mGoalStack.fSetCount( 0 );
		//This does not call into tCompositeGoal on purpose.
		// tSigAIGoal is only supposed to use mGoalStack by design and none of the other goal lists.
	}

	void tSigAIGoal::fTerminateGoalsNamed( tLogic* logic, const char* name )
	{
		tStringPtr nameP( name );
		fToSigAI( logic->fQueryGoalDriven( )->fMasterGoal( ) )->fTerminateGoalsNamedRecursive( nameP );
	}

	void tSigAIGoal::fTerminateGoalsNamedRecursive( const tStringPtr& name )
	{
		for( u32 i = 0; i < fGoalStack( ).fCount( ); ++i )
		{
			tGoalPtr& goalPtr = fGoalStack( )[ i ];
			tSigAIGoal* goal = fToSigAI( goalPtr );
			goal->fTerminateGoalsNamedRecursive( name );
			if( goal->mEditorName == name )
				goal->fTerminate( );
		}
	}

	b32 tSigAIGoal::fCanClear( u32 maxClear ) const
	{
		if( fHasSubGoals( ) )
		{
			tSigAIGoal* goal = fCurrentGoal( );
			if( goal->fPriority( ) > maxClear )
				return false;
		}

		return true;
	}

	tGoalPtr& tSigAIGoal::fFindMe( tSigAIGoal* goal )
	{
		tGoalPtrList& stack = fGoalStack( );
		for( u32 i = 0; i < stack.fCount( ); ++i )
			if( stack[ i ].fGoal( ) == goal )
				return stack[ i ];

		log_assert( false, "Could not find goal in tSigAIGoal" );
		static tGoalPtr p;
		return p;
	}
	void tSigAIGoal::fClearMotionState( )
	{
		mIsMotionState = false;
	}
	void tSigAIGoal::fSetMotionState( tLogic* logic, const char* motionStateName, const Sqrat::Object& motionStateTable, bool oneShot )
	{
		sigassert( logic && logic->fQueryAnimatable( ) );
		mIsOneShot = oneShot;
		mIsMotionState = true;
		mApplyMotionState = motionStateName ? logic->fQueryAnimatable( )->fMotionMap( ).fMapState( motionStateName ) : Sqrat::Function( );
		mMotionStateParams = motionStateTable.IsNull( ) ? Sqrat::Object( tScriptVm::fInstance( ).fEmptyTable( ) ) : motionStateTable;
		if_devmenu( mStateName = tStringPtr( motionStateName ) );
	}
	void tSigAIGoal::fExecuteMotionState( tLogic* logic )
	{
		sigassert( logic );
		Logic::tAnimatable* animatable = logic->fQueryAnimatable( );
		sigassert( animatable );

		if( mWaitForEndEvent )
			animatable->fClearResponseToEndEvent( );

		if_devmenu( animatable->fSetCurrentMotionStateName( mStateName ) );
		if( !mApplyMotionState.IsNull( ) )
			mApplyMotionState.Execute( mMotionStateParams );

		if_devmenu( sync_event_c( mStateName.fCStr( ), 0, tSync::cSCAI ); )
	}
	f32 tSigAIGoal::fEvaluateMotionState( tLogic* logic )
	{
		sigassert( logic );
		Logic::tAnimatable* animatable = logic->fQueryAnimatable( );
		sigassert( animatable );

		if( mWaitForEndEvent )
			animatable->fClearResponseToEndEvent( );

		if_devmenu( animatable->fSetCurrentMotionStateName( mStateName ) );
		f32 result = mApplyMotionState.IsNull( ) ? 0.f : mApplyMotionState.Evaluate<f32>( mMotionStateParams );
		if_devmenu( sync_event_c( mStateName.fCStr( ), 0, tSync::cSCAI ); )
		sync_event_v_c( result, tSync::cSCAI );
		return result;
	}
	void tSigAIGoal::fSetOneShotTimer( f32 timeToLive )
	{
		mIsOneShot = true;
		mTimeToLive = timeToLive;
		sigassert( mTimeToLive == mTimeToLive );
		mTimer = 0.f;
	}
	void tSigAIGoal::fAddOneShotTime( f32 time )
	{
		mTimeToLive += time;
	}
	void tSigAIGoal::fSetToWaitForEndEvent( b32 waitForEndEvent )
	{
		mWaitForEndEvent = waitForEndEvent;
	}
	void tSigAIGoal::fSetup( u32 priority, const char* editorName, Sqrat::Object& obj )
	{
		mPriority = priority;
		mEditorName = tStringPtr( editorName );
		mScriptGoal = obj.GetObject( );
	}

}}


//--------------------------------------------------------------------------------------------------------------
//
//    Script-Specific Implementation
//
//--------------------------------------------------------------------------------------------------------------
namespace Sig { namespace AI
{
	namespace
	{
		static void fClearAndPushGoal( tSigAIGoal* goal, u32 maxClear, const Sqrat::Object& obj, tLogic *logic )
		{
			tGoalPtr goalPtr = tGoalPtr( obj, 1.f );
			goal->fClearAndPushGoal( maxClear, goalPtr, logic );
		}
		static void fPushGoal(	tSigAIGoal* goal, const Sqrat::Object& obj, tLogic *logic )
		{
			tGoalPtr goalPtr = tGoalPtr( obj, 1.f );
			goal->fPushGoal( goalPtr, logic );
		}
		static void fSwitchToGoal(	tSigAIGoal* goal, const Sqrat::Object& obj, tLogic *logic )
		{
			tGoalPtr goalPtr = tGoalPtr( obj, 1.f );
			goal->fSwitchToGoal( goalPtr, logic );
		}
	}
	void tSigAIGoal::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tSigAIOnProcess, Sqrat::DefaultAllocator<tSigAIOnProcess> > classDesc( vm.fSq( ) );
			vm.fNamespace(_SC("AI")).Bind(_SC("SigAIOnProcess"), classDesc);
		}

		Sqrat::DerivedClass<tSigAIGoal, tCompositeGoal, Sqrat::NoCopy<tSigAIGoal> > classDesc( vm.fSq( ) );

		classDesc
			.GlobalFunc(_SC("ClearAndPushGoal"),	&::Sig::AI::fClearAndPushGoal)
			.GlobalFunc(_SC("PushGoal"),			&::Sig::AI::fPushGoal)
			.GlobalFunc(_SC("SwitchToGoal"),		&::Sig::AI::fSwitchToGoal)
			.Prop(_SC("ExitMode"),					&tSigAIGoal::fExitMode)
			.Func(_SC("SetMotionState"),			&tSigAIGoal::fSetMotionState)
			.Func(_SC("SetOneShotTimer"),			&tSigAIGoal::fSetOneShotTimer)
			.Func(_SC("AddOneShotTime"),			&tSigAIGoal::fAddOneShotTime)
			.Func(_SC("SetToWaitForEndEvent"),		&tSigAIGoal::fSetToWaitForEndEvent)
			.Func(_SC("ChildPriority"),				&tSigAIGoal::fChildPriority)
			.Func(_SC("Setup"),						&tSigAIGoal::fSetup)
			.Func(_SC("TerminateGoalsNamed"),		&tSigAIGoal::fTerminateGoalsNamed)
			.Func(_SC("SetOnProcess"),				&tSigAIGoal::fSetOnProcess)
			.Func(_SC(cSigAIPotentialFunction),		&tSigAIGoal::fCheckAndPushPotentialGoals)
			.Func(_SC("Persist"),					&tSigAIGoal::fPersist)
			;
 
		vm.fNamespace(_SC("AI")).Bind(_SC("SigAIGoal"), classDesc);

		vm.fConstTable( ).Const( _SC("SIGAIGOAL_EXIT_HANDLED"), int( tSigAIGoal::cExitHandled ) );
		vm.fConstTable( ).Const( _SC("SIGAIGOAL_EXIT_SUSPENDED"), int( tSigAIGoal::cExitSuspended ) );
		vm.fConstTable( ).Const( _SC("SIGAIGOAL_EXIT_COMPLETED"), int( tSigAIGoal::cExitCompleted ) );
	}
}}

