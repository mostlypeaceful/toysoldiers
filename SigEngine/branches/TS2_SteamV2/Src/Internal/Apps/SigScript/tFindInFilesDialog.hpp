#ifndef __tFindInfilesDialog__
#define __tFindInfilesDialog__
#include "tFindInDialogBase.hpp"

namespace Sig
{
	class tSigScriptWindow;
	class tWxColumnListBox;
	class tConfigurableBrowserTree;
	class tScriptNotebook;
	class tWxTextEditor;

	/// 
	/// \brief
	/// Used to search through all files currently being viewed by the
	/// script browser.
	class tFindInFilesDialog : public tFindInDialogBase
	{
		// SigScript specific data.
		tScriptNotebook*			mMainNotebook;
		tConfigurableBrowserTree*	mBrowser;

	public:
		tFindInFilesDialog( 
			tSigScriptWindow* editorWindow,
			tConfigurableBrowserTree* mBrowser, 
			tScriptNotebook* notebook );

		virtual void fFindNext( b32 searchUp = false );
		virtual void fSetSelectedText( );

	private:
		virtual void fFindInFiles( const wxString& searchText, tGrowableArray< tOccurence >& occurences, b32 findOneOccurencePerFile = false );

		virtual void fOnSearchNextPressed( wxCommandEvent& event );
		virtual void fOnReplacePressed( wxCommandEvent& event );
		virtual void fOnReplaceAllPressed( wxCommandEvent& event );

		virtual void fSelectItem( );

		/// 
		/// \brief
		/// Returns -1 if nothing is found. Otherwise, returns the pos of the found item.
		s32 fSearchNext( const char* searchFor, tWxTextEditor* editor, b32 searchUp, b32 wrap = true );
	};
}

#endif//__tFindInfilesDialog__
