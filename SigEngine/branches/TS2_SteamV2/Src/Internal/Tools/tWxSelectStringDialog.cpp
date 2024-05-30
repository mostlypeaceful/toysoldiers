#include "ToolsPch.hpp"
#include "tWxSelectStringDialog.hpp"

namespace Sig
{
	tWxSelectStringDialog::tWxSelectStringDialog( wxWindow* parent, const char* title, const tGrowableArray<std::string>& names )
		: wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize )
		, mStringListBox( 0 )
	{
		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		mStringListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE | wxLB_SORT );
		mStringListBox->SetMinSize( wxSize( 200, wxDefaultSize.y ) );
		mStringListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( tWxSelectStringDialog::fOnDoubleClick ), NULL, this );
		GetSizer( )->Add( mStringListBox, 0, wxEXPAND | wxALL, 5 );

		for( u32 i = 0; i < names.fCount( ); ++i )
		{
			mStringListBox->Append( names[ i ] );
		}

		GetSizer( )->Add( CreateStdDialogButtonSizer( wxOK | wxCANCEL ), 1, wxEXPAND | wxALL, 5 );

		Fit( );
		Layout( );
	}

	std::string tWxSelectStringDialog::fGetResultString( ) const
	{
		const s32 sel = mStringListBox->GetSelection( );
		if( sel >= 0 && sel < ( s32 )mStringListBox->GetCount( ) )
			return std::string( mStringListBox->GetString( sel ).c_str( ) );
		return "";
	}

	void tWxSelectStringDialog::fOnDoubleClick( wxCommandEvent& )
	{
		EndModal( wxID_OK );
	}
}

