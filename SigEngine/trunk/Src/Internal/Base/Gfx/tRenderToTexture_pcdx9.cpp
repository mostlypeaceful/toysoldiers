#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tRenderToTexture.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, const tFormat& format, tRenderTarget::tFormat depthFormat, u32 multiSamplePower, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
	{
		mSurfaces.fFill( NULL );
		m3DTextures.fFill( NULL );

		fCreateTextureAndRenderTarget( device, width, height, format, multiSamplePower, numLayers );

		if( mRts.fCount( ) )
		{
			mDt.fReset( NEW tRenderTarget( device, width, height, true, depthFormat, multiSamplePower ) );
			if( mDt->fFailed( ) )
				fReleaseAll( );
		}
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, const tRenderTargetPtr& depthTarget, u32 multiSamplePower, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
	{
		mSurfaces.fFill( NULL );
		m3DTextures.fFill( NULL );

		fCreateTextureAndRenderTarget( device, width, height, format, multiSamplePower, numLayers );

		const tRenderTargetPtr& firstTarget = fRenderTarget( 0 );
		if( firstTarget )
		{
			mDt = depthTarget;
			if( mDt->fFailed( ) || mDt->fWidth( ) < firstTarget->fWidth( ) || mDt->fHeight( ) < firstTarget->fHeight( ) )
				fReleaseAll( );
		}
	}

	tRenderToTexture::tRenderToTexture( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTargetPtr& depthTarget, u32 multiSamplePower, u32 numLayers )
		: tRenderToTexturePlatformBase( numLayers )
	{
		mSurfaces.fFill( NULL );
		m3DTextures.fFill( NULL );

		sigassert( renderTarget && !renderTarget->fFailed( ) );
		sigassert( depthTarget && !depthTarget->fFailed( ) );
		sigassert( depthTarget->fWidth( ) >= renderTarget->fWidth( ) && depthTarget->fHeight( ) >= renderTarget->fHeight( ) );

		mRts.fPushBack( renderTarget );
		fCreateTextureFromRenderTarget( device, 0, numLayers );

		if( mRts[ 0 ] )
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

	void tRenderToTexture::fApply( tScreen& screen ) 
	{ 
		sigassert( mDt && !mDt->fFailed( ) );

		// Apply color targets
		for( u32 i = 0; i < mRts.fCount( ); ++i )
		{
			sigassert( mRts[ i ] );
			mRts[ i ]->fApply( screen.fGetDevice( ), i );
		}

		// Apply depth target
		mDt->fApply( screen.fGetDevice( ), 0 );

		screen.mCurrentTarget.fReset( static_cast<tRenderToTexture*>( this ) );
	}

	void tRenderToTexture::fApplyDepthOnly( tScreen& screen )
	{
		// can't do depth only rendering
		fApply( screen );
	}

	void tRenderToTexture::fEndDepthOnly( tScreen& screen )
	{
		// No-op can't do depth only rendering
	}

	void tRenderToTexture::fResolveDepth( tScreen& screen, u32 slice )
	{
		// can't resolve depth texture on pc. hope the user is ok with this ;/
		fResolve( screen, NULL, slice );
	}

	void tRenderToTexture::fResolve( tScreen& screen, IDirect3DTexture9* targetOverride, u32 slice, b32 unApplyExtras )
	{
		tRenderToTexturePtr safeStoreRef( this );
		screen.mCurrentTarget.fReset( 0 );

		IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );

		for( u32 r = 0; r < mRts.fCount( ); ++r )
		{
			if( mRts[ r ]->fGetSurface( ) != mSurfaces[ r ] )
			{
				sigassert( mSurfaces[ r ] );
				IDirect3DSurface9* resolveTarget = mSurfaces[ r ];
				if( targetOverride )
					targetOverride->GetSurfaceLevel( 0, &resolveTarget );

				HRESULT hr = d3ddev->StretchRect( mRts[ r ]->fGetSurface( ), NULL, resolveTarget, NULL, D3DTEXF_NONE ); 

				if( m3DTextures[ r ] )
				{
					// need to do another copy, to the volume texture, pc dx9 only
					D3DLOCKED_BOX box;
					m3DTextures[ r ]->LockBox( 0, &box, NULL, 0 );

					D3DLOCKED_RECT rect;
					mRts[ r ]->fGetSurface( )->LockRect( &rect, NULL, D3DLOCK_READONLY );

					unsigned char* imagedataIn = (unsigned char*)rect.pBits;
					unsigned char* imagedataOut = (unsigned char*)box.pBits;

					memcpy( imagedataOut + box.SlicePitch * slice, imagedataIn, box.SlicePitch );
					
					m3DTextures[ r ]->UnlockBox( 0 );
					mRts[ r ]->fGetSurface( )->UnlockRect( );
				}

				if( resolveTarget != mSurfaces[ r ] )
					resolveTarget->Release( );
			}

			if( unApplyExtras && r > 0 )
			{
				// we're done with the extra targets
				tRenderTarget::fReset( screen.fGetDevice( ), r );
			}
		}
	}

	void tRenderToTexture::fCreateTextureAndRenderTarget( const tDevicePtr& device, u32 width, u32 height, const tFormat& format, u32 multiSamplePower, u32 numLayers )
	{
		const u32 numTargets = format.fCount( );
		for( u32 i = 0; i < numTargets; ++i )
		{
			mRts.fPushBack( tRenderTargetPtr( NEW tRenderTarget( device, width, height, false, format.mFormats[ i ], multiSamplePower, (numLayers > 1) ) ) );
			fCreateTextureFromRenderTarget( device, i, numLayers );
		}
	}

	void tRenderToTexture::fCreateTextureFromRenderTarget( const tDevicePtr& device, u32 index, u32 numLayers )
	{
		tRenderTargetPtr& renderTarget = mRts[ index ];
		fCreateTexture( device, index, renderTarget->fWidth( ), renderTarget->fHeight( ), renderTarget->fFormat( ), numLayers );

		if( !mRealTexture[ index ] ) 
			return;

		((IDirect3DTexture9*)mRealTexture[ index ])->GetSurfaceLevel( 0, &mSurfaces[ index ] );
		sigassert( mSurfaces[ index ] );

		mTextures[ index ].fSetRaw( ( tTextureReference::tPlatformHandle )(m3DTextures[ index ] ? m3DTextures[ index ] : mRealTexture[ index ]) );
		mTextures[ index ].fSetSamplingModes( tTextureFile::cFilterModeNoMip, tTextureFile::cAddressModeClamp ); // default sampling modes, can be overriden later
	}

	void tRenderToTexture::fCreateTexture( const tDevicePtr& device, u32 index, u32 width, u32 height, tRenderTarget::tFormat format, u32 numLayers )
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		D3DFORMAT d3dFormat = tRenderTarget::fConvertFormatType( format );
		if( format == tRenderTarget::cFormatNull && !device->fSupportsRenderTargetFormat( d3dFormat ) )
			d3dFormat = D3DFMT_X8R8G8B8; // fallback
		
		const s32 numMips = 1;

		// if we need more slices, we need to have a volume texture to copy them to. d3d9 :/
		if( numLayers > 1 )
		{
			IDirect3DVolumeTexture9 * realTexture = 0;
			HRESULT hr = d3ddev->CreateVolumeTexture( width, height, numLayers, numMips, D3DUSAGE_DYNAMIC, d3dFormat, D3DPOOL_DEFAULT, &realTexture, 0 );
			m3DTextures[ index ] = realTexture;
		}

		IDirect3DTexture9* realTexture = 0;
		HRESULT hr = d3ddev->CreateTexture( width, height, numMips, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &realTexture, 0 );
		mRealTexture[ index ] = realTexture;

		if( FAILED( hr ) && mRealTexture[ index ] )
			fReleaseComPtr( mRealTexture[ index ] );
	}

	void tRenderToTexture::fReleaseAll( )
	{
		for( u32 i = 0; i < mRts.fCount( ); ++i )
			mRts[ i ].fRelease( );

		mRts.fSetCount( 0 );
		mDt.fRelease( );
		for( u32 i = 0; i < mRealTexture.fCount( ); ++i )
		{
			fReleaseComPtr( mSurfaces[ i ] );
			fReleaseComPtr( mRealTexture[ i ] );
		}
	}

}}
#endif//#if defined( platform_pcdx9 )
