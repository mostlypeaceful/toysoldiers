#if defined( platform_pcdx9 )
#ifndef __tDevice_pc__
#define __tDevice_pc__
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

	public:

		static D3DMULTISAMPLE_TYPE fConvertMultiSampleType( u32 multiSamplePower );
		static void fCreatePresentParams( D3DPRESENT_PARAMETERS& d3dpp, const tScreenCreationOptions& opts );
		static void fCreatePresentParamsForMultiSwapChainDevice( D3DPRESENT_PARAMETERS& d3dpp, HWND hwnd );

	public:
		explicit tDevice( u64 windowHandleGeneric, b32 reference = false );
		explicit tDevice( const tScreenCreationOptions& opts );
		explicit tDevice( IDirect3DDevice9* unOwnedDevice );
		~tDevice( );
		void fReset( );
		IDirect3D9* fGetObject( ) const;
		inline IDirect3DDevice9* fGetDevice( ) const { return mDevice; }
		inline const D3DPRESENT_PARAMETERS& fCreationPresentParams( ) const { return mCreationPresentParams; }
		inline b32 fRequiresDeviceReset( ) const { return true; }
		b32 fSupportsRenderTargetFormat( D3DFORMAT d3dFormat ) const;
		IDirect3DTexture9* fCreateSolidColorTexture( u32 width, u32 height, const Math::tVec4f& rgba ) const;

	private:
		void fCreateDevice( D3DPRESENT_PARAMETERS& d3dpp, b32 reference = false );
		void fParseCaps( );
		b32 fMeetsMinimumSpecs( const D3DCAPS9& dxcaps, std::stringstream& errorText ) const;
		void fSetDefaultState( );
	};

}}


#endif//__tDevice_pc__
#endif//#if defined( platform_pcdx9 )
