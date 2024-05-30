#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{
	devvar( bool, Renderer_Debug_SimpleShadows, false ); 

	tScreen::tScreen( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts )
		: tScreenPlatformBase( device, sceneGraph, createOpts )
		, mSwapChain( 0 )
		, mPlatformWhiteTexture( 0 )
		, mPlatformBlackTexture( 0 )
		, mCaptureTextureScene( 0 )
	{
		D3DPRESENT_PARAMETERS d3dpp;
		tDevice::fCreatePresentParams( d3dpp, createOpts );

		fCreateSwapChain( d3dpp );

		mLastPresentParams = d3dpp;
		mCreateOpts = createOpts;

		mPlatformWhiteTexture = device->fCreateSolidColorTexture( 1, 1, Math::tVec4f( 1.f, 1.f, 1.f, 1.f ) );
		mPlatformBlackTexture = device->fCreateSolidColorTexture( 1, 1, Math::tVec4f( 0.f, 0.f, 0.f, 1.f ) );
		mWhiteTexture = ( tPlatformTextureHandle )mPlatformWhiteTexture;
		mBlackTexture = ( tPlatformTextureHandle )mPlatformBlackTexture;

		fRegisterWithDevice( device.fGetRawPtr( ) );

		fInitailize( );
	}

	tScreen::~tScreen( )
	{
		fReleaseComPtr( mPlatformWhiteTexture );
		fReleaseComPtr( mPlatformBlackTexture );
		fReleaseSwapChainResources( );
	}

	tCamera fCreateMiniMapCamera( const tScreen* screen, const Math::tAabbf& box, const tRenderToTexture& tex )
	{
		tCamera camera = screen->fViewport( 0 )->fRenderCamera( );

		//generate a constant scale factor to apply to both width/height of our bounding volume.
		// we want to shrink it down to fit inside the texture and we don't want any stretching to occur.
		const f32 constantScaleFactor = Math::fSqrt( ( tex.fWidth( ) * box.fHeight( ) ) / ( tex.fHeight( ) * box.fWidth( ) ) );

		const f32 biggestAxis = fMax( box.fWidth( ), box.fDepth( ) );
		const f32 halfAxis = biggestAxis / 2.0f * constantScaleFactor;

		//ortho camera projection
		const tLens oldLens = camera.fGetLens( );
		tLens lens;
		lens.fSetOrtho( oldLens.mNearPlane, oldLens.mFarPlane, box.fWidth( ) * -0.5f, box.fWidth( ) * 0.5f, box.fDepth( ) * -0.5f, box.fDepth( ) * 0.5f );
		camera.fSetLens( lens );

		//generate new camera matrix
		Math::tMat3f camMat = Math::tMat3f::cIdentity;
		Math::tVec3f camPos = box.fComputeCenter( );
		camPos.y = box.mMax.y + lens.mNearPlane;//in ortho, this just needs to be enough to ensure all objects are past our near plane
		log_line( 0, "Rendering minimap from: " << camPos );
		camMat.fTranslateGlobal( camPos );
		camMat.fOrientZAxis( -Math::tVec3f::cYAxis, -Math::tVec3f::cZAxis );
		camera.fSetTripod( tTripod( camMat ) );

		return camera;
	}

	void tScreen::fRenderMiniMap( tRenderToTexture& tex, const Math::tAabbf& box )
	{
		//setup new camera/rtt
		const tRenderToTexturePtr oldSceneTex = mSceneRenderToTexture;
		const tCamera oldCamera = fViewport( 0 )->fRenderCamera( );
		const Math::tVec4f oldClearColor = mCreateOpts.mDefaultClearColor;
		mCreateOpts.mDefaultClearColor = Math::tVec4f::cZeroVector;

		//modify camera/rtt
		mSceneRenderToTexture.fReset( &tex );
		fViewport( 0 )->fSetCameras( fCreateMiniMapCamera( this, box, tex ) );

		//render everything
		b32 begin = fBeginAllRendering( );
		sigassert( begin );

		tex.fApply( *this );

		mSceneGraph->fPrepareEntityClouds( mViewports.fFront( )->fRenderCamera( ) );
		for( u32 i = 0; i < mViewportDisplayList.fCount( ); ++i )
			mViewportDisplayList[ i ].fReset( );

		fBuildLightLists( );
		fBuildDisplayLists( );

		mWorldRendering = true;
		fRenderShadowMaps( );
		fRenderWorldDepth( );
		fRenderWorld( );
		mWorldRendering = false;

		fRenderPostEffects( );
		fRenderScreenSpace( );		

		fEndAllRendering( );


		//return to previous state
		mCreateOpts.mDefaultClearColor = oldClearColor;
		mSceneRenderToTexture = oldSceneTex;
		fViewport( 0 )->fSetCameras( oldCamera );
	}

	void tScreen::fOnDeviceLost( tDevice* device )
	{
		fReleaseSwapChainResources( );
	}

	void tScreen::fOnDeviceReset( tDevice* device )
	{
		fCreateSwapChain( mLastPresentParams );
	}

	void tScreen::fResize( u32 newBackBufferWidth, u32 newBackBufferHeight )
	{
		mCreateOpts.mBackBufferWidth = newBackBufferWidth;
		mCreateOpts.mBackBufferHeight = newBackBufferHeight;

		fReleaseSwapChainResources( );

		D3DPRESENT_PARAMETERS d3dpp = mLastPresentParams;
		d3dpp.BackBufferWidth = newBackBufferWidth;
		d3dpp.BackBufferHeight = newBackBufferHeight;

		fCreateSwapChain( d3dpp );

		mLastPresentParams = d3dpp;
	}

	void tScreen::fReleaseSwapChainResources( )
	{
		if( mPostEffectMgr )
			mPostEffectMgr->fDestroyRenderTargets( );
		mCurrentTarget.fRelease( );
		mSceneRenderToTexture.fRelease( );
		mScreenSpaceRenderToTexture.fRelease( );
		mWorldDepthTexture.fRelease( );
		for( u32 i = 0; i < mShadowMaps.fCount( ); ++i )
			mShadowMaps[ i ].fRelease( );
		fReleaseComPtr( mCaptureTextureScene );
		fReleaseComPtr( mSwapChain );
	}

	void tScreen::fCreateSwapChain( D3DPRESENT_PARAMETERS& d3dpp )
	{
		// Main swap chain doesnt do AA in deferred shading, GBuffer does.
		const u32 multiSamplePower = fDeferredShading( ) ? 0 : mCreateOpts.mMultiSamplePower;

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
			hr = d3ddev->CreateAdditionalSwapChain( &d3dpp, &mSwapChain );
			sigassert( !FAILED( hr ) && mSwapChain );

			hr = mSwapChain->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &rtSceneSurface );
			sigassert( !FAILED( hr ) && rtSceneSurface );
		}

		sigassert( rtSceneSurface );
		tRenderTargetPtr rtScene( NEW tRenderTarget( rtSceneSurface, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, false, tRenderTarget::cFormatRGBA8, multiSamplePower ) );
		sigassert( !rtScene->fFailed( ) );

		tRenderTargetPtr rtDepth;
		if( dtSceneSurface )
			rtDepth.fReset( NEW tRenderTarget( dtSceneSurface, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, true, tRenderTarget::cFormatD24S8, multiSamplePower ) );
		else
			rtDepth.fReset( NEW tRenderTarget( mDevice, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, true, tRenderTarget::cFormatD24S8, multiSamplePower ) );
		sigassert( !rtDepth->fFailed( ) );

		mSceneRenderToTexture.fReset( NEW tRenderToTexture( mDevice, rtScene, rtDepth, multiSamplePower ) );
		sigassert( !mSceneRenderToTexture->fFailed( ) );

		mWorldDepthTexture.fReset( NEW tRenderToTexture( mDevice, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, tRenderTarget::cFormatR32F, tRenderTarget::cFormatD24S8, 0 ) );
		if( mWorldDepthTexture->fFailed( ) )
		{
			log_warning( "Failed to create world depth texture" );
			mWorldDepthTexture.fRelease( );
		}

#if !defined( target_tools )
		mScreenSpaceRenderToTexture.fReset( NEW tRenderToTexture( 
			mDevice, 
			mSceneRenderToTexture->fWidth( ), 
			mSceneRenderToTexture->fHeight( ),
			tRenderTarget::cFormatRGBA8,
			mWorldDepthTexture->fDepthTarget( ),
			0 ) );
#endif//!defined( target_tools )

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
#ifdef target_game
		const u32 numLayers = Renderer_Debug_SimpleShadows ? 1 : 2; //mCreateOpts.mShadowMapLayerCount;
#else
		const u32 numLayers = 1;
#endif

		for( u32 i = 0; i < mShadowMaps.fCount( ); ++i )
		{
			mShadowMaps[ i ].fReset( NEW tRenderToTexture( mDevice, shadowMapResolution, shadowMapResolution, tRenderTarget::cFormatR32F, tRenderTarget::cFormatD24S8, 0, numLayers ) );
			mShadowMaps[ i ]->fTexture( ).fSetSamplingModes( tTextureFile::cFilterModeNone, tTextureFile::cAddressModeBorderWhite );
		}

		if( mPostEffectMgr )
			mPostEffectMgr->fCreateRenderTargets( *this );

		if( mDeferredShadingMaterial )
			fSetupDeferredShading( );
	}

	void tScreenPlatformBase::fSetViewportClipBox( u32 x, u32 y, u32 width, u32 height )
	{
		tScreen& self = fToScreen( );

		f32 minZ, maxZ;
		self.fGetWorldDepthSetup( minZ, maxZ );
		tRenderState::gInvertedViewportDepth = (minZ > maxZ);

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
		const u32 tenPercentW = fRound<u32>( 0.00f * ( f32 )mCreateOpts.mBackBufferWidth );
		const u32 tenPercentH = fRound<u32>( 0.00f * ( f32 )mCreateOpts.mBackBufferHeight );
		return Math::tVec2u( tenPercentW, tenPercentH );
	}

	void tScreenPlatformBase::fGetWorldDepthSetup( f32& minZ, f32& maxZ )
	{
		minZ = 0.f;
		maxZ = 1.f;
	}

	b32 tScreenPlatformBase::fDeviceCheck( )
	{
		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );
		if( d3ddev->TestCooperativeLevel( ) == D3DERR_DEVICELOST )
			return false;
		else if( d3ddev->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET )
		{
			mDevice->fReset( );
			mScreenSpaceDL.fInvalidate( );
			return false;
		}

		return true;
	}

	b32 tScreenPlatformBase::fBeginAllRendering( )
	{
		// This has been moved into fDeviceCheck() which should be used before
		// any rendering set up has occurred.
		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );
		//if( d3ddev->TestCooperativeLevel( ) == D3DERR_DEVICELOST )
		//	return false;
		//else if( d3ddev->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET )
		//{
		//	mDevice->fReset( );
		//	mScreenSpaceDL.fInvalidate( );
		//	return false;
		//}

		fHandleMultiSampleChange( ); // do this before BeginScene
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

	void tScreenPlatformBase::fClearCurrentRenderTargets( b32 clearColor, b32 clearDepth, const Math::tVec4f& rgbaClear, u32 stencilClear, b32 gBufferClear )
	{
		f32 minZ, maxZ;
		fToScreen( ).fGetWorldDepthSetup( minZ, maxZ );

		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );
		const u32 clearFlags = ( (clearColor && mCurrentTarget) ? D3DCLEAR_TARGET : 0 ) | ( clearDepth ? ( D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER ) : 0 );
		const u32 colorClear = D3DCOLOR_ARGB((u8)(rgbaClear.w*255.f),(u8)(rgbaClear.x*255.f),(u8)(rgbaClear.y*255.f),(u8)(rgbaClear.z*255.f));

		if( gBufferClear )
		{
			// Most of the gbuffer panels need to be black. The clear color should go into the emissive.
			// As far as i can tell, there is no good way to clear one target on dx9.
			// Remove targets until there is only one bound, and clear them one at at time.

			const u32 blackClear = D3DCOLOR_ARGB(0,0,0,0);
			const u32 cEmissiveBuffer = 2; //tDeferredShadingMaterial			
			sigassert( mCurrentTarget && mCurrentTarget->fTargetCount( ) > cEmissiveBuffer );

			// release emmisive buffer.
			tRenderTarget::fReset( fGetDevice( ), cEmissiveBuffer );

			// clear all remaining to black
			d3ddev->Clear( 0, 0, clearFlags, blackClear, maxZ, stencilClear );
			
			// remove non emmisive buffers.
			for( u32 i = 1; i < mCurrentTarget->fTargetCount( ); ++i )
			{
				if( i != cEmissiveBuffer )
					tRenderTarget::fReset( fGetDevice( ), i );
			}

			// apply emissive and clear to clear color.
			mCurrentTarget->fRenderTarget( cEmissiveBuffer )->fApply( mDevice, 0 );
			d3ddev->Clear( 0, 0, D3DCLEAR_TARGET, colorClear, maxZ, stencilClear );

			// reapply whole target.
			mCurrentTarget->fApply( fToScreen( ) );
		}
		else
			d3ddev->Clear( 0, 0, clearFlags, colorClear, maxZ, stencilClear );

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
			return;

		SYSTEMTIME sysTime = {0};
		GetLocalTime( &sysTime );

		tScreenPlatformBase::fBeginCaptureDump( folder, sysTime.wMonth, sysTime.wDay, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond );

		fResize( mLastPresentParams.BackBufferWidth, mLastPresentParams.BackBufferHeight );
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

