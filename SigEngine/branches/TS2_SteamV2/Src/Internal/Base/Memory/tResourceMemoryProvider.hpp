#ifndef __tResourceMemoryProvider__
#define __tResourceMemoryProvider__
#include "tChunkyHeap.hpp"
#include "tChunkySubHeap.hpp"
#include "Threads/tMutex.hpp"

namespace Sig { namespace Memory
{
	typedef tChunkyHeap<tChunkySubHeap, Threads::tMutex> tResourceMemoryProviderBase;

	class base_export tResourceMemoryProvider : public tResourceMemoryProviderBase
	{
		tResourceMemoryProvider( );
		declare_singleton_define_own_ctor_dtor( tResourceMemoryProvider );

	public:
		virtual void* fAlloc( u32 numByte, const tAllocStamp& stamp );
		virtual void* fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp );
		virtual void fFree( void* memory );
		virtual u32 fSizeOfAlloc( void* memory );
		virtual void fConsolidate( );

		// This is the location where resource that will relocate are first located.
		tHeap& fTemporaryResourceHeap( ) const;
	};

}}


#endif//__tResourceMemoryProvider__
