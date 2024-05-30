//------------------------------------------------------------------------------
// \file tScriptNotebook.hpp - 18 Aug 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tScriptNotebook__
#define __tScriptNotebook__
#include <wx/aui/auibook.h>
#include "tSigScriptWindow.hpp"

class wxStyledTextEvent;

namespace Sig
{
	class tWxTextEditor;
	class tWxScriptDesigner;
	class tScriptNotebook;
	class tLocmlConvertDictionary;

	class tScriptNotebookPage : public wxPanel
	{
		tScriptNotebook* mParent;
		tWxTextEditor* mEditorView;
		tWxScriptDesigner* mDesignerPanel;

		b32 mInTextView;

	public:
		tScriptNotebookPage( tScriptNotebook* parentNotebook, b32 advancedMode = true );

		void fToggleView( );
		void fSwitchToText( );
		void fSwitchToDesigner( );

		b32 fGetInTextEdit( ) const { return mInTextView; }

		/// 
		/// \brief Frames the view at the selected line and column. If no column
		/// is specified, the cursor will be placed at the end of indentation white space.
		void fGoToLine( s32 lineNum, s32 colNum = -1 );

		void fSelectCurrentWord( );

		tWxTextEditor* fGetEditor( );

		///
		/// Interface required from tWxTextEditor.
		void			fNewDoc( );

		b32				fOpenDoc( );
		b32				fOpenDoc( const tFilePathPtr& filename );
		b32				fSaveDoc( );
		b32				fSaveDocAs( );

		b32				fNeedsSave( ) const;
		b32				fNeedsSaveAs( ) const;

		b32				fCheckForChangesAndSave( );

		void			fCheckForOutsideChanges( );

		wxString		fGetDisplayName( ) const;
		tFilePathPtr	fGetFilePath( ) const;

		void			fUndo( );
		void			fRedo( );

		void			fRefreshDesigner( );
		void			fOnRightClick( wxMouseEvent& event );
		void			fOnAction( wxCommandEvent& event );

		wxString		fGetSelectedText( );

		void			fConvertLocmlText( const tLocmlConvertDictionary& dict, std::string lang );

	private:
		s32				fReplaceText( const tLocmlConvertDictionary& dict, std::string lang );
	};

	/// 
	/// \brief
	/// Custom notebook for viewing our scripts. Some special interfaces for script browser.
	class tScriptNotebook : public wxAuiNotebook
	{
		Win32Util::tRecentlyOpenedFileList* mRecentFiles;

		tEditorContextActionList mTabContextActions;
		tEditorContextActionList mTextContextActions;

		b32 mTextEditSet;
		b32 mSuspendOutsideCheck;

		s32 mRightClickTab;

	public:
		tScriptNotebook( wxWindow* parent, Win32Util::tRecentlyOpenedFileList* recentFiles );

		b32 fOnNotebookClose( );

		void fNewDoc( );

		u32 fOpenDoc( );
		u32 fOpenDoc( const tFilePathPtr& file );
		void fOpenAndFocus( const tFilePathPtr& file, u32 lineNum, u32 colNum );
		u32 fOpenOrFind( const tFilePathPtr& file );

		u32 fFind( const tFilePathPtr& cleanName );

		void fSaveAll( );
		void fSaveCurrent( );
		void fSaveCurrentAs( );

		void fCloseCurrent( );
		void fCloseAll( tScriptNotebookPage* skipPage = NULL );
		void fCloseAny( const tFilePathPtr& fileToClose, tScriptNotebookPage* skipPage );

		void fUndo( );
		void fRedo( );

		tScriptNotebookPage* fGetCurrent( );
		tScriptNotebookPage* fGetPage( u32 i );

		void fCheckChanges( );

		void fToggleViewMode( );

		void fFocusAllOnLine( u64 lineNum );

		void fSetContextActions( const tEditorContextActionList& tabs, const tEditorContextActionList& texts );
		tEditorContextActionList& fTabContextActions( ) { return mTabContextActions; }
		tEditorContextActionList& fTextContextActions( ) { return mTextContextActions; }

		s32 fRightClickTab( ) { return mRightClickTab; }
		tFilePathPtr fRightClickFileName( );

	private:
		u32 fOpenAndCreate( );
		u32 fOpenAndCreate( const tFilePathPtr& file );

		void fOnPageClose( wxAuiNotebookEvent& event );
		void fOnSavepointChange( wxStyledTextEvent& event );

		void fOnTabRightClick( wxAuiNotebookEvent& event );
		void fOnAction( wxCommandEvent& event );

		DECLARE_EVENT_TABLE()
	};
}

#endif//__tScriptNotebook__
