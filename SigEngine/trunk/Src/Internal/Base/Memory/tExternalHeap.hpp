#ifndef __tExternalHeap__
#define __tExternalHeap__
#include "tChunkyHeap.hpp"
#include "tExternalSubHeap.hpp"
#include "Threads/tMutex.hpp"

namespace Sig { namespace Memory
{
	typedef tChunkyHeap<tExternalSubHeap, Threads::tDummyMutex> tExternalHeapBase;

	class base_export tExternalHeap : public tExternalHeapBase
	{
		sig_make_stringstreamable( tExternalHeap, 
			Memory::fToMB<f32>( fNumBytesAllocd( ) )
			<< "/" << Memory::fToMB<f32>( fNumBytesBeingManaged( ) ) 
			<< ", " << Memory::fToMB<f32>( fLargestFreeChunkSize( ) ) 
			<< "MB contig" );

	public:
		tExternalHeap( tExternalAllocFunc allocFunc, tExternalFreeFunc freeFunc, const Memory::tMemoryOptions::tHeapBehavior& behavior, const char* name );
		virtual void* fAlloc( u32 numBytes, const tAllocStamp& stamp );
		virtual void* fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp );
		virtual void fFree( void* memory );
		virtual u32 fSizeOfAlloc( void* memory );
	};

}}

#endif//__tExternalHeap__
