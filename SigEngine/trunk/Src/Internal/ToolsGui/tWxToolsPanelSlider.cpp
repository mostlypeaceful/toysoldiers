#include "ToolsGuiPch.hpp"
#include "tWxToolsPanelSlider.hpp"

namespace Sig
{
	tWxToolsPanelSlider::tWxToolsPanelSlider( wxWindow* parent, const char* labelName, wxWindow* editorWindow, f32 initialValue, b32 showValueText )
		: wxSlider( parent, wxID_ANY, fRound<u32>( initialValue * 100 ), 0, 100 )
		, mEditorWindow( editorWindow )
		, mDisplayValueText( 0 )
	{
		wxBoxSizer* sizer = new wxBoxSizer( wxHORIZONTAL );
		wxStaticText* labelText = new wxStaticText( parent, wxID_ANY, labelName, wxDefaultPosition, 
			wxSize( tWxSlapOnControl::fLabelWidth( ), wxDefaultSize.y ), wxALIGN_RIGHT );
		sizer->Add( labelText, 0, wxTOP, 5 );
		sizer->Add( this, 1, wxEXPAND | wxALIGN_TOP, 1 );

		if( showValueText )	// display a text printout of the current value. Programmer is responsible for updating this via fSetValueTextDisplay( value ).
		{
			mDisplayValueText = new wxStaticText( parent, wxID_ANY, wxString::Format( "%.1f", initialValue ) );
			sizer->Add( mDisplayValueText, 0, wxTOP | wxRIGHT, 6 );
		}

		if( !parent->GetSizer( ) )
			parent->SetSizer( new wxBoxSizer( wxVERTICAL ) );
		parent->GetSizer( )->Add( sizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 1 );

		Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( tWxToolsPanelSlider::fScrollChanged ), NULL, this );
		Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( tWxToolsPanelSlider::fOnScrolling ), NULL, this );
	}

	void tWxToolsPanelSlider::fOnScrolling( wxScrollEvent& event )
	{
		mEditorWindow->SetFocus( );
		fOnScrollingChanged( );
	}
	void tWxToolsPanelSlider::fScrollChanged( wxScrollEvent& event )
	{
		mEditorWindow->SetFocus( );
		fOnValueChanged( );
	}

	void tWxToolsPanelSlider::fSetDisplayValueText( f32 value )
	{
		if( mDisplayValueText )
		{
			if( value > 10.f )
				mDisplayValueText->SetLabel( wxString::Format( "%.0f", value ) );
			else
				mDisplayValueText->SetLabel( wxString::Format( "%.1f", value ) );
		}
	}

}

