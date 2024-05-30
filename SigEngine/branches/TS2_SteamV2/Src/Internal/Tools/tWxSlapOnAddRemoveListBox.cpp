//------------------------------------------------------------------------------
// \file tWxSlapOnAddRemoveListBox.cpp - 18 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tWxSlapOnAddRemoveListBox.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tWxSlapOnAddRemoveListBox::tWxSlapOnAddRemoveListBox( 
		wxWindow* parent, 
		const char* label, 
		b32 sorted, 
		b32 uniqueItems )
		: tWxSlapOnListBox( parent, label, sorted, uniqueItems )
	{
		fSetupButtons( parent, "Add", "Remove" );
	}

	//------------------------------------------------------------------------------
	tWxSlapOnAddRemoveListBox::tWxSlapOnAddRemoveListBox( 
		wxWindow* parent, 
		const char* label, 
		b32 sorted, 
		b32 uniqueItems,
		const char * addLabel,
		const char * removeLabel )
		: tWxSlapOnListBox(parent, label, sorted, uniqueItems)
	{
		fSetupButtons( parent, addLabel, removeLabel );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnAddRemoveListBox::fDisableControl( )
	{
		tWxSlapOnListBox::fDisableControl( );
		mAddButton->Disable( );
		mRemoveButton->Disable( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnAddRemoveListBox::fEnableControl( )
	{
		tWxSlapOnListBox::fEnableControl( );
		mAddButton->Enable( );
		mRemoveButton->Enable( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnAddRemoveListBox::fSetupButtons( 
		wxWindow * parent, const char * addLabel, const char * removeLabel )
	{
		mButtonSizer = new wxBoxSizer( wxHORIZONTAL );
		mButtonSizer->Add( fLabelWidth( ) + fSizerPadding( ) + 1, 0 );

		const s32 buttonHeight = 20;

		mAddButton = new wxButton(
			parent,
			wxID_ANY,
			addLabel,
			wxDefaultPosition,
			wxSize( fControlWidth( ) / 2, buttonHeight ) );

		mAddButton->Connect( 
			wxEVT_COMMAND_BUTTON_CLICKED, 
			wxCommandEventHandler( tWxSlapOnAddRemoveListBox::fOnAddInternal ), 
			NULL, 
			this );
		mButtonSizer->Add( mAddButton, 1 );

		mRemoveButton = new wxButton(
			parent,
			wxID_ANY,
			removeLabel,
			wxDefaultPosition,
			wxSize( fControlWidth( ) / 2, buttonHeight ) );

		mRemoveButton->Connect( 
			wxEVT_COMMAND_BUTTON_CLICKED, 
			wxCommandEventHandler( tWxSlapOnAddRemoveListBox::fOnRemoveInternal ), 
			NULL, 
			this );
		mButtonSizer->Add( mRemoveButton, 1 );

		parent->GetSizer( )->Add( mButtonSizer, wxSizerFlags( ).Border( wxALL, fSizerPadding( ) ) );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnAddRemoveListBox::fOnRemove( )
	{
		fClearSelectedItems( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnAddRemoveListBox::fOnAddInternal( wxCommandEvent& )
	{
		fOnAdd( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnAddRemoveListBox::fOnRemoveInternal( wxCommandEvent& )
	{
		fOnRemove( );
	}
}
