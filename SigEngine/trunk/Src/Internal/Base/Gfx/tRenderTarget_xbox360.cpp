#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tRenderTarget.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	D3DFORMAT tRenderTarget::fConvertFormatType( tFormat format )
	{
		D3DFORMAT d3dFormat;
		switch( format )
		{
		case cFormatNull:
		case cFormatXRGB8: d3dFormat = D3DFMT_X8R8G8B8; break;
		case cFormatRGBA8: d3dFormat = D3DFMT_A8R8G8B8; break;
		case cFormatRGBA16F: d3dFormat = D3DFMT_A16B16G16R16F; break;
		case cFormatR32F:	d3dFormat = D3DFMT_R32F; break;
		case cFormatRG16F:	d3dFormat = D3DFMT_G16R16F; break;
		case cFormatD24S8: 
			sigassert( !"cFormatD24FS8 should be used on xbox instead of cFormatD24S8 for performance reasons (specifically for Hierarchical-Z culling to work. Please switch to that format for your depth buffer. This optimization is xbox-only." );
			d3dFormat = D3DFMT_D24S8;
			break;
		case cFormatD24FS8: d3dFormat = D3DFMT_D24FS8; break;
		default: sigassert( !"unrecognized format passed to tRenderTarget" ); break;
		}

		return d3dFormat;
	}

	u32 tRenderTarget::fFormatBitsPerPixel( tFormat format )
	{
		u32 bits = 0;

		switch( format )
		{
		case cFormatNull:
		case cFormatRGBA8: 
		case cFormatRG16F:
		case cFormatR32F:
		case cFormatD24S8:
		case cFormatD24FS8:
			bits = 32; 
			break;
		case cFormatRGBA16F:
			bits = 64; 
			break;
		default: sigassert( !"unrecognized format passed to fFormatBitsPerPixel" ); break;
		}

		return bits;
	}

	tRenderTarget::tRenderTarget( const tDevicePtr& device, u32 width, u32 height, b32 isDepth, tFormat format, const tEdramTilingDesc& tiling, u32 multiSamplePower, u32 edramStartPos )
		: tRenderTargetPlatformBase( width, height, isDepth, format, multiSamplePower )
		, mSurface( 0 )
		, mTiling( tiling )
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		D3DFORMAT d3dFormat = fConvertFormatType( format );
		if( format != cFormatNull )
		{
			const tEdramTilingDesc& tilingDesc = fTiling( );
			const Math::tVec2u realDims = tilingDesc.mTileCount ? tilingDesc.fComputeTileDims( ) : Math::tVec2u( width, height );
			const u32 multiSampleQuality = 0;

			D3DSURFACE_PARAMETERS surfParams = { 0 };
			surfParams.Base = edramStartPos / GPU_EDRAM_TILE_SIZE;

			Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysTextures ) );

			HRESULT hr;
			if( isDepth )
			{
				surfParams.HiZFunc = D3DHIZFUNC_DEFAULT;
				hr = d3ddev->CreateDepthStencilSurface( realDims.x, realDims.y, d3dFormat, tilingDesc.mMsaaType, multiSampleQuality, FALSE, &mSurface, &surfParams );
			}
			else
			{
				hr = d3ddev->CreateRenderTarget( realDims.x, realDims.y, d3dFormat, tilingDesc.mMsaaType, multiSampleQuality, FALSE, &mSurface, &surfParams );
			}
			if( FAILED( hr ) && mSurface )
				fReleaseComPtr( mSurface );

			Memory::tHeap::fResetVramContext( );
		}
	}

	tRenderTarget::~tRenderTarget( )
	{
		fReleaseComPtr( mSurface );
	}

	b32 tRenderTargetPlatformBase::fFailed( ) const
	{
		return ( static_cast< const tRenderTarget* >( this )->mSurface == 0 );
	}

	u32 tRenderTarget::fComputeEdramSize( ) const
	{
		if( mSurface )
			return mSurface->Size;
		return 0;
	}

	void tRenderTarget::fApply( const tDevicePtr& device, u32 rtIndex ) const
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );
		if( fIsDepthTarget( ) )	d3ddev->SetDepthStencilSurface( mSurface );
		else					d3ddev->SetRenderTarget( rtIndex, mSurface );

		if( rtIndex == 0 && fMultiSamplePower( ) > 0 )
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
		else
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );
	}

	void tRenderTarget::fReset( const tDevicePtr& device, u32 rtIndex )
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );
		d3ddev->SetRenderTarget( rtIndex, NULL );
	}

}}
#endif//#if defined( platform_xbox360 )

