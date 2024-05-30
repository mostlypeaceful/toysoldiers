#ifndef __tToolsGuiApp__
#define __tToolsGuiApp__
#include "tApplication.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tLightEntity.hpp"
#include "tEditorHotKey.hpp"
#include "tEditorCursorController.hpp"
#include "tGlobalLightDirGizmo.hpp"
#include "Threads/tThread.hpp"

namespace Sig
{
	class tToolsGuiMainWindow;
	class tEditableObjectContainer;
	class tEditorSelectionList;
	class tEditorActionStack;
	class tEditablePropertyTable;

	class toolsgui_export tToolsGuiApp : public tApplication
	{
		friend class tToolsGuiMainWindow;
	public:

		static const u32 cMaxRecentlyOpenedFiles = 8;
		static const char cNewDocTitle[];
		typedef tToolsGuiMainWindow* (*tNewMainWindow)( tToolsGuiApp& guiApp );

	private:

		std::string					mAppName;
		std::string					mDocName;
		std::string					mRegKeyName;// base registry key name for app

		HINSTANCE					mHinst;
		tNewMainWindow				mNewMainWindow;
		tToolsGuiMainWindow*		mMainWindow;
		Win32Util::tRecentlyOpenedFileList mRecentFiles;
		tEditorHotKeyTable			mHotKeys;
		tEditorCursorControllerPtr	mCurrentCursor, mPrevCursor;

		Gfx::tDevicePtr				mGfxDevice;
		Gfx::tLightEntityPtr		mDefaultLight;

		tGlobalLightDirGizmoPtr		mGlobalLightDirGizmo;
		tGrowableArray< tRenderOnlyGizmoPtr > mRenderOnlyGizmos;

		tEditableObjectContainer*	mEditableObjects;
		b32							mSaveInProgress;

		Math::tVec4f				mSavedFogValues;
		Math::tVec3f				mSavedFogColor;

	public:

		tToolsGuiApp( const char* appName, HINSTANCE hinst, tNewMainWindow newMainWindow );
		~tToolsGuiApp( );

		virtual void fConfigureApp( tStartupOptions& optsOut, tPlatformStartupOptions& platformOptsOut );
		virtual void fTickApp( );
		virtual void fStartupApp( );
		virtual void fShutdownApp( );

		const std::string&					fAppName( ) const { return mAppName; }
		void								fSetDocName( const std::string& name ) { mDocName = name; }
		const std::string&					fDocName( ) const { return mDocName; }
		const std::string&					fRegKeyName( ) const { return mRegKeyName; }

		b32									fShuttingDown( ) const { return mMainWindow == 0; }
		tToolsGuiMainWindow&				fMainWindow( ) const { sigassert( mMainWindow ); return *mMainWindow; }
		void								fAddRecentFile( const tFilePathPtr& path );
		const Win32Util::tRecentlyOpenedFileList& fRecentFiles( ) const { return mRecentFiles; }
		tEditorHotKeyTable&					fHotKeys( ) { return mHotKeys; }

		const Gfx::tDevicePtr&				fGfxDevice( ) const { return mGfxDevice; }

		// default light
		const Gfx::tLightEntityPtr&			fDefaultLight( ) const { return mDefaultLight; }
		void fSyncDefaultLight( tEditablePropertyTable& properties );

		tEditableObjectContainer&			fEditableObjects( ) { sigassert( mEditableObjects ); return *mEditableObjects; }
		const tEditableObjectContainer&		fEditableObjects( ) const { sigassert( mEditableObjects ); return *mEditableObjects; }
		tEditorSelectionList&				fSelectionList( );
		tEditorSelectionList&				fPrevSelectionList( );
		tEditorActionStack&					fActionStack( );
		const tEditorActionStack&			fActionStack( ) const;

		// fog
		void								fDisableFog( );
		void								fEnableFog( );

		// cursors...
		const tEditorCursorControllerPtr& fCurrentCursor( ) const { return mCurrentCursor; }
		void fSetCurrentCursor( const tEditorCursorControllerPtr& newCursor );

		// render only gizmos
		void fSetLightGizmoActive( b32 active ) { mGlobalLightDirGizmo->fSetActive( active ); }
		void fAddRenderGizmo( tRenderOnlyGizmoPtr& newGizmo ) { mRenderOnlyGizmos.fPushBack( newGizmo ); }
		void fTickRenderGizmos( );

		// scene management
		std::string fMakeWindowTitle( ) const;
		b32 fClearScene( b32 closing = false );
		b32 fSaveDoc( b32 saveAs );

	private:

		HWND fCreateMainWindow( );

	};
}

#define implement_toolsgui_application( appName, mainWindowClass ) \
	namespace { static ::Sig::tToolsGuiMainWindow* fNewMainWindow( ::Sig::tToolsGuiApp& guiApp ) { return new mainWindowClass( guiApp ); } } \
	int WINAPI WinMain( HINSTANCE h1, HINSTANCE h2, LPSTR cmdLine, int showCmd ) \
	{ \
		sigassert( Sig::Threads::tThread::fMainThreadId( ) == Sig::Threads::tThread::fCurrentThreadId( ) ); \
		::Sig::tToolsGuiApp* theApp = new ::Sig::tToolsGuiApp( #appName, h1, fNewMainWindow ); \
		const int result = theApp->fRun( cmdLine ); \
		delete theApp; \
		return result; \
	}


#endif//__tToolsGuiApp__
