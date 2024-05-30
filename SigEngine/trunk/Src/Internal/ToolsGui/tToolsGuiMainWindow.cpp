#include "ToolsGuiPch.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsMouseAndKbCamera.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Editor/tEditorSelectionList.hpp"

namespace Sig
{
	tToolsGuiMainWindow::tToolsGuiMainWindow( tToolsGuiApp& guiApp )
		: wxFrame( 0, wxID_ANY, guiApp.fAppName( ), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE /*| wxWANTS_CHARS*/ )
		, mGuiApp( guiApp )
		, mSavedLayout( guiApp.fRegKeyName( ) + "\\SavedLayout" )
		, mDialogInputActive( false )
		, mRotationGizmoSettings( 0 )
		, mBaseRecentFileActionId( 0 )
		, mRecentFilesMenu( 0 )
		, mRenderPanelContainer( 0 )
	{
		guiApp.mMainWindow = this;

		// add status bar
		const int statusWidths[] = { 0, 44, 320, -1 };
		wxStatusBar* statusBar = CreateStatusBar( array_length(statusWidths), wxST_SIZEGRIP|wxFULL_REPAINT_ON_RESIZE, 0, wxT("Status:") );
		SetStatusWidths( array_length(statusWidths), statusWidths );
		SetStatusText( "Status:", 1 );
		fSetStatus( "null" );

		// create rotation gizmo settings window
		mRotationGizmoSettings = new tRotationGizmoSettings( this, guiApp.fRegKeyName( ) + "\\RotationGizmoSettings" );
	}

	void tToolsGuiMainWindow::fClearScene( b32 closing )
	{
		SetTitle( fGuiApp( ).fMakeWindowTitle( ) );
	}

	void tToolsGuiMainWindow::fOnRightClick( wxWindow* window, wxMouseEvent& event )
	{
		tEditorContextAction::fDisplayContextMenuOnRightClick( window, event, fContextActions( ) );
	}

	b32 tToolsGuiMainWindow::fPriorityInputActive( ) const
	{
		if( fDialogInputActive( ) )
			return true;

		tWxRenderPanel* active = mRenderPanelContainer->fGetActiveRenderPanel( );
		if( active && active->fGetCamera( )->fGetIsHandlingInput( ) )
			return true;

		return false;
	}

	void tToolsGuiMainWindow::fSetStatus( const char* status )
	{
		SetStatusText( status, 2 );
	}

	void tToolsGuiMainWindow::fFrameSelection( b32 useFocusPanel )
	{
		tWxRenderPanel* renderPanel = useFocusPanel ? mRenderPanelContainer->fGetFocusRenderPanel() : mRenderPanelContainer->fGetActiveRenderPanel();
		if( renderPanel )
		{
			Math::tAabbf frameBox;

			if( renderPanel->fGetMouse( ).fButtonHeld( Input::tMouse::cButtonLeft ) &&
				!fGuiApp( ).fCurrentCursor( ).fNull( ) &&
				!fGuiApp( ).fCurrentCursor( )->fLastHoverObject( ).fNull( ) )
			{
				const Math::tVec3f frameCenter = fGuiApp( ).fCurrentCursor( )->fLastHoverIntersection( );
				const Math::tVec3f frameDiagonal = 5.f;
				frameBox = Math::tAabbf( frameCenter - frameDiagonal, frameCenter + frameDiagonal );
			}
			else if( fGuiApp( ).fSelectionList( ).fCount( ) == 0 )
			{
				fFrameAll( );
				return;
			}
			else
			{
				frameBox = fGuiApp( ).fSelectionList( ).fComputeBounding( );
			}

			renderPanel->fFrame( frameBox );
		}
	}

	void tToolsGuiMainWindow::fFrameAll( )
	{
		tWxRenderPanel* renderPanel = mRenderPanelContainer->fGetActiveRenderPanel( );
		if( renderPanel )
		{
			renderPanel->fFrame( fGuiApp( ).fEditableObjects( ).fComputeBounding( ) );
		}
	}

	void tToolsGuiMainWindow::fFrameAllEveryViewport( )
	{
		mRenderPanelContainer->fFrameAllViewports( fGuiApp( ).fEditableObjects( ).fComputeBounding( ) );
	}

	b32 tToolsGuiMainWindow::fBeginOnTick( f32* dtOut )
	{
		const f32 dt = mOnTickTimer.fGetElapsedS( );
		if( dtOut ) *dtOut = dt;
		mOnTickTimer.fResetElapsedS( );

		DWORD procIdOfForegroundWindow;
		GetWindowThreadProcessId( GetForegroundWindow( ), &procIdOfForegroundWindow );

		DWORD procIdOfApp;
		GetWindowThreadProcessId( ( HWND )GetHWND( ), &procIdOfApp );

		fSetDialogInputActive( false );

		return procIdOfForegroundWindow == procIdOfApp ? true : false;
	}

	void tToolsGuiMainWindow::fSaveLayout( )
	{
		if( IsIconized( ) || !IsShown( ) )
			return; // window is minimized, don't save

		tToolsGuiMainWindowSavedLayout layout( mSavedLayout.fRegistryKeyName( ) );
		layout.fFromWxWindow( this );

		if( layout.fIsInBounds( 2048 ) && layout != mSavedLayout )
		{
			layout.fSave( );
			mSavedLayout = layout;
		}
		else if( layout.mMaximized && !mSavedLayout.mMaximized )
		{
			// when maximized, the result of the layout settings are kind of screwy,
			// so we just set the maximized flag and save whatever the previous settings were
			mSavedLayout.mMaximized = true;
			mSavedLayout.fSave( );
		}
	}

	void tToolsGuiMainWindow::fLoadLayout( )
	{
		if( mSavedLayout.fLoad( ) && mSavedLayout.mVisible )
		{
			mSavedLayout.fToWxWindow( this );
			if( mSavedLayout.mMaximized )
				Maximize( );
		}
		else
		{
			SetSize( 1024, 768 );
			Center( );
			Maximize( );
			Show( true );
		}
	}

	void tToolsGuiMainWindow::fUpdateRecentFileMenu( )
	{
		if( !mRecentFilesMenu )
			return;

		while( mRecentFilesMenu->GetMenuItems( ).size( ) > 0 )
			mRecentFilesMenu->Delete( mRecentFilesMenu->GetMenuItems( ).front( ) );

		const Win32Util::tRecentlyOpenedFileList& recentFiles = fGuiApp( ).fRecentFiles( );
		const u32 min = fMin( tToolsGuiApp::cMaxRecentlyOpenedFiles, recentFiles.fCount( ) );
		for( u32 i = 0; i < min; ++i )
			mRecentFilesMenu->Append( mBaseRecentFileActionId + i, recentFiles[ i ].fCStr( ) );
	}

}

