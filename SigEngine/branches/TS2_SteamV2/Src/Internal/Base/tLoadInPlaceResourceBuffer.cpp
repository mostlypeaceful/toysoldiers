#include "BasePch.hpp"
#include "tLoadInPlaceResourceBuffer.hpp"
#include "Memory/tResourceMemoryProvider.hpp"
#include "tProfiler.hpp"

namespace Sig
{
	namespace
	{
		devvar( f32, Debug_ResourceDepot_MaxPendingLoadsMB, 12.f * 1024.f * 1024.f );

		static const Memory::tAllocStamp cDefaultResourceStamp( "Resource" );

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

		// This allocation sceme is used for resources will will resize.
		//  In an effort not to fragment the heap, they are first loaded out of place.
		u32 gTempMemoryAllocated = 0; // Assumes that resource buffer allocations only happen single threaded.

		static Sig::byte* fResourceAllocTempLocation( u32 numBytes, const Memory::tAllocStamp& stamp ) 
		{ 
			profile_mem( cProfileMemResTemp, numBytes );
			gTempMemoryAllocated += numBytes;
			return (Sig::byte*)Memory::tResourceMemoryProvider::fInstance( ).fTemporaryResourceHeap( ).fAlloc( numBytes, cDefaultResourceStamp );
		}

		static void fResourceFreeTempLocation( void* mem, u32 size ) 
		{ 
			profile_mem( cProfileMemResTemp, -(s32)size );
			gTempMemoryAllocated -= size;
			Memory::tResourceMemoryProvider::fInstance( ).fTemporaryResourceHeap( ).fFree( mem );
		}

		static b32 fResourceTempLocationHasSpace( u32 size )
		{
			sigassert( size < Debug_ResourceDepot_MaxPendingLoadsMB && "Single resource is over the max temp size!" );
			return ( gTempMemoryAllocated + size <= Debug_ResourceDepot_MaxPendingLoadsMB );
		}
	}

	tLoadInPlaceResourceBuffer::tLoadInPlaceResourceBuffer( )
		: mBuffer( 0 )
		, mSize( 0 )
		, mWillResize( false )
	{
	}

	tLoadInPlaceResourceBuffer::~tLoadInPlaceResourceBuffer( )
	{
		sigassert( mBuffer == 0 && mSize == 0 );
	}

	b32 tLoadInPlaceResourceBuffer::fCanAlloc( u32 numBytes, b32 willResize )
	{
		if( !willResize )
			return true;
		else
			return fResourceTempLocationHasSpace( numBytes );

	}

	void tLoadInPlaceResourceBuffer::fAlloc( u32 numBytes, b32 willResize, const Memory::tAllocStamp& stamp )
	{
		sigassert( mBuffer == 0 && mSize == 0 );
		sigassert( numBytes > 0 );
		mSize = numBytes;
		mWillResize = willResize;

		Memory::tAllocStamp stamp2 = stamp;
		stamp2.mSize = numBytes;

		if( !mWillResize )
		{
			mBuffer = fResourceAlloc( mSize, stamp2 );
			log_assert( mBuffer, "Res heap buffer size (MB): " << numBytes / (1024.f*1024.f) );
		}
		else
			mBuffer = fResourceAllocTempLocation( mSize, stamp2 );

		log_assert( mBuffer, "Temporary buffer size (MB): " << numBytes / (1024.f*1024.f) );
	}

	void tLoadInPlaceResourceBuffer::fFree( )
	{
		if( mSize > 0 )
		{
			sigassert( mBuffer );

			if( !mWillResize )
				fResourceFree( mBuffer, mSize );
			else
			{
				fResourceFreeTempLocation( mBuffer, mSize );
				mWillResize = false;
			}

			mBuffer = 0;
			mSize = 0;
		}
		sigassert( !mBuffer );
	}

	void tLoadInPlaceResourceBuffer::fResize( u32 newBufferSize )
	{
		sigassert( mWillResize && "Inapporpriate: tLoadInPlaceResourceBuffer::fResize" );

		// allocate new buffer
		Sig::byte* newBuf = fResourceAlloc( newBufferSize, cDefaultResourceStamp );

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
