#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tScreen.hpp"
#include "tEdramTilingDesc.hpp"

namespace Sig { namespace Gfx
{

	tScreen::tScreen( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts )
		: tScreenPlatformBase( device, sceneGraph, createOpts )
		, mPlatformWhiteTexture( 0 )
		, mPlatformBlackTexture( 0 )
		, mFrontBufferTexture( 0 )
	{
		sigassert( mDevice->fSingleScreenDevice( ) );

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
		fCreateSwapChain( mLastPresentParams, mCreateOpts.mMultiSamplePower );
	}

	void tScreen::fResize( u32 newBackBufferWidth, u32 newBackBufferHeight )
	{
		mCreateOpts.mBackBufferWidth = newBackBufferWidth;
		mCreateOpts.mBackBufferHeight = newBackBufferHeight;

		fReleaseSwapChainResources( );

		D3DPRESENT_PARAMETERS d3dpp = mLastPresentParams;
		d3dpp.BackBufferWidth = newBackBufferWidth;
		d3dpp.BackBufferHeight = newBackBufferHeight;

		fCreateSwapChain( d3dpp, mCreateOpts.mMultiSamplePower );

		mLastPresentParams = d3dpp;
	}

	void tScreen::fReleaseSwapChainResources( )
	{
		if( mPostEffectMgr )
			mPostEffectMgr->fDestroyRenderTargets( );
		mCurrentTarget.fRelease( );
		mSceneRenderToTexture.fRelease( );
		mPostProcessDepthTexture.fRelease( );
		mScreenSpaceRenderToTexture.fRelease( );
		mWorldDepthTexture.fRelease( );
		mShadowMap0.fRelease( );
		fReleaseComPtr( mFrontBufferTexture );
	}

	void tScreen::fCreateSwapChain( D3DPRESENT_PARAMETERS& d3dpp, u32 multiSamplePower )
	{
		HRESULT hr;
		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );

		const tEdramTilingDesc& tilingDesc = tEdramTilingDesc::fGetDefaultDescFromMSAAPower( multiSamplePower );


		// Set up tiling front buffer texture.
		// These are the size of your desired rendering surface (in this case, the whole screen).
		hr = d3ddev->CreateTexture(
			tilingDesc.mScreenWidth,
			tilingDesc.mScreenHeight,
			1, 0,
			( D3DFORMAT )/*MAKESRGBFMT*/( D3DFMT_LE_X8R8G8B8 ),
			D3DPOOL_DEFAULT,
			&mFrontBufferTexture,
			NULL );
		sigassert( !FAILED( hr ) && mFrontBufferTexture );

		mSceneRenderToTexture.fReset( NEW tRenderToTexture( mDevice, tilingDesc.mScreenWidth, tilingDesc.mScreenHeight, tRenderTarget::cFormatRGBA8, tRenderTarget::cFormatD24S8, multiSamplePower ) );

//if( multiSamplePower == 0 ) // re-use existing depth buffer as we're not MSAA
//	mWorldDepthTexture.fReset( NEW tRenderToTexture( mDevice, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, tRenderTarget::cFormatR32F, mSceneRenderToTexture->fDepthTarget( ) ) );
//else // need to create a separate, non-MSAA depth buffer
//	mWorldDepthTexture.fReset( NEW tRenderToTexture( mDevice, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, tRenderTarget::cFormatR32F, tRenderTarget::cFormatD24S8 ) );
//if( mWorldDepthTexture->fFailed( ) )
//{
//	log_warning( Log::cFlagGraphics, "Failed to create world depth texture" );
//	mWorldDepthTexture.fRelease( );
//}
		const tRenderTarget::tFormat depthTextureFormat = tRenderTarget::cFormatD24S8;
		mPostProcessDepthTexture.fReset( NEW tRenderToTexture( mDevice, mSceneRenderToTexture->fRenderTarget( ), mSceneRenderToTexture->fDepthTarget( ), &depthTextureFormat ) );
		mSceneRenderToTexture->fSetDepthResolveTexture( mPostProcessDepthTexture );

		if( multiSamplePower > 0 )
		{
			// create separate screen space render target (i.e., gui rendering, post processing)
			if( mWorldDepthTexture )
			{
				mScreenSpaceRenderToTexture.fReset( NEW tRenderToTexture( mDevice, mSceneRenderToTexture->fWidth( ), mSceneRenderToTexture->fHeight( ), tRenderTarget::cFormatRGBA8,
					mWorldDepthTexture->fDepthTarget( ) ) );
			}
			else
			{
				mScreenSpaceRenderToTexture.fReset( NEW tRenderToTexture( mDevice, mSceneRenderToTexture->fWidth( ), mSceneRenderToTexture->fHeight( ), tRenderTarget::cFormatRGBA8,
					tRenderTarget::cFormatD24S8 ) );
			}
		}
		else
		{
			// no need for a separate copy if there's no MSAA
			mScreenSpaceRenderToTexture = mSceneRenderToTexture;
		}

		// Set up the screen extents query mode that tiling will use.
		d3ddev->SetScreenExtentQueryMode( tilingDesc.mScreenExtentQueryMode );

		if( multiSamplePower > 0 )
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
		else
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );

		const u32 shadowMapResolution = mCreateOpts.mShadowMapResolution;
		const u32 numLayers = mCreateOpts.mShadowMapLayerCount;
		mShadowMap0.fReset( NEW tRenderToTexture( mDevice, shadowMapResolution, shadowMapResolution, tRenderTarget::cFormatR32F, tRenderTarget::cFormatD24S8, 0, numLayers ) );
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
		const u32 tenPercentW = fRound<u32>( 0.05f * ( f32 )mCreateOpts.mBackBufferWidth );
		const u32 tenPercentH = fRound<u32>( 0.05f * ( f32 )mCreateOpts.mBackBufferHeight );
		return Math::tVec2u( tenPercentW, tenPercentH );
	}

	b32 tScreenPlatformBase::fBeginAllRendering( )
	{
		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );

		fHandleMultiSampleChange( ); // do this before BeginScene
		return SUCCEEDED( d3ddev->BeginScene( ) );
	}

	void tScreenPlatformBase::fEndAllRendering( )
	{
		tScreen& self = fToScreen( );

		IDirect3DDevice9* d3ddev = self.mDevice->fGetDevice( );

		d3ddev->EndScene( );

		mPreSwapTimer.fStop( );

		// have to set back to default before swap
		d3ddev->SetShaderGPRAllocation( 0, 64, 64 );

		// Synchronize to presentation interval before the final resolve, so we do
		// not see tearing at 30Hz or 60Hz presentation intervals.
		if( mCreateOpts.mVsync )
			d3ddev->SynchronizeToPresentationInterval( );

		sigassert( mCurrentTarget );
		mCurrentTarget->fResolve( self, self.mFrontBufferTexture );

		// Swap to the current front buffer, so we can see it on screen.
		d3ddev->Swap( self.mFrontBufferTexture, NULL );

		// clear any bound textures
		tTextureReference::fClearBoundTextures( mDevice, 0, 16 );

		d3ddev->UnsetAll( );
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
		// unsupported on xbox 360
	}

	void tScreen::fEndCaptureDump( )
	{
		// unsupported on xbox 360
	}

	void tScreen::fCaptureFrame( const tFilePathPtr& directory, u32 frameNum, b32 saveAsPng ) const
	{
		// unsupported on xbox 360
	}

}}
#endif//#if defined( platform_xbox360 )

