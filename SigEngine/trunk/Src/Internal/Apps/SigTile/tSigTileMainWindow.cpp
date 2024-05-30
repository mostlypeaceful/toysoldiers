//------------------------------------------------------------------------------
// \file tSigTileMainWindow.cpp - 02 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigTilePch.hpp"
#include "tSigTileMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tWxToolsPanelContainer.hpp"
#include "tEditableObjectContainer.hpp"
#include "tWxRenderPanelGridSettings.hpp"
#include "tWxToolsPanel.hpp"
#include "tEditableTileCanvas.hpp"
#include "tTilePaintPanel.hpp"
#include "FileSystem.hpp"
#include "tEditorHotKey.hpp"
#include "SigTileHotKeys.hpp"
#include "tTilePaintBrushButton.hpp"
#include "tDesignMarkupPanel.hpp"
#include "tTileSetPigmentsDialog.hpp"
#include "tScriptNodeDialog.hpp"


namespace Sig
{
	enum tAction
	{
		cActionNew = 1,
		cActionOpen,
		cActionOpenRecent,
		cActionSave = cActionOpenRecent + tToolsGuiApp::cMaxRecentlyOpenedFiles,
		cActionSaveAs,
		cActionBuild,
		cActionQuit,

		cActionUndo,
		cActionRedo,

		cActionAbout,

		cActionCycleRandom,

		cActionScriptNodesDialog,
	};

	BEGIN_EVENT_TABLE(tSigTileMainWindow, wxFrame)
		EVT_CLOSE(										tSigTileMainWindow::fOnClose)
		EVT_DROP_FILES(									tSigTileMainWindow::fOnFilesDropped)
		EVT_MENU(				cActionNew,				tSigTileMainWindow::fOnAction)
		EVT_MENU(				cActionOpen,			tSigTileMainWindow::fOnAction)
		EVT_MENU(				cActionSave,			tSigTileMainWindow::fOnAction)
		EVT_MENU(				cActionSaveAs,			tSigTileMainWindow::fOnAction)
		EVT_MENU(				cActionBuild,			tSigTileMainWindow::fOnAction)
		EVT_MENU(				cActionQuit,			tSigTileMainWindow::fOnAction)

		EVT_MENU(				cActionUndo,			tSigTileMainWindow::fOnAction)
		EVT_MENU(				cActionRedo,			tSigTileMainWindow::fOnAction)

		EVT_MENU(				cActionAbout,			tSigTileMainWindow::fOnAction)

		EVT_MENU(				cActionCycleRandom,		tSigTileMainWindow::fOnAction)

		EVT_MENU(				cActionScriptNodesDialog, tSigTileMainWindow::fOnAction)
	END_EVENT_TABLE()

	tSigTileMainWindow::tSigTileMainWindow( tToolsGuiApp& guiApp )
		: tToolsGuiMainWindow( guiApp )
		, mMainSizer( NULL )
		, mToolsPanelContainer( NULL )
		, mTilePanel( NULL )
		, mTileDbPanel( NULL )
		, mScriptNodesDialog( NULL )
		, mViewMode( cTiles )
	{
		mRenderPanelContainer = new tWxRenderPanelContainer( this, guiApp.fRegKeyName( ) + "\\RenderPanels", true );
		mToolsPanelContainer = new tWxToolsPanelContainer( *this );

		// create main sizer
		mMainSizer = new wxBoxSizer( wxHORIZONTAL );
		SetSizer( mMainSizer );
		mMainSizer->Add( mRenderPanelContainer->fGetContainerPanel( ), 1, wxEXPAND | wxALL, 0 );
		mMainSizer->Add( mToolsPanelContainer->fGetContainerPanel( ), 0, wxEXPAND | wxALL, 0 );

		// create hot keys
		tEditorHotKeyTable& hotKeyTable = guiApp.fHotKeys( );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigTileNewHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigTileSaveHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigTileOpenHotKey( hotKeyTable, this ) ) );
		//mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorBuildHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigTileUndoHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigTileRedoHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigTileFrameHotKey( hotKeyTable, this ) ) );
		//mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigTileToggleViewMode( hotKeyTable, this ) ) );
		//mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigTileRotateHotKey( hotKeyTable, this, Input::tKeyboard::cButtonS, true ) ) );
		//mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigTileRotateHotKey( hotKeyTable, this, Input::tKeyboard::cButtonD, false ) ) );

		// set delegates
		mOnDirty.fFromMethod<tSigTileMainWindow, &tSigTileMainWindow::fOnDirty>( this );
		mOnAddAction.fFromMethod<tSigTileMainWindow, &tSigTileMainWindow::fOnActionUndoOrRedo>( this );
		fGuiApp( ).fActionStack( ).mOnDirty.fAddObserver( &mOnDirty );
		fGuiApp( ).fActionStack( ).mOnAddAction.fAddObserver( &mOnAddAction );
		fGuiApp( ).fActionStack( ).mOnUndo.fAddObserver( &mOnAddAction );
		fGuiApp( ).fActionStack( ).mOnRedo.fAddObserver( &mOnAddAction );


		//mScriptNodesDialog = new tScriptNodesDialog( this );
		//mEditorDialogs.fPushBack( mScriptNodesDialog );

		tRegistrySerializer::fLoad( );

		// set icon
		SetIcon( wxIcon( "appicon" ) );

		// register "recently-opened file" menu actions
		for( u32 i = 0; i < tToolsGuiApp::cMaxRecentlyOpenedFiles; ++i )
			Connect( cActionOpenRecent + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(tSigTileMainWindow::fOnAction), 0, this );

		fAddMenus( );
		fAddToolbar( );
		fAddTools( );
		fLoadLayout( );

		fNewDoc( );

		SetFocus( );

		// set the window as accepting dropped files
		DragAcceptFiles( true );	
	}

	tSigTileMainWindow::~tSigTileMainWindow( )
	{
		mHotKeyStorage.fDeleteArray( );
	}

	void tSigTileMainWindow::fSetupRendering( )
	{
		// TODO: improve screen config interface
		mRenderPanelContainer->fSetupRendering( fGuiApp( ) );
		mRenderPanelContainer->fGetFocusRenderPanel( )->fSetOrthoAndLookPos( Math::tVec3f::cZeroVector, Math::tVec3f::cYAxis, -Math::tVec3f::cZAxis );
		mRenderPanelContainer->fGetFocusRenderPanel( )->fDisableRotation( );
		mRenderPanelContainer->fGetFocusRenderPanel( )->fGetGridSettings( )->fUpdateGrid( );

		fGuiApp( ).fEditableObjects( ).fReset(
			fGuiApp( ).fGfxDevice( ), 
			mRenderPanelContainer->fGetSolidColorMaterial( ), 
			mRenderPanelContainer->fGetSolidColorGeometryAllocator( ),
			mRenderPanelContainer->fGetSolidColorIndexAllocator( ) );

		const tResourceDepotPtr& resourceDepot = fGuiApp( ).fEditableObjects( ).fGetResourceDepot( );
		mDatabase.fLoadTileAssets( resourceDepot );

		tGrowableArray< TileDb::tFile > loadedFiles;
		if( TileDb::tFile::fLoadTileDbs( loadedFiles ) )
		{
			for( u32 i = 0; i < loadedFiles.fCount( ); ++i )
			{
				if( !mDatabase.fDeserializeTileSets( loadedFiles[i], true, resourceDepot ) )
				{
					// Failure loading all tile sets. Close everything.
					Close( true );
				}
			}
		}
		else
		{
			mDatabase.fAddTileSet( resourceDepot, ToolsPaths::fMakeResAbsolute( tFilePathPtr( "Art/Dungeons/tileset_default" ) ) );
		}


		//mTileDbPanel->fOnSetUpRendering( );

		mCanvas.fReset( new tEditableTileCanvas( fGuiApp().fEditableObjects() ) );
		mCanvas->fSpawn( fGuiApp( ).fSceneGraph( )->fRootEntity( ) );
		mCanvas->fSetSize( 500, 500 );
		mCanvas->fAddToWorld();

		//for( u32 i = 0; i < mDatabase.fNumTileSets(); ++i )
		//	mCanvas->fAddTileSet( mDatabase.fTileSet(i)->fGuid(), 1.f );
	}

	void tSigTileMainWindow::fOnTick( )
	{
		f32 dt = 0.f;
		const b32 onTop = fBeginOnTick( &dt );

		if( onTop )
		{
			// step dialog boxes
			//for( u32 i = 0; i < mEditorDialogs.fCount( ); ++i )
			//	mEditorDialogs[ i ]->fOnTick( );

			// step camera
			mRenderPanelContainer->fOnTick( );

			// step cursor
			if( !fGuiApp( ).fCurrentCursor( ).fNull( ) )
				fGuiApp( ).fCurrentCursor( )->fOnTick( );

			// update any multi-segment objects
			fGuiApp( ).fEditableObjects( ).fUpdateObjectsTick( );

			// sidebar
			mToolsPanelContainer->fOnTick( );

			// step hot keys
			if( !fDialogInputActive( ) )
				fGuiApp( ).fHotKeys( ).fUpdateHotKeys( Input::tKeyboard::fInstance( ) );

			// clean the spatial set before rendering
			fGuiApp( ).fSceneGraph( )->fAdvanceTime( dt );
			fGuiApp( ).fSceneGraph( )->fKickCoRenderMTRunList( );

			// render active views of the scene
			Gfx::tDisplayStats selectedDisplayStats;
			fGuiApp( ).fSelectionList( ).fComputeSelectedDisplayStats( selectedDisplayStats );
			mRenderPanelContainer->fRender( &selectedDisplayStats );

			fSaveLayout( );
		}
		else
		{
			fSleep( 1 );
		}
	}

	void tSigTileMainWindow::fClearScene( b32 closing )
	{
		tToolsGuiMainWindow::fClearScene( closing );

		//mDatabase.fClearScriptNodeDefs( );
		//mScriptNodesDialog->fClear( );
		fSetStatus( "New scene" );

		//mTilePanel->fClearSelection( );
		fGuiApp( ).fSetCurrentCursor( tEditorCursorControllerPtr( NULL ) );

		// TODO: REMOVE
		if( mCanvas )
			mCanvas->fClear( );
	}

	void tSigTileMainWindow::fNewDoc( )
	{
		fGuiApp( ).fClearScene( );
	}

	void tSigTileMainWindow::fOpenDoc( )
	{
		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mRenderPanelContainer, 
			"Open Scene",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "untitled.tieml" ),
			wxString( "*.tieml" ),
			wxFD_OPEN ) );

		SetFocus( );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			if( !fGuiApp( ).fClearScene( ) )
				return; // user cancelled, don't try to open new file

			fOpenDoc( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );
		}
	}

	void tSigTileMainWindow::fOpenDoc( const tFilePathPtr& file )
	{
		if( !fGuiApp( ).fClearScene( ) )
			return; // user cancelled, don't try to open new file

		//mScriptNodesDialog->fClear( );

		if( FileSystem::fFileExists( file ) )
		{
			if( !fDeserializeDoc( file ) )
			{
				wxMessageBox( "The specified .tieml file is corrupt or out of date; open failed.", "Invalid File", wxOK | wxICON_WARNING );
				SetFocus( );
				fSetStatus( "Scene open failed" );
				return;
			}

			// set up new scene
			fGuiApp( ).fSetDocName( file.fCStr( ) );
			SetTitle( fGuiApp( ).fMakeWindowTitle( ) );
			fSetStatus( "Scene opened successfully" );

			fGuiApp( ).fAddRecentFile( file );
			fUpdateRecentFileMenu( );

			fFrameAllEveryViewport( );
		}
		else
		{
			wxMessageBox( "The specified file can not be found; open failed.", "File Not Found", wxOK | wxICON_WARNING );
			SetFocus( );
		}
	}

	void tSigTileMainWindow::fFrameCustom( )
	{
		tWxRenderPanel* renderPanel = mRenderPanelContainer->fGetActiveRenderPanel( );
		if( renderPanel )
		{
			renderPanel->fFrame( mCanvas->fComputeBounding( ) );
		}
	}

	void tSigTileMainWindow::fUndo( )
	{
		if( mViewMode == cTiles )
			fGuiApp( ).fActionStack( ).fUndo( );
	}

	void tSigTileMainWindow::fRedo( )
	{
		if( mViewMode == cTiles )
			fGuiApp( ).fActionStack( ).fRedo( );
	}

	void tSigTileMainWindow::fToggleViewMode( )
	{
		//if( mViewMode == cTiles )
		//{
		//	mRenderPanelContainer->fGetFocusRenderPanel( )->fSetProjectionType( Math::tVec3f( 0.f, 1.f, 1.f ).fNormalize( ), Math::tVec3f::cYAxis, Gfx::tLens::cProjectionPersp );

		//	s32 width = 0, height = 0;
		//	mRenderPanelContainer->fGetFocusRenderPanel( )->GetClientSize( &width, &height );
		//	mRenderPanelContainer->fGetFocusRenderPanel( )->fResetProjectionMatrices( width, height );
		//	mRenderPanelContainer->fGetFocusRenderPanel( )->fDisableRotation( false );
		//	mCanvas->fSetViewMode( true );

		//	tTileBrushBase* cursor = dynamic_cast<tTileBrushBase*>( fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );
		//	if( cursor )
		//		cursor->fHideCursor( );

		//	mViewMode = cModels;
		//}
		//else if( mViewMode == cModels )
		//{ 
		//	mRenderPanelContainer->fGetFocusRenderPanel( )->fSetProjectionType( Math::tVec3f::cYAxis, -Math::tVec3f::cZAxis, Gfx::tLens::cProjectionOrtho );

		//	s32 width = 0, height = 0;
		//	mRenderPanelContainer->fGetFocusRenderPanel( )->GetClientSize( &width, &height );
		//	mRenderPanelContainer->fGetFocusRenderPanel( )->fResetProjectionMatrices( width, height );
		//	mRenderPanelContainer->fGetFocusRenderPanel( )->fDisableRotation( );
		//	mCanvas->fSetViewMode( false );

		//	tTileBrushBase* cursor = dynamic_cast<tTileBrushBase*>( fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );
		//	if( cursor )
		//		cursor->fShowCursor( );

		//	mViewMode = cTiles;
		//}
	}

	void tSigTileMainWindow::fOpenScriptNodesDialog( )
	{
		//mScriptNodesDialog->Show( true );
	}

	void tSigTileMainWindow::fAddMenus( )
	{
		wxMenuBar *menuBar = new wxMenuBar;

		// add file menu
		{
			wxMenu *subMenu = new wxMenu;
			menuBar->Append( subMenu, "&File" );

			subMenu->Append( cActionNew, "&New\tCtrl+N" );
			subMenu->Append( cActionOpen, "&Open...\tCtrl+O" );
			subMenu->Append( cActionSave, "&Save\tCtrl+S" );
			subMenu->Append( cActionSaveAs, "Save &As..." );
			subMenu->AppendSeparator();

			mRecentFilesMenu = new wxMenu;
			mBaseRecentFileActionId = cActionOpenRecent;
			subMenu->AppendSubMenu( mRecentFilesMenu, "Recen&t Files" );
			fUpdateRecentFileMenu( );

			subMenu->AppendSeparator();
			subMenu->Append( cActionBuild, "&Build\tCtrl+Shift+B" );

			subMenu->AppendSeparator();
			subMenu->Append( cActionQuit, "E&xit" );
		}

		// add edit menu
		{
			wxMenu *subMenu = new wxMenu;
			menuBar->Append( subMenu, "&Edit" );

			subMenu->Append( cActionUndo, "&Undo\tCtrl+Z" );
			subMenu->Append( cActionRedo, "&Redo\tCtrl+Y" );
		}

		// add help menu
		{
			wxMenu *menuHelp = new wxMenu;
			menuBar->Append( menuHelp, "&Help" );

			menuHelp->Append( cActionAbout, "&About..." );
		}

		SetMenuBar( menuBar );
	}

	void tSigTileMainWindow::fAddToolbar( )
	{
		// setup primary/default toolbar
		wxToolBar* mainToolBar = mRenderPanelContainer->fGetToolBar( );
		mainToolBar->SetToolBitmapSize( wxSize( 16, 16 ) );
		mainToolBar->AddTool( cActionNew, "New Scene", wxBitmap( "newdoc" ), wxNullBitmap, wxITEM_NORMAL, "Create a new, empty scene" );
		mainToolBar->AddTool( cActionOpen, "Open Scene", wxBitmap( "opendoc" ), wxNullBitmap, wxITEM_NORMAL, "Load an existing scene" );
		mainToolBar->AddTool( cActionSave, "Save Scene", wxBitmap( "savedoc" ), wxNullBitmap, wxITEM_NORMAL, "Save current scene" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionUndo, "Undo", wxBitmap( "undo" ), wxNullBitmap, wxITEM_NORMAL, "Undo last action" );
		mainToolBar->AddTool( cActionRedo, "Redo", wxBitmap( "redo" ), wxNullBitmap, wxITEM_NORMAL, "Redo previous action" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionAbout, "About", wxBitmap( "help" ), wxNullBitmap, wxITEM_NORMAL, "Get help" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionCycleRandom, "Bake randoms", wxBitmap( "bakerandom" ), wxNullBitmap, wxITEM_NORMAL, "Pick new random tiles to preview" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionScriptNodesDialog, "Toggle Script Palette Dialog", wxBitmap( "tilesetbrushdialog" ), wxNullBitmap, wxITEM_NORMAL, "Open the script palettes dialog" );

		mainToolBar->Realize( );
	}

	void tSigTileMainWindow::fAddTools( )
	{
		tWxToolsPanel* panel	= new tWxToolsPanel( mToolsPanelContainer, 320, wxColor( 0x55, 0x55, 0x55 ), wxColour( 0xff, 0xff, 0xff ) );
		//mTileDbPanel			= new tTileDbPanel( this, panel );
		//mTilePanel				= new tTilePaintPanel( panel, this );
		//mMarkupPanel			= new tDesignMarkupPanel( panel, this );

		//mTileDbPanel->fSetPaintPanel( mTilePanel );

		mToolsPanelContainer->fAfterAllToolsPanelsAdded( );
	}

	void tSigTileMainWindow::fOnActionUndoOrRedo( tEditorActionStack& stack )
	{
	}

	void tSigTileMainWindow::fOnDirty( tEditorActionStack& stack )
	{
		SetTitle( fGuiApp( ).fMakeWindowTitle( ) );
	}

	void tSigTileMainWindow::fOnClose(wxCloseEvent& event)
	{
		// clear scene before closing; this will prompt
		// the user to save any changes if the scene is dirty
		if( fGuiApp( ).fClearScene( true ) )
		{
			PostQuitMessage( 0 );
			event.Skip( );
		}
		else
		{
			event.Veto( );
		}
	}

	void tSigTileMainWindow::fOnAction(wxCommandEvent& event)
	{
		const s32 id = event.GetId( );

		if( id >= cActionOpenRecent && id < cActionOpenRecent + tToolsGuiApp::cMaxRecentlyOpenedFiles )
		{
			// open recent action
			fOpenRecent( id - cActionOpenRecent );
		}
		else
		{	
			switch( id )
			{
				// file menu
			case cActionNew:		fNewDoc( ); break;
			case cActionOpen:		fOpenDoc( ); break;
			case cActionSave:		fGuiApp( ).fSaveDoc( false ); /*mTileDbPanel->fOnSave( );*/ break;
			case cActionSaveAs:		fGuiApp( ).fSaveDoc( true ); /*mTileDbPanel->fOnSave( );*/ break;
			//case cActionBuild:		fBuild( ); break;
			case cActionQuit:		Close( false ); break;

				// edit menu
			case cActionUndo:		fUndo( ); break;
			case cActionRedo:		fRedo( ); break;

			case cActionCycleRandom:	mCanvas->fPreviewRandomized( &mDatabase ); break;

			//case cActionScriptNodesDialog: mScriptNodesDialog->fToggle( ); break;

				// help menu
			case cActionAbout:
				wxMessageBox( "SigTile is used to make modular game play areas using sets of tiles.", "About SigTile", wxOK | wxICON_INFORMATION );
				break;

			default: { log_warning( "Unrecognized action!" ); }
				break;
			}
		}
	}


	std::string tSigTileMainWindow::fRegistryKeyName( ) const
	{
		return fGuiApp( ).fRegKeyName( ) + "\\" + ToolsPaths::fGetCurrentProjectName( );
	}
	void tSigTileMainWindow::fSaveInternal( HKEY hKey )
	{
	}
	void tSigTileMainWindow::fLoadInternal( HKEY hKey )
	{
	}

	std::string tSigTileMainWindow::fEditableFileExt( ) const
	{
		return std::string( "null" );
	}

	void tSigTileMainWindow::fOpenRecent( u32 ithRecentFile )
	{
		const Win32Util::tRecentlyOpenedFileList& recentFiles = fGuiApp( ).fRecentFiles( );
		fOpenDoc( recentFiles[ ithRecentFile ] );
	}

	b32 tSigTileMainWindow::fSerializeDoc( const tFilePathPtr& path )
	{
		//Tieml::tFile tiemlFile;

		//fGuiApp( ).fEditableObjects( ).fSerialize( tiemlFile.mTiles );
		//mDatabase.fSerializeScriptNodeDefs( tiemlFile );

		//b32 success = tiemlFile.fSaveXml( path, true );

		//SetFocus( );

		return true;
	}

	b32 tSigTileMainWindow::fDeserializeDoc( const tFilePathPtr& path )
	{
		//Tieml::tFile tiemlFile;
		//if( !tiemlFile.fLoadXml( path ) )
		//	return false;

		// Palettes must be deserialized before tiles.
		//mDatabase.fDeserializeScriptNodeDefs( tiemlFile, fGuiApp( ).fEditableObjects( ).fGetResourceDepot( ) );
		//mScriptNodesDialog->fRebuildNodes( );

		//tEditableObjectContainer& edObjs = fGuiApp( ).fEditableObjects( );

		//const b32 success = edObjs.fDeserialize( tiemlFile.mTiles );
		//SetFocus( );

		return true;
	}

	void tSigTileMainWindow::fOnFilesDropped(wxDropFilesEvent& event)
	{
		// TODO
		//// Open the first valid sigml.
		//const u32 numFiles = event.GetNumberOfFiles( );
		//const wxString* files = event.GetFiles( );
		//for( u32 i = 0; i < numFiles; ++i )
		//{
		//	const wxString thisFile = files[ i ];
		//	if( !StringUtil::fCheckExtension( thisFile.c_str( ), ".sigml" ) )
		//		continue;

		//	tFilePathPtr filePath( thisFile.c_str( ) );
		//	//fOpenDoc( filePath );
		//	return;
		//}
	}
}
