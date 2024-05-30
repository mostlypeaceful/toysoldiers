#ifndef __tShadeMainWindow__
#define __tShadeMainWindow__
#include "tWxSavedLayout.hpp"
#include "Editor/tEditorAction.hpp"
#include "tShadeNodeCanvas.hpp"
#include "tMatEdMainWindow.hpp"

namespace Sig
{
	class tShadeControlPanel;

	class tShadeMainWindow : public wxFrame
	{
	public:
		static const u32 cMaxRecentlyOpenedFiles = 8;
		static const char cNewDocTitle[];
	private:
		tWxSavedLayout mSavedLayout;
		wxToolBar* mToolBar;
		wxChoice* mVsStyle;
		wxChoice* mHlslPreviewPlatform;
		tShadeNodeCanvas* mCanvas;
		tShadeControlPanel* mControlPanel;
		tMatEdMainWindow* mMatEd;
		tMaterialPreviewBundlePtr mMatPreview;
		wxMenu* mRecentFilesMenu;
		std::string	mDocName;
		Win32Util::tRecentlyOpenedFileList mRecentFiles;
		tEditorActionStack::tNotifyEvent::tObserver mOnDirty, mOnAddAction;
		tDAGNodeCanvas::tOnSelectionChanged::tObserver mOnSelChanged;
	public:
		tShadeMainWindow( );
		~tShadeMainWindow( );
		void fSetStatus( const char* status );
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
		void fOnGeomStyleChanged( wxCommandEvent& event );
		void fOnIdle( wxIdleEvent& event );
		void fOnSelChanged( );
	protected: // doc management
		std::string fMakeWindowTitle( ) const;
		b32 fClearScene( );
		void fNewDoc( );
		b32 fSaveDoc( b32 saveAs );
		void fOpenDoc( );
		void fOpenDoc( const tFilePathPtr& file );
		void fOpenRecent( u32 ithRecentFile );
		void fAddRecentFile( const tFilePathPtr& path );
		void fUpdateRecentFileMenu( );
		void fBuild( );
		void fSerialize( const tFilePathPtr& path );
		b32 fDeserialize( const tFilePathPtr& path );
		void fSyncMatEd( );
		void fShowHlsl( );
	};
}

#endif//__tShadeMainWindow__
