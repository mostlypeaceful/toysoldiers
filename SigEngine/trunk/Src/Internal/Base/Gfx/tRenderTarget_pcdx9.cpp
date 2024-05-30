#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tRenderTarget.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	D3DFORMAT tRenderTarget::fConvertFormatType( tFormat format )
	{
		D3DFORMAT d3dFormat;
		switch( format )
		{
		case cFormatNull: d3dFormat = ( D3DFORMAT )MAKEFOURCC('N','U','L','L'); break;
		case cFormatXRGB8: d3dFormat = D3DFMT_X8R8G8B8; break;
		case cFormatRGBA8: d3dFormat = D3DFMT_A8R8G8B8; break;
		case cFormatR32F: d3dFormat = D3DFMT_R32F; break;
		case cFormatD24S8: d3dFormat = D3DFMT_D24S8; break;
		case cFormatD24FS8: d3dFormat = D3DFMT_D24S8; break;
		default: sigassert( !"unrecognized format passed to tRenderTarget" ); break;
		}

		return d3dFormat;
	}

	tRenderTarget::tRenderTarget( const tDevicePtr& device, u32 width, u32 height, b32 isDepth, tFormat format, u32 multiSamplePower, b32 lockable )
		: tRenderTargetPlatformBase( width, height, isDepth, format, multiSamplePower )
		, mSurface( 0 )
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		D3DFORMAT d3dFormat = fConvertFormatType( format );
		if( format == cFormatNull && !device->fSupportsRenderTargetFormat( d3dFormat ) )
			d3dFormat = D3DFMT_X8R8G8B8; // fallback
		const D3DMULTISAMPLE_TYPE d3dMultiSample = tDevice::fConvertMultiSampleType( multiSamplePower );
		const u32 multiSampleQuality = 0;

		HRESULT hr;		
		if( fIsDepthTarget( ) )
			hr = d3ddev->CreateDepthStencilSurface( width, height, d3dFormat, d3dMultiSample, multiSampleQuality, lockable, &mSurface, 0 );
		else
			hr = d3ddev->CreateRenderTarget( width, height, d3dFormat, d3dMultiSample, multiSampleQuality, lockable, &mSurface, 0 );
		if( FAILED( hr ) && mSurface )
			fReleaseComPtr( mSurface );
	}

	tRenderTarget::tRenderTarget( IDirect3DSurface9* rt, u32 width, u32 height, b32 isDepth, tFormat format, u32 multiSamplePower )
		: tRenderTargetPlatformBase( width, height, isDepth, format, multiSamplePower )
		, mSurface( rt )
	{
	}

	tRenderTarget::~tRenderTarget( )
	{
		fReleaseComPtr( mSurface );
	}

	b32 tRenderTargetPlatformBase::fFailed( ) const
	{
		return ( static_cast< const tRenderTarget* >( this )->mSurface == 0 );
	}

	void tRenderTarget::fApply( const tDevicePtr& device, u32 rtIndex ) const
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );
		if( fIsDepthTarget( ) ) d3ddev->SetDepthStencilSurface( mSurface );
		else					d3ddev->SetRenderTarget( rtIndex, mSurface );

		if( rtIndex == 0 && fMultiSamplePower( ) > 0 )
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
		else
			d3ddev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );
	}

	void tRenderTarget::fSaveToDisk( const tFilePathPtr& absFilePath, D3DXIMAGE_FILEFORMAT format /* = D3DXIFF_PNG */ ) const
	{
		HRESULT hr = D3DXSaveSurfaceToFile( absFilePath.fCStr( ), format, mSurface, NULL, NULL );
		if( !SUCCEEDED(hr) )
			log_warning( "Failed saving " << absFilePath << " to disk. HR: " << (void*)hr );
	}

	void tRenderTarget::fReset( const tDevicePtr& device, u32 rtIndex )
	{
		sigassert( rtIndex != 0 && "Most directX implementations will not let you set the rendertarget 0 to null." );
		IDirect3DDevice9* d3ddev = device->fGetDevice( );
		d3ddev->SetRenderTarget( rtIndex, NULL );
	}

}}
#endif//#if defined( platform_pcdx9 )

