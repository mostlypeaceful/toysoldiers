#include "ToolsPch.hpp"
#include "tWxSlapOnComboBox.hpp"

namespace Sig
{
	tWxSlapOnComboBox::tWxSlapOnComboBox( wxWindow* parent, const char* label, wxArrayString* choices, s64 styleFlags )
		: tWxSlapOnControl( parent, label )
		, mComboBox( 0 )
	{
		if( choices )
		{
			mComboBox = new wxComboBox(
				parent,
				wxID_ANY,
				wxEmptyString,
				wxDefaultPosition,
				wxDefaultSize,
				*choices,
				styleFlags );
		}
		else
		{
			mComboBox = new wxComboBox(
				parent,
				wxID_ANY,
				wxEmptyString,
				wxDefaultPosition,
				wxDefaultSize,
				0,
				0,
				styleFlags );
		}

		fAddWindowToSizer( mComboBox, true );

		mComboBox->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( tWxSlapOnComboBox::fOnControlUpdatedInternal ), NULL, this );
	}

	void tWxSlapOnComboBox::fEnableControl( )
	{
		mComboBox->Enable( );
	}
	
	void tWxSlapOnComboBox::fDisableControl( )
	{
		mComboBox->Disable( );
	}

	void tWxSlapOnComboBox::fSetSelection( u32 val )
	{
		mComboBox->SetSelection( val );
	}

	u32 tWxSlapOnComboBox::fGetSelection( )
	{
		return mComboBox->GetSelection( );
	}

	std::string tWxSlapOnComboBox::fGetString( )
	{
		return mComboBox->GetString( fGetSelection( ) ).c_str( );
	}

	void tWxSlapOnComboBox::fClearChoices( )
	{
		mComboBox->Clear( );
	}

	void tWxSlapOnComboBox::fAddString( const wxString& addedString )
	{
		mComboBox->Append( addedString );
	}

	void tWxSlapOnComboBox::fOnControlUpdatedInternal( wxCommandEvent& )
	{
		fOnControlUpdated( );
	}

}
