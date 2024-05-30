#include "BasePch.hpp"
#include "tLoadInPlaceResourceBuffer.hpp"
#include "Memory/tResourceMemoryProvider.hpp"
#include "tApplication.hpp"
#include "tProfiler.hpp"

namespace Sig
{
	namespace
	{
		static Sig::byte* fResourceAlloc( u32 numBytes, const Memory::tAllocStamp& stamp ) 
		{ 
			profile_mem( cProfileMemRes, numBytes );
			return ( Sig::byte* )Memory::tResourceMemoryProvider::fInstance( ).fAlloc( numBytes, stamp ); 
		}
		static void fResourceFree( void* mem, u32 size ) 
		{ 
			profile_mem( cProfileMemRes, -s32( size ) );
			Memory::tResourceMemoryProvider::fInstance( ).fFree( mem ); 
		}
		static const Memory::tAllocStamp* fResourceGetStamp( void* mem )
		{
			return Memory::tResourceMemoryProvider::fInstance( ).fGetStamp( mem );
		}
	}

	tLoadInPlaceResourceBuffer::tLoadInPlaceResourceBuffer( )
		: mBuffer( 0 )
		, mSize( 0 )
	{
	}

	tLoadInPlaceResourceBuffer::~tLoadInPlaceResourceBuffer( )
	{
		sigassert( mBuffer == 0 && mSize == 0 );
	}

	void tLoadInPlaceResourceBuffer::fAlloc( u32 numBytes, const Memory::tAllocStamp& stamp )
	{
		sigassert( mBuffer == 0 && mSize == 0 );
		sigassert( numBytes > 0 );
		mSize = numBytes;

		mBuffer = fResourceAlloc( mSize, stamp );
		log_assert( mBuffer, "Resource heap buffer size (MB): " << numBytes / (1024.f*1024.f) );
	}

	void tLoadInPlaceResourceBuffer::fFree( )
	{
		if( mSize > 0 )
		{
			sigassert( mBuffer );
			fResourceFree( mBuffer, mSize );

			mBuffer = NULL;
			mSize = 0;
		}

		sigassert( !mBuffer );
	}

	void tLoadInPlaceResourceBuffer::fResize( u32 newBufferSize )
	{
		// retrieve former stamp if possible
		const Memory::tAllocStamp* oldStamp = mBuffer ? fResourceGetStamp( mBuffer ) : NULL;		
		Memory::tAllocStamp newStamp = oldStamp ? *oldStamp : Memory::tAllocStamp( NULL, ~0, "Relocated Resource" );
		newStamp.mSize = newBufferSize;

		// allocate new buffer
		Sig::byte* newBuf = fResourceAlloc( newBufferSize, newStamp );

		// copy old data if any
		fMemCpy( newBuf, mBuffer, fMin( newBufferSize, mSize ) );

		// free old buffer
		fFree( );

		// store new values
		mBuffer = newBuf;
		mSize = newBufferSize;
	}

	Sig::byte* tLoadInPlaceResourceBuffer::fGetBuffer( )
	{
		return mBuffer;
	}

	const Sig::byte* tLoadInPlaceResourceBuffer::fGetBuffer( ) const
	{
		return mBuffer;
	}
	
	u32 tLoadInPlaceResourceBuffer::fGetBufferSize( ) const
	{
		return mSize;
	}
}
