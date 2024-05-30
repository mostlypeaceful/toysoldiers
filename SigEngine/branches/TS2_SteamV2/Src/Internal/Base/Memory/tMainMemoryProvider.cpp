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
			return malloc( numBytes );
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
		return malloc( numBytes );
#endif
	}

	void* tMainMemoryProvider::fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp )
	{
#ifdef sig_use_memmgr
		return fReallocInternal( memory, numBytes, stamp );
#else
		return realloc( memory, numBytes );
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
