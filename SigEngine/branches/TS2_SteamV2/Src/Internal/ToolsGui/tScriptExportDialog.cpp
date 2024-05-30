#include "ToolsGuiPch.hpp"
#include "tScriptExportDialog.hpp"


namespace Sig { namespace ScriptData
{

	tScriptDataDialog::tScriptDataDialog( wxWindow* parent )
		: wxDialog( parent, wxID_ANY, "Browser", wxDefaultPosition, wxSize( 500, 600 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
	{
		wxSizer* sizer = new wxBoxSizer( wxVERTICAL );
		SetSizer( sizer );

		wxButton* button = new wxButton( this, wxID_ANY, "Rebuild" );
		sizer->Add( button, 0, wxEXPAND | wxALL );
		button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tScriptDataDialog::fRebuild ), NULL, this );

		mTree = new wxTreeCtrl( this );
		sizer->Add( mTree, 10, wxEXPAND | wxALL );
		mTree->Connect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxCommandEventHandler( tScriptDataDialog::fTreeSelect ), NULL, this );

		// Dynamic context testing
		sizer->Add( new wxStaticText( this, wxID_ANY, "Context:" ), 0, wxEXPAND | wxALL );
		mStatus = new wxStaticText( this, wxID_ANY, "Test" );
		sizer->Add( mStatus, 0, wxEXPAND | wxALL );

		mTextBox = new tWxTextBox( this );

		sizer->Add( mTextBox, 0, wxEXPAND | wxALL );
		mTextBox->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( tScriptDataDialog::fOnKeyDown ), NULL, this );
		mTextBox->Connect( wxEVT_KEY_UP, wxKeyEventHandler( tScriptDataDialog::fOnKeyUp ), NULL, this );	
		mTextBox->Connect( wxEVT_CHAR, wxKeyEventHandler( tScriptDataDialog::fOnKeyChar ), NULL, this );	
		mTextBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( tScriptDataDialog::fOnTextChanged ), NULL, this );
		mTextBox->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( tScriptDataDialog::fOnCursorMove ), NULL, this );

		mTree->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( tScriptDataDialog::fOnKeyDown ), NULL, this );
		
		mList = new wxListBox( this, wxID_ANY );
		sizer->Add( mList, 1, wxEXPAND | wxALL );

		mContextStack = new wxListBox( this, wxID_ANY );
		sizer->Add( mContextStack, 3, wxEXPAND | wxALL );

		// Setup data
		mData.fReset( new tScriptExportData( ) );

		if( !mData->fLoad( ) )
			tScriptExportDataParser::fBuild( *mData, this );

		fFillTree( );
		fReset( );
	}

	void tScriptDataDialog::fShow( const char* searchText )
	{
		Show( );

		if( !searchText || strlen( searchText ) <= 0)
		{
			currentSearchText = "";
			currentSearchIndex = 0;
		}
		else if( searchText == currentSearchText )
		{
			if( currentSearchIndex >= currentSearchResults.fCount( ) )
			{
				wxMessageBox("No results left.", "No results left.");

				currentSearchText = "";
				currentSearchIndex = 0;

				return;
			}

			wxTreeItemId node = currentSearchResults[currentSearchIndex++];
			mTree->SelectItem( node);
			mTree->SetFocus( );			// This will highlight the current item in blue as if the user clicked on it
			mTree->Expand( node );
		}
		else
		{
			currentSearchResults.fSetCount( 0 );

			tScriptExportDataParser::fFindInTree( searchText, mTree, mTree->GetRootItem( ), currentSearchResults );
			if( currentSearchResults.fCount( ) )
			{
				currentSearchText = searchText;
				currentSearchIndex = 0;

				wxTreeItemId node = currentSearchResults[currentSearchIndex++];
				mTree->SelectItem( node);
				mTree->SetFocus( );			// This will highlight the current item in blue as if the user clicked on it
				mTree->Expand( node );
			}
			else
				wxMessageBox("No results found.", "No results found.");		
		}
	}

	void tScriptDataDialog::fRebuild( wxCommandEvent& event )
	{
		mData.fReset( new tScriptExportData( ) );
		tScriptExportDataParser::fBuild( *mData, this );

		fFillTree( );
		fReset( );
	}

	void tScriptDataDialog::fFillTree( )
	{
		SetLabel( "Browser" );
		tScriptExportDataParser::fFillTree( *mData, mTree );
	}

	void tScriptDataDialog::fPushContext( const tContextAction& action, u32 charPos, u32 delLength )
	{
		mStack.fPushContext( action, charPos, delLength );
		fFillList( );
	}

	void tScriptDataDialog::fEvaluateControlChar( char character )
	{
		std::string symbolTxt = fTextAfterDot( );

		tSymbol* symbol = mStack.fFindSymbol( symbolTxt );

		fPushContext( mStack.fNewContextFromSymbol( symbol, character ), mTextBox->GetValue( ).length( ), 1 );
	}

	void tScriptDataDialog::fOnKeyDown( wxKeyEvent& event )
	{
		if( event.GetKeyCode( ) == WXK_UP )
		{
			fIncrementListSelection( -1 );
			return;
		}
		else if( event.GetKeyCode( ) == WXK_DOWN )
		{
			fIncrementListSelection( 1 );
			return;
		}
		else if( event.GetKeyCode( ) == WXK_TAB )
		{
			tSymbol* mem = fGetSelectedListSymbol( );
			if( mem )
			{
				fReplaceTextAfterDot( mem->mName );
			}

			return;
		}
		else if( event.GetKeyCode() == WXK_F12 )
		{
			fShow( currentSearchText );
			SetFocus( );
		}

		event.Skip( );
	}

	void tScriptDataDialog::fOnKeyUp( wxKeyEvent& event )
	{
		event.Skip( );
	}

	void tScriptDataDialog::fOnKeyChar( wxKeyEvent& event )
	{
		char key = event.GetKeyCode( );

		if( mStack.fControlChars( ).fFind( key ) )
		{
			fEvaluateControlChar( (char)event.GetKeyCode( ) );
		}

		event.Skip( );
	}

	void tScriptDataDialog::fOnTextChanged( wxCommandEvent& event )
	{
		if( mTextBox->GetValue( ).length( ) == 0 )
			fReset( );

		fFillList( );

		event.Skip( );
	}

	void tScriptDataDialog::fOnCursorMove( wxMouseEvent& event )
	{
		mStack.fSetContextPosition( mTextBox->GetInsertionPoint( ) - 1 );
		fFillList( );
		event.Skip( );
	}

	void tScriptDataDialog::fIncrementListSelection( int val )
	{
		int select = mList->GetSelection( );
		if( val == -1 && select > 0 )
		{
			--select;
		}
		else if( val == 1 && select < (s32)mList->GetCount( ) - 1 && mList->GetCount( ) )
		{
			++select;
		}
		mList->SetSelection( select );
		mLastSelectedSymbol = fGetSelectedListSymbol( );
	}

	tSymbol* tScriptDataDialog::fGetSelectedListSymbol( )
	{
		if( mList->GetSelection( ) == -1 )
			return NULL;
		else
			return static_cast<tSymbol*>( mList->GetClientData( mList->GetSelection( ) ) );
	}

	void tScriptDataDialog::fFillList( )
	{
		const std::string substr = fTextAfterDot( );
		mLastSelectedSymbol = fGetSelectedListSymbol( );
		mList->Clear( );

		mStatus->SetLabel( mStack.fStatus( ) );
		mStack.fFillContextList( mContextStack );

		std::string subLower = StringUtil::fToLower( substr );
		u32 newIndex = ~0;

		for( u32 i = 0; i < mStack.fSymbols( ).fCount( ); ++i )
		{
			std::string memName = mStack.fSymbols( )[ i ].mName;
			std::string memLower = StringUtil::fToLower( memName );

			if( substr.length( ) == 0 || memLower.find( subLower ) != ~0 )
			{
				tSymbol* data = &mStack.fSymbols( )[ i ];
				if( data == mLastSelectedSymbol )
					newIndex = mList->GetCount( );
				mList->Append( memName, data );
			}
		}

		if( newIndex != ~0 )
		{
			mList->SetSelection( newIndex );
		}
		else
		{
			if( mList->GetCount( ) )
			{
				mList->SetSelection( 0 );
				mLastSelectedSymbol = fGetSelectedListSymbol( );
			}
			else
				mLastSelectedSymbol = NULL;
		}
	}

	void tScriptDataDialog::fReset( )
	{
		mStack.fReset( );
		mStack.fSetData( mData );
		fPushContext( tContextAction( ~0, cActionPush, new tNamespaceContext( mData->fFindNameSpace( "" ) ) ), 0, 0 );
		fPushContext( tContextAction( ~0, cActionPush, new tClassNamespaceContext( mData->fFindClass( "Entity" ), false ) ), 0, 0 );
		fPushContext( tContextAction( ~0, cActionPush, new tFunctionBodyContext( mData->fFindClass( "Entity" )->fFindMember( "OnEntityCreate" ) ) ), 0, 0 );
	}

	std::string tScriptDataDialog::fTextAfterDot( ) const
	{
		wxString value = mTextBox->GetValue( );

		static const char* controls[ ] = { ".", "=", ")", "(", "," };

		for( u32 i = 0; i < array_length( controls ); ++i )
			if( value.EndsWith( controls[ i ] ) )
				return "";

		for( u32 i = 0; i < array_length( controls ); ++i )
		{
			tGrowableArray<std::string> bits;
			StringUtil::fSplit( bits, value.c_str( ), controls[ i ], true );

			if( bits.fCount( ) )
				value = bits.fBack( );
		}

		return StringUtil::fEatWhiteSpace( value );
	}

	void tScriptDataDialog::fReplaceTextAfterDot( const std::string& text )
	{
		wxString value = mTextBox->GetValue( );

		static const char* controls[ ] = { ".", "=", ")", "(", "," };

		int dotPos = -1;
		
		for( u32 i = 0; i < array_length( controls ); ++i )
			dotPos = fMax( dotPos, (int)value.find_last_of( controls[ i ] ) );

		if( dotPos == -1 )
			value = "";
		else if( dotPos + 1 < (s32)value.Length( ) )
		{
			int whiteSpace = 0;
			for( whiteSpace = dotPos + 1; whiteSpace < (s32)value.Length( ); ++whiteSpace )
				if( value.at( whiteSpace ) != ' ' && value.at( whiteSpace ) != '\t' )
					break;
			whiteSpace -= dotPos + 1;
			wxString ws( value.begin( ) + dotPos + 1, whiteSpace );
			value.Remove( dotPos + 1 );
			value += ws;
		}

		value += text;
		int end =  value.length( );

		mTextBox->SetValue( value );
		mTextBox->SetSelection( end, end );
	}

	void tScriptDataDialog::fTreeSelect( wxCommandEvent& event )
	{
		if( mTree->GetSelection( ).IsOk( ) )
		{
			//fSetContext( dynamic_cast<tTreeData*>( mTree->GetItemData( mTree->GetSelection( ) ) ) );
		}
	}

} }
