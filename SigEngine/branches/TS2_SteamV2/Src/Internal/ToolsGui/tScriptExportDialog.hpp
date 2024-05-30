#ifndef __tScriptExportDialog__
#define __tScriptExportDialog__

#include "wx/treectrl.h"
#include "tScriptExportParser.hpp"
#include "tScriptContextStack.hpp"

namespace Sig { namespace ScriptData
{

	class toolsgui_export tWxTextBox : public wxTextCtrl
	{
	public:
		tWxTextBox( wxWindow* parent )
			: wxTextCtrl( parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_TAB )
		{ }

		//virtual void SetSelection( long from, long to )
		//{
		//	log_warning( 0, " insert : " << from );
		//}
	};

	class toolsgui_export tScriptDataDialog : public wxDialog
	{
	public:
		tScriptDataDialog( wxWindow* parent );

		wxTreeCtrl* fTree( ) { return mTree; }

		void fRebuild( wxCommandEvent& event );
		void fTreeSelect( wxCommandEvent& event );
		void fShowMember( const std::string& path );
		void fOnKeyDown( wxKeyEvent& event );
		void fOnKeyUp( wxKeyEvent& event );
		void fOnKeyChar( wxKeyEvent& event );
		void fOnTextChanged( wxCommandEvent& event );
		void fOnCursorMove( wxMouseEvent& event );	
		std::string fTextAfterDot( ) const;
		void fShow( const char* searchText = NULL );

	private:
		wxTreeCtrl* mTree;
		wxStaticText* mStatus;
		tWxTextBox* mTextBox;
		tScriptExportDataPtr mData;
		wxListBox* mList;
		wxListBox* mContextStack;

		tGrowableArray<wxTreeItemId> currentSearchResults;
		wxString currentSearchText;
		u32 currentSearchIndex;

		tScriptContextStack mStack;

		void fFillTree( );

		// context list
		tSymbol* mLastSelectedSymbol;
		tSymbol* fGetSelectedListSymbol( );
		void fIncrementListSelection( int val );
		void fReplaceTextAfterDot( const std::string& text );
		void fEvaluateControlChar( char character );

		// stack stuff
		void fPushContext( const tContextAction& action, u32 charPos, u32 delLength );
		void fFillList( );
		void fReset( );
	};

} }

#endif//__tScriptExportDialog__
