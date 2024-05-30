#include "ToolsGuiPch.hpp"
#include "tWxNotificationMessage.hpp"

namespace Sig
{
	static const u32 cItemSpacer = 6;
	static const u32 cTextSpacer = 20;

	tWxNotificationMessage::tWxNotificationMessage( wxWindow* parent, const std::string& title, const std::string& text )
		: wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE )
	{
		wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );
		wxStaticText* staticText = new wxStaticText( this, wxID_ANY, text.c_str( ), wxDefaultPosition, wxDefaultSize );
		mainSizer->Add( staticText, 0, wxALL, cTextSpacer );

		wxSizer* buttons = CreateStdDialogButtonSizer( wxOK );
		mainSizer->Add( buttons, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxTOP, cItemSpacer );
		mainSizer->AddSpacer( cItemSpacer );

		SetSizerAndFit( mainSizer );
	}
}
