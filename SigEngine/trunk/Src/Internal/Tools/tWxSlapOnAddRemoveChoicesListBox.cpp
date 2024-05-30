//------------------------------------------------------------------------------
// \file tWxSlapOnAddRemoveChoicesListBox.cpp - 14 Apr 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tWxSlapOnAddRemoveChoicesListBox.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tWxSlapOnAddRemoveChoicesListBox::tWxSlapOnAddRemoveChoicesListBox( 
		wxWindow* parent, 
		const char* label, 
		b32 sorted, 
		b32 uniqueItems,
		const wxString enumNames[], 
		u32 numEnumNames, 
		u32 defChoice)
		: tWxSlapOnAddRemoveListBox( parent, label, sorted, uniqueItems )
	{
		fSetupChoices( parent, enumNames, numEnumNames, defChoice );
	}

	//------------------------------------------------------------------------------
	tWxSlapOnAddRemoveChoicesListBox::tWxSlapOnAddRemoveChoicesListBox( 
		wxWindow* parent, 
		const char* label, 
		b32 sorted, 
		b32 uniqueItems,
		const char * addLabel,
		const char * removeLabel,
		const wxString enumNames[], 
		u32 numEnumNames, 
		u32 defChoice)
		: tWxSlapOnAddRemoveListBox( parent, label, sorted, uniqueItems, addLabel, removeLabel )
	{
		fSetupChoices( parent, enumNames, numEnumNames, defChoice );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnAddRemoveChoicesListBox::fDisableControl( )
	{
		tWxSlapOnAddRemoveListBox::fDisableControl( );
		mChoice->Disable( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnAddRemoveChoicesListBox::fEnableControl( )
	{
		tWxSlapOnAddRemoveListBox::fEnableControl( );
		mChoice->Enable( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnAddRemoveChoicesListBox::fOnAdd( )
	{
		const s32 sel = mChoice->GetSelection( );
		if( sel >= 0 && sel < ( s32 )mChoice->GetCount( ) )
			fAddItem( mChoice->GetString( sel ) );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnAddRemoveChoicesListBox::fSetupChoices(
		wxWindow * parent,
		const wxString enumNames[], 
		u32 numEnumNames, 
		u32 defChoice )
	{
		wxBoxSizer * sizer = new wxBoxSizer( wxHORIZONTAL );
		sizer->Add( fLabelWidth( ) + 2 * fSizerPadding( ) + 1, 0 );

		mChoice = new wxChoice( 
			parent, 
			wxID_ANY, 
			wxDefaultPosition, 
			wxSize( fControlWidth( ), wxDefaultSize.y ), 
			numEnumNames, 
			enumNames );

		sizer->Add( mChoice, 1, 0, fSizerPadding( ) );

		if( defChoice < numEnumNames )
			mChoice->Select( defChoice );
		else
			mChoice->Select( 0 );

		//fAddWindowToSizer( mChoice, false );

		parent->GetSizer( )->Add( sizer );
	}

}
