#include "ToolsPch.hpp"
#include "tWxNumericText.hpp"

namespace Sig
{

	tWxNumericText::tWxNumericText( wxWindow* parent, f32 min, f32 max, u32 precision, const wxPoint& pos, const wxSize& size )
		: wxTextCtrl( parent, wxID_ANY, wxEmptyString, pos, size, wxTE_LEFT | wxTE_PROCESS_ENTER, wxTextValidator( wxFILTER_NUMERIC ) )
		, mMin( fMin( min, max ) )
		, mMax( fMax( min, max ) )
		, mPrecision( precision )
	{
		Connect( wxEVT_COMMAND_TEXT_ENTER, wxTextEventHandler( tWxNumericText::fOnTextBoxEnter ), NULL, this );
		Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( tWxNumericText::fOnTextBoxLostFocus ), NULL, this );
	}

	void tWxNumericText::fSetValue( f32 value, b32 notify)
	{
		std::stringstream ss;
		ss.precision( mPrecision );
		ss << std::fixed << fClamp( value, mMin, mMax );
		ss >> value;
		value = fClamp( value, mMin, mMax );
		ChangeValue( ss.str( ) );
		if( notify )
			fOnValueUpdated( );
	}

	f32 tWxNumericText::fGetValue( ) const
	{
		wxString textBoxValue = GetValue( );

		std::stringstream ss;
		ss.precision( mPrecision );
		ss << std::fixed << textBoxValue;

		f32 value = 0.f;
		ss >> value;
		value = fClamp( value, mMin, mMax );

		return value;
	}

	void tWxNumericText::fSetIndeterminate( )
	{
		SetValue( "" );
	}

	b32 tWxNumericText::fIsIndeterminate( ) const
	{
		wxString textBoxValue = GetValue( );
		return textBoxValue == "";
	}

	void tWxNumericText::fOnTextBoxEnter( wxCommandEvent& )
	{
		fOnTextModified( );
	}

	void tWxNumericText::fOnTextBoxLostFocus( wxFocusEvent& )
	{
		fOnLostFocus( );
	}

	void tWxNumericText::fOnTextModified( )
	{
		const f32 value = fGetValue( );
		std::stringstream ss;
		ss << value;
		ChangeValue( ss.str( ) );
		fOnValueUpdated( );
	}

}

