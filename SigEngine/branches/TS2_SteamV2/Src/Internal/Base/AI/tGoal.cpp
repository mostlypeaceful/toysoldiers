#include "BasePch.hpp"
#include "tGoal.hpp"
#include "tSigAIGoal.hpp"
#include "Logic/tAnimatable.hpp"
#include "Logic/tGoalDriven.hpp"

//--------------------------------------------------------------------------------------------------------------
//
//    tGoal
//
//--------------------------------------------------------------------------------------------------------------
namespace Sig { namespace AI
{
	tGoal::tGoal( ) 
		: mStatus( cStatusInactive )
		, mTriggerCheckFrequency( Math::cInfinity )// will never trigger unless overriden
		, mTriggerCheckElapsed( 0.f )
	{
	}
	f32 tGoal::fComputePriority( tLogic* logic )
	{
		return 1.f;
	}
	void tGoal::fProcess( tGoalPtr& goalPtr, tLogic* logic, f32 dt )
	{
		fOnProcess( goalPtr, logic, dt );
	}
}}

//--------------------------------------------------------------------------------------------------------------
//
//    tGoalPtr
//
//--------------------------------------------------------------------------------------------------------------
namespace Sig { namespace AI
{

	const char* tGoalPtr::fDebugTypeName( ) const
	{
		if( fIsNull( ) )
			return "(null)";
		else if( fIsCodeOwned( ) )
			return fGoal( )->fDebugTypeName( );
		else
		{
			Sqrat::Function f( fScriptObject( ), "DebugTypeName" );
			sigassert( !f.IsNull( ) );
			return f.Evaluate<const char*>( );
		}
	}
#ifdef sig_devmenu
	void fPushIndent( std::stringstream& ss, u32 indent )
	{
		const u32 spacing = 3;
		for( u32 i = 0; i < spacing * indent; ++i )
			ss << " ";
	}

	b32 tGoalPtr::fAddWorldDebugText( std::stringstream& ss, u32 indent ) const
	{
		if( fIsNull( ) )
			return false;

		tCompositeGoal* compositeGoal = fGoal( )->fDynamicCast< tCompositeGoal >( );
		tSigAIGoal* aiGoal = fGoal( )->fDynamicCast< tSigAIGoal >( );

		fPushIndent( ss, indent );

		if( aiGoal ) 
		{
			ss << "Goal: " << fDebugTypeName( );

			if( aiGoal->fIsOneShot( ) ) 
				ss << " (" << aiGoal->fTimeToLive( ) << ")";
		}
		else 
		{
			if( compositeGoal ) ss << "Composite Goal: ";
			else ss << "Goal: ";

			ss << fDebugTypeName( ) << " (" << mRelativePriority << ")";
		}

		ss << std::endl;

		const std::string extraText = fExtraWorldDebugText( );
		if( extraText.length( ) > 0 )
		{
			fPushIndent( ss, indent + 1 );
			ss << extraText << std::endl;
		}

		if( compositeGoal )
		{
			// Print out goal stack, top to bottom
			if( compositeGoal->fGoalStack( ).fCount( ) > 0 )
			{
				for( s32 i = compositeGoal->fGoalStack( ).fCount( ) - 1; i >= 0 ; --i )
					compositeGoal->fGoalStack( )[ i ].fAddWorldDebugText( ss, indent + 1 );
			}
			if( compositeGoal->mPotentialGoals.fCount( ) > 0 )
			{
				fPushIndent( ss, indent );
				ss << "Potential Goals:" << std::endl;
				for( u32 i = 0; i < compositeGoal->mPotentialGoals.fCount( ); ++i )
				{
					fPushIndent( ss, indent + 1 );
					ss << compositeGoal->mPotentialGoals[ i ].fDebugTypeName( ) << " (" << compositeGoal->mPotentialGoals[ i ].mRelativePriority << ")"  << std::endl;
				}
			}
			if( compositeGoal->mTriggeredGoals.fCount( ) > 0 )
			{
				fPushIndent( ss, indent );
				ss << "Triggerable Goals:" << std::endl;
				for( u32 i = 0; i < compositeGoal->mTriggeredGoals.fCount( ); ++i )
				{
					fPushIndent( ss, indent + 1 );
					ss << compositeGoal->mTriggeredGoals[ i ].fDebugTypeName( ) << " (" << compositeGoal->mTriggeredGoals[ i ].mRelativePriority << ")" << std::endl;
				}
			}

			return false;
		}
		else
		{
			return true;
		}
	}
	std::string tGoalPtr::fExtraWorldDebugText( ) const
	{
		if( fIsNull( ) )
			return std::string( );
		else if( fIsCodeOwned( ) )
			return fGoal( )->fExtraWorldDebugText( );
		else
		{
			Sqrat::Function f( fScriptObject( ), "ExtraWorldDebugText" );
			sigassert( !f.IsNull( ) );
			return f.Evaluate<std::string>( );
		}
	}
#endif//sig_devmenu
	f32 tGoalPtr::fComputePriority( tLogic* logic )
	{
		if( fIsNull( ) )
			return 0.f;
		else if( fIsCodeOwned( ) )
			return mRelativePriority * fGoal( )->fComputePriority( logic );
		else
		{
			Sqrat::Function f( fScriptObject( ), "ComputePriority" );
			sigassert( !f.IsNull( ) );
			return mRelativePriority * f.Evaluate<f32>( fGetLogicForScript( logic ) );
		}
	}
	void tGoalPtr::fOnInsertion( tLogic* logic ) const
	{
		if( fIsNull( ) )
			return;
		if( fIsCodeOwned( ) )
			fGoal( )->fOnInsertion( logic );
		else
		{
			Sqrat::Function f( fScriptObject( ), "OnInsertion" );
			sigassert( !f.IsNull( ) );
			f.Execute( fGetLogicForScript( logic ) );
		}
	}
	b32 tGoalPtr::fActivateIfNotActive( tLogic* logic )
	{
		if( fIsNull( ) || fGoal( )->mStatus == tGoal::cStatusActive )
			return false; // already active

		// activate BEFORE we invoke OnActivate callback, in case goal wants to immediately change status
		fGoal( )->mStatus = tGoal::cStatusActive;

		if( fIsCodeOwned( ) )
			fGoal( )->fOnActivate( logic );
		else
		{
			Sqrat::Function f( fScriptObject( ), "OnActivate" );
			sigassert( !f.IsNull( ) );
			f.Execute( fGetLogicForScript( logic ) );
		}

		return true;
	}
	b32 tGoalPtr::fSuspendIfActive( tLogic* logic )
	{
		if( !logic )
		{
			//skip all this if our logic is gone.
			return false;
		}

		if( fIsNull( ) || fGoal( )->mStatus == tGoal::cStatusInactive )
			return false; //already suspended
		else if( fIsCodeOwned( ) )
		{
			fGoal( )->mStatus = tGoal::cStatusInactive; // Doin this first prevents an infinite loop in tSigAIGoal, where OnSuspend may cause another SuspendIfActive call.
			fGoal( )->fOnSuspend( logic );
		}
		else
		{
			fGoal( )->mStatus = tGoal::cStatusInactive;
			Sqrat::Function f( fScriptObject( ), "OnSuspend" );
			sigassert( !f.IsNull( ) );
			f.Execute( fGetLogicForScript( logic ) );
		}

		return true;
	}
	b32 tGoalPtr::fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e )
	{
		if( fIsCodeOwned( ) )
			return fGoal( )->fHandleLogicEvent( logic, e );
		else if( fScriptObject( ).IsNull( ) )
			return false;
		else 
		{
			Sqrat::Function f( fScriptObject( ), "HandleLogicEvent" );
			sigassert( !f.IsNull( ) );
			return f.Evaluate<bool>( fGetLogicForScript( logic ), e )!=0;
		}
	}

	void tGoalPtr::fOnDelete( )
	{
		if( fGoal( ) )
			return fGoal( )->fOnDelete( );
	}
}}


//--------------------------------------------------------------------------------------------------------------
//
//    tCompositeGoal
//
//--------------------------------------------------------------------------------------------------------------
namespace Sig { namespace AI
{
	tCompositeGoal::tCompositeGoal( )
		: mCurrentPriority( 0.f ), mSuspendAfterSubGoals( false )
	{
	}
	tCompositeGoal::~tCompositeGoal( )
	{
		fRemoveSubGoals( 0 );
	}
	void tCompositeGoal::fAddPotentialGoal( const tGoalPtr& goal )
	{
		mPotentialGoals.fPushBack( goal );
	}
	void tCompositeGoal::fClearPotentialGoals( tLogic* logic )
	{
		mPotentialGoals.fSetCount( 0 );
	}
	void tCompositeGoal::fAddTriggeredGoal( const tGoalPtr& goal )
	{
		mTriggeredGoals.fPushBack( goal );
	}
	void tCompositeGoal::fClearTriggeredGoals( tLogic* logic )
	{
		mTriggeredGoals.fSetCount( 0 );
	}
	void tCompositeGoal::fAddConcurrentGoal( const tGoalPtr& goal )
	{
		mConcurrentGoals.fPushBack( goal );
	}
	void tCompositeGoal::fClearConcurrentGoals( tLogic* logic )
	{
		mConcurrentGoals.fSetCount( 0 );
	}
	void tCompositeGoal::fAddImmediateGoal( const tGoalPtr& subGoal, tLogic* logic )
	{
		sigassert( !mGoalStack.fFind( subGoal ) );
		mGoalStack.fPushBack( subGoal );
		subGoal.fOnInsertion( logic );
	}
	void tCompositeGoal::fPushGoalList( const tGoalPtrList& goals, tLogic* logic )
	{
		for( u32 i = goals.fCount( ); i > 0; --i )
			fAddImmediateGoal( goals[ i - 1 ], logic );
	}
	void tCompositeGoal::fRemoveSubGoals( tLogic* logic )
	{
		for( u32 i = 0; i < mGoalStack.fCount( ); ++i )
			mGoalStack[ i ].fSuspendIfActive( logic );
		mGoalStack.fSetCount( 0 );
	}
	void tCompositeGoal::fOnActivate( tLogic* logic )
	{
		if( !fHasSubGoals( ) )
		{
			fProcessTriggeredGoals( logic, 0.f, true );
			if( !fHasSubGoals( ) )
				fProcessPotentialGoals( logic, 0.f );
		}

		if( fHasSubGoals( ) )
		{
			tGoalPtr& currentGoal = fCurrentGoal( );
			if( currentGoal.fActivateIfNotActive( logic ) )
				mCurrentPriority = currentGoal.fComputePriority( logic );
		}

		for( u32 i = 0; i < mConcurrentGoals.fCount( ); ++i )
			mConcurrentGoals[ i ].fActivateIfNotActive( logic );
	}
	void tCompositeGoal::fOnSuspend( tLogic* logic )
	{
		if( fHasSubGoals( ) )
			fCurrentGoal( ).fSuspendIfActive( logic );

		for( u32 i = 0; i < mConcurrentGoals.fCount( ); ++i )
			mConcurrentGoals[ i ].fSuspendIfActive( logic );
	}
	void tCompositeGoal::fOnProcess( tGoalPtr& goalPtr, tLogic* logic, f32 dt )
	{
		fProcessSubGoals( goalPtr, logic, dt );

		for( u32 i = 0; i < mConcurrentGoals.fCount( ); ++i )
			mConcurrentGoals[ i ].fGoal( )->fOnProcess( goalPtr, logic, dt );
	}
	b32 tCompositeGoal::fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e )
	{
		for( u32 i = 0; i < mConcurrentGoals.fCount( ); ++i )
			if( mConcurrentGoals[ i ].fHandleLogicEvent( logic, e ) ) 
				return true;

		if( fHasSubGoals( ) )
			return fCurrentGoal( ).fHandleLogicEvent( logic, e );

		return false;
	}
	void tCompositeGoal::fOnDelete( )
	{
		for( u32 i = 0; i < mConcurrentGoals.fCount( ); ++i )
			mConcurrentGoals[ i ].fOnDelete( );
		mConcurrentGoals.fSetCount( 0 );
		for( u32 i = 0; i < mGoalStack.fCount( ); ++i )
			mGoalStack[ i ].fOnDelete( );
		mGoalStack.fSetCount( 0 );
		for( u32 i = 0; i < mPotentialGoals.fCount( ); ++i )
			mPotentialGoals[ i ].fOnDelete( );
		mPotentialGoals.fSetCount( 0 );
		for( u32 i = 0; i < mTriggeredGoals.fCount( ); ++i )
			mTriggeredGoals[ i ].fOnDelete( );
		mTriggeredGoals.fSetCount( 0 );
	}
	tGoalPtr tCompositeGoal::fPopCurrentGoal( )
	{
		return mGoalStack.fPopBack( );
	}
	f32 tCompositeGoal::fComputePriority( tLogic* logic )
	{
		if( mGoalStack.fCount( ) > 0 || mPotentialGoals.fCount( ) > 0 )
			return 1.f;
		else
			return 0.f;
	}
	void tCompositeGoal::fProcessSubGoals( tGoalPtr& goalPtr, tLogic* logic, f32 dt )
	{
		// remove all completed goals
		while( fHasSubGoals( ) && fCurrentGoal( ).fGoal( )->fStatus( ) == cStatusComplete )
		{
			fPopCurrentGoal( ).fSuspendIfActive( logic );
		}

		if( mSuspendAfterSubGoals && !fHasSubGoals( ) )
		{
			mStatus = cStatusComplete;
			mSuspendAfterSubGoals = false;
			return;
		}
		
		// determine which goals to switch to next
		fProcessTriggeredGoals( logic, dt, false );

		// ensure that any goals that are no longer at the top of the goal stack get suspended if necessary (BEFORE activating current goal)
		for( s32 i = mGoalStack.fCount( ) - 2; i >= 0; --i )
			mGoalStack[ i ].fSuspendIfActive( logic );

		// process sub-goals
		if( fHasSubGoals( ) )
		{
			tGoalPtr currentGoal = fCurrentGoal( );
			if( currentGoal.fActivateIfNotActive( logic ) )
				mCurrentPriority = currentGoal.fComputePriority( logic );
			currentGoal.fGoal( )->fProcess( currentGoal, logic, dt );

			const tStatus status = currentGoal.fGoal( )->fStatus( );
			sigassert( status != cStatusInactive );

			if( status == cStatusComplete && mGoalStack.fCount( ) > 1 )
				mStatus = cStatusActive;
		}

		// we're out of sub-goals, go through potentials and see if they want a turn
		if( !fHasSubGoals( ) )
			fProcessPotentialGoals( logic, dt );

		// if still no sub-goals, then we're done
		if( !fHasSubGoals( ) )
			mStatus = cStatusComplete;
	}
	void tCompositeGoal::fProcessTriggeredGoals( tLogic* logic, f32 dt, b32 forceCheck )
	{
		// check for any triggerable goals that need to be triggered now

		u32 bestI = ~0;
		f32 bestDesirability = fHasSubGoals( ) ? mCurrentPriority : 0.f; // have to be more desirable than current

		for( u32 i = 0; i < mTriggeredGoals.fCount( ); ++i )
		{
			tGoalPtr& goal = mTriggeredGoals[ i ];
			tGoal& cppGoal = *goal.fGoal( );

			if( cppGoal.fStatus( ) == cStatusActive )
			{
				cppGoal.mTriggerCheckElapsed = 0.f;
				continue;
			}

			if( cppGoal.mTriggerCheckElapsed >= cppGoal.mTriggerCheckFrequency || forceCheck )
			{
				cppGoal.mTriggerCheckElapsed = 0.f;

				const f32 desirability = goal.fComputePriority( logic );
				if( desirability > 0.f && desirability >= bestDesirability )
				{
					if( !mGoalStack.fFind( goal ) )
					{
						bestDesirability = desirability;
						bestI = i;
					}
				}
			}
			else
				cppGoal.mTriggerCheckElapsed += dt;
		}

		if( bestI < mTriggeredGoals.fCount( ) )
			fAddImmediateGoal( mTriggeredGoals[ bestI ], logic );
	}
	void tCompositeGoal::fProcessPotentialGoals( tLogic* logic, f32 dt )
	{
		sigassert( !fHasSubGoals( ) );

		u32 bestI = ~0;
		f32 bestDesirability = 0.f; // have to be at least somewhat desirable

		for( u32 i = 0; i < mPotentialGoals.fCount( ); ++i )
		{
			const f32 desirability = mPotentialGoals[ i ].fComputePriority( logic );
			if( desirability > 0.f && desirability >= bestDesirability )
			{
				bestDesirability = desirability;
				bestI = i;
			}
		}
		if( bestI < mPotentialGoals.fCount( ) )
			fAddImmediateGoal( mPotentialGoals[ bestI ], logic );
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
		static void fAddPotentialGoal(	tCompositeGoal* goal, const Sqrat::Object& obj, f32 relPri ) { goal->fAddPotentialGoal( tGoalPtr( obj, relPri ) ); }
		static void fAddTriggeredGoal(	tCompositeGoal* goal, const Sqrat::Object& obj, f32 relPri ) { goal->fAddTriggeredGoal( tGoalPtr( obj, relPri ) ); }
		static void fAddImmediateGoal(	tCompositeGoal* goal, const Sqrat::Object& obj, f32 relPri, tLogic* logic ) { goal->fAddImmediateGoal( tGoalPtr( obj, relPri ), logic ); }
		static void fAddConcurrentGoal(	tCompositeGoal* goal, const Sqrat::Object& obj, f32 relPri ) { goal->fAddConcurrentGoal( tGoalPtr( obj, relPri ) ); }
	}
	void tGoal::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tGoal, Sqrat::NoCopy<tGoal> > classDesc( vm.fSq( ) );

		classDesc
			.Func(_SC("DebugTypeName"),					&tGoal::fDebugTypeName)
			.Func(_SC("SetTriggerCheckFrequency"),		&tGoal::fSetTriggerCheckFrequency)
			.Func(_SC("ComputePriority"),				&tGoal::fComputePriority)
			.Func(_SC("OnInsertion"),					&tGoal::fOnInsertion)
			.Func(_SC("OnActivate"),					&tGoal::fOnActivate)
			.Func(_SC("OnSuspend"),						&tGoal::fOnSuspend)
			.Func(_SC("HandleLogicEvent"),				&tGoal::fHandleLogicEvent)
			.Func(_SC("MarkAsComplete"),				&tGoal::fMarkAsComplete)
			.Func(_SC("Terminate"),						&tGoal::fTerminate)
			.Prop(_SC("Active"),						&tGoal::fActive)
#ifdef sig_devmenu
			.Func(_SC("ExtraWorldDebugText"),			&tGoal::fExtraWorldDebugText)
#endif//sig_devmenu
			;

		vm.fNamespace(_SC("AI")).Bind(_SC("Goal"), classDesc);
	}
	void tCompositeGoal::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tCompositeGoal, tGoal, Sqrat::NoCopy<tCompositeGoal> > classDesc( vm.fSq( ) );

		classDesc
			.GlobalFunc(_SC("AddPotentialGoal"),	&::Sig::AI::fAddPotentialGoal)
			.GlobalFunc(_SC("AddTriggeredGoal"),	&::Sig::AI::fAddTriggeredGoal)
			.GlobalFunc(_SC("AddImmediateGoal"),	&::Sig::AI::fAddImmediateGoal)
			.GlobalFunc(_SC("AddConcurrentGoal"),	&::Sig::AI::fAddConcurrentGoal)
			.Func(_SC("ClearExistingGoals"),		&tCompositeGoal::fRemoveSubGoals)
			.Func(_SC("ClearPotentialGoals"),		&tCompositeGoal::fClearPotentialGoals)
			.Func(_SC("ClearTriggeredGoals"),		&tCompositeGoal::fClearTriggeredGoals)
			.Func(_SC("ClearConcurrentGoals"),		&tCompositeGoal::fClearConcurrentGoals)
			.Func(_SC("SuspendAfterSubGoals"),		&tCompositeGoal::fSuspendAfterSubGoals)
			
			;

		vm.fNamespace(_SC("AI")).Bind(_SC("CompositeGoal"), classDesc);
	}
}}

