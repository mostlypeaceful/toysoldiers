#include "ToolsPch.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "wx/tglbtn.h"

namespace Sig
{

	tWxSlapOnCheckBox::tWxSlapOnCheckBox( wxWindow* parent, const char* label, b32 asButton, tWxSlapOnGridSizer* parentSizer )
		: tWxSlapOnControl( parent, asButton ? "" : label, false, parentSizer )
		, mCheckBox( 0 )
		, mButton( 0 )
	{
		const u32 height = 20;
		const u32 buttonWidth = 35;

		if( asButton )
		{
			mButton = new wxToggleButton( 
				parent, 
				wxID_ANY, 
				label, 
				wxDefaultPosition, 
				wxSize( buttonWidth, height ), 
				wxCHK_3STATE | wxALIGN_LEFT | /*wxTAB_TRAVERSAL | wxWANTS_CHARS | */wxBORDER_RAISED );

			fAddWindowToSizer( mButton, true );

			mButton->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( tWxSlapOnCheckBox::fOnControlUpdatedInternal ), NULL, this );
		}
		else
		{
			mCheckBox = new wxCheckBox( 
				parent, 
				wxID_ANY, 
				"", 
				wxDefaultPosition, 
				wxSize( wxDefaultSize.x, height ), 
				wxCHK_3STATE | wxALIGN_LEFT | /*wxTAB_TRAVERSAL | wxWANTS_CHARS | */wxBORDER_RAISED );

			fAddWindowToSizer( mCheckBox, true );

			mCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( tWxSlapOnCheckBox::fOnControlUpdatedInternal ), NULL, this );

		}

		fSetValue( cFalse );
	}

	void tWxSlapOnCheckBox::fEnableControl( )
	{
		if( mCheckBox )
			mCheckBox->Enable( );
		else
			mButton->Enable( );
	}
	
	void tWxSlapOnCheckBox::fDisableControl( )
	{
		if( mCheckBox )
			mCheckBox->Disable( );
		else
			mButton->Disable( );
	}

	void tWxSlapOnCheckBox::fSetToolTip( const wxString& toolTip )
	{
		if( mCheckBox )
			mCheckBox->SetToolTip( toolTip );
		else
			mButton->SetToolTip( toolTip );

		tWxSlapOnControl::SetToolTip( toolTip );
	}

	void tWxSlapOnCheckBox::fOnControlUpdatedInternal( wxCommandEvent& )
	{
		fOnControlUpdated( );
	}

	void tWxSlapOnCheckBox::fSetValue( s32 val )
	{
		if( val < 0 )
		{
			if( mCheckBox )
				mCheckBox->Set3StateValue( wxCHK_UNDETERMINED );
			//else
			// TODO 3RD BUTTON STATE
		}
		else
		{
			if( mCheckBox )
				mCheckBox->SetValue( val != 0 );
			else
				mButton->SetValue( val != 0 );
		}
	}

	s32 tWxSlapOnCheckBox::fGetValue( )
	{
		if( mCheckBox )
		{
			switch( mCheckBox->Get3StateValue( ) )
			{
			case wxCHK_UNCHECKED:	return cFalse;
			case wxCHK_CHECKED:		return cTrue;
			}

			return cGray;
		}
		else
		{
			if( mButton->GetValue( ) )
				return cTrue;
			else
				return cFalse;
			
			// TODO 3RD BUTTON STATE
		}
	}

	tWxSlapOnCheckBoxDataSync::tWxSlapOnCheckBoxDataSync( wxWindow* parent, const char* label, s32& data )
		: tWxSlapOnCheckBox( parent, label )
		, mData( data )
	{
		fSetValue( data );
	}

	void tWxSlapOnCheckBoxDataSync::fSetValue( s32 val )
	{
		mData = val;
		tWxSlapOnCheckBox::fSetValue( val );
	}

	void tWxSlapOnCheckBoxDataSync::fOnControlUpdated( )
	{
		mData = fGetValue( );
	}

}
