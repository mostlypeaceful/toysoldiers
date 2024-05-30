//------------------------------------------------------------------------------
// \file tGameArchiveSave.cpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tGameArchiveSave.hpp"

namespace Sig
{
	tGameArchiveSave::tGameArchiveSave( )
		: tGameArchive( cGameArchiveModeSave )
	{
	}

	tGameArchiveSave::tGameArchiveSave( tFilePathArchiver* filePathArchiver )
		: tGameArchive( cGameArchiveModeSave )
	{
		mFilePathArchiver = filePathArchiver;
	}

	void tGameArchiveSave::fPut( const void* data, u32 numBytes )
	{
		if( mBuffer.fCapacity( ) < mBuffer.fCount( ) + numBytes )
			mBuffer.fGrowCapacity( fMax( fClamp<u32>( 2 * mBuffer.fCapacity( ), 256, 10*1024 ), numBytes ) );

		u32 start = mBuffer.fCount( );
		mBuffer.fSetCount( start + numBytes );
		memcpy( mBuffer.fBegin( ) + start, data, numBytes );
	}

	tGrowableArray<Sig::byte>& tGameArchiveSave::fBuffer( )
	{
		return mBuffer;
	}

	const tGrowableArray<Sig::byte>& tGameArchiveSave::fBuffer( ) const
	{
		return mBuffer;
	}

	void tGameArchiveSave::fTransferBufferTo( tGrowableBuffer& buffer )
	{
		mBuffer.fDisown( buffer );
	}

	void tGameArchiveSave::fEncrypt( )
	{
		// encrypt buffer
		tGrowableArray<byte> encrypted;
		tEncryption::fEncrypt( mBuffer, encrypted );
		mBuffer.fSwap( encrypted );
	}
}
