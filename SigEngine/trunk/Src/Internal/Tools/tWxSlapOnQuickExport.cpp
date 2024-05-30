#include "ToolsPch.hpp"
#include "tWxSlapOnQuickExport.hpp"
#include "WxUtil.hpp"

namespace Sig
{
	const char tWxSlapOnQuickExport::cPathNotSet[]="[path not set]";

	tWxSlapOnQuickExport::tWxSlapOnQuickExport( wxWindow* parent, const char* label )
		: tWxSlapOnControl( parent, "" ) // we use the label for the button text
	{
		const s32 buttonHeight = 36;

		mExport = new wxButton(
			parent,
			wxID_ANY,
			label,
			wxDefaultPosition,
			wxSize( fControlWidth( ), buttonHeight )/*,
			wxTAB_TRAVERSAL | wxWANTS_CHARS*/ );

		mPath = new wxStaticText(
			parent,
			wxID_ANY,
			cPathNotSet,
			wxDefaultPosition,
			wxDefaultSize );

		mDoQuick = new wxCheckBox( 
			parent, 
			wxID_ANY, 
			"Quick Export", 
			wxDefaultPosition, 
			wxSize( wxDefaultSize.x, wxDefaultSize.y ), 
			wxALIGN_LEFT | /*wxTAB_TRAVERSAL | wxWANTS_CHARS | */wxBORDER_RAISED );

		mDoSelected = new wxCheckBox( 
			parent, 
			wxID_ANY, 
			"Export Selected", 
			wxDefaultPosition, 
			wxSize( wxDefaultSize.x, wxDefaultSize.y ), 
			wxALIGN_LEFT | /*wxTAB_TRAVERSAL | wxWANTS_CHARS | */wxBORDER_RAISED );

		wxBoxSizer* leftSizer = new wxBoxSizer( wxVERTICAL );
		wxBoxSizer* rightSizer = new wxBoxSizer( wxVERTICAL );
		wxBoxSizer* rightHorzSizer = new wxBoxSizer( wxHORIZONTAL );

		mSizer->Add( leftSizer, wxSizerFlags( ).Expand( ) );
		mSizer->Add( rightSizer, wxSizerFlags( ).Expand( ) );

		leftSizer->Add( mExport, 0, wxALIGN_LEFT | wxTOP | wxBOTTOM, fSizerPadding( ) );
		rightSizer->Add( mPath, 0, wxALIGN_LEFT | wxLEFT | wxTOP, fSizerPadding( ) );

		rightHorzSizer->Add( mDoQuick, 0, wxALIGN_LEFT | wxALIGN_BOTTOM | wxLEFT | wxTOP, fSizerPadding( ) );
		rightHorzSizer->Add( mDoSelected, 0, wxALIGN_LEFT | wxALIGN_BOTTOM | wxLEFT | wxTOP, fSizerPadding( ) );
		rightSizer->Add( rightHorzSizer, wxSizerFlags( ).Expand( ) );

		mExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tWxSlapOnQuickExport::fOnButtonPressedInternal ), NULL, this );
	}

	void tWxSlapOnQuickExport::fEnableControl( )
	{
		mExport->Enable( true );
		mDoQuick->Enable( true );
		mDoSelected->Enable( true );
	}

	void tWxSlapOnQuickExport::fDisableControl( )
	{
		mExport->Enable( false );
		mDoQuick->Enable( false );
		mDoSelected->Enable( false );
	}

	wxString tWxSlapOnQuickExport::fGetValue( )
	{
		return ToolsPaths::fMakeResAbsolute( tFilePathPtr( mPath->GetLabelText( ).c_str( ) ) ).fCStr( );
	}

	void tWxSlapOnQuickExport::fSetValue( const char* path )
	{
		const tFilePathPtr resRelative = ToolsPaths::fMakeResRelative( tFilePathPtr( path ) );
		mPath->SetLabel( resRelative.fCStr( ) );
	}

	wxString tWxSlapOnQuickExport::fGetFileWildcard( ) const
	{
		return "*.*"; // i.e., "*.raw;*.txt"
	}

	void tWxSlapOnQuickExport::fOnButtonPressedInternal( wxCommandEvent& )
	{
		if( mPath->GetLabelText( ) == cPathNotSet || !mDoQuick->GetValue( ) )
		{
			wxString wildCard = fGetFileWildcard( );

			std::string openFilePath;
			if( WxUtil::fBrowseForFile( openFilePath, mExport->GetParent( ), "Export", 0, 0, wildCard.c_str( ), wxFD_SAVE ) )
				fSetValue( openFilePath.c_str( ) );
			else
				return;

			// notify derived type
			fOnControlUpdated( );
		}

		sigassert( mPath->GetLabelText( ) != cPathNotSet );

		fOnButtonPressed( );
	}

}

