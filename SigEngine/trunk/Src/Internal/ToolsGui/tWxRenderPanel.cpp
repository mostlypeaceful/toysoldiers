#include "ToolsGuiPch.hpp"
#include "tWxRenderPanel.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tWxRenderPanelGridSettings.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "Editor/tEditableObjectContainer.hpp"

// cameras
#include "tToolsMouseAndKbCamera.hpp"
#include "Editor/tEditableCameraEntity.hpp"

// graphics
#include "Gfx/tSolidColorMaterial.hpp"

namespace Sig
{
	class tToggleGridSettingsContextAction : public tEditorContextAction
	{
		tWxRenderPanel* mRenderPanel;
		u32 mId;
	public:
		tToggleGridSettingsContextAction( tWxRenderPanel* renderPanel )
			: mRenderPanel( renderPanel )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mRenderPanel == mRenderPanel->fGetContainer( )->fGetActiveRenderPanel( ) )
			{
				menu.Append( mId, _T("&Grid Settings..."));
				menu.AppendSeparator( );
				return true;
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mId )
			{
				if( mRenderPanel->fGetGridSettings( )->IsIconized( ) )
					mRenderPanel->fGetGridSettings( )->Restore( );
				const wxPoint screenPos = wxPoint( mRenderPanel->fGetMouse( ).fGetState( ).mCursorPosX, mRenderPanel->fGetMouse( ).fGetState( ).mCursorPosY );
				mRenderPanel->fGetGridSettings( )->SetPosition( mRenderPanel->ClientToScreen( screenPos ) );
				mRenderPanel->fGetGridSettings( )->Show( true );
			}
			else
				return false;
			return true;
		}
	};
}

namespace Sig
{
	tWxRenderPanel::tWxRenderPanel( tWxRenderPanelContainer* container, wxWindow* parentWindow, const std::string& regKeyName )
		: wxPanel( parentWindow )
		, mRegKeyName( regKeyName )
		, mContainer( container )
		, mGrid( new Gfx::tSolidColorGrid( ) )
		, mGridSettings( 0 )
		, mShowGrid( true )
		, mSnapToGrid( false )
		, mTickFrame( 0 )
		, mLastSizeFrame( 0 )
	{
		// setup wx window stuff
		SetBackgroundColour( wxColour( 0x33, 0x33, 0x33, wxALPHA_OPAQUE ) );
		Connect( wxEVT_SIZE, wxSizeEventHandler( tWxRenderPanel::fOnSize ), NULL, this );
		Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( tWxRenderPanel::fOnRightClick ), NULL, this);
		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tWxRenderPanel::fOnAction ), NULL, this );

		// create grid settings dialog
		mGridSettings = new tWxRenderPanelGridSettings( this, regKeyName + "\\GridSettings", *mGrid, mShowGrid, mSnapToGrid );

		fMainWindow( )->fContextActions( ).fPushBack( tEditorContextActionPtr( new tToggleGridSettingsContextAction( this ) ) );
	}

	tWxRenderPanel::~tWxRenderPanel( )
	{
	}

	HWND tWxRenderPanel::fGetRenderHwnd( )
	{
		return ( HWND )GetHWND( );
	}

	tToolsGuiMainWindow* tWxRenderPanel::fMainWindow( )
	{
		return mContainer->fMainWindow( );
	}

	void tWxRenderPanel::fResetProjectionMatrices( u32 bbWidth, u32 bbHeight )
	{
		const f32 aspect = (f32)bbWidth / bbHeight;

		const Gfx::tTripod tripod = mScreen->fViewport( 0 )->fLogicCamera( ).fGetTripod( );
		const Gfx::tLens oldLens = mScreen->fViewport( 0 )->fLogicCamera( ).fGetLens( );

		Gfx::tLens lens;
		lens.fSetPerspective( oldLens.mNearPlane, oldLens.mFarPlane, aspect, Math::cPiOver4 );
		Gfx::tCamera camera( lens, tripod );

		if( oldLens.mProjectionType == Gfx::tLens::cProjectionOrtho )
			tToolsMouseAndKbCamera::fUpdateOrthoLens( camera );

		mScreen->fViewport( 0 )->fSetCameras( camera );

		Gfx::tLens screenLens;
		screenLens.fSetScreen( 0.0f, 1.f, 0.f, (f32)bbWidth, (f32)bbHeight, 0.f );
		mScreen->fGetScreenSpaceCamera( ).fSetup( screenLens, Gfx::tTripod( Math::tVec3f::cZeroVector, Math::tVec3f::cZAxis, Math::tVec3f::cYAxis ) );
	}

	void tWxRenderPanel::fSetupRendering( tToolsGuiApp& guiApp )
	{
		// create screen
		Gfx::tScreenCreationOptions screenCreateOpts;
		screenCreateOpts.mWindowHandle = ( u64 )fGetRenderHwnd( );
		screenCreateOpts.mFullScreen = false;
		screenCreateOpts.mBackBufferWidth = 320;
		screenCreateOpts.mBackBufferHeight = 240;
		screenCreateOpts.mVsync = Gfx::VSYNC_NONE;
		screenCreateOpts.mAutoDepthStencil = guiApp.fGfxDevice( )->fSingleScreenDevice( );
		screenCreateOpts.mResourceDepot = guiApp.fResourceDepot( );
		Input::tKeyboard::fInstance( ).fCaptureState( );
		if( Input::tKeyboard::fInstance( ).fButtonHeld( Input::tKeyboard::cButtonLCtrl )||Input::tKeyboard::fInstance( ).fButtonHeld( Input::tKeyboard::cButtonRCtrl ) )
			screenCreateOpts.mMultiSamplePower = Gfx::tScreen::fDefaultMultiSamplePower( );
		mScreen.fReset( new Gfx::tScreen( guiApp.fGfxDevice( ), guiApp.fSceneGraph( ), screenCreateOpts ) );
		mScreen->fSetRgbaClearColor( Math::tVec4f( 0x33/255.f, 0x33/255.f, 0x33/255.f, 1.f ) );

		// create single viewport
		mScreen->fSetViewportCount( 1 );
		const Math::tVec3f origin = Math::tVec3f::cZeroVector;

		Gfx::tLens lens;
		lens.fSetPerspective( 0.1f, 10000.f, 9.f/16.f, Math::cPiOver4 );
		mScreen->fViewport( 0 )->fSetCameras( Gfx::tCamera( lens, Gfx::tTripod( origin + Math::tVec3f( 0.f, 10.0f, 40.0f ), origin, Math::tVec3f( 0.f, 1.f, 0.f ) ) ) );
		mCamera = tEditorCameraPtr( new tToolsMouseAndKbCamera( mScreen->fViewport( 0 ), mMouse, Input::tKeyboard::fInstance( ) ) );

		fResetProjectionMatrices( screenCreateOpts.mBackBufferWidth, screenCreateOpts.mBackBufferHeight );


		// setup grid with allocators
		mGrid->fResetDeviceObjects( 
			fMainWindow( )->fGuiApp( ).fGfxDevice( ),
			mContainer->fGetSolidColorMaterial( ), 
			mContainer->fGetSolidColorGeometryAllocator( ), 
			mContainer->fGetSolidColorIndexAllocator( ) );

		// actually create the grid lines
		fSetupGrid( );

		// setup text
		const Math::tVec4f debugTextRgba = Math::tVec4f( 0.7f, 0.7f, 1.f, 0.75f );

		mFpsText.fReset( new Gui::tFpsText( ) );
		mFpsText->fSetDevFont( );
		mFpsText->fSetRgbaTint( debugTextRgba );
		mStatsText.fReset( new Gui::tText( ) );
		mStatsText->fSetDevFont( );
		mStatsText->fSetRgbaTint( debugTextRgba );

		// setup mouse
		mMouse.fStartup( ( Input::tMouse::tGenericWindowHandle )fGetRenderHwnd( ) );
	}

	void tWxRenderPanel::fOnTick( )
	{
		if( mScreen.fNull( ) )
			return;

		mMouse.fCaptureState( );

		fCheckForDialogInput( );

		if( !fMainWindow( )->fDialogInputActive( ) )
		{
			if( mMouse.fCursorInClientArea( ) && fGetMouse( ).fButtonDown( Input::tMouse::cButtonLeft ) )
				fMainWindow( )->SetFocus( );

			if( mPreviewingCamera )
			{
				// Take movements from gizmos
				Gfx::tCamera cam = mScreen->fViewport( 0 )->fRenderCamera( );
				mPreviewingCamera->fExtractCameraData( cam );
				mScreen->fViewport( 0 )->fSetCameras( cam );
			}

			if( mCamera )
			{
				mCamera->fSetIsHandlingInput( false );
				mCamera->fOnTick( fMainWindow()->fGuiApp().fGetFrameDeltaTime() );
			}

			if( mPreviewingCamera )
			{
				// Push movements from mCamera' fOnTick
				mPreviewingCamera->fDriveCamera( mScreen->fViewport(0)->fLogicCamera() );
				if( mCamera->fGetIsHandlingInput() )
					fMainWindow( )->fGuiApp( ).fActionStack( ).fForceSetDirty( );
			}
		}
	}

	void tWxRenderPanel::fRender( const Gfx::tDisplayStats* selectedDisplayStats )
	{
		if( mScreen.fNull( ) )
			return;

		if( mLastSizeFrame < mTickFrame - 1 )
		{
			// we wait a couple frames after resizing so we're not constantly resetting
			mLastSizeFrame = ~0;
			const wxSize bbSize = GetClientSize( );
			if( bbSize.x > 0 && bbSize.y > 0 )
			{
				mCamera->fSetWindowRes( Math::tVec2u( GetSize( ).x, GetSize( ).y ) );
				mScreen->fResize( bbSize.x, bbSize.y );
				fResetProjectionMatrices( bbSize.x, bbSize.y );
			}
		}

		if( !mScreen->fDeviceCheck( ) )
			return;

		if( mShowGrid )
			mScreen->fAddWorldSpaceDrawCall( Gfx::tRenderableEntityPtr( mGrid.fGetRawPtr( ) ) );

		mFpsText->fBuild( *mScreen );
		mFpsText->fAddDrawCall( *mScreen, 10.f, 10.f );

		char selStatsText[256]={0};
		if( selectedDisplayStats )
		{
			_snprintf( selStatsText, array_length( selStatsText ), 
				"selected tris = %d\n",
				selectedDisplayStats->fTotalTriCount( ) );
		}

		std::string previewName = mPreviewingCamera ? (std::string("Previewing camera: ") + mPreviewingCamera->fDisplayName( )) : std::string("");

		const Math::tVec3f gridPos = fGetGridCenter( );
		char statsText[256]={0};
		_snprintf( statsText, array_length( statsText ), 
			"%s"
			"tris = %d\n"
			"batches = %d\n"
			"draws = %d\n"
			"%s\n",
			selStatsText,
			mScreen->fGetWorldStats( ).fTotalTriCount( ), 
			mScreen->fGetWorldStats( ).mBatchSwitches, 
			mScreen->fGetWorldStats( ).mNumDrawCalls,
			previewName.length() ? previewName.c_str( ) : "" );

		mStatsText->fSetPosition( Math::tVec3f( 10.f, 10.f + 1.f * mStatsText->fLineHeight( ), 0.f ) );
		mStatsText->fBakeBox( 2000, statsText, ( u32 )strlen( statsText ), Gui::tText::cAlignLeft );
		Gfx::tDrawCall statsDrawCall = mStatsText->fDrawCall( );
		if( statsDrawCall.fValid( ) )
			mScreen->fAddScreenSpaceDrawCall( statsDrawCall );

		Gfx::tGeometryBufferVRamAllocator::fGlobalPreRender( );
		Gfx::tIndexBufferVRamAllocator::fGlobalPreRender( );

		mScreen->fRender( );

		Gfx::tGeometryBufferVRamAllocator::fGlobalPostRender( );
		Gfx::tIndexBufferVRamAllocator::fGlobalPostRender( );

		mFpsText->fPostRender( );
	}

	void tWxRenderPanel::fFrame( const Math::tAabbf& frameBox )
	{
		mCamera->fFrame( frameBox );
	}

	void tWxRenderPanel::fSetProjectionType( const Math::tVec3f& viewAxis, const Math::tVec3f& up, u32 projType )
	{
		Gfx::tCamera cameraData = mScreen->fViewport( 0 )->fLogicCamera( );

		mCamera->fSetTripod( cameraData, cameraData.fGetTripod( ).mLookAt, viewAxis, up );
		mCamera->fSetOrtho( cameraData, projType );

		mScreen->fViewport( 0 )->fSetCameras( cameraData );
	}

	void tWxRenderPanel::fSetOrthoAndLookPos( const Math::tVec3f& lookPos, const Math::tVec3f& viewAxis, const Math::tVec3f& up )
	{
		Gfx::tCamera cameraData = mScreen->fViewport( 0 )->fLogicCamera( );

		mCamera->fSetTripod( cameraData, lookPos, viewAxis, up );
		mCamera->fSetOrtho( cameraData, Gfx::tLens::cProjectionOrtho );

		mScreen->fViewport( 0 )->fSetCameras( cameraData );
	}

	void tWxRenderPanel::fDisableRotation( b32 disable )
	{
		mCamera->fDisableRotation( disable ); 
	}

	void tWxRenderPanel::fDisableOrthoToggle( b32 disable  ) 
	{ 
		mCamera->fDisableOrthoToggle( disable ); 
	}

	void tWxRenderPanel::fSetPreviewCamera( tEditableCameraEntity* cam )
	{
		// Going to preview a camera.
		if( !mPreviewingCamera && cam )
		{
			mPreviousCamera = mScreen->fViewport( 0 )->fLogicCamera( );
			mCamera->fSetHandAdjusting( true );
		}
		
		// Coming from a preview camera.
		if( !cam && mPreviewingCamera )
		{
			Gfx::tCamera currentCam = mScreen->fViewport( 0 )->fRenderCamera( );

			// Trust me on this one. Need to pass in the currentCam so that the old perspective
			// lens gets saved but we still want to apply the original camera's data.
			if( mPreviousCamera.fGetLens().mProjectionType == Gfx::tLens::cProjectionOrtho )
			{
				mCamera->fSetOrtho( currentCam, mPreviousCamera.fGetLens().mProjectionType );
				fDisableRotation( true );
			}
			mScreen->fViewport( 0 )->fSetCameras( mPreviousCamera );
			
			mCamera->fSetHandAdjusting( false );
		}

		// Set up the preview camera if we're turning that on.
		mPreviewingCamera.fReset( cam );
		if( mPreviewingCamera )
		{
			// Note: this set up is probably not going to support preview cameras that are themselves ortho.
			Gfx::tCamera cam = mScreen->fViewport( 0 )->fRenderCamera( );
			const b32 previouslyOrtho = cam.fGetLens().mProjectionType == Gfx::tLens::cProjectionOrtho; // Keep track for later.

			mPreviewingCamera->fExtractCameraData( cam );
			
			// If we're becoming less ortho, need to set that after we've pulled data from the preview cam.
			if( previouslyOrtho )
			{
				mCamera->fSetOrtho( cam, cam.fGetLens().mProjectionType );
				fDisableRotation( false );
			}
			mScreen->fViewport( 0 )->fSetCameras( cam );
		}
	}

	void tWxRenderPanel::fSnapVertex( Math::tVec3f& vert ) const
	{
		mGridSettings->fSnapVertex( vert );
	}

	b32 tWxRenderPanel::fCheckForDialogInput( )
	{
		const b32 isActive = mGridSettings->fIsActive( );
		if( isActive )
			fMainWindow( )->fSetDialogInputActive( );
		return isActive;
	}

	void tWxRenderPanel::fSetupGrid( )
	{
		mGridSettings->fUpdateGrid( );
	}

	void tWxRenderPanel::fOnAction( wxCommandEvent& event )
	{
		tEditorContextAction::fHandleContextActionFromRightClick( this, event, fMainWindow( )->fContextActions( ) );
	}

	void tWxRenderPanel::fOnSize( wxSizeEvent& event )
	{
		mLastSizeFrame = mTickFrame;
		event.Skip( );
	}

	void tWxRenderPanel::fOnRightClick( wxMouseEvent& event )
	{
		fMainWindow( )->fOnRightClick( this, event );
	}

}
