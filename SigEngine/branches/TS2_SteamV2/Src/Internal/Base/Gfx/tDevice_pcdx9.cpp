#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tDevice.hpp"
#include "tDeviceResource.hpp"
#include "tScreen.hpp"
#include "tApplication.hpp"
#include "tSceneGraph.hpp"
#include "tGameAppBase.hpp"

namespace Sig { namespace Gfx
{
	devvar( b32, Renderer_Settings_IgnoreGamma, 1 );

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
		d3dMultiSampleTypes[ 2 ] = D3DMULTISAMPLE_8_SAMPLES;

		return d3dMultiSampleTypes[ fMin( multiSamplePower, d3dMultiSampleTypes.fCount( ) - 1 ) ];
	}

	void tDevice::fCreatePresentParams( 
		D3DPRESENT_PARAMETERS& d3dpp, 
		HWND hwnd,
		u32 bbWidth,
		u32 bbHeight,
		u32 multiSamplePower,
		u32 vsync,
		b32 windowed,
		b32 autoDepthStencil )
	{
		d3dpp.BackBufferWidth = bbWidth;
		d3dpp.BackBufferHeight = bbHeight;
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.BackBufferCount = 0;
		d3dpp.MultiSampleType = fConvertMultiSampleType( multiSamplePower );
		d3dpp.MultiSampleQuality = 0;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = hwnd;
		d3dpp.Windowed = windowed;
		d3dpp.EnableAutoDepthStencil = autoDepthStencil;
		d3dpp.AutoDepthStencilFormat = autoDepthStencil ? D3DFMT_D24S8 : (D3DFORMAT)0;
#if defined(target_game)
		d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
#else
		d3dpp.Flags = 0;
#endif
		d3dpp.FullScreen_RefreshRateInHz = 0;
		switch( vsync ) {
			case 0: d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; break;
			case 2: d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_TWO; break;
			case 1:
			default:d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; break;
		}
	}

	void tDevice::fCreatePresentParams(
		D3DPRESENT_PARAMETERS& d3dpp, 
		const tScreenCreationOptions& opts,
		b32 autoDepthStencil )
	{
		HWND hwnd = ( HWND )opts.mWindowHandle;
		fCreatePresentParams( d3dpp, hwnd, opts.mBackBufferWidth, opts.mBackBufferHeight, opts.mMultiSamplePower, opts.mVsync, !opts.mFullScreen, autoDepthStencil );
	}

	void tDevice::fCreatePresentParamsForMultiSwapChainDevice( D3DPRESENT_PARAMETERS& d3dpp, HWND hwnd )
	{
		fCreatePresentParams( d3dpp, hwnd, 0, 0, 0, false, true, false );
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
		, mHasFocus( 1 )
	{
		mHasBufferedWindowDimensions = false;
		mHasBufferedFullscreenDimensions = false;
		mBufferedWindowX = CW_USEDEFAULT;
		mBufferedWindowY = CW_USEDEFAULT;
		mNeedsReset = false;

		mSingleScreenDevice = false;
		tD3DObject::fInstance( ).fAddClient( );
		D3DPRESENT_PARAMETERS d3dpp;
		fCreatePresentParamsForMultiSwapChainDevice( d3dpp, ( HWND )windowHandleGeneric );
		fCreateDevice( d3dpp, reference );
		fParseCaps( );
		fSetDefaultState( );
	}

	tDevice::tDevice( const tScreenCreationOptions& opts )
		: mDevice( 0 )
		, mHasFocus( 1 )
	{
		mHasBufferedWindowDimensions = false;
		mHasBufferedFullscreenDimensions = false;
		mBufferedWindowX = CW_USEDEFAULT;
		mBufferedWindowY = CW_USEDEFAULT;
		mNeedsReset = false;

		mSingleScreenDevice = true;
		tD3DObject::fInstance( ).fAddClient( );
		D3DPRESENT_PARAMETERS d3dpp;
		fCreatePresentParams( d3dpp, opts, true );
		fCreateDevice( d3dpp, false );
		fParseCaps( );
		fSetDefaultState( );
	}

	tDevice::tDevice( IDirect3DDevice9* unOwnedDevice )
		: mDevice( unOwnedDevice )
		, mHasFocus( 1 )
	{
		mHasBufferedWindowDimensions = false;
		mHasBufferedFullscreenDimensions = false;
		mBufferedWindowX = CW_USEDEFAULT;
		mBufferedWindowY = CW_USEDEFAULT;
		mNeedsReset = false;

		mBorrowedDevice = true;
		mDevice->AddRef( );
	}

	tDevice::~tDevice( )
	{
		fReleaseComPtr( mDevice );
		if( !mBorrowedDevice )
			tD3DObject::fInstance( ).fRemoveClient( );
	}

	b32 tDevice::fPrepareForGameTick()
	{
		// text if device is lost...
		HRESULT hr = mDevice->TestCooperativeLevel();
		if (hr != D3D_OK || fNeedsReset())
		{
			if( fHasFocus( ) )
				fReset();
			return false;
		}
		return true;
	}

	void tDevice::fResetDisplayMode( u32 width, u32 height, b32 fullscreen, u32 vsync )
	{
		switch (vsync)
		{
		case 0:		mCreationPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;		break;
		case 1:		mCreationPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;      		break;
		default: break;
		}

		mCreationPresentParams.BackBufferWidth = width;
		mCreationPresentParams.BackBufferHeight = height;
		mCreationPresentParams.Windowed = !fullscreen;
		mNeedsReset = true;
	}

	void tDevice::fGetDisplayMode( u32 &width, u32 &height, b32 &fullscreen, u32 &vsync )
	{
		switch (mCreationPresentParams.PresentationInterval)
		{
		case D3DPRESENT_INTERVAL_IMMEDIATE:
			vsync = 0;
			break;
		case D3DPRESENT_INTERVAL_ONE:
			vsync = 1;
			break;
		default:
			vsync = 2;
			break;
		}

		fullscreen = !mCreationPresentParams.Windowed;
		width = mCreationPresentParams.BackBufferWidth;
		height = mCreationPresentParams.BackBufferHeight;
	}

	// we won't really use fullscreen mode if we are using a virtual (multi-monitor) display size
	// the rest of the system doesn't need to know this (and should behave like its in fullscreen mode)
	// also, we can't do INTERVAL TWO in windowed mode
	static void ModifyPresentParamsForUse( D3DPRESENT_PARAMETERS &modifiedParams, const D3DPRESENT_PARAMETERS &sourceParams )
	{
		modifiedParams = sourceParams;

		IDirect3D9* d3dObject = tD3DObject::fInstance( );
		DWORD qualityLevels;
		while (modifiedParams.MultiSampleType > 0)
		{
			HRESULT res = d3dObject->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, sourceParams.BackBufferFormat, sourceParams.Windowed, modifiedParams.MultiSampleType, &qualityLevels);
			if (FAILED(res))
			{
				modifiedParams.MultiSampleType = (D3DMULTISAMPLE_TYPE)(modifiedParams.MultiSampleType-1);
				log_line(0,"Downgrading Multisampling to " << modifiedParams.MultiSampleType);
			}
			else
			{
				break;
			}
		}

		u32 displayWidth = (u32)GetSystemMetrics(SM_CXSCREEN);
		u32 displayHeight = (u32)GetSystemMetrics(SM_CYSCREEN);
		if (modifiedParams.Windowed == true)
		{
			if (modifiedParams.PresentationInterval == D3DPRESENT_INTERVAL_TWO)
				modifiedParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			modifiedParams.FullScreen_RefreshRateInHz = 0;
			modifiedParams.BackBufferWidth = fMin((u32)modifiedParams.BackBufferWidth,displayWidth);
			modifiedParams.BackBufferHeight = fMin((u32)modifiedParams.BackBufferHeight,displayHeight);
		}
		else
		{
			// snap to the closest matching resolution...
			DEVMODE dm;
			int i=0;
			int bestdiff = 10000;
			int closestWidth = 0;
			int closestHeight = 0;
			DWORD freq = 0;

			// find the closest resolution
			while( EnumDisplaySettings(NULL,i++, &dm ) ) 
			{
				int diff = fAbs((int)dm.dmPelsWidth - (int)modifiedParams.BackBufferWidth) + fAbs((int)dm.dmPelsHeight - (int)modifiedParams.BackBufferHeight);
				if (diff <= bestdiff)
				{
					closestWidth = dm.dmPelsWidth;
					closestHeight = dm.dmPelsHeight;
					bestdiff = diff;
				}
			}

			modifiedParams.BackBufferWidth = closestWidth;
			modifiedParams.BackBufferHeight = closestHeight;

			// find the highest frequency
			i = 0;
			modifiedParams.FullScreen_RefreshRateInHz = 0;
			while( EnumDisplaySettings(NULL,i++, &dm ) ) 
			{
				if (dm.dmPelsWidth == modifiedParams.BackBufferWidth && dm.dmPelsHeight == modifiedParams.BackBufferHeight)
				{
					modifiedParams.FullScreen_RefreshRateInHz = fMax((u32)modifiedParams.FullScreen_RefreshRateInHz, (u32)dm.dmDisplayFrequency);
				}
			}
		}

#if defined( target_game )
		log_line( Log::cFlagGraphics, "Modify Reset Params: "
			<< sourceParams.BackBufferWidth << "x" << sourceParams.BackBufferHeight << " " << (sourceParams.Windowed ? "WINDOWED" : "FULLSCREEN") << " @"
			<< sourceParams.FullScreen_RefreshRateInHz << "Hz " << sourceParams.PresentationInterval << " ===> "
			<< modifiedParams.BackBufferWidth << "x" << modifiedParams.BackBufferHeight << " " << (modifiedParams.Windowed ? "WINDOWED" : "FULLSCREEN") << " @"
			<< modifiedParams.FullScreen_RefreshRateInHz << "Hz " << modifiedParams.PresentationInterval );
#endif
	}

	void tDevice::fSetMultisamplePower( int multisamplePower )
	{
		mCreationPresentParams.MultiSampleType = fConvertMultiSampleType( multisamplePower );
	}

	void tDevice::fReset( )
	{
		D3DPRESENT_PARAMETERS modifiedPresentParams;
		ModifyPresentParamsForUse( modifiedPresentParams, mCreationPresentParams );

		log_line( Log::cFlagGraphics, "D3D Device was lost, releasing resources..." );
		for( u32 i = 0; i < mDeviceResources.fCount( ); ++i )
			mDeviceResources[ i ]->fOnDeviceLost( this );
		log_line( Log::cFlagGraphics, "... resetting device ..." );
		HRESULT result = mDevice->Reset( &modifiedPresentParams );
		if (FAILED(result))
		{
			if (result == D3DERR_DEVICELOST)
			{
				log_line( Log::cFlagGraphics, "... device is still lost after attempting reset, come back later ..." );
			}
			else
			{
				log_line( Log::cFlagGraphics, "... failed to reset ..." );
			}
			return;
		}
		tApplication::fInstance( ).fSceneGraph( )->fWaitForLogicRunListsToComplete();
		log_line( Log::cFlagGraphics, "... re-creating resources ..." );
		for( u32 i = 0; i < mDeviceResources.fCount( ); ++i )
			mDeviceResources[ i ]->fOnDeviceReset( this );
		tGameAppBase::fInstance( ).fOnDeviceReset( );

		fSetDefaultState( );
		mNeedsReset = false;
		tGameAppBase::fInstance( ).fShowWindow( mCreationPresentParams.BackBufferWidth, mCreationPresentParams.BackBufferHeight, mBufferedWindowX, mBufferedWindowY, false, !mCreationPresentParams.Windowed );
		log_line( Log::cFlagGraphics, "D3D Device was restored." );
		tGameAppBase::fInstance( ).fResetPlayerCameraAspectRatios( );

		// inform systems that the params have changed...
		for( u32 i=0; i<mOnParamsChanged.fCount( ); i++ )
		{
			mOnParamsChanged[i]( tDevicePtr( this ) );
		}
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

	b32 tDevice::fOwns( IDirect3DResource9* resource ) const
	{
		sigassert( resource );

		IDirect3DDevice9* resourceDevice = 0;
		HRESULT hr = resource->GetDevice(&resourceDevice);
		sigassert( SUCCEEDED(hr) );

		const b32 result = mDevice == resourceDevice;

		resourceDevice->Release();
		return result;
	}

	b32 tDevice::fOwns( IDirect3DVertexDeclaration9* resource ) const
	{
		sigassert( resource );

		IDirect3DDevice9* resourceDevice = 0;
		HRESULT hr = resource->GetDevice(&resourceDevice);
		sigassert( SUCCEEDED(hr) );

		const b32 result = mDevice == resourceDevice;

		resourceDevice->Release();
		return result;
	}

	void tDevice::fCreateDevice( D3DPRESENT_PARAMETERS& d3dpp, b32 reference )
	{
		IDirect3D9* d3dObject = tD3DObject::fInstance( );

		// configure device creation parameters
		const D3DDEVTYPE deviceType = reference ? D3DDEVTYPE_NULLREF : D3DDEVTYPE_HAL;
		const DWORD createFlags = reference ? ( D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE ) : ( D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE );

		ModifyPresentParamsForUse( mCreationPresentParams, d3dpp );

		// create the device
		HRESULT hr = d3dObject->CreateDevice( 
			D3DADAPTER_DEFAULT,
			deviceType,
			d3dpp.hDeviceWindow,
			createFlags,
			&mCreationPresentParams,
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

	void tDevice::fToggleFullscreen( )
	{
		// save current toggle settings...
		if (mCreationPresentParams.Windowed)
		{
			mHasBufferedWindowDimensions = true;
			tGameAppBase::fInstance( ).fGetWindowPosition(mBufferedWindowX, mBufferedWindowY);
			mBufferedWindowWidth = mCreationPresentParams.BackBufferWidth;
			mBufferedWindowHeight = mCreationPresentParams.BackBufferHeight;
		}
		else
		{
			mHasBufferedFullscreenDimensions = true;
			mBufferedFullscreenWidth = mCreationPresentParams.BackBufferWidth;
			mBufferedFullscreenHeight = mCreationPresentParams.BackBufferHeight;
		}


		// we need the current device rez...
		DEVMODE dm;
		EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS, &dm );

		if ( mCreationPresentParams.Windowed )
		{
			if (mHasBufferedFullscreenDimensions)
			{
				mCreationPresentParams.BackBufferWidth = mBufferedFullscreenWidth;
				mCreationPresentParams.BackBufferHeight = mBufferedFullscreenHeight;
			}
			else
			{
				mCreationPresentParams.BackBufferWidth = dm.dmPelsWidth;
				mCreationPresentParams.BackBufferHeight = dm.dmPelsHeight;
			}
			mCreationPresentParams.FullScreen_RefreshRateInHz = 0;
		}
		else
		{
			if (mHasBufferedWindowDimensions)
			{
				mCreationPresentParams.BackBufferWidth = mBufferedWindowWidth;
				mCreationPresentParams.BackBufferHeight = mBufferedWindowHeight;
			}
			else
			{
				mCreationPresentParams.BackBufferWidth = fMin((u32)1280, (u32)dm.dmPelsWidth);
				mCreationPresentParams.BackBufferHeight = fMin((u32)720, (u32)dm.dmPelsHeight);
			}

			if (mCreationPresentParams.PresentationInterval == D3DPRESENT_INTERVAL_TWO)
				mCreationPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			mCreationPresentParams.FullScreen_RefreshRateInHz = 0;
		}
		mCreationPresentParams.Windowed = !mCreationPresentParams.Windowed;
		mNeedsReset = true;
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

	void tDevicePlatformBase::fSetShaderGPRAllocation( b32 vertexHeavy, b32 pixelHeavy )
	{
		// can't control this on pcdx9
	}

	Math::tRect tDevicePlatformBase::fAdjustRectFor16_9( const Math::tRect &rect )
	{
		float rL = (float)rect.mL / 1280.0f;
		float rR = (float)rect.mR / 1280.0f;
		float rT = (float)rect.mT / 720.0f;
		float rB = (float)rect.mB / 720.0f;

		// adjust the scissor rect for 1280x720 aspect ratio
		// expect in 'rect' to be in 1280x720 coords
		tDevice *dev = static_cast<tDevice*>( this );
		int width = dev->fGetCreationPresentParams( ).BackBufferWidth;
		int height = dev->fGetCreationPresentParams( ).BackBufferHeight;

		float dL = 0.0f;
		float dR = (float)width;
		float dT = 0.0f;
		float dB = (float)height;

		float currentAspectRatio = (float)width / (float)height;
		float requiredAspectRatio = 1280.0f / 720.0f;
		if (currentAspectRatio > requiredAspectRatio)
		{
			float c = (1.0f - (requiredAspectRatio / currentAspectRatio)) * 0.5f * (float)width;
			dL += c;
			dR -= c;
		}
		else
		{
			float c = (1.0f - (currentAspectRatio / requiredAspectRatio)) * 0.5f * (float)height;
			dT += c;
			dB -= c;
		}

		f32 dH = dB - dT;
		f32 dW = dR - dL;
		Math::tRect outRect;
		outRect.mL = dL + rL * dW;
		outRect.mR = dL + rR * dW;
		outRect.mT = dT + rT * dH;
		outRect.mB = dT + rB * dH;
		return outRect;
	}

	void tDevicePlatformBase::fSetScissorRect( const Math::tRect* rect )
	{
		tDevice *dev = static_cast<tDevice*>( this );
		IDirect3DDevice9* d3ddev = dev->fGetDevice( );
		if( rect )
		{
			d3ddev->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );

			Math::tRect outRect = fAdjustRectFor16_9( *rect );
			RECT d3drect; 
			d3drect.top		= fRound<int>( outRect.mT );
			d3drect.left	= fRound<int>( outRect.mL );
			d3drect.bottom	= fRound<int>( outRect.mB );
			d3drect.right	= fRound<int>( outRect.mR );

			d3ddev->SetScissorRect( &d3drect );
		}
		else
			d3ddev->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
	}

	void tDevicePlatformBase::fSetGammaRamp( f32 gamma )
	{
		if( Renderer_Settings_IgnoreGamma )
			return;

		HDC displayContext = GetDC(NULL);

		if (displayContext != NULL)
		{
			WORD gammaRamp[3][256];

			for (size_t i = 0; i < 256; i++)
			{
				int value = ( int ) i * ( int ( gamma * 255 ) + 128 );
				value = value > 65535 ? 65535 : value;

				gammaRamp[ 0 ][ i ] = ( WORD )value;
				gammaRamp[ 1 ][ i ] = ( WORD )value;
				gammaRamp[ 2 ][ i ] = ( WORD )value;
			}

			SetDeviceGammaRamp( displayContext, gammaRamp );
			ReleaseDC(NULL, displayContext);
		}
	}
}}
#endif//#if defined( platform_pcdx9 )

