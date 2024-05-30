#include "BasePch.hpp"
#include "tMainMemoryProvider.hpp"
#include "tApplication.hpp"

#define sig_use_memmgr

namespace Sig { namespace Memory
{
	namespace
	{
		static void* fRawAlloc( u32 numBytes )
		{
			void* mem = malloc( numBytes );
			sigassert( mem && "OUT OF MEMORY" );
			return mem;
		}
		static void fRawFree( void* mem )
		{
			free( mem );
		}
	}

	tMainMemoryProvider::tMainMemoryProvider( )
		: tChunkyHeap( fRawAlloc, fRawFree, tApplication::fMemoryOptions( ).mMainProvider, "tMainMemoryProvider" )
	{ 
	}

	void* tMainMemoryProvider::fAlloc( u32 numBytes, const tAllocStamp& stamp )
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

	void* tMainMemoryProvider::fRealloc( void* oldMem, u32 numBytes, const tAllocStamp& stamp )
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

	void tMainMemoryProvider::fFree( void* memory )
	{
#ifdef sig_use_memmgr
		fFreeInternal( memory );
#else
		free( memory );
#endif
	}

	u32 tMainMemoryProvider::fSizeOfAlloc( void* memory )
	{
#ifdef sig_use_memmgr
		return tMainMemoryProviderBase::fSizeOfAlloc( memory );
#else
		return 0;
#endif
	}

}}
