//------------------------------------------------------------------------------
// \file tGotoLineDialog.cpp - 12 Aug 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "SigScriptPch.hpp"
#include "tGotoLineDialog.hpp"

namespace Sig
{
	static const u32 cItemSpacer			= 6;
	static const u32 cDialogFlags = wxDEFAULT_DIALOG_STYLE | wxTAB_TRAVERSAL | wxSTAY_ON_TOP;

	//------------------------------------------------------------------------------
	// Dialog section
	//------------------------------------------------------------------------------
	tGotoLineDialog::tGotoLineDialog( wxWindow* parent, s32 current, s32 max )
		: wxDialog( parent, wxID_ANY, "Go To Line", wxDefaultPosition, wxSize(300, 110), cDialogFlags )
		, mMax( max )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetBackgroundColour( GetBackgroundColour( ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		std::stringstream lineText;
		lineText << "Line Number (1 - " << mMax << "):";

		wxStaticText* staticText = new wxStaticText( this, wxID_ANY, lineText.str( ).c_str( ), wxDefaultPosition, wxDefaultSize );
		GetSizer( )->Add( staticText, 0, wxALL, cItemSpacer );

		mLineNumber = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator( wxFILTER_NUMERIC )	 );
		GetSizer( )->Add( mLineNumber, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, cItemSpacer );

		std::stringstream ss;
		ss << current;
		mLineNumber->SetValue( ss.str( ) );

		// Add button that will handle Esc and also closing the dialog too.
		wxSizer* buttonsSizer = CreateButtonSizer( wxOK | wxCANCEL );
		GetSizer( )->Add( buttonsSizer, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT, cItemSpacer );
		GetSizer( )->AddSpacer( cItemSpacer );

		Layout( );
		Refresh( );

		mLineNumber->SetFocus( );
		mLineNumber->SelectAll( );
	}

	s32 tGotoLineDialog::fGetLineNumber( ) const
	{
		wxString textBoxValue = mLineNumber->GetValue( );

		std::stringstream ss;
		ss << std::fixed << textBoxValue;

		s32 value = 0;
		ss >> value;

		// They are supposed to enter from 1 to max.
		--value;

		value = fClamp( value, 0, mMax );

		return value;
	}
}
