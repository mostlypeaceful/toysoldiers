#include "BasePch.hpp"
#include "tGameArchive.hpp"

namespace Sig
{
	tGameArchiveSave::tGameArchiveSave( )
		: tGameArchive( cModeSave )
	{
	}

	void tGameArchiveSave::fPut( const void* data, u32 numBytes )
	{
		if( mBuffer.fCapacity( ) < mBuffer.fCount( ) + numBytes )
			mBuffer.fGrowCapacity( fMax( fClamp<u32>( 2 * mBuffer.fCapacity( ), 256, 10*1024 ), numBytes ) );

		u32 start = mBuffer.fCount( );
		mBuffer.fSetCount( start + numBytes );
		memcpy( mBuffer.fBegin( ) + start, data, numBytes );
	}

}



namespace Sig
{
	tGameArchiveLoad::tGameArchiveLoad( const ::Sig::byte* data, u32 dataLen )
		: tGameArchive( cModeLoad )
		, mData( data )
		, mDataLen( dataLen )
		, mReadPos( 0 )
	{
	}

	void tGameArchiveLoad::fGet( void* data, u32 numBytes )
	{
		sigassert( mReadPos + numBytes <= mDataLen );
		fMemCpy( data, fCurrentPos( ), numBytes );
		fAdvance( numBytes );
	}

	void tGameArchiveLoad::fAdvance( u32 numBytes )
	{
		sigassert( mReadPos + numBytes <= mDataLen );
		mReadPos += numBytes;
	}

}
