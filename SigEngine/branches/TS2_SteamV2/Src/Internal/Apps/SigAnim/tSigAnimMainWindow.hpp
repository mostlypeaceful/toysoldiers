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

	namespace Anifig { class tFile; }
	

	class tSigAnimMainWindow : public tToolsGuiMainWindow
	{
		wxBoxSizer*					mMainSizer;

		// Tools Panel
		tWxToolsPanelContainer*		mToolsPanelContainer;
		
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

	private:

		// Setup
		void fBuildAnimPackTable( );
		void fAddTools( );

		void fProcessAnimPackWaitList( );
		void fGetAnimPacksForSkeleton( 
			const tResourcePtr & skelRes, tSkeletableSgFileRefEntity * entity );

		void fAddResource( const tResourcePtr & res );
		void fRemoveResource( const tResourcePtr & res );

		void fOnObjectAdded( tEditableObjectContainer & container, const tEntityPtr & entity );

		void fOnClose( wxCloseEvent& event );
		void fOnAction(wxCommandEvent& event);
		void fOpenDoc( );
		
		b32 fSerializeAnifigFile( const tFilePathPtr& filePath );
		void fSerializeAnifigFile( Anifig::tFile & file );

		b32 fDeserializeAnifigFile( const tFilePathPtr& filePath );
		void fDeserializeAnifigFile( const Anifig::tFile & file );

		void fCreateAnipk( );
		void fReloadResources( );

		void fRenderSkeletons( );
		void fRenderStackText( );
		void fUpdateTimeline( );

	private:

		class tAnimPackList : 
			public tRefCounter,
			public tGrowableArray<tResourcePtr> { };
		typedef tRefCounterPtr<tAnimPackList> tAnimPackListPtr;

		tAnimationTreePanel*	mAnimationTreePanel;
		tAnifigPanel*			mAnifigPanel;
		tLoadObjectPanel*		mLoadObjectPanel;
		tManipulateObjectPanel* mManipTools;
		tSigAnimTimeline*		mTimeline;
		tEntityControlPanel*	mAnimControlPanel;

		b32						mRefreshSklmlBrowser;
		tSklmlBrowser*			mSklmlBrowser;

		tHashTable<tFilePathPtr, tAnimPackListPtr> mAnimPacksBySkeleton;
		tGrowableArray<tEntityPtr> mWaitingForAnimPacks;
		tHashTable<tResourcePtr, u32> mResources;
		tEditableObjectContainer::tOnObjectAdded::tObserver mOnObjectAdded;

		tEditorHotKeyPtr mFrameCameraHotKey;
		tEditorHotKeyPtr mNextFrameHotKey;
		tEditorHotKeyPtr mPreviousFrameHotKey;
		tEditorHotKeyPtr mSetAnimEventHotKey;


		Gui::tTextPtr mCurrentStackText;

	
		DECLARE_EVENT_TABLE()
	};
}

#endif//_tSigAnimMainWindow__
