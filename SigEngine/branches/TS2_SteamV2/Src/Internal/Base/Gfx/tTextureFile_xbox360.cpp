#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tTextureFile.hpp"
#include "tDevice.hpp"
#include "xgraphics.h"

namespace Sig { namespace Gfx
{
	devvar_clamp( u32, Renderer_Settings_MaxAnisotropy, 4, 1, 8, 0 );

	namespace
	{
		D3DFORMAT fConvertFormat( tTextureFile::tFormat format, b32& isDxtFormat )
		{
			D3DFORMAT o = ( D3DFORMAT )0;
			isDxtFormat = false;

			switch( format )
			{
			case tTextureFile::cFormatA8R8G8B8:		o = D3DFMT_A8R8G8B8; break;
			case tTextureFile::cFormatDXT1:			o = D3DFMT_DXT1; isDxtFormat = true; break;
			case tTextureFile::cFormatDXT3:			o = D3DFMT_DXT3; isDxtFormat = true; break;
			case tTextureFile::cFormatDXT5:			o = D3DFMT_DXT5; isDxtFormat = true; break;
			case tTextureFile::cFormatR5G6B5:		o = D3DFMT_R5G6B5; break;
			case tTextureFile::cFormatA8:			o = D3DFMT_A8; break;
			default:								sigassert( !"un-recognized texture format!" ); break;
			}

			return o;
		}

		void fApplyFilterModeInternal(
			const tDevicePtr& device, 
			u32 slot,
			IDirect3DBaseTexture9* d3dtexture,
			tTextureFile::tFilterMode filter )
		{
			IDirect3DDevice9* d3ddev = device->fGetDevice( );

			if( filter == tTextureFile::cFilterModeNone )
			{
				d3ddev->SetSamplerState( slot, D3DSAMP_MINFILTER, D3DTEXF_POINT );
				d3ddev->SetSamplerState( slot, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
				d3ddev->SetSamplerState( slot, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
			}
			else
			{
				const tDeviceCaps& caps = device->fCaps( );

				const u32 mipCount = d3dtexture->GetLevelCount( );
				const u32 maxAniso = fMin<u32>( Renderer_Settings_MaxAnisotropy, caps.mMaxAnisotropy );
				if( maxAniso > 1 && mipCount > 1 )
				{
					// aniso supported, use it
					d3ddev->SetSamplerState( slot, D3DSAMP_MAXANISOTROPY, maxAniso );
					d3ddev->SetSamplerState( slot, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
				}
				else
				{
					// aniso not supported, use linear
					d3ddev->SetSamplerState( slot, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
				}

				d3ddev->SetSamplerState( slot, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

				if( filter == tTextureFile::cFilterModeNoMip || mipCount == 1 )
					d3ddev->SetSamplerState( slot, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
				else
					d3ddev->SetSamplerState( slot, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
			}
		}
		void fApplyFilterModeInternal(
			const tDevicePtr& device, 
			u32 slot,
			tTextureFile::tFilterMode filter )
		{
			IDirect3DDevice9* d3ddev = device->fGetDevice( );

			if( filter == tTextureFile::cFilterModeNone )
			{
				d3ddev->SetSamplerState( slot, D3DSAMP_MINFILTER, D3DTEXF_POINT );
				d3ddev->SetSamplerState( slot, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
				d3ddev->SetSamplerState( slot, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
			}
			else
			{
				d3ddev->SetSamplerState( slot, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
				d3ddev->SetSamplerState( slot, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

				if( filter == tTextureFile::cFilterModeNoMip )
					d3ddev->SetSamplerState( slot, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
				else
					d3ddev->SetSamplerState( slot, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
			}
		}
		inline void fApplyAddressMode( 
			IDirect3DDevice9* d3ddev,
			u32 slot,
			tTextureFile::tAddressMode sigAddressMode,
			D3DSAMPLERSTATETYPE d3dAddressAxis )
		{
			switch( sigAddressMode )
			{
			case tTextureFile::cAddressModeWrap:		
				d3ddev->SetSamplerState( slot, d3dAddressAxis, D3DTADDRESS_WRAP ); break;
			case tTextureFile::cAddressModeClamp:		
				d3ddev->SetSamplerState( slot, d3dAddressAxis, D3DTADDRESS_CLAMP ); break;
			case tTextureFile::cAddressModeMirror:		
				d3ddev->SetSamplerState( slot, d3dAddressAxis, D3DTADDRESS_MIRROR ); break;
			case tTextureFile::cAddressModeBorderWhite:	
				d3ddev->SetSamplerState( slot, d3dAddressAxis, D3DTADDRESS_BORDER ); 
				d3ddev->SetSamplerState( slot, D3DSAMP_BORDERCOLOR, 0xffffffff ); break;
			case tTextureFile::cAddressModeBorderBlack:	
				d3ddev->SetSamplerState( slot, d3dAddressAxis, D3DTADDRESS_BORDER ); 
				d3ddev->SetSamplerState( slot, D3DSAMP_BORDERCOLOR, 0x00000000 ); break;
			}
		}



		void fApplyAddressMode(
			const tDevicePtr& device, 
			u32 slot,
			tTextureFile::tAddressMode u,
			tTextureFile::tAddressMode v,
			tTextureFile::tAddressMode w )
		{
			IDirect3DDevice9* d3ddev = device->fGetDevice( );
			fApplyAddressMode( d3ddev, slot, u, D3DSAMP_ADDRESSU );
			fApplyAddressMode( d3ddev, slot, v, D3DSAMP_ADDRESSV );
			fApplyAddressMode( d3ddev, slot, w, D3DSAMP_ADDRESSW );
		}
	}

	tTextureFile::tPlatformHandle tTextureFile::fCreateTexture( 
		const tDevicePtr& device,
		u32 width,
		u32 height,
		u32 mipCount,
		tFormat format,
		tType type,
		u32 arrayCount )
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		IDirect3DBaseTexture9* dxtex = 0;

		b32 isDxtFormat = false;
		const D3DFORMAT dxformat = fConvertFormat( format, isDxtFormat );

		if( isDxtFormat )
		{
			// clamp dxt formats to be no smaller than a 4x4 block
			width = fMax( 4u, width );
			height = fMax( 4u, height );
		}

		switch( type )
		{
		case cType2d:
			{
				if( arrayCount > 0 )
				{
					IDirect3DArrayTexture9* dxtex2dArray = 0;
					d3ddev->CreateArrayTexture( 
						width,
						height,
						arrayCount,
						mipCount,
						0, // no usage
						dxformat,
						D3DPOOL_MANAGED,
						&dxtex2dArray,
						0 );
					dxtex = dxtex2dArray;
				}
				else
				{
					IDirect3DTexture9* dxtex2d = 0;
					d3ddev->CreateTexture( 
						width,
						height,
						mipCount,
						0, // no usage
						dxformat,
						D3DPOOL_MANAGED,
						&dxtex2d,
						0 );
					dxtex = dxtex2d;
				}
			}
			break;

		case cTypeCube:
			{
				IDirect3DCubeTexture9* dxtexCube = 0;
				sigassert( width == height );
				d3ddev->CreateCubeTexture(
					width,
					mipCount,
					0,
					dxformat,
					D3DPOOL_MANAGED,
					&dxtexCube,
					0 );
				dxtex = dxtexCube;
			}
			break;

		default:
			sigassert( !"unrecognized texture type" );
			break;
		}

		sigassert( dxtex );
		return ( tPlatformHandle )dxtex;
	}

	void tTextureFile::fDestroyTexture( 
		tPlatformHandle rawHandle )
	{
		if( rawHandle )
			( ( IDirect3DBaseTexture9* )rawHandle )->Release( );
	}

	tTextureFile::tLockedMip tTextureFile::fLockMip(
		tPlatformHandle rawHandle,
		u32 iface, 
		u32 imip,
		tType type,
		b32 isArray )
	{
		switch( type )
		{
		case cType2d:
			{
				if( isArray )
				{
					IDirect3DArrayTexture9* dxtex2darray = ( IDirect3DArrayTexture9* )rawHandle;
					sigassert( dxtex2darray );

					D3DLOCKED_RECT rect;
					dxtex2darray->LockRect( iface, imip, &rect, 0, 0 );

					return tLockedMip( ( Sig::byte* )rect.pBits, rect.Pitch );
				}
				else
				{
					IDirect3DTexture9* dxtex2d = ( IDirect3DTexture9* )rawHandle;
					sigassert( dxtex2d );

					D3DLOCKED_RECT rect;
					dxtex2d->LockRect( imip, &rect, 0, 0 );

					return tLockedMip( ( Sig::byte* )rect.pBits, rect.Pitch );
				}
			}
			break;

		case cTypeCube:
			{
				IDirect3DCubeTexture9* dxtexCube = ( IDirect3DCubeTexture9* )rawHandle;
				sigassert( dxtexCube );

				D3DLOCKED_RECT rect;
				dxtexCube->LockRect( ( D3DCUBEMAP_FACES )iface, imip, &rect, 0, 0 );

				return tLockedMip( ( Sig::byte* )rect.pBits, rect.Pitch );
			}
			break;

		default:
			sigassert( !"unrecognized texture type" );
			break;
		}

		return tLockedMip( );
	}

	void tTextureFile::fUnlockMip(
		tPlatformHandle rawHandle,
		u32 iface, 
		u32 imip,
		tType type,
		b32 isArray )
	{
		switch( type )
		{
		case cType2d:
			{
				if( isArray )
				{
					IDirect3DArrayTexture9* dxtex2darray = ( IDirect3DArrayTexture9* )rawHandle;
					sigassert( dxtex2darray );

					dxtex2darray->UnlockRect( iface, imip );
				}
				else
				{
					IDirect3DTexture9* dxtex2d = ( IDirect3DTexture9* )rawHandle;
					sigassert( dxtex2d );

					dxtex2d->UnlockRect( imip );
				}
			}
			break;

		case cTypeCube:
			{
				IDirect3DCubeTexture9* dxtexCube = ( IDirect3DCubeTexture9* )rawHandle;
				sigassert( dxtexCube );

				dxtexCube->UnlockRect( ( D3DCUBEMAP_FACES )iface, imip );
			}
			break;

		default:
			sigassert( !"unrecognized texture type" );
			break;
		}
	}

	void tTextureFile::fApply( 
		tPlatformHandle rawTexHandle, 
		const tDevicePtr& device, 
		u32 slot, 
		tFilterMode filter, 
		tAddressMode u, 
		tAddressMode v, 
		tAddressMode w )
	{
		IDirect3DBaseTexture9* d3dtexture = ( IDirect3DBaseTexture9* )rawTexHandle;
		sigassert( d3dtexture );

		fApplyAddressMode( device, slot, u, v, w );
		fApplyFilterModeInternal( device, slot, d3dtexture, filter );
		device->fGetDevice( )->SetTexture( slot, d3dtexture );
	}
	void tTextureFile::fApplyFilterMode(
		const tDevicePtr& device, 
		u32 slot, 
		tFilterMode filter )
	{
		fApplyFilterModeInternal( device, slot, filter );
	}

	namespace
	{
		inline u32 fMipHeight( u32 baseHeight, u32 imip )
		{
			for( u32 i = 0; i < imip; ++i )
				baseHeight /= 2;
			return fMax( 1u, baseHeight );
		}
		inline u32 fSourcePitch( u32 bufferSize, u32 baseHeight, u32 imip, D3DFORMAT d3dformat )
		{
			const u32 mipHeight = fMipHeight( baseHeight, imip );
			const u32 srcPitch = bufferSize / mipHeight;
			switch( d3dformat ) {
				case D3DFMT_DXT1:
				case D3DFMT_DXT3:
				case D3DFMT_DXT5: return srcPitch * 4;
			}
			return srcPitch;
		}
	}

	void tTextureFile::fCopyMipInternal( const tDevicePtr& device, u32 iface, u32 imip, const Sig::byte* mipBuffer, u32 bufferSize )
	{
		IDirect3DBaseTexture9* d3dtexture = ( IDirect3DBaseTexture9* )mPlatformHandle;

		b32 isDxt=false;
		const D3DFORMAT d3dformat = fConvertFormat( mFormat, isDxt );

		tLockedMip mip = fLockMip( mPlatformHandle, iface, imip, mType, mIsAtlas );

		const u32 srcPitch = fSourcePitch( bufferSize, mHeight, imip, d3dformat );

#if sig_resource_logging
		//Time::tStopWatch timer;
#endif//sig_resource_logging

		XGTileTextureLevel( 
			mWidth, mHeight, imip, 
			XGGetGpuFormat( d3dformat ), 
			XGIsPackedTexture( d3dtexture ) ? 0 : XGTILE_NONPACKED, 
			mip.mBits, NULL, mipBuffer, srcPitch, 0 );

#if sig_resource_logging
		//if( mWidth >= 512 && mHeight >= 512 )
		//{
		//	if( tResource::fLoggingEnabled( ) )
		//		log_line( Log::cFlagResource, "\t[XGTileTextureLevel took " << timer.fGetElapsedMs( ) << " ms]" );
		//}
#endif//sig_resource_logging

		fUnlockMip( mPlatformHandle, iface, imip, mType, mIsAtlas );
	}
}}

#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{
	void tTextureReference::fClearBoundTextures( const tDevicePtr& device, u32 begin, u32 end )
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );
		for( u32 i = begin; i < end; ++i )
			d3ddev->SetTexture( i, 0 );
	}
}}
#endif//#if defined( platform_xbox360 )

