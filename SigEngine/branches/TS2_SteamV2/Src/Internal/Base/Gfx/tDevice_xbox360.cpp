#include "BasePch.hpp"
#if defined( platform_xbox360 )
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

	void tDevice::fCreatePresentParams( 
		D3DPRESENT_PARAMETERS& d3dpp, 
		u32 bbWidth,
		u32 bbHeight,
		u32 multiSamplePower,
		u32 vsync,
		b32 autoDepthStencil )
	{
		// TODOXBOX360

		fZeroOut( d3dpp );

		d3dpp.BackBufferWidth = bbWidth;
		d3dpp.BackBufferHeight = bbHeight;
		d3dpp.BackBufferFormat = ( D3DFORMAT )/*MAKESRGBFMT*/( D3DFMT_A8R8G8B8 );
		d3dpp.MultiSampleType = fConvertMultiSampleType( multiSamplePower );
		d3dpp.MultiSampleQuality = 0;
		d3dpp.BackBufferCount = 0;
		d3dpp.EnableAutoDepthStencil = FALSE;
		d3dpp.DisableAutoBackBuffer = TRUE;
		d3dpp.DisableAutoFrontBuffer = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.FullScreen_RefreshRateInHz = 0;
		switch( vsync ) {
			case 0: d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; break;
			case 1: d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; break;
			case 2:
			default:d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_TWO; break;
		}

		// resizing the ring buffer supports a larger or smaller amount of draw calls per scene.
		// The default is 2MB of secondary ring buffer; here we'll change that to 4MB.
		D3DRING_BUFFER_PARAMETERS& rbParams = d3dpp.RingBufferParameters;
		rbParams.PrimarySize = 0;  // Direct3D will use the default size of 32KB
		rbParams.SecondarySize = DWORD( 4.f * 1024 * 1024 );
		rbParams.SegmentCount = 0; // Direct3D will use the default segment count of 32

		// Setting the pPrimary and pSecondary members to NULL means that Direct3D will
		// allocate the ring buffers itself.  You can optionally provide a buffer that
		// you allocated yourself (it must be write-combined physical memory, aligned to
		// GPU_COMMAND_BUFFER_ALIGNMENT).
		rbParams.pPrimary = NULL;
		rbParams.pSecondary = NULL;
	}

	void tDevice::fCreatePresentParams(
		D3DPRESENT_PARAMETERS& d3dpp, 
		const tScreenCreationOptions& opts,
		b32 autoDepthStencil )
	{
		fCreatePresentParams( d3dpp, opts.mBackBufferWidth, opts.mBackBufferHeight, opts.mMultiSamplePower, opts.mVsync, autoDepthStencil );
	}

	void tDevicePlatformBase::fEnumerateDisplayModes( tDisplayModeList& displayModes )
	{
		// TODOXBOX360

		// If we are outputting an interlaced mode with less than 720 lines (360 lines per field),
		// we cannot use a 1920 x 1080 front buffer because the video scaler cannot downsample
		// that far.

		//XVIDEO_MODE videoMode;
		//XGetVideoMode( &videoMode );

		//u32 effectiveHeight = videoMode.dwDisplayHeight;
		//if( videoMode.fIsInterlaced )
		//	effectiveHeight /= 2;

		//b32 allow1080 = true;
		//if( ( 1080.0f / ( f32 )effectiveHeight ) > 3.0f )
		//	allow1080 = false;

		//displayModes.fPushBack( tDisplayMode( videoMode.dwDisplayWidth, videoMode.dwDisplayHeight, 0 ) );

		displayModes.fPushBack( tDisplayMode( 1280, 720, 0 ) );
	}

	tDevice::tDevice( const tScreenCreationOptions& opts )
	{
		mSingleScreenDevice = true;
		tD3DObject::fInstance( ).fAddClient( );
		D3DPRESENT_PARAMETERS d3dpp;
		fCreatePresentParams( d3dpp, opts, true );
		fCreateDevice( d3dpp );
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
		if( mDevice->Reset( &mCreationPresentParams ) == D3DERR_DEVICELOST )
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

	void tDevice::fCreateDevice( D3DPRESENT_PARAMETERS& d3dpp )
	{
		IDirect3D9* d3dObject = tD3DObject::fInstance( );

		// configure device creation parameters
		const D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;
		const DWORD d3dThreadFlags = 0/*D3DCREATE_CREATE_THREAD_ON_2 | D3DCREATE_CREATE_THREAD_ON_3*/; // didn't observe any benefit from using these, so went back to 0 (default)
		const DWORD createFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_BUFFER_2_FRAMES | d3dThreadFlags;

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

		// the sole purpose of this gamma ramp is to empirically try to match the PC
		mDevice->GetGammaRamp( 0, &mGammaRamp );
		const u32 arrayLen = array_length( mGammaRamp.red );
		const f32 maxVal = ( f32 )0xffc0; // empirical
		for( u32 i = 0; i < arrayLen; ++i )
		{
			const f32 zeroToOne = ( i / ( arrayLen - 1.f ) );
			const f32 r = 1.000f * zeroToOne;
			const f32 g = 1.000f * zeroToOne;
			const f32 b = 1.000f * zeroToOne;
			const f32 gammaValR = powf( r, 0.85f );
			const f32 gammaValG = powf( g, 0.85f );
			const f32 gammaValB = powf( b, 0.85f );
			mGammaRamp.red[ i ]		= fRound<u32>( gammaValR * maxVal );
			mGammaRamp.green[ i ]	= fRound<u32>( gammaValG * maxVal );
			mGammaRamp.blue[ i ]	= fRound<u32>( gammaValB * maxVal );
		}
		mDevice->SetGammaRamp( 0, D3DSGR_IMMEDIATE, &mGammaRamp );
	}

	void tDevice::fParseCaps( )
	{
		// parse caps
		mCaps.mMaxAnisotropy = 8;
	}

	void tDevice::fSetDefaultState( )
	{
		const f32 lineWidth = 2.f;
		mDevice->SetRenderState( D3DRS_LINEWIDTH, *( DWORD* )&lineWidth );

		// cut out comparison function
		mDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

		// set texture array state (no filtering between slices, clamp to first and last slice)
		for( u32 i = 0; i < 16; ++i )
		{
			mDevice->SetSamplerState( i, D3DSAMP_SEPARATEZFILTERENABLE, TRUE );
			mDevice->SetSamplerState( i, D3DSAMP_MINFILTERZ, D3DTEXF_POINT );
			mDevice->SetSamplerState( i, D3DSAMP_MAGFILTERZ, D3DTEXF_POINT );
			mDevice->SetSamplerState( i, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP );
			mDevice->SetSamplerState( i, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_THREEEIGHTHS );
		}
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

	devvar_clamp( u32, Renderer_GPRS_ShadowMapPS,	2, 1, 6, 0 );
	devvar_clamp( u32, Renderer_GPRS_PostProcessVS, 2, 1, 6, 0 );
	devvar_clamp( u32, Renderer_GPRS_WorldVS,		3, 1, 6, 0 );

	void tDevicePlatformBase::fSetShaderGPRAllocation( b32 vertexHeavy, b32 pixelHeavy )
	{
		IDirect3DDevice9* d3ddev = static_cast<tDevice*>( this )->fGetDevice( );

		const u32 shadowMapPS = Renderer_GPRS_ShadowMapPS * 16;
		const u32 postProcessVS = Renderer_GPRS_PostProcessVS * 16;
		const u32 worldVS = Renderer_GPRS_WorldVS * 16;

		if( vertexHeavy && !pixelHeavy )
			d3ddev->SetShaderGPRAllocation( 0, 128 - shadowMapPS, shadowMapPS );
		else if( !vertexHeavy && pixelHeavy )
			d3ddev->SetShaderGPRAllocation( 0, postProcessVS, 128 - postProcessVS );
		else if( vertexHeavy && pixelHeavy )
			d3ddev->SetShaderGPRAllocation( 0, worldVS, 128 - worldVS );
		else
			d3ddev->SetShaderGPRAllocation( 0, 0, 0 ); // restores to default
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
		IDirect3DDevice9* d3ddev = static_cast<tDevice*>( this )->fGetDevice( );

		const f32 power = Math::fLerp( 1.f / 0.9f, 1.f / 2.2f, fClamp( gamma, 0.f, 1.f ) );
		D3DGAMMARAMP ramp;
		for( u32 i = 0; i < 256; ++i )
		{
			const u32 value = u32( powf( i / 255.f, power ) * 65535 );
			ramp.red[ i ] = value;
			ramp.green[ i ] = value;
			ramp.blue[ i ] = value;
		}

		d3ddev->SetGammaRamp( 0, D3DSGR_NO_CALIBRATION, &ramp );
	}

}}
#endif//#if defined( platform_xbox360 )

