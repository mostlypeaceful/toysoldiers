#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tRenderToTexture.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, tRenderTarget::tFormat depthFormat )
		: tRenderToTexturePlatformBase( 1 )
		, mTexture( 0 )
		, mSurface( 0 )
	{
		fCreateTextureAndRenderTarget( device, width, height, format );

		if( mRt )
		{
			mDt.fReset( NEW tRenderTarget( device, width, height, depthFormat ) );
			if( mDt->fFailed( ) )
				fReleaseAll( );
		}
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, const tRenderTargetPtr& depthTarget )
		: tRenderToTexturePlatformBase( 1 )
		, mTexture( 0 )
		, mSurface( 0 )
	{
		fCreateTextureAndRenderTarget( device, width, height, format );

		if( mRt )
		{
			mDt = depthTarget;
			if( mDt->fFailed( ) || mDt->fWidth( ) < mRt->fWidth( ) || mDt->fHeight( ) < mRt->fHeight( ) )
				fReleaseAll( );
		}
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTargetPtr& depthTarget )
		: tRenderToTexturePlatformBase( 1 )
		, mTexture( 0 )
		, mSurface( 0 )
	{
		sigassert( renderTarget && !renderTarget->fFailed( ) );
		sigassert( !depthTarget || !depthTarget->fFailed( ) );
		sigassert( !depthTarget || (depthTarget->fWidth( ) >= renderTarget->fWidth( ) && depthTarget->fHeight( ) >= renderTarget->fHeight( )) );

		fCreateTextureFromRenderTarget( device, renderTarget );

		if( mRt )
			mDt = depthTarget;
	}

	tRenderToTexture::~tRenderToTexture( )
	{
		fReleaseAll( );
	}

	b32 tRenderToTexturePlatformBase::fCanRenderToSelf( ) const
	{
		return false;
	}

	b32 tRenderToTexturePlatformBase::fResolveClearsTarget( ) const
	{
		return false;
	}

	void tRenderToTexture::fResolve( tScreen& screen, IDirect3DTexture9* targetOverride, u32 slice )
	{
		log_assert( slice == 0, "Invalid slice index passed to tRenderToTexture::fResolve - array textures not supported on the PC" );

		tRenderToTexturePtr safeStoreRef( this );
		screen.mCurrentTarget.fReset( 0 );

		if( mRt->fGetSurface( ) == mSurface )
			return; // no need to resolve render targets that aren't MSAA

		IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );

		IDirect3DSurface9* resolveTarget = mSurface;
		if( targetOverride )
			targetOverride->GetSurfaceLevel( 0, &resolveTarget );
		sigassert( mSurface );

		d3ddev->StretchRect( mRt->fGetSurface( ), 0, resolveTarget, 0, D3DTEXF_NONE ); 

		if( resolveTarget != mSurface )
			resolveTarget->Release( );
	}

	void tRenderToTexture::fCreateTextureAndRenderTarget( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format )
	{
		fCreateTexture( device, width, height, format );

		if( !mTexture ) return;

		mTexture->GetSurfaceLevel( 0, &mSurface );
		sigassert( mSurface );

		mSurface->AddRef( ); // need an extra add ref before passing surface to render target
		mRt.fReset( NEW tRenderTarget( mSurface, width, height, format ) );
		sigassert( !mRt->fFailed( ) );

		tTextureReference::fSetRaw( ( tPlatformHandle )mTexture );
		tTextureReference::fSetSamplingModes( tTextureFile::cFilterModeNoMip, tTextureFile::cAddressModeClamp ); // default sampling modes, can be overriden later
	}

	void tRenderToTexture::fCreateTextureFromRenderTarget( const tDevicePtr& device, const tRenderTargetPtr& renderTarget )
	{
		fCreateTexture( device, renderTarget->fWidth( ), renderTarget->fHeight( ), renderTarget->fFormat( ) );

		if( !mTexture ) return;

		mTexture->GetSurfaceLevel( 0, &mSurface );
		sigassert( mSurface );

		mRt = renderTarget;

		tTextureReference::fSetRaw( ( tPlatformHandle )mTexture );
		tTextureReference::fSetSamplingModes( tTextureFile::cFilterModeNoMip, tTextureFile::cAddressModeClamp ); // default sampling modes, can be overriden later
	}

	void tRenderToTexture::fCreateTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format )
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		D3DFORMAT d3dFormat = tRenderTarget::fConvertFormatType( format );
		if( format == tRenderTarget::cFormatNull && !device->fSupportsRenderTargetFormat( d3dFormat ) )
			d3dFormat = D3DFMT_X8R8G8B8; // fallback

		const s32 numMips = 1;
		const HRESULT hr = d3ddev->CreateTexture( width, height, numMips, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &mTexture, 0 );
		if( FAILED( hr ) && mTexture )
			fReleaseComPtr( mTexture );
	}

	void tRenderToTexture::fReleaseAll( )
	{
		mRt.fRelease( );
		mDt.fRelease( );
		fReleaseComPtr( mSurface );
		fReleaseComPtr( mTexture );
	}

}}
#endif//#if defined( platform_pcdx9 )
