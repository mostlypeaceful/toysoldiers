#ifndef __tSigAIGoal__
#define __tSigAIGoal__

#include "tGoal.hpp"
#include "tMotionMap.hpp"

namespace Sig { namespace AI
{

	///
	/// \brief Callback delegate for onprocess functions.
	typedef tDelegate< void ( tLogic* f32 ) > tSigAIOnProcess;
	
	///
	/// \brief Encapsulates added behavior for the SigAI editor
	class base_export tSigAIGoal : public tCompositeGoal, public Anim::tBlendReference
	{
		friend class tGoalPtr;
		debug_watch( tSigAIGoal );
		declare_uncopyable( tSigAIGoal );
		define_dynamic_cast( tSigAIGoal, tCompositeGoal );

		// hide these methods, dont call them when using the SigAI system
		void fAddImmediateGoal( b32 ) { }
		void fRemoveSubGoals( b32 ) { }
		void fAddPotentialGoal( b32 ) { }
		void fAddTriggeredGoal( b32 ) { }
 
		static u32 gCount;
	public:
		/// tExitMode describes the context during an fOnSuspend
		enum tExitMode { cExitHandled, cExitSuspended, cExitCompleted, cExitModeCount };
		
		tSigAIGoal( );
		~tSigAIGoal( );
		void fCommonInit( );
		static u32 fGlobalCount( ) { return gCount; }

		/// Assert valid types and cast to tSigAIGoal
		tSigAIGoal* fToSigAI( const tGoalPtr& goal ) const
		{
			return goal.fGoal( )->fStaticCast< tSigAIGoal >( );
		}

		inline tSigAIGoal* fCurrentGoal( ) const { return fToSigAI( tCompositeGoal::fCurrentGoal( ) ); }
		inline u32 fExitMode( ) const					{ return mExitMode; }
		inline void fSetExitMode( u32 mode )			{ mExitMode = mode; }

		// The parent pointer is only used for fSwitchToGoal, never any messaging
		inline tSigAIGoal* fParent( ) const					{ return mParent; }
		inline void fSetParent( tSigAIGoal* parent )		{ mParent = parent; }

		/// Any goal with a fPriority higher than maxClear will block this action.
		inline u32 fPriority( ) const { return mPriority; }
		inline void fSetPriority( u32 priority ) { mPriority = priority; }
		inline u32 fChildPriority( ) const { return mCurrentChildPriority; }
		inline b32 fIsOneShot( ) const { return mIsOneShot; }
		inline b32 fIsMotionGoal( ) const { return mIsMotionState; }
		inline void fPersist( ) { mPersist = true; }
		inline f32 fTimeToLive( ) const { return (mTimeToLive - mTimer); }
		inline f32 fProgress( ) const { return ( mTimeToLive != 0 )? mTimer / mTimeToLive: 0; }
		f32 fElapsedTime( ) const { return mTimer; }
		f32 fTotalTime( ) const { return mTimeToLive; }

		/*
			If the top goal's priority is equal or less than maxClear, the stack will be cleared and this goal pushed.
		*/
		b32 fClearAndPushGoal( u32 maxClear, const tGoalPtr& goal );

		/*
			this goal will be pushed.
		*/
		void fPushGoal( const tGoalPtr& goal );
		
		/*
			If the top goal's priority is equal or less than maxClear, this goal will be pushed.
		*/
		void fSwitchToGoal( tGoalPtr& goal );
		
		/*
			Starting fromt he bottom of the stack, find the first goal with "priority", push this goal below it.
			If no goal is found, this will essentially push this goal on top.
		*/
		void fPushBelow( u32 priority, const tGoalPtr& goal );

		/// Triggered goals and potential goals use a new mechanism.
		/// Priorities are calculated via function members of this class,
		///  rather than the class they will be pushing. Since different "pushers"
		///  may have a different interpretation of their priority.

		///
		/// \brief Processes sub-goals.
		virtual void	fOnProcess( tGoalPtr& goalPtr, f32 dt );
		virtual void	fOnActivate( );
		virtual void	fOnSuspend( );

		///
		/// \brief Events are now forwarded to the top of the stack and processed down.
		virtual b32		fHandleLogicEvent( const Logic::tEvent& e );

		void			fTerminateGoalsNamedRecursive( const tStringPtr& name );
		void			fCompleteGoalsNamedRecursive( const tStringPtr& name );

		virtual void	fOnDelete( );

	public:
		/// \brief Function to be called when no goals exist in the stack. Could push a new goal.
		virtual void fCheckAndPushPotentialGoals( Sqrat::Object logicScriptObject ) { }

		/// \brief The name of the above function, as exposed to script. Used in the SigAI editor.
		static const char cSigAIPotentialFunction[];

		static void		fExportScriptInterface( tScriptVm& vm );

	protected:
		/// Returns true if the current goal is <= maxClear
		b32				fCanClear( u32 maxClear ) const;

		// Finds the actual tGoalPtr in the subgoal list.
		tGoalPtr&		fFindMe( tSigAIGoal* goal );
		void			fProcessSubGoals( tGoalPtr& goalPtr, f32 dt );
		void			fProcessPotentialGoals( tGoalPtr& goalPtr, f32 dt );

		// Motion stuff
		void fSetMotionState( const char* motionStateNam, const Sqrat::Object& motionStateTable, bool oneShot );
		void fClearMotionState( );
		void fExecuteMotionState( );
		f32  fEvaluateMotionState( );

		void fSetIsOneShot( b32 oneShot ) { mIsOneShot = oneShot; }

		// MarkGoalAsComplete after timer expires
		void fSetOneShotTimer( f32 timeToLive );
		void fAddOneShotTime( f32 time );

		void fSetToWaitForEndEvent( b32 waitForEndEvent );

		void fSetup( u32 priority, const char* editorName, Sqrat::Object& obj );
		void fSetOnProcess( const tSigAIOnProcess& process ) { mOnProcess = process; }
		void fAddMoStateRef( const char* moStateName );

		/// \brief Recursively search goal stack for goals with debug name match, terminate them
		void fTerminateGoalsNamedFromScript( const char* name );
		void fCompleteGoalsNamedFromScript( const char* name );

	private:
		tSigAIGoal*		mParent;
		u32				mPriority;
		u32				mCurrentChildPriority;
		b32				mSwappedOnExit;
		u32				mExitMode;
		tStringPtr		mEditorName;
		tSigAIOnProcess mOnProcess;

		// anim stuff
		tStringPtr		mApplyMotionStateName;
		Sqrat::Function mOnTick;
		Sqrat::Object	mMotionStateParams;
		HSQOBJECT		mScriptGoal;
		b8 mIsOneShot;
		b8 mIsMotionState;
		b8 mPersist;
		b8 mReceivedEndEvent;

		f32 mTimeToLive;
		f32 mTimer;

		Sqrat::Function fFindMoStateFunction( );
		const char* fScriptDebugTypeName( ) const;

		tGrowableArray< Anim::tSigAnimMoMap::tBlendData* > mBlendReferences;
		void fAddMotionRefs( b32 add );

		// From tBlendReference
		virtual void fAnimEnded( );
	};

}}

#endif//__tSigAIGoal__
