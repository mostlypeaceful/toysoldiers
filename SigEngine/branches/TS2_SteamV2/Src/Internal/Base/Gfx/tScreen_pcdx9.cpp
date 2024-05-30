#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{

	tScreen::tScreen( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts )
		: tScreenPlatformBase( device, sceneGraph, createOpts )
		, mSwapChain( 0 )
		, mPlatformWhiteTexture( 0 )
		, mPlatformBlackTexture( 0 )
		, mCaptureTextureScene( 0 )
	{
		D3DPRESENT_PARAMETERS d3dpp;
		tDevice::fCreatePresentParams( d3dpp, createOpts, mDevice->fSingleScreenDevice( )  );

		fCreateSwapChain( d3dpp, createOpts.mMultiSamplePower );

		mLastPresentParams = d3dpp;
		mCreateOpts = createOpts;

		mPlatformWhiteTexture = device->fCreateSolidColorTexture( 1, 1, Math::tVec4f( 1.f, 1.f, 1.f, 1.f ) );
		mPlatformBlackTexture = device->fCreateSolidColorTexture( 1, 1, Math::tVec4f( 0.f, 0.f, 0.f, 1.f ) );
		mWhiteTexture = ( tPlatformTextureHandle )mPlatformWhiteTexture;
		mBlackTexture = ( tPlatformTextureHandle )mPlatformBlackTexture;

		fRegisterWithDevice( device.fGetRawPtr( ) );
	}

	tScreen::~tScreen( )
	{
		fReleaseComPtr( mPlatformWhiteTexture );
		fReleaseComPtr( mPlatformBlackTexture );
		fReleaseSwapChainResources( );
	}

	void tScreen::fOnDeviceLost( tDevice* device )
	{
		fReleaseSwapChainResources( );
	}

	void tScreen::fOnDeviceReset( tDevice* device )
	{
		mLastPresentParams = mDevice->fCreationPresentParams();
		mCreateOpts.mBackBufferWidth = mLastPresentParams.BackBufferWidth;
		mCreateOpts.mBackBufferHeight = mLastPresentParams.BackBufferHeight;
		fCreateSwapChain( mLastPresentParams, mCreateOpts.mMultiSamplePower );
		fSetupScreenSpaceCamera( );
	}

	void tScreen::fResize( u32 newBackBufferWidth, u32 newBackBufferHeight, const u32* multiSamplePowerOverride )
	{
		mCreateOpts.mBackBufferWidth = newBackBufferWidth;
		mCreateOpts.mBackBufferHeight = newBackBufferHeight;

		fReleaseSwapChainResources( );

		D3DPRESENT_PARAMETERS d3dpp = mLastPresentParams;
		d3dpp.BackBufferWidth = newBackBufferWidth;
		d3dpp.BackBufferHeight = newBackBufferHeight;

		const u32 multiSamplePower = multiSamplePowerOverride ? *multiSamplePowerOverride : mCreateOpts.mMultiSamplePower;
		d3dpp.MultiSampleType = tDevice::fConvertMultiSampleType( multiSamplePower );

		fCreateSwapChain( d3dpp, multiSamplePower );

		mLastPresentParams = d3dpp;
		fSetupScreenSpaceCamera( );
	}

	void tScreen::fReleaseSwapChainResources( )
	{
		if( mPostEffectMgr )
			mPostEffectMgr->fDestroyRenderTargets( );
		mCurrentTarget.fRelease( );
		mSceneRenderToTexture.fRelease( );
		mScreenSpaceRenderToTexture.fRelease( );
		mWorldDepthTexture.fRelease( );
		mShadowMap0.fRelease( );
		fReleaseComPtr( mCaptureTextureScene );
		fReleaseComPtr( mSwapChain );
	}

	void tScreen::fUpdateShadowSize( b32 enabled, int layers, int resolution )
	{
		// enabled - not currently supported.
		mCreateOpts.mShadowMapResolution = resolution;

		// note that PC only supports 1 layer.
		mCreateOpts.mShadowMapLayerCount = layers;
	}

	void tScreen::fCreateSwapChain( D3DPRESENT_PARAMETERS& d3dpp, u32 multiSamplePower )
	{
		HRESULT hr;
		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );

		IDirect3DSurface9* rtSceneSurface = 0, *dtSceneSurface = 0;
		if( mDevice->fSingleScreenDevice( ) )
		{
			hr = d3ddev->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &rtSceneSurface );
			sigassert( !FAILED( hr ) && rtSceneSurface );

			hr = d3ddev->GetDepthStencilSurface( &dtSceneSurface );
			sigassert( !FAILED( hr ) && dtSceneSurface );
		}
		else
		{
			d3dpp.FullScreen_RefreshRateInHz = d3dpp.Windowed ? 0 : 60;
			d3dpp.PresentationInterval = 1;
			hr = d3ddev->CreateAdditionalSwapChain( &d3dpp, &mSwapChain );

			if (FAILED(hr))
			{
				if (hr == D3DERR_NOTAVAILABLE)
				{
					log_line(0,"Not Avail");
				}
				else if (hr == D3DERR_DEVICELOST)
				{
					log_line(0,"Lost");
				}
				else if (hr == D3DERR_INVALIDCALL)
				{
					log_line(0,"invalid");
				}
				else if (hr == D3DERR_OUTOFVIDEOMEMORY)
				{
					log_line(0,"out");
				}
				else
				{
					log_line(0,"bleh");
				}
			}

			sigassert( !FAILED( hr ) && mSwapChain );

			hr = mSwapChain->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &rtSceneSurface );
			sigassert( !FAILED( hr ) && rtSceneSurface );
		}

		sigassert( rtSceneSurface );
		tRenderTargetPtr rtScene( NEW tRenderTarget( rtSceneSurface, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, tRenderTarget::cFormatRGBA8, multiSamplePower ) );
		sigassert( !rtScene->fFailed( ) );

		tRenderTargetPtr rtDepth;
		if( dtSceneSurface )
			rtDepth.fReset( NEW tRenderTarget( dtSceneSurface, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, tRenderTarget::cFormatD24S8, multiSamplePower ) );
		else
			rtDepth.fReset( NEW tRenderTarget( mDevice, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, tRenderTarget::cFormatD24S8, multiSamplePower ) );
		sigassert( !rtDepth->fFailed( ) );

		mSceneRenderToTexture.fReset( NEW tRenderToTexture( mDevice, rtScene, rtDepth ) );
		sigassert( !mSceneRenderToTexture->fFailed( ) );

		if( multiSamplePower == 0 ) // re-use existing depth buffer as we're not MSAA
			mWorldDepthTexture.fReset( NEW tRenderToTexture( mDevice, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, tRenderTarget::cFormatR32F, rtDepth ) );
		else // need to create a separate, non-MSAA depth buffer
			mWorldDepthTexture.fReset( NEW tRenderToTexture( mDevice, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, tRenderTarget::cFormatR32F, tRenderTarget::cFormatD24S8 ) );
		if( mWorldDepthTexture->fFailed( ) )
		{
			log_warning( Log::cFlagGraphics, "Failed to create world depth texture" );
			mWorldDepthTexture.fRelease( );
		}

#if !defined( target_tools )
		mScreenSpaceRenderToTexture.fReset( NEW tRenderToTexture( 
			mDevice, 
			mSceneRenderToTexture->fWidth( ), 
			mSceneRenderToTexture->fHeight( ),
			tRenderTarget::cFormatRGBA8,
			mWorldDepthTexture->fDepthTarget( ) ) );
#endif//!defined( target_tools )

		if( multiSamplePower > 0 )
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
		else
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );

		if( fCapturing( ) )
		{
			hr = d3ddev->CreateTexture( 
				d3dpp.BackBufferWidth,
				d3dpp.BackBufferHeight,
				1,
				0,
				d3dpp.BackBufferFormat,
				D3DPOOL_SYSTEMMEM,
				&mCaptureTextureScene,
				0 );
			sigassert( !FAILED(hr) && mCaptureTextureScene );
		}

		const u32 shadowMapResolution = mCreateOpts.mShadowMapResolution;

		mShadowMap0.fReset( NEW tRenderToTexture( mDevice, shadowMapResolution, shadowMapResolution, tRenderTarget::cFormatR32F, tRenderTarget::cFormatD24S8 ) );
		mShadowMap0->fSetSamplingModes( tTextureFile::cFilterModeNone, tTextureFile::cAddressModeBorderWhite );

		if( mPostEffectMgr )
			mPostEffectMgr->fCreateRenderTargets( *this );
	}

	void tScreenPlatformBase::fSetViewportClipBox( u32 x, u32 y, u32 width, u32 height, f32 minZ, f32 maxZ )
	{
		tScreen& self = fToScreen( );

		IDirect3DDevice9* d3ddev = self.mDevice->fGetDevice( );

		D3DVIEWPORT9 vp = {0};

		vp.X = x;
		vp.Y = y;
		vp.Width = width;
		vp.Height = height;
		vp.MinZ = minZ;
		vp.MaxZ = maxZ;

		d3ddev->SetViewport( &vp );
	}

	Math::tVec2u tScreenPlatformBase::fComputeGuiSafeEdge( ) const
	{
		//JPodesta - assume GUI is always fits in 1280x720 ortho coords  - black borders will appear around it on other aspect ratios
		const u32 tenPercentW = fRound<u32>( 0.05f * 1280.0f );//( f32 )mCreateOpts.mBackBufferWidth );
		const u32 tenPercentH = fRound<u32>( 0.05f * 720.0f );//( f32 )mCreateOpts.mBackBufferHeight );
		return Math::tVec2u( tenPercentW, tenPercentH );
	}

	b32 tScreenPlatformBase::fBeginAllRendering( )
	{
		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );
		if( d3ddev->TestCooperativeLevel( ) == D3DERR_DEVICELOST )
			return false;
		else if( d3ddev->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET )
		{
			mDevice->fReset( );
			return false;
		}
		return SUCCEEDED( d3ddev->BeginScene( ) );
	}

	void tScreenPlatformBase::fEndAllRendering( )
	{

		tScreen& self = fToScreen( );


		IDirect3DDevice9* d3ddev = self.mDevice->fGetDevice( );

		d3ddev->EndScene( );

		mPreSwapTimer.fStop( );

		if( fCheckForAutoCaptureTermination( ) )
			fEndCaptureDump( );
		if( fCapturing( ) )

		{
			self.fCaptureFrame( mCaptureData->mFolder, mCaptureData->mFrame, false );
			++mCaptureData->mFrame;
		}

		if( self.mSwapChain ) // multi-swapchain/windowed rendering
			self.mSwapChain->Present( 0, 0, 0, 0, 0 );
		else // full-screen/single-swapchain rendering
			d3ddev->Present( 0, 0, 0, 0 );
	}

	void tScreenPlatformBase::fClearCurrentRenderTargets( b32 clearColor, b32 clearDepth, const Math::tVec4f& rgbaClear, f32 zClear, u32 stencilClear )
	{
		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );
		const u32 clearFlags = ( clearColor ? D3DCLEAR_TARGET : 0 ) | ( clearDepth ? ( D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER ) : 0 );
		d3ddev->Clear( 0, 0, clearFlags, D3DCOLOR_ARGB((u8)(rgbaClear.w*255.f),(u8)(rgbaClear.x*255.f),(u8)(rgbaClear.y*255.f),(u8)(rgbaClear.z*255.f)), zClear, stencilClear );
		d3ddev->SetTexture( 0, 0 );
	}

	void tScreenPlatformBase::fDisableMSAA( )
	{
		tScreen& self = fToScreen( );
		IDirect3DDevice9* d3ddev = self.mDevice->fGetDevice( );
		d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );
	}

	void tScreenPlatformBase::fReEnableMSAA( )
	{
		tScreen& self = fToScreen( );
		IDirect3DDevice9* d3ddev = self.mDevice->fGetDevice( );
		if( self.mLastPresentParams.MultiSampleType == D3DMULTISAMPLE_NONE )
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );
		else
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
	}

	void tScreen::fBeginCaptureDump( const tFilePathPtr& folder )
	{
		if( mCreateOpts.mFullScreen )
			return; // TODO currently we don't support changing multi sample mode while in full screen

		SYSTEMTIME sysTime = {0};
		GetLocalTime( &sysTime );

		tScreenPlatformBase::fBeginCaptureDump( folder, sysTime.wMonth, sysTime.wDay, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond );

		u32 forceNoMultiSample = 0;
		fResize( mLastPresentParams.BackBufferWidth, mLastPresentParams.BackBufferHeight, &forceNoMultiSample );
	}

	void tScreen::fEndCaptureDump( )
	{
		tScreenPlatformBase::fEndCaptureDump( );
		fResize( mLastPresentParams.BackBufferWidth, mLastPresentParams.BackBufferHeight );
	}

	void tScreen::fCaptureFrame( const tFilePathPtr& directory, u32 frameNum, b32 saveAsPng ) const
	{
		const D3DXIMAGE_FILEFORMAT fileFormat = saveAsPng ? D3DXIFF_PNG : D3DXIFF_BMP;
		const char* extension = saveAsPng ? ".png" : ".bmp";

		const tFilePathPtr sceneName = fCreateCaptureDumpPath( directory, "scene", extension, frameNum );
		const tFilePathPtr depthName = fCreateCaptureDumpPath( directory, "depth", ".dds", frameNum );

		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );

		IDirect3DSurface9* frameCaptureSurface = 0;

		// grab top mip of sysmem texture
		mCaptureTextureScene->GetSurfaceLevel( 0, &frameCaptureSurface );

		// copy render target from vram to sysmem
		D3DXLoadSurfaceFromSurface( frameCaptureSurface, 0, 0, mSceneRenderToTexture->fRenderTarget( )->fGetSurface( ), 0, 0, D3DX_DEFAULT, 0 );

		// save to file
		D3DXSaveSurfaceToFile( sceneName.fCStr( ) , fileFormat, frameCaptureSurface, 0, 0 );

		// release top mip
		frameCaptureSurface->Release( );

		fCaptureCurrentCameras( );
	}

}}
#endif//#if defined( platform_pcdx9 )

