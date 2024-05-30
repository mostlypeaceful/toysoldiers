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
		mPostProcessDepthTexture.fRelease( );
		mScreenSpaceRenderToTexture.fRelease( );
		mWorldDepthTexture.fRelease( );
		for( u32 i = 0; i < mShadowMaps.fCount( ); ++i )
			mShadowMaps[ i ].fRelease( );
		fReleaseComPtr( mFrontBufferTexture );
	}

	void tScreen::fCreateSwapChain( D3DPRESENT_PARAMETERS& d3dpp )
	{
		// Main swap chain doesnt do AA in deferred shading, GBuffer does.
		const u32 multiSamplePower = fDeferredShading( ) ? 0 : mCreateOpts.mMultiSamplePower;

		HRESULT hr;
		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );

		//create gpu counters
#ifdef sig_profile_xbox_gpu
		for(u32 i = 0; i < 3; ++i)
		{
			d3ddev->CreatePerfCounters( &mGPUPerfCounterStart[ i ], 1 );
			d3ddev->CreatePerfCounters( &mGPUPerfCounterEnd[ i ], 1 );
		}
		d3ddev->EnablePerfCounters( TRUE );

		D3DPERFCOUNTER_EVENTS events;
		fZeroOut( events );
		events.RBBM[0] = GPUPE_RBBM_CP_NRT_BUSY;
		d3ddev->SetPerfCounterEvents( &events, 0 );
#endif//sig_profile_xbox_gpu

		const u32 cScreenWidth = 1280;
		const u32 cScreenHeight = 720;
		const tRenderTarget::tFormat cColorFormat = tRenderTarget::cFormatRGBA8; //tRenderTarget::cFormatRGBA16F; //
		const tRenderTarget::tFormat cDepthFormat = tRenderTarget::cFormatD24FS8;
		
		// Set up tiling front buffer texture.
		// These are the size of your desired rendering surface (in this case, the whole screen).
		Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysTextures ) );
		hr = d3ddev->CreateTexture(
			cScreenWidth,
			cScreenHeight,
			1, 0,
			( D3DFORMAT )/*MAKESRGBFMT*/( D3DFMT_LE_X8R8G8B8 ),
			D3DPOOL_DEFAULT,
			&mFrontBufferTexture,
			NULL );
		sigassert( !FAILED( hr ) && mFrontBufferTexture );
		Memory::tHeap::fResetVramContext( );

		mSceneRenderToTexture.fReset( NEW tRenderToTexture( mDevice, cScreenWidth, cScreenHeight, cColorFormat, cDepthFormat, multiSamplePower ) );
		mPostProcessDepthTexture.fReset( NEW tRenderToTexture( mDevice, mSceneRenderToTexture->fRenderTarget( ), mSceneRenderToTexture->fDepthTarget( ), &cDepthFormat ) );
		mSceneRenderToTexture->fSetDepthResolveTexture( mPostProcessDepthTexture );

		if( multiSamplePower > 0 )
		{
			// create separate screen space render target (i.e., gui rendering, post processing)
			if( mWorldDepthTexture )
			{
				mScreenSpaceRenderToTexture.fReset( NEW tRenderToTexture( mDevice, mSceneRenderToTexture->fWidth( ), mSceneRenderToTexture->fHeight( ), tRenderTarget::cFormatRGBA8, mWorldDepthTexture->fDepthTarget( ), 0 ) );
			}
			else
			{
				mScreenSpaceRenderToTexture.fReset( NEW tRenderToTexture( mDevice, mSceneRenderToTexture->fWidth( ), mSceneRenderToTexture->fHeight( ), tRenderTarget::cFormatRGBA8, tRenderTarget::cFormatD24FS8, 0 ) );
			}
		}
		else
		{
			// no need for a separate copy if there's no MSAA
			mScreenSpaceRenderToTexture = mSceneRenderToTexture;
		}

		// Set up the screen extents query mode that tiling will use.
		d3ddev->SetScreenExtentQueryMode( (D3DSCREENEXTENTQUERYMODE)mSceneRenderToTexture->fRenderTarget( )->fTiling( ).mScreenExtentQueryMode );

		if( multiSamplePower > 0 )
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
		else
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );

		const u32 shadowMapResolution = mCreateOpts.mShadowMapResolution;
		const u32 numLayers = 2; //mCreateOpts.mShadowMapLayerCount;

		for( u32 i = 0; i < mShadowMaps.fCount( ); ++i )
		{
			mShadowMaps[ i ].fReset( NEW tRenderToTexture( mDevice, shadowMapResolution, shadowMapResolution, tRenderTarget::cFormatD24FS8, tRenderTarget::cFormatD24FS8, 0, numLayers ) );
			mShadowMaps[ i ]->fTexture( ).fSetSamplingModes( tTextureFile::cFilterModeNone, tTextureFile::cAddressModeBorderBlack );
			mShadowMaps[ i ]->fSetDepthResolveTexture( mShadowMaps[ i ] );
		}

		if( mPostEffectMgr )
			mPostEffectMgr->fCreateRenderTargets( *this );
	}

	void tScreenPlatformBase::fGetWorldDepthSetup( f32& minZ, f32& maxZ )
	{
		if( mWorldRendering )
		{
			// reversed to take advantage of hi-z on xbox.
			minZ = 1.f;
			maxZ = 0.f;
		}
		else
		{
			minZ = 0.f;
			maxZ = 1.f;
		}
	}

	void tScreenPlatformBase::fSetViewportClipBox( u32 x, u32 y, u32 width, u32 height )
	{
		tScreen& self = fToScreen( );

		f32 minZ, maxZ;
		fGetWorldDepthSetup( minZ, maxZ );
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
		if( mCreateOpts.mVsync != VSYNC_NONE )
			d3ddev->SynchronizeToPresentationInterval( );

		sigassert( mCurrentTarget );
		mCurrentTarget->fResolve( self, self.mFrontBufferTexture );

		// Swap to the current front buffer, so we can see it on screen.
		d3ddev->Swap( self.mFrontBufferTexture, NULL );

		// clear any bound textures
		tTextureReference::fClearBoundTextures( mDevice, 0, 16 );

		d3ddev->UnsetAll( );
	}

	void tScreenPlatformBase::fClearCurrentRenderTargets( b32 clearColor, b32 clearDepth, const Math::tVec4f& rgbaClear, u32 stencilClear, b32 gBufferClear )
	{
		IDirect3DDevice9* d3ddev = mDevice->fGetDevice( );

		f32 minZ, maxZ;
		fToScreen( ).fGetWorldDepthSetup( minZ, maxZ );

		if( clearColor )
			mLastClearRGBA = rgbaClear;
		
		tRenderToTexture* target = mCurrentTarget.fGetRawPtr( );
		if( target && target->fRenderTarget( 0 )->fTiling( ).mTileCount > 0 )
		{
			const tEdramTilingDesc& tiling = target->fRenderTarget( 0 )->fTiling( );

			D3DVECTOR4 color = { rgbaClear.x, rgbaClear.y, rgbaClear.z, rgbaClear.w };
			const u32 clearFlags = ( (clearColor && mCurrentTarget) ? D3DCLEAR_TARGET0 : 0 ) | ( clearDepth ? ( D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL ) : 0 );
			
			d3ddev->SetPredication( D3DPRED_TILE( 0 ) );
			d3ddev->ClearF( clearFlags, &tiling.mTilingRects[0], &color, maxZ, stencilClear );
			
			if( clearColor )
			{
				for( u32 i = 1; i < target->fTargetCount( ); ++i )
				{
					u32 enumVal;
					switch( i )
					{
					case 1: enumVal = D3DCLEAR_TARGET1; break;
					case 2: enumVal = D3DCLEAR_TARGET2; break;
					case 3: enumVal = D3DCLEAR_TARGET3; break;
					default: sigassert( !"Invalid target to clear!" );
					};

					d3ddev->ClearF( enumVal, &tiling.mTilingRects[0], &color, maxZ, stencilClear );
				}
			}

			d3ddev->SetPredication( 0 );
		}
		else
		{
			D3DCOLOR color = D3DCOLOR_ARGB((u8)(rgbaClear.w*255.f),(u8)(rgbaClear.x*255.f),(u8)(rgbaClear.y*255.f),(u8)(rgbaClear.z*255.f));
			const u32 clearFlags = ( (clearColor && mCurrentTarget) ? D3DCLEAR_TARGET : 0 ) | ( clearDepth ? ( D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER ) : 0 );
			d3ddev->Clear( 0, 0, clearFlags, color, maxZ, stencilClear );
		}
		
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

	void tScreen::fRenderWorldOpaqueBegin( )
	{
		mDevice->fGetDevice( )->SetRenderState( D3DRS_HIZENABLE, D3DHIZ_AUTOMATIC );
		mDevice->fGetDevice( )->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATEREQUAL );
		mDevice->fGetDevice( )->BeginZPass( 0 );
	}

	void tScreen::fRenderWorldOpaqueEnd( )
	{
		mDevice->fGetDevice( )->EndZPass( );
	}

	void tScreen::fCaptureFrame( const tFilePathPtr& directory, u32 frameNum, b32 saveAsPng ) const
	{
		// unsupported on xbox 360
	}

}}
#endif//#if defined( platform_xbox360 )

