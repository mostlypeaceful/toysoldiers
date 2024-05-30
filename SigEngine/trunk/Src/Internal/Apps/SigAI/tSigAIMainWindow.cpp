#include "SigAIPch.hpp"
#include "tSigAIMainWindow.hpp"
#include "tSigAIControlPanel.hpp"
#include "tMatEdMainWindow.hpp"
#include "tStrongPtr.hpp"
#include "FileSystem.hpp"
#include "tAssetGenScanner.hpp"
#include "tFileReader.hpp"
#include "tEditScriptSnippetDialog.hpp"
#include "tFindInSnippetsDialog.hpp"
#include "tConfigurableBrowserTree.hpp"
#include "tSigAIExplicitDependenciesDialog.hpp"
#include "tProjectXMLDialog.hpp"

namespace Sig
{
	enum tAction
	{
		cActionNewDoc = 1,
		cActionOpenDoc,
		cActionOpenRecent,
		cActionSave = cActionOpenRecent + tSigAIMainWindow::cMaxRecentlyOpenedFiles,
		cActionSaveAs,
		cActionChooseMoMap,
		cActionEditMoMap,
		cActionRefreshMoMap,
		cActionClearMoMap,
		cActionBuild,
		cActionViewScript,
		cActionViewSnippets,
		cActionQuit,

		cActionUndo,
		cActionRedo,
		cActionCopy,
		cActionPaste,
		cActionFind,
		cActionReplace,
		cActionEditProjectSettings,

		cActionAbout,

		cActionFrameSelected,
		cActionFrameAll,
		cActionExplicitDependencies
	};

	namespace
	{
		const std::string gGoamlExt = Goaml::fGetFileExtension( );

		b32 fIsScriptFile( const tFilePathPtr& path )
		{
			return StringUtil::fCheckExtension( path.fCStr( ), ".goaml" );
		}
	}

	class tGoamlBrowser : public tConfigurableBrowserTree
	{
		tSigAIMainWindow* mParent;

	public:
		tGoamlBrowser( wxWindow* parent, tSigAIMainWindow* mainParent, u32 minHeight )
			: tConfigurableBrowserTree( parent, fIsScriptFile, minHeight, true, true )
			, mParent( mainParent )
		{ }

	private:
		virtual void fOpenDoc( const tFilePathPtr& file )
		{
			mParent->fOpenDoc( file );
		}
	};

	const char tSigAIMainWindow::cNewDocTitle[] = "(untitled)";



	tSigAIMainWindow::tSigAIMainWindow( )
		: wxFrame( (wxFrame *)NULL, -1, "SigAI" )
		, mSavedLayout( ToolsPaths::fGetSignalRegistryKeyName( ) + "\\SigAI\\MainWindow" )
		, mToolBar( 0 )
		, mCanvas( 0 )
		, mControlPanel( 0 )
		, mRecentFilesMenu( 0 )
		, mDocName( cNewDocTitle )
		, mRecentFiles( ToolsPaths::fGetSignalRegistryKeyName( ) + "\\SigAI\\MainWindow\\" + ToolsPaths::fGetCurrentProjectName( ) + "\\RecentFiles" )
		, mExplicitDependenciesDialog( NULL )
	{
		Log::fSetLogFilterMask( Log::cFlagNetwork );

		SetIcon( wxIcon( "appicon" ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );
		Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( tSigAIMainWindow::fOnClose ) );
		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tSigAIMainWindow::fOnAction ) );
		Connect( wxEVT_IDLE, wxIdleEventHandler( tSigAIMainWindow::fOnIdle ) );

		mRecentFiles.fLoad( );
		fCreateMainMenu( );
		fAddToolbar( );

		// status bar...
		const int statusWidths[] = { 0, 60, -1 };
		wxStatusBar* statusBar = CreateStatusBar( array_length(statusWidths), wxST_SIZEGRIP|wxFULL_REPAINT_ON_RESIZE, 0, wxT("Status:") );
		SetStatusWidths( array_length(statusWidths), statusWidths );
		SetStatusText( "      Status:", 1 );

		// create canvas, control panel, and material editor
		mCanvas = new tSigAINodeCanvas( this, this );
		mControlPanel = new tSigAIControlPanel( this, mCanvas );

		// add control panel
		wxSizer* mainSizer = new wxBoxSizer( wxHORIZONTAL );
		GetSizer( )->Add( mainSizer, 1, wxEXPAND | wxALL );
		mainSizer->Add( mControlPanel, 0, wxALIGN_LEFT | wxEXPAND | wxALL );

		// add node canvas...
		mainSizer->Add( mCanvas, 6, wxALIGN_LEFT | wxEXPAND | wxALL );
		SetBackgroundColour( mCanvas->fBgColor( ) );

		mBrowser = new tGoamlBrowser( this, this, wxDefaultSize.y );
		mainSizer->Add( mBrowser, 1, wxALIGN_RIGHT | wxEXPAND | wxALL, 5 );
		mBrowser->fRefresh( );

		// load saved UI settings
		fLoadLayout( );

		// set delegates
		mOnDirty.fFromMethod<tSigAIMainWindow, &tSigAIMainWindow::fOnDirty>( this );
		mOnAddAction.fFromMethod<tSigAIMainWindow, &tSigAIMainWindow::fOnActionUndoOrRedo>( this );
		mOnSelChanged.fFromMethod<tSigAIMainWindow,&tSigAIMainWindow::fOnSelChanged>( this );
		mCanvas->mOnSelChanged.fAddObserver( &mOnSelChanged );
		mCanvas->fEditorActions( ).mOnDirty.fAddObserver( &mOnDirty );
		mCanvas->fEditorActions( ).mOnAddAction.fAddObserver( &mOnAddAction );
		mCanvas->fEditorActions( ).mOnUndo.fAddObserver( &mOnAddAction );
		mCanvas->fEditorActions( ).mOnRedo.fAddObserver( &mOnAddAction );

		mExplicitDependenciesDialog = new tSigAIExplicitDependenciesDialog( this );

		fNewDoc( );

		mCanvas->SetFocus( );
	}
	tSigAIMainWindow::~tSigAIMainWindow( )
	{
	}
	void tSigAIMainWindow::fSetStatus( const char* status )
	{
		SetStatusText( status, 2 );
	}
	void tSigAIMainWindow::fLoadLayout( )
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
	void tSigAIMainWindow::fSaveLayout( )
	{
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
	void tSigAIMainWindow::fOnDirty( tEditorActionStack& stack )
	{
		SetTitle( fMakeWindowTitle( ) );
	}
	void tSigAIMainWindow::fOnActionUndoOrRedo( tEditorActionStack& stack )
	{
		// TODO: refresh stuff that needs it
	}
	void tSigAIMainWindow::fCreateMainMenu( )
	{
		wxMenuBar* mainMenu = new wxMenuBar;

		{
			wxMenu* subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&File" );

			subMenu->Append( cActionNewDoc, "&New\tCtrl+N" );
			subMenu->Append( cActionOpenDoc, "&Open...\tCtrl+O" );
			subMenu->Append( cActionSave, "&Save\tCtrl+S" );
			subMenu->Append( cActionSaveAs, "Save &As..." );
			subMenu->AppendSeparator( );
			subMenu->Append( cActionChooseMoMap, "Choose &MoMap" );
			subMenu->Append( cActionRefreshMoMap, "&Refresh MoMap" );
			subMenu->Append( cActionClearMoMap, "Clear MoMap" );
			
			subMenu->AppendSeparator( );
			subMenu->Append( cActionExplicitDependencies, "Explicit &Dependencies" );
			subMenu->AppendSeparator( );

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
			subMenu->AppendSeparator( );
			subMenu->Append( cActionFind, "&Find\tCtrl+F" );
			subMenu->Append( cActionReplace, "R&eplace\tCtrl+H" );

			subMenu->AppendSeparator( );
			subMenu->Append( cActionEditProjectSettings, "Edit Flags" );
		}
		// add "window" menu; might change this later
		{
			wxMenu *subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&Window" );

			subMenu->Append( cActionFrameSelected, "Frame &Selected\t F" );
			subMenu->Append( cActionFrameAll, "Frame &All\t A" );
			subMenu->AppendSeparator();
		}
		{
			wxMenu* subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&Help" );

			subMenu->Append( cActionAbout, "&About..." );
		}

		SetMenuBar( mainMenu );
	}
	void tSigAIMainWindow::fAddToolbar( )
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
		mainToolBar->AddTool( cActionExplicitDependencies, "ExplicitDepends", wxBitmap( "explicitdepends" ), wxNullBitmap, wxITEM_NORMAL, "Explicit Dependencies" );
		
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionBuild, "Build Current Goaml", wxBitmap( "build" ), wxNullBitmap, wxITEM_NORMAL, "Build the current Goaml" );
		mainToolBar->AddTool( cActionViewScript, "View Generated Script", wxBitmap( "diff" ), wxNullBitmap, wxITEM_NORMAL, "View the generated script" );
		//mainToolBar->AddTool( cActionViewSnippets, "View All Snippets", wxBitmap( "diff" ), wxNullBitmap, wxITEM_NORMAL, "View all the user entered script snippets" );

		
		
		mainToolBar->Realize( );
	}
	void tSigAIMainWindow::fOnClose( wxCloseEvent& event )
	{
		fSaveLayout( );

		if( fClearScene( ) )
			event.Skip( );
		else
			event.Veto( );
	}
	void tSigAIMainWindow::fOnAction( wxCommandEvent& event )
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
			case cActionChooseMoMap:fChooseMoMap( ); break;
			case cActionRefreshMoMap:  fRefreshMoMap( ); break;
			case cActionClearMoMap: fClearMoMap( ); break;
			case cActionBuild:		fBuild( ); break;
			case cActionViewScript: fViewScript( ); break;
			case cActionViewSnippets: fViewSnippets( ); break;
			case cActionQuit:		Close( false ); break;

				// edit menu
			case cActionUndo:		mCanvas->fUndo( ); break;
			case cActionRedo:		mCanvas->fRedo( ); break;
			case cActionCopy:		mCanvas->fCopy( ); break;
			case cActionPaste:		mCanvas->fPaste( ); break;
			case cActionFind:		fFind( ); break;
			case cActionReplace:	fReplace( ); break;
			case cActionEditProjectSettings: fEditProjectSettings( ); break;

				// window menu
			case cActionFrameSelected:	mCanvas->fFrame( true ); break;
			case cActionFrameAll:		mCanvas->fFrame( false ); break;
			case cActionExplicitDependencies:
				mExplicitDependenciesDialog->fToggle( );
				break;
			case cActionAbout:
				wxMessageBox( "SigAI is a visual, node-based AI state-machine/goal tool.", "About SigAI", wxOK | wxICON_INFORMATION );
				break;

			default: { log_warning( "Unrecognized action!" ); }
				break;
			}
		}
	}
	void tSigAIMainWindow::fOnIdle( wxIdleEvent& event )
	{
	}
	void tSigAIMainWindow::fOnSelChanged( )
	{
		if( mControlPanel )
			mControlPanel->fOnEventSelectionChanged( mCanvas->fSelectedNodes( ), mCanvas->fSelectedConnections( ), mCanvas->fSelectedOutputs( ) );
	}
	std::string tSigAIMainWindow::fMakeWindowTitle( ) const
	{
		return ( mCanvas->fEditorActions( ).fIsDirty( ) ? "* " : "" ) + StringUtil::fNameFromPath( mDocName.c_str( ) ) + " ~ SigAI ~ " + ToolsPaths::fMakeResRelative( tFilePathPtr( mDocName ) ).fCStr( );
	}
	b32 tSigAIMainWindow::fClearScene( )
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
		mExplicitDependenciesDialog->fSetDependencyList( tFilePathPtrList( ) );

		return true;
	}
	void tSigAIMainWindow::fNewDoc( )
	{
		if( !fClearScene( ) )
			return;
		fSetStatus( "New goaml" );
		mCanvas->fAddDefaultNode( );
		mCanvas->fEditorActions( ).fReset( );
		SetTitle( fMakeWindowTitle( ) );
	}
	b32 tSigAIMainWindow::fSaveDoc( b32 saveAs )
	{
		if( saveAs || mDocName == cNewDocTitle )
		{
			// browse for a new path
			tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
				mCanvas, 
				"Save Goaml As",
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

		if( !fSerialize( tFilePathPtr( mDocName.c_str( ) ) ) )
			return false;

		mCanvas->fEditorActions( ).fClearDirty( );
		SetTitle( fMakeWindowTitle( ) );
		fSetStatus( "Document saved successfully" );
		return true;
	}
	void tSigAIMainWindow::fOpenDoc( )
	{
		if( !fClearScene( ) )
			return; // user cancelled, don't try to open new file

		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mCanvas, 
			"Open Goaml",
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
	void tSigAIMainWindow::fOpenDoc( const tFilePathPtr& file )
	{
		if( !fClearScene( ) )
			return; // user cancelled, don't try to open new file

		if( FileSystem::fFileExists( file ) )
		{
			if( !fDeserialize( file ) )
			{
				wxMessageBox( "The specified .goaml file is corrupt or out of date; open failed.", "Invalid File", wxOK | wxICON_WARNING );
				fSetStatus( "Goaml open failed" );
				return;
			}

			// set up new scene
			mDocName = file.fCStr( );
			SetTitle( fMakeWindowTitle( ) );
			fSetStatus( "Goaml opened successfully" );
			fAddRecentFile( file );
			fUpdateRecentFileMenu( );

			mCanvas->fFrame( );
		}
		else
		{
			wxMessageBox( "The specified file can not be found; open failed.", "File Not Found", wxOK | wxICON_WARNING );
		}
	}
	void tSigAIMainWindow::fOpenRecent( u32 ithRecentFile )
	{
		const Win32Util::tRecentlyOpenedFileList& recentFiles = mRecentFiles;
		fOpenDoc( recentFiles[ ithRecentFile ] );
	}
	void tSigAIMainWindow::fAddRecentFile( const tFilePathPtr& path )
	{
		mRecentFiles.fAdd( path );
		mRecentFiles.fSave( );
	}
	void tSigAIMainWindow::fUpdateRecentFileMenu( )
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
	b32 tSigAIMainWindow::fBuild( )
	{
		if( mDocName == cNewDocTitle )
		{
			wxMessageBox( "You must save your file before building it.", "Save First", wxOK | wxICON_WARNING );
			return false;
		}
		else if( mCanvas->fEditorActions( ).fIsDirty( ) )
		{
			fSaveDoc( false );
		}

		tAssetGenScanner::fProcessSingleFile( tFilePathPtr( mDocName ), true );
		return true;
	}
	void tSigAIMainWindow::fChooseMoMap( )
	{
		wxString file = wxFileSelector( "Choose MotionMap file", NULL, NULL, ".momap", "Momap Files (*.momap)|*.momap" );
		if( !file.empty( ) )
		{
			tFilePathPtr path = ToolsPaths::fMakeResRelative( tFilePathPtr( std::string( file ) ) );
			mControlPanel->fLoadMoMap( path );
			mCanvas->fEditorActions( ).fForceSetDirty( true );
		}		
	}
	void tSigAIMainWindow::fRefreshMoMap( )
	{
		if( _stricmp( mControlPanel->fMoMapPath( ).fCStr( ), tSigAIControlPanel::cNoMoMapText.c_str( ) ) == 0 )
			wxMessageBox( "No MoMap chosen yet.", "Oops" );
		else
			mControlPanel->fLoadMoMap( mControlPanel->fMoMapPath( ) );
	}
	void tSigAIMainWindow::fClearMoMap( )
	{
		if( wxMessageBox( wxString( "Are you sure you want to clear the momap?\n" ) + mControlPanel->fMoMapPath( ).fCStr( ),
			"Sure?", wxYES | wxNO | wxICON_WARNING ) == wxYES )
		{
			mControlPanel->fLoadMoMap( tFilePathPtr::cNullPtr );
			mCanvas->fEditorActions( ).fForceSetDirty( true );
		}
	}
	void tSigAIMainWindow::fViewScript( )
	{
		Goaml::tFile goamlFile;
		mCanvas->fToGoamlFile( goamlFile );
		
		std::string scriptString = goamlFile.fBuildScript( "_0123456_", true, mCanvas->fSelectedNodes( ) );
		tEditScriptSnippetDialog *diag = new tEditScriptSnippetDialog( this, "Generated script file.", "", scriptString, 0, false );
		diag->fShowDialog( );
	}

	namespace
	{
		std::string fBlock( const std::string& c, const std::string& e )	
		{
			const std::string commentBar = "//////////////////////////////////////////////////////\n";
			return commentBar + "\t" + c + " : " + e + "\n" + commentBar;
		}
	}

	void tSigAIMainWindow::fViewSnippets( )
	{
		//std::string string;

		//for( u32 i = 0; i < mCanvas->fAllNodes( ).fCount( ); ++i )
		//{
		//	tGoalAINode* goal = dynamic_cast<tGoalAINode*>( mCanvas->fAllNodes( )[ i ].fGetRawPtr( ) );
		//	if( goal )
		//	{
		//	}


		//tEditScriptSnippetDialog *diag = new tEditScriptSnippetDialog( this, "All script snippets.", "", wxString( string ), 0, false );
		//diag->fShowDialog( );
	}

	void tSigAIMainWindow::fFind( )
	{
		tFindInSnippetsDialog *diag = new tFindInSnippetsDialog( this );
		diag->fOpenFindInFiles( );	
	}

	void tSigAIMainWindow::fReplace( )
	{
		tFindInSnippetsDialog *diag = new tFindInSnippetsDialog( this );
		diag->fOpenReplaceInFiles( );	
	}
	
	void tSigAIMainWindow::fEditProjectSettings( )
	{
		tProjectXMLDialog* diag = new tProjectXMLDialog( this );
		diag->ShowModal( );
		fOnSelChanged( );
	}

	b32 tSigAIMainWindow::fSerialize( const tFilePathPtr& path )
	{
		Goaml::tFile goamlFile;

		// collect nodes/connections etc
		mCanvas->fToGoamlFile( goamlFile );
		goamlFile.fSetMoMap( mControlPanel->fMoMapPath( ) );
		goamlFile.mExplicitDependencies = mExplicitDependenciesDialog->fDependencyList( );

		goamlFile.fQuickSortXMLNodes( );

		// Assign node ids in consecutive order, this avoids gaps in ids or ids higher than the total number of nodes
		for( u32 i = 0; i < goamlFile.mNodes.fCount(); i++ )
		{
			goamlFile.mNodes[ i ]->fSetUniqueNodeId( i );
		}

		// save to xml
		return goamlFile.fSaveXml( path, true );
	}
	b32 tSigAIMainWindow::fDeserialize( const tFilePathPtr& path )
	{
		Goaml::tFile goamlFile;
		if( !goamlFile.fLoadXml( path ) )
			return false;

		// create nodes/connections etc
		mCanvas->fFromGoamlFile( goamlFile );
		mControlPanel->fLoadMoMap( goamlFile.fMoMap( ) );
		mExplicitDependenciesDialog->fSetDependencyList( goamlFile.mExplicitDependencies );

		return true;
	}

}
