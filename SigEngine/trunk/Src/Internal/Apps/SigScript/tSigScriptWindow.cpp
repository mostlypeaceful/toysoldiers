#include "SigScriptPch.hpp"
#include "tSigScriptWindow.hpp"
#include "FileSystem.hpp"
#include "tProjectFile.hpp"
#include "tWxTextEditor.hpp"
#include "tConfigurableBrowserTree.hpp"
#include "tScriptNotebook.hpp"
#include "tFindInFilesDialog.hpp"
#include "tSearchableOpenFilesDialog.hpp"
#include "threads\tProcess.hpp"
#include "tGotoLineDialog.hpp"
#include "tScriptContextActions.hpp"
#include "tLocmlControlPanel.hpp"

namespace Sig
{
	static const wxColour cBackColor = wxColour( 0x44, 0x44, 0x55 );
	static const wxColour cTextColor = wxColour( 0xff, 0xff, 0x99 );
	static const wxColour cGroupTitleColour = wxColour( 0xff, 0xff, 0xff );
	static const u32 cMaxRecentlyOpenedFiles = 8;

	enum
	{
		ID_Quit = 1,
		ID_About,

		ID_New,
		ID_Open,
		ID_OpenRecent,
		ID_Save = ID_OpenRecent + cMaxRecentlyOpenedFiles,
		ID_SaveAll,
		ID_SaveAs,
		ID_CloseThis,
		ID_CloseAll,
		ID_CloseAllButThis,
		ID_Undo,
		ID_Redo,

		ID_Find,
		ID_FindInFiles,
		ID_Replace,
		ID_ReplaceInFiles,
		ID_GotoLine,

		ID_SearchNext,
		ID_SearchPrev,

		ID_ToUpper,

		ID_BuildFile,
		ID_BuildRes,

		ID_DiffFile,

		ID_ToggleEditorMode,
		ID_ShowExportBrowser,
		ID_ShowPerformanceBrowser,
		ID_ShowLocmlControl,

		ID_CommentSelection,
		ID_UncommentSelection,
	};

	static b32 fIsSupportedFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), ".nut" ) 
			|| StringUtil::fCheckExtension( path.fCStr( ), ".locml" )
			|| StringUtil::fCheckExtension( path.fCStr( ), ".goaml" );
	}

	///
	/// \brief
	/// A browser tree configured to search for script (.nut) files.
	class tScriptBrowser : public tConfigurableBrowserTree
	{
		tSigScriptWindow* mParent;

	public:
		tScriptBrowser( wxWindow* parent, tSigScriptWindow* mainParent, u32 minHeight )
			: tConfigurableBrowserTree( parent, fIsSupportedFile, minHeight, true, true )
			, mParent( mainParent )
		{ }

		virtual wxColour fProvideCustomEntryColor( const tFileEntryData& fileEntryData )
		{
			// TODO: clean up your extension checks
			if( StringUtil::fCheckExtension( fileEntryData.fXmlPath( ).fCStr( ), ".locml" ) )
				return wxColour( 0x00, 0xAA, 0x00 );
			else if( StringUtil::fCheckExtension( fileEntryData.fXmlPath( ).fCStr( ), ".goaml" ) )
				return wxColour( 0x00, 0xAA, 0xCC );

			return wxColour( 0x00, 0x00, 0xFF );
		}

		virtual std::string fConvertDisplayName( const std::string& simplePath, u32& sortValue )
		{
			std::stringstream fullDisplay;

			// TODO: clean up your extension checks
			if( StringUtil::fCheckExtension( simplePath, ".locml" ) )
				fullDisplay << "[locml] ";
			else if( StringUtil::fCheckExtension( simplePath, ".goaml" ) )
				fullDisplay << "[goaml] ";
			else
				fullDisplay << "[script] ";

			fullDisplay << StringUtil::fStripExtension( simplePath.c_str( ) );

			return fullDisplay.str( );
		}

	private:
		virtual void fOpenDoc( const tFilePathPtr& file )
		{
			mParent->fOpenDoc( file );
		}
	};

	/// 
	/// \brief
	/// Timer that regulates how often check changes updates.
	class tCheckChangesTimer : public wxTimer
	{
		tSigScriptWindow* mScriptWindow;

	public:
		tCheckChangesTimer( tSigScriptWindow* scriptWindow )
			: wxTimer( scriptWindow )
			, mScriptWindow( scriptWindow )
		{ }

		virtual void Notify( )
		{
			mScriptWindow->fNotebook( )->fCheckChanges( );
			mScriptWindow->fSaveLayout( );
		}
	};

	/// 
	/// \brief
	/// Time that updates every frame for things that should be onticked.
	class tOnTickTimer : public wxTimer
	{
		tSigScriptWindow* mScriptWindow;

	public:
		tOnTickTimer( tSigScriptWindow* scriptWindow )
			: wxTimer( scriptWindow )
			, mScriptWindow( scriptWindow )
		{ }

		virtual void Notify( )
		{
			if( !mScriptWindow->fNotebook( )->fGetCurrent( ) )
				return;

			mScriptWindow->fToolbar( )->EnableTool( ID_ToggleEditorMode, mScriptWindow->fNotebook( )->fGetCurrent( )->fGetEditor( )->fIsScript( ) != 0 );
			mScriptWindow->fToolbar( )->EnableTool( ID_Undo, mScriptWindow->fNotebook( )->fGetCurrent( )->fGetEditor( )->CanUndo( ) );
			mScriptWindow->fToolbar( )->EnableTool( ID_Redo, mScriptWindow->fNotebook( )->fGetCurrent( )->fGetEditor( )->CanRedo( ) );
		}
	};

	///
	/// \class tOpenFileInNewWindowOption
	/// \brief Menu option for opening scripts in new windows.
	class tOpenFileInNewWindowOption : public tMenuOption
	{
	public:
		void tAddOption( const tWxDirectoryBrowser::tFileEntryData* entryData, wxTreeEvent& event, wxMenu& menu )
		{
			if( entryData )
				menu.Append( mActionId, _T("Open file in new window") );
		}

		void tExecuteOption( const tWxDirectoryBrowser::tFileEntryData* entryData, wxCommandEvent& event )
		{
			ToolsPaths::fLaunchSigScript( entryData->fXmlPath( ).fCStr( ) );
		}
	};

	///
	/// \class tExploreToFileOption
	/// \brief Opens the selected file in a new explorer instance.
	class tExploreToFileOption : public tMenuOption
	{
	public:
		void tAddOption( const tWxDirectoryBrowser::tFileEntryData* entryData, wxTreeEvent& event, wxMenu& menu )
		{
			if( entryData )
				menu.Append( mActionId, _T("Open containing folder") );
		}

		void tExecuteOption( const tWxDirectoryBrowser::tFileEntryData* entryData, wxCommandEvent& event )
		{
			Win32Util::fExploreToDirectoryAndSelectFile( ToolsPaths::fMakeResAbsolute( entryData->fXmlPath( ) ).fCStr( ) );
		}
	};

	BEGIN_EVENT_TABLE(tSigScriptWindow, wxFrame)
		EVT_CLOSE(												tSigScriptWindow::fOnClose)
		EVT_DROP_FILES(											tSigScriptWindow::fOnFilesDropped)
		EVT_MENU(						ID_About,				tSigScriptWindow::fOnAbout)
		EVT_MENU(						ID_Quit,				tSigScriptWindow::fOnQuit)
		EVT_MENU(						ID_New,					tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_Open,				tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_Save,				tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_SaveAll,				tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_SaveAs,				tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_CloseThis,			tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_CloseAll,			tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_CloseAllButThis,		tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_Undo,				tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_Redo,				tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_Find,				tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_FindInFiles,			tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_Replace,				tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_ReplaceInFiles,		tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_GotoLine,			tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_SearchNext,			tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_SearchPrev,			tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_ToUpper,				tSigScriptWindow::fOnAction)		
		EVT_MENU(						ID_BuildFile,			tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_BuildRes,			tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_DiffFile,			tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_ToggleEditorMode,	tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_ShowLocmlControl,		tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_ShowExportBrowser,	tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_ShowPerformanceBrowser,	tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_CommentSelection,	tSigScriptWindow::fOnAction)
		EVT_MENU(						ID_UncommentSelection,	tSigScriptWindow::fOnAction)
		EVT_AUINOTEBOOK_PAGE_CHANGED(	wxID_ANY,				tSigScriptWindow::fOnPageChange )
		EVT_AUINOTEBOOK_PAGE_CLOSED(	wxID_ANY,				tSigScriptWindow::fOnPageChange )
	END_EVENT_TABLE()

	tSigScriptWindow::tSigScriptWindow( const wxString& title/*, const wxPoint& pos, const wxSize& size*/ )
		   : wxFrame( (wxFrame *)NULL, -1, title, wxPoint(-1,-1), wxSize(1200,800) )
		   , mRegKeyName( ToolsPaths::fGetSignalRegistryKeyName( ) + "\\SigScript" )
		   , mRecentFiles( fRegistryKeyName( ) + "\\" + ToolsPaths::fGetCurrentProjectName( ) + "\\RecentFiles" )
		   , mRecentFilesMenu( 0 )
		   , mSavedLayout( fRegistryKeyName( ) + "\\SavedLayout" )
		   , mChangesTimer( 0 )
	{
		mAui.SetManagedWindow( this );

		//
		// set the window as accepting dropped files
		//

		DragAcceptFiles( true );

		// 
		// set colors
		//

		SetBackgroundColour( cBackColor );
		SetForegroundColour( cTextColor );

		//
		// set icon...
		//

		SetIcon(wxIcon("appicon"));

		//
		// add menu...
		//

		wxMenu *menuFile = new wxMenu;
		menuFile->Append( ID_New, "&New\tCtrl+N" );
		menuFile->Append( ID_Open, "&Open...\tCtrl+O" );
		menuFile->Append( ID_Save, "&Save\tCtrl+S" );
		menuFile->Append( ID_SaveAll, "Save All\tCtrl+Shift+S" );
		menuFile->Append( ID_SaveAs, "Save &As..." );
		menuFile->AppendSeparator();

		mRecentFilesMenu = new wxMenu;
		mBaseRecentFileActionId = ID_OpenRecent;
		menuFile->AppendSubMenu( mRecentFilesMenu, "Recen&t Files" );

		menuFile->AppendSeparator();
		menuFile->Append( ID_About, "&About..." );
		menuFile->AppendSeparator();
		menuFile->Append( ID_Quit, "E&xit" );

		wxMenu *menuEdit = new wxMenu;
		menuEdit->Append( ID_Undo, "&Undo" );
		menuEdit->Append( ID_Redo, "&Redo" );
		menuEdit->AppendSeparator();
		menuEdit->Append( ID_Find, "Quick Find\tCtrl+F" );
		menuEdit->Append( ID_FindInFiles, "Find In Files\tCtrl+Shift+F" );
		menuEdit->Append( ID_Replace, "Quick Replace\tCtrl+H" );
		menuEdit->Append( ID_ReplaceInFiles, "Replace In Files\tCtrl+Shift+H" );
		menuEdit->Append( ID_GotoLine, "Goto...\tCtrl+G" );
		menuEdit->AppendSeparator();
		menuEdit->Append( ID_SearchPrev, "Find Previous\tF2" );
		menuEdit->Append( ID_SearchNext, "Find Next\tF3" );
		menuEdit->AppendSeparator();
		menuEdit->Append( ID_CommentSelection, "Comment Selection\tCtrl+K" );
		menuEdit->Append( ID_UncommentSelection, "&Uncomment Selection\tCtrl+U" );
		menuEdit->AppendSeparator( );
		menuEdit->Append( ID_ToUpper, "To Upper\tF5");

		wxMenu *menuBuild = new wxMenu;
		menuBuild->Append( ID_BuildFile, "&Build Current File\tCtrl+B" );
		menuBuild->Append( ID_BuildRes, "Build All &Res\tCtrl+Shift+B" );

		wxMenu *menuWindow = new wxMenu;
		menuWindow->Append( ID_ShowLocmlControl, "&Locml Control Panel");
		menuWindow->AppendSeparator();
		menuWindow->Append( ID_CloseThis, "Close &Window\tCtrl+W" );
		menuWindow->Append( ID_CloseAll, "Close &All Windows" );
		menuWindow->Append( ID_CloseAllButThis, "Close All But &This" );

		wxMenu *devMenu = new wxMenu;
		devMenu->Append( ID_ShowExportBrowser, "Script Export Browser\tF12" );
		devMenu->Append( ID_ShowPerformanceBrowser, "Performance Browser\tF11");

		wxMenuBar *menuBar = new wxMenuBar;
		menuBar->Append( menuFile, "&File" );
		menuBar->Append( menuEdit, "&Edit" );
		menuBar->Append( menuBuild, "&Build" );
		menuBar->Append( menuWindow, "&Window" );
		menuBar->Append( devMenu, "&DevMenu" );


		SetMenuBar( menuBar );

		//
		// add status bar...
		//

		//int statusWidths[] = { 0, 80, -1 };
		//wxStatusBar* statusBar = CreateStatusBar( array_length(statusWidths), wxST_SIZEGRIP|wxFULL_REPAINT_ON_RESIZE, 0, wxT("Placeholder:") );
		//SetStatusWidths( array_length(statusWidths), statusWidths );
		//SetStatusText( "Placeholder:", 1 );
		//SetStatusText( "null", 2 );


		//
		// add toolbar
		//

		mToolbar = new wxToolBar( this, wxID_ANY );
		mToolbar->SetToolBitmapSize( wxSize( 16, 16 ) );
		mToolbar->AddTool( ID_New, "New Script", wxBitmap( "newdoc" ), wxNullBitmap, wxITEM_NORMAL, "Create a new, empty script" );
		mToolbar->AddTool( ID_Open, "Open Script", wxBitmap( "opendoc" ), wxNullBitmap, wxITEM_NORMAL, "Load an existing script" );
		mToolbar->AddTool( ID_Save, "Save Script", wxBitmap( "savedoc" ), wxNullBitmap, wxITEM_NORMAL, "Save current script" );
		mToolbar->AddTool( ID_SaveAll, "Save All Open", wxBitmap( "savedoc" ), wxNullBitmap, wxITEM_NORMAL, "Save all open scripts" );
		mToolbar->AddSeparator( );
		mToolbar->AddTool( ID_Undo, "Undo", wxBitmap( "undo" ), wxNullBitmap, wxITEM_NORMAL, "Undo the last action" );
		mToolbar->AddTool( ID_Redo, "Redo", wxBitmap( "redo" ), wxNullBitmap, wxITEM_NORMAL, "Redo the current action" );
		mToolbar->AddSeparator( );
		mToolbar->AddTool( ID_BuildFile, "Build Current Script", wxBitmap( "buildfile" ), wxNullBitmap, wxITEM_NORMAL, "Build the current open file" );
		mToolbar->AddSeparator( );
		mToolbar->AddTool( ID_BuildRes, "Build Res", wxBitmap( "buildres" ), wxNullBitmap, wxITEM_NORMAL, "Build the whole res directory" );
		mToolbar->AddSeparator( );
		mToolbar->AddTool( ID_DiffFile, "Diff File", wxBitmap( "difffile" ), wxNullBitmap, wxITEM_NORMAL, "Diff the current file against the depot" );
		mToolbar->AddTool( ID_ToggleEditorMode, "Toggle Editor View", wxBitmap( "difffile" ), wxNullBitmap, wxITEM_NORMAL, "Switch between designer mode and advanced mode" );

		mToolbar->Realize( );

		mAui.AddPane( mToolbar, wxAuiPaneInfo()
			.Name( wxT("main_toolbar") )
			.Caption( wxT("Tools") )
			.ToolbarPane()
			.Top()
			.LeftDockable(false)
			.RightDockable(false) );


		// 
		// add notebook panel
		//

		mScriptNotebook = new tScriptNotebook( this, &mRecentFiles );
		mAui.AddPane( mScriptNotebook, wxAuiPaneInfo()
			.Name( wxT("notebook_content") )
			.CenterPane()
			.PaneBorder(false) );

		{
			mBrowser = new tScriptBrowser( this, this, wxDefaultSize.y );
			mBrowser->fAddMenuOption( tMenuOptionPtr( new tOpenFileInNewWindowOption() ) );
			mBrowser->fAddMenuOption( tMenuOptionPtr( new tExploreToFileOption() ) );
			mAui.AddPane( mBrowser, wxAuiPaneInfo()
				.Name( wxT("script_browser") )
				.Caption( wxT("Script Browser") )
				.BestSize( wxSize(270,100) )
				.MinSize( wxSize(270,100) )
				.Right()
				.Floatable(true)
				.CloseButton(false) );
			mBrowser->fRefresh( );

			mLocmlControl = new tLocmlControlPanel( this, mScriptNotebook );
			mAui.AddPane( mLocmlControl, wxAuiPaneInfo()
				.Name( wxT("locml_control") )
				.Caption( wxT("Locml Control") )
				.BestSize( wxSize(270,380) ) // Empirically derived magic numbers!
				.MinSize( wxSize(270,380) )
				.FloatingSize( wxSize(270,380) )
				.Right()
				.Floatable(true)
				.CloseButton(true)
				.Hide() );
		}

		//
		// set up dialogs
		//

		mFindInFilesDialog = new tFindInFilesDialog( this, mBrowser, mScriptNotebook );
		mOpenFiles = new tSearchableOpenFilesDialog( this, mBrowser );

		//
		// center the window
		//

		Centre( );

		//
		// load layout and set up events for recent files
		//
		for( u32 i = 0; i < cMaxRecentlyOpenedFiles; ++i )
			Connect( ID_OpenRecent + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(tSigScriptWindow::fOnAction), 0, this );

		mRecentFiles.fLoad( );
		fLoadLayout( );

		//
		// set timers for on tick and changes monitoring.
		//

		mChangesTimer = new tCheckChangesTimer( this );
		mChangesTimer->Start( 100 );
		mOnTickTimer = new tOnTickTimer( this );
		mOnTickTimer->Start( 8 );

		//
		// context actions for right click menu (inside the script's frame)
		// 

		mContextActions.fPushBack( tEditorContextActionPtr( new tStandardContextActions( mScriptNotebook ) ) );
		mContextActions.fPushBack( tEditorContextActionPtr( new tInsertLocTextContextAction( mScriptNotebook ) ) );
		mContextActions.fPushBack( tEditorContextActionPtr( new tInsertSigImportContextAction( mScriptNotebook ) ) );
		mContextActions.fPushBack( tEditorContextActionPtr( new tInsertSigVarContextAction( mScriptNotebook ) ) );
		mContextActions.fPushBack( tEditorContextActionPtr( new tInsertSigExportContextAction( mScriptNotebook ) ) );

		tEditorContextActionList tabActions;
		tabActions.fPushBack( tEditorContextActionPtr( new tOpenInExplorerContextAction( mScriptNotebook ) ) );

		mScriptNotebook->fSetContextActions( tabActions, mContextActions );

		mScriptDataDialog = new ScriptData::tScriptDataDialog( this );

		mScriptProfilerDialog = new ScriptData::tScriptProfilerDialog( this );

		//debugging, go straight to perf browser
		//fShowPerformanceBrowser( );

		mAui.Update( );
	}

	tSigScriptWindow::~tSigScriptWindow( )
	{
		mAui.UnInit( );

		if( mChangesTimer )
		{
			mChangesTimer->Stop( );
			delete mChangesTimer;
		}
		if( mOnTickTimer )
		{
			mOnTickTimer->Stop( );
			delete mOnTickTimer;
		}
	}

	void tSigScriptWindow::fOnAbout(wxCommandEvent& WXUNUSED(event))
	{
		wxMessageBox( "This application allows you to edit scripts.",
					  "About SigScript", wxOK | wxICON_INFORMATION );
	}

	void tSigScriptWindow::fOnClose(wxCloseEvent& event)
	{
		// Returns true if the event should be vetoed.
		if( mScriptNotebook->fOnNotebookClose( ) )
		{
			event.Veto( );
			return;
		}
		else
		{
			// Not vetoed, pass it along.
			event.Skip( );
		}

		fSaveLayout( );
	}

	void tSigScriptWindow::fOnQuit(wxCommandEvent& event)
	{
		// Returns true if the event should be vetoed.
		if( mScriptNotebook->fOnNotebookClose( ) )
			return;

		fSaveLayout( );

		Close( true );
	}

	void tSigScriptWindow::fOpenDoc( const tFilePathPtr& path )
	{
		mScriptNotebook->fOpenDoc( path );
		fUpdateRecentFileMenu( );
	}

	void tSigScriptWindow::fClearAll( )
	{
		while( mScriptNotebook->GetPageCount( ) > 0 )
			mScriptNotebook->DeletePage( 0 );

		SetTitle( "SigScript" );
	}

	void tSigScriptWindow::fFocusAllOnLine( u64 lineNum )
	{
		mScriptNotebook->fFocusAllOnLine( lineNum );
	}

	void tSigScriptWindow::fOnAction(wxCommandEvent& event)
	{
		const s32 id = event.GetId( );

		if( id >= ID_OpenRecent && id < ID_OpenRecent + cMaxRecentlyOpenedFiles )
		{
			fOpenDoc( mRecentFiles[ id - ID_OpenRecent ] );
			return;
		}

		switch( id )
		{
		case ID_New:			mScriptNotebook->fNewDoc( ); break;
		case ID_Open:			
			{
				mOpenFiles->SetPosition( fGetCenter( mOpenFiles->GetSize( ) ) );

				tFilePathPtrList filesToOpen;
				mOpenFiles->fGetSelectedFiles( filesToOpen );

				for( u32 i = 0; i < filesToOpen.fCount( ); ++i )
					mScriptNotebook->fOpenDoc( filesToOpen[i] );

				fUpdateRecentFileMenu( );
				break;
			}
		case ID_Save:			mScriptNotebook->fSaveCurrent( ); fUpdateRecentFileMenu( ); break;
		case ID_SaveAll:		mScriptNotebook->fSaveAll( ); fUpdateRecentFileMenu( ); break;
		case ID_SaveAs:			mScriptNotebook->fSaveCurrentAs( ); fUpdateRecentFileMenu( ); break;
		case ID_CloseThis:		mScriptNotebook->fCloseCurrent( ); break;
		case ID_CloseAll:		mScriptNotebook->fCloseAll( ); SetTitle( "SigScript" ); break;
		case ID_CloseAllButThis: mScriptNotebook->fCloseAll( mScriptNotebook->fGetCurrent( ) );
		case ID_Undo:			mScriptNotebook->fUndo( ); break;
		case ID_Redo:			mScriptNotebook->fRedo( ); break;
		case ID_Find:			mFindInFilesDialog->fOpenFind( );	break;
		case ID_FindInFiles:	mFindInFilesDialog->fOpenFindInFiles( );	break;
		case ID_Replace:		mFindInFilesDialog->fOpenReplace( );	break;
		case ID_ReplaceInFiles:	mFindInFilesDialog->fOpenReplaceInFiles( );	break;
		case ID_GotoLine:		
			{
				const s32 max = mScriptNotebook->fGetCurrent( )->fGetEditor( )->GetLineCount( );
				const s32 current = mScriptNotebook->fGetCurrent( )->fGetEditor( )->GetCurrentLine( )+1; // Scintilla editor line 0 = visible line 1.
				tStrongPtr< tGotoLineDialog > gotoDialog( new tGotoLineDialog( this, current, max ) );

				gotoDialog->SetPosition( fGetCenter( gotoDialog->GetSize( ) ) );

				if( gotoDialog->ShowModal( ) == wxID_OK )
				{
					const s32 lineNumber = gotoDialog->fGetLineNumber( );
					mScriptNotebook->fGetCurrent( )->fGoToLine( lineNumber );
				}
				break;
			}

		case ID_SearchNext:
			{
				mFindInFilesDialog->fSetSelectedText( ); 
				mFindInFilesDialog->fFindNext( );	
				break;
			}
		case ID_SearchPrev:
			{
				mFindInFilesDialog->fSetSelectedText( ); 
				mFindInFilesDialog->fFindNext( true );	
				break;
			}
		case ID_ToUpper:
			{
				tScriptNotebookPage* currPage = mScriptNotebook->fGetCurrent( );
				if( !currPage )
					return;

				currPage->fGetEditor( )->fToUpper( );
				break;
			}
		case ID_BuildFile:
			{
				tScriptNotebookPage* currPage = mScriptNotebook->fGetCurrent( );
				if( !currPage )
					return;

				// Attempt to save before building.
				if( !currPage->fSaveDoc( ) )
					return;

				tFilePathPtr filePath = currPage->fGetFilePath( );
				std::stringstream cmdLine; cmdLine << "-pcdx9 -xbox360 -game2xbox -m -file " << ToolsPaths::fMakeResRelative( filePath ).fCStr( );
				std::stringstream exe; exe << ToolsPaths::fGetEngineBinFolder( ).fCStr( ) << "\\AssetGen.exe";

				Threads::tProcess::fSpawnAndForget( exe.str( ).c_str( ), cmdLine.str( ).c_str( ), ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) );

				break;
			}
		case ID_BuildRes:
			{
				mScriptNotebook->fSaveAll( );
				Threads::tProcess::fSpawnAndForget( NULL, "cmd.exe /C BuildRes.cmd", NULL, false, true );
				break;
			}

		case ID_DiffFile:
			{
				tScriptNotebookPage* currPage = mScriptNotebook->fGetCurrent( );
				if( !currPage )
					return;

				tFilePathPtr filePath = currPage->fGetFilePath( );
				std::string caseSensitivePath = Win32Util::fGetCaseSensitiveFile( filePath );

				std::stringstream cmdLine; cmdLine << "TortoiseProc.exe /command:diff /path:" << caseSensitivePath << " /closeonend:1";
				const s32 test = system( cmdLine.str( ).c_str( ) );
				break;
			}

		case ID_ToggleEditorMode:
			{
				mScriptNotebook->fToggleViewMode( );
				break;
			}

		case ID_ShowLocmlControl:
			{
				wxAuiPaneInfo& pane = mAui.GetPane( wxT("locml_control") );
				if( !pane.IsShown() )
				{
					pane.Float().Show();
					mAui.Update();
				}
				break;
			}

		case ID_ShowExportBrowser:
			{
				tScriptNotebookPage* currPage = mScriptNotebook->fGetCurrent( );
				if( !currPage )
					return;

				mScriptDataDialog->fShow( currPage->fGetSelectedText( ) );
				break;
			}

		case ID_ShowPerformanceBrowser:
			{
				fShowPerformanceBrowser( );
				break;
			}

		case ID_CommentSelection:
			{
				mScriptNotebook->fGetCurrent( )->fGetEditor( )->fCommentSelection( );
				break;
			}

		case ID_UncommentSelection:
			{
				mScriptNotebook->fGetCurrent( )->fGetEditor( )->fUncommentSelection( );
				break;
			}

		default:
			break;
		}
	}

	void tSigScriptWindow::fShowPerformanceBrowser( )
	{
		mScriptProfilerDialog->fShow( make_delegate_memfn( Sig::ScriptData::tScriptProfilerDialog::tOpenDocCallback, tSigScriptWindow, fOpenDoc ),
			make_delegate_memfn( Sig::ScriptData::tScriptProfilerDialog::tFocusAllOnLineCallback, tSigScriptWindow, fFocusAllOnLine ) );
	}

	void tSigScriptWindow::fUpdateRecentFileMenu( )
	{
		if( !mRecentFilesMenu )
			return;

		while( mRecentFilesMenu->GetMenuItems( ).size( ) > 0 )
			mRecentFilesMenu->Delete( mRecentFilesMenu->GetMenuItems( ).front( ) );

		const u32 min = fMin( cMaxRecentlyOpenedFiles, mRecentFiles.fCount( ) );
		for( u32 i = 0; i < min; ++i )
			mRecentFilesMenu->Append( mBaseRecentFileActionId + i, mRecentFiles[ i ].fCStr( ) );
	}

	void tSigScriptWindow::fSaveInternal( HKEY hKey )
	{
		Win32Util::fSetRegistryKeyValue( hKey, mScriptNotebook->GetSelection( ), "SelectedPage" );

		u32 pagesSaved = 0;
		for( u32 i = 0; i < mScriptNotebook->GetPageCount( ); ++i )
		{
			if( mScriptNotebook->fGetPage( i )->fNeedsSaveAs( ) )
				continue; // Do not save blank new scripts.

			const std::string keyValue = mScriptNotebook->fGetPage( i )->fGetFilePath( ).fCStr( );

			const std::string keyName = ToolsPaths::fGetCurrentProjectName( ) + StringUtil::fAppend( "\\ScriptPage", i );
			Win32Util::fSetRegistryKeyValue( hKey, keyValue, keyName.c_str( ) );
			++pagesSaved;
		}

		Win32Util::fSetRegistryKeyValue( hKey, pagesSaved, "PageCount" );
	}
	void tSigScriptWindow::fLoadInternal( HKEY hKey )
	{
		u32 numPages = 0;
		Win32Util::fGetRegistryKeyValue( hKey, numPages, "PageCount" );
		if( numPages > 0 )
		{
			for( u32 i = 0; i < numPages; ++i )
			{
				const std::string keyName = ToolsPaths::fGetCurrentProjectName( ) + StringUtil::fAppend( "\\ScriptPage", i );
				std::string keyValue;
				if( Win32Util::fGetRegistryKeyValue( hKey, keyValue, keyName.c_str( ) ) && keyValue.length( ) > 0 )
					fOpenDoc( tFilePathPtr( keyValue ) );
			}
		}

		u32 selectedPage = 0;
		if( Win32Util::fGetRegistryKeyValue( hKey, selectedPage, "SelectedPage" ) )
		{
			mScriptNotebook->SetSelection( selectedPage );
		}
	}

	void tSigScriptWindow::fSaveLayout( )
	{
		if( IsIconized( ) || !IsShown( ) )
			return; // window is minimized, don't save

		tSigScriptWindowSavedLayout layout( mSavedLayout.fRegistryKeyName( ) );
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

		fSave( );
	}

	void tSigScriptWindow::fLoadLayout( )
	{
		fLoad( );
		fUpdateRecentFileMenu( );

		if( mSavedLayout.fLoad( ) && mSavedLayout.mVisible )
		{
			mSavedLayout.fToWxWindow( this );
			if( mSavedLayout.mMaximized )
				Maximize( );
		}
		else
		{
			SetSize( 1200, 800 );
			Center( );
			Show( true );
		}
	}

	void tSigScriptWindow::fOnPageChange(wxAuiNotebookEvent& event)
	{
		tScriptNotebookPage* currentEd = mScriptNotebook->fGetCurrent( );
		if( !currentEd )
		{
			SetTitle( "SigScript" );
			return;
		}

		std::stringstream title; title << "SigScript ~ " << ToolsPaths::fMakeResRelative( currentEd->fGetFilePath( ) ).fCStr( );
		SetTitle( title.str( ).c_str( ) );
	}

	void tSigScriptWindow::fOnFilesDropped(wxDropFilesEvent& event)
	{
		const u32 numFiles = event.GetNumberOfFiles( );
		const wxString* files = event.GetFiles( );
		for( u32 i = 0; i < numFiles; ++i )
		{
			const wxString thisFile = files[ i ];
			tFilePathPtr filePath( thisFile.c_str( ) );
			const b32 isScript = fIsSupportedFile( filePath );
			if( isScript )
			{
				fOpenDoc( filePath );
			}
		}
	}

}
