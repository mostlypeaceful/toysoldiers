#include "BasePch.hpp"
#include "tExternalHeap.hpp"

namespace Sig { namespace Memory
{
	tExternalHeap::tExternalHeap( tExternalAllocFunc allocFunc, tExternalFreeFunc freeFunc, const Memory::tMemoryOptions::tHeapBehavior& behavior, const char* name )
		: tChunkyHeap( allocFunc, freeFunc, behavior, name )
	{
	}

	void* tExternalHeap::fAlloc( u32 numBytes, const tAllocStamp& stamp )
	{
		return fAllocInternal( numBytes, 0, stamp );
	}

	void* tExternalHeap::fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp )
	{
		return fReallocInternal( memory, numBytes, stamp );
	}

	void tExternalHeap::fFree( void* memory )
	{
		fFreeInternal( memory );
	}

	u32 tExternalHeap::fSizeOfAlloc( void* memory )
	{
		return tExternalHeapBase::fSizeOfAlloc( memory );
	}

}}
