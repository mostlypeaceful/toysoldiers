#ifndef _tSigFxMainWindow__
#define _tSigFxMainWindow__
#include "tToolsGuiMainWindow.hpp"
#include "tSigFxFile.hpp"
#include "tWxProgressDialog.hpp"
#include "tManipulateEffectPanel.hpp"
#include "tCreateNewEffectPanel.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "Editor/tEditorAction.hpp"
#include "tTabControl.hpp"
#include "tAttractorPropertiesPanel.hpp"
#include "tParticleSystemPropertiesPanel.hpp"
#include "tMeshSystemPropertiesPanel.hpp"
#include "tReferenceEntityPanel.hpp"
#include "wx/clrpicker.h"
#include "tSigFxSystem.hpp"

namespace Sig
{
	class tWxRenderPanelContainer;
	class tSigFxMatEd;
	class tMeshSelectorDialog;
	class tSigFxControlPanel;
	class tSigFxKeyline;
	class tSigFxGraphline;
	class tSigFxTimeline;

	class tSaveLoadProgressDialog : public tWxProgressDialog
	{
	public:
		tSaveLoadProgressDialog( wxWindow* parent, const char* title )
			: tWxProgressDialog( parent, title ) { SetSize( 300, GetSize( ).y ); }
		virtual void fUpdate( u32 ithObject, u32 totalObjects ) = 0;
	};

	class tEffectsFileSaveLoadProgressDialog : public tSaveLoadProgressDialog
	{
	private:
		tStringPtr mMessage;
	public:
		tEffectsFileSaveLoadProgressDialog( wxWindow* parent, const char* title, b32 loading )
			: tSaveLoadProgressDialog( parent, title )
		{
			if( loading)
				mMessage = tStringPtr( "Loading effects file...");
			else
				mMessage = tStringPtr("Saving effects file...");
		}

		virtual void fUpdate( u32 ithObject, u32 totalObjects )
		{
			const f32 progress = ithObject / ( f32 )totalObjects;
			tWxProgressDialog::fUpdate( progress, mMessage.fCStr() );
		}
	};

	class tSigFxClipboard
	{
	private:
		
		tGraphPtr mCopiedGraph;

	public:

		void fClearCopy( )
		{
			mCopiedGraph.fRelease( );
		}
		void fSetCopiedGraph( tSigFxGraph* graph )
		{
			mCopiedGraph.fReset( ( *graph->fRawGraph( ) ).fGetRawPtr( ) );
		}
		b32 fCanPaste( tSigFxGraph* graph ) const
		{
			if( mCopiedGraph.fNull( ) )
				return false;
			tGraph* copied = mCopiedGraph.fGetRawPtr( );
			tGraph* pasty = graph->fRawGraph( )->fGetRawPtr( );
			if( copied == pasty || copied->fGetID( ) != pasty->fGetID( ) )
				return false;
			return true;
		}

		void fPasteGraph( tSigFxGraph* graph )
		{
			if( fCanPaste( graph ) )
			{
				graph->fClear( );
				for( u32 i = 0; i < mCopiedGraph->fNumKeyframes( ); ++i )
				{
					tKeyframePtr key = mCopiedGraph->fNewKeyFromKey( mCopiedGraph->fKeyframe( i ) );
					graph->fAddKeyframe( key );
				}
			}

			graph->fBuildValues( );
			graph->fMarkAsDirty( );
			graph->fFrame( );
		}
	};

	class tSigFxMainWindow : public tToolsGuiMainWindow
	{
	public:
		static const char cNewDocTitle[];
		static const u32 cMaxTemplateFiles = 10000;
	private:

		tGrowableArray< tEditorHotKeyPtr > mHotKeyStorage;

		tSigFxMatEd*				mMatEd;
		tMeshSelectorDialog*		mMeshSelector;
		tWxToolsPanelContainer*		mToolsPanelContainer;

		tManipulateEffectPanel*		mManipPanel;
		tCreateNewEffectPanel*		mCreateNewEffectPanel;
		tParticleSystemPropertiesPanel* mParticleSystemPropertiesPanel;
		tAttractorPropertiesPanel*	mAttractorPropertiesPanel;
		tMeshSystemPropertiesPanel* mMeshSystemPropertiesPanel;
		tReferenceEntityPanel*		mReferenceEntityPanel;

		FX::tSigFxSystem*	mFxScene;

		tTabControl*				mTabControl;
		tSigFxKeyline*				mKeyline;
		tSigFxGraphline*			mGraphline;
		tSigFxTimeline*				mTimeline;

		std::string	mDocName;
		wxString mLastDirectory;
		wxString mLastImportDirectory;
		wxString mTemplateFxFilePath;

		tEditorActionStack::tNotifyEvent::tObserver mOnDirty, mOnAddAction;
		tEditorSelectionList::tOnSelectionChanged::tObserver mOnSelChanged;

		tStrongPtr<tSaveLoadProgressDialog> mProgressBar;

		wxChoice* mPlaybackTimeFrequency;
		wxColourPickerCtrl* mBackgroundColourPicker;

		f32 mIdealFrameDt;
		f32 mLastElapsed;
		f32 mTimeSinceLastIdealUpdate;
		Time::tStopWatch mTimer;

		b32 mSelectionChanged;
		u32 mLastNumSceneObjects;
		b32 mSyncNextFrame;

		b32 mCaptureFrame;
		b32 mPreviewingCurrentDoc;
		b32 mCheckCommandArgs;
		b32 mSavingInProgress;
		tFilePathPtr mPreviewSaveFile;

		tEntityPtr mBackgroundLevelEntity;

		struct tTemplateFileFolder
		{
			tTemplateFileFolder( )
			{
				fZeroOut( this );
			}
			tTemplateFileFolder( const tFilePathPtr& path, wxMenu* parentMenu )
			{
				mFolderPath = path;
				mParentMenu = parentMenu;
			}
			wxMenu* mParentMenu;
			tFilePathPtr mFolderPath;
			tFilePathPtrList mFileList;
			tFilePathPtrList mFolderList;
		};

		tGrowableArray< tTemplateFileFolder > mTemplateFoldersList;
		tGrowableArray< tFilePathPtr > mLinearTemplatePaths;

		tSigFxClipboard mSigFxClipboard; 

	public:
		tSigFxMainWindow( tToolsGuiApp& guiApp );
		virtual ~tSigFxMainWindow( );

		virtual void fSetupRendering( );
		virtual void fOnTick( );
		tWxRenderPanelContainer* fRenderPanelContainer( ) const { return mRenderPanelContainer; }

		void fAddMenus( );
		void fAddToolbar( );
		void fAddToolPanel( );
		void fUndo( );
		void fRedo( );
		void fImmediateRefresh( b32 forcekeyframeupdate );

		void fSync( );
		void fDoActualSync( );

		void fOnDirty( tEditorActionStack& stack );
		void fOnMouseEvents( wxMouseEvent& event );
		void fOnSelChanged( tEditorSelectionList& selectedObjects );
		void fOnActionUndoOrRedo( tEditorActionStack& stack );
		void fOnPlaybackTimeChanged( wxCommandEvent& event );
		void fOnBackgroundColourChanged( wxColourPickerEvent& event );
		void fOnShowLevelInBackgroundClicked( wxCommandEvent& event );
		void fCenterSelectedObjects( );

		void fCopySelectedObjects( );
		void fPreview( u32 id );

		FX::tSigFxSystem* fFxScene( ) { return mFxScene; }
		tSigFxMatEd* fSigFxMatEd( ) const { return mMatEd; }

		void fNewDoc( );
		void fOpenDoc( );
		b32 fSaveDoc( b32 saveAs );
		void fOpenDoc( const tFilePathPtr& file );
		void fImport( );
		b32 fBuild( );

		tSigFxClipboard* fSigFxClipboard( ) { return &mSigFxClipboard; }

	private:
		void fOnClose( wxCloseEvent& event );
		void fOnAction( wxCommandEvent& event );
		
		void fStartRecording( );
		void fCaptureTemplateFrame( );

		virtual std::string fEditableFileExt( ) const;
		void fOpenRecent( u32 ithRecentFile );

		void fBuildImportTemplateMenu( tTemplateFileFolder& curTemplateFolder, b32 root, u32 &menuIdxCount );

		b32 fImportFxmlFile( const tFilePathPtr& filePath );
		b32 fDeserializeFxmlFile( const tFilePathPtr& filePath );
		virtual b32 fSerializeDoc( const tFilePathPtr& path );

		void fOnObjectSerialized( u32 nObjectsSerialized, u32 nObjectsTotal );

		DECLARE_EVENT_TABLE( )
	};
}

#endif//_tSigFxMainWindow__
