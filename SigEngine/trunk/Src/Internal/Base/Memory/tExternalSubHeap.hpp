//------------------------------------------------------------------------------
// \file tExternalSubHeap.hpp - 29 Sep 2010
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tExternalSubHeap__
#define __tExternalSubHeap__
#include "tChunkySubHeap.hpp"
#include "tPool.hpp"

namespace Sig { namespace Memory
{
	class tExternalSubHeap : public tChunkySubHeap
	{
	public:

		// take on some user-specified memory and manage it
		void	fInit( void* managedAddr, u32 managedBytes, u32 alignment, u32 maxFreeChunks, u32 minUserMemSize );
		// stop managing the memory
		void	fShutdown( );
		// attempt to consolidate/defragment the cache of free chunks
		virtual void fConsolidate();

		byte* fAlloc( u32 bytes, u32 alignment, const tAllocStamp& stamp );

	private:

		virtual tChunkyHeapAllocHeader* fHeaderFromClientAddr( byte* memPtr ) const;
		virtual tChunkyHeapAllocHeader* fNewHeader( byte* startOfNewMemory, u32 numBytes, tChunkyHeapAllocHeader::tState state, const tAllocStamp& stamp );
		virtual void fFreeHeader( tChunkyHeapAllocHeader* header );
		virtual u32 fComputeOccupyingHeaderSize( );

		//tPool		mHeaderPool;
	};
}}

#endif//__tExternalHeap__
