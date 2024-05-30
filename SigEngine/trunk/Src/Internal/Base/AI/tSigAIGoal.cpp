#include "BasePch.hpp"
#include "tSigAIGoal.hpp"
#include "Logic/tAnimatable.hpp"
#include "Logic/tGoalDriven.hpp"

namespace Sig { namespace AI
{
	namespace
	{
		const f32 cNoTimerOverride = -1.0f;
	}

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
		mReceivedEndEvent = false;
		mTimeToLive = 0;
		mTimer = 0;
		sq_resetobject( &mScriptGoal );
		++gCount;

		sync_line_c( tSync::cSCAI );
	}

	b32 tSigAIGoal::fClearAndPushGoal( u32 maxClear, const tGoalPtr& goal )
	{
		sync_event_v_c( maxClear, tSync::cSCAI );
		if( fCanClear( maxClear ) )
		{
			tCompositeGoal::fRemoveSubGoals( );
			fPushGoal( goal );
			return true;
		}

		return false;
	}

	void tSigAIGoal::fPushGoal( const tGoalPtr& goal )
	{
		sync_line_c( tSync::cSCAI );
		tSigAIGoal* aiGoal = fToSigAI( goal );
		aiGoal->fSetParent( this );

		tCompositeGoal::fAddImmediateGoal( goal );

		mCurrentChildPriority = fCurrentGoal( )->fPriority( );
	}

	void tSigAIGoal::fSwitchToGoal( tGoalPtr& goal )
	{
		sync_line_c( tSync::cSCAI );
		log_assert( fParent( ), "Attempting to switch from goal [" << fScriptDebugTypeName( ) << "] to [" << goal.fDebugTypeName( ) << "], but there is no parent goal (i.e., someone fucked up in SigAI)" );

		// Terminate ourselves
		fSetExitMode( cExitCompleted );
		tGoalPtr myGoal = fParent( )->fFindMe( this );
		myGoal.fSuspendIfActive( );
		fParent( )->fGoalStack( ).fFindAndEraseOrdered( myGoal );

		// Push new goal onto parent's stack
		fParent( )->fPushGoal( goal );
	}

	void tSigAIGoal::fPushBelow( u32 priority, const tGoalPtr& goal )
	{
		sync_line_c( tSync::cSCAI );
		tSigAIGoal* aiGoal = fToSigAI( goal );
		aiGoal->fSetParent( this );

		tGoalPtrList& goalList = fGoalStack( );
		u32 insertPt = 0;
		for( insertPt = 0; insertPt < goalList.fCount( ); ++insertPt )
			if( fToSigAI( goalList[ insertPt ] )->fPriority( ) >= priority )
				break;

		tCompositeGoal::fInsertGoal( goal, insertPt );

		mCurrentChildPriority = fCurrentGoal( )->fPriority( );
	}

	void tSigAIGoal::fOnActivate( )
	{
		//sigassert( sq_isnull( mOnTick.GetEnv( ) ) && "Double fOnActivate called. We are in the shit."  );
		mOnTick = Sqrat::Function( Sqrat::Object( mScriptGoal ), "OnTick" );

		// Motion stuff
		if( mBlendReferences.fCount( ) )
		{
			fAddMotionRefs( true );
			mPersist = true;
		}
		else if( mIsMotionState )
		{
			if( mIsOneShot )
			{
				mTimeToLive = fEvaluateMotionState( );
				if( mTimeToLive != mTimeToLive )
				{
					log_warning( "No value was returned from MoMap state [" << mApplyMotionStateName << "]" );
					mTimeToLive = 0.0f;
				}
				mTimer = 0.f;
			}
			else
			{
				fExecuteMotionState( );
			}
		}

		if( !fHasSubGoals( ) && !sq_isnull( mScriptGoal ) )
		{
			tGoalPtr goalPtr = tGoalPtr( Sqrat::Object( mScriptGoal ), 1.f );
			fProcessPotentialGoals( goalPtr, 0.f );
		}

		if( fHasSubGoals( ) )
		{
			tGoalPtr& currentGoal = tCompositeGoal::fCurrentGoal( );
			if( currentGoal.fActivateIfNotActive( ) )
				mCurrentChildPriority = fCurrentGoal( )->fPriority( );
		}
	}

	void tSigAIGoal::fOnSuspend( )
	{
		mOnTick.Release( );

		if( mBlendReferences.fCount( ) )
			fAddMotionRefs( false );

		tCompositeGoal::fOnSuspend( );
	}

	void tSigAIGoal::fOnProcess( tGoalPtr& goalPtr, f32 dt )
	{
		if( mIsOneShot )
		{
			if( mTimer >= mTimeToLive )
				fMarkAsComplete( );
			else
				mTimer += dt;
		}

		fProcessSubGoals( goalPtr, dt );

		if( !mOnTick.IsNull( ) )
			mOnTick.Execute( );
	}

	void tSigAIGoal::fProcessSubGoals( tGoalPtr& goalPtr, f32 dt )
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
				gp.fSuspendIfActive( ); //may cause the top goal to be swapped with another
				
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
			fProcessPotentialGoals( goalPtr, dt );

		// ensure that any goals that are no longer at the top of the goal stack get suspended if necessary (BEFORE activating current goal)
		for( s32 i = fGoalStack( ).fCount( ) - 2; i >= 0; --i )
			fGoalStack( )[ i ].fSuspendIfActive( );

		// process sub-goals
		if( fHasSubGoals( ) )
		{
			tGoalPtr currentGoal = tCompositeGoal::fCurrentGoal( );
			sigassert( currentGoal.fGoal( )->fStatus( ) != cStatusTerminated );

			mCurrentChildPriority = fCurrentGoal( )->fPriority( );
			currentGoal.fActivateIfNotActive( );
			currentGoal.fGoal( )->fProcess( currentGoal, dt );
			//currentGoal may very well not be the current goal anymore
		}
		else if( !mIsMotionState && !mIsOneShot && !mPersist )
			mStatus = cStatusComplete; // if still no sub-goals, then we're done (if not a motion goal, or one shot overridden )
	}

	void tSigAIGoal::fProcessPotentialGoals( tGoalPtr& goalPtr, f32 dt )
	{
		if( !goalPtr.fScriptObject( ).IsNull( ) )
		{
			Sqrat::Function f( goalPtr.fScriptObject( ), cSigAIPotentialFunction );
			if( !f.IsNull( ) )
				f.Execute( );
		}
		else
			fCheckAndPushPotentialGoals( tGoal::fGetLogicForScript( ) );
	}

	b32 tSigAIGoal::fHandleLogicEvent( const Logic::tEvent& e )
	{
		if( fHasSubGoals( ) )
		{
			for( s32 i = fGoalStack( ).fCount( ) - 1; i >= 0; --i )
				if( fGoalStack( )[ i ].fHandleLogicEvent( e ) ) 
					return true;
		}
		return false;
	}

	void tSigAIGoal::fOnDelete( )
	{
		for( u32 i = 0; i < mGoalStack.fCount( ); ++i )
			mGoalStack[ i ].fOnDelete( );
		mGoalStack.fSetCount( 0 );
		tCompositeGoal::fOnDelete( ); // tSigAIGoal "shouldn't" have anything in these lists so this "should" be a noop -- but fuck that as rationale for not calling it, I'm feeling paranoid and these leaks make no sense.
		mOnTick = Sqrat::Function( );
		mMotionStateParams = Sqrat::Object( );
		mScriptGoal = HSQOBJECT( );
	}

	void tSigAIGoal::fTerminateGoalsNamedFromScript( const char* name )
	{
		tStringPtr nameP( name );
		fGoalDriven( )->fTerminateGoalsNamedRecursive( nameP );
	}

	void tSigAIGoal::fCompleteGoalsNamedFromScript( const char* name )
	{
		tStringPtr nameP( name );
		fGoalDriven( )->fCompleteGoalsNamedRecursive( nameP );
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

	void tSigAIGoal::fCompleteGoalsNamedRecursive( const tStringPtr& name )
	{
		for( u32 i = 0; i < fGoalStack( ).fCount( ); ++i )
		{
			tGoalPtr& goalPtr = fGoalStack( )[ i ];
			tSigAIGoal* goal = fToSigAI( goalPtr );
			goal->fCompleteGoalsNamedRecursive( name );
			if( goal->mEditorName == name )
				goal->fMarkAsComplete( );
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
	void tSigAIGoal::fSetMotionState( const char* motionStateName, const Sqrat::Object& motionStateTable, bool oneShot )
	{
		mIsOneShot = oneShot;
		mIsMotionState = true;
		mApplyMotionStateName = tStringPtr( motionStateName );
		mMotionStateParams = motionStateTable.IsNull( ) ? Sqrat::Object( tScriptVm::fInstance( ).fEmptyTable( ) ) : motionStateTable;
	}
	Sqrat::Function tSigAIGoal::fFindMoStateFunction( )
	{
		sigassert( fOldLogic( ) );
		Logic::tAnimatable* animatable = fOldLogic( )->fQueryAnimatable( );
		sigassert( animatable );

		if( animatable->fSigAnimMotionMap( ) )
		{
			log_warning_nospam( "Tried to find a motion state on an Animatable who is using a tSigAnimMomap. This call is ignored! moState: " << mApplyMotionStateName );
		}
		else
		{
			if( mApplyMotionStateName.fLength( ) )
			{
				if_devmenu( animatable->fSetCurrentMotionStateName( mApplyMotionStateName ) );
				return animatable->fMotionMap( ).fMapState( mApplyMotionStateName.fCStr( ) );
			}
		}

		return Sqrat::Function( );
	}
	const char* tSigAIGoal::fScriptDebugTypeName( ) const
	{
		const tGoalPtr goalPtr( Sqrat::Object( mScriptGoal ), 1.0f );
		return goalPtr.fDebugTypeName( );
	}
	void tSigAIGoal::fExecuteMotionState( )
	{
		Sqrat::Function moState = fFindMoStateFunction( );
		if( !moState.IsNull( ) )
			moState.Execute( mMotionStateParams );

		if_devmenu( sync_event_c( mStateName.fCStr( ), 0, tSync::cSCAI ); )
	}
	f32 tSigAIGoal::fEvaluateMotionState( )
	{
		Sqrat::Function moState = fFindMoStateFunction( );
		f32 result = moState.IsNull( ) ? 0.f : moState.Evaluate<f32>( mMotionStateParams );
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
	void tSigAIGoal::fSetup( u32 priority, const char* editorName, Sqrat::Object& obj )
	{
		mPriority = priority;
		mEditorName = tStringPtr( editorName );
		mScriptGoal = obj.GetObject( );
	}
	void tSigAIGoal::fAddMoStateRef( const char* moStateName )
	{
		sigassert( fOldLogic( ) );
		Logic::tAnimatable* animatable = fOldLogic( )->fQueryAnimatable( );
		sigassert( animatable );
		sigassert( animatable->fSigAnimMotionMap( ) && "You are using the new anim system but have not specified a new momap/animap pair in the logic." );

		Anim::tSigAnimMoMap::tBlendData* blend = animatable->fSigAnimMotionMap( )->fFindBlendData( tStringPtr( moStateName ) );
		log_assert( blend, "Could not find blend named: " << moStateName );

		sigassert( !mBlendReferences.fFind( blend ) );
		mBlendReferences.fPushBack( blend );
	}
	void tSigAIGoal::fAddMotionRefs( b32 add )
	{
		if( add )
		{
			for( u32 i = 0; i < mBlendReferences.fCount( ); ++i )
				mBlendReferences[ i ]->fAddReference( this );
		}
		else
		{
			for( u32 i = 0; i < mBlendReferences.fCount( ); ++i )
				mBlendReferences[ i ]->fRemoveReference( this );
		}
	}
	void tSigAIGoal::fAnimEnded( )
	{
		fMarkAsComplete( );
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
			goal->fClearAndPushGoal( maxClear, goalPtr );
		}
		static void fPushGoal( tSigAIGoal* goal, const Sqrat::Object& obj, tLogic *logic )
		{
			tGoalPtr goalPtr = tGoalPtr( obj, 1.f );
			goal->fPushGoal( goalPtr );
		}
		static void fSwitchToGoal(tSigAIGoal* goal, const Sqrat::Object& obj, tLogic *logic )
		{
			tGoalPtr goalPtr = tGoalPtr( obj, 1.f );
			goal->fSwitchToGoal( goalPtr );
		}
		static void fPushBelow(	tSigAIGoal* goal, u32 priority, const Sqrat::Object& obj, tLogic *logic )
		{
			tGoalPtr goalPtr = tGoalPtr( obj, 1.f );
			goal->fPushBelow( priority, goalPtr );
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
			.GlobalFunc(_SC("PushBelow"),			&::Sig::AI::fPushBelow)
			.Prop(_SC("ExitMode"),					&tSigAIGoal::fExitMode)
			.Func(_SC("SetMotionState"),			&tSigAIGoal::fSetMotionState)
			.Func(_SC("SetOneShotTimer"),			&tSigAIGoal::fSetOneShotTimer)
			.Func(_SC("AddOneShotTime"),			&tSigAIGoal::fAddOneShotTime)
			.Func(_SC("ChildPriority"),				&tSigAIGoal::fChildPriority)
			.Func(_SC("Setup"),						&tSigAIGoal::fSetup)
			.Func(_SC("TerminateGoalsNamed"),		&tSigAIGoal::fTerminateGoalsNamedFromScript)
			.Func(_SC("CompleteGoalsNamed"),		&tSigAIGoal::fCompleteGoalsNamedFromScript)
			.Func(_SC("SetOnProcess"),				&tSigAIGoal::fSetOnProcess)
			.Func(_SC(cSigAIPotentialFunction),		&tSigAIGoal::fCheckAndPushPotentialGoals)
			.Func(_SC("Persist"),					&tSigAIGoal::fPersist)
			.Func(_SC("AddMoStateRef"),				&tSigAIGoal::fAddMoStateRef)
			.Prop(_SC("ElapsedTime"),				&tSigAIGoal::fElapsedTime)
			.Prop(_SC("TotalTime"),					&tSigAIGoal::fTotalTime)
			;
 
		vm.fNamespace(_SC("AI")).Bind(_SC("SigAIGoal"), classDesc);

		vm.fConstTable( ).Const( _SC("SIGAIGOAL_EXIT_HANDLED"), int( tSigAIGoal::cExitHandled ) );
		vm.fConstTable( ).Const( _SC("SIGAIGOAL_EXIT_SUSPENDED"), int( tSigAIGoal::cExitSuspended ) );
		vm.fConstTable( ).Const( _SC("SIGAIGOAL_EXIT_COMPLETED"), int( tSigAIGoal::cExitCompleted ) );
		vm.fConstTable( ).Const( _SC("SIGAIGOAL_NO_TIMER_OVERRIDE"), float( cNoTimerOverride ) );
	}
}}

