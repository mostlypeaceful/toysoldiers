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

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, const tFormat& format, tRenderTarget::tFormat depthFormat, u32 multiSamplePower, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
	{
		fCreateTextureAndRenderTarget( device, width, height, format, depthFormat, multiSamplePower );

		if( mRts.fCount( ) )
		{
			u32 depthRamStart = 0;
			for( u32 i = 0; i < mRts.fCount( ); ++i )
				depthRamStart += mRts[ i ]->fComputeEdramSize( );

			mDt.fReset( NEW tRenderTarget( device, width, height, true, depthFormat, mRts[ 0 ]->fTiling( ), multiSamplePower, depthRamStart ) );
			if( mDt->fFailed( ) )
				fReleaseAll( );
		}
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, const tRenderTargetPtr& depthTarget, u32 multiSamplePower, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
	{
		sigassert( depthTarget );
		sigassert( multiSamplePower == depthTarget->fMultiSamplePower( ) && "MSAA has to match!" );
		fCreateTextureAndRenderTarget( device, width, height, tFormat( format ), depthTarget->fFormat( ), multiSamplePower );

		if( mRts.fCount( ) )
		{
			mDt = depthTarget;
			const tRenderTargetPtr& firstRT = fRenderTarget( 0 );
			if( mDt->fFailed( ) || mDt->fWidth( ) < firstRT->fWidth( ) || mDt->fHeight( ) < firstRT->fHeight( ) )
				fReleaseAll( );
		}
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTargetPtr& depthTarget, const tRenderTarget::tFormat* textureFormatOverride, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
	{
		sigassert( renderTarget && !renderTarget->fFailed( ) );
		sigassert( depthTarget && !depthTarget->fFailed( ) );
		sigassert( depthTarget->fWidth( ) >= renderTarget->fWidth( ) && depthTarget->fHeight( ) >= renderTarget->fHeight( ) );

		mRts.fPushBack( renderTarget );
		fCreateTextureFromRenderTarget( device, 0, textureFormatOverride );

		if( mRts.fCount( ) )
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

	void tRenderToTexture::fApply( tScreen& screen )
	{
		sigassert( mDt && !mDt->fFailed( ) );

		// Apply color targets
		for( u32 i = 0; i < mRts.fCount( ); ++i )
		{
			sigassert( mRts[ i ] );
			mRts[ i ]->fApply( screen.fGetDevice( ), i );
		}

		// Apply depth target
		mDt->fApply( screen.fGetDevice( ), 0 );

		screen.mCurrentTarget.fReset( static_cast<tRenderToTexture*>( this ) );

		const tRenderTargetPtr& firstRT = fRenderTarget( 0 );
		if( firstRT && firstRT->fTiling( ).mTileCount > 0 )
		{
			const tEdramTilingDesc& tilingDesc = firstRT->fTiling( );
			IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );
			
			d3ddev->BeginTiling(
				D3DTILING_SKIP_FIRST_TILE_CLEAR,
				tilingDesc.mTileCount,
				tilingDesc.mTilingRects,
				0, 1.0f, 0L );
		}
	}

	void tRenderToTexture::fApplyDepthOnly( tScreen& screen )
	{
		IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );
		screen.mCurrentTarget.fReset( 0 );

		// No color writes
		d3ddev->SetRenderState( D3DRS_COLORWRITEENABLE, 0 );

		for( u32 i = 0; i < cMaxTargets; ++i )
			tRenderTarget::fReset( screen.fGetDevice( ), i );

		mDt->fApply( screen.fGetDevice( ), 0 );
	}

	void tRenderToTexture::fEndDepthOnly( tScreen& screen )
	{
		// Renable color writes
		IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );
		d3ddev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL );
	}

	void tRenderToTexture::fResolveDepth( tScreen& screen, u32 slice )
	{
		sigassert( mDepthResolveTexture );
		IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );
		d3ddev->Resolve( D3DRESOLVE_DEPTHSTENCIL | D3DRESOLVE_FRAGMENT0, NULL,
			mDepthResolveTexture->mRealTexture[ 0 ], NULL, 0, slice, NULL, 0.0f, 0x0, NULL );
	}

	void tRenderToTexture::fResolve( tScreen& screen, IDirect3DBaseTexture9* targetOverride, u32 slice, b32 unApplyExtras )
	{
		profile_pix( "tRenderToTexture::fResolve" );
		tRenderToTexturePtr safeStoreRef( this );
		screen.mCurrentTarget.fReset( 0 );

		if( mDepthResolveTexture )
			fResolveDepth( screen, slice );

		IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );

		const tRenderTargetPtr& firstRT = fRenderTarget( 0 );
		if( !firstRT )
			return;

		// Only color resolving after here.
		if( firstRT->fTiling( ).mTileCount > 0 )
		{
			const tEdramTilingDesc& tiling = firstRT->fTiling( );

			//b32 simpleResolve = mRts.fCount( ) == 1;
			//if( simpleResolve )
			//{
			//	sigassert( slice == 0 );
			//	// multi-sampling means we're using a tiled render target, so we must explicitly end tiling
			//	d3ddev->EndTiling( D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_ALLFRAGMENTS,
			//							 NULL,
			//							 targetOverride ? targetOverride : mRealTexture[ 0 ],
			//							 NULL,
			//							 1.0f,
			//							 0L,
			//							 NULL );
			//}
			//else
			{
				// we have more than one target, so we have to do complicated resolve.

				f32 minZ, maxZ;
				screen.fGetWorldDepthSetup( minZ, maxZ );

				const Math::tVec4f& lastClearColor = screen.fLastRgbaClearColor( );
				D3DVECTOR4 clearColor = { lastClearColor.x, lastClearColor.y, lastClearColor.z, lastClearColor.w };

				for( u32 i = 0; i < tiling.mTileCount; ++i )
				{
					// Set predication to tile i.
					d3ddev->SetPredication( D3DPRED_TILE( i ) );

					// Destination point is the upper left corner of the tiling rect.
					D3DPOINT* pDestPoint = ( D3DPOINT* )&tiling.mTilingRects[ i ];

					//// Resolve fragment 0 of every pixel in the depth/stencil buffer.
					//d3ddev->Resolve( D3DRESOLVE_DEPTHSTENCIL | D3DRESOLVE_FRAGMENT0,
					//	&m_pTilingRects[i],
					//	m_pResolveTexture[3],
					//	pDestPoint,
					//	0, 0,
					//	&ClearColorBlack,
					//	1.0f, 0L, NULL );

					// Resolve render targets
					for( u32 r = 0; r < mRts.fCount( ); ++r )
					{
						u32 operation = (D3DRESOLVE_RENDERTARGET0 + r) | D3DRESOLVE_CLEARRENDERTARGET;
						
						if( r == 0 )
							operation |= D3DRESOLVE_CLEARDEPTHSTENCIL;

						d3ddev->Resolve( operation,
							&tiling.mTilingRects[ i ],
							targetOverride ? targetOverride : mRealTexture[ r ],
							pDestPoint,
							0, 0,
							&clearColor,
							maxZ, 0L, NULL );
					}
				}

				// Restore predication to default.
				d3ddev->SetPredication( 0 );

				// End tiling.
				// When we pass NULL as the pDestTexture parameter to EndTiling(), it disables
				// the automatic resolve that EndTiling() normally performs.
				d3ddev->EndTiling( 0, NULL, NULL, NULL, maxZ, 0L, NULL );

				if( unApplyExtras )
				{
					// we're done with the extra targets
					for( u32 i = 1; i < mRts.fCount( ); ++i )
						tRenderTarget::fReset( screen.fGetDevice( ), i );
				}
			}
		}
		else
		{
			// We're not tiling in this scenario.  Just resolve rendertarget 0 to the scene resolve texture.
			d3ddev->Resolve( D3DRESOLVE_RENDERTARGET0,
								   NULL,
								   targetOverride ? targetOverride : mRealTexture[ 0 ],
								   NULL,
								   0,
								   slice,
								   NULL,
								   1.0f,
								   0,
								   NULL );
		}
	}

	void tRenderToTexture::fCreateTextureAndRenderTarget( const tDevicePtr& device, u32 width, u32 height, const tFormat& format, tRenderTarget::tFormat depthFormat, u32 multiSamplePower )
	{
		const u32 numTargets = format.fCount( );
		tEdramTilingDesc tiling = tEdramTilingDesc::fChooseTiling( width, height, numTargets, multiSamplePower, tRenderTarget::fFormatBitsPerPixel( format.mFormats[ 0 ] ), tRenderTarget::fFormatBitsPerPixel( depthFormat ) );

		sigassert( mRts.fCount( ) == 0 && "You'll need to clean this up first!" );

		u32 ramStart = 0;
		for( u32 i = 0; i < numTargets; ++i )
		{
			mRts.fPushBack( tRenderTargetPtr( NEW tRenderTarget( device, width, height, false, format.mFormats[ i ], tiling, multiSamplePower, ramStart ) ) );
			fCreateTextureFromRenderTarget( device, i );
			ramStart += mRts.fBack( )->fComputeEdramSize( );
		}
	}

	void tRenderToTexture::fCreateTextureFromRenderTarget( const tDevicePtr& device, u32 index, const tRenderTarget::tFormat* textureFormatOverride )
	{
		tRenderTargetPtr& renderTarget = mRts[ index ];
		fCreateTexture( device, index, renderTarget->fWidth( ), renderTarget->fHeight( ), textureFormatOverride ? *textureFormatOverride : renderTarget->fFormat( ), mNumLayers );

		if( !mRealTexture[ index ] )
			return;

		mTextures[ index ].fSetRaw( ( tTextureReference::tPlatformHandle )mRealTexture[ index ] );
		mTextures[ index ].fSetSamplingModes( tTextureFile::cFilterModeNoMip, tTextureFile::cAddressModeClamp ); // default sampling modes, can be overriden later
	}

	void tRenderToTexture::fCreateTexture( const tDevicePtr& device, u32 index, u32 width, u32 height, tRenderTarget::tFormat format, u32 numLayers )
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		D3DFORMAT d3dFormat = tRenderTarget::fConvertFormatType( format );
		if( format == tRenderTarget::cFormatNull && !device->fSupportsRenderTargetFormat( d3dFormat ) )
			d3dFormat = D3DFMT_X8R8G8B8; // fallback

		HRESULT hr;
		const s32 numMips = 1;

		Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysTextures ) );

		if( numLayers > 1 )
		{
			IDirect3DArrayTexture9* realTexture = 0;
			hr = d3ddev->CreateArrayTexture( width, height, numLayers, numMips, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &realTexture, 0 );
			mRealTexture[ index ] = realTexture;
		}
		else
		{
			sigassert( numLayers == 1 );

			IDirect3DTexture9* realTexture = 0;
			hr = d3ddev->CreateTexture( width, height, numMips, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &realTexture, 0 );
			mRealTexture[ index ] = realTexture;
		}

		Memory::tHeap::fResetVramContext( );

		if( FAILED( hr ) && mRealTexture[ index ] )
			fReleaseComPtr( mRealTexture[ index ] );
	}

	void tRenderToTexture::fReleaseAll( )
	{
		for( u32 i = 0; i < mRts.fCount( ); ++i )
			mRts[ i ].fRelease( );

		mRts.fSetCount( 0 );
		mDt.fRelease( );
		for( u32 i = 0; i < mRealTexture.fCount( ); ++i )
			fReleaseComPtr( mRealTexture[ i ] );
	}


}}
#endif//#if defined( platform_xbox360 )
