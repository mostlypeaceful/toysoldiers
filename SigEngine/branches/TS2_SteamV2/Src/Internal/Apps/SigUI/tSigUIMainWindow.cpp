#include "SigUIPch.hpp"
#include "tSigUIMainWindow.hpp"
#include "tToolsGuiApp.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tWxToolsPanel.hpp"
#include "tWxToolsPanelContainer.hpp"
#include "Editor/tEditableObjectContainer.hpp"

// specific tools panel tools
//#include "tManipulateObjectPanel.hpp"

namespace Sig
{
	enum tAction
	{
		cActionNew = 1,
		cActionOpen,
		cActionOpenRecent,
		cActionSave = cActionOpenRecent + tToolsGuiApp::cMaxRecentlyOpenedFiles,
		cActionSaveAs,
		cActionImport,
		cActionPreview,
		cActionPreviewRelease,
		cActionPreviewProfile,
		cActionBuild,
		cActionQuit,

		cActionUndo,
		cActionRedo,

		cActionAbout,

		cActionFrameSel,
		cActionFrameAll,

	};

	BEGIN_EVENT_TABLE(tSigUIMainWindow, wxFrame)
		EVT_CLOSE(										tSigUIMainWindow::fOnClose)
		EVT_MENU(				cActionNew,				tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionOpen,			tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionSave,			tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionSaveAs,			tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionImport,			tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionPreview,			tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionPreviewRelease,	tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionPreviewProfile,	tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionBuild,			tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionQuit,			tSigUIMainWindow::fOnAction)

		EVT_MENU(				cActionUndo,			tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionRedo,			tSigUIMainWindow::fOnAction)

		EVT_MENU(				cActionAbout,			tSigUIMainWindow::fOnAction)

		EVT_MENU(				cActionFrameSel,		tSigUIMainWindow::fOnAction)
		EVT_MENU(				cActionFrameAll,		tSigUIMainWindow::fOnAction)

	END_EVENT_TABLE()

	tSigUIMainWindow::tSigUIMainWindow( tToolsGuiApp& guiApp )
		: tToolsGuiMainWindow( guiApp )
		, mPreviewPlatform( cPlatformPcDx9 )
		, mPlatformComboBox( 0 )
		, mMainSizer( 0 )
		, mToolsPanelContainer( 0 )
	{
		// create primary window containers
		mRenderPanelContainer = new tWxRenderPanelContainer( this, guiApp.fRegKeyName( ) + "\\RenderPanels", true );
		mToolsPanelContainer = new tWxToolsPanelContainer( *this );

		// create main sizer
		mMainSizer = new wxBoxSizer( wxHORIZONTAL );
		SetSizer( mMainSizer );
		mMainSizer->Add( mRenderPanelContainer->fGetContainerPanel( ), 1, wxEXPAND | wxALL, 0 );
		mMainSizer->Add( mToolsPanelContainer->fGetContainerPanel( ), 0, wxEXPAND | wxALL, 0 );



		tRegistrySerializer::fLoad( );

		// set icon
		SetIcon( wxIcon( "appicon" ) );

		// register "recently-opened file" menu actions
		for( u32 i = 0; i < tToolsGuiApp::cMaxRecentlyOpenedFiles; ++i )
			Connect( cActionOpenRecent + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(tSigUIMainWindow::fOnAction), 0, this );

		fAddMenus( );
		fAddToolbar( );
		fAddTools( );
		fLoadLayout( );

		SetFocus( );

		fNewDoc( );
	}

	tSigUIMainWindow::~tSigUIMainWindow( )
	{
		
	}

	void tSigUIMainWindow::fSetupRendering( )
	{
		mRenderPanelContainer->fSetupRendering( fGuiApp( ) );

		fGuiApp( ).fEditableObjects( ).fReset(
			fGuiApp( ).fGfxDevice( ),
			mRenderPanelContainer->fGetSolidColorMaterial( ), 
			mRenderPanelContainer->fGetSolidColorGeometryAllocator( ),
			mRenderPanelContainer->fGetSolidColorIndexAllocator( ) );

		//mManipTools->fSetSelectionCursor( );
	}

	void tSigUIMainWindow::fOnTick( )
	{
		wxToolBar* mainToolBar = mRenderPanelContainer->fGetToolBar( );
		if( mainToolBar )
		{
			mainToolBar->EnableTool( cActionSave, fGuiApp( ).fActionStack( ).fIsDirty( )!=0 );
			mainToolBar->EnableTool( cActionUndo, fGuiApp( ).fActionStack( ).fHasMoreUndos( )!=0 );
			mainToolBar->EnableTool( cActionRedo, fGuiApp( ).fActionStack( ).fHasMoreRedos( )!=0 );
			mainToolBar->EnableTool( cActionFrameSel, fGuiApp( ).fSelectionList( ).fCount( ) > 0 );
		}

		f32 dt = 0.f;
		const b32 onTop = fBeginOnTick( &dt );

		if( onTop )
		{
			//// step dialog boxes
			//for( u32 i = 0; i < mEditorDialogs.fCount( ); ++i )
			//{
			//	mEditorDialogs[ i ]->fSetTopMost( true );
			//	mEditorDialogs[ i ]->fOnTick( );
			//}

			// step camera
			mRenderPanelContainer->fOnTick( );

			// step cursor
			if( !fGuiApp( ).fCurrentCursor( ).fNull( ) )
				fGuiApp( ).fCurrentCursor( )->fOnTick( );

			// step side bar guis
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

			//std::stringstream objectStats;
			//objectStats << "Objects: selected = " << fGuiApp( ).fSelectionList( ).fCount( ) << 
			//				" | hidden = " << fGuiApp( ).fEditableObjects( ).fGetHiddenCount( ) << 
			//				" | frozen = " << fGuiApp( ).fEditableObjects( ).fGetFrozenCount( ) << 
			//				" | total = " << fGuiApp( ).fEditableObjects( ).fGetObjectCount( );
			//SetStatusText( objectStats.str( ).c_str( ), 3 );
		}
		else
		{
			//for( u32 i = 0; i < mEditorDialogs.fCount( ); ++i )
			//	mEditorDialogs[ i ]->fSetTopMost( false );
			fSleep( 1 );
		}
	}

	void tSigUIMainWindow::fAddMenus( )
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
			subMenu->Append( cActionImport, "Import" );
			subMenu->AppendSeparator();

			mRecentFilesMenu = new wxMenu;
			mBaseRecentFileActionId = cActionOpenRecent;
			subMenu->AppendSubMenu( mRecentFilesMenu, "Recen&t Files" );
			fUpdateRecentFileMenu( );

			subMenu->AppendSeparator();
			subMenu->Append( cActionPreview, "&Preview Internal\tCtrl+P" );
			subMenu->Append( cActionPreviewRelease, "Preview Release\tCtrl+Shift+P" );
			subMenu->Append( cActionPreviewProfile, "Preview Profile" );
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

	void tSigUIMainWindow::fAddToolbar( )
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
		mainToolBar->AddTool( cActionFrameSel, "FrameSel", wxBitmap( "framesel" ), wxNullBitmap, wxITEM_NORMAL, "Frame Selected" );
		mainToolBar->AddTool( cActionFrameAll, "FrameAll", wxBitmap( "frameall" ), wxNullBitmap, wxITEM_NORMAL, "Frame All" );

		for( u32 i = 0; i < 6; ++i )
			mainToolBar->AddSeparator( );

		wxStaticText* text = new wxStaticText( mainToolBar, wxID_ANY, "Preview Platform:" );
		text->SetBackgroundColour( wxColour( 0xdd, 0xdd, 0xff, 0x00 ) );
		mainToolBar->AddControl( text );

		tDynamicArray<wxString> choices( cPlatformLastPlusOne - cPlatformFirst );
		choices[ cPlatformWii - cPlatformFirst ] = wxString( "Wii" );
		choices[ cPlatformPcDx9 - cPlatformFirst ] = wxString( "PcDx9" );
		choices[ cPlatformPcDx10 - cPlatformFirst ] = wxString( "PcDx10" );
		choices[ cPlatformXbox360 - cPlatformFirst ] = wxString( "Xbox360" );
		choices[ cPlatformPs3Ppu - cPlatformFirst ] = wxString( "Ps3" );
		sigassert( !mPlatformComboBox );
		mPlatformComboBox = new wxChoice( mainToolBar, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices.fCount( ), choices.fBegin( ) );
		mPlatformComboBox->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( tSigUIMainWindow::fOnPlatformChanged ), NULL, this );
		mainToolBar->AddControl( mPlatformComboBox );
		mPlatformComboBox->Select( mPreviewPlatform - cPlatformFirst );

		mainToolBar->Realize( );
	}

	void tSigUIMainWindow::fAddTools( )
	{
		// create misc panel and tools
		{
			tWxToolsPanel* panel = new tWxToolsPanel( mToolsPanelContainer, 320, wxColor( 0x55, 0x55, 0x55 ), wxColour( 0xff, 0xff, 0xff ) );
			//mManipTools =	new tManipulateObjectPanel( panel );
			//				new tCreateObjectPanel( panel );
			//				new tReferenceObjectPanel( this, panel );
			//mLayerPanel =	new tLayerPanel( panel );
			//				new tPathToolsPanel( this, panel );
			//				new tHeightFieldVertexPaintPanel( this, panel );
			//mHeightFieldMaterials = new tHeightFieldMaterialPaintPanel( this, panel );
		}

		mToolsPanelContainer->fAfterAllToolsPanelsAdded( );
	}

	void tSigUIMainWindow::fNewDoc( )
	{
		fGuiApp( ).fClearScene( );

		fSetStatus( "New scene" );
	}

	void tSigUIMainWindow::fOnClose(wxCloseEvent& event)
	{
		//for( u32 i = 0; i < mEditorDialogs.fCount( ); ++i )
		//	mEditorDialogs[ i ]->fSave( );

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

	void tSigUIMainWindow::fOnAction(wxCommandEvent& event)
	{
		const s32 id = event.GetId( );

		if( id >= cActionOpenRecent && id < cActionOpenRecent + tToolsGuiApp::cMaxRecentlyOpenedFiles )
		{
			// open recent action
			//fOpenRecent( id - cActionOpenRecent );
		}
		else
		{
			switch( id )
			{
				// file menu
			//case cActionNew:		fNewDoc( ); break;
			//case cActionOpen:		fOpenDoc( ); break;
			//case cActionSave:		fGuiApp( ).fSaveDoc( false ); break;
			//case cActionSaveAs:		fGuiApp( ).fSaveDoc( true ); break;
			//case cActionImport:		fImportDoc( ); break;
			//case cActionPreview:	fPreview( false ); break;
			//case cActionPreviewRelease:	fPreview( true ); break;
			//case cActionPreviewProfile: fPreview( true, true ); break;
			//case cActionBuild:		fBuild( ); break;
			case cActionQuit:		Close( false ); break;

				// edit menu
			//case cActionUndo:		fUndo( ); break;
			//case cActionRedo:		fRedo( ); break;

				// random
			//case cActionFrameSel:	fFrameSelection( ); break;
			//case cActionFrameAll:	fFrameAll( ); break;

				// help menu
			case cActionAbout:
				wxMessageBox( "SigEd is the uber content creation tool for SigEngine. Build worlds, levels, entities, all in one tool.", "About SigEd", wxOK | wxICON_INFORMATION );
				break;

			default: { log_warning( 0, "Unrecognized action!" ); }
				break;
			}
		}
	}

	void tSigUIMainWindow::fOnPlatformChanged(wxCommandEvent& event)
	{
		mPreviewPlatform = ( tPlatformId )( cPlatformFirst + mPlatformComboBox->GetSelection( ) );
		tRegistrySerializer::fSave( );
	}


	std::string tSigUIMainWindow::fRegistryKeyName( ) const
	{
		return fGuiApp( ).fRegKeyName( ) + "\\" + ToolsPaths::fGetCurrentProjectName( );
	}
	void tSigUIMainWindow::fSaveInternal( HKEY hKey )
	{
		Win32Util::fSetRegistryKeyValue( hKey, reinterpret_cast<int&>( mPreviewPlatform ), "PreviewPlatform" );
	}
	void tSigUIMainWindow::fLoadInternal( HKEY hKey )
	{
		Win32Util::fGetRegistryKeyValue( hKey, reinterpret_cast<int&>( mPreviewPlatform ), "PreviewPlatform" );
	}
}
