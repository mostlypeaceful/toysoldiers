#include "ShadePch.hpp"
#include "tShadeMainWindow.hpp"
#include "tShadeControlPanel.hpp"
#include "tMatEdMainWindow.hpp"
#include "tStrongPtr.hpp"
#include "FileSystem.hpp"
#include "tAssetGenScanner.hpp"
#include "Derml.hpp"
#include "HlslGen/tHlslGenTree.hpp"
#include "HlslGen/tHlslWriter.hpp"
#include "editor/tHlslViewerDialog.hpp"

namespace Sig
{
	enum tAction
	{
		cActionNewDoc = 1,
		cActionOpenDoc,
		cActionOpenRecent,
		cActionSave = cActionOpenRecent + tShadeMainWindow::cMaxRecentlyOpenedFiles,
		cActionSaveAs,
		cActionBuild,
		cActionQuit,

		cActionUndo,
		cActionRedo,
		cActionCopy,
		cActionPaste,

		cActionAbout,

		cActionFrameSelected,
		cActionFrameAll,
		cActionToggleMatEd,
		cActionRefreshMatEd,
	};

	namespace
	{
		const std::string gGoamlExt = Derml::fGetFileExtension( );
	}

	const char tShadeMainWindow::cNewDocTitle[] = "(untitled)";

	tShadeMainWindow::tShadeMainWindow( )
		: wxFrame( (wxFrame *)NULL, -1, "SigShade" )
		, mSavedLayout( ToolsPaths::fGetSignalRegistryKeyName( ) + "\\Shade\\MainWindow" )
		, mToolBar( 0 )
		, mVsStyle( 0 )
		, mHlslPreviewPlatform( 0 )
		, mCanvas( 0 )
		, mControlPanel( 0 )
		, mMatEd( 0 )
		, mRecentFilesMenu( 0 )
		, mDocName( cNewDocTitle )
		, mRecentFiles( ToolsPaths::fGetSignalRegistryKeyName( ) + "\\Shade\\MainWindow\\" + ToolsPaths::fGetCurrentProjectName( ) + "\\RecentFiles" )
	{
		Log::fSetLogFilterMask( Log::cFlagNetwork );

		SetIcon( wxIcon( "appicon" ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );
		Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( tShadeMainWindow::fOnClose ) );
		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tShadeMainWindow::fOnAction ) );
		Connect( wxEVT_IDLE, wxIdleEventHandler( tShadeMainWindow::fOnIdle ) );

		mRecentFiles.fLoad( );
		fCreateMainMenu( );
		fAddToolbar( );

		// status bar...
		const int statusWidths[] = { 0, 60, -1 };
		wxStatusBar* statusBar = CreateStatusBar( array_length(statusWidths), wxST_SIZEGRIP|wxFULL_REPAINT_ON_RESIZE, 0, wxT("Status:") );
		SetStatusWidths( array_length(statusWidths), statusWidths );
		SetStatusText( "      Status:", 1 );

		// create canvas, control panel, and material editor
		mCanvas = new tShadeNodeCanvas( this );
		mControlPanel = new tShadeControlPanel( this, mCanvas );
		mMatEd = new tMatEdMainWindow( this, mCanvas->fEditorActions( ), ToolsPaths::fGetSignalRegistryKeyName( ) + "\\Shade\\MatEd", false, false, true );
		mMatEd->fSetupPreviewWindow( );
		mMatPreview.fReset( new tMaterialPreviewBundle( mMatEd->fDevice( ) ) );
		mMatEd->fSetPreviewBundle( mMatPreview );

		// add control panel
		wxSizer* mainSizer = new wxBoxSizer( wxHORIZONTAL );
		GetSizer( )->Add( mainSizer, 1, wxEXPAND | wxALL );
		mainSizer->Add( mControlPanel, 0, wxALIGN_LEFT | wxEXPAND | wxALL );

		// add node canvas...
		mainSizer->Add( mCanvas, 1, wxALIGN_LEFT | wxEXPAND | wxALL );
		SetBackgroundColour( mCanvas->fBgColor( ) );

		// load saved UI settings
		fLoadLayout( );

		// set delegates
		mOnDirty.fFromMethod<tShadeMainWindow, &tShadeMainWindow::fOnDirty>( this );
		mOnAddAction.fFromMethod<tShadeMainWindow, &tShadeMainWindow::fOnActionUndoOrRedo>( this );
		mOnSelChanged.fFromMethod<tShadeMainWindow,&tShadeMainWindow::fOnSelChanged>( this );
		mCanvas->mOnSelChanged.fAddObserver( &mOnSelChanged );
		mCanvas->fEditorActions( ).mOnDirty.fAddObserver( &mOnDirty );
		mCanvas->fEditorActions( ).mOnAddAction.fAddObserver( &mOnAddAction );
		mCanvas->fEditorActions( ).mOnUndo.fAddObserver( &mOnAddAction );
		mCanvas->fEditorActions( ).mOnRedo.fAddObserver( &mOnAddAction );

		fNewDoc( );

		mCanvas->SetFocus( );
	}
	tShadeMainWindow::~tShadeMainWindow( )
	{
		delete mMatEd;
		mMatEd = NULL;
	}
	void tShadeMainWindow::fSetStatus( const char* status )
	{
		SetStatusText( status, 2 );
	}
	void tShadeMainWindow::fLoadLayout( )
	{
		if( mSavedLayout.fLoad( ) && mSavedLayout.mVisible )
		{
			mSavedLayout.fToWxWindow( this );
			if( mSavedLayout.mMaximized )
				Maximize( );
		}
		else
		{
			SetSize( 1024, 768 );
			Center( );
			Maximize( );
			Show( true );
		}
	}
	void tShadeMainWindow::fSaveLayout( )
	{
		mMatEd->fSave( );

		if( IsIconized( ) || !IsShown( ) )
			return; // window is minimized, don't save

		tWxSavedLayout layout( mSavedLayout.fRegistryKeyName( ) );
		layout.fFromWxWindow( this );

		if( layout.fIsInBounds( 2048 ) && layout != mSavedLayout )
		{
			layout.fSave( );
			mSavedLayout = layout;
		}
		else if( layout.mMaximized && !mSavedLayout.mMaximized )
		{
			// when maximized, the result of the layout settings are kind of screwy,
			// so we just set the maximized flag and save whatever the previous settings were
			mSavedLayout.mMaximized = true;
			mSavedLayout.fSave( );
		}
	}
	void tShadeMainWindow::fOnDirty( tEditorActionStack& stack )
	{
		SetTitle( fMakeWindowTitle( ) );
	}
	void tShadeMainWindow::fOnActionUndoOrRedo( tEditorActionStack& stack )
	{
		// TODO: refresh stuff that needs it
	}
	void tShadeMainWindow::fCreateMainMenu( )
	{
		wxMenuBar* mainMenu = new wxMenuBar;

		{
			wxMenu* subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&File" );

			subMenu->Append( cActionNewDoc, "&New\tCtrl+N" );
			subMenu->Append( cActionOpenDoc, "&Open...\tCtrl+O" );
			subMenu->Append( cActionSave, "&Save\tCtrl+S" );
			subMenu->Append( cActionSaveAs, "Save &As..." );
			subMenu->AppendSeparator();

			mRecentFilesMenu = new wxMenu;
			subMenu->AppendSubMenu( mRecentFilesMenu, "Recen&t Files" );
			fUpdateRecentFileMenu( );

			subMenu->AppendSeparator( );
			subMenu->Append( cActionBuild, "&Build\tCtrl+Shift+B" );

			subMenu->AppendSeparator( );
			subMenu->Append( cActionQuit, "E&xit" );
		}
		{
			wxMenu *subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&Edit" );

			subMenu->Append( cActionUndo, "&Undo\tCtrl+Z" );
			subMenu->Append( cActionRedo, "&Redo\tCtrl+Y" );

			subMenu->AppendSeparator( );
			subMenu->Append( cActionCopy, "&Copy\tCtrl+C" );
			subMenu->Append( cActionPaste, "&Paste\tCtrl+V" );
		}
		// add "window" menu; might change this later
		{
			wxMenu *subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&Window" );

			subMenu->Append( cActionFrameSelected, "Frame &Selected\t F" );
			subMenu->Append( cActionFrameAll, "Frame &All\t A" );
			subMenu->AppendSeparator();
			subMenu->Append( cActionToggleMatEd, "&Material Preview\tCtrl+Shift+M" );
			subMenu->Append( cActionRefreshMatEd, "&Refresh Material Preview\tF5" );
		}
		{
			wxMenu* subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&Help" );

			subMenu->Append( cActionAbout, "&About..." );
		}

		SetMenuBar( mainMenu );
	}
	void tShadeMainWindow::fAddToolbar( )
	{
		// setup primary/default toolbar
		mToolBar = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHORIZONTAL );
		GetSizer( )->Add( mToolBar, 0, wxEXPAND | wxALL, 0 );
		wxToolBar* mainToolBar = mToolBar;
		mainToolBar->SetToolBitmapSize( wxSize( 16, 16 ) );
		mainToolBar->AddTool( cActionNewDoc, "New Scene", wxBitmap( "newdoc" ), wxNullBitmap, wxITEM_NORMAL, "Create a new, empty scene" );
		mainToolBar->AddTool( cActionOpenDoc, "Open Scene", wxBitmap( "opendoc" ), wxNullBitmap, wxITEM_NORMAL, "Load an existing scene" );
		mainToolBar->AddTool( cActionSave, "Save Scene", wxBitmap( "savedoc" ), wxNullBitmap, wxITEM_NORMAL, "Save current scene" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionUndo, "Undo", wxBitmap( "undo" ), wxNullBitmap, wxITEM_NORMAL, "Undo last action" );
		mainToolBar->AddTool( cActionRedo, "Redo", wxBitmap( "redo" ), wxNullBitmap, wxITEM_NORMAL, "Redo previous action" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionAbout, "About", wxBitmap( "help" ), wxNullBitmap, wxITEM_NORMAL, "Get help" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionFrameSelected, "FrameSel", wxBitmap( "framesel" ), wxNullBitmap, wxITEM_NORMAL, "Frame Selected" );
		mainToolBar->AddTool( cActionFrameAll, "FrameAll", wxBitmap( "frameall" ), wxNullBitmap, wxITEM_NORMAL, "Frame All" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionToggleMatEd, "MaterialPreview", wxBitmap( "mated" ), wxNullBitmap, wxITEM_NORMAL, "Material Preview" );

		for( u32 i = 0; i < 6; ++i )
			mainToolBar->AddSeparator( );

		// Geometry Type
		{
			wxStaticText* text = new wxStaticText( mainToolBar, wxID_ANY, "Geometry Type:" );
			text->SetBackgroundColour( wxColour( 0xdd, 0xdd, 0xff, 0x00 ) );
			mainToolBar->AddControl( text );

			tDynamicArray<wxString> choices( HlslGen::cVshStyleCount );
			choices[ HlslGen::cVshMeshModel ]	= wxString( "MeshModel" );
			choices[ HlslGen::cVshFacingQuads ] = wxString( "FacingParticleQuad" );
			sigassert( !mVsStyle );
			mVsStyle = new wxChoice( mainToolBar, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices.fCount( ), choices.fBegin( ) );
			mVsStyle->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( tShadeMainWindow::fOnGeomStyleChanged ), NULL, this );
			mainToolBar->AddControl( mVsStyle );
			mVsStyle->Select( 0 );
		}

		// HLSL Preview Mode
		{
			mainToolBar->AddSeparator( );

			wxStaticText* text = new wxStaticText( mainToolBar, wxID_ANY, "HLSL Preview Platform:" );
			text->SetBackgroundColour( wxColour( 0xdd, 0xdd, 0xff, 0x00 ) );
			mainToolBar->AddControl( text );

			tDynamicArray<wxString> choices( 2 ); // Future upgrade: HlslGen::cPidCount
			choices[ 0 ] = wxString( "Dx9" );
			choices[ 1 ] = wxString( "Xbox360" );

			sigassert( !mHlslPreviewPlatform );
			mHlslPreviewPlatform = new wxChoice( mainToolBar, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices.fCount( ), choices.fBegin( ) );

			mainToolBar->AddControl( mHlslPreviewPlatform );
			mHlslPreviewPlatform->Select( 0 );
		}

		mainToolBar->Realize( );
	}
	void tShadeMainWindow::fOnClose( wxCloseEvent& event )
	{
		fSaveLayout( );

		if( fClearScene( ) )
			event.Skip( );
		else
			event.Veto( );
	}
	void tShadeMainWindow::fOnAction( wxCommandEvent& event )
	{
		const int id = event.GetId( );
		if( id >= cActionOpenRecent && id < cActionOpenRecent + cMaxRecentlyOpenedFiles )
		{
			// open recent action
			fOpenRecent( id - cActionOpenRecent );
		}
		else
		{
			switch( id )
			{
				// file menu
			case cActionNewDoc:		fNewDoc( ); break;
			case cActionOpenDoc:	fOpenDoc( ); break;
			case cActionSave:		fSaveDoc( false ); break;
			case cActionSaveAs:		fSaveDoc( true ); break;
			case cActionBuild:		fBuild( ); break;
			case cActionQuit:		Close( false ); break;

				// edit menu
			case cActionUndo:		mCanvas->fUndo( ); break;
			case cActionRedo:		mCanvas->fRedo( ); break;
			case cActionCopy:		mCanvas->fCopy( ); break;
			case cActionPaste:		mCanvas->fPaste( ); break;

				// window menu
			case cActionFrameSelected:	mCanvas->fFrame( true ); break;
			case cActionFrameAll:		mCanvas->fFrame( false ); break;
			case cActionToggleMatEd:	fSyncMatEd( ); mMatEd->fToggle( ); break;
			case cActionRefreshMatEd:	fSyncMatEd( ); break;
			case cActionAbout:			fShowHlsl( ); break;

			default: { log_warning( "Unrecognized action!" ); }
				break;
			}
		}
	}
	void tShadeMainWindow::fOnGeomStyleChanged( wxCommandEvent& event )
	{
		mCanvas->fEditorActions( ).fForceSetDirty( );
	}
	void tShadeMainWindow::fOnIdle( wxIdleEvent& event )
	{
		fSaveLayout( );

		if( mMatEd )
		{
			mMatEd->fAutoHandleTopMost( ( HWND )GetHWND( ) );
			if( mMatEd->fOnTick( ) )
				event.RequestMore( );
		}
	}
	void tShadeMainWindow::fOnSelChanged( )
	{
		if( mControlPanel )
			mControlPanel->fOnSelectionChanged( mCanvas->fSelectedNodes( ) );
	}
	std::string tShadeMainWindow::fMakeWindowTitle( ) const
	{
		return "SigShade ~ " + mDocName + ( mCanvas->fEditorActions( ).fIsDirty( ) ? "*" : "" );
	}
	b32 tShadeMainWindow::fClearScene( )
	{
		if( mCanvas->fEditorActions( ).fIsDirty( ) )
		{
			const int result = wxMessageBox( "You have unsaved changes - would you like to save them before resetting?",
						  "Save Changes?", wxYES | wxNO | wxCANCEL | wxICON_WARNING );

			if(			result == wxYES )			{ if( !fSaveDoc( false ) ) return false; }
			else if(	result == wxNO )			{ }
			else if(	result == wxCANCEL )		{ return false; }
			else									{ log_warning( "Unknown result returned from Message Box" ); }
		}

		mCanvas->fClearCanvas( );
		mDocName = cNewDocTitle;
		mVsStyle->Select( 0 );
		return true;
	}
	void tShadeMainWindow::fNewDoc( )
	{
		if( !fClearScene( ) )
			return;
		fSetStatus( "New shader" );
		mCanvas->fAddDefaultOutputNode( );
		mCanvas->fEditorActions( ).fReset( );
		SetTitle( fMakeWindowTitle( ) );
		fSyncMatEd( );
	}
	b32 tShadeMainWindow::fSaveDoc( b32 saveAs )
	{
		if( saveAs || mDocName == cNewDocTitle )
		{
			// browse for a new path
			tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
				mCanvas, 
				"Save Shader As",
				wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
				wxString( "untitled" + gGoamlExt ),
				wxString( "*" + gGoamlExt ),
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT ) );

			if( openFileDialog->ShowModal( ) != wxID_OK )
				return false; // cancelled

			mDocName = openFileDialog->GetPath( );
			fAddRecentFile( tFilePathPtr( mDocName.c_str( ) ) );
			fUpdateRecentFileMenu( );
		}
		else
		{
			// not doing a save as; if we're not dirty, then skip
			if( !mCanvas->fEditorActions( ).fIsDirty( ) )
				return true;
		}

		fSerialize( tFilePathPtr( mDocName.c_str( ) ) );
		mCanvas->fEditorActions( ).fClearDirty( );
		SetTitle( fMakeWindowTitle( ) );
		fSetStatus( "Document saved successfully" );
		return true;
	}
	void tShadeMainWindow::fOpenDoc( )
	{
		if( !fClearScene( ) )
			return; // user cancelled, don't try to open new file

		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mCanvas, 
			"Open Shader",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "untitled" + gGoamlExt ),
			wxString( "*" + gGoamlExt ),
			wxFD_OPEN ) );

		SetFocus( );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			fOpenDoc( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );
		}
	}
	void tShadeMainWindow::fOpenDoc( const tFilePathPtr& file )
	{
		if( !fClearScene( ) )
			return; // user cancelled, don't try to open new file

		if( FileSystem::fFileExists( file ) )
		{
			if( !fDeserialize( file ) )
			{
				wxMessageBox( "The specified .derml file is corrupt or out of date; open failed.", "Invalid File", wxOK | wxICON_WARNING );
				fSetStatus( "Shader open failed" );
				return;
			}

			// set up new scene
			mDocName = file.fCStr( );
			SetTitle( fMakeWindowTitle( ) );
			fSetStatus( "Shader opened successfully" );
			fAddRecentFile( file );
			fUpdateRecentFileMenu( );

			mCanvas->fFrame( );

			fSyncMatEd( );
		}
		else
		{
			wxMessageBox( "The specified file can not be found; open failed.", "File Not Found", wxOK | wxICON_WARNING );
		}
	}
	void tShadeMainWindow::fOpenRecent( u32 ithRecentFile )
	{
		const Win32Util::tRecentlyOpenedFileList& recentFiles = mRecentFiles;
		fOpenDoc( recentFiles[ ithRecentFile ] );
	}
	void tShadeMainWindow::fAddRecentFile( const tFilePathPtr& path )
	{
		mRecentFiles.fAdd( path );
		mRecentFiles.fSave( );
	}
	void tShadeMainWindow::fUpdateRecentFileMenu( )
	{
		if( !mRecentFilesMenu )
			return;

		while( mRecentFilesMenu->GetMenuItems( ).size( ) > 0 )
			mRecentFilesMenu->Delete( mRecentFilesMenu->GetMenuItems( ).front( ) );

		const Win32Util::tRecentlyOpenedFileList& recentFiles = mRecentFiles;
		const u32 min = fMin( cMaxRecentlyOpenedFiles, recentFiles.fCount( ) );
		for( u32 i = 0; i < min; ++i )
			mRecentFilesMenu->Append( cActionOpenRecent + i, recentFiles[ i ].fCStr( ) );
	}
	void tShadeMainWindow::fBuild( )
	{
		if( mDocName == cNewDocTitle )
		{
			wxMessageBox( "You must save your file before building it.", "Save First", wxOK | wxICON_WARNING );
			return;
		}
		else if( mCanvas->fEditorActions( ).fIsDirty( ) )
		{
			fSaveDoc( false );
		}

		tAssetGenScanner::fProcessSingleFile( tFilePathPtr( mDocName ), true );
	}
	void tShadeMainWindow::fSerialize( const tFilePathPtr& path )
	{
		Derml::tFile dermlFile;

		// collect nodes/connections etc
		mCanvas->fToDermlFile( dermlFile );

		// generate shaders before saving so as to produce material glue indices for each node
		HlslGen::tHlslInput input;
		HlslGen::tHlslOutput output;
		HlslGen::tHlslGenTree::fGenerateShaders( dermlFile, input, output );

		// get geometry style
		dermlFile.mGeometryStyle = ( HlslGen::tVshStyle )fClamp( mVsStyle->GetSelection( ), 0, HlslGen::cVshStyleCount - 1 );

		// save to xml
		// No material properties from sig shade
		dermlFile.fSaveXml( path, true );
	}
	b32 tShadeMainWindow::fDeserialize( const tFilePathPtr& path )
	{
		Derml::tFile dermlFile;
		if( !dermlFile.fLoadXml( path ) )
			return false;

		// Rip out all the material properties - this is necessary to resolve an issue
		// where the material previewer was modifying the canvas shade node mat props
		for( u32 n = 0; n < dermlFile.mNodes.fCount( ); ++n )
			dermlFile.mNodes[ n ]->fMatProps( ).fClear( );

		// create nodes/connections etc
		mCanvas->fFromDermlFile( dermlFile );
		mVsStyle->Select( dermlFile.mGeometryStyle );
		return true;
	}

	void tShadeMainWindow::fSyncMatEd( )
	{
		// Because the mtl file is gonna create new nodes we need to reset this after the derml is done
		const u32 nextUniqueNodeId = tShadeNode::fNextUniqueNodeId( );

		Derml::tFile dermlFile;
		mCanvas->fToDermlFile( dermlFile );

		// In order to create unique shade nodes we save the derml to 
		// xml and then reload it from that xml - kind of ugly, but a sufficient solution
		tXmlSerializer xmlSer;
		dermlFile.fSaveXml( xmlSer );
		tXmlDeserializer xmlDeser;
		xmlDeser.fCurRoot( ) = xmlSer.fCurRoot( );
		dermlFile.fLoadXml( xmlDeser );


		Derml::tMtlFile mtlFile;
		mtlFile.fFromShaderFile( dermlFile, ToolsPaths::fMakeResRelative( tFilePathPtr( mDocName ) ) );

		mMatEd->fFromDermlMtlFile( mtlFile );

		// generate shaders for material preview
		mMatPreview->fGenerateShaders( dermlFile, HlslGen::cToolTypeDefault );

		// Reset the next id, since all those new nodes are about to die
		tShadeNode::fSetNextUniqueNodeId( nextUniqueNodeId );
	}

	void tShadeMainWindow::fShowHlsl( )
	{
		//save + load in derml
		if( !fSaveDoc( false ) )
			return;

		Derml::tFile dermlFile;
		if( !dermlFile.fLoadXml( tFilePathPtr( mDocName ) ) )
			return;

		//generate hlsl
		HlslGen::tHlslPlatformId hlslPid = HlslGen::cPidDefault;
		if( dermlFile.mGeometryStyle == HlslGen::cVshMeshModel )
		{
			if( mHlslPreviewPlatform->GetSelection( ) == 0)
				hlslPid = HlslGen::cPidPcDx9;
			else // selection == 1
				hlslPid = HlslGen::cPidXbox360;
		}

		HlslGen::tHlslInput input = HlslGen::tHlslInput( hlslPid, dermlFile.mGeometryStyle, true, true, false );
		HlslGen::tHlslOutput output;
		HlslGen::tHlslGenTree::fGenerateShaders( dermlFile, input, output );

		//show dialog
		tHlslViewerDialog dialog( mCanvas, output );
		dialog.ShowModal();
	}

}
