#include "ToolsPch.hpp"
#include "tMaterialPreviewPanel.hpp"
#include "tApplication.hpp"

namespace Sig
{
	namespace
	{
		static const wxColour cBgColor = wxColour( 0xaa, 0xaa, 0xaa );
	}

	tMaterialPreviewPanel::tMaterialPreviewPanel( wxWindow* parent, u32 width, u32 height,
		wxButton* playButton,
		wxButton* modeButton,
		wxButton* resetButton,
		wxButton* ambButton,
		wxButton* frontButton,
		wxButton* rimButton,
		wxButton* backButton )
		: wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( width, height ), wxBORDER_NONE )
		, mOgSize( width, height )
		, mSceneGraph( new tSceneGraph( tLogicThreadPoolPtr( new tLogicThreadPool( 1 ) ) ) )
		, mNormalClearColor( cBgColor.Red( )/255.f, cBgColor.Green( )/255.f, cBgColor.Blue( )/255.f )
		, mErrorClearColor( 0x77/255.f, 0x33/255.f, 0x33/255.f )
		, mLastElapsed( 0.f )
		, mYaw( 0.f )
		, mDoYaw( false )
		, mCurrentPreviewGeometry( 0 )
		, mPlayButton( playButton )
		, mModeButton( modeButton )
		, mResetButton( resetButton )
		, mAmbButton( ambButton )
		, mFrontButton( frontButton )
		, mRimButton( rimButton )
		, mBackButton( backButton )
	{
		SetBackgroundColour( cBgColor );
		mSceneGraph->fSetSpatialBounds( 100.f, 0.f, 50.f, 0.f );
	}
	tMaterialPreviewPanel::~tMaterialPreviewPanel( )
	{
	}
	void tMaterialPreviewPanel::fSetup( const Gfx::tDevicePtr& device )
	{
		mTextureCache.fReset( new Dx9Util::tTextureCache( device ) );

		// This safety check is needed for tools who have no other graphics.
		//  A default device needs to be set to load default materials.
		if( !Gfx::tDevice::fGetDefaultDevice( ) )
			Gfx::tDevice::fSetDefaultDevice( device );

		tResourceDepotPtr resources( NEW tResourceDepot( ) );
		tResourceProviderPtr resourceProvider = tResourceProviderPtr( NEW tFileSystemResourceProvider( ) );
		resourceProvider->fSetRootPath( ToolsPaths::fGetCurrentProjectGamePlatformFolder( cCurrentPlatform ) );
		resources->fAddResourceProvider( resourceProvider );

		// create screen
		Gfx::tScreenCreationOptions screenCreateOpts;
		screenCreateOpts.mWindowHandle = ( u64 )GetHWND( );
		screenCreateOpts.mFullScreen = false;
		screenCreateOpts.mBackBufferWidth = GetSize( ).x;
		screenCreateOpts.mBackBufferHeight = GetSize( ).y;
		screenCreateOpts.mVsync = Gfx::VSYNC_NONE;
		screenCreateOpts.mAutoDepthStencil = device->fSingleScreenDevice( );
		screenCreateOpts.mShadowMapLayerCount = 2;
		screenCreateOpts.mShadowMapResolution = 512;
		screenCreateOpts.mResourceDepot = resources;
		mScreen.fReset( new Gfx::tScreen( device, mSceneGraph, screenCreateOpts ) );

		// create single viewport
		mScreen->fSetViewportCount( 1 );

		mInitialCameraXform = Math::tMat3f::cIdentity;
		mInitialCameraXform.fTranslateLocal( Math::tVec3f( 0.f, 0.0f, -3.5f ) );

		Gfx::tLens lens;
		lens.fSetPerspective( 0.1f, 1000.f, ( f32 )GetSize( ).x / ( f32 )GetSize( ).y, Math::cPiOver4 );
		mScreen->fViewport( 0 )->fSetCameras( Gfx::tCamera(  lens, Gfx::tTripod( mInitialCameraXform ) ) );

		Gfx::tLens screenLens;
		screenLens.fSetScreen( 0.0f, 1.f, 0.f, (f32)GetSize( ).x, (f32)GetSize( ).y, 0.f );
		mScreen->fGetScreenSpaceCamera( ).fSetup( screenLens, Gfx::tTripod( Math::tVec3f::cZeroVector, Math::tVec3f::cZAxis, Math::tVec3f::cYAxis ) );


		// create default world light
		{
			Gfx::tLight light;
			light.fSetTypeDirection( );
			const Math::tVec3f ambient( 0.2f, 0.2f, 0.2f );
			const Math::tVec3f front( 1.25f, 1.25f, 1.25f );
			const Math::tVec3f rim( 0.0f, 0.0f, 0.0f );
			const Math::tVec3f back( 0.3f, 0.3f, 0.6f );
			light.fSetColors( front, rim, back, ambient );
			mInitialLightXform = Math::tMat3f::cIdentity;
			mInitialLightXform.fOrientZAxis( Math::tVec3f( -0.5f, -0.5f, 0.f ).fNormalize( ), Math::tVec3f::cYAxis );
			mDefaultLight.fReset( new Gfx::tLightEntity( mInitialLightXform, light, "PrimaryDirectional" ) );
			mDefaultLight->fSetCastsShadow( true );
			mDefaultLight->fSpawnImmediate( mSceneGraph->fRootEntity( ) );
			mDefaultLight->fUpdateDefaultLightDirection( mInitialLightXform.fZAxis( ) );

			//Gfx::tLightEntity::fSetShadowMapDefaults( 20.f, 0.f, 1000.f, 20.f, 20.f );
		}

		if( mPlayButton )
			mPlayButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonPlay ), NULL, this );
		if( mModeButton )
			mModeButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonMode ), NULL, this );
		if( mResetButton )
			mResetButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonReset ), NULL, this );
		if( mAmbButton )
			mAmbButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonAmb ), NULL, this );
		if( mFrontButton )
			mFrontButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonFront ), NULL, this );
		if( mRimButton )
			mRimButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonRim ), NULL, this );
		if( mBackButton )
			mBackButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonBack ), NULL, this );
	}
	void tMaterialPreviewPanel::fSetPreviewBundle( const tMaterialPreviewBundlePtr& bundle )
	{
		mPreviewBundle = bundle;
	}
	void tMaterialPreviewPanel::fRender( )
	{
		if( !mScreen )
			return;

		const f32 elapsed = mTimer.fGetElapsedS( );
		const f32 dt = elapsed - mLastElapsed;
		mLastElapsed = elapsed;

		if( int( elapsed - dt ) < int( elapsed ) )
			mTextureCache->fRefresh( ); // refresh once per second

		if( mDoYaw )
			mYaw += 0.5f * dt;

		Gfx::tScreen* originalScreen = mSceneGraph->fScreen( );
		mSceneGraph->fSetScreen( mScreen.fGetRawPtr( ) );

		if( mPreviewBundle && mTextureCache )
		{
			b32 animateLightAndCamera = true;
			if( animateLightAndCamera )
			{
				Math::tMat3f yawMat( Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cYAxis, mYaw ) ) );

				mDefaultLight->fMoveTo( yawMat * mInitialLightXform );

				Gfx::tCamera cam = mScreen->fViewport( 0 )->fLogicCamera( );
				cam.fSetTripod( Gfx::tTripod( yawMat * mInitialCameraXform  ) );
				mScreen->fViewport( 0 )->fSetCameras(cam );
			}
			else
			{
				mPreviewBundle->fSetYaw( mYaw );
			}

			mPreviewBundle->fUpdateMaterial( mMaterialFile, *mTextureCache );
			mPreviewBundle->fSetCurrentPreviewGeometry( (tMaterialPreviewBundle::tPreviewGeometryMode)mCurrentPreviewGeometry );
			mPreviewBundle->fAddToSceneGraph( *mSceneGraph );
		}

		mSceneGraph->fAdvanceTime( dt );
		mSceneGraph->fKickCoRenderMTRunList( );
		mScreen->fRootCanvas( ).fOnTickCanvas( dt );

		const Math::tVec3f& clearColor = true ? mNormalClearColor : mErrorClearColor;
		mScreen->fSetRgbaClearColor( Math::tVec4f( clearColor.x, clearColor.y, clearColor.z, 1.f ) );
		mScreen->fRender( );

		if( mPreviewBundle )
			mPreviewBundle->fRemoveFromSceneGraph( );

		mScreen->fRootCanvas( ).fOnTickCanvas( dt );
		mSceneGraph->fSetScreen( originalScreen );

		// If we don't set these back to NULL, the viewport
		// background gradient is not rendered correctly because
		// Maya starts drawing with our shaders!
		mScreen->fGetDevice( )->fGetDevice( )->SetPixelShader( NULL );
		mScreen->fGetDevice( )->fGetDevice( )->SetVertexShader( NULL );
	}
	void tMaterialPreviewPanel::fSetTimeRunning( b32 run )
	{
		if( run && !mTimer.fRunning( ) )
		{
			SetSize( mOgSize );
			mTimer.fStart( );
		}
		else if( !run && mTimer.fRunning( ) )
		{
			SetSize( 1, 1 );
			mTimer.fStop( );
		}
	}
	void tMaterialPreviewPanel::fClear( )
	{
		mPreviewBundle.fRelease( );
		mMaterialFile = Derml::tMtlFile( );
	}
	void tMaterialPreviewPanel::fFromDermlMtlFile( const Derml::tMtlFile& mtlFile )
	{
		mMaterialFile = mtlFile;
	}
	void tMaterialPreviewPanel::fOnButtonPlay( wxCommandEvent& )
	{
		mDoYaw = !mDoYaw;
	}
	void tMaterialPreviewPanel::fOnButtonMode( wxCommandEvent& )
	{
		mCurrentPreviewGeometry = ( mCurrentPreviewGeometry + 1 ) % tMaterialPreviewBundle::cPreviewGeometryModeCount;
	}
	void tMaterialPreviewPanel::fOnButtonReset( wxCommandEvent& )
	{
		mYaw = 0.f;
	}
	void tMaterialPreviewPanel::fOnButtonAmb( wxCommandEvent& )
	{
	}
	void tMaterialPreviewPanel::fOnButtonFront( wxCommandEvent& )
	{
	}
	void tMaterialPreviewPanel::fOnButtonRim( wxCommandEvent& )
	{
	}
	void tMaterialPreviewPanel::fOnButtonBack( wxCommandEvent& )
	{
	}
}
