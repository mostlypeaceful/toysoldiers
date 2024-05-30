#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tRenderToTexture.hpp"
#include "tScreen.hpp"
#include "tEdramTilingDesc.hpp"
#include "xgraphics.h"

namespace Sig { namespace Gfx
{

	namespace
	{
		// Note: this "As16SRGB" shite is ripped from the ATG source.

		// Linear to high-precision texture format mapping table. Maps GPU standard formats to equivalent high-precision sampling format
		// (i.e. _AS_16_16_16_16 etc). Used to create good sRGB formats for textures.
		// Any entry that has no mapping just maps to the same format value.
		const DWORD gMapLinearToSrgbGpuFormat[] = 
		{
			GPUTEXTUREFORMAT_1_REVERSE,
			GPUTEXTUREFORMAT_1,
			GPUTEXTUREFORMAT_8,
			GPUTEXTUREFORMAT_1_5_5_5,
			GPUTEXTUREFORMAT_5_6_5,
			GPUTEXTUREFORMAT_6_5_5,
			GPUTEXTUREFORMAT_8_8_8_8_AS_16_16_16_16,
			GPUTEXTUREFORMAT_2_10_10_10_AS_16_16_16_16,
			GPUTEXTUREFORMAT_8_A,
			GPUTEXTUREFORMAT_8_B,
			GPUTEXTUREFORMAT_8_8,
			GPUTEXTUREFORMAT_Cr_Y1_Cb_Y0_REP,     
			GPUTEXTUREFORMAT_Y1_Cr_Y0_Cb_REP,      
			GPUTEXTUREFORMAT_16_16_EDRAM,          
			GPUTEXTUREFORMAT_8_8_8_8_A,
			GPUTEXTUREFORMAT_4_4_4_4,
			GPUTEXTUREFORMAT_10_11_11_AS_16_16_16_16,
			GPUTEXTUREFORMAT_11_11_10_AS_16_16_16_16,
			GPUTEXTUREFORMAT_DXT1_AS_16_16_16_16,
			GPUTEXTUREFORMAT_DXT2_3_AS_16_16_16_16,  
			GPUTEXTUREFORMAT_DXT4_5_AS_16_16_16_16,
			GPUTEXTUREFORMAT_16_16_16_16_EDRAM,
			GPUTEXTUREFORMAT_24_8,
			GPUTEXTUREFORMAT_24_8_FLOAT,
			GPUTEXTUREFORMAT_16,
			GPUTEXTUREFORMAT_16_16,
			GPUTEXTUREFORMAT_16_16_16_16,
			GPUTEXTUREFORMAT_16_EXPAND,
			GPUTEXTUREFORMAT_16_16_EXPAND,
			GPUTEXTUREFORMAT_16_16_16_16_EXPAND,
			GPUTEXTUREFORMAT_16_FLOAT,
			GPUTEXTUREFORMAT_16_16_FLOAT,
			GPUTEXTUREFORMAT_16_16_16_16_FLOAT,
			GPUTEXTUREFORMAT_32,
			GPUTEXTUREFORMAT_32_32,
			GPUTEXTUREFORMAT_32_32_32_32,
			GPUTEXTUREFORMAT_32_FLOAT,
			GPUTEXTUREFORMAT_32_32_FLOAT,
			GPUTEXTUREFORMAT_32_32_32_32_FLOAT,
			GPUTEXTUREFORMAT_32_AS_8,
			GPUTEXTUREFORMAT_32_AS_8_8,
			GPUTEXTUREFORMAT_16_MPEG,
			GPUTEXTUREFORMAT_16_16_MPEG,
			GPUTEXTUREFORMAT_8_INTERLACED,
			GPUTEXTUREFORMAT_32_AS_8_INTERLACED,
			GPUTEXTUREFORMAT_32_AS_8_8_INTERLACED,
			GPUTEXTUREFORMAT_16_INTERLACED,
			GPUTEXTUREFORMAT_16_MPEG_INTERLACED,
			GPUTEXTUREFORMAT_16_16_MPEG_INTERLACED,
			GPUTEXTUREFORMAT_DXN,
			GPUTEXTUREFORMAT_8_8_8_8_AS_16_16_16_16,
			GPUTEXTUREFORMAT_DXT1_AS_16_16_16_16,
			GPUTEXTUREFORMAT_DXT2_3_AS_16_16_16_16,
			GPUTEXTUREFORMAT_DXT4_5_AS_16_16_16_16,
			GPUTEXTUREFORMAT_2_10_10_10_AS_16_16_16_16,
			GPUTEXTUREFORMAT_10_11_11_AS_16_16_16_16,
			GPUTEXTUREFORMAT_11_11_10_AS_16_16_16_16,
			GPUTEXTUREFORMAT_32_32_32_FLOAT,
			GPUTEXTUREFORMAT_DXT3A,
			GPUTEXTUREFORMAT_DXT5A,
			GPUTEXTUREFORMAT_CTX1,
			GPUTEXTUREFORMAT_DXT3A_AS_1_1_1_1,
			GPUTEXTUREFORMAT_8_8_8_8_GAMMA_EDRAM,
			GPUTEXTUREFORMAT_2_10_10_10_FLOAT_EDRAM,
		};

		DWORD fGetAs16SRGBFormatGPU( D3DFORMAT fmtBase )
		{
			return gMapLinearToSrgbGpuFormat[ (fmtBase & D3DFORMAT_TEXTUREFORMAT_MASK) >> D3DFORMAT_TEXTUREFORMAT_SHIFT ];
		}

		void fConvertTextureToAs16SRGBFormat( IDirect3DTexture9& tex )
		{
			// First thing, mark the texture SignX, SignY and SignZ as sRGB.
			tex.Format.SignX = GPUSIGN_GAMMA;
			tex.Format.SignY = GPUSIGN_GAMMA;
			tex.Format.SignZ = GPUSIGN_GAMMA;

			// Get the texture format...
			XGTEXTURE_DESC desc;
			XGGetTextureDesc( &tex, 0, &desc );

			// ...and convert it to a "good" format (AS_16_16_16_16).
			tex.Format.DataFormat = fGetAs16SRGBFormatGPU( desc.Format );
		}
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, tRenderTarget::tFormat depthFormat, u32 multiSamplePower, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
		, mRealTexture( 0 )
	{
		fCreateTextureAndRenderTarget( device, width, height, format, multiSamplePower );

		if( mRt )
		{
			mDt.fReset( NEW tRenderTarget( device, width, height, depthFormat, multiSamplePower, mRt->fComputeEdramSize( ) ) );
			if( mDt->fFailed( ) )
				fReleaseAll( );
		}
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, const tRenderTargetPtr& depthTarget, u32 multiSamplePower, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
		, mRealTexture( 0 )
	{
		fCreateTextureAndRenderTarget( device, width, height, format, multiSamplePower );

		if( mRt )
		{
			mDt = depthTarget;
			if( mDt->fFailed( ) || mDt->fWidth( ) < mRt->fWidth( ) || mDt->fHeight( ) < mRt->fHeight( ) )
				fReleaseAll( );
		}
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTargetPtr& depthTarget, const tRenderTarget::tFormat* textureFormatOverride, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
		, mRealTexture( 0 )
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

		if( mRt && mRt->fMultiSamplePower( ) > 0 )
		{
			// multi-sampling means we're using a tiled render target, so we must explicitly begin tiling

			IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );
			const tEdramTilingDesc& tilingDesc = tEdramTilingDesc::fGetDefaultDescFromMSAAPower( mRt->fMultiSamplePower( ) );

			d3ddev->BeginTiling(
				tilingDesc.mTilingFlags,
				tilingDesc.mTileCount,
				tilingDesc.mTilingRects,
				0, 1.0f, 0L );
		}
	}

	void tRenderToTexture::fResolve( tScreen& screen, IDirect3DBaseTexture9* targetOverride, u32 slice )
	{
		tRenderToTexturePtr safeStoreRef( this );
		screen.mCurrentTarget.fReset( 0 );

		IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );

		if( mDepthResolveTexture )
		{
			d3ddev->Resolve( D3DRESOLVE_DEPTHSTENCIL | D3DRESOLVE_FRAGMENT0, NULL,
								   mDepthResolveTexture->mRealTexture, NULL, 0, 0, NULL, 0.0f, 0x0, NULL );
		}

		if( mRt && mRt->fMultiSamplePower( ) > 0 )
		{
			sigassert( slice == 0 );

			// multi-sampling means we're using a tiled render target, so we must explicitly end tiling
			d3ddev->EndTiling( D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_ALLFRAGMENTS,
									 NULL,
									 targetOverride ? targetOverride : mRealTexture,
									 NULL,
									 1.0f,
									 0L,
									 NULL );
		}
		else
		{
			// We're not tiling in this scenario.  Just resolve rendertarget 0 to the scene resolve texture.
			d3ddev->Resolve( D3DRESOLVE_RENDERTARGET0,
								   NULL,
								   targetOverride ? targetOverride : mRealTexture,
								   NULL,
								   0,
								   slice,
								   NULL,
								   1.0f,
								   0,
								   NULL );
		}
	}

	void tRenderToTexture::fCreateTextureAndRenderTarget( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, u32 multiSamplePower )
	{
		mRt.fReset( NEW tRenderTarget( device, width, height, format, multiSamplePower ) );
		fCreateTextureFromRenderTarget( device, mRt );
	}

	void tRenderToTexture::fCreateTextureFromRenderTarget( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTarget::tFormat* textureFormatOverride )
	{
		fCreateTexture( device, renderTarget->fWidth( ), renderTarget->fHeight( ), textureFormatOverride ? *textureFormatOverride : renderTarget->fFormat( ), mNumLayers );

		if( !mRealTexture )
			return;

		mRt = renderTarget;

		tTextureReference::fSetRaw( ( tPlatformHandle )mRealTexture );
		tTextureReference::fSetSamplingModes( tTextureFile::cFilterModeNoMip, tTextureFile::cAddressModeClamp ); // default sampling modes, can be overriden later
	}

	void tRenderToTexture::fCreateTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, u32 numLayers )
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		D3DFORMAT d3dFormat = tRenderTarget::fConvertFormatType( format );
		if( format == tRenderTarget::cFormatNull && !device->fSupportsRenderTargetFormat( d3dFormat ) )
			d3dFormat = D3DFMT_X8R8G8B8; // fallback

		HRESULT hr;
		const s32 numMips = 1;

		if( numLayers > 1 )
		{
			IDirect3DArrayTexture9* realTexture = 0;
			hr = d3ddev->CreateArrayTexture( width, height, numLayers, numMips, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &realTexture, 0 );
			mRealTexture = realTexture;
		}
		else
		{
			sigassert( numLayers == 1 );

			IDirect3DTexture9* realTexture = 0;
			hr = d3ddev->CreateTexture( width, height, numMips, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &realTexture, 0 );
			mRealTexture = realTexture;
		}

		if( FAILED( hr ) && mRealTexture )
			fReleaseComPtr( mRealTexture );
	}

	void tRenderToTexture::fReleaseAll( )
	{
		mRt.fRelease( );
		mDt.fRelease( );
		fReleaseComPtr( mRealTexture );
	}


}}
#endif//#if defined( platform_xbox360 )
