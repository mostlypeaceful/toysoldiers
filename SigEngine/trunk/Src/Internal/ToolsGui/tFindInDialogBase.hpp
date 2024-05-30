#ifndef __tFindInDialogBase__
#define __tFindInDialogBase__
//#include <wx/dialog.h>

class wxScrolledWindow;
class wxTextCtrl;
class wxCheckBox;
class wxAuiNotebook;

namespace Sig
{
	class tWxColumnListBox;
	class tConfigurableBrowserTree;
	class tWxTextEditor;


	/// 
	/// \brief
	/// Common dialog logic for searching, such as in files or snippets.
	class toolsgui_export tFindInDialogBase : public wxDialog
	{
	protected:
		struct toolsgui_export tOccurence
		{
			tFilePathPtr	mFile;
			u32				mLineNum;
			u32				mColNum;
			wxString		mDisplayLine;

			void fReplaceString( std::string& str, u32 replaceLen, const std::string& with ) const;
		};

		wxWindow*					mParent;
		b32							mFirstOpen;
		wxString					mFileTypeName; //appended to "Find in ..."
		wxComboBox*					mComboBox;
		wxBoxSizer*					mHorizontalSizer;

		// Term to search
		wxTextCtrl*					mSearchText;

		// Text to replace instances with.
		wxStaticText*				mReplaceTitle;
		wxTextCtrl*					mReplacementText;

		// Buttons.
		wxButton*			mFindNext; 
		wxButton*			mFindAll;
		wxButton*			mReplaceButton;
		wxButton*			mReplaceAllButton;
		wxButton*			mSelectButton;

		// Search options.
		wxCheckBox*			mMatchCase;
		wxCheckBox*			mMatchWholeWord;
		wxCheckBox*			mSearchUp;
		wxCheckBox*			mMatchMultiplePerLine;

		// Results
		tWxColumnListBox*				mResultsBox;
		tGrowableArray< tOccurence >	mOccurences;

		b32	mEnableSingleFinds;
		tGrowableArray< u32 > mFindOptionsId;
		tGrowableArray< u32 > mFindOptionsIndex;

	public:
		tFindInDialogBase( wxWindow* parent, const wxString& fileTypeName = "Files", b32 enableSingleFinds = true );

		void fOpenFind( );
		void fOpenFindInFiles( );
		void fOpenReplace( );
		void fOpenReplaceInFiles( );

		virtual void fFindNext( b32 searchUp = false ) { }
		virtual void fSetSelectedText( ) { }

	protected:
		void fBuildGui( );

		void fCommonOpen( );

		void fFindInFile( const wxString& findStr, const tDynamicBuffer& text, const tFilePathPtr& file, tGrowableArray< tOccurence >& occurences, b32 findOneOccurence = false );
		virtual void fFindInFiles( const wxString& searchText, tGrowableArray< tOccurence >& occurences, b32 findOneOccurencePerFile = false ) { }

		void fOnEnter( wxCommandEvent& event );
		virtual void fProcessEnter( wxCommandEvent& event );
		virtual void fOnSearchNextPressed( wxCommandEvent& event ) { }
		void fOnSearchAllPressed( wxCommandEvent& event );
		void fOnListBoxDClick( wxMouseEvent& event );
		void fOnSelectPressed( wxCommandEvent& event );
		void fOnReplacementTextEnter( wxCommandEvent& event );
		virtual void fOnReplacePressed( wxCommandEvent& event ) { }
		virtual void fOnReplaceAllPressed( wxCommandEvent& event ) { }
		void fOnComboBoxChange( wxCommandEvent& event );
		void fOnHotkey(wxCommandEvent& event);

		const char* fFindStr( const char* searchIn, const char* searchFor );
		virtual void fSelectItem( ) { }

	};
}

#endif//__tFindInDialogBase__
