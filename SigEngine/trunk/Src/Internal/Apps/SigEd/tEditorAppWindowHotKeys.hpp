#ifndef __tEditorAppWindowHotKeys__
#define __tEditorAppWindowHotKeys__

#include "tEditableTileCanvas.hpp"


namespace Sig
{

	class tEditorNewHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorNewHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonN, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fNewDoc( ); }
	};
	class tEditorSaveHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorSaveHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonS, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fGuiApp( ).fSaveDoc( false ); }
	};
	class tEditorOpenHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorOpenHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonO, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fOpenDoc( ); }
	};
	class tEditorBrowseHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorBrowseHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonO, tEditorHotKey::cOptionCtrl | tEditorHotKey::cOptionShift ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fBrowseDoc( ); }
	};
	class tEditorCutHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorCutHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonX, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fCut( ); }
	};
	class tEditorCopyHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorCopyHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonC, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fCopy( ); }
	};
	class tEditorPasteHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorPasteHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonV, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fPaste( ); }
	};
	class tEditorDuplicateHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorDuplicateHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonD, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fDuplicateSelected( ); }
	};
	class tEditorGroupHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorGroupHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonG, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fGroupSelected( ); }
	};
	class tEditorBreakGroupHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorBreakGroupHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonG, tEditorHotKey::cOptionCtrl | tEditorHotKey::cOptionAlt ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fBreakSelected( ); }
	};
	class tEditorSelectAllHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorSelectAllHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonA, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fSelectAll( ); }
	};
	class tEditorBuildHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorBuildHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonB, tEditorHotKey::cOptionCtrl | tEditorHotKey::cOptionShift ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fBuild( ); }
	};
	class tEditorPreviewHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorPreviewHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonP, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fPreview( tEditorAppWindow::cPreviewBuildConfigInternal ); }
	};
	class tEditorPreviewPlaytestHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorPreviewPlaytestHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonP, tEditorHotKey::cOptionCtrl | tEditorHotKey::cOptionShift ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fPreview( tEditorAppWindow::cPreviewBuildConfigPlaytest ); }
	};

	class tEditorUndoHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorUndoHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonZ, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fUndo( ); }
	};
	class tEditorRedoHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorRedoHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonY, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fRedo( ); }
	};

	class tEditorFrameSelectedHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorFrameSelectedHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonF, 0 ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fFrameSelection( ); }
	};
	class tEditorFrameAllHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorFrameAllHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonA, 0 ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fFrameAll( ); }
	};
	class tEditorToggleObjectPropsHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorToggleObjectPropsHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonO, tEditorHotKeyEntry::cOptionCtrl | tEditorHotKeyEntry::cOptionShift ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fToggleObjectProperties( ); }
	};
	class tEditorToggleGlobalPropsHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorToggleGlobalPropsHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonG, tEditorHotKeyEntry::cOptionCtrl | tEditorHotKeyEntry::cOptionShift ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fToggleGlobalProperties( ); }
	};
	class tEditorToggleFindHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorToggleFindHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonF, tEditorHotKeyEntry::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fToggleObjectBrowser( ); }
	};

	class tEditorHideSelectedHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorHideSelectedHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonH, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fHideSelected( ); }
	};

	class tEditorHideUnselectedHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorHideUnselectedHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonH, tEditorHotKeyEntry::cOptionShift ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fHideUnselected( ); }
	};

	class tEditorUnhideAllHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorUnhideAllHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonH, tEditorHotKey::cOptionCtrl | tEditorHotKeyEntry::cOptionShift ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fUnhideAll( ); }
	};

	class tEditorToggleViewMode : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorToggleViewMode( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonSpace, 0 ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fToggleViewMode( ); }
	};

	class tEditorShiftLeftHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorShiftLeftHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonLeft, 0 ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const
		{
			tEditorSelectionList& selected = mEditorWindow->fGuiApp().fEditableObjects().fGetSelectionList();
			if( selected.fCount() == 0 )
				return;

			tEditableTileCanvas* tileCanvas = selected[0]->fDynamicCast< tEditableTileCanvas >();
			if( !tileCanvas )
				return;

			tileCanvas->fShiftHorizontal( 1 );
		}
	};
	class tEditorShiftRightHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorShiftRightHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonRight, 0 ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const
		{
			tEditorSelectionList& selected = mEditorWindow->fGuiApp().fEditableObjects().fGetSelectionList();
			if( selected.fCount() == 0 )
				return;

			tEditableTileCanvas* tileCanvas = selected[0]->fDynamicCast< tEditableTileCanvas >();
			if( !tileCanvas )
				return;

			tileCanvas->fShiftHorizontal( -1 );
		}
	};
	class tEditorShiftUpHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorShiftUpHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonUp, 0 ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const
		{
			tEditorSelectionList& selected = mEditorWindow->fGuiApp().fEditableObjects().fGetSelectionList();
			if( selected.fCount() == 0 )
				return;

			tEditableTileCanvas* tileCanvas = selected[0]->fDynamicCast< tEditableTileCanvas >();
			if( !tileCanvas )
				return;

			tileCanvas->fShiftVertical( 1 );
		}
	};
	class tEditorShiftDownHotKey : public tEditorHotKey
	{
		tEditorAppWindow* mEditorWindow;
	public:
		tEditorShiftDownHotKey( tEditorHotKeyTable& table, tEditorAppWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonDown, 0 ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const
		{
			tEditorSelectionList& selected = mEditorWindow->fGuiApp().fEditableObjects().fGetSelectionList();
			if( selected.fCount() == 0 )
				return;

			tEditableTileCanvas* tileCanvas = selected[0]->fDynamicCast< tEditableTileCanvas >();
			if( !tileCanvas )
				return;

			tileCanvas->fShiftVertical( -1 );
		}
	};
}


#endif//__tEditorAppWindowHotKeys__
