//------------------------------------------------------------------------------
// \file tGameArchiveLoad.cpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tGameArchive.hpp"

namespace Sig
{
	tGameArchiveLoad::tGameArchiveLoad( const ::Sig::byte* data, u32 dataLen )
		: tGameArchive( cGameArchiveModeLoad )
		, mData( data )
		, mDataLen( dataLen )
		, mReadPos( 0 )
	{
		mHeader.mPlatform = cCurrentPlatform;
		mHeader.mVersion = 0;
	}

	tGameArchiveLoad::tGameArchiveLoad( const ::Sig::byte* data, u32 dataLen, tFilePathArchiver* filePathArchiver )
		: tGameArchive( cGameArchiveModeLoad )
		, mData( data )
		, mDataLen( dataLen )
		, mReadPos( 0 )
	{
		mHeader.mPlatform = cCurrentPlatform;
		mHeader.mVersion = 0;
		mFilePathArchiver = filePathArchiver;
	}

	u32 tGameArchiveLoad::fPeekHeaderVersion( )
	{
		mSafeToSaveLoad = true;
		fSaveLoad( mHeader );
		fResetReadPos( );
		mSafeToSaveLoad = false;
		return mHeader.mVersion;
	}



	void tGameArchiveLoad::fResetReadPos( )
	{
		mReadPos = 0;
	}

	void tGameArchiveLoad::fGet( void* data, u32 numBytes )
	{
		if( 0 == numBytes )
			return;

		sigassert( data );
		sigassert( fCurrentPos( ) );

		if( fBytesRemaining() < numBytes )
		{
			log_warning( "tGameArchiveLoad::fGet Ran out of bytes" );
			fFail();
			return;
		}

		fMemCpy( data, fCurrentPos( ), numBytes );
		fAdvance( numBytes );
	}

	const ::Sig::byte* tGameArchiveLoad::fCurrentPos( ) const
	{
		return mData + mReadPos;
	}

	u32 tGameArchiveLoad::fBytesRemaining() const
	{
		return mDataLen>mReadPos ? mDataLen-mReadPos : 0u;
	}

	void tGameArchiveLoad::fAdvance( u32 numBytes )
	{
		if( fBytesRemaining() < numBytes )
		{
			log_warning( "tGameArchiveLoad::fAdvance Ran out of bytes" );
			fFail();
			return;
		}

		mReadPos += numBytes;
	}

	b32 tGameArchiveLoad::fSanityCheckArrayAlloc( u32 typeSize, u32 arrayCount )
	{
		// TODO/FEATURE: Advisory "max alloc" limit?
		if( fBytesRemaining() < arrayCount )
		{
			log_warning( "tGameArchiveLoad::fSanityCheckArrayAlloc failed: bytes remaining (" << fBytesRemaining( ) << ") < arrayCount (" << arrayCount << ")" );
			fFail();
		}

		return !fFailed();
	}

	b32 tGameArchiveLoad::fDecrypt( )
	{
		tGrowableArray<byte> encrypted;
		encrypted.fInsert( 0, mData, mDataLen );

		tEncryption::fDecrypt( encrypted, mDecryptedBuffer );
		mData = mDecryptedBuffer.fBegin( );
		mDataLen = mDecryptedBuffer.fCount( );

		return (mDecryptedBuffer.fCount( ) != 0);
	}
}
