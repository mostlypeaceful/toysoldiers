#include "ToolsGuiPch.hpp"
#include "tManipulationGizmoAction.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tToolsGuiApp.hpp"
#include "Editor/tEditableObject.hpp"

namespace Sig
{

	tManipulationGizmoAction::tManipulationGizmoAction( tToolsGuiMainWindow& mainWindow, const tEditorSelectionList& selectedObjects )
		: tEditorButtonManagedCursorAction( mainWindow )
		, mSelectedObjects( selectedObjects )
	{
		fSetIsLive( true );

		mUndoXforms.fNewArray( mSelectedObjects.fCount( ) );
		mRedoXforms.fNewArray( mSelectedObjects.fCount( ) );

		// save the current object state for undo
		fSaveXforms( mUndoXforms );
	}

	void tManipulationGizmoAction::fEnd( )
	{
		// save the new object state for redo
		fSaveXforms( mRedoXforms );
		tEditorAction::fSetIsLive( false );
	}
	void tManipulationGizmoAction::fUndo( )
	{
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fReset( mSelectedObjects );
		fResetXforms( mUndoXforms );
	}
	void tManipulationGizmoAction::fRedo( )
	{
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fReset( mSelectedObjects );
		fResetXforms( mRedoXforms );
	}
	void tManipulationGizmoAction::fSaveXforms( tDynamicArray< Math::tMat3f >& xforms )
	{
		sigassert( xforms.fCount( ) == mSelectedObjects.fCount( ) );
		for( u32 i = 0; i < xforms.fCount( ); ++i )
			xforms[ i ] = mSelectedObjects[ i ]->fDynamicCast< tEditableObject >( )->fObjectToWorld( );
	}
	void tManipulationGizmoAction::fResetXforms( const tDynamicArray< Math::tMat3f >& xforms )
	{
		sigassert( xforms.fCount( ) == mSelectedObjects.fCount( ) );
		for( u32 i = 0; i < xforms.fCount( ); ++i )
			mSelectedObjects[ i ]->fDynamicCast< tEditableObject >( )->fMoveTo( xforms[ i ] );
	}

}

