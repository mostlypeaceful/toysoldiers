#include "BasePch.hpp"
#if defined( platform_ios )
#include "tDevice.hpp"
#include "tDeviceResource.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{
	void tDevicePlatformBase::fEnumerateDisplayModes( tDisplayModeList& displayModes )
	{
		// TODOIOS

		displayModes.fPushBack( tDisplayMode( 1280, 720, 0 ) );
	}

	tDevice::tDevice( const tScreenCreationOptions& opts )
	{
		mSingleScreenDevice = true;
		//tD3DObject::fInstance( ).fAddClient( );
		//D3DPRESENT_PARAMETERS d3dpp;
		//fCreatePresentParams( d3dpp, opts, true );
		//fCreateDevice( d3dpp );
		//fParseCaps( );
		fSetDefaultState( );
	}

	tDevice::~tDevice( )
	{
		//fReleaseComPtr( mDevice );
		//if( !mBorrowedDevice )
		//	tD3DObject::fInstance( ).fRemoveClient( );
	}

	void tDevice::fSetDefaultState( )
	{
//		const f32 lineWidth = 2.f;
//		mDevice->SetRenderState( D3DRS_LINEWIDTH, *( DWORD* )&lineWidth );
//
//		// cut out comparison function
//		mDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
//
//		// set texture array state (no filtering between slices, clamp to first and last slice)
//		for( u32 i = 0; i < 16; ++i )
//		{
//			mDevice->SetSamplerState( i, D3DSAMP_SEPARATEZFILTERENABLE, TRUE );
//			mDevice->SetSamplerState( i, D3DSAMP_MINFILTERZ, D3DTEXF_POINT );
//			mDevice->SetSamplerState( i, D3DSAMP_MAGFILTERZ, D3DTEXF_POINT );
//			mDevice->SetSamplerState( i, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP );
//			mDevice->SetSamplerState( i, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_THREEEIGHTHS );
//		}
	}

	void tDevicePlatformBase::fSetShaderGPRAllocation( u32 vsGprs, u32 psGprs )
	{
		// can't control this on ios
	}
	
	void tDevicePlatformBase::fSetScissorRect( const Math::tRect* rect )
	{
	}
	
	void tDevicePlatformBase::fSetGammaRamp( f32 gamma )
	{
	}
}}
#endif//#if defined( platform_ios )

