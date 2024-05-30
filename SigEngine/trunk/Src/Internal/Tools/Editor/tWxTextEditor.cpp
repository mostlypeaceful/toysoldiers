//------------------------------------------------------------------------------
// \file tWxTextEditor.cpp - 18 Aug 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "ToolsPch.hpp"
#include "tWxTextEditor.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	static const wxString gSquirrelKeywords(
		_T("break case catch class clone continue ")
		_T("const default delegate delete else enum ")
		_T("foreach extends for function if in local ")
		_T("null resume return sigvars sigexport sigimport switch this throw ")
		_T("try typeof while parent yield constructor ")
		_T("vargc vargv instanceof true false static ") );

	enum tMarginTypes
	{
		cLineNumberMargin = 0,
		cFoldingMargin,
		cThirdMargin,
		cFourthMargin,
		cFifthMargin,

		cNumMargins
	};

	static const tFilePathPtr cDefaultScriptName( "untitled.nut" );
	static const tFilePathPtr cDefaultLocmlName( "untitled.locml" );
	static const wxString cSupportedFiles( "Squirrel files (*.nut)|*.nut|LOCML files (*.locml)|*.locml|GOAML files (*.goaml)|*.goaml" );
	static const u32 cLineMargin = 48;
	static const u32 cCharWidth = 8; // GetCharWidth says 6 but PointFromPosition and my math says 8.
	static const u32 cMinWidth = 800;

	tWxTextEditor::tWxTextEditor( 
		wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		const wxString& name )
		: wxStyledTextCtrl( parent, id, pos, size, style, name )
		, mParentPanel( parent )
		, mAbsoluteFilePath( cDefaultScriptName )
		, mHasUnsavedChanges( false )
		, mIsBrandNew( true )
		, mLastTimeStamp( 0 )
		, mPromptingOutsideChanges( false )
		, mFileType( cNumSupportedFiles )
	{
		// Set so that CharAdded can be used.
		SetModEventMask( wxSTC_MOD_INSERTTEXT | wxSTC_MOD_DELETETEXT );

		Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( tWxTextEditor::fOnRightClick ), NULL, this );
	}

	void tWxTextEditor::fConfigureForSquirrel( )
	{
		mFileType = cSquirrelFile;

		// Do common initialization.
		fSharedInit( );

		// Squirrel is pretty similar to C++.
		SetLexer( wxSTC_LEX_CPP );
		SetKeyWords( 0, gSquirrelKeywords );

		// Set up lexer-specific types.
		StyleSetSpec( wxSTC_C_DEFAULT,					"fore:#000000,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_COMMENT,					"fore:#008000,back:#FFFFFF,italic" );
		StyleSetSpec( wxSTC_C_COMMENTLINE,				"fore:#008000,back:#FFFFFF,italic,bold" );
		StyleSetSpec( wxSTC_C_COMMENTDOC,				"fore:#008000,back:#FFFFFF,italic,underline" );
		StyleSetSpec( wxSTC_C_NUMBER,					"fore:#000080,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_WORD,						"fore:#0000FF,back:#FFFFFF,bold" );
		StyleSetSpec( wxSTC_C_STRING,					"fore:#808080,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_CHARACTER,				"fore:#000000,italic,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_UUID,						"fore:#008000,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_PREPROCESSOR,				"fore:#0000FF,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_OPERATOR,					"fore:#000000,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_IDENTIFIER,				"fore:#000000,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_STRINGEOL,				"fore:#FF0000,italic,underline,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_VERBATIM,					"fore:#008000,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_REGEX,					"fore:#FF0000,italic,underline,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_COMMENTLINEDOC,			"fore:#008000,italic,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_WORD2,					"fore:#FF0000,italic,underline,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_COMMENTDOCKEYWORD,		"fore:#008000,back:#FFFFFF" );
		StyleSetSpec( wxSTC_C_COMMENTDOCKEYWORDERROR,	"fore:#FF0000,italic,underline,back:#FFFFFF" );

		return;
	}

	void tWxTextEditor::fConfigureForXml( )
	{
		mFileType = cLocmlFile;

		// Do common initialization.
		fSharedInit( );

		// XML lexer.
		SetLexer( wxSTC_LEX_XML );

		// Set XML colors.
        StyleSetSpec( wxSTC_H_DEFAULT,				"fore:#000000,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_TAG,					"fore:#0000FF,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_TAGUNKNOWN,			"fore:#0000FF,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_ATTRIBUTE,			"fore:#FF0000,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_ATTRIBUTEUNKNOWN,		"fore:#FF0000,bold,back:#FFFFFF");
        StyleSetSpec( wxSTC_H_NUMBER,				"fore:#000000,back:#FFFFFF");
        StyleSetSpec( wxSTC_H_DOUBLESTRING,			"fore:#777777,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_SINGLESTRING,			"fore:#777777,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_OTHER,				"fore:#0000FF,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_COMMENT,				"fore:#007700,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_ENTITY,				"fore:#FF0000,bold,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_TAGEND,				"fore:#0000FF,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_XMLSTART,				"fore:#0000FF,back:#FFFFFF" );
        StyleSetSpec( wxSTC_H_XMLEND,				"fore:#0000FF,back:#FFFFFF");
        StyleSetSpec( wxSTC_H_CDATA,				"fore:#FF0000,back:#FFFFFF" );

		SetStyleBits( GetStyleBitsNeeded( ) );
	}

	void tWxTextEditor::fNewDoc( )
	{
		if( fCheckForChangesAndSave( ) )
			return;

		ClearAll( );
	}

	b32 tWxTextEditor::fOpenDoc( )
	{
		if( fCheckForChangesAndSave( ) )
			return false;

		// Open browse dialog.
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mParentPanel, 
			"Open File",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "" ),
			cSupportedFiles,
			wxFD_OPEN ) );

		SetFocus( );

		if( openFileDialog->ShowModal( ) == wxID_OK )
			return fOpenDoc( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );

		return false;
	}

	b32 tWxTextEditor::fOpenDoc( const tFilePathPtr& filename )
	{
		if( fCheckForChangesAndSave( ) )
			return false;

		if( LoadFile( filename.fCStr( ) ) )
		{
			const b32 isLocml = StringUtil::fCheckExtension( filename.fCStr( ), ".locml" );
			const b32 isScript = StringUtil::fCheckExtension( filename.fCStr( ), ".nut" );
			const b32 isGoaml = StringUtil::fCheckExtension( filename.fCStr( ), ".goaml" );

			if( isLocml || isGoaml )
			{
				fConfigureForXml( );
			}
			else if( isScript )
			{
				fConfigureForSquirrel( );
			}
			else
				return false; // some unsupported type

			mIsBrandNew = false;
			mAbsoluteFilePath = filename;
			mLastTimeStamp = FileSystem::fGetLastModifiedTimeStamp( mAbsoluteFilePath );

			fValidateLineLengths( );

			return true;
		}

		return false;
	}

	b32 tWxTextEditor::fSaveDoc( )
	{
		if( fNeedsSaveAs( ) )
			return fSaveDocAs( );
		
		if( !Win32Util::fIsFileReadOnly( mAbsoluteFilePath ) || ToolsPaths::fPromptToCheckout( mAbsoluteFilePath ) )
		{
			if( SaveFile( mAbsoluteFilePath.fCStr( ) ) )
			{
				mIsBrandNew = false;
				mLastTimeStamp = FileSystem::fGetLastModifiedTimeStamp( mAbsoluteFilePath );
				return true;
			}
		}

		return false;
	}

	b32 tWxTextEditor::fSaveDocAs( )
	{
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mParentPanel, 
			"Save File As",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			StringUtil::fNameFromPath( mAbsoluteFilePath.fCStr( ) ),
			cSupportedFiles,
			wxFD_SAVE ) );

		SetFocus( );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			if( SaveFile( openFileDialog->GetPath( ).c_str( ) ) )
			{
				mIsBrandNew = false;
				mAbsoluteFilePath = tFilePathPtr( openFileDialog->GetPath( ).c_str( ) );
				mLastTimeStamp = FileSystem::fGetLastModifiedTimeStamp( mAbsoluteFilePath );
				return true;
			}
		}

		return false;
	}

	b32 tWxTextEditor::fNeedsSaveAs( ) const
	{
		return mIsBrandNew;
	}

	b32 tWxTextEditor::fCheckForChangesAndSave( )
	{
		if( !mHasUnsavedChanges )
			return false;
		
		const int result = wxMessageBox( "You have unsaved changes - would you like to save them before resetting?",
					  "Save Changes?", wxYES | wxNO | wxCANCEL | wxICON_WARNING );

		if(			result == wxYES )			{ if( !fSaveDoc( ) ) return true; }
		else if(	result == wxNO )			{ }
		else if(	result == wxCANCEL )		{ return true; }
		else									{ log_warning( "Unknown result returned from Message Box" ); }

		return false;
	}

	b32 tWxTextEditor::fCheckForOutsideChanges( )
	{
		b32 reloaded = false;

		if( mPromptingOutsideChanges )
			return reloaded;

		const u64 newTimeStamp = FileSystem::fGetLastModifiedTimeStamp( mAbsoluteFilePath );

		if( newTimeStamp == mLastTimeStamp )
			return reloaded;

		// Lock out any other attempt to check changes.
		mPromptingOutsideChanges = true;

		// Pop message box.
		std::string warningMessage( mAbsoluteFilePath.fCStr( ) );
		warningMessage += "\n\nThis file has been modified outside the editor.\nReload the file?";
		const int result = wxMessageBox( warningMessage,
			"Reload File?", wxYES | wxNO | wxICON_WARNING );

		// Open or don't.
		if( result == wxYES )
		{
			reloaded = LoadFile( mAbsoluteFilePath.fCStr( ) );
		}
		else if( result == wxNO ) { }
		else
		{ 
			log_warning( "Unknown result returned from Message Box" ); 
		}

		mLastTimeStamp = FileSystem::fGetLastModifiedTimeStamp( mAbsoluteFilePath );

		// Release lock.
		mPromptingOutsideChanges = false;

		return reloaded;
	}

	wxString tWxTextEditor::fGetDisplayName( ) const
	{ 
		std::string cleanName = StringUtil::fNameFromPath( mAbsoluteFilePath.fCStr( ) );
		if( mHasUnsavedChanges )
			cleanName += '*';
		return wxString( cleanName ); 
	}

	void tWxTextEditor::fFrameCaretIfNecessary( s32 currentLine )
	{
		if( currentLine == -1 )
			currentLine = LineFromPosition( GetCurrentPos( ) );

		// Center if the caret goes out of frame.
		s32 beginLine = GetFirstVisibleLine( );
		s32 endLine = beginLine + LinesOnScreen( );

		if( currentLine < beginLine || currentLine >= endLine )
			EnsureVisibleEnforcePolicy( currentLine );
	}

	void tWxTextEditor::fCommentSelection( )
	{
		s32 selectionStart = 0, selectionEnd = 0;
		GetSelection( &selectionStart, &selectionEnd );
		if( selectionStart == selectionEnd )
			return;

		s32 startLine = LineFromPosition( selectionStart );
		s32 endLine = LineFromPosition( selectionEnd );

		s32 minPos = std::numeric_limits<s32>::max( );
		for( s32 i = startLine; i <= endLine; ++i )
		{
			const s32 minPosForThisLine = GetLineIndentPosition( i ) - PositionFromLine( i );
			minPos = fMin( minPos, minPosForThisLine );
		}

		BeginUndoAction( );
		for( s32 i = startLine; i <= endLine; ++i )
		{
			const s32 insertPos = PositionFromLine( i ) + minPos;
			InsertText( insertPos, "//" );
			if( i != endLine || selectionEnd > insertPos )
				selectionEnd += 2;
		}
		EndUndoAction( );

		SetSelection( selectionStart, selectionEnd );
	}

	void tWxTextEditor::fUncommentSelection( )
	{
		s32 selectionStart = 0, selectionEnd = 0;
		GetSelection( &selectionStart, &selectionEnd );
		if( selectionStart == selectionEnd )
			return;

		s32 startLine = LineFromPosition( selectionStart );
		s32 endLine = LineFromPosition( selectionEnd );

		BeginUndoAction( );
		for( s32 i = startLine;i <= endLine; ++i )
			fDestroyCommentSlashes( i );
		EndUndoAction( );
	}

	void tWxTextEditor::fToUpper( )
	{
		int start, end;
		BeginUndoAction( );
			GetSelection( &start, &end );
			wxString text = GetTextRange( start, end );
			ReplaceSelection( text.Upper( ) );
		EndUndoAction( );

		SetSelection( start, end );
	}

	void tWxTextEditor::fSharedInit( )
	{
		// Initialize everything to a blank slate.
		StyleClearAll();
		wxFont defaultFont( 10, wxMODERN, wxNORMAL, wxNORMAL );
		for( u32 i = 0; i < wxSTC_STYLE_LASTPREDEFINED; ++i )
		{
			// Why isn't the font argument const?
			StyleSetFont( i, defaultFont );
			StyleSetForeground( i, "BLACK" );
			StyleSetBackground( i, "WHITE" );
		}

		// Configure line numbers.
		SetMarginType( cLineNumberMargin, wxSTC_MARGIN_NUMBER );
		StyleSetForeground( wxSTC_STYLE_LINENUMBER, "DARK GREY" );
		StyleSetBackground( wxSTC_STYLE_LINENUMBER, "WHITE" );
		SetMarginWidth( cLineNumberMargin, cLineMargin );

		// Set aux margins as invisible.
		for( u32 i = cFoldingMargin; i < cNumMargins; ++i )
		{
			SetMarginType( i, wxSTC_MARGIN_SYMBOL );
			SetMarginWidth( i, 0 );
			SetMarginSensitive( i, false );
		}

		// Set minimum width.
		SetScrollWidth( cMinWidth );

		// Tab sizes.
		SetTabWidth( 4 );

		// View the edge.
		SetEdgeColumn( 80 );
		SetEdgeMode( wxSTC_EDGE_LINE );

		// Cursor visibility. (Used only for find/replace cursor framing)
		SetVisiblePolicy( wxSTC_VISIBLE_STRICT, 0 );

		// Set up some general font styles.
		StyleSetSpec( wxSTC_STYLE_LINENUMBER,			"back:#C0C0C0" );
		StyleSetSpec( wxSTC_STYLE_BRACELIGHT,			"fore:#000000,back:#EEEEEE,bold" );
		StyleSetSpec( wxSTC_STYLE_BRACEBAD,				"fore:#FF0000,back:#FFFFFF,bold" );
		StyleSetForeground( wxSTC_STYLE_DEFAULT,		"DARK GREY" );
		StyleSetForeground( wxSTC_STYLE_INDENTGUIDE,	"DARK GREY" );
	}

	void tWxTextEditor::fDestroyCommentSlashes( s32 line )
	{
		s32 pos = GetLineIndentPosition( line );
		const s32 endLine = GetLineEndPosition( line );

		for( ; pos < endLine; ++pos )
		{
			if( GetCharAt( pos ) == '/' && GetCharAt( pos+1 ) == '/' )
			{
				SetTargetStart( pos );
				SetTargetEnd( pos+2 );
				ReplaceTarget( "" );
				break;
			}

			if( GetCharAt( pos ) != ' ' && GetCharAt( pos ) != '	' )
				break;
		}
	}

	s32 tWxTextEditor::fFindOpenBraceBack( s32 searchPos, s32 bracketHandicap )
	{
		s32 openBrackets = bracketHandicap; // Skip the bracket that exists right at the start of a line.
		for( s32 i = searchPos; i >= 0; --i )
		{
			const s32 charAt = GetCharAt( i );
			if( charAt == '{' )
			{
				if( openBrackets == 0 )
					return i;
				else
					--openBrackets;
			}
			else if( charAt == '}' )
				++openBrackets;
		}

		return wxSTC_INVALID_POSITION;
	}

	void tWxTextEditor::fSavepointReached( wxStyledTextEvent& event )
	{
		mHasUnsavedChanges = false;
		event.Skip( );
	}

	void tWxTextEditor::fSavepointLeft( wxStyledTextEvent& event )
	{
		mHasUnsavedChanges = true;
		event.Skip( );
	}

	void tWxTextEditor::fModified( wxStyledTextEvent& event )
	{
		fValidateLineLengths( );
	}

	void tWxTextEditor::fCharacterAdded( wxStyledTextEvent& event )
	{
		const s32 keyId = event.GetKey( );
		if( keyId == WXK_RETURN )
		{
			// currentLine is the line the cursor is on after the character is added.
			const u32 currentLine = GetCurrentLine( );
			const u32 currentIndent = GetLineIndentation( currentLine );
			const u32 previousIndent = GetLineIndentation( currentLine-1 );

			u32 totalIndent = previousIndent+currentIndent;
			if( GetCharAt( GetLineEndPosition( currentLine-1 ) - 1 ) == '{' )
				totalIndent += GetTabWidth( );

			SetLineIndentation( currentLine, totalIndent );
			GotoPos( GetLineIndentPosition( currentLine ) );
		}

		if( keyId == '}' )
		{
			// Move
			const s32 foundPos = fFindOpenBraceBack( GetLineIndentPosition( GetCurrentLine( ) ) );
			if( foundPos == wxSTC_INVALID_POSITION )
				return;

			const u32 currentLine = GetCurrentLine( );
			SetLineIndentation( currentLine, GetLineIndentation( LineFromPosition( foundPos ) ) );
		}
	}

	void tWxTextEditor::fUpdateUI( wxStyledTextEvent& event )
	{
		const s32 foundBracket = fFindOpenBraceBack( GetCurrentPos( )-1, 0 );
		const s32 matchingBracket = BraceMatch( foundBracket );
		if( matchingBracket == -1 )
		{
			BraceBadLight( foundBracket );
			return;
		}

		BraceHighlight( foundBracket, matchingBracket );
	}

	void tWxTextEditor::fValidateLineLengths( )
	{
		fValidateLineLengths( 0, GetLineCount( )-1 );
	}

	void tWxTextEditor::fValidateLineLengths( s32 startLine, s32 endLine )
	{
		sigassert( startLine <= endLine && startLine < GetLineCount( ) && endLine < GetLineCount( ) );

		s32 longestLen = 0;
		s32 longestLine = -1;
		for( s32 i = startLine; i <= endLine; ++i )
		{
			const s32 thisLen = LineLength( i );
			if( longestLen < thisLen )
			{
				longestLen = LineLength( i );
				longestLine = i;
			}
		}

		if( longestLine != -1 )
		{
			if( mWidestLineNum != longestLine || mWidestLineCount != longestLen )
			{
				mWidestLineCount = longestLen;
				mWidestLineNum = longestLine;
			}
		}

		const u32 width = cCharWidth * mWidestLineCount + cLineMargin;
		SetScrollWidth( (width < cMinWidth) ? cMinWidth : width );
	}

	void tWxTextEditor::fOnRightClick( wxMouseEvent& event )
	{
		// Pass this up at least one level explicitly. Mouse events aren't propagated
		// upward by default so it has to be forced.
		event.Skip( );
		event.ResumePropagation( 1 );
	}



	BEGIN_EVENT_TABLE(tWxTextEditor, wxStyledTextCtrl)
		EVT_STC_SAVEPOINTREACHED(	wxID_ANY,	tWxTextEditor::fSavepointReached )
		EVT_STC_SAVEPOINTLEFT(		wxID_ANY,	tWxTextEditor::fSavepointLeft )
		EVT_STC_MODIFIED(			wxID_ANY,	tWxTextEditor::fModified )
		EVT_STC_CHARADDED(			wxID_ANY,	tWxTextEditor::fCharacterAdded )
		EVT_STC_UPDATEUI(			wxID_ANY,	tWxTextEditor::fUpdateUI )
	END_EVENT_TABLE()

}
