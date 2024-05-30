#include "ToolsPch.hpp"
#include "tEditScriptSnippetDialog.hpp"

namespace Sig
{
	static const u32 cOutsideBorderSpacer	= 6;

	tEditScriptSnippetDialog::tEditScriptSnippetDialog( wxWindow* parent, const wxString& title, const wxString& staticText, const wxString& text, u32 cursorPos, b32 warnUnsaved )
		: wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxRESIZE_BORDER | wxTAB_TRAVERSAL | wxSTAY_ON_TOP )
		, mOriginalText( text )
		, mReturnCode( cResultUnchanged )
		, mWarnUnsaved( warnUnsaved )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		if( staticText.length( ) > 0 )
		{
			wxStaticText* sText = new wxStaticText( this, wxID_ANY, staticText );
			GetSizer( )->Add( sText, 0, wxALL | wxEXPAND, cOutsideBorderSpacer );
		}

		mText = new tWxTextEditor( this, wxID_ANY );
		mText->fConfigureForSquirrel( );
		mText->SetText( text );
		mText->SetSelection( int( cursorPos ), int( cursorPos ) );
		GetSizer( )->Add( mText, 1, wxALL | wxEXPAND, cOutsideBorderSpacer );

		wxBoxSizer* horizSizer = new wxBoxSizer( wxHORIZONTAL );
		GetSizer( )->Add( horizSizer, 0, wxEXPAND, 0 );

		mAcceptButton = new wxButton( this, wxID_ANY, "Accept" );
		horizSizer->Add( mAcceptButton, 0, wxRIGHT | wxBOTTOM, cOutsideBorderSpacer );

		mCancelButton = new wxButton( this, wxID_ANY, "Cancel" );
		horizSizer->Add( mCancelButton, 0, wxRIGHT | wxBOTTOM, cOutsideBorderSpacer );

		mAcceptButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditScriptSnippetDialog::fOnAccept ), NULL, this );
		mCancelButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditScriptSnippetDialog::fOnCancel ), NULL, this );
		Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( tEditScriptSnippetDialog::fOnClose ), NULL, this );
		
		SetSize( 800, 500 );
	}

	tEditScriptSnippetDialog::tDialogResult tEditScriptSnippetDialog::fShowDialog( u32 row, u32 col )
	{
		SetPosition( GetParent( )->GetScreenPosition( ) + (GetParent( )->GetSize( ) - GetSize( )) / 2 );
		Show( );
		fFocus( row, col );
		ShowModal( );
		return mReturnCode;
	}
	
	void tEditScriptSnippetDialog::fFocus( u32 row, u32 col )
	{
		mText->SetFocus( );
		const u32 position = mText->PositionFromLine( row ) + col;
		mText->GotoPos( position );
	}

	void tEditScriptSnippetDialog::fOnAccept( wxCommandEvent& event )
	{
		mReturnCode = cResultChanged;
		Close( true );
	}

	void tEditScriptSnippetDialog::fOnCancel( wxCommandEvent& event )
	{
		mReturnCode = cResultUnchanged;
		Close( );
	}

	void tEditScriptSnippetDialog::fOnClose(wxCloseEvent& event)
	{
		if ( mWarnUnsaved && event.CanVeto() && mText->GetText( ) != mOriginalText )
		{
			switch ( wxMessageBox("Would you like to save your changes?",
				"Script Snippet",
				wxICON_QUESTION | wxYES_NO | wxCANCEL ) )
			{
			case wxCANCEL:
				{
					event.Veto();
					return;
				}
			case wxYES:
				{
					mReturnCode = cResultChanged;
					Close( true );
				}
			default:
				break;
			}
		}

		Destroy();
		event.Skip( );
	}

	std::string tEditScriptSnippetDialog::fGetScript( ) const
	{
		std::string script = mText->GetText( );
		return StringUtil::fRemoveAllOf( script, "\r" );
	}

}