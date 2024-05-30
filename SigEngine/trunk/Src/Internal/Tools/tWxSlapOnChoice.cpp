#include "ToolsPch.hpp"
#include "tWxSlapOnChoice.hpp"

namespace Sig
{
	tWxSlapOnChoice::tWxSlapOnChoice( wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defChoice )
		: tWxSlapOnControl( parent, label )
		, mChoice( 0 )
	{
		mChoice = new wxChoice( 
			parent, 
			wxID_ANY, 
			wxDefaultPosition, 
			wxSize( fControlWidth( ), wxDefaultSize.y ), 
			numEnumNames, 
			enumNames/*,
			wxTAB_TRAVERSAL | wxWANTS_CHARS*/ );

		fAddWindowToSizer( mChoice, true );

		mChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( tWxSlapOnChoice::fOnControlUpdatedInternal ), NULL, this );

		if( defChoice < numEnumNames )
			mChoice->SetSelection( defChoice );
		else
			mChoice->SetSelection( 0 );
	}

	void tWxSlapOnChoice::fEnableControl( )
	{
		mChoice->Enable( );
	}
	
	void tWxSlapOnChoice::fDisableControl( )
	{
		mChoice->Disable( );
	}

	void tWxSlapOnChoice::fSetValue( u32 index )
	{
		mChoice->SetSelection( index );
	}

	u32 tWxSlapOnChoice::fGetValue( )
	{
		return mChoice->GetSelection( );
	}

	wxString tWxSlapOnChoice::fGetValueString( )
	{
		return mChoice->GetString( mChoice->GetSelection( ) );
	}

	void tWxSlapOnChoice::fSetToolTip( const wxString& toolTip )
	{
		mChoice->SetToolTip( toolTip );
		tWxSlapOnControl::SetToolTip( toolTip );
	}

	void tWxSlapOnChoice::fOnControlUpdatedInternal( wxCommandEvent& )
	{
		fOnControlUpdated( );
	}

}
