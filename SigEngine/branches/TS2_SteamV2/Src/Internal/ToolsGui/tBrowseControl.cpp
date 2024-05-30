//------------------------------------------------------------------------------
// \file tBrowseControl.cpp - 19 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsGuiPch.hpp"
#include "tBrowseControl.hpp"
#include "tStrongPtr.hpp"
#include "WxUtil.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tBrowseControl
	//------------------------------------------------------------------------------
	tBrowseControl::tBrowseControl( wxWindow* parent, wxBoxSizer* ownerSizer, const char* name )
		: mParent( parent ), mTextBox( 0 )
	{
		wxBoxSizer* vbox = new wxBoxSizer( wxVERTICAL );
		wxBoxSizer* hbox0 = new wxBoxSizer( wxHORIZONTAL );
		wxBoxSizer* hbox1 = new wxBoxSizer( wxHORIZONTAL );

		wxStaticText* staticText = new wxStaticText( 
			parent, wxID_ANY, name, wxDefaultPosition, wxDefaultSize );
		wxButton* browseButton = new wxButton( parent, -1, wxT( "Browse" ) );

		hbox0->Add( staticText, 1, wxLEFT | wxTOP, 3 );
		hbox0->Add( browseButton, 1, wxLEFT, 0  );

		mTextBox = new wxTextCtrl( parent, wxID_ANY );
		mTextBox->SetMinSize( wxSize( 400, -1 ) );
		hbox1->Add( mTextBox, 1, wxLEFT | wxTOP, 3 );

		vbox->Add( hbox0, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 10 );
		vbox->Add( hbox1, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxEXPAND, 10 );

		ownerSizer->Add( vbox, 0, wxALIGN_LEFT | wxTOP | wxBOTTOM | wxEXPAND, 10 );

		browseButton->Connect(
			wxEVT_COMMAND_BUTTON_CLICKED, 
			wxCommandEventHandler(tBrowseControl::fOnBrowseInternal), 
			NULL, this);
	}

	//------------------------------------------------------------------------------
	void tBrowseControl::fSetText( const wxString& text )
	{
		mTextBox->SetValue( text );
	}

	//------------------------------------------------------------------------------
	wxString tBrowseControl::fGetText( ) const
	{
		return mTextBox->GetValue( );
	}

	//------------------------------------------------------------------------------
	void tBrowseControl::fOnBrowseInternal( wxCommandEvent & )
	{
		fOnBrowse( );
	}

	//------------------------------------------------------------------------------
	// tFolderBrowseControl
	//------------------------------------------------------------------------------
	tFolderBrowseControl::tFolderBrowseControl( 
		wxWindow* parent, wxBoxSizer* ownerSizer, const char* name )
		: tBrowseControl( parent, ownerSizer, name )
	{
	}

	//------------------------------------------------------------------------------
	void tFolderBrowseControl::fOnBrowse( )
	{
		tStrongPtr<wxDirDialog> openFileDialog( new wxDirDialog( fParentWindow( ) ) );

		if( openFileDialog->ShowModal( ) == wxID_OK )
			fSetText( openFileDialog->GetPath( ) );
	}

	//------------------------------------------------------------------------------
	// tFileBrowseControl
	//------------------------------------------------------------------------------
	tFileBrowseControl::tFileBrowseControl( 
		wxWindow * parent, 
		wxBoxSizer * ownerSizer, 
		const char * name, 
		const char * fileFilter )
		: tBrowseControl( parent, ownerSizer, name )
		, mFileFilter( fileFilter )
	{
	}

	//------------------------------------------------------------------------------
	void tFileBrowseControl::fOnBrowse( )
	{
		std::string path;
		b32 picked = WxUtil::fBrowseForFile( 
			path, fParentWindow( ), "", "", "", mFileFilter.c_str( ) );

		if( picked )
			fSetText( path.c_str( ) );
	}
}
