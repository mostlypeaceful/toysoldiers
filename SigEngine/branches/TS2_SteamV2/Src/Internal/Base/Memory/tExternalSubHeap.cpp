//------------------------------------------------------------------------------
// \file tExternalSubHeap.cpp - 29 Sep 2010
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tExternalSubHeap.hpp"

namespace Sig { namespace Memory
{

	void tExternalSubHeap::fInit( void* managedAddr, u32 managedBytes, u32 alignment, u32 maxFreeChunks, u32 minUserMemSize )
	{
		//mHeaderPool.fCreate( fAlignHigh<u32>( sizeof(tChunkyHeapAllocHeader), 16 ), 512, 1, "ExternalHeapHeaderPool" );

		tChunkySubHeap::fInit( 
			managedAddr, 
			managedBytes, 
			alignment, 
			maxFreeChunks, 
			minUserMemSize );
	}

	void tExternalSubHeap::fShutdown( )
	{
		tChunkySubHeap::fShutdown( );

		//mHeaderPool.fClearPageMemory( );
	}

	byte* tExternalSubHeap::fAlloc( u32 bytes, u32 alignment, const tAllocStamp& stamp )
	{
		alignment = fMax( alignment, mAlignment );

		if( alignment > mAlignment )
			bytes += alignment;

		tChunkyHeapAllocHeader* header = tChunkySubHeap::fAllocInternal( bytes, stamp );

		if( !header )
			return 0;

		if( alignment > mAlignment )
		{
			byte* initialMemPtr = header->mClientAddr;
			byte* alignedMemPtr = ( byte* )( u32 )fAlignHigh<u32>( ( u32 )header->mClientAddr, alignment );

			const u32 differential = ( u32 )( alignedMemPtr - initialMemPtr );

			if( differential == 0 )
				return header->mClientAddr;

			sigassert( differential >= mAlignment );

			tChunkyHeapAllocHeader* newHeader = fSplit( header, differential, true );
			newHeader->mStamp = stamp;

			sigassert( newHeader );
			sigassert( newHeader->mClientAddr == alignedMemPtr );
			sigassert( header->mClientByteCount == differential );

			fFreeInternal( header );

			return newHeader->mClientAddr;
		}

		return header->mClientAddr;
	}

	tChunkyHeapAllocHeader* tExternalSubHeap::fHeaderFromClientAddr( byte* memPtr ) const
	{
		// search each in-use chunk
		for( const tChunkyHeapAllocHeader* i = mInUseHead; i; i = i->mNext )
			if( i->mClientAddr == memPtr )
				return ( tChunkyHeapAllocHeader* )i;

		// search each free chunk
		for( const tChunkyHeapAllocHeader* i = mFreeChunkHead; i; i = i->mNext )
			if( i->mClientAddr == memPtr )
				return ( tChunkyHeapAllocHeader* )i;

		return 0;
	}

	tChunkyHeapAllocHeader* tExternalSubHeap::fNewHeader( byte* startOfNewMemory, u32 numBytes, tChunkyHeapAllocHeader::tState state, const tAllocStamp& stamp )
	{
		tChunkyHeapAllocHeader* o = NEW tChunkyHeapAllocHeader( startOfNewMemory + mOccupyingHeaderSize, numBytes - mOccupyingHeaderSize, state );//( tChunkyHeapAllocHeader* )mHeaderPool.fAlloc( );
		//*o = tChunkyHeapAllocHeader( startOfNewMemory + mOccupyingHeaderSize, numBytes - mOccupyingHeaderSize, state );
		o->mStamp = stamp;
		return o;
	}

	void tExternalSubHeap::fFreeHeader( tChunkyHeapAllocHeader* header )
	{
		//mHeaderPool.fFree( header );
		delete header;
	}

	u32 tExternalSubHeap::fComputeOccupyingHeaderSize( )
	{
		return 0;
	}

	void tExternalSubHeap::fConsolidate( )
	{
		tChunkySubHeap::fConsolidate( );
		//mHeaderPool.fClean( );
	}

}}

