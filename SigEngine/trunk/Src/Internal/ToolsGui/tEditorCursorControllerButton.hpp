#ifndef __tEditorCursorControllerButton__
#define __tEditorCursorControllerButton__
#include "tWxSlapOnRadioBitmapButton.hpp"
#include "tEditorCursorController.hpp"
#include "Editor/tEditorAction.hpp"

namespace Sig
{
	class tToolsGuiMainWindow;
	class tWxToolsPanelTool;
	class tEditorCursorControllerButtonGroup;

	///
	/// \brief Base type for buttons that live on a tools panel which modify
	/// the current tEditorCursorController object. I.e., when one of these buttons
	/// are pressed, all other buttons of this type need to become de-selected, and the
	/// global cursor object needs to change. This type facilitates that stuff.
	class toolsgui_export tEditorCursorControllerButton : public tWxSlapOnRadioBitmapButton
	{
		tEditorCursorControllerButtonGroup* mParent;
	public:
		tEditorCursorControllerButton( 
			tEditorCursorControllerButtonGroup* parent, 
			const wxBitmap& selected, 
			const wxBitmap& deSelected,
			const char* toolTip );
		virtual tEditorCursorControllerPtr fCreateCursorController( ) = 0;
		virtual void fOnNextCursorController( ) { }
		inline tEditorCursorControllerButtonGroup* fGetParent( ) { return mParent; }
	};

	///
	/// \brief The container gui object group for objects of type tEditorCursorControllerButton.
	class toolsgui_export tEditorCursorControllerButtonGroup : public tWxSlapOnRadioBitmapButtonGroup
	{
		tToolsGuiMainWindow& mMainWindow;
	public:
		tEditorCursorControllerButtonGroup( 
			tWxToolsPanelTool* parent, 
			const char* label,
			b32 collapsible,
			u32 maxButtonsPerRow = 4 );
		virtual void fOnSelChanged( );
		inline tToolsGuiMainWindow& fMainWindow( ) { return mMainWindow; }
	};


	///
	/// \brief Cursor types that are enabled/disabled by a tEditorCursorControllerButton
	/// should derived from this type in order to benefit from guaranteed tools panel
	/// button state enabling/disabling when the cursor is set from external locations.
	class toolsgui_export tEditorButtonManagedCursorController : public tEditorCursorController
	{
	protected:
		tEditorCursorControllerButton* mButton;
	public:
		tEditorButtonManagedCursorController( tEditorCursorControllerButton* button );
		tEditorButtonManagedCursorController( tToolsGuiApp & toolsApp );

		virtual void fOnNextCursor( tEditorCursorController* nextController );
		inline tEditorCursorControllerButton* fGetButton( ) { return mButton; }
	};


	///
	/// \brief Base class for editor actions that desire to reset the current cursor
	/// controller to whatever it was at the time of the action.
	class toolsgui_export tEditorButtonManagedCursorAction : public tEditorAction
	{
		tToolsGuiMainWindow&				mMainWindow;
		tEditorCursorControllerButton*		mCursorButton;
	public:
		tEditorButtonManagedCursorAction( tToolsGuiMainWindow& mainWindow );
		inline tToolsGuiMainWindow& fMainWindow( ) { return mMainWindow; }
		virtual void fOnBackOnTop( );
	};

	///
	/// \brief Hot key class for enabling a cursor controller button via a key combination.
	class toolsgui_export tEnableCursorButtonHotKey : public tEditorHotKey
	{
		tEditorCursorControllerButton* mButton;
		b32 mClearSelFirst;
	public:
		tEnableCursorButtonHotKey( tEditorCursorControllerButton* button, tEditorHotKeyTable& table, Input::tKeyboard::tButton kbButton, u32 opts = 0, b32 clearSelFirst = false ) 
			: tEditorHotKey( table, kbButton, opts ), mButton( button ), mClearSelFirst( clearSelFirst ) { }
		virtual void fFire( ) const { if( mClearSelFirst ) mButton->fGetParent( )->fClearSelection( ); mButton->fGetParent( )->fSetSelected( mButton ); }
	};

}

#endif//__tEditorCursorControllerButton__

