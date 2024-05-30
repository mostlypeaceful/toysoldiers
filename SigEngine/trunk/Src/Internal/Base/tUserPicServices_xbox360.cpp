//------------------------------------------------------------------------------
// \file tUserPicServices_xbox360.cpp - 15 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tUserPicServices.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tUserPicServices
	//------------------------------------------------------------------------------
	void tUserPicServices::fPlatformShutdown( )
	{
		tPictureMap::tIterator itr = mPictureMap.fBegin( );
		tPictureMap::tIterator end = mPictureMap.fEnd( );
		for( ; itr != end; ++itr )
		{
			if( itr->fNullOrRemoved( ) )
				continue;

			tPicture & pic = *itr->mValue;
			if( pic.mState == cStateBusy )
			{
				sigcheckfail_xoverlapped_done_else_wait_cancel( &pic.mOverlapped );
				pic.mState = cStateNull;
			}
		}
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fQueryPic( const tPictureKey & key, tPicture & picture )
	{
		sigassert( picture.mState == cStateNull && "Sanity!" );

		fCreateTexture( picture.mTexture, key.mSmall );

		IDirect3DTexture9* d3dTex = (IDirect3DTexture9*)picture.mTexture.fGetRaw( );
		picture.mState = cStateBusy;

		// Query for the picture
		{
			D3DLOCKED_RECT rect;
			d3dTex->LockRect( 0, &rect, NULL, 0 );

			key.mKey.fGetPlatformSpecific( &picture.mPlatformKey );

			fZeroOut( picture.mOverlapped );
			DWORD result = XUserReadGamerPictureByKey(
				&picture.mPlatformKey, 
				key.mSmall ? TRUE : FALSE, 
				(PBYTE)rect.pBits, 
				rect.Pitch, 
				key.mSmall ? 32 : 64, 
				&picture.mOverlapped );

			sigassert( result == ERROR_IO_PENDING );
		}
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fProcessPic( tPicture & picture )
	{
		sigassert( picture.mState == cStateBusy && "Sanity!" );

		if( !XHasOverlappedIoCompleted( &picture.mOverlapped ) )
			return;

		DWORD error = XGetOverlappedExtendedError( &picture.mOverlapped );
		sigassert( error == ERROR_SUCCESS );

		IDirect3DTexture9* d3dTex = (IDirect3DTexture9*)picture.mTexture.fGetRaw( );
		d3dTex->UnlockRect( 0 );

		picture.mState = cStateReady;
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fCreateTexture( Gfx::tTextureReference & out, b32 small )
	{
		sigassert( !out.fGetRaw( ) );

		const Gfx::tDevicePtr & device = Gfx::tDevice::fGetDefaultDevice( );

		IDirect3DDevice9* d3ddev = device->fGetDevice( );
		IDirect3DTexture9* d3dtex = 0;

		const u32 dim = small ? 32 : 64; // From XDK docs

		// create the texture
		Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextTexture ) );

		HRESULT hr = d3ddev->CreateTexture( dim, dim, 1, 0, D3DFMT_LIN_A8R8G8B8, 0, &d3dtex, 0 );
		sigassert( SUCCEEDED( hr ) && d3dtex );

		out.fSetRaw( (Gfx::tTextureReference::tPlatformHandle)d3dtex );
		out.fSetSamplingModes( 
			Gfx::tTextureFile::cFilterModeNone, 
			Gfx::tTextureFile::cAddressModeClamp );

		Memory::tHeap::fResetVramContext( );
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fDestroyTexture( Gfx::tTextureReference & in )
	{
		IDirect3DTexture9* d3dtex = (IDirect3DTexture9*)in.fGetRaw( );
		sigassert( d3dtex );

		Gfx::fReleaseComPtr( d3dtex );
		in.fSetRaw( NULL );
	}

} // ::Sig

