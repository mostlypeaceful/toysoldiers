#include "SigAIPch.hpp"
#include "tFindInSnippetsDialog.hpp"
#include "tSigAIMainWindow.hpp"
#include "tWxColumnListBox.hpp"
#include "tWxTextEditor.hpp"
#include "FileSystem.hpp"
#include "tEditablePropertyTypes.hpp"


namespace Sig
{
	//////////////////////////////////////////////////////////////////////////
	// Dialog section.
	//////////////////////////////////////////////////////////////////////////

	enum tLocation
	{
		cNodeID,
		cHandlerID,
		cNodeName,
		cHandlerName
	};

	tFindInSnippetsDialog::tFindInSnippetsDialog( tSigAIMainWindow* parent )
		: tFindInDialogBase( parent, "Snippets", false )
		, mParent( parent )
	{
	}

	void tFindInSnippetsDialog::fFindNext( b32 searchUp )
	{

	}

	tDynamicBuffer fToBuffer( const std::string& str )
	{
		tDynamicBuffer ret;
		ret.fInsert( 0, (byte*)str.c_str( ), str.length( ) );
		ret.fPushBack( 0 );
		return ret;
	}

	void tFindInSnippetsDialog::fFindInFiles( const wxString& searchText, tGrowableArray< tOccurence >& occurences, b32 findOneOccurencePerFile )
	{
		tSigAINodeCanvas* canvas = mParent->fCanvas( );

		for( u32 i = 0; i < canvas->fAllNodes( ).fCount( ); ++i )
		{
			tGoalAINode* node = dynamic_cast<tGoalAINode*>( canvas->fAllNodes( )[ i ].fGetRawPtr( ) );
			sigassert( node );

			//search properties
			tEditablePropertyTable& table = node->fAIProps( );
			tEditablePropertyTable::tIteratorNoNullOrRemoved it( table.fBegin( ), table.fEnd( ) );
			for( ; it.fNotDone( ); ++it )
			{
				std::string value;
				tEditableProperty* p = it->mValue.fGetRawPtr( );

				if( dynamic_cast< tEditablePropertyScriptString* >(p) || dynamic_cast< tEditablePropertyString* >(p) || dynamic_cast< tEditablePropertyCustomString* >(p) )
				{
					std::string script;
					p->fGetData( script );

					std::string location = StringUtil::fToString( i ) + "\\" + StringUtil::fToString( -1 ) + "\\" + node->fDisplayableName( ) + "\\" + it->mValue->fGetName( );
					fFindInFile( searchText, fToBuffer( script ), tFilePathPtr( location ), occurences, findOneOccurencePerFile );
				}
			}

			//search handlers
			for( u32 h = 0; h < node->fHandlerCount( ); ++h )
			{
				tGoalEventHandler* handler = node->fHandler( h );

				if( handler->fScriptValid( ) )
				{
					std::string location = StringUtil::fToString( i ) + "\\" + StringUtil::fToString( h ) + "\\" + node->fDisplayableName( ) + "\\" + handler->fOutputName( ) + " ";
					std::string script = handler->fScript( );

					fFindInFile( searchText, fToBuffer( script ), tFilePathPtr( location ), occurences, findOneOccurencePerFile );
				}
			}
		}
	}

	void tFindInSnippetsDialog::fProcessEnter( wxCommandEvent& event )
	{
		// If we're searching for everything, find it.
		if( mComboBox->GetSelection( ) == 0 )
			fOnSearchAllPressed( event );

		// The replace window isn't supposed to respond to Enter.
	}

	void tFindInSnippetsDialog::fOnSearchNextPressed( wxCommandEvent& event )
	{

	}

	void tFindInSnippetsDialog::fOnReplacePressed( wxCommandEvent& event )
	{

	}

	void tFindInSnippetsDialog::fOnReplaceAllPressed( wxCommandEvent& event )
	{
		if( mSearchText->IsEmpty( ) )
			return;

		const wxString searchText = mSearchText->GetLineText( 0 );
		u32 replaceLen = searchText.length( );
		const wxString replacementText = mReplacementText->GetLineText( 0 );

		tGrowableArray< tOccurence > occurences;
		fFindInFiles( searchText, occurences, false );

		tEditorActionPtr mainAction;
		u32 replacements = 0;

		for( u32 i = 0; i < occurences.fCount( ); ++i )
		{
			const tOccurence& thisOcc = occurences[ i ];

			tGrowableArray< std::string > strs;
			StringUtil::fSplit( strs, thisOcc.mFile.fCStr( ), "\\" );

			s32 node = atoi( strs[ cNodeID ].c_str( ) );
			s32 handler = atoi( strs[ cHandlerID ].c_str( ) );

			if( node < s32(mParent->fCanvas( )->fAllNodes( ).fCount( )) )
			{
				tGoalAINode* goal = dynamic_cast<tGoalAINode*>( mParent->fCanvas( )->fAllNodes( )[ node ].fGetRawPtr( ) );
				if( handler == -1 )
				{
					// this is a property.
					std::string value;
					(*goal->fAIProps( ).fFind( strs[ cHandlerName ] ))->fGetData( value );
					thisOcc.fReplaceString( value, replaceLen, std::string( replacementText ) );
					++replacements;

					tEditorActionPtr action = tEditorActionPtr( new tEditPropertyAction( *mParent->fCanvas( ), tGoalAINodePtr( goal ), strs[ cHandlerName ], value ) );
					if( !mainAction )
					{
						mParent->fCanvas( )->fEditorActions( ).fBeginCompoundAction( );
						mainAction = action;
					}
					else
						mParent->fCanvas( )->fEditorActions( ).fAddAction( action );
				}
				else if( s32(goal->fHandlerCount( )) )
				{
					std::string value = goal->fHandler( handler )->fScript( );
					thisOcc.fReplaceString( value, replaceLen, std::string( replacementText ) );
					++replacements;

					tEditorActionPtr action = tEditorActionPtr( new tScriptChangeAction( *mParent->fCanvas( ), tGoalEventHandlerPtr( goal->fHandler( handler ) ), value ) );
					if( !mainAction )
					{
						mParent->fCanvas( )->fEditorActions( ).fBeginCompoundAction( );
						mainAction = action;
					}
					else
						mParent->fCanvas( )->fEditorActions( ).fAddAction( action );
				}
			}	
		}

		if( mainAction )
		{
			mainAction->fSetTitle( StringUtil::fToString( replacements ) + " replacements" );
			mParent->fCanvas( )->fEditorActions( ).fEndCompoundAction( mainAction );
		}

		mParent->fCanvas( )->Refresh( );

		wxMessageBox( StringUtil::fToString( replacements ) + " replacements made.", "Finished." );
	}

	const char* tFindInSnippetsDialog::fFindStr( const char* searchIn, const char* searchFor )
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

	void tFindInSnippetsDialog::fSelectItem( )
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

			tGrowableArray< std::string > strs;
			StringUtil::fSplit( strs, thisOcc.mFile.fCStr( ), "\\" );

			s32 node = atoi( strs[ cNodeID ].c_str( ) );
			s32 handler = atoi( strs[ cHandlerID ].c_str( ) );

			if( node < s32(mParent->fCanvas( )->fAllNodes( ).fCount( )) )
			{
				tGoalAINode* goal = dynamic_cast<tGoalAINode*>( mParent->fCanvas( )->fAllNodes( )[ node ].fGetRawPtr( ) );
				if( handler == -1 )
				{
					// this is a property.
					mParent->fCanvas( )->fSelectSingleNode( tDAGNodePtr( goal ) );
					mParent->fCanvas( )->fFrame( true );
				}
				else if( handler < s32(goal->fHandlerCount( )) )
				{
					mParent->fCanvas( )->fEditHandler( tGoalEventHandlerPtr( goal->fHandler( handler ) ), thisOcc.mLineNum, thisOcc.mColNum );
				}
			}
		}
	}
}
