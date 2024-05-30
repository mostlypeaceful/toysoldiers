#include "SigScriptPch.hpp"
#include "tFindInFilesDialog.hpp"
#include "tSigScriptWindow.hpp"
#include "tWxColumnListBox.hpp"
#include "tConfigurableBrowserTree.hpp"
#include "tScriptNotebook.hpp"
#include "tWxTextEditor.hpp"
#include "FileSystem.hpp"


namespace Sig
{
	//////////////////////////////////////////////////////////////////////////
	// Dialog section.
	//////////////////////////////////////////////////////////////////////////

	tFindInFilesDialog::tFindInFilesDialog( tSigScriptWindow* editorWindow, tConfigurableBrowserTree* browser, tScriptNotebook* notebook )
		: tFindInDialogBase( editorWindow )
		, mMainNotebook( notebook )
		, mBrowser( browser )
	{
	}

	void tFindInFilesDialog::fFindNext( b32 searchUp )
	{
		if( mSearchText->IsEmpty( ) )
			return;

		tScriptNotebookPage* currentPage = mMainNotebook->fGetCurrent( );
		if( !currentPage )
			return;

		const wxString searchText = mSearchText->GetLineText( 0 );

		fSearchNext( searchText, currentPage->fGetEditor( ), searchUp );
	}

	void tFindInFilesDialog::fSetSelectedText( )
	{
		tScriptNotebookPage* currentPage = mMainNotebook->fGetCurrent( );
		if( currentPage )
		{
			s32 selectStart = 0, selectEnd = 0;
			currentPage->fGetEditor( )->GetSelection( &selectStart, &selectEnd );
			if( selectStart == selectEnd )
			{
				const s32 currentPos = currentPage->fGetEditor( )->GetCurrentPos( );
				selectStart = currentPage->fGetEditor( )->WordStartPosition( currentPos, true );
				selectEnd = currentPage->fGetEditor( )->WordEndPosition( currentPos, true );

				currentPage->fGetEditor( )->SetSelection( selectStart, selectEnd );
			}
			wxString selected = currentPage->fGetEditor( )->GetSelectedText( );
			if( !selected.IsEmpty( ) )
				mSearchText->SetLabel( selected );
		}
	}

	
	void tFindInFilesDialog::fFindInFiles( const wxString& searchText, tGrowableArray< tOccurence >& occurences, b32 findOneOccurencePerFile )
	{
		const tFilePathPtrList& paths = mBrowser->fGetRelevantPaths( );

		for( u32 i = 0; i < paths.fCount( ); ++i )
		{
			tDynamicBuffer buffer;
			FileSystem::fReadFileToBuffer( buffer, paths[i], "\0" );

			fFindInFile( searchText, buffer, paths[i], occurences, findOneOccurencePerFile );
		}
	}

	void tFindInFilesDialog::fOnSearchNextPressed( wxCommandEvent& event )
	{
		if( mSearchText->IsEmpty( ) )
			return;

		tScriptNotebookPage* currentPage = mMainNotebook->fGetCurrent( );
		if( !currentPage )
			return;

		const wxString searchText = mSearchText->GetLineText( 0 );

		fSearchNext( searchText, currentPage->fGetEditor( ), mSearchUp->GetValue( ) );
	}


	void tFindInFilesDialog::fOnReplacePressed( wxCommandEvent& event )
	{
		if( mSearchText->IsEmpty( ) )
			return;

		tScriptNotebookPage* currentPage = mMainNotebook->fGetCurrent( );
		if( !currentPage )
			return;

		s32 startPos = 0, endPos = 0;
		currentPage->fGetEditor( )->GetSelection( &startPos, &endPos );
		const wxString searchText = mSearchText->GetLineText( 0 );

		if( startPos == endPos )
		{
			fSearchNext( searchText, currentPage->fGetEditor( ), mSearchUp->GetValue( ) );
			return;
		}

		const wxString replacementText = mReplacementText->GetLineText( 0 );
		currentPage->fGetEditor( )->ReplaceSelection( replacementText );

		fSearchNext( searchText, currentPage->fGetEditor( ), mSearchUp->GetValue( ) );
	}

	void tFindInFilesDialog::fOnReplaceAllPressed( wxCommandEvent& event )
	{
		if( mSearchText->IsEmpty( ) )
			return;

		const wxString searchText = mSearchText->GetLineText( 0 );
		const wxString replacementText = mReplacementText->GetLineText( 0 );

		// last argument == true means there will only be one record per file.
		tGrowableArray< tOccurence > occurences;
		fFindInFiles( searchText, occurences, true );

		for( u32 i = 0; i < occurences.fCount( ); ++i )
		{
			const u32 openedIdx = mMainNotebook->fOpenDoc( occurences[i].mFile );
			tScriptNotebookPage* page = mMainNotebook->fGetPage( openedIdx );
			page->fGetEditor( )->BeginUndoAction( );
			page->fGetEditor( )->SetCurrentPos( 0 );
			page->fGetEditor( )->SetAnchor( 0 );

			// Replace all strings found in this file.
			while( fSearchNext( searchText, page->fGetEditor( ), false, false ) != wxSTC_INVALID_POSITION )
				page->fGetEditor( )->ReplaceSelection( replacementText );

			page->fGetEditor( )->EndUndoAction( );
		}
	}
	
	void tFindInFilesDialog::fSelectItem( )
	{
		// Test for if anything is selected.
		u64 selected = mResultsBox->GetFirstSelected( );
		if( selected == -1 )
			return;

		// Retrieval above has primed the selected idx. Loop through selected
		// entries until we run out of items selected.
		for( ; selected != -1; selected = mResultsBox->GetNextSelected( selected ) )
		{
			const u32 occurenceIdx = mResultsBox->GetItemData( selected );
			sigassert( occurenceIdx < mOccurences.fCount( ) );

			// Open and focus on the line
			const tOccurence& thisOcc = mOccurences[ occurenceIdx ];
			mMainNotebook->fOpenAndFocus( thisOcc.mFile, thisOcc.mLineNum, thisOcc.mColNum );
		}
	}

	s32 tFindInFilesDialog::fSearchNext( const char* searchFor, tWxTextEditor* editor, b32 searchUp, b32 wrap )
	{
		u32 flags = 0;
		if( mMatchCase->GetValue( ) )
			flags |= wxSTC_FIND_MATCHCASE;
		if( mMatchWholeWord->GetValue( ) )
			flags |= wxSTC_FIND_WHOLEWORD;

		s32 foundPos = -1;
		s32 curs = editor->GetCurrentPos();
		b32 wasWrapped = false;
		if( searchUp )
		{
			editor->SearchAnchor( );
			foundPos = editor->SearchPrev( flags, searchFor );

			// Do wrapping.
			if( foundPos == -1 && wrap )
			{
				editor->GotoPos( editor->GetLineEndPosition( editor->GetLineCount( )-1 ) );
				editor->SearchAnchor( );
				foundPos = editor->SearchPrev( flags, searchFor );
				wasWrapped = true;
			}
		}
		else
		{
			s32 startPos = 0, endPos = 0;
			editor->GetSelection( &startPos, &endPos );
			editor->GotoPos( endPos );
			editor->SearchAnchor();
			foundPos = editor->SearchNext( flags, searchFor );

			// Do wrapping.
			if( foundPos == -1 && wrap )
			{
				editor->GotoPos( 0 );
				editor->SearchAnchor( );
				foundPos = editor->SearchNext( flags, searchFor );
				wasWrapped = true;
			}
		}

		if( foundPos != -1 )
		{
			// Center if the caret goes out of frame.
			s32 targetLine = editor->LineFromPosition( foundPos );
			editor->fFrameCaretIfNecessary( targetLine );
		}

		return foundPos;
	}
}
