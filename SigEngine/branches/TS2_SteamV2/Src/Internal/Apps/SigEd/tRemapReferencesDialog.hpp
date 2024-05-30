#ifndef __tRemapReferencesDialog__
#define __tRemapReferencesDialog__
#include "tEditorDialog.hpp"

namespace Sig
{
	class tEditorAppWindow;

	class tRemapReferencesDialog : public tEditorDialog
	{
		wxScrolledWindow*		mMainPanel;
		wxListBox*				mListBox;
		wxCheckBox*				mIncludeHiddens;

	public:
		tRemapReferencesDialog( tEditorAppWindow* editorWindow );
	    virtual bool Show( bool show = true );

	private:
		void fBuildGui( );
		tFilePathPtr fGetSelectedPath( ) const;
		void fOnSelectPressed( wxCommandEvent& event );
		void fOnRefreshPressed( wxCommandEvent& event );
		void fOnListBoxDClick( wxMouseEvent& event );
		void fRefreshList( );
		void fReplaceReference( const tFilePathPtr& oldRef, const tFilePathPtr& newRef );
	};

}

#endif//__tRemapReferencesDialog__
