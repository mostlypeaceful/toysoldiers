#ifndef __tToolsGuiMainWindow__
#define __tToolsGuiMainWindow__
#include "tWxSavedLayout.hpp"
#include "Editor/tEditorContextAction.hpp"
#include "tRotationGizmoSettings.hpp"

namespace Sig { namespace Gfx
{
	class tDevicePtr;
}}

namespace Sig
{
	class tToolsGuiApp;
	class tWxRenderPanelContainer;

	///
	/// \brief Derived registry serializer for saving the layout of a tEditorAppWindow.
	class tToolsGuiMainWindowSavedLayout : public tWxSavedLayout
	{
	public:
		tToolsGuiMainWindowSavedLayout( const std::string& regKeyName ) : tWxSavedLayout( regKeyName ) { }
	};

	///
	/// \brief Represents the main window for a gui tools app (i.e., an editor).
	class toolsgui_export tToolsGuiMainWindow : public wxFrame
	{
		tToolsGuiApp&					mGuiApp;
		tToolsGuiMainWindowSavedLayout	mSavedLayout;
		tEditorContextActionList		mContextActions;
		Time::tStopWatch				mOnTickTimer;
		b32								mDialogInputActive;
		tRotationGizmoSettings*		mRotationGizmoSettings;

	protected:

		u32								mBaseRecentFileActionId;
		wxMenu*							mRecentFilesMenu;
		tWxRenderPanelContainer*		mRenderPanelContainer;

	public:
		explicit tToolsGuiMainWindow( tToolsGuiApp& guiApp );
		virtual ~tToolsGuiMainWindow( ) { }
		virtual void fSetupRendering( ) = 0;
		virtual void fOnTick( ) = 0;
		virtual void fClearScene( b32 closing );
		virtual b32 fSerializeDoc( const tFilePathPtr& path ) { return true; }
		virtual void fOnRightClick( wxWindow* window, wxMouseEvent& event );
		virtual std::string fEditableFileExt( ) const { return ""; }

		// gui app
		tToolsGuiApp& fGuiApp( ) { return mGuiApp; }
		const tToolsGuiApp& fGuiApp( ) const { return mGuiApp; }

		// render panels
		tWxRenderPanelContainer* fRenderPanelContainer( ) { return mRenderPanelContainer; }
		void fFrameSelection( b32 useFocusPanel = false );
		void fFrameAll( );
		void fFrameAllEveryViewport( );
		virtual void fFrameCustom( ) { }

		// input
		void fSetDialogInputActive( b32 active = true ) { mDialogInputActive = active; }
		b32	 fDialogInputActive( ) const { return mDialogInputActive; }
		b32	 fPriorityInputActive( ) const;

		// status
		void fSetStatus( const char* status );

		// actions
		tEditorContextActionList& fContextActions( ) { return mContextActions; }

		// misc.
		void fUpdateRecentFileMenu( );

		// rotation gizmo settings
		tRotationGizmoSettings*				fRotationGizmoSettings( ) const { return mRotationGizmoSettings; }

	protected:
		b32 fBeginOnTick( f32* dtOut = 0 );
		void fSaveLayout( );
		void fLoadLayout( );
	};

}

#endif//__tToolsGuiMainWindow__
