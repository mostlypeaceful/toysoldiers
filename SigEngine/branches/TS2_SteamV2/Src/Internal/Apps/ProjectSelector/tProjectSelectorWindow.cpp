#include "ProjectSelectorPch.hpp"
#include "tProjectSelectorWindow.hpp"
#include "FileSystem.hpp"
#include "tProjectFile.hpp"
#include "tProjectXMLDialog.hpp"

namespace Sig
{

	enum
	{
		ID_Quit = 1,
		ID_About,
		ID_Activate,
		ID_SetupDefault,
		ID_ModifyGameSettings,
		ID_CompileGameSettings,
	};

	BEGIN_EVENT_TABLE(tProjectSelectorWindow, wxFrame)
		EVT_CLOSE(									tProjectSelectorWindow::fOnClose)
		EVT_MENU(				ID_About,			tProjectSelectorWindow::fOnAbout)
		EVT_MENU(				ID_Quit,			tProjectSelectorWindow::fOnQuit)
		EVT_MENU(				ID_Activate,		tProjectSelectorWindow::fSetActiveProject)
		EVT_MENU(				ID_SetupDefault,	tProjectSelectorWindow::fSetupDefaultsForActiveProject)
		EVT_MENU(				ID_ModifyGameSettings,	tProjectSelectorWindow::fModifyGameSettings)
		EVT_MENU(				ID_CompileGameSettings,	tProjectSelectorWindow::fCompileGameSettings)
		EVT_MENU(				wxID_NEW,			tProjectSelectorWindow::fOnNew)
		EVT_MENU(				wxID_EDIT,			tProjectSelectorWindow::fOnEdit)
		EVT_MENU(				wxID_DELETE,		tProjectSelectorWindow::fOnDelete)
	END_EVENT_TABLE()

	tProjectSelectorWindow::tProjectSelectorWindow( const wxString& title/*, const wxPoint& pos, const wxSize& size*/ )
		   : wxFrame( (wxFrame *)NULL, -1, title, wxPoint(-1,-1), wxSize(600,300) )
		   , mListBox( 0 )
	{
		//
		// set icon...
		//

		SetIcon(wxIcon("appicon"));

		//
		// add menu...
		//

		wxMenu *menuFile = new wxMenu;
		menuFile->Append( ID_About, "&About..." );
		menuFile->AppendSeparator();
		menuFile->Append( ID_Quit, "E&xit" );
		wxMenuBar *menuBar = new wxMenuBar;
		menuBar->Append( menuFile, "&File" );
		SetMenuBar( menuBar );

		//
		// add status bar...
		//

		int statusWidths[] = { 0, 80, -1 };
		wxStatusBar* statusBar = CreateStatusBar( array_length(statusWidths), wxST_SIZEGRIP|wxFULL_REPAINT_ON_RESIZE, 0, wxT("Active Profile:") );
		SetStatusWidths( array_length(statusWidths), statusWidths );
		SetStatusText( "Active Profile:", 1 );
		SetStatusText( "null", 2 );

		//
		// add main background panel...
		//

		wxPanel *panel = new wxPanel( this, wxID_ANY );

		wxBoxSizer *vbox = new wxBoxSizer( wxVERTICAL );
		panel->SetSizer( vbox );

		//
		// add Existing Profiles static text...
		//

		wxBoxSizer* hbox0 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox0, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

		wxStaticText* staticText = new wxStaticText( panel, wxID_ANY, wxT("Existing Profiles"), wxDefaultPosition, wxDefaultSize );
		hbox0->Add( staticText, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxLEFT, 5 );

		//
		// add List Box of profiles...
		//

		wxBoxSizer* hbox1 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox1, 1, wxGROW | wxALL, 0 );

		mListBox = new wxListBox( panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxArrayString(), wxLB_SINGLE );
		hbox1->Add( mListBox, 1, wxGROW | wxALL, 5 );

		//
		// add Button for adding new profiles...
		//

		wxBoxSizer* hbox2 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox2, 0, wxALIGN_RIGHT | wxALL, 0 );

		wxButton *newProfile = new wxButton(panel, wxID_ANY, wxT("New Profile"), wxDefaultPosition, wxDefaultSize );
		hbox2->Add( newProfile, 0, wxALIGN_BOTTOM | wxALL, 5 );


		//
		// hook up events...
		//

		newProfile->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectSelectorWindow::fOnNew), NULL, this);

		mListBox->Connect(wxEVT_LEFT_DCLICK,	wxMouseEventHandler(tProjectSelectorWindow::fOnListBoxDClick),	NULL, this);
		mListBox->Connect(wxEVT_RIGHT_UP,		wxMouseEventHandler(tProjectSelectorWindow::fOnListBoxRUp),	NULL, this);

		//
		// center the window
		//

		Centre( );

		//
		// create properties dialog, but don't show it yet
		//

		fCreateSettingsDialog( );

		//
		// load profiles saved to registry
		//

		fLoadProfiles( );
	}

	tProjectSelectorWindow::~tProjectSelectorWindow( )
	{
		// delete all profiles
		for( u32 index = 0; index < ( s32 )mListBox->GetCount( ); ++index )
			delete ( ToolsPaths::tProjectProfile* )mListBox->GetClientData( index );
	}

	void tProjectSelectorWindow::fCreateSettingsDialog( )
	{
		mSettingsDialog.fReset( new tProjectSettingsDialog( this ) );
	}

	ToolsPaths::tProjectProfile* tProjectSelectorWindow::fGetProfileAt( int index )
	{
		if( index >= 0 && index < ( s32 )mListBox->GetCount( ) )
			return ( ToolsPaths::tProjectProfile* )mListBox->GetClientData( index );
		return 0;
	}

	void tProjectSelectorWindow::fAddProfile( const ToolsPaths::tProjectProfile& profile )
	{
		ToolsPaths::tProjectProfile* newProfile = new ToolsPaths::tProjectProfile( profile );

		const b32 isActive = newProfile->mActiveProfile;
		newProfile->mActiveProfile = false;
		const int index = mListBox->Append( profile.fToString( ), newProfile );
		if( isActive )
			fActivateProfile( index );
	}

	void tProjectSelectorWindow::fEditCurrentProfile( )
	{
		const s32 index = mListBox->GetSelection( );
		ToolsPaths::tProjectProfile* profile = fGetProfileAt( index );
		if( profile )
		{
			if( mSettingsDialog->fDoDialog( *profile, false ) )
			{
				const b32 isActive = profile->mActiveProfile;
				profile->mActiveProfile = false;
				mListBox->SetString( index, profile->fToString( ).c_str( ) );
				if( isActive )
					fActivateProfile( index );
			}
		}
	}

	void tProjectSelectorWindow::fActivateProfile( int index )
	{
		if( !fGetProfileAt( index ) )
			return; // nothing is selected

		// go through all objects and de-activate everyone else
		for( u32 i = 0; i < mListBox->GetCount( ); ++i )
		{
			ToolsPaths::tProjectProfile* profile = fGetProfileAt( i );
			sigassert( profile );

			if( i == index )
			{
				profile->mActiveProfile = true;
				profile->fConfigureEnvironment( );
				SetStatusText( profile->mProfileName.c_str( ), 2 );
			}
			else
			{
				profile->mActiveProfile = false;
			}

			mListBox->SetString( i, profile->fToString( ).c_str( ) );
		}
	}

	void tProjectSelectorWindow::fLoadProfiles( )
	{
		tGrowableArray<ToolsPaths::tProjectProfile> savedProfiles;
		ToolsPaths::tProjectProfile::fLoadProfilesFromRegistry( savedProfiles );
		for( u32 i = 0; i < savedProfiles.fCount( ); ++i )
			fAddProfile( savedProfiles[i] );
	}

	void tProjectSelectorWindow::fSaveProfiles( )
	{
		tGrowableArray<ToolsPaths::tProjectProfile> savedProfiles( mListBox->GetCount( ) );
		for( u32 i = 0; i < savedProfiles.fCount( ); ++i )
			savedProfiles[i] = *fGetProfileAt( i );
		ToolsPaths::tProjectProfile::fSaveProfilesToRegistry( savedProfiles );
	}

	void tProjectSelectorWindow::fOnAbout(wxCommandEvent& WXUNUSED(event))
	{
		wxMessageBox( "This application allows you to create and edit project profiles, as well as set the active project.",
					  "About ProjectSelector", wxOK | wxICON_INFORMATION );
	}

	void tProjectSelectorWindow::fOnClose(wxCloseEvent& event)
	{
		fSaveProfiles( );
		event.Skip( );
	}

	void tProjectSelectorWindow::fOnQuit(wxCommandEvent& WXUNUSED(event))
	{
		Close( true );
	}

	void tProjectSelectorWindow::fSetActiveProject(wxCommandEvent& event)
	{
		fActivateProfile( mListBox->GetSelection( ) );
	}

	void tProjectSelectorWindow::fSetupDefaultsForActiveProject(wxCommandEvent& event)
	{
		// create project root folder
		FileSystem::fCreateDirectory( ToolsPaths::fGetCurrentProjectRootFolder( ) );

		// create project res folder
		FileSystem::fCreateDirectory( ToolsPaths::fGetCurrentProjectResFolder( ) );

		// create engine root folder
		FileSystem::fCreateDirectory( ToolsPaths::fGetEngineRootFolder( ) );

		// create the project file
		tFilePathPtr currentProjectFile = ToolsPaths::fGetCurrentProjectFilePath( );
		if( !FileSystem::fFileExists( currentProjectFile ) )
		{
			tProjectFile defaultFile;
			defaultFile.fSetupProjectDefaults( );
			defaultFile.fSaveXml( currentProjectFile, true );
		}

	}

	void tProjectSelectorWindow::fCompileGameSettings(wxCommandEvent& event)
	{
		tFilePathPtr currentProjectFile = ToolsPaths::fGetCurrentProjectFilePath( );
		if( FileSystem::fFileExists( currentProjectFile ) )
		{
			tProjectFile projFile;
			projFile.fLoadXml( currentProjectFile );
			projFile.fCompileGameSettings( true );
		}
	}

	void tProjectSelectorWindow::fModifyGameSettings(wxCommandEvent& event)
	{
		tProjectXMLDialog* diag = new tProjectXMLDialog( this );
		diag->Show( true );
	}

	void tProjectSelectorWindow::fOnNew(wxCommandEvent& event)
	{
		ToolsPaths::tProjectProfile profile;
		if( mSettingsDialog->fDoDialog( profile, true ) )
			fAddProfile( profile );
	}

	void tProjectSelectorWindow::fOnEdit(wxCommandEvent& event)
	{
		fEditCurrentProfile( );
	}

	void tProjectSelectorWindow::fOnDelete(wxCommandEvent& event)
	{
		if( mListBox->GetSelection( ) >= 0 && mListBox->GetSelection( ) < ( s32 )mListBox->GetCount( ) )
		{
			const int index = mListBox->GetSelection( );
			ToolsPaths::tProjectProfile* profile = fGetProfileAt( index );
			if( profile )
				delete profile;
			mListBox->Delete( index );
		}
	}

	void tProjectSelectorWindow::fOnListBoxDClick(wxMouseEvent& event)
	{
		fSetActiveProject( wxCommandEvent( ) );
	}

	void tProjectSelectorWindow::fOnListBoxRUp(wxMouseEvent& event)
	{
		wxPoint pos = event.GetPosition();

		wxMenu menu;

		menu.Append(ID_Activate, _T("&Set as Active Project"));

		ToolsPaths::tProjectProfile* currentProfile = fGetProfileAt( mListBox->GetSelection( ) );
		if( currentProfile && currentProfile->mActiveProfile )
		{
			menu.AppendSeparator();
			menu.Append(ID_SetupDefault, _T("&Create Default Directory Structure"));
			menu.Append(ID_ModifyGameSettings, _T("&Modify Game Settings"));
			menu.Append(ID_CompileGameSettings, _T("Compile &Game Settings"));
		}
		menu.AppendSeparator();
		menu.Append(wxID_NEW, _T("&New"));
		menu.Append(wxID_EDIT, _T("&Edit"));
		menu.AppendSeparator();
		menu.Append(wxID_DELETE, _T("Delete"));

		PopupMenu(&menu, pos.x, pos.y);
	}

}
