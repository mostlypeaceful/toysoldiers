#ifndef __tEditorAction__
#define __tEditorAction__
#include "tDelegate.hpp"
#include "tRefCounterPtr.hpp"
#include "tRingBuffer.hpp"

namespace Sig
{

	///
	/// \brief Represents an action the modifies/edits the state of the
	/// current document. This is the minimal granularity of actions
	/// that can be undone/redone.
	class tools_export tEditorAction : public tRefCounter
	{
		b32 mIsLive;
		std::string mTitle;
	public:
		tEditorAction( ) : mIsLive( false ) { }
		virtual ~tEditorAction( ) { }
		virtual void fUndo( ) = 0;
		virtual void fRedo( ) = 0;

		///
		/// \brief This means that the action above it on the stack
		/// was just undone, or that this action was just re-done.
		virtual void fOnBackOnTop( ) { }

		///
		/// \brief For multi-frame actions, you should return true
		/// as long as the action is still happening (as long as it is "live").
		inline b32 fIsLive( ) { return false; }

		///
		/// \brief For multi-frame actions, you can implement final
		/// actions in this method; provides a standard interface; be
		/// sure to call fSetIsLive( false ) in here if you override this method.
		virtual void fEnd( ) { fSetIsLive( false ); }

		///
		/// \brief Override this to indicate that the derived action
		/// does not in fact mark the scene as dirty. These actions will
		/// still interact with the undo stack, they just won't force
		/// the user to save the scene.
		virtual b32 fDirtyingAction( ) const { return true; }

		void fSetTitle( const std::string& title ) { mTitle = title; }
		const std::string& fTitle( ) const { return mTitle; }

	protected:

		inline void fSetIsLive( b32 isLive ) { mIsLive = isLive; }
	};

	typedef tRefCounterPtr< tEditorAction > tEditorActionPtr;

	///
	/// \brief Stores a history of actions, facilitating undo/redo.
	class tools_export tEditorActionStack
	{
	public:
		typedef tEvent<void ( tEditorActionStack& )> tNotifyEvent;
		tNotifyEvent mOnDirty;
		tNotifyEvent mOnAddAction;
		tNotifyEvent mOnUndo;
		tNotifyEvent mOnRedo;

	private:
		struct tCompoundAction : public tEditorAction
		{
			tEditorActionPtr mAction;
			tGrowableArray< tEditorActionPtr > mSubActions; 

			virtual void fUndo( );
			virtual void fRedo( );
			virtual void fOnBackOnTop( );
			virtual void fEnd( );
			virtual b32 fDirtyingAction( );
		};

		tRingBuffer< tEditorActionPtr > mActions;
		tRefCounterPtr< tCompoundAction > mCompoundAction;
		u32 mTotalActionCount;
		u32 mCurrentActionCount;
		b32 mDirty;

	public:

		explicit tEditorActionStack( u32 historySize = 100 );

		void fAddAction( const tEditorActionPtr& action );
		tEditorActionPtr fPopAction( );
		void fBeginCompoundAction( );
		void fEndCompoundAction( const tEditorActionPtr& action );

		void fForceSetDirty( b32 dirty = true ) { if( dirty ) fMarkDirty( ); else fClearDirty( ); }
		void fUndo( );
		void fRedo( );
		void fReset( );
		b32  fIsLive( );
		void fClearDirty( ) { mDirty = false; }
		inline b32 fIsDirty( ) const { return mDirty; }
		inline b32 fHasMoreUndos( ) const { return mCurrentActionCount > 0; }
		inline b32 fHasMoreRedos( ) const { return mTotalActionCount - mCurrentActionCount > 0; }

	private:

		void fMarkDirty( );
	};


}

#endif//__tEditorAction__
