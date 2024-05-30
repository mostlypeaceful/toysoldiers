#include "ToolsPch.hpp"
#include "tEditorAction.hpp"

namespace Sig
{
	tEditorActionStack::tEditorActionStack( u32 historySize )
		: mOnDirty( false )
		, mOnAddAction( false )
		, mOnUndo( false )
		, mOnRedo( false )
		, mActions( fMin( 2000u, fMax( 10u, historySize ) ) )
		, mTotalActionCount( 0 )
		, mCurrentActionCount( 0 )
		, mDirty( false )
	{
	}

	void tEditorActionStack::fAddAction( const tEditorActionPtr& action )
	{
		if( mCompoundAction )
		{
			mCompoundAction->mSubActions.fPushBack( action );
			return;
		}

		// note that adding an action severs the current tree of undos/redos,
		// we basically re-start from wherever we are in the stack; in other
		// words, all redos are left hanging, but we can still undo

		while( mActions.fNumItems( ) > mCurrentActionCount )
		{
			tEditorActionPtr tmp;
			mActions.fPopLIFO( tmp );
		}

		mActions.fPut( action );

		// reset counts
		mCurrentActionCount = mActions.fNumItems( );
		mTotalActionCount = mCurrentActionCount;

		if( action->fDirtyingAction( ) )
			fMarkDirty( );
		mOnAddAction.fFire( *this );
	}

	tEditorActionPtr tEditorActionStack::fPopAction( )
	{
		if( !fHasMoreUndos( ) ) 
			return tEditorActionPtr( );

		return mActions[ --mCurrentActionCount ];
	}

	void tEditorActionStack::fBeginCompoundAction( )
	{
		log_assert( !mCompoundAction, "System only supports a single compound action at a time :(" );
		mCompoundAction.fReset( new tCompoundAction( ) );
	}

	void tEditorActionStack::fEndCompoundAction( const tEditorActionPtr& action )
	{
		log_assert( mCompoundAction, "No compound action started" );
		mCompoundAction->mAction = action;
		mCompoundAction->fSetTitle( action->fTitle( ) );

		tEditorActionPtr compound( mCompoundAction.fGetRawPtr( ) );
		mCompoundAction.fRelease( );

		fAddAction( compound );
	}

	void tEditorActionStack::fUndo( )
	{
		if( !fHasMoreUndos( ) || fIsLive( ) )
			return;

		--mCurrentActionCount;
		mActions[ mCurrentActionCount ]->fOnBackOnTop( );
		mActions[ mCurrentActionCount ]->fUndo( );
		if( mActions[ mCurrentActionCount ]->fDirtyingAction( ) )
			fMarkDirty( );

		mOnUndo.fFire( *this );

		const std::string& title = mActions[ mCurrentActionCount ]->fTitle( );
		if( title.length( ) > 0 ) wxMessageBox( title + " undone.", "Undone." );
	}

	void tEditorActionStack::fRedo( )
	{
		if( !fHasMoreRedos( ) || fIsLive( ) )
			return;

		mActions[ mCurrentActionCount ]->fRedo( );
		mActions[ mCurrentActionCount ]->fOnBackOnTop( );
		if( mActions[ mCurrentActionCount ]->fDirtyingAction( ) )
			fMarkDirty( );

		mOnRedo.fFire( *this );

		const std::string& title = mActions[ mCurrentActionCount ]->fTitle( );
		if( title.length( ) > 0 ) wxMessageBox( title + " redone.", "Redone." );

		++mCurrentActionCount;
	}

	void tEditorActionStack::fReset( )
	{
		mActions = tRingBuffer< tEditorActionPtr >( mActions.fCapacity( ) );
		mTotalActionCount = 0;
		mCurrentActionCount = 0;
		mDirty = false;
	}

	b32 tEditorActionStack::fIsLive( )
	{
		if( mCurrentActionCount > 0 )
			return mActions[ mCurrentActionCount - 1 ]->fIsLive( );
		return false;
	}

	void tEditorActionStack::fMarkDirty( )
	{
		if( mDirty )
			return;
		mDirty = true;
		mOnDirty.fFire( *this );
	}


	void tEditorActionStack::tCompoundAction::fUndo( )
	{
		mAction->fUndo( );
		for( s32 i = mSubActions.fCount( )-1; i >= 0; --i )
			mSubActions[ i ]->fUndo( );
	}

	void tEditorActionStack::tCompoundAction::fRedo( )
	{
		for( u32 i = 0; i < mSubActions.fCount( ); ++i )
			mSubActions[ i ]->fRedo( );

		mAction->fRedo( );
	}

	void tEditorActionStack::tCompoundAction::fOnBackOnTop( )
	{
		for( u32 i = 0; i < mSubActions.fCount( ); ++i )
			mSubActions[ i ]->fOnBackOnTop( );

		mAction->fOnBackOnTop( );
	}

	void tEditorActionStack::tCompoundAction::fEnd( )
	{
		for( u32 i = 0; i < mSubActions.fCount( ); ++i )
			mSubActions[ i ]->fEnd( );

		mAction->fEnd( );
	}

	b32 tEditorActionStack::tCompoundAction::fDirtyingAction( )
	{
		return mAction->fDirtyingAction( );
	}


}

