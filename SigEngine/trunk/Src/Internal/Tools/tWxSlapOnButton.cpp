#include "ToolsPch.hpp"
#include "tWxSlapOnButton.hpp"
#include "tStrongPtr.hpp"

namespace Sig
{
	tWxSlapOnButton::tWxSlapOnButton( wxWindow* parent, const char* label, const char* spacerLabel )
		: tWxSlapOnControl( parent, spacerLabel ) // we use the label for the button text
	{
		const s32 buttonHeight = 36;

		mButton = new wxButton(
			parent,
			wxID_ANY,
			label,
			wxDefaultPosition,
			wxSize( fControlWidth( ), buttonHeight )/*,
			wxTAB_TRAVERSAL | wxWANTS_CHARS*/ );

		fAddWindowToSizer( mButton, true );

		mButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tWxSlapOnButton::fOnButtonPressedInternal ), NULL, this );
	}

	void tWxSlapOnButton::fEnableControl( )
	{
		mButton->Enable( true );
	}

	void tWxSlapOnButton::fDisableControl( )
	{
		mButton->Enable( false );
	}

	void tWxSlapOnButton::fOnButtonPressedInternal( wxCommandEvent& )
	{
		// notify derived type
		fOnControlUpdated( );
	}

}

