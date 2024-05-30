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
			void* mem = malloc( size );
			sigassert( mem && "OUT OF MEMORY" );
			return mem;
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
		void* mem = malloc( numBytes );
		if( !mem )
			Log::fFatalError( );
		return mem;
#endif
	}

	void* tResourceMemoryProvider::fRealloc( void* oldMem, u32 numBytes, const tAllocStamp& stamp )
	{
#ifdef sig_use_memmgr
		return fReallocInternal( oldMem, numBytes, stamp );
#else
		void* mem = realloc( oldMem, numBytes );
		if( !mem )
			Log::fFatalError( );
		return mem;
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
		//log_warning( "res chunks: " << tChunkyHeap::fNumFreeChunks( ) );
	}

}}
