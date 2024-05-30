#ifndef __tSigAIMainWindow__
#define __tSigAIMainWindow__
#include "tWxSavedLayout.hpp"
#include "Editor/tEditorAction.hpp"
#include "tSigAINodeCanvas.hpp"
#include "tMatEdMainWindow.hpp"

namespace Sig
{
	class tSigAIControlPanel;
	class tGoamlBrowser;
	class tSigAIExplicitDependenciesDialog;

	class tSigAIMainWindow : public wxFrame
	{
	public:
		static const u32 cMaxRecentlyOpenedFiles = 8;
		static const char cNewDocTitle[];
	private:
		tWxSavedLayout mSavedLayout;
		wxToolBar* mToolBar;
		tSigAINodeCanvas* mCanvas;
		tSigAIControlPanel* mControlPanel;
		tGoamlBrowser* mBrowser;
		wxMenu* mRecentFilesMenu;
		std::string	mDocName;
		Win32Util::tRecentlyOpenedFileList mRecentFiles;
		tEditorActionStack::tNotifyEvent::tObserver mOnDirty, mOnAddAction;
		tDAGNodeCanvas::tOnSelectionChanged::tObserver mOnSelChanged;
		tSigAIExplicitDependenciesDialog* mExplicitDependenciesDialog;
	public:
		tSigAIMainWindow( );
		~tSigAIMainWindow( );
		void fSetStatus( const char* status );
		void fViewScript( );
		tSigAINodeCanvas* fCanvas( ) { return mCanvas; }
		void fOpenDoc( const tFilePathPtr& file );
		tEditorActionStack& fEditorAction( ) { return mCanvas->fEditorActions( ); }
	protected:
		void fLoadLayout( );
		void fSaveLayout( );
		void fOnDirty( tEditorActionStack& stack );
		void fOnActionUndoOrRedo( tEditorActionStack& stack );
	private:
		void fCreateMainMenu( );
		void fAddToolbar( );
		void fOnClose( wxCloseEvent& event );
		void fOnAction( wxCommandEvent& event );
		void fOnIdle( wxIdleEvent& event );
		void fOnSelChanged( );
	protected: // doc management
		std::string fMakeWindowTitle( ) const;
		b32 fClearScene( );
		void fNewDoc( );
		b32 fSaveDoc( b32 saveAs );
		void fOpenDoc( );
		void fOpenRecent( u32 ithRecentFile );
		void fAddRecentFile( const tFilePathPtr& path );
		void fUpdateRecentFileMenu( );
		void fChooseMoMap( );
		void fRefreshMoMap( );
		void fClearMoMap( );
		b32 fBuild( );
		void fViewSnippets( );
		void fFind( );
		void fReplace( );
		void fEditProjectSettings( );
		b32 fSerialize( const tFilePathPtr& path );
		b32 fDeserialize( const tFilePathPtr& path );
	};
}

#endif//__tSigAIMainWindow__
