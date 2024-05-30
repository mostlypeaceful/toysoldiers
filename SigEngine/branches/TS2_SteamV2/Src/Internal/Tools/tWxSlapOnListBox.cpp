//------------------------------------------------------------------------------
// \file tWxSlapOnListBox.cpp - 17 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tWxSlapOnListBox.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tWxSlapOnListBox::tWxSlapOnListBox( wxWindow* parent, const char* label, b32 sorted, b32 uniqueItems )
		: tWxSlapOnControl( parent, label )
		, mUniqueItems( uniqueItems )
	{
		u32 style = wxLB_EXTENDED | wxLB_NEEDED_SB | wxLB_HSCROLL;
		if( sorted )
			style |= wxLB_SORT;

		mListBox = new wxListBox( 
			parent, 
			wxID_ANY, 
			wxDefaultPosition, 
			wxSize( fControlWidth( ), wxDefaultSize.y ),
			0, // strCount
			0, // strs
			style);

		fAddWindowToSizer( mListBox, false );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnListBox::fEnableControl( )
	{
		mListBox->Enable( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnListBox::fDisableControl( )
	{
		mListBox->Disable( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnListBox::fSetToolTip( const wxString& toolTip )
	{
		mListBox->SetToolTip( toolTip );
		tWxSlapOnControl::SetToolTip( toolTip );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnListBox::fAddItem( const char * item )
	{
		if( mUniqueItems && mListBox->FindString( item ) != wxNOT_FOUND )
			return;

		mListBox->AppendString( item );
		fOnControlUpdated( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnListBox::fAddItems( u32 itemCount, const char * items[] )
	{
		if( mUniqueItems )
		{
			for( u32 i = 0; i < itemCount; ++i )
			{
				if( mListBox->FindString( items[ i ] ) == wxNOT_FOUND )
					mListBox->Append( items[ i ] );
			}
		}
		else 
		{
			for( u32 i = 0; i < itemCount; ++i )
				mListBox->Append( items[ i ] );
		}

		fOnControlUpdated( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnListBox::fClearItems( )
	{
		mListBox->Clear( );
		fOnControlUpdated( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnListBox::fClearSelectedItems( )
	{
		wxArrayInt selections;
		while( mListBox->GetSelections( selections ) )
			mListBox->Delete( selections[0] );

		fOnControlUpdated( );
	}

	//------------------------------------------------------------------------------
	u32 tWxSlapOnListBox::ItemCount( ) const
	{
		return mListBox->GetCount( );
	}

	//------------------------------------------------------------------------------
	void tWxSlapOnListBox::Item( u32 index, wxString & itemOut ) const
	{
		itemOut = mListBox->GetString( index );
	}

	//------------------------------------------------------------------------------
	wxWindow * tWxSlapOnListBox::fGetParentWindow( )
	{
		return mListBox->GetParent( );
	}
}