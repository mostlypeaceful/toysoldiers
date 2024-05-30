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
		static void fCreatePresentParams( 
			D3DPRESENT_PARAMETERS& d3dpp, 
			HWND hwnd,
			u32 bbWidth,
			u32 bbHeight,
			u32 multiSamplePower,
			u32 vsync,
			b32 windowed,
			b32 autoDepthStencil );
		static void fCreatePresentParams(
			D3DPRESENT_PARAMETERS& d3dpp, 
			const tScreenCreationOptions& opts,
			b32 autoDepthStencil );
		static void fCreatePresentParamsForMultiSwapChainDevice( D3DPRESENT_PARAMETERS& d3dpp, HWND hwnd );

		const D3DPRESENT_PARAMETERS &fGetCreationPresentParams( ) { return mCreationPresentParams; }


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
		b32 fOwns( IDirect3DResource9* resource ) const;
		b32 fOwns( IDirect3DVertexDeclaration9* resource ) const;
		IDirect3DTexture9* fCreateSolidColorTexture( u32 width, u32 height, const Math::tVec4f& rgba ) const;

		typedef tDelegate< void ( const tDevicePtr & ) > tCallback;
		void fAddParamsChangedCallback( const tCallback &callback ) { mOnParamsChanged.fPushBack(callback); }
		void fRemoveParamsChangedCallback( const tCallback &callback ) { mOnParamsChanged.fFindAndErase(callback); }

		// set the d3d multisample levels
		void fSetMultisamplePower( int multisamplePower );

		// does this device have render focus..
		// if we have focus but have no device, we will need to reset
		b32 fHasFocus() { return mHasFocus; }

		// tell the device if it has focus or not.
		void fSetHasFocus( b32 focus, b32 resetDevice ) { mHasFocus = focus; }

		// toggle between windowed and full screen mode
		void fToggleFullscreen();

		// check if we have lost the device and need resetings
		b32 fNeedsReset() { return mNeedsReset; }

		// reset if necessary
		// if this returns FALSE, the device is not ready for further processing, skip the game update and try again next frame
		b32 fPrepareForGameTick();
		void fResetDisplayMode( u32 width, u32 height, b32 fullscreen, u32 vsync );
		void fGetDisplayMode( u32 &width, u32 &height, b32 &fullscreen, u32 &vsync );

	private:
		void fCreateDevice( D3DPRESENT_PARAMETERS& d3dpp, b32 reference = false );
		void fParseCaps( );
		b32 fMeetsMinimumSpecs( const D3DCAPS9& dxcaps, std::stringstream& errorText ) const;
		void fSetDefaultState( );
		b32 mHasFocus;

		b32 mNeedsReset;
		b32 mHasBufferedWindowDimensions;
		b32 mHasBufferedFullscreenDimensions;
		int mBufferedWindowX;
		int mBufferedWindowY;
		int mBufferedWindowWidth;
		int mBufferedWindowHeight;
		int mBufferedFullscreenWidth;
		int mBufferedFullscreenHeight;

		tGrowableArray<tCallback> mOnParamsChanged;
	};

}}


#endif//__tDevice_pc__
#endif//#if defined( platform_pcdx9 )
