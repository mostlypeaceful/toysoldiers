#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tMaterialFile.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		void fApplyPlatformVertexShader( const tDevicePtr& device, const tMaterialFile::tShaderPlatformHandle psh )
		{
			IDirect3DDevice9* d3ddev = device->fGetDevice( );

			IDirect3DVertexShader9* dxvs = ( IDirect3DVertexShader9* )psh;

			sigassert( dxvs );
			d3ddev->SetVertexShader( dxvs );
		}

		void fApplyPlatformPixelShader( const tDevicePtr& device, const tMaterialFile::tShaderPlatformHandle psh )
		{
			IDirect3DDevice9* d3ddev = device->fGetDevice( );

			IDirect3DPixelShader9* dxps = ( IDirect3DPixelShader9* )psh;

			sigassert( dxps );
			d3ddev->SetPixelShader( dxps );
		}

		tMaterialFile::tShaderPlatformHandle fCreatePlatformVertexShader( const tDevicePtr& device, const Sig::byte* shaderBuffer, u32 numBytes )
		{
			IDirect3DDevice9* d3ddev = device->fGetDevice( );

			IDirect3DVertexShader9* dxvs = 0;

			d3ddev->CreateVertexShader(
				( DWORD* )shaderBuffer,
				&dxvs );

			sigassert( dxvs );
			return ( tMaterialFile::tShaderPlatformHandle )dxvs;
		}

		tMaterialFile::tShaderPlatformHandle fCreatePlatformPixelShader( const tDevicePtr& device, const Sig::byte* shaderBuffer, u32 numBytes )
		{
			IDirect3DDevice9* d3ddev = device->fGetDevice( );

			IDirect3DPixelShader9* dxps = 0;

			d3ddev->CreatePixelShader(
				( DWORD* )shaderBuffer,
				&dxps );

			sigassert( dxps );
			return ( tMaterialFile::tShaderPlatformHandle )dxps;
		}

		void fDestroyPlatformVertexShader( tMaterialFile::tShaderPlatformHandle& vs )
		{
			IDirect3DVertexShader9* dxvs = ( IDirect3DVertexShader9* )vs;
			fReleaseComPtr( dxvs );
			vs = 0;
		}

		void fDestroyPlatformPixelShader( tMaterialFile::tShaderPlatformHandle& ps )
		{
			IDirect3DPixelShader9* dxps = ( IDirect3DPixelShader9* )ps;
			fReleaseComPtr( dxps );
			ps = 0;
		}
	}

	void tMaterialFile::fApplyShader( const tDevicePtr& device, u32 ithShaderList, u32 ithShader ) const
	{
		const tShaderPointer& shPtr = mShaderLists[ ithShaderList ][ ithShader ];

		if( shPtr.mType == cShaderBufferTypeVertex )
			fApplyPlatformVertexShader( device, shPtr.mPlatformHandle );
		else if( shPtr.mType == cShaderBufferTypePixel )
			fApplyPlatformPixelShader( device, shPtr.mPlatformHandle );
		else
		{
			sigassert( !"Invalid shader type for platform pcdx9!" );
		}
	}

	void tMaterialFile::fCreateShaderPlatformSpecific( const tDevicePtr& device, tShaderPointer& shPtr, const Sig::byte* shaderBuffer )
	{
		if( shPtr.mType == cShaderBufferTypeVertex )
			shPtr.mPlatformHandle = fCreatePlatformVertexShader( device, shaderBuffer, shPtr.mBufferSize );
		else if( shPtr.mType == cShaderBufferTypePixel )
			shPtr.mPlatformHandle = fCreatePlatformPixelShader( device, shaderBuffer, shPtr.mBufferSize );
		else
		{
			sigassert( !"Invalid shader type for platform pcdx9!" );
		}
	}

	void tMaterialFile::fDestroyShaderPlatformSpecific( tShaderPointer& shPtr )
	{
		if( shPtr.mType == cShaderBufferTypeVertex )
			fDestroyPlatformVertexShader( shPtr.mPlatformHandle );
		else if( shPtr.mType == cShaderBufferTypePixel )
			fDestroyPlatformPixelShader( shPtr.mPlatformHandle );
		else
		{
			sigassert( !"Invalid shader type for platform pcdx9!" );
		}
	}

}}
#endif//#if defined( platform_pcdx9 )
