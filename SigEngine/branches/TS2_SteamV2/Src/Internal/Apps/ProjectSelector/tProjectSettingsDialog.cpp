#include "ProjectSelectorPch.hpp"
#include "tProjectSettingsDialog.hpp"

namespace Sig
{
	BEGIN_EVENT_TABLE(tProjectSettingsDialog, wxDialog)
		EVT_CLOSE(			tProjectSettingsDialog::fOnClose)
	END_EVENT_TABLE()

	tProjectSettingsDialog::tProjectSettingsDialog( wxWindow* parent )
	   : wxDialog( parent, wxID_ANY, "Project Settings", wxPoint(-1,-1), wxSize(-1,-1) )
	   , mProfileName( 0 )
	   , mWorkSpaceOverride( 0 )
	   , mActiveProject( 0 )
	{
		wxPanel* panel = new wxPanel( this );

		wxBoxSizer* vbox = new wxBoxSizer( wxVERTICAL );

		{
			wxBoxSizer* hbox2 = new wxBoxSizer( wxHORIZONTAL );
			wxStaticText* staticText = new wxStaticText( this, wxID_ANY, "Profile Name:", wxDefaultPosition, wxDefaultSize );
			mProfileName = new wxTextCtrl( this, wxID_ANY, "" );
			mProfileName->SetMinSize( wxSize( 200, -1 ) );
			hbox2->Add( staticText, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 3 );
			hbox2->Add( mProfileName, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxLEFT, 5 );
			vbox->Add( hbox2, 0, wxALIGN_RIGHT | wxTOP | wxLEFT | wxRIGHT, 10 );
		}

		{
			wxBoxSizer* hbox2 = new wxBoxSizer( wxHORIZONTAL );
			wxStaticText* staticText = new wxStaticText( this, wxID_ANY, "Workspace Override:", wxDefaultPosition, wxDefaultSize );
			mWorkSpaceOverride = new wxTextCtrl( this, wxID_ANY, "" );
			mWorkSpaceOverride->SetMinSize( wxSize( 200, -1 ) );
			hbox2->Add( staticText, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 3 );
			hbox2->Add( mWorkSpaceOverride, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxLEFT, 5 );
			vbox->Add( hbox2, 0, wxALIGN_RIGHT | wxTOP | wxLEFT | wxRIGHT, 10 );
		}

		mEnginePath.fReset( new tFolderBrowseControl( this, vbox, "Engine Path" ) );
		mProjectPath.fReset( new tFolderBrowseControl( this, vbox, "Project Path" ) );

		wxBoxSizer* hbox0 = new wxBoxSizer( wxHORIZONTAL );
		mActiveProject = new wxCheckBox( this, wxID_ANY, wxT("Active Project") );
		hbox0->Add( mActiveProject, 1 );

		wxBoxSizer* hbox1 = new wxBoxSizer( wxHORIZONTAL );
		wxButton* okButton = new wxButton( this, wxID_ANY, wxT("Ok") );
		wxButton* closeButton = new wxButton( this, wxID_ANY, wxT("Cancel") );

		okButton->Connect(		wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectSettingsDialog::fOnOk), NULL, this );
		closeButton->Connect(	wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectSettingsDialog::fOnCancel), NULL, this );

		hbox1->Add( okButton, 1 );
		hbox1->Add( closeButton, 1, wxLEFT | wxRIGHT, 5);

		vbox->Add( panel, 1 );
		vbox->Add( hbox0, 0, wxALIGN_LEFT | wxLEFT, 15 );
		vbox->Add( hbox1, 0, wxALIGN_RIGHT | wxALL, 10 );

		SetSizer( vbox );
		Fit( );
	}

	tProjectSettingsDialog::~tProjectSettingsDialog( )
	{
	}

	b32 tProjectSettingsDialog::fDoDialog( ToolsPaths::tProjectProfile& projectProfile, b32 newProfile )
	{
		Center( );

		if( newProfile )
		{
			// clear controls
			mProfileName->SetValue( "" );
			mWorkSpaceOverride->SetValue( "" );
			mEnginePath->fSetText( "" );
			mProjectPath->fSetText( "" );
			mActiveProject->SetValue( false );
		}
		else
		{
			// fill controls with values from projectProfile
			mProfileName->SetValue( projectProfile.mProfileName.c_str( ) );
			mWorkSpaceOverride->SetValue( projectProfile.mWorkSpaceOverride.c_str( ) );
			mEnginePath->fSetText( projectProfile.mEnginePath.fCStr( ) );
			mProjectPath->fSetText( projectProfile.mProjectPath.fCStr( ) );
			mActiveProject->SetValue( projectProfile.mActiveProfile!=0 );
		}

		if( ShowModal( ) == wxID_OK )
		{
			// fill in profile with new values

			projectProfile.mProfileName = mProfileName->GetValue( );
			projectProfile.mWorkSpaceOverride = mWorkSpaceOverride->GetValue( );
			projectProfile.mEnginePath = tFilePathPtr( mEnginePath->fGetText( ) );
			projectProfile.mProjectPath = tFilePathPtr( mProjectPath->fGetText( ) );
			projectProfile.mActiveProfile = mActiveProject->GetValue( )!=0;

			return true;
		}

		return false;
	}

	void tProjectSettingsDialog::fOnClose(wxCloseEvent& event)
	{
		MakeModal( false );
		Show( false );
	}

	void tProjectSettingsDialog::fOnOk(wxCommandEvent& event)
	{
		SetReturnCode( wxID_OK );
		Close( true );
	}

	void tProjectSettingsDialog::fOnCancel(wxCommandEvent& event)
	{
		SetReturnCode( 0 );
		Close( true );
	}

}
