#include "BasePch.hpp"
#include "tResourceMemoryProvider.hpp"
#include "tApplication.hpp"

#define sig_use_memmgr

namespace Sig { namespace Memory
{
	namespace
	{
		void* fRawAlloc( u32 size )
		{
			return malloc( size );
		}

		void fRawDealloc( void* mem )
		{
			free( mem );
		}
	}

	tResourceMemoryProvider::tResourceMemoryProvider( )
		: tChunkyHeap( fRawAlloc, fRawDealloc, tApplication::fMemoryOptions( ).mResourceProvider, "tResourceMemoryProvider" )
	{
	}

	void* tResourceMemoryProvider::fAlloc( u32 numBytes, const tAllocStamp& stamp )
	{
#ifdef sig_use_memmgr
		return fAllocInternal( numBytes, 0, stamp );
#else
		return malloc( numBytes );
#endif
	}

	void* tResourceMemoryProvider::fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp )
	{
#ifdef sig_use_memmgr
		return fReallocInternal( memory, numBytes, stamp );
#else
		return realloc( memory, numBytes );
#endif
	}

	void tResourceMemoryProvider::fFree( void* memory )
	{
#ifdef sig_use_memmgr
		fFreeInternal( memory );
#else
		free( memory );
#endif
	}

	u32 tResourceMemoryProvider::fSizeOfAlloc( void* memory )
	{
#ifdef sig_use_memmgr
		return tResourceMemoryProviderBase::fSizeOfAlloc( memory );
#else
		return 0;
#endif
	}

	tHeap& tResourceMemoryProvider::fTemporaryResourceHeap( ) const
	{
		return tMallocHeap::fInstance( );
	}
	
	void tResourceMemoryProvider::fConsolidate( )
	{
		tChunkyHeap::fConsolidate( );
		//log_warning( 0, "res chunks: " << tChunkyHeap::fNumFreeChunks( ) );
	}

}}
