#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tRenderTarget.hpp"
#include "tDevice.hpp"
#include "tEdramTilingDesc.hpp"

namespace Sig { namespace Gfx
{
	D3DFORMAT tRenderTarget::fConvertFormatType( tFormat format )
	{
		D3DFORMAT d3dFormat;
		switch( format )
		{
		case cFormatNull:
		case cFormatRGBA8: d3dFormat = ( D3DFORMAT )/*MAKESRGBFMT*/( D3DFMT_A8R8G8B8 ); break;
		case cFormatR32F: d3dFormat = D3DFMT_R32F; break;
		case cFormatD24S8: d3dFormat = D3DFMT_D24S8; break;
		default: sigassert( !"unrecognized format passed to tRenderTarget" ); break;
		}

		return d3dFormat;
	}

	tRenderTarget::tRenderTarget( const tDevicePtr& device, u32 width, u32 height, tFormat format, u32 multiSamplePower, u32 edramSize )
		: tRenderTargetPlatformBase( width, height, format, multiSamplePower )
		, mSurface( 0 )
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		D3DFORMAT d3dFormat = fConvertFormatType( format );
		if( format != cFormatNull )
		{
			const tEdramTilingDesc& tilingDesc = tEdramTilingDesc::fGetDefaultDescFromMSAAPower( multiSamplePower );

			const Math::tVec2u realDims = ( multiSamplePower > 0 ) ? tilingDesc.fComputeTileDims( ) : Math::tVec2u( width, height );
			const u32 multiSampleQuality = 0;

			D3DSURFACE_PARAMETERS surfParams = { 0 };
			surfParams.Base = edramSize / GPU_EDRAM_TILE_SIZE;

			HRESULT hr;
			if( fIsDepthTarget( ) )
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
		}
	}

	tRenderTarget::tRenderTarget( IDirect3DSurface9* rt, u32 width, u32 height, tFormat format, u32 multiSamplePower )
		: tRenderTargetPlatformBase( width, height, format, multiSamplePower )
		, mSurface( rt )
	{
		sigassert( !"not allowed" );
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
		if( fIsDepthTarget( ) ) d3ddev->SetDepthStencilSurface( mSurface );
		else					d3ddev->SetRenderTarget( rtIndex, mSurface );
	}

}}
#endif//#if defined( platform_xbox360 )

