//------------------------------------------------------------------------------
// \file SigTileHotKeys.hpp - 21 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __SigTileHotKeys__
#define __SigTileHotKeys__

namespace Sig
{
	class tSigTileNewHotKey : public tEditorHotKey
	{
		tSigTileMainWindow* mMainWindow;
	public:
		tSigTileNewHotKey( tEditorHotKeyTable& table, tSigTileMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonN, tEditorHotKey::cOptionCtrl ), mMainWindow( editorWindow ) { }
		virtual void fFire( ) const { mMainWindow->fNewDoc( ); }
	};
	class tSigTileSaveHotKey : public tEditorHotKey
	{
		tSigTileMainWindow* mMainWindow;
	public:
		tSigTileSaveHotKey( tEditorHotKeyTable& table, tSigTileMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonS, tEditorHotKey::cOptionCtrl ), mMainWindow( editorWindow ) { }
		virtual void fFire( ) const { mMainWindow->fGuiApp( ).fSaveDoc( false ); }
	};
	class tSigTileOpenHotKey : public tEditorHotKey
	{
		tSigTileMainWindow* mMainWindow;
	public:
		tSigTileOpenHotKey( tEditorHotKeyTable& table, tSigTileMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonO, tEditorHotKey::cOptionCtrl ), mMainWindow( editorWindow ) { }
		virtual void fFire( ) const { mMainWindow->fOpenDoc( ); }
	};
	//class tEditorBuildHotKey : public tEditorHotKey
	//{
	//	tSigTileMainWindow* mMainWindow;
	//public:
	//	tEditorBuildHotKey( tEditorHotKeyTable& table, tSigTileMainWindow* editorWindow ) 
	//		: tEditorHotKey( table, Input::tKeyboard::cButtonB, tEditorHotKey::cOptionCtrl | tEditorHotKey::cOptionShift ), mMainWindow( editorWindow ) { }
	//	virtual void fFire( ) const { mMainWindow->fBuild( ); }
	//};

	class tSigTileUndoHotKey : public tEditorHotKey
	{
		tSigTileMainWindow* mMainWindow;
	public:
		tSigTileUndoHotKey( tEditorHotKeyTable& table, tSigTileMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonZ, tEditorHotKey::cOptionCtrl ), mMainWindow( editorWindow ) { }
		virtual void fFire( ) const { mMainWindow->fUndo( ); }
	};
	class tSigTileRedoHotKey : public tEditorHotKey
	{
		tSigTileMainWindow* mMainWindow;
	public:
		tSigTileRedoHotKey( tEditorHotKeyTable& table, tSigTileMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonY, tEditorHotKey::cOptionCtrl ), mMainWindow( editorWindow ) { }
		virtual void fFire( ) const { mMainWindow->fRedo( ); }
	};

	class tSigTileFrameHotKey : public tEditorHotKey
	{
		tSigTileMainWindow* mMainWindow;
	public:
		tSigTileFrameHotKey( tEditorHotKeyTable& table, tSigTileMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonF, 0 ), mMainWindow( editorWindow ) { }
		virtual void fFire( ) const { mMainWindow->fFrameCustom( ); }
	};

	class tSigTileToggleViewMode : public tEditorHotKey
	{
		tSigTileMainWindow* mMainWindow;
	public:
		tSigTileToggleViewMode( tEditorHotKeyTable& table, tSigTileMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonSpace, 0 ), mMainWindow( editorWindow ) { }
		virtual void fFire( ) const 
		{
			mMainWindow->fToggleViewMode( ); 
		}
	};

	class tSigTileRotateHotKey : public tEditorHotKey
	{
		tSigTileMainWindow* mMainWindow;
		b32 mCcw;
	public:
		tSigTileRotateHotKey( tEditorHotKeyTable& table, tSigTileMainWindow* editorWindow, u32 button, b32 ccw ) 
			: tEditorHotKey( table, button, 0 ), mMainWindow( editorWindow ) 
			, mCcw( ccw )
		{ }
		virtual void fFire( ) const 
		{
			mMainWindow->fTilePaintPanel( )->fRotateSelectedTile( mCcw );
		}
	};
}

#endif // __SigTileHotKeys__