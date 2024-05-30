#include "BasePch.hpp"
#if defined( platform_ios )
#include "tRenderToTexture.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{
	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, tRenderTarget::tFormat depthFormat, u32 multiSamplePower, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
	{
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, const tRenderTargetPtr& depthTarget, u32 multiSamplePower, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
	{
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTargetPtr& depthTarget, const tRenderTarget::tFormat* textureFormatOverride, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
	{
		sigassert( renderTarget && !renderTarget->fFailed( ) );
		sigassert( depthTarget && !depthTarget->fFailed( ) );
		sigassert( depthTarget->fWidth( ) >= renderTarget->fWidth( ) && depthTarget->fHeight( ) >= renderTarget->fHeight( ) );

		fCreateTextureFromRenderTarget( device, renderTarget, textureFormatOverride );

		if( mRt )
			mDt = depthTarget;
	}

	tRenderToTexture::~tRenderToTexture( )
	{
		fReleaseAll( );
	}

	b32 tRenderToTexturePlatformBase::fCanRenderToSelf( ) const
	{
		return true;
	}

	b32 tRenderToTexturePlatformBase::fResolveClearsTarget( ) const
	{
		return true;
	}

	void tRenderToTexture::fApply( tScreen& screen, u32 index )
	{
		tRenderToTexturePlatformBase::fApply( screen, index );
	}

	void tRenderToTexture::fResolve( tScreen& screen, void* targetOverride, u32 slice )
	{
	}

	void tRenderToTexture::fCreateTextureAndRenderTarget( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, u32 multiSamplePower )
	{
		mRt.fReset( NEW tRenderTarget( device, width, height, format, multiSamplePower ) );
		fCreateTextureFromRenderTarget( device, mRt );
	}

	void tRenderToTexture::fCreateTextureFromRenderTarget( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTarget::tFormat* textureFormatOverride )
	{
	}

	void tRenderToTexture::fCreateTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, u32 numLayers )
	{
//		IDirect3DDevice9* d3ddev = device->fGetDevice( );
//
//		D3DFORMAT d3dFormat = tRenderTarget::fConvertFormatType( format );
//		if( format == tRenderTarget::cFormatNull && !device->fSupportsRenderTargetFormat( d3dFormat ) )
//			d3dFormat = D3DFMT_X8R8G8B8; // fallback
//
//		HRESULT hr;
//		const s32 numMips = 1;
//
//		if( numLayers > 1 )
//		{
//			IDirect3DArrayTexture9* realTexture = 0;
//			hr = d3ddev->CreateArrayTexture( width, height, numLayers, numMips, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &realTexture, 0 );
//			mRealTexture = realTexture;
//		}
//		else
//		{
//			sigassert( numLayers == 1 );
//
//			IDirect3DTexture9* realTexture = 0;
//			hr = d3ddev->CreateTexture( width, height, numMips, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &realTexture, 0 );
//			mRealTexture = realTexture;
//		}
//
//		if( FAILED( hr ) && mRealTexture )
//			fReleaseComPtr( mRealTexture );
	}

	void tRenderToTexture::fReleaseAll( )
	{
		mRt.fRelease( );
		mDt.fRelease( );
	}


}}
#endif//#if defined( platform_ios )
