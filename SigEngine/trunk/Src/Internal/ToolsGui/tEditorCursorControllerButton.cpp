#include "ToolsGuiPch.hpp"
#include "tEditorCursorControllerButton.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxToolsPanel.hpp"

namespace Sig
{
	tEditorCursorControllerButton::tEditorCursorControllerButton( 
		tEditorCursorControllerButtonGroup* parent, 
		const wxBitmap& selected, 
		const wxBitmap& deSelected,
		const char* toolTip )
		: tWxSlapOnRadioBitmapButton( parent, selected, deSelected, toolTip )
		, mParent( parent )
	{
	}

	tEditorCursorControllerButtonGroup::tEditorCursorControllerButtonGroup( 
		tWxToolsPanelTool* parent, 
		const char* label,
		b32 collapsible,
		u32 maxButtonsPerRow )
		: tWxSlapOnRadioBitmapButtonGroup( parent->fGetMainPanel( ), label, collapsible, maxButtonsPerRow )
		, mMainWindow( parent->fGuiApp( ).fMainWindow( ) )
	{
	}

	void tEditorCursorControllerButtonGroup::fOnSelChanged( )
	{
		const s32 selIndex = fGetSelected( );
		if( selIndex >= 0 && selIndex < ( s32 )mButtons.fCount( ) )
		{
			tEditorCursorControllerButton* ccb = dynamic_cast< tEditorCursorControllerButton* >( mButtons[ selIndex ] );
			if( ccb )
			{
				mMainWindow.fGuiApp( ).fSetCurrentCursor( ccb->fCreateCursorController( ) );

				// in case clearing the current selection just cleared our button, 
				// we explicitly set the selection again; we must pass false to avoid infinite recursion
				fSetSelected( selIndex, false );
			}
			else
			{
				log_warning( "Button pressed is not of type tEditorCursorControllerButton!" );
			}
		}
		//else
		//{
		//	// this should only happen in response to the current cursor changing elsewhere
		//}
	}


	tEditorButtonManagedCursorController::tEditorButtonManagedCursorController( tEditorCursorControllerButton* button )
		: tEditorCursorController( button->fGetParent( )->fMainWindow( ).fGuiApp( ) )
		, mButton( button )
	{
	}

	tEditorButtonManagedCursorController::tEditorButtonManagedCursorController( tToolsGuiApp & toolsApp )
		: tEditorCursorController( toolsApp )
		, mButton( 0 )
	{
	}

	void tEditorButtonManagedCursorController::fOnNextCursor( tEditorCursorController* nextController )
	{
		if( mButton && mButton->fGetParent( ) )
		{
			mButton->fOnNextCursorController( );
			mButton->fGetParent( )->fClearSelection( );
		}
		tEditorCursorController::fOnNextCursor( nextController );
	}


	tEditorButtonManagedCursorAction::tEditorButtonManagedCursorAction( tToolsGuiMainWindow& mainWindow )
		: mMainWindow( mainWindow )
		, mCursorButton( 0 )
	{
		tEditorButtonManagedCursorController* cursor = 
			mMainWindow.fGuiApp( ).fCurrentCursor( ).fNull( ) ? 0 : dynamic_cast< tEditorButtonManagedCursorController* >( mMainWindow.fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );
		if( cursor )
			mCursorButton = cursor->fGetButton( );
	}

	void tEditorButtonManagedCursorAction::fOnBackOnTop( )
	{
		if( mCursorButton && mCursorButton->fGetParent( ) )
			mCursorButton->fGetParent( )->fSetSelected( mCursorButton );
	}

}
