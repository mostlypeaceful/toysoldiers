//------------------------------------------------------------------------------
// \file tWxTextEditor.hpp - 18 Aug 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tWxTextEditor__
#define __tWxTextEditor__
#include "tStrongPtr.hpp"
#include <wx/stc/stc.h>

namespace Sig
{
	enum tFileType
	{
		cSquirrelFile,
		cLocmlFile,

		cNumSupportedFiles,
	};


	/// To use this object you must link with: Release( wxmsw28_stc.lib, wxmsw28_aui.lib ), Debug( wxmsw28d_stc.lib, wxmsw28d_aui.lib )
	///  for the wxStyledTextCtrl
	class tools_export tWxTextEditor : public wxStyledTextCtrl
	{
		wxWindow*		mParentPanel;
		tFilePathPtr	mAbsoluteFilePath;
		b32				mHasUnsavedChanges;
		b32				mIsBrandNew;
		u32				mWidestLineCount;
		u32				mWidestLineNum;

		u64				mLastTimeStamp;
		b32				mPromptingOutsideChanges;
		tFileType		mFileType;

	public:
		tWxTextEditor( 
			wxWindow* parent,
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxString( "Default Text Editor Control" ) );

		void fConfigureForSquirrel( );
		void fConfigureForXml( );

		b32 fIsLocml( ) const { return mFileType == cLocmlFile; }
		b32 fIsScript( ) const { return mFileType == cSquirrelFile; }

		void fNewDoc( );
		b32 fOpenDoc( );
		b32 fOpenDoc( const tFilePathPtr& filename );
		b32 fSaveDoc( );
		b32 fSaveDocAs( );

		b32 fNeedsSave( ) const { return mHasUnsavedChanges; }
		b32 fNeedsSaveAs( ) const;

		///
		/// \brief
		/// Checks for changes in the editor. If so, tries to save. Returns true
		/// if the event this is being called from should be canceled (vetoed).
		b32 fCheckForChangesAndSave( );

		// Returns true if reloaded new changes
		b32 fCheckForOutsideChanges( );

		wxString fGetDisplayName( ) const;
		tFilePathPtr fGetFilePath( ) const { return mAbsoluteFilePath; }

		/// 
		/// \brief This will center the view on the caret if it is outside the frame.
		/// -1 means calculate the current line automatically.
		void fFrameCaretIfNecessary( s32 currentLine = -1 );

		/// 
		/// \brief
		/// Prepends (or removes) comment slashes to each line that is selected.
		void fCommentSelection( );
		void fUncommentSelection( );

		// Converts selection to upper case.
		void fToUpper( );

	private:
		void fSharedInit( );

		void fDestroyCommentSlashes( s32 line );
		s32 fFindOpenBraceBack( s32 searchPos, s32 bracketHandicap = -1 );

		void fSavepointReached( wxStyledTextEvent& event );
		void fSavepointLeft( wxStyledTextEvent& event );
		void fModified( wxStyledTextEvent& event );
		void fCharacterAdded( wxStyledTextEvent& event );
		void fUpdateUI( wxStyledTextEvent& event );

		void fValidateLineLengths( );
		void fValidateLineLengths( s32 startLine, s32 endLine );

		void fOnRightClick( wxMouseEvent& event );

		DECLARE_EVENT_TABLE()
	};

}

#endif//__tWxTextEditor__
