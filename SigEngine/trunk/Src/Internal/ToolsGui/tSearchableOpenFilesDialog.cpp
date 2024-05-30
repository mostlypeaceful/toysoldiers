#include "ToolsGuiPch.hpp"
#include "tSearchableOpenFilesDialog.hpp"
#include "tWxColumnListBox.hpp"
#include "tWxDirectoryBrowser.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	static const u32 cItemSpacer			= 2;
	static const u32 cOutsideBorderSpacer	= 6;

	tSearchableOpenFilesDialog::tSearchableOpenFilesDialog( wxWindow* parent, tWxDirectoryBrowser* browser, s64 listBoxStyleFlags )
		: wxDialog( parent, wxID_ANY, wxString( "Open File" ), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxTAB_TRAVERSAL )
		, mSourceBrowser( browser )
	{
		fCommonCtor( listBoxStyleFlags );
	}

	tSearchableOpenFilesDialog::tSearchableOpenFilesDialog( wxWindow* parent, const tFilePathPtr& resFolder, s64 listBoxStyleFlags )
		: wxDialog( parent, wxID_ANY, wxString( "Open File" ), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxTAB_TRAVERSAL )
		, mSourceBrowser( NULL )
		, mResFolder( resFolder )
	{
		fCommonCtor( listBoxStyleFlags );

		FileSystem::fGetFileNamesInFolder( 
			mWorkingFileSet,
			mResFolder,
			true,
			true );
	}

	void tSearchableOpenFilesDialog::fGetSelectedFiles( tFilePathPtrList& selectedFiles )
	{
		mFilterText->Clear( );
		mFilterText->SetFocus( );

		// Build cache of row data for filtering.
		fPrepData( );

		// Pop the display.
		ShowModal( );

		// When it closes, copy out the selected files and clear the selection.
		selectedFiles = mSelectedFiles;
		mSelectedFiles.fDeleteArray( );
	}

	tFilePathPtr tSearchableOpenFilesDialog::fGetSelectedFile( )
	{
		tFilePathPtrList files;
		fGetSelectedFiles( files );

		if( files.fCount( ) == 0 )
			return tFilePathPtr( );

		return files[0];
	}

	// first less than second = -1
	// first equal to second = 0
	// first greater than second = 1
	s32 wxCALLBACK tSearchableOpenFilesDialog::fSortList( long itemData1, long itemData2, long sortData )
	{
		tSearchableOpenFilesDialog* This = (tSearchableOpenFilesDialog*)sortData;

		const wxString& first = This->mRows[ itemData1 ].fFileName( );
		const wxString& second = This->mRows[ itemData2 ].fFileName( );
		
		return strcmp( first.c_str( ), second.c_str( ) );
	}

	void tSearchableOpenFilesDialog::fPrepData( )
	{
		tFilePathPtrList files;
		if( mSourceBrowser )
		{
			if( mAdditionalExtensionFilters.fCount( ) == 0 )
				files = mSourceBrowser->fGetFilteredPaths( );
			else
				files = mSourceBrowser->fGetFilteredPathsOfType( mAdditionalExtensionFilters );
		}
		else
		{
			files = tWxDirectoryBrowser::fGetPathsOfType( mWorkingFileSet, mAdditionalExtensionFilters );
		}

		mRows.fDeleteArray( );
		mRows.fSetCount( files.fCount( ) );
		mResultsBox->DeleteAllItems( );

		for( u32 i = 0; i < files.fCount( ); ++i )
		{
			mRows[i] = tRowData( wxString( StringUtil::fNameFromPath( files[i].fCStr( ) ) ), wxString( files[i].fCStr( ) ) );
			const u64 createdIdx = mResultsBox->fCreateRow( mRows[i].mColumns );
			mResultsBox->SetItemData( createdIdx, createdIdx );
		}

		mResultsBox->SortItems( fSortList, (long)this );
		mResultsBox->fAutosizeAllColumns( );

		mResultsBox->Select( 0 );
	}

	void tSearchableOpenFilesDialog::fCommonCtor( s64 listBoxStyleFlags )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		mResultsBox = new tWxColumnListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | listBoxStyleFlags );
		GetSizer( )->Add( mResultsBox, 1, wxALL | wxEXPAND, cOutsideBorderSpacer );

		mResultsBox->fCreateColumn( "File" );
		mResultsBox->fCreateColumn( "Path" );

		wxBoxSizer* horizSizer = new wxBoxSizer( wxHORIZONTAL );
		GetSizer( )->Add( horizSizer, 0, wxEXPAND, 0 );

		mFilterText = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxPROCESS_ENTER );
		horizSizer->Add( mFilterText, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, cOutsideBorderSpacer );

		mAcceptButton = new wxButton( this, wxID_ANY, "Open" );
		horizSizer->Add( mAcceptButton, 0, wxRIGHT | wxBOTTOM, cOutsideBorderSpacer );

		// Add button that will handle Esc and also closing the dialog too.
		wxSizer* buttonsSizer = CreateButtonSizer( wxCANCEL );
		GetSizer( )->Add( buttonsSizer, 0, wxALIGN_RIGHT | wxRIGHT, 2 );
		GetSizer( )->AddSpacer( cOutsideBorderSpacer );

		mFilterText->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( tSearchableOpenFilesDialog::fUpdateResults ), NULL, this );
		mFilterText->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( tSearchableOpenFilesDialog::fOnOpen ), NULL, this );
		mFilterText->Connect( wxEVT_CHAR, wxKeyEventHandler( tSearchableOpenFilesDialog::fOnChar ), NULL, this );
		mAcceptButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tSearchableOpenFilesDialog::fOnOpen ), NULL, this );
		mResultsBox->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxCommandEventHandler( tSearchableOpenFilesDialog::fOnOpen ), NULL, this );
		mResultsBox->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( tSearchableOpenFilesDialog::fOnItemSelect ), NULL, this );

		SetSize( 800, 500 );
	}

	void tSearchableOpenFilesDialog::fUpdateResults( wxCommandEvent& event )
	{
		tGrowableArray< std::string > includes;
		tGrowableArray< std::string > excludes;
		StringUtil::fSplitSearchParams( includes, excludes, mFilterText->GetLineText( 0 ).c_str() );

		mResultsBox->DeleteAllItems( );

		for( u32 i = 0; i < mRows.fCount( ); ++i )
		{
			// Filter for all inclusions.
			if( !StringUtil::fAndSearch( mRows[i].fPath().c_str(), includes ) )
				continue;

			// Filter for all exclusions.
			if( !StringUtil::fNotSearch( mRows[i].fPath().c_str(), excludes ) )
				continue;

			// Create entry and save row.
			const u64 createdIdx = mResultsBox->fCreateRow( mRows[i].mColumns );
			mResultsBox->SetItemData( createdIdx, i );
		}

		mResultsBox->SortItems( fSortList, (long)this );
		mResultsBox->fAutosizeAllColumns( );

		mResultsBox->Select( 0 );
	}

	void tSearchableOpenFilesDialog::fOnOpen( wxCommandEvent& event )
	{
		mSelectedFiles.fDeleteArray( );

		// Retrieval above has primed the selected idx. Loop through selected
		// entries until we run out of items selected.
		u64 selected = mResultsBox->GetFirstSelected( );
		for( ; selected != -1; selected = mResultsBox->GetNextSelected( selected ) )
		{
			tFilePathPtr filepath( mRows[ mResultsBox->GetItemData( selected ) ].fPath( ).c_str( ) );
			mSelectedFiles.fPushBack( filepath );
		}

		Close( );
	}

	//NOTE: Most of the code below could be removed if I could've figured out how
	// to pass wxKeyEvents to mResultsBox 'cause then I wouldn't have had to
	// re-implement when the wxListView already does when it has focus :(
	void tSearchableOpenFilesDialog::fOnItemSelect( wxListEvent& event )
	{
		//mSelectPos = event.GetIndex( );
		if( !mWeAreSelecting && !mAnchorInit )
			mSelectPos = event.GetIndex( );
		if( !mWeAreSelecting && mAnchorInit )
		{		
			mAnchorPos = event.GetIndex( );
			mAnchorInit = false;
		}
		event.Skip( );
	}

	void tSearchableOpenFilesDialog::fClearAllSelected( )
	{
		int sel = mResultsBox->GetFirstSelected( );
		while( sel != -1 )
		{
			mResultsBox->Select( sel, false );
			sel = mResultsBox->GetNextSelected( sel );
		}
	}

	void tSearchableOpenFilesDialog::fUpdateSelect( int dir, b32 shiftDown )
	{
		if( mResultsBox->GetItemCount( ) <= 0 )
			return;

		const int prevPos = mSelectPos;
		mSelectPos = fClamp<int>( mSelectPos + dir, 0, mResultsBox->GetItemCount( ) - 1 );

		if( !shiftDown )
		{
			fClearAllSelected( );
			mAnchorPos = mSelectPos;
		}
		else if( dir > 0 )//if we are above the shift position and are moving down
		{
			if( prevPos < mAnchorPos )
				mResultsBox->Select( prevPos, false );
		}
		else //if we are below the shift position and are moving up
		{
			if( prevPos > mAnchorPos )
				mResultsBox->Select( prevPos, false );
		}

		mResultsBox->EnsureVisible( mSelectPos );
		mResultsBox->Select( mSelectPos );
	}

	void tSearchableOpenFilesDialog::fOnChar( wxKeyEvent& event )
	{
		switch( event.GetKeyCode( ) )
		{
		case WXK_DOWN:
			mWeAreSelecting = true;
			mAnchorInit = true;
			fUpdateSelect( 1, event.ShiftDown( ) );
			mWeAreSelecting = false;
			break;
		case WXK_UP:
			mWeAreSelecting = true;
			mAnchorInit = true;
			fUpdateSelect( -1, event.ShiftDown( ) );
			mWeAreSelecting = false;
			break;
		default:
			event.Skip( );
		}
	}


}