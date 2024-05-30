//------------------------------------------------------------------------------
// \file tSigAnimMainWindow.hpp - 18 Jul 2008
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef _tSigAnimMainWindow__
#define _tSigAnimMainWindow__
#include "tToolsGuiMainWindow.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "tConfigurableBrowserTree.hpp"

namespace Sig
{
	class tWxRenderPanelContainer;

	class tWxToolsPanel;
	class tWxToolsPanelContainer;

	class tSkeletableSgFileRefEntity;
	class tAnimationTreePanel;
	class tAnifigPanel;
	class tLoadObjectPanel;
	class tManipulateObjectPanel;
	class tSigAnimTimeline;
	class tSklmlBrowser;
	class tEntityControlPanel;
	class tSigAnimDialog;
	class tSearchableOpenFilesDialog;

	namespace Anifig { class tFile; }
	

	class tSigAnimMainWindow : public tToolsGuiMainWindow
	{
	public:
		explicit tSigAnimMainWindow( tToolsGuiApp& guiApp );
		virtual ~tSigAnimMainWindow( );
		virtual void fSetupRendering( );
		virtual void fOnTick( );

		b32 fSerializeDoc( const tFilePathPtr& path );
		virtual std::string fEditableFileExt( ) const { return ".anifig"; }
		void fOpenDoc( const tFilePathPtr& file );
		virtual void fOnRightClick( wxWindow* window, wxMouseEvent& event );

		void fToggleAnimEd( );
		void fNextFrame( );
		void fPreviousFrame( );
		void fShowBackupScenes( wxCommandEvent& event );
		void fLockFPS( b32 lock );

	private:

		// Setup
		void fAddTools( );

		void fProcessAnimPackWaitList( );

		void fAddResource( const tResourcePtr & res );
		void fRemoveResource( const tResourcePtr & res );

		void fOnObjectAdded( tEditableObjectContainer & container, const tEntityPtr & entity );
		void fOnSelectionChanged( tEditorSelectionList & list );

		void fOnClose( wxCloseEvent& event );
		void fOnAction(wxCommandEvent& event);
		void fOpenDoc( );
		
		b32 fSerializeAnifigFile( const tFilePathPtr& filePath );
		void fSerializeAnifigFile( Anifig::tFile & file );

		b32 fDeserializeSceneFile( const tFilePathPtr& filePath );
		void fDeserializeAnifigFile( const Anifig::tFile & file );
		void fDeserializeSigmlFile( const Sigml::tFile & file);

		void fCreateAnipk( );
		void fCreateMomap( );
		void fReloadResources( );

		void fRenderSkeletons( );
		void fRenderStackText( );
		void fUpdateTimeline( );

		void fUpdateBackupFileList( );
		void fOpenBackupDoc( u32 index );

	private:

		wxBoxSizer*					mMainSizer;

		// Tools Panel
		tWxToolsPanelContainer*		mToolsPanelContainer;

		tGrowableArray<tEntityPtr> mWaitingForAnimPacks;
		tAnimationTreePanel*	mAnimationTreePanel;
		tAnifigPanel*			mAnifigPanel;
		tLoadObjectPanel*		mLoadObjectPanel;
		tManipulateObjectPanel* mManipTools;
		tSigAnimTimeline*		mTimeline;
		tEntityControlPanel*	mAnimControlPanel;

		tSigAnimDialog*			mMoMapEditorDialog;
		tSearchableOpenFilesDialog* mOpenDialog;

		b32						mRefreshSklmlBrowser;
		tSklmlBrowser*			mSklmlBrowser;

		tHashTable<tResourcePtr, u32> mResources;
		tEditableObjectContainer::tOnObjectAdded::tObserver mOnObjectAdded;
		tEditorSelectionList::tOnSelectionChanged::tObserver mOnSelectionChanged;

		tEditorHotKeyPtr mFrameCameraHotKey;
		tEditorHotKeyPtr mNextFrameHotKey;
		tEditorHotKeyPtr mPreviousFrameHotKey;
		tEditorHotKeyPtr mSetAnimEventHotKey;

		tFilePathPtrList mBackupFileList;
		tFilePathPtr     mOpenDocPath;
		static const u32 mDateMarkerLength = 10;

		Gui::tTextPtr mCurrentStackText;

		f32	mTimeSinceRender;
		b32 mLockThirtyFPS;
	
		DECLARE_EVENT_TABLE()
	};
}

#endif//_tSigAnimMainWindow__
