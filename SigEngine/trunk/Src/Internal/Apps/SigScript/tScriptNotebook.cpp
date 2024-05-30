//------------------------------------------------------------------------------
// \file tScriptNotebook.cpp - 18 Aug 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "SigScriptPch.hpp"
#include "tScriptNotebook.hpp"
#include "tWxTextEditor.hpp"
#include "tWxScriptDesigner.hpp"
#include "tLocmlConvertDictionary.hpp"

namespace Sig
{
	enum
	{
		ID_ClosePage = 1,
	};

	tScriptNotebookPage::tScriptNotebookPage( tScriptNotebook* parentNotebook, b32 inEditor )
		: wxPanel( parentNotebook )
		, mParent( parentNotebook )
		, mInTextView( inEditor )
	{
		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		mEditorView = new tWxTextEditor( this );
		mEditorView->UsePopUp( false );
		GetSizer( )->Add( mEditorView, 1, wxEXPAND | wxALL );

		mDesignerPanel = new tWxScriptDesigner( this, mEditorView );
		GetSizer( )->Add( mDesignerPanel, 1, wxEXPAND | wxALL );

		Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( tScriptNotebookPage::fOnRightClick ), NULL, this );
		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tScriptNotebookPage::fOnAction ), NULL, this );
	}

	void tScriptNotebookPage::fToggleView( )
	{
		if( fGetInTextEdit( ) )
			fSwitchToDesigner( );
		else
			fSwitchToText( );
	}

	void tScriptNotebookPage::fSwitchToText( )
	{
		mInTextView = true;

		GetSizer( )->Show( mEditorView, true, true );
		GetSizer( )->Hide( mDesignerPanel, true );

		GetSizer( )->Layout( );
	} 

	void tScriptNotebookPage::fSwitchToDesigner( )
	{
		mInTextView = false;

		fRefreshDesigner( );

		GetSizer( )->Show( mDesignerPanel, true, true );
		GetSizer( )->Hide( mEditorView, true );

		GetSizer( )->Layout( );
	}

	void tScriptNotebookPage::fGoToLine( s32 lineNum, s32 colNum )
	{
		if( lineNum < 0 ) lineNum = 0;
		if( colNum < 0 ) colNum = 0;

		lineNum = fMin( lineNum, mEditorView->GetLineCount( ) );
		const s32 lineBegin = mEditorView->PositionFromLine( lineNum );
		colNum = fMin( colNum, mEditorView->GetLineEndPosition( lineNum ) - lineBegin );

		const s32 targetPos = ( colNum == -1 ) ? mEditorView->GetLineIndentPosition( lineNum ) : lineBegin + colNum;

		mEditorView->SetFocus( );
		mEditorView->SetAnchor( targetPos );
		mEditorView->SetCurrentPos( targetPos );
		mEditorView->fFrameCaretIfNecessary( lineNum );
	}

	void tScriptNotebookPage::fSelectCurrentWord( )
	{
		mEditorView->SetSelectionEnd( mEditorView->WordEndPosition( mEditorView->GetCurrentPos( ), true ) );
	}

	tWxTextEditor* tScriptNotebookPage::fGetEditor( )
	{
		return mEditorView;
	}

	void tScriptNotebookPage::fNewDoc( ) 
	{ 
		mEditorView->fNewDoc( ); 
	}

	b32 tScriptNotebookPage::fOpenDoc( ) 
	{ 
		b32 status = mEditorView->fOpenDoc( ); 

		if( status ) 
			fRefreshDesigner( ); 

		return status;
	}

	b32 tScriptNotebookPage::fOpenDoc( const tFilePathPtr& filename ) 
	{ 
		b32 status = mEditorView->fOpenDoc( filename ); 

		if( !status )
			return false;

		if( mEditorView->fIsScript( ) ) 
			fRefreshDesigner( ); 

		return true;
	}

	b32 tScriptNotebookPage::fSaveDoc( ) { return mEditorView->fSaveDoc( ); }
	b32 tScriptNotebookPage::fSaveDocAs( ) { return mEditorView->fSaveDocAs( ); }

	b32 tScriptNotebookPage::fNeedsSave( ) const { return mEditorView->fNeedsSave( ); }
	b32 tScriptNotebookPage::fNeedsSaveAs( ) const { return mEditorView->fNeedsSaveAs( ); }

	b32 tScriptNotebookPage::fCheckForChangesAndSave( ) { return mEditorView->fCheckForChangesAndSave( ); }

	void tScriptNotebookPage::fCheckForOutsideChanges( ) 
	{ 
		if( mEditorView->fCheckForOutsideChanges( ) )
			fRefreshDesigner( );
	}

	wxString tScriptNotebookPage::fGetDisplayName( ) const { return mEditorView->fGetDisplayName( ); }
	tFilePathPtr tScriptNotebookPage::fGetFilePath( ) const { return mEditorView->fGetFilePath( ); }

	void tScriptNotebookPage::fUndo( ) 
	{ 
		mEditorView->Undo( );
		fRefreshDesigner( );
	}

	void tScriptNotebookPage::fRedo( ) 
	{ 
		mEditorView->Redo( );
		fRefreshDesigner( );
	}

	void tScriptNotebookPage::fRefreshDesigner( )
	{
		mDesignerPanel->fParseScript( );
	}

	void tScriptNotebookPage::fOnRightClick( wxMouseEvent& event )
	{
		tEditorContextAction::fDisplayContextMenuOnRightClick( this, event, mParent->fTextContextActions( ) );
	}

	void tScriptNotebookPage::fOnAction( wxCommandEvent& event )
	{
		tEditorContextAction::fHandleContextActionFromRightClick( this, event, mParent->fTextContextActions( ) );
	}



	tScriptNotebook::tScriptNotebook( wxWindow* parent, Win32Util::tRecentlyOpenedFileList* recentFiles )

		: wxAuiNotebook( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
		wxAUI_NB_TAB_MOVE 
		| wxAUI_NB_CLOSE_BUTTON
		| wxAUI_NB_MIDDLE_CLICK_CLOSE 
		| wxAUI_NB_WINDOWLIST_BUTTON
		| wxAUI_NB_SCROLL_BUTTONS )
		, mRecentFiles( recentFiles )
		, mTextEditSet( true )
		, mSuspendOutsideCheck( false )
		, mRightClickTab( -1 )
	{
		Connect( wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP, wxAuiNotebookEventHandler( tScriptNotebook::fOnTabRightClick ), NULL, this );
		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tScriptNotebook::fOnAction ), NULL, this );
	}

	b32 tScriptNotebook::fOnNotebookClose( )
	{
		b32 vetoed = false;
		const u32 numPages = GetPageCount( );
		for( u32 i = 0; i < numPages; ++i )
		{
			tScriptNotebookPage* ithPage = static_cast< tScriptNotebookPage* >(  GetPage( i ) );
			sigassert( ithPage );

			// If a dialog is needed, select the page so the user knows which one they're saving.
			// It would be nice if these could be combined with fSaveDoc or fCheckForChangesAndSave.
			if( ithPage->fNeedsSave( ) )
				SetSelection( i );

			// This checks for changes/saves and returns true if the event needs to be vetoed.
			if( ithPage->fCheckForChangesAndSave( ) )
			{
				vetoed = true;
				break;
			}
		}

		return vetoed;
	}

	void tScriptNotebook::fNewDoc( )
	{
		tScriptNotebookPage* newTab = new tScriptNotebookPage( this, mTextEditSet );
		newTab->fGetEditor( )->fConfigureForSquirrel( );
		AddPage( newTab, newTab->fGetDisplayName( ), 1 );

		if( mTextEditSet )
			newTab->fSwitchToText( );
		else
			newTab->fSwitchToDesigner( );
	}

	u32 tScriptNotebook::fOpenDoc( )
	{
		return fOpenAndCreate( );
	}

	u32 tScriptNotebook::fOpenDoc( const tFilePathPtr& file )
	{
		return fOpenAndCreate( file );
	}

	void tScriptNotebook::fOpenAndFocus( const tFilePathPtr& file, u32 lineNum, u32 colNum )
	{
		const u32 openedIdx = fOpenAndCreate( file );
		if( openedIdx == -1 )
			return;

		// Can't set this stuff for designer view.
		if( !mTextEditSet )
			return;

		tScriptNotebookPage* openedPage = static_cast< tScriptNotebookPage* >( GetPage( openedIdx ) );
		sigassert( openedPage );

		openedPage->fGoToLine( lineNum, colNum );
		openedPage->fSelectCurrentWord( );
	}

	u32 tScriptNotebook::fOpenOrFind( const tFilePathPtr& file )
	{
		u32 retIdx = fFind( file );
		if( retIdx != -1 )
			return retIdx;

		return fOpenDoc( file );
	}

	void tScriptNotebook::fSaveAll( )
	{
		const u32 numPages = GetPageCount( );
		u32 numberNeedingCheckout = 0;
		for( u32 i = 0; i < numPages; ++i )
		{
			tScriptNotebookPage* ithPage = static_cast< tScriptNotebookPage* >( GetPage( i ) );
			sigassert( ithPage );

			if( !ithPage->fNeedsSave() && !ithPage->fNeedsSaveAs() )
				continue;

			if( Win32Util::fIsFileReadOnly( ithPage->fGetFilePath() ) )
				++numberNeedingCheckout;
		}
		
		if( numberNeedingCheckout > 1 
			&& wxMessageBox( "Check out all files that need saving?", "Check Out Files", wxYES_NO, this ) == wxYES)
		{
			ToolsPaths::fCheckOutAllSubsequent( true );
		}

		for( u32 i = 0; i < numPages; ++i )
		{
			tScriptNotebookPage* ithPage = static_cast< tScriptNotebookPage* >( GetPage( i ) );
			sigassert( ithPage );

			// If a dialog is needed, select the page so the user knows which one they're saving.
			if( ithPage->fNeedsSaveAs( ) )
				SetSelection( i );

			if( ithPage->fNeedsSave( ) && ithPage->fSaveDoc( ) )
				SetPageText( i, ithPage->fGetDisplayName( ) );
		}

		ToolsPaths::fCheckOutAllSubsequent( false );
	}

	void tScriptNotebook::fSaveCurrent( )
	{
		tScriptNotebookPage* currentTab = fGetCurrent( );
		if( !currentTab )
			return;

		if( currentTab->fSaveDoc( ) )
			SetPageText( GetPageIndex( currentTab ), currentTab->fGetDisplayName( ) );
	}

	void tScriptNotebook::fSaveCurrentAs( )
	{
		tScriptNotebookPage* currentTab = fGetCurrent( );
		if( !currentTab )
			return;

		mSuspendOutsideCheck = true;

		if( currentTab->fSaveDocAs( ) )
		{
			mRecentFiles->fAdd( currentTab->fGetFilePath( ) );
			mRecentFiles->fSave( );

			SetPageText( GetPageIndex( currentTab ), currentTab->fGetDisplayName( ) );

			fCloseAny( currentTab->fGetFilePath( ), currentTab );
		}

		mSuspendOutsideCheck = false;
	}

	void tScriptNotebook::fCloseCurrent( )
	{
		tScriptNotebookPage* currentPage = static_cast< tScriptNotebookPage* >( GetPage( GetSelection( ) ) );
		if( !currentPage )
			return;

		if( currentPage->fCheckForChangesAndSave( ) )
			return;

		DeletePage( GetSelection( ) );
	}

	void tScriptNotebook::fCloseAll( tScriptNotebookPage* skipPage )
	{
		const u32 numPages = GetPageCount( );
		for( u32 i = 0; i < numPages; ++i )
		{
			tScriptNotebookPage* ithPage = static_cast< tScriptNotebookPage* >( GetPage( i ) );
			sigassert( ithPage );

			if( ithPage == skipPage )
				continue;

			// If a dialog is needed, select the page so the user knows which one they're saving.
			// It would be nice if these could be combined with fSaveDoc or fCheckForChangesAndSave.
			if( ithPage->fNeedsSave( ) )
				SetSelection( i );

			// This checks for changes/saves and returns true if the event needs to be vetoed.
			if( ithPage->fCheckForChangesAndSave( ) )
				return;
		}

		for( u32 i = 0; i < GetPageCount( ); ++i )
		{
			tScriptNotebookPage* ithPage = static_cast< tScriptNotebookPage* >(  GetPage( i ) );
			sigassert( ithPage );
			if( ithPage == skipPage )
				continue;

			DeletePage( i-- );
		}
	}

	void tScriptNotebook::fCloseAny( const tFilePathPtr& fileToClose, tScriptNotebookPage* skipPage )
	{
		for( u32 i = 0; i < GetPageCount( ); ++i )
		{
			tScriptNotebookPage* ithPage = static_cast< tScriptNotebookPage* >( GetPage( i ) );
			sigassert( ithPage );

			if( ithPage == skipPage )
				continue;

			if( ithPage->fGetFilePath( ) != fileToClose )
				continue;

			// Check if they want to discard the existing file.
			SetSelection( i );

			const int result = wxMessageBox( "You have saved over a file that is currently open. Would you like to save the old file under a different name? The old version will be lost if it is not saved.",
				"Save Old File?", wxYES | wxNO | wxICON_WARNING );

			if(			result == wxYES )			{ ithPage->fSaveDocAs( ); }
			else if(	result == wxNO )			{ }
			else									{ log_warning( "Unknown result returned from Message Box" ); }

			DeletePage( i-- );
		}
	}

	void tScriptNotebook::fUndo( )
	{
		tScriptNotebookPage* currentTab = fGetCurrent( );
		if( !currentTab )
			return;

		currentTab->fUndo( );
	}

	void tScriptNotebook::fRedo( )
	{
		tScriptNotebookPage* currentTab = fGetCurrent( );
		if( !currentTab )
			return;

		currentTab->fRedo( );
	}

	tScriptNotebookPage* tScriptNotebook::fGetCurrent( )
	{
		const u32 selectedPage = GetSelection( );
		if( selectedPage == -1 )
			return 0;

		sigassert( selectedPage < GetPageCount( ) );
		return static_cast< tScriptNotebookPage* >( GetPage( selectedPage ) );
	}

	tScriptNotebookPage* tScriptNotebook::fGetPage( u32 i )
	{
		sigassert( i < GetPageCount( ) );
		return static_cast< tScriptNotebookPage* >( GetPage( i ) );
	}

	wxString tScriptNotebookPage::fGetSelectedText( )
	{
		return mEditorView->GetSelectedText( );
	}

	void tScriptNotebookPage::fConvertLocmlText( const tLocmlConvertDictionary& dict, std::string lang )
	{
		tWxTextEditor* editor = fGetEditor();

		editor->SetCurrentPos( 0 );
		editor->SetAnchor( 0 );

		while( fReplaceText( dict, lang ) != wxSTC_INVALID_POSITION ) { }
	}

	/*

	const Time::tStamp convertingStamp = Time::fGetStamp();

	CALL

	const f32 convertingTime = Time::fGetElapsedMs( convertingStamp, Time::fGetStamp() );
	log_warning( "Conv " << mLangTargets[i]->fGetLang() << " Time ms: " << convertingTime );

	*/

	s32 tScriptNotebookPage::fReplaceText( const tLocmlConvertDictionary& dict, std::string lang )
	{
		static wxString cTagStart( "<Text>" );
		static wxString cTagEnd( "</Text>" );

		static wxString cNameStart( "<i name=\"" );
		static wxString cNameEnd( "\"" );

		const u32 flags = 0;

		tWxTextEditor* editor = fGetEditor();

		// Move to the end of the current selection.
		s32 startPos = 0, endPos = 0;
		editor->GetSelection( &startPos, &endPos );
		editor->GotoPos( endPos );
		editor->SearchAnchor();


		// First, grab the name key.
		s32 nameStart = -1;
		editor->SearchNext( flags, cNameStart );
		editor->GetSelection( &startPos, &nameStart );

		if( nameStart == wxSTC_INVALID_POSITION )
			return wxSTC_INVALID_POSITION;

		editor->GotoPos( nameStart );
		editor->SearchAnchor();
		s32 nameEnd = editor->SearchNext( flags, cNameEnd );
		if( nameEnd == wxSTC_INVALID_POSITION )
			return wxSTC_INVALID_POSITION;

		editor->SetSelection( nameStart, nameEnd );
		wxString key = editor->GetSelectedText();


		// Search for the opening tag.
		s32 replaceStart = -1, replaceEnd = -1;

		editor->SearchNext( flags, cTagStart );
		editor->GetSelection( &startPos, &replaceStart );

		if( replaceStart == wxSTC_INVALID_POSITION )
			return wxSTC_INVALID_POSITION;


		// Search for the closing tag.
		editor->GotoPos( nameEnd );
		editor->SearchAnchor();
		replaceEnd = editor->SearchNext( flags, cTagEnd );
		if( replaceEnd == wxSTC_INVALID_POSITION )
			return wxSTC_INVALID_POSITION;

		// Make the replacement.
		editor->SetSelection( replaceStart, replaceEnd );

		std::string replacement = dict.fGetReplacementText( std::string( key.mb_str() ), lang );

		wxString rep( replacement.c_str() );
		editor->ReplaceSelection( rep );

		// Report ending position.
		editor->GetSelection( &startPos, &endPos );
		return endPos;
	}

	void tScriptNotebook::fCheckChanges( )
	{
		if( mSuspendOutsideCheck )
			return;

		for( u32 i = 0; i < GetPageCount( ); ++i )
			static_cast< tScriptNotebookPage* >( GetPage( i ) )->fCheckForOutsideChanges( );
	}

	void tScriptNotebook::fToggleViewMode( )
	{
		if( !fGetCurrent( )->fGetEditor( )->fIsScript( ) )
			return;

		if( mTextEditSet )
		{
			for( u32 i = 0; i < GetPageCount( ); ++i )
			{
				tScriptNotebookPage* thisPage = static_cast< tScriptNotebookPage* >( GetPage( i ) );
				if( !thisPage->fGetEditor( )->fIsScript( ) )
					continue;
				thisPage->fSwitchToDesigner( );
			}
		}
		else
		{
			for( u32 i = 0; i < GetPageCount( ); ++i )
			{
				tScriptNotebookPage* thisPage = static_cast< tScriptNotebookPage* >( GetPage( i ) );
				if( !thisPage->fGetEditor( )->fIsScript( ) )
					continue;
				thisPage->fSwitchToText( );
			}
		}

		mTextEditSet = !mTextEditSet;
	}

	void tScriptNotebook::fFocusAllOnLine( u64 lineNum )
	{
		for( u32 i = 0; i < GetPageCount( ); ++i )
		{
			tScriptNotebookPage* thisPage = static_cast< tScriptNotebookPage* >( GetPage( i ) );
			thisPage->fGoToLine( lineNum );
		}
	}

	void tScriptNotebook::fSetContextActions( const tEditorContextActionList& tabs, const tEditorContextActionList& texts )
	{
		mTabContextActions = tabs;
		mTextContextActions = texts;
	}

	tFilePathPtr tScriptNotebook::fRightClickFileName( )
	{
		const s32 sel = fRightClickTab( );
		sigassert( sel >= 0 );

		return fGetPage( sel )->fGetFilePath( );
	}

	u32 tScriptNotebook::fFind( const tFilePathPtr& file )
	{
		const u32 numPages = GetPageCount( );
		for( u32 i = 0; i < numPages; ++i )
		{
			tScriptNotebookPage* ithPage = static_cast< tScriptNotebookPage* >(  GetPage( i ) );
			sigassert( ithPage );

			if( ithPage->fGetFilePath( ) == file )
				return i;
		}

		return -1;
	}

	u32 tScriptNotebook::fOpenAndCreate( )
	{
		// Open browse dialog.
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			NULL, 
			"Open Script",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "untitled.nut" ),
			wxString( "*.nut" ),
			wxFD_OPEN ) );

		SetFocus( );

		if( openFileDialog->ShowModal( ) == wxID_OK )
			return fOpenDoc( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );

		return -1;
	}

	u32 tScriptNotebook::fOpenAndCreate( const tFilePathPtr& file )
	{
		const u32 existingFile = fFind( file );
		if( existingFile != -1 )
		{
			SetSelection( existingFile );

			mRecentFiles->fAdd( fGetCurrent( )->fGetFilePath( ) );
			mRecentFiles->fSave( );

			return existingFile;
		}

		tScriptNotebookPage* newTab = new tScriptNotebookPage( this );
		newTab->fOpenDoc( file );
		AddPage( newTab, newTab->fGetDisplayName( ), 1 );

		if( mTextEditSet )
			newTab->fSwitchToText( );
		else
			newTab->fSwitchToDesigner( );

		mRecentFiles->fAdd( newTab->fGetFilePath( ) );
		mRecentFiles->fSave( );

		return GetSelection( );
	}

	void tScriptNotebook::fOnPageClose( wxAuiNotebookEvent& event )
	{
		const u32 closingIdx = event.GetSelection( );
		tScriptNotebookPage* closingPage = static_cast< tScriptNotebookPage* >(  GetPage( closingIdx ) );
		sigassert( closingPage );
  
		if( closingPage->fCheckForChangesAndSave( ) )
			event.Veto( );
	}

	void tScriptNotebook::fOnSavepointChange( wxStyledTextEvent& event )
	{
		const u32 numPages = GetPageCount( );
		for( u32 i = 0; i < numPages; ++i )
		{
			tScriptNotebookPage* ithPage = static_cast< tScriptNotebookPage* >(  GetPage( i ) );
			sigassert( ithPage );

			SetPageText( i, ithPage->fGetDisplayName( ) );
		}
	}

	void tScriptNotebook::fOnTabRightClick( wxAuiNotebookEvent& event )
	{
		mRightClickTab = event.GetSelection( );
		tEditorContextAction::fDisplayContextMenuOnRightClick( this, wxGetMouseState( ), mTabContextActions );
	}

	void tScriptNotebook::fOnAction( wxCommandEvent& event )
	{
		tEditorContextAction::fHandleContextActionFromRightClick( this->GetParent( ), event, mTabContextActions );
		mRightClickTab = -1;
	}

	BEGIN_EVENT_TABLE(tScriptNotebook, wxAuiNotebook)
		EVT_AUINOTEBOOK_PAGE_CLOSE(		wxID_ANY,	tScriptNotebook::fOnPageClose )
		EVT_STC_SAVEPOINTREACHED(		wxID_ANY,	tScriptNotebook::fOnSavepointChange )
		EVT_STC_SAVEPOINTLEFT(			wxID_ANY,	tScriptNotebook::fOnSavepointChange )
	END_EVENT_TABLE()
}
