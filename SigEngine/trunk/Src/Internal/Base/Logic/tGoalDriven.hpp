#ifndef __tGoalDriven__
#define __tGoalDriven__
#include "tLogic.hpp"
#include "AI/tGoal.hpp"
#include "AI/tSigAIData.hpp"

namespace Sig { namespace Logic
{

	class base_export tGoalDriven : public tLogic
	{
		debug_watch( tGoalDriven );
		declare_uncopyable( tGoalDriven );
	private:
		b32				mEnabled;
		AI::tGoalPtr	mMasterGoal;
		tGrowableArray< AI::tGoalPtr > mAdditionalGoals;

		tHashTable< tStringPtr, tStringPtr > mMotionStateMap;

	public:
		AI::tAIData		mData;

		tGoalDriven( );
		virtual ~tGoalDriven( );

		virtual void fOnDelete( ) OVERRIDE;
		virtual void fOnSpawn( ) OVERRIDE;
		virtual void fOnPause( b32 paused ) OVERRIDE;
		virtual void fActST( f32 dt ) OVERRIDE;
		virtual b32 fHandleLogicEvent( const Logic::tEvent& e ) OVERRIDE;

		///
		/// \brief See if a certain subgoal type is active
		template< typename GoalType >
		b32  fHasActiveGoalWithType( ) const
		{
			if( mMasterGoal.fHasActiveGoalWithType< GoalType >( ) )
				return true;

			for( u32 i = 0; i < mAdditionalGoals.fCount( ); ++i )
			{
				if( mAdditionalGoals[ i ].fHasActiveGoalWithType< GoalType >( ) )
					return true;
			}
			return false;
		}

		void fSetEnabled( b32 enabled );
		const AI::tGoalPtr& fMasterGoal( ) const { return mMasterGoal; }
		void fSetMasterGoal( const AI::tGoalPtr& goal ) { mMasterGoal.fOnDelete( ); mMasterGoal = goal; }
		void fClearGoals( );

		void fTerminateGoalsNamedRecursive( const tStringPtr& name );
		void fCompleteGoalsNamedRecursive( const tStringPtr & name );

		// This mapped motion state thing is essentially virtual motion state calls.
		//  Return the remapped actual motion state.
		tStringPtr fMapMotionState( const tStringPtr& key );
		void fSetMapMotionState( const tStringPtr& key, const tStringPtr& override );
		void fClearMapMotionStates( );

		if_devmenu( virtual void fAddWorldDebugText( std::stringstream& ss ) const; );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	private:// script-specific private member functions (exposed only to script, not code)
		Sqrat::Object				fScriptGoalObject( ) const { return mMasterGoal.fScriptObject( ); }
		void						fSetScriptGoalObject( const Sqrat::Object& obj );
		void						fAddAdditionalGoalFromScript( const Sqrat::Object& obj );
	};

}}

#endif//__tGoalDriven__
