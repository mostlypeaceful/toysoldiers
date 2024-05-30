#ifndef __tGoal__
#define __tGoal__

namespace Sig { namespace AI
{
	class tGoal;
	class tGoalPtr;
	class tCompositeGoal;
	
	class base_export tGoal : public tRefCounter
	{
		friend class tGoalPtr;
		friend class tCompositeGoal;
		debug_watch( tGoal );
		declare_uncopyable( tGoal );
		define_dynamic_cast_base( tGoal );
	public:
		enum tStatus
		{
			cStatusInactive,
			cStatusActive,
			cStatusComplete,
			cStatusTerminated,
			cStatusCount
		};

	protected:
		tStatus			mStatus;
		f32				mTriggerCheckFrequency;
		f32				mTriggerCheckElapsed;
		Logic::tGoalDriven*	mOwner;

	public:
		tGoal( );
		virtual ~tGoal( );

		tStatus			fStatus( ) const { return mStatus; }
		void			fMarkAsComplete( ) { mStatus = cStatusComplete; }
		b32				fActive( ) { return ( mStatus == cStatusActive ); }
		void			fTerminate( ) { mStatus = cStatusTerminated; }

		///
		/// \brief Call to tick the goal.
		void			fProcess( tGoalPtr& goalPtr, f32 dt );

	protected:

		///
		/// \brief Updates the frequency at which this goal should be checked for triggering.
		void			fSetTriggerCheckFrequency( f32 freq ) { mTriggerCheckFrequency = freq; }

		///
		/// \brief Calculates how desirable this goal is at this moment in time, in [0 (least desirable), 1 (most desirable)]. Will be called on
		/// both 'potential' and 'triggered' goals.
		virtual f32		fComputePriority( );

		///
		/// \brief Called to notify the goal that it is being inserted into a goal stack for the first time.
		virtual void	fOnInsertion( Logic::tGoalDriven* goalDriven ) { mOwner = goalDriven; }

		///
		/// \brief Called to notify the goal that it is being activated.
		/// \note This method will only be called if the goal's current status is not active; the status won't be set to active until
		/// after this call, meaning you can check the previous status inside this method (i.e., failed, or just inactive).
		virtual void	fOnActivate( ) { }

		///
		/// \brief Called prior to suspension or destruction of the goal. It is important to note that this method should not completely
		/// destroy itself! Instead, only do only what's necessary to make the goal inactive for a period;
		/// the goal should be able to continue successfully after a call to fTryActivate( ).
		/// \note This method will only be called if the goal's current status is not inactive; the status won't be set to inactive until
		/// after this call, meaning you can check the previous status inside this method (i.e., active, complete, failed, etc).
		virtual void	fOnSuspend( ) { }

		///
		/// \brief The goal's core tick method. Called only while the goal is active.
		virtual void	fOnProcess( tGoalPtr& goalPtr, f32 dt ) { }

		///
		/// \brief Process logic events. Return true if handled.
		virtual b32		fHandleLogicEvent( const Logic::tEvent& e ) { return false; }

		virtual void	fOnDelete( ) { }

		///
		/// \brief So goals can print extra world debug text.
		if_devmenu( virtual std::string fExtraWorldDebugText( ) const { return std::string( ); } )

		///
		/// \brief This logic is the old, "main" logic.
		tLogic* fOldLogic( );
		Sqrat::Object fGetLogicForScript( );

		///
		/// \brief This is specifically the goal driven owner component.
		Logic::tGoalDriven* fGoalDriven( ) { return mOwner; }

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	};

	///
	/// \brief Wrapper around base goal type, providing glue between script and code.
	class base_export tGoalPtr : public tScriptOrCodeObjectPtr<tGoal>
	{
		debug_watch( tGoalPtr );
		f32 mRelativePriority;
	public:
		tGoalPtr( );
		tGoalPtr( const Sqrat::Object& o, f32 relativePriority );
		tGoalPtr( tGoal* o, f32 relativePriority );
		~tGoalPtr( );

		tGoal* fGoal( ) const { return fCodeObject( ); }

		///
		/// \brief Debug-only type name access (for this goal).
		const char*		fDebugTypeName( ) const;

		///
		/// \brief Debug-only type name of the current active sub-goal.
		if_devmenu( b32	fAddWorldDebugText( std::stringstream& ss, u32 indent ) const );
		if_devmenu( std::string fExtraWorldDebugText( ) const );

		///
		/// \brief See above for tGoal - calls into script object.
		f32				fComputePriority( );

		///
		/// \brief Notify the goal that it's being inserted into the goal stack.
		void			fOnInsertion( Logic::tGoalDriven* goalDriven ) const;

		///
		/// \brief Call to make the goal current/active (either before first time active, or after re-awakening from being suspended).
		/// \return true if the goal was previously suspended (i.e., it was newly activated during this call)
		b32				fActivateIfNotActive( );

		///
		/// \brief Call to suspend, or else before destruction.
		/// \return true if the goal was previously active (i.e., it was newly suspended during this call)
		b32				fSuspendIfActive( );

		///
		/// \brief Forwards logic events to sub-goals.
		b32				fHandleLogicEvent( const Logic::tEvent& e );

		void			fOnDelete( );

		template< typename GoalType >
		b32 fHasActiveGoalWithType( ) const
		{

			tGoal* goal = fGoal( );

			if( !goal )
				return false;

			if( goal->fDynamicCast< GoalType >( ) != NULL )
				return true;

			if( tCompositeGoal* cgoal = goal->fDynamicCast< tCompositeGoal >( ) )
			{
				for( u32 i = 0; i < cgoal->mGoalStack.fCount(); ++i )
				{
					if( cgoal->mGoalStack[ i ].fHasActiveGoalWithType< GoalType >( ) )
						return true;
				}
			}

			return false;

		}
	};

	typedef tGrowableArray<tGoalPtr> tGoalPtrList;


	///
	/// \brief Encapsulates a high-level goal type by braking it down into smaller sub-goals. Note that the sub-goals can themselves
	/// be other composite goals (they need not be atomic goals).
	///  Concurrent goals act like another set of goals at this composite goal level.
	///   They are activated and suspended and processed in step with this one.
	class base_export tCompositeGoal : public tGoal
	{
		friend class tGoalPtr;
		debug_watch( tCompositeGoal );
		declare_uncopyable( tCompositeGoal );
		define_dynamic_cast( tCompositeGoal, tGoal );
	protected:
		tGoalPtrList	mPotentialGoals;
		tGoalPtrList	mTriggeredGoals;
		tGoalPtrList	mGoalStack;
		tGoalPtrList	mConcurrentGoals;
		f32				mCurrentPriority;
		b32				mSuspendAfterSubGoals;

	public:
		tCompositeGoal( );
		virtual ~tCompositeGoal( );

		///
		/// \brief Returns 1.f if has subgoals or potential goals, else returns 0.
		f32				fComputePriority( );

		///
		/// \brief Adds a potential goal to the list
		void			fAddPotentialGoal( const tGoalPtr& goal );
		void			fClearPotentialGoals( );

		///
		/// \brief Adds a triggered goal to the list
		void			fAddTriggeredGoal( const tGoalPtr& goal );
		void			fClearTriggeredGoals( );

		///
		/// \brief Adds a concurrent goal to the list
		void			fAddConcurrentGoal( const tGoalPtr& goal );
		void			fClearConcurrentGoals( );

		///
		/// \brief Pushes a new sub goal; this goal becomes the current goal (LIFO, stack-style)
		void			fAddImmediateGoal( const tGoalPtr& subGoal );

		///
		/// \brief Same as fAddImmediateGoal, but lets you specify the index.
		void			fInsertGoal( const tGoalPtr& subGoal, u32 index );

		///
		/// \brief Pushes a set of new sub goals. The goals are pushed in reverse order,
		/// such that the first goal in the list becomes the current goal, the second is second, etc.
		/// This allows for a conceptually easier way to group a sequence of goals, so that you can add them
		/// to the goal list in the order they should be executed and then make a single call to fPushGoalList, 
		/// instead of having to call fAddImmediateGoal for each goal in the reverse order of their execution.
		/// \note This method is only supported if fSupportsSubGoals returns true.
		void			fPushGoalList( const tGoalPtrList& goals );

		///
		/// \brief Removes any existing goals.
		void			fRemoveSubGoals( );

		///
		/// \brief See if I have any goals on the active goal stack.
		b32				fHasSubGoals( ) const { return mGoalStack.fCount( ) > 0; }

		///
		/// \brief Get the actual count of goals in the goal stack.
		u32				fSubGoalCount( ) const { return mGoalStack.fCount( ); }

		///
		/// \brief Access the current goal.
		inline tGoalPtr&		fCurrentGoal( )			{ return mGoalStack.fBack( ); }
		inline const tGoalPtr&	fCurrentGoal( ) const	{ return mGoalStack.fBack( ); }
		inline tGoalPtrList&	fGoalStack( )			{ return mGoalStack; }

		///
		/// \brief Let subgoals finish and then consider this goal complete.
		void			fSuspendAfterSubGoals( ) { mSuspendAfterSubGoals = true; }

	protected:

		///
		/// \brief Activate the goal (and this goal's current sub-goal). Derived types should call this method if overriden.
		virtual void	fOnActivate( );

		///
		/// \brief Suspend the goal (and this goal's current sub-goal). Derived types should call this method if overriden.
		virtual void	fOnSuspend( );

		///
		/// \brief Processes sub-goals. Derived types should call this method if overriden.
		virtual void	fOnProcess( tGoalPtr& goalPtr, f32 dt );

		///
		/// \brief Process or forward logic events. Return true if handled.
		virtual b32		fHandleLogicEvent( const Logic::tEvent& e );

		virtual void	fOnDelete( );

	protected: // interface for derived types to manipulate
		tGoalPtr		fPopCurrentGoal( );
		void			fProcessSubGoals( tGoalPtr& goalPtr, f32 dt );
		void			fProcessTriggeredGoals( f32 dt, b32 forceCheck );
		void			fProcessPotentialGoals( f32 dt );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	};

	///
	/// \brief Helper class for caching the derived logic type (call fAcquireDerivedLogic in fOnActivate of your goal, then use fLogic( ) to access your derived Logic type.)
	template<class tDerivedLogicType>
	class tDerivedLogicGoalHelper
	{
		debug_watch( tDerivedLogicGoalHelper<tDerivedLogicType> );
		declare_uncopyable( tDerivedLogicGoalHelper );
	public:
		tDerivedLogicGoalHelper( ) : mDerivedLogic( 0 ) { }
		void fAcquireDerivedLogic( tLogic* logic ) { if( !mDerivedLogic ) mDerivedLogic = logic->fDynamicCast< tDerivedLogicType >( ); sigassert( mDerivedLogic ); }
		inline tDerivedLogicType* fLogic( ) const { return mDerivedLogic; }
	private:
		tDerivedLogicType* mDerivedLogic;
	};

}}

#endif//__tGoal__
