#include "BasePch.hpp"
#include "tGoalDriven.hpp"
#include "tProfiler.hpp"

// For terminating goals
#include "Ai/tSigAIGoal.hpp"



namespace Sig { namespace Logic
{
	tGoalDriven::tGoalDriven( )
		: mEnabled( true )
	{
		mData.fSetEventHandler( this );
	}
	tGoalDriven::~tGoalDriven( )
	{
		fOnDelete( );
	}
	void tGoalDriven::fOnDelete( )
	{
		fClearGoals( );
		tLogic::fOnDelete( );
	}
	void tGoalDriven::fSetEnabled( b32 enabled )
	{
		mEnabled = enabled;
	}
	void tGoalDriven::fClearGoals( )
	{
		mMasterGoal.fOnDelete( );
		mMasterGoal = AI::tGoalPtr( );
		for( u32 i = 0; i < mAdditionalGoals.fCount(); ++i )
			mAdditionalGoals[ i ].fOnDelete( );
		mAdditionalGoals.fSetCount( 0 );
	}
	void tGoalDriven::fOnSpawn( )
	{
		fOnPause( false );

		AI::tGoal* masterGoal = mMasterGoal.fGoal( );
		if( masterGoal )
			mMasterGoal.fActivateIfNotActive( ); 

		for( u32 i = 0; i < mAdditionalGoals.fCount( ); ++i )
			mAdditionalGoals[ i ].fActivateIfNotActive( ); 
	}
	void tGoalDriven::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
		}
		else
		{
			fRunListInsert( cRunListActST );
		}
	}
	void tGoalDriven::fActST( f32 dt )
	{
		if( !mEnabled )
			return;

		AI::tGoal* masterGoal = mMasterGoal.fGoal( );
		if( masterGoal )
		{
			mMasterGoal.fActivateIfNotActive( ); 
			masterGoal->fProcess( mMasterGoal, dt );
		}

		for( u32 i = 0; i < mAdditionalGoals.fCount( ); ++i )
		{
			AI::tGoal* goal = mAdditionalGoals[ i ].fGoal( );
			mAdditionalGoals[ i ].fActivateIfNotActive( ); 
			goal->fProcess( mAdditionalGoals[ i ], dt );
		}
	}
	b32 tGoalDriven::fHandleLogicEvent( const Logic::tEvent& e )
	{
		profile( cProfilePerfScriptEventHandlers );
		if( mMasterGoal.fHandleLogicEvent( e ) )
			return true;

		for( u32 i = 0; i < mAdditionalGoals.fCount( ); ++i )
			if( mAdditionalGoals[ i ].fHandleLogicEvent( e ) )
				return true;

		return false;
	}
	void tGoalDriven::fSetScriptGoalObject( const Sqrat::Object& obj )
	{
		mMasterGoal.fOnDelete( );
		mMasterGoal = AI::tGoalPtr( obj, 1.f );
		mMasterGoal.fOnInsertion( this );

		// warm start any potential goals.
		fActST( 0.001f );
	}
	void tGoalDriven::fAddAdditionalGoalFromScript( const Sqrat::Object& obj )
	{
		mAdditionalGoals.fPushBack( AI::tGoalPtr( obj, 1.f ) );
		mAdditionalGoals.fBack( ).fOnInsertion( this );

		// warm start any potential goals.
		fActST( 0.001f );
	}
	void tGoalDriven::fTerminateGoalsNamedRecursive( const tStringPtr& name )
	{
		AI::tSigAIGoal* goal = mMasterGoal.fGoal( ) ? mMasterGoal.fGoal( )->fDynamicCast<AI::tSigAIGoal>( ) : NULL;
		if( goal )
			goal->fTerminateGoalsNamedRecursive( name );

		for( u32 i = 0; i < mAdditionalGoals.fCount( ); ++i )
		{
			sigassert( mAdditionalGoals[ i ].fGoal( ) ); //should be set if in this list.
			goal = mAdditionalGoals[ i ].fGoal( )->fDynamicCast<AI::tSigAIGoal>( );
			if( goal )
				goal->fTerminateGoalsNamedRecursive( name );
		}
	}
	void tGoalDriven::fCompleteGoalsNamedRecursive( const tStringPtr & name )
	{
		AI::tSigAIGoal* goal = mMasterGoal.fGoal( ) ? mMasterGoal.fGoal( )->fDynamicCast<AI::tSigAIGoal>( ) : NULL;
		if( goal )
			goal->fCompleteGoalsNamedRecursive( name );

		for( u32 i = 0; i < mAdditionalGoals.fCount( ); ++i )
		{
			sigassert( mAdditionalGoals[ i ].fGoal( ) ); //should be set if in this list.
			goal = mAdditionalGoals[ i ].fGoal( )->fDynamicCast<AI::tSigAIGoal>( );
			if( goal )
				goal->fCompleteGoalsNamedRecursive( name );
		}
	}
	tStringPtr tGoalDriven::fMapMotionState( const tStringPtr& key )
	{
		tStringPtr* result = mMotionStateMap.fFind( key );
		return result ? *result : key;
	}
	void tGoalDriven::fSetMapMotionState( const tStringPtr& key, const tStringPtr& override )
	{
		mMotionStateMap[ key ] = override;
	}
	void tGoalDriven::fClearMapMotionStates( )
	{
		mMotionStateMap.fClear( );
	}

#ifdef sig_devmenu
	void tGoalDriven::fAddWorldDebugText( std::stringstream& ss ) const
	{
		mMasterGoal.fAddWorldDebugText( ss, 0 );
		for( u32 i = 0; i < mAdditionalGoals.fCount( ); ++i )
			mAdditionalGoals[ i ].fAddWorldDebugText( ss, 0 );
	}
#endif

	void tGoalDriven::fExportScriptInterface( tScriptVm& vm )
	{
		AI::tAIData::fExportScriptInterface( vm );

		Sqrat::DerivedClass<tGoalDriven, tLogic, Sqrat::NoConstructor > classDesc( vm.fSq( ) );

		classDesc
			.Prop(_SC("MasterGoal"),			&tGoalDriven::fScriptGoalObject, &tGoalDriven::fSetScriptGoalObject)
			.Func(_SC("AddGoal"),				&tGoalDriven::fAddAdditionalGoalFromScript)
			.Var(_SC("AIData"),					&tGoalDriven::mData)
			.Func(_SC("MapMotionState"),		&tGoalDriven::fMapMotionState)
			.Func(_SC("SetMapMotionState"),		&tGoalDriven::fSetMapMotionState)
			.Func(_SC("ClearMapMotionStates"),	&tGoalDriven::fClearMapMotionStates)
			;

		vm.fRootTable( ).Bind(_SC("GoalDriven"), classDesc);
	}
}}
