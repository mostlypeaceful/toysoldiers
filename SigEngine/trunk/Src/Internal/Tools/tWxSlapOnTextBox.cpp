#include "ToolsPch.hpp"
#include "tWxSlapOnTextBox.hpp"

namespace Sig
{
	tWxSlapOnTextBox::tWxSlapOnTextBox( wxWindow* parent, const char* label, s32 widthOverride, b32 labelIsButton )
		: tWxSlapOnControl( parent, label, labelIsButton )
	{
		const u32 textCtrlWidth = ( widthOverride > 0 ) ? widthOverride : fControlWidth( );

		mTextCtrl = new wxTextCtrl( 
			parent, 
			wxID_ANY, 
			wxEmptyString, 
			wxDefaultPosition, 
			wxSize( textCtrlWidth, wxDefaultSize.y ), 
			wxTE_LEFT | wxTE_PROCESS_ENTER );

		fAddWindowToSizer( mTextCtrl, false );

		mTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxTextEventHandler( tWxSlapOnTextBox::fOnTextBoxEnter ), NULL, this );
		mTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( tWxSlapOnTextBox::fOnTextBoxLostFocus ), NULL, this );
	}

	void tWxSlapOnTextBox::fEnableControl( )
	{
		mTextCtrl->Enable( true );
	}

	void tWxSlapOnTextBox::fDisableControl( )
	{
		mTextCtrl->Enable( false );
	}

	b32 tWxSlapOnTextBox::fIsEmpty( ) const
	{
		return mTextCtrl->IsEmpty( );
	}

	std::string tWxSlapOnTextBox::fGetValue( ) const
	{
		return mTextCtrl->GetValue( ).c_str( );
	}

	void tWxSlapOnTextBox::fSetValue( const std::string& s )
	{
		mLastValue = s;
		mTextCtrl->SetValue( s.c_str( ) );
	}

	void tWxSlapOnTextBox::fSetToolTip( const wxString& toolTip )
	{
		mTextCtrl->SetToolTip( toolTip );
		tWxSlapOnControl::SetToolTip( toolTip );
	}

	void tWxSlapOnTextBox::fOnTextBoxEnter( wxCommandEvent& )
	{
		if( fGetValue( ) != mLastValue )
			fOnTextModified( );
	}

	void tWxSlapOnTextBox::fOnTextBoxLostFocus( wxFocusEvent& )
	{
		if( fGetValue( ) != mLastValue )
			fOnTextModified( );
	}

	void tWxSlapOnTextBox::fOnTextModified( )
	{
		mLastValue = fGetValue( );
		fOnControlUpdated( );
	}

}
