#include "BasePch.hpp"
#if defined( platform_pcdx11 ) || defined( platform_metro )
#include "tRenderTarget.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	DXGI_FORMAT tRenderTarget::fConvertFormatType( tFormat format )
	{
		switch( format )
		{
		case cFormatNull:	return DXGI_FORMAT_UNKNOWN;
		case cFormatRGBA8:	return DXGI_FORMAT_R8G8B8A8_UNORM;
		case cFormatR32F:	return DXGI_FORMAT_R32_FLOAT;
		case cFormatD24S8:	return DXGI_FORMAT_D24_UNORM_S8_UINT;
		default:
			sigassert( !"unrecognized format passed to tRenderTarget" );
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	tRenderTarget::tRenderTarget( const tDevicePtr& device, u32 width, u32 height, tFormat format, u32 multiSamplePower )
		: tRenderTargetPlatformBase( width, height, format, multiSamplePower )
		, mTexture( 0 )
	{
		ID3D11Device* d3ddev = device->fGetDevice( );
		DXGI_FORMAT d3dformat = fConvertFormatType( format );
		if( format == cFormatNull && !device->fSupportsRenderTargetFormat( d3dformat ) )
			d3dformat = DXGI_FORMAT_R8G8B8A8_UNORM; // fallback

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width			= width;
		texDesc.Height			= height;
		texDesc.MipLevels		= 1;
		texDesc.ArraySize		= 1;
		texDesc.Format			= d3dformat;
		texDesc.SampleDesc		= tDevice::fConvertMultiSampleType( multiSamplePower );
		texDesc.Usage			= D3D11_USAGE_DEFAULT;
		texDesc.BindFlags		= fIsDepthTarget() ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags	= 0;
		texDesc.MiscFlags		= 0;

		const b32 textureMultisampled = texDesc.SampleDesc.Count>1;

		HRESULT hr = d3ddev->CreateTexture2D( &texDesc, NULL, &mTexture );
		sigassert( !FAILED( hr ) && mTexture );
		fCreateView(device);
	}

	tRenderTarget::tRenderTarget( const tDevicePtr& device, ID3D11Texture2D* rt, u32 width, u32 height, tFormat format, u32 multiSamplePower )
		: tRenderTargetPlatformBase( width, height, format, multiSamplePower )
		, mTexture( rt )
	{
		fCreateView(device);
	}

	tRenderTarget::~tRenderTarget( )
	{
		if ( fIsDepthTarget() )
			fReleaseComPtr( _mDsView );
		else
			fReleaseComPtr( _mRtView );

		fReleaseComPtr( mTexture );
	}

	b32 tRenderTargetPlatformBase::fFailed( ) const
	{
		return ( static_cast< const tRenderTarget* >( this )->mTexture == 0 );
	}

	void tRenderTarget::fApply( const tDevicePtr& device, u32 rtIndex ) const
	{
		const b32 isDepthTarget = fIsDepthTarget();

		ID3D11DeviceContext* context = device->fGetDeviceContext( );
		ID3D11RenderTargetView* rtviews[ D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT ] = {0};
		ID3D11DepthStencilView* dsview = 0;
		context->OMGetRenderTargets( D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtviews, &dsview );

		if ( isDepthTarget )
		{
			rtIndex = (u32)-1;
			if ( dsview ) dsview->Release();
			dsview = fGetDepthStencilView( );
		}
		else
		{
			sigassert( 0<=rtIndex && rtIndex<array_length(rtviews) );
			if ( rtviews[rtIndex] ) rtviews[rtIndex]->Release();
			rtviews[rtIndex] = fGetRenderTargetView();
		}

		context->OMSetRenderTargets( D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtviews, dsview );

		for ( int i=0 ; i<array_length(rtviews) ; ++i )
		{
			if ( rtviews[i] && i != rtIndex )
				rtviews[i]->Release();
		}
	}

	void tRenderTarget::fCreateView( const tDevicePtr& device )
	{
		HRESULT hr;

		ID3D11Device* d3ddev = device->fGetDevice( );

		const b32 textureMultisampled = fMultiSamplePower()>0;

		DXGI_FORMAT d3dformat = tRenderTarget::fConvertFormatType(fFormat());
		sigassert( fFormat() != tRenderTarget::cFormatNull );
		if ( fFormat()==tRenderTarget::cFormatNull && !device->fSupportsRenderTargetFormat( d3dformat ) )
			d3dformat = DXGI_FORMAT_R8G8B8A8_UNORM; // fallback

		if ( fIsDepthTarget() )
		{
			_mDsView = 0;

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = d3dformat;
			dsvDesc.ViewDimension = textureMultisampled ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Flags = 0;
			if ( !textureMultisampled )
			{
				dsvDesc.Texture2D.MipSlice = 0;
			}
			else
			{
				//dsvDesc.Texture2DMS has no fields.
			}
			hr = d3ddev->CreateDepthStencilView( mTexture, &dsvDesc, &_mDsView );
			sigassert(!FAILED(hr) && fGetDepthStencilView());
		}
		else
		{
			_mRtView = 0;

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = d3dformat;
			rtvDesc.ViewDimension = textureMultisampled ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
			if ( !textureMultisampled )
			{
				rtvDesc.Texture2D.MipSlice = 0;
			}
			else
			{
				//dsvDesc.Texture2DMS has no fields.
			}
			hr = d3ddev->CreateRenderTargetView( mTexture, &rtvDesc, &_mRtView );
			sigassert(!FAILED(hr) && fGetRenderTargetView());
		}
	}

}}
#endif//#if defined( platform_pcdx11 ) || defined( platform_metro )
