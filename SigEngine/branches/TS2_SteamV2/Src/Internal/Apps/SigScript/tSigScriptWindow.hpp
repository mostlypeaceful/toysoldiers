#ifndef __tSigScriptWindow__
#define __tSigScriptWindow__
#include "tWxSavedLayout.hpp"
#include "Editor/tEditorContextAction.hpp"
#include "tScriptExportDialog.hpp"
#include "tScriptProfilerDialog.hpp"


class wxAuiNotebookEvent;
class wxAuiManagerEvent;

namespace Sig
{
	class tScriptNotebook;
	class tFindInFilesDialog;
	class tScriptBrowser;
	class tCheckChangesTimer;
	class tOnTickTimer;
	class tSearchableOpenFilesDialog;

	///
	/// \brief Derived registry serializer for saving the layout of a tEditorAppWindow.
	class tSigScriptWindowSavedLayout : public tWxSavedLayout
	{
	public:
		tSigScriptWindowSavedLayout( const std::string& regKeyName ) : tWxSavedLayout( regKeyName ) { }
	};

	/// 
	/// \brief Encapsulates the SigScript editor.
	class tSigScriptWindow : public wxFrame, public Win32Util::tRegistrySerializer
	{
		wxToolBar*				mToolbar;
		tScriptNotebook*		mScriptNotebook;
		wxBoxSizer*				mMainSizer;
		tFindInFilesDialog*		mFindInFilesDialog;
		tScriptBrowser*			mBrowser;
		std::string				mRegKeyName;
		Win32Util::tRecentlyOpenedFileList mRecentFiles;
		wxMenu*					mRecentFilesMenu;
		u32						mBaseRecentFileActionId;
		tSigScriptWindowSavedLayout mSavedLayout;
		tCheckChangesTimer*		mChangesTimer;
		tOnTickTimer*			mOnTickTimer;
		tSearchableOpenFilesDialog* mOpenFiles;
		tEditorContextActionList mContextActions;
		ScriptData::tScriptDataDialog*		mScriptDataDialog;
		ScriptData::tScriptProfilerDialog*	mScriptProfilerDialog;

	public:
		tSigScriptWindow(const wxString& title);
		~tSigScriptWindow( );
		void fOnClose(wxCloseEvent& event);
		void fOnAbout(wxCommandEvent& event);
		void fOnQuit(wxCommandEvent& event);

		void fOpenDoc( const tFilePathPtr& path );

		// WARNING: Does not ask to save.
		void fClearAll( );

		void fFocusAllOnLine( u64 lineNum );

		tEditorContextActionList& fContextActions( ) { return mContextActions; }

		wxToolBar* fToolbar( ) { return mToolbar; }
		tScriptNotebook* fNotebook( ) { return mScriptNotebook; }

		wxPoint fGetCenter( const wxSize& childSize )
		{
			wxPoint windowPos = GetPosition( );
			wxSize windowSize = GetSize( );
			windowPos.x += windowSize.x / 2.f;
			windowPos.y += windowSize.y / 2.f;

			windowPos.x -= childSize.x / 2.f;
			windowPos.y -= childSize.y / 2.f;
			
			return windowPos;
		}

	private:
		std::string fRegistryKeyName( ) const { return mRegKeyName; }
		void fOnAction(wxCommandEvent& event);
		void fUpdateRecentFileMenu( );

		void fSaveInternal( HKEY hKey );
		void fLoadInternal( HKEY hKey );

	public:
		void fSaveLayout( );
	private:
		void fLoadLayout( );
		void fShowPerformanceBrowser( );

		void fOnPageChange(wxAuiNotebookEvent& event);
		void fOnFilesDropped(wxDropFilesEvent& event);

		DECLARE_EVENT_TABLE()
	};
}

#endif//__tSigScriptWindow__

