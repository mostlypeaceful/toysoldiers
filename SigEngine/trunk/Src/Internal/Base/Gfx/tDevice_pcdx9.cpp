#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tDevice.hpp"
#include "tDeviceResource.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{

	///
	/// \brief Global d3d object manager, allows for multiple devices to automatically
	/// share the same d3d object.
	class base_export tD3DObject
	{
		declare_singleton_define_own_ctor_dtor( tD3DObject );
	private:
		s32 mRefCount;
		IDirect3D9*	mObject;
	public:
		void fAddClient( )
		{
			if( !mObject )
			{
				mObject = Direct3DCreate9( D3D_SDK_VERSION );
				sigassert( mObject );
				sigassert( mRefCount == 0 );
			}

			++mRefCount;
		}
		void fRemoveClient( )
		{
			--mRefCount;
			sigassert( mRefCount >= 0 );
			if( mRefCount == 0 )
				fReleaseComPtr( mObject );
		}
		operator IDirect3D9*( ) const { return mObject; }
	private:
		tD3DObject( ) : mRefCount( 0 ), mObject( 0 ) { }
		~tD3DObject( ) { sigassert( mRefCount == 0 ); }
	};

	D3DMULTISAMPLE_TYPE tDevice::fConvertMultiSampleType( u32 multiSamplePower )
	{
		tFixedArray<D3DMULTISAMPLE_TYPE,3> d3dMultiSampleTypes;
		d3dMultiSampleTypes[ 0 ] = D3DMULTISAMPLE_NONE;
		d3dMultiSampleTypes[ 1 ] = D3DMULTISAMPLE_2_SAMPLES;
		d3dMultiSampleTypes[ 2 ] = D3DMULTISAMPLE_4_SAMPLES;

		return d3dMultiSampleTypes[ fMin( multiSamplePower, d3dMultiSampleTypes.fCount( ) - 1 ) ];
	}

	void tDevice::fCreatePresentParams( D3DPRESENT_PARAMETERS& d3dpp, const tScreenCreationOptions& opts )
	{
		tRenderTarget::tFormat backBuffFormat = opts.mFormat == tRenderTarget::cFormatNull ? tRenderTarget::cFormatXRGB8 : opts.mFormat;

		d3dpp.BackBufferWidth = opts.mBackBufferWidth;
		d3dpp.BackBufferHeight = opts.mBackBufferHeight;
		d3dpp.BackBufferFormat = tRenderTarget::fConvertFormatType( backBuffFormat );
		d3dpp.BackBufferCount = 0;
		d3dpp.MultiSampleType = fConvertMultiSampleType( opts.mMultiSamplePower );
		d3dpp.MultiSampleQuality = 0;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = (HWND)opts.mWindowHandle;
		d3dpp.Windowed = !opts.mFullScreen;
		d3dpp.EnableAutoDepthStencil = opts.mAutoDepthStencil;
		d3dpp.AutoDepthStencilFormat = opts.mAutoDepthStencil ? D3DFMT_D24S8 : (D3DFORMAT)0;
		d3dpp.Flags = 0;
		d3dpp.FullScreen_RefreshRateInHz = 0;
		switch( opts.mVsync )
		{
			case VSYNC_NONE: d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; break;
			case VSYNC_30HZ: d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_TWO; break;
			case VSYNC_60HZ: //fall-through
			default: d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; break;
		}
	}

	void tDevice::fCreatePresentParamsForMultiSwapChainDevice( D3DPRESENT_PARAMETERS& d3dpp, HWND hwnd )
	{
		tScreenCreationOptions opts;
		opts.mBackBufferWidth = 0;
		opts.mBackBufferHeight = 0;
		opts.mMultiSamplePower = 0;
		opts.mVsync = Gfx::VSYNC_NONE;
		opts.mFullScreen = false;
		opts.mAutoDepthStencil = false;
		opts.mWindowHandle = (u64)hwnd;
		fCreatePresentParams( d3dpp, opts );
	}

	void tDevicePlatformBase::fEnumerateDisplayModes( tDisplayModeList& displayModes )
	{
		tD3DObject::fInstance( ).fAddClient( );
		IDirect3D9* d3d = tD3DObject::fInstance( );

		const u32 adapterModeCount = d3d->GetAdapterModeCount( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8 );
		for( u32 i = 0; i < adapterModeCount; ++i )
		{
			D3DDISPLAYMODE d3ddm;
			if( SUCCEEDED( d3d->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &d3ddm ) ) )
			{
				displayModes.fPushBack( tDisplayMode( d3ddm.Width, d3ddm.Height, 0 ) );
			}
		}

		tD3DObject::fInstance( ).fRemoveClient( );
	}

	tDevice::tDevice( u64 windowHandleGeneric, b32 reference )
		: mDevice( 0 )
	{
		mSingleScreenDevice = false;
		tD3DObject::fInstance( ).fAddClient( );
		D3DPRESENT_PARAMETERS d3dpp;
		fCreatePresentParamsForMultiSwapChainDevice( d3dpp, ( HWND )windowHandleGeneric );
		fCreateDevice( d3dpp, reference );
		fParseCaps( );
		fSetDefaultState( );
	}

	tDevice::tDevice( const tScreenCreationOptions& opts )
	{
		mSingleScreenDevice = true;
		tD3DObject::fInstance( ).fAddClient( );
		D3DPRESENT_PARAMETERS d3dpp;
		fCreatePresentParams( d3dpp, opts );
		fCreateDevice( d3dpp, false );
		fParseCaps( );
		fSetDefaultState( );
	}

	tDevice::tDevice( IDirect3DDevice9* unOwnedDevice )
		: mDevice( unOwnedDevice )
	{
		mBorrowedDevice = true;
		mDevice->AddRef( );
	}

	tDevice::~tDevice( )
	{
		fReleaseComPtr( mDevice );
		if( !mBorrowedDevice )
			tD3DObject::fInstance( ).fRemoveClient( );
	}

	void tDevice::fReset( )
	{
		log_line( Log::cFlagGraphics, "D3D Device was lost, releasing resources..." );
		for( u32 i = 0; i < mDeviceResources.fCount( ); ++i )
			mDeviceResources[ i ]->fOnDeviceLost( this );
		log_line( Log::cFlagGraphics, "... resetting device ..." );
		if( mDevice->Reset( &mCreationPresentParams ) != D3D_OK )
		{
			log_line( Log::cFlagGraphics, "... device is still lost after attempting reset, come back later ..." );
			return;
		}
		log_line( Log::cFlagGraphics, "... re-creating resources ..." );
		for( u32 i = 0; i < mDeviceResources.fCount( ); ++i )
			mDeviceResources[ i ]->fOnDeviceReset( this );
		fSetDefaultState( );
		log_line( Log::cFlagGraphics, "D3D Device was restored." );
	}

	IDirect3D9* tDevice::fGetObject( ) const
	{
		return ( IDirect3D9* )tD3DObject::fInstance( );
	}

	b32 tDevice::fSupportsRenderTargetFormat( D3DFORMAT d3dFormat ) const
	{
		return ( fGetObject( )->CheckDeviceFormat( 
			D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, 
			D3DFMT_X8R8G8B8, 
			D3DUSAGE_RENDERTARGET, 
			D3DRTYPE_SURFACE, 
			d3dFormat ) ) == D3D_OK;
	}

	void tDevice::fCreateDevice( D3DPRESENT_PARAMETERS& d3dpp, b32 reference )
	{
		IDirect3D9* d3dObject = tD3DObject::fInstance( );

		// configure device creation parameters
		const D3DDEVTYPE deviceType = reference ? D3DDEVTYPE_NULLREF : D3DDEVTYPE_HAL;
		const DWORD createFlags = reference ? ( D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE ) : ( D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE );

		// create the device
		HRESULT hr = d3dObject->CreateDevice( 
			D3DADAPTER_DEFAULT,
			deviceType,
			d3dpp.hDeviceWindow,
			createFlags,
			&d3dpp,
			&mDevice );

		if( FAILED( hr ) || !mDevice )
		{
			Log::fFatalError( "Could not initialize graphics module under Direct3D. The application will now abort, sorry." );
		}

		mCreationPresentParams = d3dpp;
	}

	void tDevice::fParseCaps( )
	{
		// get device caps
		D3DCAPS9 dxcaps;
		mDevice->GetDeviceCaps( &dxcaps );

		std::stringstream errorText;
		if( !fMeetsMinimumSpecs( dxcaps, errorText ) )
		{
			errorText << "Your graphics card does not meet the minimum specs. The application will now abort, sorry." << std::endl;
			Log::fFatalError( errorText.str( ).c_str( ) );
		}

		// parse caps
		mCaps.mMaxAnisotropy = dxcaps.MaxAnisotropy;
	}

	b32 tDevice::fMeetsMinimumSpecs( const D3DCAPS9& dxcaps, std::stringstream& errorText ) const
	{
		b32 valid = true;

		// validate caps (better to crash now with reasonable asserts explaining the problem
		// then cryptic crashes later in the program)
		const u8 vsVersionMajor = fHighByte( fLowU16( dxcaps.VertexShaderVersion ) );
		const u8 vsVersionMinor = fLowByte( fLowU16( dxcaps.VertexShaderVersion ) );
		const u8 psVersionMajor = fHighByte( fLowU16( dxcaps.PixelShaderVersion ) );
		const u8 psVersionMinor = fLowByte( fLowU16( dxcaps.PixelShaderVersion ) );
		if( vsVersionMajor < 3 )
		{
			errorText << "Your Vertex Shader does not meet minimum specs of VS 3.0 (your version is " << (u32)vsVersionMajor << "." << (u32)vsVersionMinor << ")." << std::endl;
			valid = false;
		}
		if( psVersionMajor < 3 )
		{
			errorText << "Your Pixel Shader does not meet minimum specs of PS 3.0 (your version is " << (u32)psVersionMajor << "." << (u32)psVersionMinor << ")." << std::endl;
			valid = false;
		}

		return valid;
	}

	void tDevice::fSetDefaultState( )
	{
		// set default state
		mDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
		mDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
		mDevice->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE, FALSE );
	}

	IDirect3DTexture9* tDevice::fCreateSolidColorTexture( u32 width, u32 height, const Math::tVec4f& rgba ) const
	{
		IDirect3DDevice9* d3ddev = fGetDevice( );
		IDirect3DTexture9* d3dtex = 0;

		// create the texture
		HRESULT hr = d3ddev->CreateTexture( width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &d3dtex, 0 );
		sigassert( !FAILED( hr ) && d3dtex );

		// convert color to d3d 32-bit color
		const u32 d3dcolor = D3DCOLOR_RGBA( (u8)(rgba.x * 255.f), (u8)(rgba.y * 255.f), (u8)(rgba.z * 255.f), (u8)(rgba.w * 255.f) );

		// lock the top mip
		D3DLOCKED_RECT rect;
		d3dtex->LockRect( 0, &rect, 0, 0 );

		// now copy the color to the mip bits
		u32* mipBits = ( u32* )rect.pBits;
		const u32 numTexels = rect.Pitch / 4;
		for( u32 iTexel = 0; iTexel < numTexels; ++iTexel )
			*mipBits++ = d3dcolor;

		// unlock top mip
		d3dtex->UnlockRect( 0 );

		return d3dtex;
	}

	void tDevicePlatformBase::fSetShaderGPRAllocation( u32 vsGprs, u32 psGprs )
	{
		// can't control this on pcdx9
	}

	void tDevicePlatformBase::fSetScissorRect( const Math::tRect* rect )
	{
		IDirect3DDevice9* d3ddev = static_cast<tDevice*>( this )->fGetDevice( );

		if( rect )
		{
			d3ddev->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );

			RECT d3drect; 
			d3drect.top		= fRound<int>( rect->mT );
			d3drect.left	= fRound<int>( rect->mL );
			d3drect.bottom	= fRound<int>( rect->mB );
			d3drect.right	= fRound<int>( rect->mR );

			d3ddev->SetScissorRect( &d3drect );
		}
		else
			d3ddev->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
	}

	void tDevicePlatformBase::fSetGammaRamp( f32 gamma )
	{
	}
}}
#endif//#if defined( platform_pcdx9 )

