#include "ToolsGuiPch.hpp"
#include "FileSystem.hpp"
#include "tExplicitDependenciesDialog.hpp"
#include "tWxSlapOnGroup.hpp"
#include "tStrongPtr.hpp"

namespace Sig
{
	tExplicitDependenciesDialog::tExplicitDependenciesDialog( wxFrame* myWindow )
		: mMyWindow( myWindow )
		, mMainPanel( 0 )
		, mListBox( 0 )
	{
		myWindow->SetIcon( wxIcon( "appicon" ) );
		myWindow->SetTitle( "Add Explicit Dependencies" );

		mMainPanel = new wxScrolledWindow( myWindow );
		mMainPanel->SetBackgroundColour( myWindow->GetBackgroundColour( ) );
		myWindow->SetSizer( new wxBoxSizer( wxVERTICAL ) );
		myWindow->GetSizer( )->Add( mMainPanel, 1, wxEXPAND | wxALL, 0 );

		fBuildGui( );
		fRefreshList( );
	}

	void tExplicitDependenciesDialog::fBuildGui( )
	{
		wxBoxSizer *vbox = new wxBoxSizer( wxVERTICAL );
		mMainPanel->SetSizer( vbox );

		//
		// add Existing Resources static text...
		//

		wxBoxSizer* hbox0 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox0, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

		wxStaticText* staticText = new wxStaticText( mMainPanel, wxID_ANY, wxT("Existing References"), wxDefaultPosition, wxDefaultSize );
		hbox0->Add( staticText, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxLEFT, 5 );

		//
		// add List Box of resources...
		//

		wxBoxSizer* hbox1 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox1, 1, wxGROW | wxALL, 0 );

		mListBox = new wxListBox( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxArrayString(), wxLB_SINGLE );
		hbox1->Add( mListBox, 1, wxGROW | wxALL, 5 );

		//
		// add Buttons...
		//

		wxBoxSizer* hbox2 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox2, 0, wxALIGN_RIGHT | wxALL, 0 );

		wxButton* selectItems = new wxButton( mMainPanel, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize );
		hbox2->Add( selectItems, 0, wxALIGN_BOTTOM | wxALL, 5 );

		wxButton* refreshList = new wxButton( mMainPanel, wxID_ANY, wxT("Remove"), wxDefaultPosition, wxDefaultSize );
		hbox2->Add( refreshList, 0, wxALIGN_BOTTOM | wxALL, 5 );


		// connect events

		selectItems->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tExplicitDependenciesDialog::fOnAddPressed), NULL, this );
		refreshList->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tExplicitDependenciesDialog::fOnSubPressed), NULL, this );
		mListBox->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler(tExplicitDependenciesDialog::fOnListBoxDClick), NULL, this);

		mMyWindow->Layout( );
		mMyWindow->Refresh( );
	}

	tFilePathPtr tExplicitDependenciesDialog::fGetSelectedPath( ) const
	{
		wxString listBoxSelectedString = mListBox->GetString( mListBox->GetSelection( ) );
		return tFilePathPtr( listBoxSelectedString.c_str( ) );
	}

	struct tSortFilePathPtrAlphabetically
	{
		inline b32 operator()( const tFilePathPtr& a, const tFilePathPtr& b ) const
		{
			return _stricmp( a.fCStr( ), b.fCStr( ) ) < 0;
		}
	};

	namespace
	{
		static const wxString cAllowedFileTypes( "*.mshml;*.sigml;*.nut;*.anipk;*.fxml;*.goaml" );
	}

	void tExplicitDependenciesDialog::fOnAddPressed( wxCommandEvent& event )
	{
		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mMyWindow, 
			"Add Dependency",
			wxString( "" ),
			wxString( "" ),
			cAllowedFileTypes,
			wxFD_OPEN ) );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			fAdd( ToolsPaths::fMakeResRelative( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) ) );
			fOnChanged( );
		}
	}

	void tExplicitDependenciesDialog::fOnSubPressed( wxCommandEvent& event )
	{
		fRemove( fGetSelectedPath( ) );
		fOnChanged( );
	}

	void tExplicitDependenciesDialog::fOnListBoxDClick( wxMouseEvent& event )
	{
		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mMyWindow, 
			"Select New Dependency",
			wxString( "" ),
			wxString( "" ),
			cAllowedFileTypes,
			wxFD_OPEN ) );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			fRemove( fGetSelectedPath( ) );
			fAdd( ToolsPaths::fMakeResRelative( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) ) );
			fOnChanged( );
		}
	}

	void tExplicitDependenciesDialog::fAdd( const tFilePathPtr& path )
	{
		mDependencyList.fFindOrAdd( path );
		fRefreshList( );
	}

	void tExplicitDependenciesDialog::fRemove( const tFilePathPtr& path )
	{
		mDependencyList.fFindAndErase( path );
		fRefreshList( );
	}

	void tExplicitDependenciesDialog::fRefreshList( )
	{
		mListBox->Clear( );

		std::sort( mDependencyList.fBegin( ), mDependencyList.fEnd( ), tSortFilePathPtrAlphabetically( ) );

		for( u32 i = 0; i < mDependencyList.fCount( ); ++i )
		{
			wxString listBoxString = mDependencyList[ i ].fCStr( );
			mListBox->Append( listBoxString );
		}
	}

}

