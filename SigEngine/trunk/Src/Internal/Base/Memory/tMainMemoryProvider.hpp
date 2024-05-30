#ifndef __tMainMemoryProvider__
#define __tMainMemoryProvider__
#include "tChunkyHeap.hpp"
#include "tChunkySubHeap.hpp"
#include "Threads/tMutex.hpp"

namespace Sig { namespace Memory
{
	typedef tChunkyHeap<tChunkySubHeap, Threads::tMutex> tMainMemoryProviderBase;

	class base_export tMainMemoryProvider : public tMainMemoryProviderBase
	{
		sig_make_stringstreamable( tMainMemoryProvider, 
			Memory::fToMB<f32>( fNumBytesAllocd( ) )
			<< "/" << Memory::fToMB<f32>( fNumBytesBeingManaged( ) ) 
			<< ", " << Memory::fToMB<f32>( fLargestFreeChunkSize( ) ) 
			<< "MB contig" );

	private:
		tMainMemoryProvider( );
		declare_singleton_define_own_ctor_dtor( tMainMemoryProvider );

	public:
		virtual void* fAlloc( u32 numBytes, const tAllocStamp& stamp );
		virtual void* fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp );
		virtual void fFree( void* memory );
		virtual u32 fSizeOfAlloc( void* memory );
	};

}}


#endif//__tMainMemoryProvider__
