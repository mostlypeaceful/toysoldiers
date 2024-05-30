#ifndef __tSearchableOpenFilesDialog__
#define __tSearchableOpenFilesDialog__

namespace Sig
{
	class tWxDirectoryBrowser;
	class tWxColumnListBox;

	class toolsgui_export tSearchableOpenFilesDialog : public wxDialog
	{
		tWxDirectoryBrowser*	mSourceBrowser;
		tWxColumnListBox*		mResultsBox;
		wxTextCtrl*				mFilterText;
		wxButton*				mAcceptButton;
		tFilePathPtrList		mSelectedFiles;
		tFilePathPtrList		mWorkingFileSet;
		tGrowableArray<const char*> mAdditionalExtensionFilters;
		int						mSelectPos;
		int						mAnchorPos;
		b16						mAnchorInit;
		b16						mWeAreSelecting;

		struct tRowData
		{
			tRowData( ) { }

			tRowData( const wxString& filename, const wxString& fullPath )
				: mColumns( 2 )
			{
				mColumns[0] = filename;
				mColumns[1] = fullPath;
			}

			const wxString& fFileName( ) const { return mColumns[0]; }
			const wxString& fPath( ) const { return mColumns[1]; }

			tGrowableArray< wxString > mColumns;
		};

		tGrowableArray< tRowData > mRows;

	public:
		tSearchableOpenFilesDialog( wxWindow* parent, tWxDirectoryBrowser* browser, s64 listBoxStyleFlags = 0 );

		void fGetSelectedFiles( tFilePathPtrList& selectedFiles );
		tFilePathPtr fGetSelectedFile( );
		void fSetAdditionalExtensionFilters( const tGrowableArray<const char*> addedFilters ) { mAdditionalExtensionFilters = addedFilters; }

	private:
		static s32 wxCALLBACK fSortList( long item1, long item2, long WXUNUSED(sortData) );
		void fPrepData( );

		void fUpdateResults( wxCommandEvent& event );
		void fOnOpen( wxCommandEvent& event );
		void fOnChar( wxKeyEvent& event );
		void fOnItemSelect( wxListEvent& event );
		void fUpdateSelect( int dir, b32 shiftDown );
		void fClearAllSelected( );
		
	};
}

#endif//__tSearchableOpenFilesDialog__
