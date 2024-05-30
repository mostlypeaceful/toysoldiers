//------------------------------------------------------------------------------
// \file tSigTileMainWindow.hpp - 02 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tSigTileMainWindow__
#define __tSigTileMainWindow__
#include "tToolsGuiMainWindow.hpp"
#include "tTileCanvas.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tEditableTileDb.hpp"
#include "Editor/tEditorAction.hpp"

namespace Sig
{
	class tWxToolsPanelContainer;
	class tSigTileDialog;
	class tTilePaintPanel;
	class tTileDbPanel;
	class tDesignMarkupPanel;
	class tTileSetPigmentsDialog;
	class tScriptNodesDialog;

	enum tViewModes
	{
		cTiles,
		cModels,
		
		cModeCount
	};

	///
	/// \brief Top level, parent frame window containing all sub windows for the application.
	/// Basically a simple container that delegates to its children.
	class tSigTileMainWindow : public tToolsGuiMainWindow, public Win32Util::tRegistrySerializer
	{
		wxBoxSizer* mMainSizer;
		tWxToolsPanelContainer* mToolsPanelContainer;

		tEditorActionStack::tNotifyEvent::tObserver mOnDirty, mOnAddAction;

		tTileCanvasPtr mCanvas;
		tEditableTileDb mDatabase;

		tTilePaintPanel* mTilePanel;
		tTileDbPanel* mTileDbPanel;
		tDesignMarkupPanel* mMarkupPanel;

		tTileSetPigmentsDialog* mTileSetBrushesDialog;
		tScriptNodesDialog* mScriptNodesDialog;

		tGrowableArray<tSigTileDialog*> mEditorDialogs;

		tGrowableArray< tEditorHotKeyPtr > mHotKeyStorage;

		tViewModes mViewMode;

	public:
		tSigTileMainWindow( tToolsGuiApp& guiApp );
		~tSigTileMainWindow( );
		virtual void fSetupRendering( );
		virtual void fOnTick( );
		virtual void fClearScene( b32 closing );

		void fNewDoc( );
		void fOpenDoc( );
		void fOpenDoc( const tFilePathPtr& file );

		virtual void fFrameCustom( );

		void fUndo( );
		void fRedo( );

		void fToggleViewMode( );
		tViewModes fViewMode( ) const { return mViewMode; }

		void fOpenPalettesDialog( );
		void fOpenScriptNodesDialog( );

		tTileCanvasPtr			fCanvas( ) { return mCanvas; }
		tEditableTileDb*		fDataBase( ) { return &mDatabase; }
		tTilePaintPanel*		fTilePaintPanel( ) { return mTilePanel; }
		tTileDbPanel*			fTileDbPanel( ) { return mTileDbPanel; }
		tDesignMarkupPanel*		fMarkupPanel( ) { return mMarkupPanel; }

	private:

		void fAddMenus( );
		void fAddToolbar( );
		void fAddTools( );

		void fOnActionUndoOrRedo( tEditorActionStack& stack );
		void fOnDirty( tEditorActionStack& stack );
		void fOnClose(wxCloseEvent& event);
		void fOnAction(wxCommandEvent& event);
		void fOnPlatformChanged(wxCommandEvent& event);

		virtual std::string fRegistryKeyName( ) const;
		virtual void fSaveInternal( HKEY hKey );
		virtual void fLoadInternal( HKEY hKey );

		virtual std::string fEditableFileExt( ) const;

		void fOpenRecent( u32 ithRecentFile );
		virtual b32 fSerializeDoc( const tFilePathPtr& path );
		b32 fDeserializeDoc( const tFilePathPtr& path );

		void fOnFilesDropped(wxDropFilesEvent& event);

		DECLARE_EVENT_TABLE()
	};

}

#endif//__tSigTileMainWindow__
