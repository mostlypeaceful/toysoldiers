#include "ToolsGuiPch.hpp"
#include "tFindInDialogBase.hpp"
#include "tWxColumnListBox.hpp"
#include "FileSystem.hpp"


namespace Sig
{
	static const u32 cOutsideBorderSpacer	= 2;
	static const u32 cItemSpacer			= 6;

	enum tFindTabs
	{
		cFind = 0,
		cFindInFiles,
		cReplace,
		cReplaceInFiles,
		cFindTabsCount
	};

	// This should match the enum.
	static const wxString cFindLabels[ cFindTabsCount ] =
	{
		"Quick Find",
		"Find In ",
		"Quick Replace",
		"Replace In ",
	};

	enum tDialogEvents
	{
		ID_Find = 1,
		ID_FindInFiles,
		ID_Replace,
		ID_ReplaceInFiles,
		ID_SearchNext,
		ID_SearchPrev,

		Num_Events
	};

	static const u32 cDialogFlags = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxTAB_TRAVERSAL | wxSTAY_ON_TOP;

	//////////////////////////////////////////////////////////////////////////
	// Dialog section.
	//////////////////////////////////////////////////////////////////////////

	void tFindInDialogBase::tOccurence::fReplaceString( std::string& str, u32 replaceLen, const std::string& with ) const
	{
		const char* start = str.c_str( );
		for( u32 i = 0; i < mLineNum; ++i )
			start = StringUtil::fReadLine( start );
		start += mColNum;

		std::string a( str.c_str( ), start );
		std::string c( start + replaceLen );

		str = a + with + c;
	}

	tFindInDialogBase::tFindInDialogBase( wxWindow* parent, const wxString& fileTypeName, b32 enableSingleFinds )
		: wxDialog( parent, wxID_ANY, "Find and Replace", wxDefaultPosition, wxDefaultSize, cDialogFlags )
		, mParent( parent )
		, mFirstOpen( true )
		, mFileTypeName( fileTypeName )
		, mEnableSingleFinds( enableSingleFinds )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetBackgroundColour( GetBackgroundColour( ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		fBuildGui( );

		// Add button that will handle Esc and also closing the dialog too.
		wxSizer* buttonsSizer = CreateButtonSizer( wxCANCEL );
		GetSizer( )->Add( buttonsSizer, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT, cOutsideBorderSpacer );
		GetSizer( )->AddSpacer( cOutsideBorderSpacer );

		Layout( );
		Refresh( );
	}

	void tFindInDialogBase::fOpenFind( )
	{
		mComboBox->SetSelection( mFindOptionsIndex[ cFind ] );

		// Turn some things on.
		mHorizontalSizer->Show( mSearchUp );
		mHorizontalSizer->Show( mFindNext );

		// Turn some things off.
		GetSizer( )->Hide( mReplaceTitle );
		GetSizer( )->Hide( mReplacementText );
		mHorizontalSizer->Hide( mFindAll );
		mHorizontalSizer->Hide( mReplaceButton );
		mHorizontalSizer->Hide( mReplaceAllButton );
		GetSizer( )->Hide( mResultsBox );

		fCommonOpen( );
	}

	void tFindInDialogBase::fOpenFindInFiles( )
	{
		mComboBox->SetSelection(  mFindOptionsIndex[ cFindInFiles ] );

		// Turn some things on.
		mHorizontalSizer->Show( mFindAll );
		GetSizer( )->Show( mResultsBox );
		//GetSizer( )->Show( mSelectButton );

		// Turn some things off.
		GetSizer( )->Hide( mReplaceTitle );
		GetSizer( )->Hide( mReplacementText );
		mHorizontalSizer->Hide( mSearchUp );
		mHorizontalSizer->Hide( mFindNext );
		mHorizontalSizer->Hide( mReplaceButton );
		mHorizontalSizer->Hide( mReplaceAllButton );

		fCommonOpen( );

		// 544 is exactly how wide Replace and Replace In Files are.
		SetSize( 544, -1 );
	}

	void tFindInDialogBase::fOpenReplace( )
	{
		mComboBox->SetSelection(  mFindOptionsIndex[ cReplace ] );

		// Turn some things on.
		
		GetSizer( )->Show( mReplaceTitle );
		GetSizer( )->Show( mReplacementText );
		mHorizontalSizer->Show( mSearchUp );
		mHorizontalSizer->Show( mFindNext );
		mHorizontalSizer->Show( mReplaceButton );

		// Turn some things off.
		mHorizontalSizer->Hide( mFindAll );
		GetSizer( )->Hide( mResultsBox );
		mHorizontalSizer->Hide( mReplaceAllButton );

		fCommonOpen( );
	}

	void tFindInDialogBase::fOpenReplaceInFiles( )
	{
		mComboBox->SetSelection(  mFindOptionsIndex[ cReplaceInFiles ] );

		// Turn some things on.
		GetSizer( )->Show( mReplaceTitle );
		GetSizer( )->Show( mReplacementText );
		mHorizontalSizer->Show( mReplaceAllButton );
		if( mEnableSingleFinds ) 
		{
			mHorizontalSizer->Show( mSearchUp );
			mHorizontalSizer->Show( mFindNext );
		}

		// Turn some things off.
		mHorizontalSizer->Hide( mFindAll );
		if( !mEnableSingleFinds ) 
		{
			mHorizontalSizer->Hide( mSearchUp );
			mHorizontalSizer->Hide( mFindNext );
		}
		GetSizer( )->Hide( mResultsBox );
		mHorizontalSizer->Hide( mReplaceButton );

		fCommonOpen( );
	}

	void tFindInDialogBase::fBuildGui( )
	{
		SetBackgroundColour( GetBackgroundColour( ) );

		// determine which options are actually available, build a value lookup table
		mFindOptionsId.fSetCount( 0 );
		mFindOptionsIndex.fSetCount( 0 );
		wxString findLabels[ cFindTabsCount ];

		u32 index = 0;
		for( u32 i = 0; i < cFindTabsCount; ++i )
		{
			mFindOptionsId.fPushBack( i );
			mFindOptionsIndex.fPushBack( index );

			if( i == cFindInFiles || i == cReplaceInFiles )
				findLabels[ index++ ] = cFindLabels[ i ] + mFileTypeName;
			else if( mEnableSingleFinds )
				findLabels[ index++ ] = cFindLabels[ i ];
			else
				mFindOptionsId.fPopBack( );
		}

		mComboBox = new wxComboBox( this, wxID_ANY, findLabels[0], wxDefaultPosition, wxDefaultSize, index, findLabels, wxCB_DROPDOWN | wxCB_READONLY );
		GetSizer( )->Add( mComboBox, 0, wxALL, cItemSpacer );

		// Text box and title.
		wxStaticText* staticText = new wxStaticText( this, wxID_ANY, "Find what:", wxDefaultPosition, wxDefaultSize );
		GetSizer( )->Add( staticText, 0, wxALL, cItemSpacer );

		mSearchText = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
		GetSizer( )->Add( mSearchText, 0, wxLEFT | wxRIGHT | wxEXPAND, cItemSpacer );

		// Add replacement text and title.
		mReplaceTitle = new wxStaticText( this, wxID_ANY, "Replace with:", wxDefaultPosition, wxDefaultSize );
		GetSizer( )->Add( mReplaceTitle, 0, wxALL, cItemSpacer );

		mReplacementText = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
		GetSizer( )->Add( mReplacementText, 0, wxLEFT | wxRIGHT | wxEXPAND, cItemSpacer );

		// this for options.
		mHorizontalSizer = new wxBoxSizer( wxHORIZONTAL );
		GetSizer( )->Add( mHorizontalSizer, 0, wxEXPAND, 0 );

		// Match case.
		mMatchCase = new wxCheckBox( this, wxID_ANY, "Match case" );
		mMatchCase->SetValue( false );
		mHorizontalSizer->Add( mMatchCase, 0, wxALL | wxALIGN_CENTER_VERTICAL, cItemSpacer );
		mHorizontalSizer->AddSpacer( cItemSpacer );

		// Match whole word.
		mMatchWholeWord = new wxCheckBox( this, wxID_ANY, "Match whole word" );
		mMatchWholeWord->SetValue( false );
		mHorizontalSizer->Add( mMatchWholeWord, 0, wxALL | wxALIGN_CENTER_VERTICAL, cItemSpacer );
		mHorizontalSizer->AddSpacer( cItemSpacer );

		// Search up.
		mSearchUp = new wxCheckBox( this, wxID_ANY, "Search up" );
		mSearchUp->SetValue( false );
		mHorizontalSizer->Add( mSearchUp, 0, wxALL | wxALIGN_CENTER_VERTICAL, cItemSpacer );
		mHorizontalSizer->AddSpacer( cItemSpacer );

		// Find Next button.
		mFindNext = new wxButton( this, wxID_ANY, "Find Next", wxDefaultPosition, wxDefaultSize );
		mHorizontalSizer->Add( mFindNext, 1, wxALIGN_RIGHT | wxALL, cItemSpacer );

		// Find All button.
		mFindAll = new wxButton( this, wxID_ANY, "Find All", wxDefaultPosition, wxDefaultSize );
		mHorizontalSizer->Add( mFindAll, 1, wxALIGN_RIGHT | wxALL, cItemSpacer );

		// Replace button.
		mReplaceButton = new wxButton( this, wxID_ANY, "Replace", wxDefaultPosition, wxDefaultSize );
		mHorizontalSizer->Add( mReplaceButton, 1, wxALIGN_RIGHT | wxALL, cItemSpacer );

		// Replace All button.
		mReplaceAllButton = new wxButton( this, wxID_ANY, "Replace All", wxDefaultPosition, wxDefaultSize );
		mHorizontalSizer->Add( mReplaceAllButton, 1, wxALIGN_RIGHT | wxALL, cItemSpacer );

		// Results list box.
		mResultsBox = new tWxColumnListBox( this, wxID_ANY, wxDefaultPosition, wxSize( wxDefaultSize.x, 200 ), wxLC_NO_HEADER | wxLC_REPORT | wxLC_SINGLE_SEL );
		GetSizer( )->Add( mResultsBox, 1, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, cItemSpacer );

		// Make one column. The title is not seen because of wxLC_NO_HEADER above.
		mResultsBox->fCreateColumn( "Search Results" );

		// Select item button.
		//mSelectButton = new wxButton( this, wxID_ANY, "Select", wxDefaultPosition, wxDefaultSize );
		//GetSizer( )->Add( mSelectButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, cItemSpacer );


		// Set up hot keys.
		wxAcceleratorEntry entries[Num_Events];
		entries[ID_Find].Set(			wxACCEL_CTRL, 'F', ID_Find );
		entries[ID_FindInFiles].Set(	wxACCEL_CTRL | wxACCEL_SHIFT, 'F', ID_FindInFiles );
		entries[ID_Replace].Set(		wxACCEL_CTRL, 'H', ID_Replace );
		entries[ID_ReplaceInFiles].Set(	wxACCEL_CTRL | wxACCEL_SHIFT, 'H', ID_ReplaceInFiles );
		entries[ID_SearchNext].Set(		wxACCEL_NORMAL, WXK_F3, ID_SearchNext );
		entries[ID_SearchPrev].Set(		wxACCEL_NORMAL, WXK_F2, ID_SearchPrev );

		wxAcceleratorTable accel( Num_Events, entries );
		SetAcceleratorTable( accel );

		// Connect all events.
		mComboBox->Connect(			wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(tFindInDialogBase::fOnComboBoxChange), NULL, this );
		mSearchText->Connect(		wxEVT_COMMAND_TEXT_ENTER,		wxCommandEventHandler(tFindInDialogBase::fOnEnter), NULL, this );
		mReplacementText->Connect(	wxEVT_COMMAND_TEXT_ENTER,		wxCommandEventHandler(tFindInDialogBase::fOnEnter), NULL, this );
		mFindNext->Connect(			wxEVT_COMMAND_BUTTON_CLICKED,	wxCommandEventHandler(tFindInDialogBase::fOnSearchNextPressed), NULL, this );
		mFindAll->Connect(			wxEVT_COMMAND_BUTTON_CLICKED,	wxCommandEventHandler(tFindInDialogBase::fOnSearchAllPressed), NULL, this );
		mReplaceButton->Connect(	wxEVT_COMMAND_BUTTON_CLICKED,	wxCommandEventHandler(tFindInDialogBase::fOnReplacePressed), NULL, this );
		mReplaceAllButton->Connect(	wxEVT_COMMAND_BUTTON_CLICKED,	wxCommandEventHandler(tFindInDialogBase::fOnReplaceAllPressed), NULL, this );
		//mSelectButton->Connect(		wxEVT_COMMAND_BUTTON_CLICKED,	wxCommandEventHandler(tFindInDialogBase::fOnSelectPressed), NULL, this );
		mResultsBox->Connect(		wxEVT_LEFT_DCLICK,				wxMouseEventHandler(tFindInDialogBase::fOnListBoxDClick), NULL, this);

		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(tFindInDialogBase::fOnHotkey), NULL, this );

		Layout( );
		Refresh( );
	}

	void tFindInDialogBase::fCommonOpen( )
	{
		mSearchText->SetFocus( );

		// Handle centering.
		if( mFirstOpen )
		{
			wxSize parentWindowSize = mParent->GetSize( );
			wxPoint parentWindowPos = mParent->GetPosition( );
			wxSize dialogSize = GetSize( );
			wxPoint dialogPos = GetPosition( );

			wxPoint dialogCenter( dialogPos.x + dialogSize.x / 2.f, dialogPos.y + dialogSize.y / 2.f );
			if( dialogCenter.x < parentWindowPos.x || dialogCenter.x > (parentWindowPos.x + parentWindowSize.x)
				|| dialogCenter.y < parentWindowPos.y || dialogCenter.y > (parentWindowPos.y + parentWindowSize.y) )
			{
				wxPoint newPosition( parentWindowPos.x + parentWindowSize.x / 2.f, parentWindowPos.y + parentWindowSize.y / 2.f );

				SetPosition( newPosition );
			}

			mFirstOpen = false;
		}

		
		fSetSelectedText( );

		// Always select the text box so people can type immediately.
		mSearchText->SelectAll( );

		SetSize( -1, -1, -1, -1, wxSIZE_AUTO );
		Layout( );
		Show( );
	}

	void tFindInDialogBase::fFindInFile( const wxString& findStr, const tDynamicBuffer& text, const tFilePathPtr& file, tGrowableArray< tOccurence >& occurences, b32 findOneOccurence )
	{
		// Break out all the lines by the newline delimiter.
		tGrowableArray< std::string > lines;
		StringUtil::fSplit( lines, ( const char* )text.fBegin( ), "\n" );

		// Look through all lines for 
		for( u32 i = 0; i < lines.fCount( ); ++i )
		{
			const char* lineStart = lines[i].c_str( );
			const char* foundPos = 0;
			if( foundPos = fFindStr( lines[i].c_str( ), findStr.c_str( ) ) )
			{
				tOccurence newOccurence;
				newOccurence.mFile = file;
				newOccurence.mLineNum = i;
				newOccurence.mColNum = foundPos - lineStart;

				std::stringstream thing;
				thing << file.fCStr( );
				thing << "(";
				thing << i;
				thing << "):            ";
				thing << lines[i];

				newOccurence.mDisplayLine = thing.str( );

				if( !findOneOccurence )
				{
					// Create the row and record the index for future retrieval.
					const u64 addedIdx = mResultsBox->fCreateRow( newOccurence.mDisplayLine );
					mResultsBox->SetItemData( addedIdx, occurences.fCount( ) );
				}

				occurences.fPushBack( newOccurence );

				if( findOneOccurence )
					return;
			}
		}
	}

	void tFindInDialogBase::fOnEnter( wxCommandEvent& event )
	{
		// The combobox is zero-based but the different event IDs are 1-based.
		const u32 id = mFindOptionsId[ mComboBox->GetSelection( ) + 1 ];

		switch( id )
		{
		case ID_Find:
		case ID_Replace:
		case ID_ReplaceInFiles:
			fOnSearchNextPressed( event ); 
			break;

		case ID_FindInFiles:
			fOnSearchAllPressed( event );
			break;

		default: break;
		}
	}

	void tFindInDialogBase::fOnSearchAllPressed( wxCommandEvent& event )
	{
		if( mSearchText->IsEmpty( ) )
			return;

		// Clear out the old results but leave columns intact.
		mResultsBox->DeleteAllItems( );

		const wxString searchText = mSearchText->GetLineText( 0 );
		mOccurences.fDeleteArray( );

		fFindInFiles( searchText, mOccurences );

		// Resize the column so all the entries fit
		mResultsBox->SetColumnWidth( 0, wxLIST_AUTOSIZE_USEHEADER );
	}

	void tFindInDialogBase::fOnListBoxDClick( wxMouseEvent& event )
	{
		fSelectItem( );
	}

	void tFindInDialogBase::fOnSelectPressed( wxCommandEvent& event )
	{
		fSelectItem( );
	}

	void tFindInDialogBase::fOnReplacementTextEnter( wxCommandEvent& event )
	{
		fOnSearchNextPressed( event );
	}

	void tFindInDialogBase::fOnComboBoxChange( wxCommandEvent& event )
	{
		const s32 newIdx = mFindOptionsId[ mComboBox->GetSelection( ) ];

		switch( newIdx )
		{
		case cFind:				fOpenFind( ); break;
		case cFindInFiles:		fOpenFindInFiles( ); break;
		case cReplace:			fOpenReplace( ); break;
		case cReplaceInFiles:	fOpenReplaceInFiles( ); break;
			
		default: break;
		}
	}

	void tFindInDialogBase::fOnHotkey(wxCommandEvent& event)
	{
		const s32 id = event.GetId( );

		switch( id )
		{
		case ID_Find:			fOpenFind( ); break;
		case ID_FindInFiles:	fOpenFindInFiles( ); break;
		case ID_Replace:		fOpenReplace( ); break;
		case ID_ReplaceInFiles:	fOpenReplaceInFiles( ); break;
		case ID_SearchPrev:		fFindNext( true ); break;
		case ID_SearchNext:		fFindNext( ); break;

		default: break;
		}
	}


	const char* tFindInDialogBase::fFindStr( const char* searchIn, const char* searchFor )
	{
		const b32 matchCase = mMatchCase->GetValue( );
		const char* foundPos = (matchCase) ? strstr( searchIn, searchFor ) : StringUtil::fStrStrI( searchIn, searchFor );
		if( !foundPos )
			return foundPos;

		const b32 matchWord = mMatchWholeWord->GetValue( );
		if( matchWord )
		{
			const u32 strLen = strlen( searchFor );

			// Check front of string. First: if the search substring starts the same position as the string
			// to search in, then there's nothing before it and we are ok.
			if( foundPos != searchIn )
			{
				// Check the character before the found position. If it's an alpha char, match word fails.
				char prevChar = *(foundPos - 1 );
				if( isalpha( prevChar ) )
					return 0;
			}

			// Check the back of the string. Test if the end of the search text is the end of the found word.
			const char* endPos = (foundPos + strLen );
			const char* endOfSearchFor = searchFor + strlen( searchFor );
			if( endPos != endOfSearchFor )
			{
				// Check the end character is not alpha.
				char endChar = *endPos;
				if( isalpha( *endPos ) )
					return 0;
			}
		}

		return foundPos;
	}
}
