#include "ToolsPch.hpp"
#include "tWxSlapOnControl.hpp"

namespace Sig
{
	u32 tWxSlapOnControl::gLabelWidth	= 100;
	u32 tWxSlapOnControl::gControlWidth	= 200;
	u32 tWxSlapOnControl::gSizerPadding	= 4;

	tWxSlapOnGridSizer::tWxSlapOnGridSizer( wxWindow* parent, u32 columns, u32 leftPadding )
		: tWxAutoDelete( parent )
		, mGrid( new wxGridSizer( columns ) )
	{
		if( !parent->GetSizer( ) )
			parent->SetSizer( new wxGridSizer( wxVERTICAL ) );

		wxBoxSizer* padding = new wxBoxSizer( wxHORIZONTAL );
		padding->AddSpacer( leftPadding );
		padding->Add( mGrid );

		parent->GetSizer( )->Add( padding );
	}

	tWxSlapOnGridSizer::~tWxSlapOnGridSizer( )
	{
		// who needs cleanup?
		// jk but it's not clear when to clean up wx widgets so feel free if you know.
	}



	tWxSlapOnControl::tWxSlapOnControl( wxWindow* parent, const char* label, b32 labelIsButton, tWxSlapOnGridSizer* parentSizer )
		: tWxAutoDelete( parent )
		, mLabel( NULL )
		, mSizer( NULL )
	{		
		// the vertical sizer is necessary to isolate the group button at the top
		// child to it will be a grid sizer with 1 or more columns
		if( !parentSizer && !parent->GetSizer( ) )
			parent->SetSizer( new wxBoxSizer( wxVERTICAL ) );

		// this is the smallest sizer, containing the label, and a control such as a textbox + browse button or a numeric up down.
		mSizer = new wxBoxSizer( wxHORIZONTAL );

		// Add it to the parent
		if( parentSizer )
			parentSizer->fGetSizer( )->Add( mSizer, wxSizerFlags( ).Expand( ) );
		else
			parent->GetSizer( )->Add( mSizer, wxSizerFlags( ).Expand( ) );

		if( label && strlen( label ) > 0 )
		{
			if( !labelIsButton )
			{
				mLabel = new wxStaticText( 
					parent, 
					wxID_ANY, 
					label, 
					wxDefaultPosition, 
					wxSize( gLabelWidth, wxDefaultSize.y ),
					wxALIGN_RIGHT | wxST_NO_AUTORESIZE  );
			}
			else
			{
				mLabel = new wxButton(
					parent,
					wxID_ANY,
					label,
					wxDefaultPosition,
					wxSize( gLabelWidth, wxDefaultSize.y ),
					wxALIGN_CENTER_VERTICAL | wxST_NO_AUTORESIZE );

				mLabel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tWxSlapOnControl::fOnLabelButtonClicked), NULL, this );
			}

			mSizer->Add( mLabel, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxLEFT | wxRIGHT, gSizerPadding );
		}
	}

	tWxSlapOnControl::~tWxSlapOnControl( )
	{
	}

	wxString tWxSlapOnControl::fGetLabelText( )
	{
		if( !mLabel )
			return "";
		else
			return mLabel->GetLabelText( );
	}

	void tWxSlapOnControl::fAddWindowToSizer( wxWindow* window, b32 rightMost )
	{
		mSizer->Add( window, 0, wxALIGN_BOTTOM | wxTOP | ( rightMost ? /*wxRIGHT*/0 : 0 ), gSizerPadding );
	}

	void tWxSlapOnControl::SetToolTip( const wxString& toolTip )
	{
		if( mLabel )
			mLabel->SetToolTip( toolTip );
	}

}
