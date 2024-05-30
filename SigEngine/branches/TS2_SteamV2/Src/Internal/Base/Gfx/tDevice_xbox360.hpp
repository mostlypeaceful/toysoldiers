#if defined( platform_xbox360 )
#ifndef __tDevice_xbox360__
#define __tDevice_xbox360__
#ifndef __tDevice__
#error This file must be included from tDevice.hpp!
#endif//__tDevice__

namespace Sig { namespace Gfx
{
	template<class tComIface>
	void fReleaseComPtr( tComIface& comIface )
	{
		if( comIface )
		{
			comIface->Release( );
			comIface = 0;
		}
	}

	template<class tComIface>
	u32 fComPtrRefCount( tComIface comIface )
	{
		if( !comIface )
			return 0;
		comIface->AddRef( );
		return ( u32 )comIface->Release( );
	}

	struct tDeviceCaps;
	struct tScreenCreationOptions;

	class base_export tDevice : public tDevicePlatformBase
	{
	private:
		IDirect3DDevice9*		mDevice;
		D3DPRESENT_PARAMETERS	mCreationPresentParams;
		D3DGAMMARAMP			mGammaRamp;

	public:

		static D3DMULTISAMPLE_TYPE fConvertMultiSampleType( u32 multiSamplePower );
		static void fCreatePresentParams( 
			D3DPRESENT_PARAMETERS& d3dpp, 
			u32 bbWidth,
			u32 bbHeight,
			u32 multiSamplePower,
			u32 vsync,
			b32 autoDepthStencil );
		static void fCreatePresentParams(
			D3DPRESENT_PARAMETERS& d3dpp, 
			const tScreenCreationOptions& opts,
			b32 autoDepthStencil );

	public:
		explicit tDevice( const tScreenCreationOptions& opts );
		explicit tDevice( IDirect3DDevice9* unOwnedDevice );
		~tDevice( );
		void fReset( );
		IDirect3D9* fGetObject( ) const;
		inline IDirect3DDevice9* fGetDevice( ) const { return mDevice; }
		inline const D3DPRESENT_PARAMETERS& fCreationPresentParams( ) const { return mCreationPresentParams; }
		inline const D3DGAMMARAMP& fDefaultGammaRamp( ) const { return mGammaRamp; }
		inline b32 fRequiresDeviceReset( ) const { return true; }
		b32 fSupportsRenderTargetFormat( D3DFORMAT d3dFormat ) const;
		IDirect3DTexture9* fCreateSolidColorTexture( u32 width, u32 height, const Math::tVec4f& rgba ) const;

	private:
		void fCreateDevice( D3DPRESENT_PARAMETERS& d3dpp );
		void fParseCaps( );
		void fSetDefaultState( );
	};

}}


#endif//__tDevice_xbox360__
#endif//#if defined( platform_xbox360 )
