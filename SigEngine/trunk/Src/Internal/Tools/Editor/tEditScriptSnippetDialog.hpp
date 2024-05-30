#ifndef __tEditScriptSnippetDialog__
#define __tEditScriptSnippetDialog__

#include "tWxTextEditor.hpp"

namespace Sig
{

	class tools_export tEditScriptSnippetDialog : public wxDialog
	{
	public:
		enum tDialogResult { cResultUnchanged, cResultChanged };

		tEditScriptSnippetDialog( wxWindow* parent, const wxString& title, const wxString& staticText, const wxString& text, u32 cursorPos = 0, b32 warnUnsaved = true );

		tDialogResult fShowDialog( u32 row = 0, u32 col = 0 );
		tWxTextEditor *fTextEditor( ) const { return mText; }

		std::string fGetScript( ) const;

		void fFocus( u32 row, u32 col );


	private:
		void fOnAccept( wxCommandEvent& event );
		void fOnCancel( wxCommandEvent& event );
		void fOnClose(wxCloseEvent& event);

		tWxTextEditor*			mText;
		wxString				mOriginalText;
		wxButton*				mAcceptButton;
		wxButton*				mCancelButton;
		tDialogResult			mReturnCode;
		b32 mWarnUnsaved;
	};

}

#endif//__tEditScriptSnippetDialog__
