#include "SigEdPch.hpp"
#include "tSigEdMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tWxToolsPanelContainer.hpp"
#include "tEditableObjectProperties.hpp"
#include "tRemapReferencesDialog.hpp"
#include "tSigEdExplicitDependenciesDialog.hpp"
#include "tFxFileRefEntity.hpp"
#include "tObjectBrowserDialog.hpp"

// specific tools panel tools
#include "tPlaceObjectCursor.hpp"
#include "tManipulateObjectPanel.hpp"
#include "tLayerPanel.hpp"
#include "tCreateObjectPanel.hpp"
#include "tReferenceObjectPanel.hpp"
#include "tPathToolsPanel.hpp"
#include "tHeightFieldVertexPaintPanel.hpp"
#include "tHeightFieldMaterialPaintPanel.hpp"
#include "DecalToolsPanels.hpp"
#include "tGroundCoverPanel.hpp"
#include "tNavGraphToolsPanel.hpp"

#include "tSearchableOpenFilesDialog.hpp"
#include <wx/listbase.h>

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

		cActionShowOnePanel,
		cActionShowFourPanels,

		cActionObjectProperties,
		cActionGlobalProperties,
		cActionRemapReferences,
		cActionExplicitDependencies,
		cActionObjectBrowser,
		cActionTextEditor,
	};

	BEGIN_EVENT_TABLE(tSigEdMainWindow, wxFrame)
		EVT_CLOSE(										tSigEdMainWindow::fOnClose)
		EVT_DROP_FILES(									tSigEdMainWindow::fOnFilesDropped)
		EVT_MENU(				cActionNew,				tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionOpen,			tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionSave,			tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionSaveAs,			tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionImport,			tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionPreview,			tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionPreviewRelease,	tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionPreviewProfile,	tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionBuild,			tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionQuit,			tSigEdMainWindow::fOnAction)

		EVT_MENU(				cActionUndo,			tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionRedo,			tSigEdMainWindow::fOnAction)

		EVT_MENU(				cActionAbout,			tSigEdMainWindow::fOnAction)

		EVT_MENU(				cActionFrameSel,		tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionFrameAll,		tSigEdMainWindow::fOnAction)

		EVT_MENU(				cActionShowOnePanel,	tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionShowFourPanels,	tSigEdMainWindow::fOnAction)

		EVT_MENU(				cActionObjectProperties,tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionGlobalProperties,tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionRemapReferences,	tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionExplicitDependencies,tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionObjectBrowser,	tSigEdMainWindow::fOnAction)
		EVT_MENU(				cActionTextEditor,		tSigEdMainWindow::fOnAction)

	END_EVENT_TABLE()

	tSigEdMainWindow::tSigEdMainWindow( tToolsGuiApp& guiApp )
		: tEditorAppWindow( guiApp )
		, mPlatformComboBox( 0 )
	{
		tRegistrySerializer::fLoad( );

		// If there is no previous default platform specified
		// in the registry, pick one to prevent a crash when
		// trying to preview
		if( mPreviewPlatform == cPlatformNone )
			mPreviewPlatform = cPlatformXbox360;

		// set icon
		SetIcon( wxIcon( "appicon" ) );

		// register "recently-opened file" menu actions
		for( u32 i = 0; i < tToolsGuiApp::cMaxRecentlyOpenedFiles; ++i )
			Connect( cActionOpenRecent + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(tSigEdMainWindow::fOnAction), 0, this );

		fAddMenus( );
		fAddToolbar( );
		fAddTools( );
		fLoadLayout( );

		SetFocus( );

		fNewDoc( );

		// set the window as accepting dropped files
		DragAcceptFiles( true );	
	}

	tSigEdMainWindow::~tSigEdMainWindow( )
	{
		
	}

	void tSigEdMainWindow::fSetupRendering( )
	{
		tEditorAppWindow::fSetupRendering( );
		mManipTools->fSetSelectionCursor( );

		// file to open must be opened after everything's ready for file opens
		const std::string& cmdLineBuffer = fGuiApp( ).fGetCmdLine( );
		const tCmdLineOption file( "file", cmdLineBuffer );
		if( file.fFound( ) )
			fOpenDoc( ToolsPaths::fMakeResAbsolute( tFilePathPtr( file.fGetTypedOption<std::string>( ) ) ) );
	}

	void tSigEdMainWindow::fOnTick( )
	{
		wxToolBar* mainToolBar = mRenderPanelContainer->fGetToolBar( );
		if( mainToolBar )
		{
			mainToolBar->EnableTool( cActionSave, fGuiApp( ).fActionStack( ).fIsDirty( )!=0 );
			mainToolBar->EnableTool( cActionUndo, fGuiApp( ).fActionStack( ).fHasMoreUndos( )!=0 );
			mainToolBar->EnableTool( cActionRedo, fGuiApp( ).fActionStack( ).fHasMoreRedos( )!=0 );
			mainToolBar->EnableTool( cActionFrameSel, fGuiApp( ).fSelectionList( ).fCount( ) > 0 );
		}

		tEditorAppWindow::fOnTick( );
	}

	void tSigEdMainWindow::fAddMenus( )
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

		// add "window" menu; might change this later
		{
			wxMenu *subMenu = new wxMenu;
			menuBar->Append( subMenu, "&Window" );

			subMenu->Append( cActionFrameSel, "&Frame Selected\tF" );
			subMenu->Append( cActionFrameAll, "Frame &All\tA" );
			subMenu->AppendSeparator();
			subMenu->Append( cActionObjectProperties, "&Object Properties\tCtrl+Shift+O" );
			subMenu->Append( cActionGlobalProperties, "&Global Properties\tCtrl+Shift+G" );
			subMenu->Append( cActionRemapReferences, "&Remap References" );
			subMenu->Append( cActionExplicitDependencies, "Explicit &Dependencies" );
			subMenu->AppendSeparator();
			subMenu->Append( cActionShowOnePanel, "Show One Panel" );
			subMenu->Append( cActionShowFourPanels, "Show Four Panel" );
		}

		// add help menu
		{
			wxMenu *menuHelp = new wxMenu;
			menuBar->Append( menuHelp, "&Help" );

			menuHelp->Append( cActionAbout, "&About..." );
		}

		SetMenuBar( menuBar );
	}

	void tSigEdMainWindow::fAddToolbar( )
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
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionObjectProperties, "ObjectProperties", wxBitmap( "objectproperties" ), wxNullBitmap, wxITEM_NORMAL, "Object Properties" );
		mainToolBar->AddTool( cActionGlobalProperties, "GlobalProperties", wxBitmap( "globalproperties" ), wxNullBitmap, wxITEM_NORMAL, "Global Properties" );
		mainToolBar->AddTool( cActionRemapReferences, "RemapReferences", wxBitmap( "remaprefs" ), wxNullBitmap, wxITEM_NORMAL, "Remap References" );
		mainToolBar->AddTool( cActionExplicitDependencies, "ExplicitDepenencies", wxBitmap( "explicitdepends" ), wxNullBitmap, wxITEM_NORMAL, "Explicit Dependencies" );
		mainToolBar->AddTool( cActionObjectBrowser, "ObjectBrowser", wxBitmap( "objectbrowser" ), wxNullBitmap, wxITEM_NORMAL, "Object Browser" );


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
		choices[ cPlatformPs3Spu - cPlatformFirst ] = wxString( "Ignore(Ps3Spu)" );
		choices[ cPlatformiOS - cPlatformFirst ] = wxString( "Ignore(iOS)" );
		sigassert( !mPlatformComboBox );
		mPlatformComboBox = new wxChoice( mainToolBar, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices.fCount( ), choices.fBegin( ) );
		mPlatformComboBox->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( tSigEdMainWindow::fOnPlatformChanged ), NULL, this );
		mainToolBar->AddControl( mPlatformComboBox );
		mPlatformComboBox->Select( mPreviewPlatform - cPlatformFirst );

		mainToolBar->Realize( );
	}

	void tSigEdMainWindow::fAddTools( )
	{
		// create misc panel and tools
		{
			tWxToolsPanel* panel = new tWxToolsPanel( mToolsPanelContainer, 320, wxColor( 0x55, 0x55, 0x55 ), wxColour( 0xff, 0xff, 0xff ) );
			mManipTools =	new tManipulateObjectPanel( panel );
							new tCreateObjectPanel( panel );
			mRefObjPanel =	new tReferenceObjectPanel( this, panel );
			mLayerPanel =	new tLayerPanel( panel );
							new tPathToolsPanel( this, panel );
							new tPathDecalToolsPanel( this, panel );
							new tNavGraphToolsPanel( this, panel );
							new tHeightFieldVertexPaintPanel( this, panel );
			mHeightFieldMaterials = new tHeightFieldMaterialPaintPanel( this, panel );
			mGCPanel = 		new tGroundCoverPanel( this, panel );
		}

		// Has to be created after the object browser is ready.
		mOpenDialog = new tSearchableOpenFilesDialog( this, mRefObjPanel->fGetBrowser( ), wxLC_SINGLE_SEL );

		tGrowableArray<const char*> additionalFilters;
		additionalFilters.fPushBack( ".sigml" );
		mOpenDialog->fSetAdditionalExtensionFilters( additionalFilters );

		mToolsPanelContainer->fAfterAllToolsPanelsAdded( );
	}

	void tSigEdMainWindow::fOnClose(wxCloseEvent& event)
	{
		tEditorAppWindow::fOnClose( );

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

	void tSigEdMainWindow::fOnAction(wxCommandEvent& event)
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
			case cActionSave:		fGuiApp( ).fSaveDoc( false ); break;
			case cActionSaveAs:		fGuiApp( ).fSaveDoc( true ); break;
			case cActionImport:		fImportDoc( ); break;
			case cActionPreview:	fPreview( false ); break;
			case cActionPreviewRelease:	fPreview( true ); break;
			case cActionPreviewProfile: fPreview( true, true ); break;
			case cActionBuild:		fBuild( ); break;
			case cActionQuit:		Close( false ); break;

				// edit menu
			case cActionUndo:		fUndo( ); break;
			case cActionRedo:		fRedo( ); break;

				// window menu
			case cActionFrameSel:	fFrameSelection( ); break;
			case cActionFrameAll:	fFrameAll( ); break;

			case cActionShowOnePanel: fRenderPanelContainer( )->fShowOne( ); break;
			case cActionShowFourPanels: fRenderPanelContainer( )->fShowFour( ); break;

			case cActionObjectProperties:
				fToggleObjectProperties( );
				break;
			case cActionGlobalProperties:
				fToggleGlobalProperties( );
				break;
			case cActionRemapReferences:
				mRemapReferencesDialog->fToggle( );
				break;
			case cActionExplicitDependencies:
				mExplicitDependenciesDialog->fToggle( );
				break;
			case cActionObjectBrowser:
				mObjectBrowserDialog->fToggle( );
				break;

				// help menu
			case cActionAbout:
				wxMessageBox( "SigEd is the uber content creation tool for SigEngine. Build worlds, levels, entities, all in one tool.", "About SigEd", wxOK | wxICON_INFORMATION );
				break;

			default: { log_warning( 0, "Unrecognized action!" ); }
				break;
			}
		}
	}

	void tSigEdMainWindow::fOnPlatformChanged(wxCommandEvent& event)
	{
		mPreviewPlatform = ( tPlatformId )( cPlatformFirst + mPlatformComboBox->GetSelection( ) );
		tRegistrySerializer::fSave( );
	}

	void tSigEdMainWindow::fOnRightClick( wxWindow* window, wxMouseEvent& event )
	{
		if( fGuiApp( ).fCurrentCursor( ) && dynamic_cast< tPlaceObjectCursorBase* >( fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) ) )
			mManipTools->fSetSelectionCursor( );
		else
			tEditorAppWindow::fOnRightClick( window, event );
	}

	std::string tSigEdMainWindow::fRegistryKeyName( ) const
	{
		return fGuiApp( ).fRegKeyName( ) + "\\" + ToolsPaths::fGetCurrentProjectName( );
	}
	void tSigEdMainWindow::fSaveInternal( HKEY hKey )
	{
		Win32Util::fSetRegistryKeyValue( hKey, reinterpret_cast<int&>( mPreviewPlatform ), "PreviewPlatform" );
	}
	void tSigEdMainWindow::fLoadInternal( HKEY hKey )
	{
		Win32Util::fGetRegistryKeyValue( hKey, reinterpret_cast<int&>( mPreviewPlatform ), "PreviewPlatform" );
	}

	void tSigEdMainWindow::fOnFilesDropped(wxDropFilesEvent& event)
	{
		// Open the first valid sigml.
		const u32 numFiles = event.GetNumberOfFiles( );
		const wxString* files = event.GetFiles( );
		for( u32 i = 0; i < numFiles; ++i )
		{
			const wxString thisFile = files[ i ];
			if( !StringUtil::fCheckExtension( thisFile.c_str( ), ".sigml" ) )
				continue;

			tFilePathPtr filePath( thisFile.c_str( ) );
			fOpenDoc( filePath );
			return;
		}
	}
}
