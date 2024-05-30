#ifndef __tSigAnimDialog__
#define __tSigAnimDialog__
#include "tWxSavedLayout.hpp"
#include "Editor/tEditorAction.hpp"
#include "tSigAnimNodeCanvas.hpp"
#include "Momap.hpp"

namespace Sig
{

	class tSigAnimNodeCanvas;
	class tSigAnimNodeControlPanel;
	class tAnimapEditor;
	class tSkeletableSgFileRefEntity;

	namespace Anim { class tSigAnimMoMap; }

	class toolsgui_export tSigAnimDialog : public wxFrame
	{
	public:
		static const u32 cMaxRecentlyOpenedFiles = 8;
		static const char cNewDocTitle[];
	private:
		tWxSavedLayout mSavedLayout;
		wxToolBar* mToolBar;
		tSigAnimNodeCanvas* mCanvas;
		tSigAnimNodeControlPanel* mControlPanel;
		wxMenu* mRecentFilesMenu;
		tAnimapEditor* mAnimapEditor;
		tRefCounterPtr<tSkeletableSgFileRefEntity> mCurrentlyEditing;
		//Momap::tContextData mContextData;

		std::string	mDocName;
		Win32Util::tRecentlyOpenedFileList mRecentFiles;
		tEditorActionStack::tNotifyEvent::tObserver mOnDirty, mOnAddAction;
		tDAGNodeCanvas::tOnSelectionChanged::tObserver mOnSelChanged;

	public:
		tSigAnimDialog( );
		~tSigAnimDialog( );

		tSigAnimNodeControlPanel* fControlPanel( ) { return mControlPanel; }
		tSigAnimNodeCanvas* fCanvas( ) { return mCanvas; }
		tAnimapEditor* fAnimapEditor( ) { return mAnimapEditor; }
		//Momap::tContextData& fContextData( ) { return mContextData; }
		
		Anim::tSigAnimMoMap* fMakeMoMap( );

		void fOpenDoc( const tFilePathPtr& file );
		tEditorActionStack& fEditorAction( ) { return mCanvas->fEditorActions( ); }

		void fSetSkeletonEntity( tSkeletableSgFileRefEntity* entity );

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
		void fSetStatus( const char* status );

		b32 fClearScene( );
		void fNewDoc( );
		b32 fSaveDoc( b32 saveAs );
		void fOpenDoc( );
		void fOpenRecent( u32 ithRecentFile );
		void fAddRecentFile( const tFilePathPtr& path );
		void fUpdateRecentFileMenu( );
		b32 fBuild( );

		b32 fSerialize( const tFilePathPtr& path );
		b32 fDeserialize( const tFilePathPtr& path );
	};
}

#endif//__tSigAnimDialog__
