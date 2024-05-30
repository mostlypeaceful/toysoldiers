#include "ToolsPch.hpp"
#include "tWxSlapOnSpinner.hpp"

namespace Sig
{


	tWxSlapOnSpinner::tWxSlapOnSpinner( wxWindow* parent, const char* label, f32 min, f32 max, f32 increment, u32 precision, s32 widthOverride )
		: tWxSlapOnControl( parent, label )
		, mLastValue( 0.f )
		, mTextCtrl( 0 )
		, mSpinner( 0 )
		, mMin( fMin( min, max ) )
		, mMax( fMax( min, max ) )
		, mIncrement( fMax( increment, 0.000001f ) )
		, mPrecision( precision )
	{
		const u32 spinnerWidth = 20;
		const u32 textCtrlWidth = ( ( widthOverride > 0 ) ? widthOverride : fControlWidth( ) ) - spinnerWidth;

		mTextCtrl = new wxTextCtrl( 
			parent, 
			wxID_ANY, 
			wxEmptyString, 
			wxDefaultPosition, 
			wxSize( textCtrlWidth, wxDefaultSize.y ), 
			wxTE_LEFT | wxTE_PROCESS_ENTER,
			wxTextValidator( wxFILTER_NUMERIC ) );

		mSpinner = new wxSpinButton( 
			parent, 
			wxID_ANY, 
			wxDefaultPosition, 
			wxSize( spinnerWidth, mTextCtrl->GetSize( ).y ), 
			wxSP_VERTICAL | wxSP_ARROW_KEYS );

		const s32 spinMin = fRound<s32>( min / increment );
		const s32 spinMax = fRound<s32>( max / increment );
		mSpinner->SetRange( spinMin, spinMax );

		fAddWindowToSizer( mTextCtrl, false );
		fAddWindowToSizer( mSpinner, true );

		mSpinner->Connect( wxEVT_SCROLL_THUMBTRACK, wxSpinEventHandler( tWxSlapOnSpinner::fOnSpin ), NULL, this );
		mTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxTextEventHandler( tWxSlapOnSpinner::fOnTextBoxEnter ), NULL, this );
		mTextCtrl->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( tWxSlapOnSpinner::fOnTextBoxLostFocus ), NULL, this );

		fSetValueNoEvent( 0.f );
	}

	void tWxSlapOnSpinner::fEnableControl( )
	{
		mTextCtrl->Enable( );
		mSpinner->Enable( );
	}
	
	void tWxSlapOnSpinner::fDisableControl( )
	{
		mTextCtrl->Disable( );
		mSpinner->Disable( );
	}

	b32 tWxSlapOnSpinner::fHasFocus( )
	{
		return
			( GetFocus( ) == ( HWND )mSpinner->GetHWND( ) ) ||
			( GetFocus( ) == ( HWND )mTextCtrl->GetHWND( ) );
	}

	f32 tWxSlapOnSpinner::fGetValue( )
	{
		return mSpinner->GetValue( ) * mIncrement;
	}

	void tWxSlapOnSpinner::fSetValue( f32 value )
	{
		mSpinner->SetValue( fRound<s32>( value / mIncrement ) );

		// fire off a "spinned" event
		wxSpinEvent e;
		fOnSpin( e );
	}

	void tWxSlapOnSpinner::fSetToolTip( const wxString& toolTip )
	{
		mSpinner->SetToolTip( toolTip );
		mTextCtrl->SetToolTip( toolTip );
		tWxSlapOnControl::SetToolTip( toolTip );
	}

	void tWxSlapOnSpinner::fSetValueNoEvent( f32 value )
	{
		value = fClamp( value, mMin, mMax );
		mLastValue = value;

		mSpinner->SetValue( fRound<s32>( value / mIncrement ) );

		std::stringstream ss;
		ss.precision( mPrecision );
		ss << std::fixed << value;

		mTextCtrl->ChangeValue( ss.str( ) );
	}

	void tWxSlapOnSpinner::fSetIndeterminateNoEvent( )
	{
		mLastValue = 0.f;
		mSpinner->SetValue( 0.f );
		mTextCtrl->ChangeValue( wxString("") );
	}

	void tWxSlapOnSpinner::fOnSpin( wxSpinEvent& event )
	{
		const f32 value = fGetValue( );

		fSetValueNoEvent( value );

		fOnControlUpdated( );
	}

	void tWxSlapOnSpinner::fOnTextBoxEnter( wxCommandEvent& )
	{
		fOnTextModified( );
	}

	void tWxSlapOnSpinner::fOnTextBoxLostFocus( wxFocusEvent& )
	{
		fOnTextModified( );
	}

	void tWxSlapOnSpinner::fOnTextModified( )
	{
		wxString textBoxValue = mTextCtrl->GetValue( );

		std::stringstream ss;
		ss.precision( mPrecision );
		ss << std::fixed << textBoxValue;

		f32 value = fGetValue( );
		ss >> value;

		value = fClamp( value, mMin, mMax );

		if( value != mLastValue )
		{
			fSetValueNoEvent( value );
			mLastValue = fGetValue( );
			fOnControlUpdated( );
		}
	}

	void tWxSlapOnSpinner::fOnControlUpdated( )
	{ 
		wxSpinEvent e( wxEVT_SCROLL_THUMBTRACK, mSpinner->GetId( ) );
		e.SetEventObject( this );
		e.SetClientData( GetClientData( ) );
		ProcessEvent( e );
	}
}
