//------------------------------------------------------------------------------
// \file tChunkySubHeap.cpp - 28 Sep 2010
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tChunkySubHeap.hpp"

namespace Sig { namespace Memory
{
	static_assert( sizeof( tChunkyHeapAllocHeader ) == 24 );

	namespace
	{
		static const tAllocStamp cHeaderStampFree( "Free Header" );
		static const tAllocStamp cHeaderStampAlloced( "Alloced Header" );
	}

	tChunkySubHeap::tChunkySubHeap( )
		: mManagedAddr( 0 )
		, mManagedBytes( 0 )
		, mFreeBytes( 0 )
		, mFreeChunkHead( 0 )
		, mInUseHead( 0 )
		, mNumFreeChunks( 0 )
		, mNumUsedChunks( 0 )
		, mMinUserMemSize( 0 )
		, mAlignment( 0 )
		, mOccupyingHeaderSize( 0 )
		, mMaxFreeChunks( 0 )
	{
	}

	void tChunkySubHeap::fInit( void* managedAddr, u32 managedBytes, u32 alignment, u32 maxFreeChunks, u32 minUserMemSize )
	{
		sigassert( managedAddr != 0 && managedBytes > 0 );
		sigassert( mManagedAddr == 0 && mManagedBytes == 0 );
		mManagedAddr = managedAddr;
		mManagedBytes = managedBytes;
		mFreeBytes = mManagedBytes;

		mAlignment = alignment;
		mMaxFreeChunks = maxFreeChunks;
		mMinUserMemSize = minUserMemSize;
		mOccupyingHeaderSize = fComputeOccupyingHeaderSize( );

		// start with the whole pool in one chunk:

		// create the chunk
		byte* alignedStart = ( byte* )fAlignHigh( ( u32 )mManagedAddr, mAlignment );
		const size_t wastedBytes = ( size_t )( alignedStart - ( byte* )mManagedAddr );
		tChunkyHeapAllocHeader* header = fNewHeader( alignedStart, mManagedBytes - wastedBytes, tChunkyHeapAllocHeader::cStateGarbage, cHeaderStampFree );

		// fill the memory with debug tokens
		if( mOccupyingHeaderSize > 0 )
			fMarkMemAsGarbage( header->mClientAddr, header->mClientByteCount );

		// add to to the free list
		fAddToList( header, mFreeChunkHead, mNumFreeChunks );
	}

	void tChunkySubHeap::fShutdown( )
	{
		mManagedAddr=0;
		mManagedBytes=0;
		mFreeBytes=0;
		mFreeChunkHead=0;
		mInUseHead=0;
		mNumFreeChunks=0;
		mNumUsedChunks=0;
		mAlignment=0;
		mOccupyingHeaderSize=0;
		mMaxFreeChunks=0;
		mMinUserMemSize = 0;
	}

	void* tChunkySubHeap::fAlloc( u32 bytes, u32 alignmentIgnored, const tAllocStamp& stamp )
	{
		tChunkyHeapAllocHeader* header = fAllocInternal( bytes, stamp );
		return header ? header->mClientAddr : 0;
	}

	u32 tChunkySubHeap::fFree( void* address )
	{
		if( fContains( address ) )
		{
			// backup to the start of the actual chunk header
			tChunkyHeapAllocHeader* header = fHeaderFromClientAddr( ( byte* )address );

			const u32 numBytes = header->mClientByteCount;

			fFreeInternal( header );

			return numBytes;
		}

		return 0;
	}

	tChunkyHeapAllocHeader* tChunkySubHeap::fHeaderFromClientAddr( byte* memPtr ) const
	{
		return reinterpret_cast<tChunkyHeapAllocHeader*>( memPtr - mOccupyingHeaderSize );
	}

	tChunkyHeapAllocHeader* tChunkySubHeap::fNewHeader( byte* startOfNewMemory, u32 numBytes, tChunkyHeapAllocHeader::tState state, const tAllocStamp& stamp )
	{
		sigassert( fIsAligned( ( u32 )startOfNewMemory, mAlignment ) );
		tChunkyHeapAllocHeader* header = reinterpret_cast<tChunkyHeapAllocHeader*>( startOfNewMemory );
		new ( header )  tChunkyHeapAllocHeader( startOfNewMemory + mOccupyingHeaderSize, numBytes - mOccupyingHeaderSize, state );
		header->mStamp.fReset( stamp );
		return header;
	}

	void tChunkySubHeap::fFreeHeader( tChunkyHeapAllocHeader* header )
	{
		if( mOccupyingHeaderSize > 0 )
			fMarkMemAsFreed( header, mOccupyingHeaderSize );
	}

	u32 tChunkySubHeap::fComputeOccupyingHeaderSize( )
	{
		return ( u32 )fAlignHigh( ( u32 )sizeof(tChunkyHeapAllocHeader), mAlignment );
	}

	tChunkyHeapAllocHeader* tChunkySubHeap::fAllocInternal( u32 bytes, const tAllocStamp& stamp )
	{
		// clamp request to minimum size, and align
		bytes = fAlignHigh( fMax( mMinUserMemSize, bytes ), mAlignment );

		// we try at most twice
		for( u32 attempt=0; attempt<2; ++attempt )
		{
			// first look thru existing chunk cache
			tChunkyHeapAllocHeader* bestFit = fFindBestFit( bytes );

			if( !bestFit )
			{
				// we couldn't find a chunk big enough to satisfy the request
				if( attempt==0 )
				{
					// since it's the first time thru, consolidate the existing 
					// chunks in the cache, and try again...
					fConsolidate( );
				}

				continue;
			}

			// we got a chunk that is big enough!...
			fValidateChunk( bestFit );

			tChunkyHeapAllocHeader* lowPartOfBestFit = bestFit;

			// see if there's extra memory to shave off this chunk
			if( bestFit->mClientByteCount > bytes )
			{
				bestFit = fSplit( bestFit, bytes, false );
			}

			if( lowPartOfBestFit == bestFit )
			{
				// remove this chunk from the free list...
				fRemoveFromList( bestFit, mFreeChunkHead, mNumFreeChunks );
				// ...and put it onto the in-use list
				fAddToList( bestFit, mInUseHead, mNumUsedChunks );

				// fill the memory with debug tokens
				if( mOccupyingHeaderSize > 0 )
					fMarkMemAsAllocated( bestFit->mClientAddr, bestFit->mClientByteCount );

				// update the chunk's state
				bestFit->mState = tChunkyHeapAllocHeader::cStateAllocd;
				
				bestFit->mStamp.fReset( stamp );

				// update memory size of free list
				mFreeBytes -= bestFit->mClientByteCount + mOccupyingHeaderSize;
			}

			// return the chunk (advanced to past the header)
			assert_mem_valid( ( fIsAligned<u32>( (u32)bestFit->mClientAddr, mAlignment ) ) && "Memory is not correctly aligned!" );
			return bestFit;
		}

		// hard luck:
		// we went thru twice looking, and after the first time we consolidated the cache;
		// unfortunately, there just isn't enough contiguous memory available
		return 0;
	}

	void tChunkySubHeap::fFreeInternal( tChunkyHeapAllocHeader* header )
	{
		// validate the chunk
		fValidateChunk( header );

		// fill the memory with debug tokens
		if( mOccupyingHeaderSize > 0 )
			fMarkMemAsFreed( header->mClientAddr, header->mClientByteCount );

		// update the chunk's state
		header->mState = tChunkyHeapAllocHeader::cStateFreed;

		// dont clear the name, so we can keep track of where this free block came from
		//header->mStamp.fRelease( );

		// remove the chunk from the checked out list...
		fRemoveFromList( header, mInUseHead, mNumUsedChunks );

		// ...and put it on back on the free list
		fAddToList( header, mFreeChunkHead, mNumFreeChunks );
		// update memory size of free list
		mFreeBytes += header->mClientByteCount + mOccupyingHeaderSize;

		// see if we've accumulated a lot of free chunks;
		if( mNumFreeChunks % mMaxFreeChunks == 0 )
		{
			// if so, try consolidating them
			fConsolidate();
		}
	}


	tAlignedAllocation tChunkySubHeap::fAllocAligned( u32 bytes, u32 alignment, const tAllocStamp& stamp )
	{
		tAlignedAllocation o;
		if( alignment <= mAlignment )
		{
			o.mOriginalAddr = o.mAlignedAddr = fAlloc( bytes, 0, stamp );
		}
		else
		{
			o.mOriginalAddr = fAlloc( bytes + alignment - 1, 0, stamp );
			o.mAlignedAddr = ( void* )fAlignHigh<u32>( ( u32 )o.mOriginalAddr, alignment );
		}

		return o;
	}

	void tChunkySubHeap::fFreeAligned( const tAlignedAllocation& alignedAlloc )
	{
		fFree( ( byte* )alignedAlloc.mOriginalAddr );
	}

	void tChunkySubHeap::fSplit( byte* mem, u32 neededBytes )
	{
		// back up to the actual chunk header
		tChunkyHeapAllocHeader* header = fHeaderFromClientAddr( mem );

		fSplit( header, neededBytes, false );
	}

	tChunkyHeapAllocHeader* tChunkySubHeap::fSplit( tChunkyHeapAllocHeader* header, u32 neededBytes, b32 returnNewHeader )
	{
		// clamp request to minimum size, and align
		neededBytes = fAlignHigh( fMax( mMinUserMemSize, neededBytes ), mAlignment );

		// validate
		fValidateChunk( header );

		// make sure there's enough room left over after splitting to make it worth it
		if( (header->mClientByteCount <= neededBytes + mOccupyingHeaderSize) || (header->mClientByteCount - neededBytes - mOccupyingHeaderSize < mMinUserMemSize) )
			return header;

		// split off a new chunk
		tChunkyHeapAllocHeader* newHeader = fNewHeader( 
			header->mClientAddr + neededBytes, 
			header->mClientByteCount - neededBytes, 
			( tChunkyHeapAllocHeader::tState )header->mState,
			cHeaderStampAlloced );

		// update the old header to reflect how much memory it now contains
		header->mClientByteCount = neededBytes;

		if( returnNewHeader )
		{
			if( newHeader->mState == tChunkyHeapAllocHeader::cStateFreed || newHeader->mState == tChunkyHeapAllocHeader::cStateGarbage )
				mFreeBytes -= newHeader->mClientByteCount + mOccupyingHeaderSize;

			newHeader->mState = tChunkyHeapAllocHeader::cStateAllocd;

			fAddToList( newHeader, mInUseHead, mNumUsedChunks );

			return newHeader;
		}
		// check if this memory was previously allocated;
		// if it was, mark it now as having been freed (state transition)
		else if( newHeader->mState == tChunkyHeapAllocHeader::cStateAllocd )
		{
			newHeader->mState = tChunkyHeapAllocHeader::cStateFreed;

			// update memory size of free list
			mFreeBytes += newHeader->mClientByteCount + mOccupyingHeaderSize;

			// fill the memory with debug tokens
			if( mOccupyingHeaderSize > 0 )
				fMarkMemAsFreed( newHeader->mClientAddr, newHeader->mClientByteCount );
		}

		// add the new chunk to the free list if it wasn't requested to be returned
		fAddToList( newHeader, mFreeChunkHead, mNumFreeChunks );
		return header;
	}

	b32 tChunkySubHeap::fContains( void* address ) const
	{
		return(	( address >= ( byte* )mManagedAddr )
			&&	( address <  ( byte* )mManagedAddr + mManagedBytes ) );
	}

	u32 tChunkySubHeap::fGetAllocSize( void* mem ) const
	{
		tChunkyHeapAllocHeader* header = fHeaderFromClientAddr( ( byte* )mem );
		fValidateChunk( header );
		return header->mClientByteCount;
	}

	const tAllocStamp* tChunkySubHeap::fGetStamp( void* address ) const
	{
		tChunkyHeapAllocHeader* header = fHeaderFromClientAddr( ( byte* )address );
		fValidateChunk( header );
		return header->mStamp.fGetRawPtr( );
	}

	b32 tChunkySubHeap::fValidate( ) const
	{
#ifndef sig_memory_validate
		return true;
#else//sig_memory_validate
		b32 valid = true;

		// re-compute these values, and make sure it all adds up
		u32 memFree=0;
		u32 memInUse=0;

		// go thru and validate each free chunk
		for( tChunkyHeapAllocHeader* i = mFreeChunkHead; i; i = i->mNext )
		{
			valid = valid && fValidateChunk( i );
			memFree += i->mClientByteCount + mOccupyingHeaderSize;
		}

		// go thru and validate each in-use chunk
		for( tChunkyHeapAllocHeader* i = mInUseHead; i; i = i->mNext )
		{
			valid = valid && fValidateChunk( i );
			memInUse += i->mClientByteCount + mOccupyingHeaderSize;
		}

		const char* onlyReportThisTextOnce = "(WARNING) tChunkySubHeap::Validate(): Raw memory heap validation has failed!\n";
		if( !valid )
			Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce);

		// make sure our just-computed value of free memory matches our expectations
		if( memFree != mFreeBytes )
		{
			if( valid )
				Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce);
			Log::fPrintf( Log::cFlagMemory, "\tconflict between tracked free memory size and computed free memory size!\n");
			valid = false;
		}

		// make sure our just-computed sum of free and in-use memory matches the total amount in the heap
		if( memFree + memInUse != mManagedBytes )
		{
			if( valid )
				Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce);
			Log::fPrintf( Log::cFlagMemory, "\tcomputed free size + computed in-use size does not equal the entire heap size!\n");
			valid = false;
		}

		assert_mem_valid( valid );
		return valid;
#endif//sig_memory_validate
	}

	void tChunkySubHeap::fDump( ) const
	{
#ifdef sig_memory_dump
		Log::fPrintf( Log::cFlagMemory,  "-----------------------------------------------------\n");
		Log::fPrintf( Log::cFlagMemory,  "(NOTIFY) tChunkySubHeap::Dump(): Start dump of heap memory\n" );

		// To string access is unmaintained here
		//for( tChunkyHeapAllocHeader* i=mFreeChunkHead; i; i=i->mNext )
		//	Log::fPrintf( Log::cFlagMemory,  "Free at 0x%p '%s', %d bytes.\n", i->mClientAddr, i->mStamp->fToString( ).c_str( ), i->mClientByteCount );

		//for( tChunkyHeapAllocHeader* i=mInUseHead; i; i=i->mNext )
		//	Log::fPrintf( Log::cFlagMemory,  "Used at 0x%p '%s', %d bytes.\n", i->mClientAddr, i->mStamp->fToString( ).c_str( ), i->mClientByteCount );

		Log::fPrintf( Log::cFlagMemory,  "Total=%d, Used=%d, Free=%d, Biggest Free Block=%d\n", fNumBytesBeingManaged(), fNumBytesAllocd(), fNumBytesFree(), fLargestFreeChunkSize() );
		Log::fPrintf( Log::cFlagMemory,  "(NOTIFY) tChunkySubHeap::Dump(): End dump of heap memory\n" );
		Log::fPrintf( Log::cFlagMemory,  "----------------------------------------------------\n");
#endif//sig_memory_dump
	}

	void tChunkySubHeap::fDumpNamedAllocations( tMemoryDumpOptions& memoryDumpOptions, tPageDump* page ) const
	{
#ifdef sig_memory_dump
		if( page )
		{
			page->mAddress = (u32)mManagedAddr;
			page->mAllocations.fSetCapacity( 300 );
			for( tChunkyHeapAllocHeader* i = mInUseHead; i; i = i->mNext )
			{
				if( i->mStamp.fGetRawPtr( ) )
				{
					page->mAllocations.fPushBack( tAllocDump( tStampDump( *i->mStamp, (u32)i->mClientAddr, *memoryDumpOptions.mDump ) ) );
				}
			}
			page->mFreed.fSetCapacity( 300 );
			for( tChunkyHeapAllocHeader* i = mFreeChunkHead; i; i = i->mNext )
			{
				if( i->mStamp.fGetRawPtr( ) )
				{
					page->mFreed.fPushBack( tAllocDump( tStampDump( *i->mStamp, (u32)i->mClientAddr, *memoryDumpOptions.mDump ) ) );
				}
			}
		}

#endif//sig_memory_dump
	}

	void tChunkySubHeap::fDumpSubset( const char* nameToLookFor ) const
	{
#ifdef sig_memory_dump
		// TODO, UNMAINTAINED

		//u32 totalMemory=0, matchingMemory=0;// memory that matched our substr
		//u32 allocNodes=0, allocMemory=0; // memory & nodes htat were allocated
		//u32 totalNodes=0, matchingNodes=0;// total memory & nodes, including unallocated

		//for( tChunkyHeapAllocHeader* i=mFreeChunkHead; i; i=i->mNext )
		//{
		//	totalMemory += i->mClientByteCount;
		//	++totalNodes;
		//}

		//for( tChunkyHeapAllocHeader* i=mInUseHead; i; i=i->mNext )
		//{
		//	if ( i->mName && StringUtil::fStrStrI( i->mName, nameToLookFor ) )
		//	{
		//		// node's name has 'substr' in it.
		//		++matchingNodes;
		//		matchingMemory += i->mClientByteCount;
		//	}
		//	allocMemory += i->mClientByteCount;
		//	allocNodes++;

		//	totalMemory += i->mClientByteCount;
		//	++totalNodes;
		//}

		//if( matchingNodes==0 )
		//{
		//	// this allocation probably came from a MemPool!!!
		//	return;
		//}

		//Log::fPrintf( Log::cFlagMemory, "=================================================================\n");
		//Log::fPrintf( Log::cFlagMemory, "memDumpHeapSubset() Report for substring '%s':\n",nameToLookFor);
		//Log::fPrintf( Log::cFlagMemory, "=================================================================\n");
		//Log::fPrintf( Log::cFlagMemory, "%d of %d nodes (%.2f percent) had a label containing the string '%s'.\n", 
		//					matchingNodes, totalNodes, 100.0f*((float)matchingNodes)/(float)totalNodes, nameToLookFor);
		//Log::fPrintf( Log::cFlagMemory, "These nodes used a combined %d bytes (%.2f KB) of memory, for an average of %.2f bytes per node.\n", 
		//					matchingMemory, ((float)matchingMemory)/1024.0f, ((float)matchingMemory) / (float) matchingNodes );
		//Log::fPrintf( Log::cFlagMemory, "Overall, your nodes used %.3f percent of the total committed heap size of %d bytes (%.2f KBs).\n",
		//					100.0f*((float)matchingMemory)/(float)allocMemory, allocMemory, ((float)allocMemory)/1024.0f );
		//Log::fPrintf( Log::cFlagMemory, "Finally, you used %d nodes out of a total %d committed nodes, or %.2f percent of the total.\n\n",
		//					matchingNodes, allocNodes, 100.0f*((float)matchingNodes)/(float)allocNodes );
		//Log::fPrintf( Log::cFlagMemory, "Global Heap Info: (%d total bytes, %d committed (%.2f percent).\n",
		//					totalMemory, allocMemory, 100.0f*((float)allocMemory)/(float)totalMemory );
		//Log::fPrintf( Log::cFlagMemory, "                  (%d total nodes, %d committed (%.2f percent).\n",
		//					totalNodes, allocNodes, 100.0f*((float)allocNodes)/(float)totalNodes );
		//Log::fPrintf( Log::cFlagMemory, "=================================================================\n\n");
#endif//sig_memory_dump

	}

	u32 tChunkySubHeap::fLargestFreeChunkSize( ) const
	{
		u32 best=0;
		for( const tChunkyHeapAllocHeader* i = mFreeChunkHead; i; i = i->mNext )
			best = i->mClientByteCount > best ? i->mClientByteCount : best;
		return best;
	}

	tChunkyHeapAllocHeader* tChunkySubHeap::fFindBestFit( u32 bytes ) const
	{
		tChunkyHeapAllocHeader* bestFit=0;

		// look if we have an existing chunk that will work
		for( tChunkyHeapAllocHeader* i = mFreeChunkHead; i; i = i->mNext )
		{
			// NEW WAY
			if( i->mClientByteCount >= bytes )
			{
				if( !bestFit || ( i->mClientAddr < bestFit->mClientAddr ) )
					bestFit = i;
			}

			//// OLD WAY
			//if( i->mClientByteCount > bytes )
			//{
			//	// this case is pretty good: we have an available chunk, tho it's bigger than what we need;
			//	// we will keep track of this chunk (provided it's better than what we already have),
			//	// and keep looking for a perfect fit
			//	if( !bestFit || bestFit->mClientByteCount > i->mClientByteCount )
			//		bestFit = i;
			//}
			//else if( i->mClientByteCount == bytes)
			//{
			//	// this is the IDEAL case: looking for a chunk, and we have one that's a perfect fit;
			//	// we will essentially just return this chunk, after managing our free chunk array;
			//	bestFit = i;
			//	break;
			//}
		}

		return bestFit;
	}

	void tChunkySubHeap::fSort( tChunkyHeapAllocHeader*& listHead )
	{
		// we assume the list is non-null
		sigassert( listHead );

		// we use a dummy head/sentinel object to improve the performance and simplify the sort;
		// obviously, no memory address can be <= 0 (NULL), so this is a simple sentinel value to use:
		u32 dummyListSize = 0;
		tChunkyHeapAllocHeader dummyHead = tChunkyHeapAllocHeader(0, 0, tChunkyHeapAllocHeader::cStateGarbage);
		// insert our dummy head at the start of the list
		fAddToList( &dummyHead, listHead, dummyListSize );
		sigassert( listHead==&dummyHead );

		// because we assumed the list was non-null to start with, and now that we've added a dummy head,
		// we must have at least 2 elements in our list; hence it's safe to start the sort at listHead->mNext->mNext:
		for( tChunkyHeapAllocHeader* i = listHead->mNext->mNext, *inext=0; i; i = inext )
		{
			inext = i->mNext;
			tChunkyHeapAllocHeader* j = i;

			for( ; i->mClientAddr < j->mPrev->mClientAddr; j = j->mPrev )
			{
				// do nothing, we're just walking down the list until we find a slot for chunk i;
				// i.e., go until we find the first chunk to the left of i that's smaller than i
			}

			// only move chunk i down if we actually found a lower slot (could have already been sorted)
			if( i != j )
			{
				fRemoveFromList( i, listHead, dummyListSize );
				fInsertIntoList( i, j->mPrev, listHead, dummyListSize );
			}
		}

		// advance the list to one past the start (because of dummy/sentinel object)
		listHead = listHead->mNext;
		listHead->mPrev = 0;
	}

	void tChunkySubHeap::fConsolidate()
	{
		if( !mFreeChunkHead )
		{
			//log_warning( "tChunkySubHeap::Consolidate() being called with no free chunks!" );
			return;
		}

		fSort( mFreeChunkHead );

		// now consolidate:
		// this entails, for each chunk, looking at all the chunks whose memory addresses are
		// higher; for each of these, we see if there are no outstanding chunks in between them;
		// provided this is the case, the higher chunk can be deleted, while the given
		// chunk's number-of-bytes can just be incremented...
		for( tChunkyHeapAllocHeader* i = mFreeChunkHead; i; i = i->mNext )
		{
			for( tChunkyHeapAllocHeader* j = i->mNext; j; j = j->mNext )
			{
				if( j->mClientAddr - i->mClientAddr == ( int )( i->mClientByteCount + mOccupyingHeaderSize ) )
				{
					// get rid of chunk j, it's contiguous with chunk i,
					// and consolidate its memory with chunk i:
					tChunkyHeapAllocHeader* prev = j->mPrev;
					i->mClientByteCount += j->mClientByteCount + mOccupyingHeaderSize;
					fRemoveFromList( j, mFreeChunkHead, mNumFreeChunks );
					fFreeHeader( j );
					j = prev;
				}
				//else
				//{
				//	// because the list is sorted, if this chunk (j) couldn't be consolidated
				//	// with the lower chunk (i), then there's no point in going up further;
				//	// clearly, the higher chunks in memory won't be able to consolidate with i either...
				//	// this is what gives us that this step is O(N), and not O(N^2).
				//	break;
				//}
			}
		}
	}

	void tChunkySubHeap::fAddToList( tChunkyHeapAllocHeader* header, tChunkyHeapAllocHeader*& listHead, u32& count )
	{
		sigassert( !header->mNext && !header->mPrev );

		tChunkyHeapAllocHeader* oldHead = listHead;
		header->mNext = oldHead;
		if( header->mNext )
			header->mNext->mPrev = header;
		header->mPrev = 0;
		listHead = header;
		++count;
	}

	void tChunkySubHeap::fInsertIntoList( tChunkyHeapAllocHeader* header, tChunkyHeapAllocHeader* insertAfter, tChunkyHeapAllocHeader*& listHead, u32& count )
	{
		sigassert( !header->mNext && !header->mPrev );

		if( insertAfter )
		{
			header->mNext = insertAfter->mNext;
			if( header->mNext )
				header->mNext->mPrev = header;
			header->mPrev = insertAfter;
			insertAfter->mNext = header;
		}
		else // just add it at the head
		{
			fAddToList( header, listHead, count );
		}
	}

	void tChunkySubHeap::fRemoveFromList( tChunkyHeapAllocHeader* header, tChunkyHeapAllocHeader*& listHead, u32& count )
	{
		--count;
		if( header==listHead )
			listHead = header->mNext;
		if( header->mPrev )
			header->mPrev->mNext = header->mNext;
		if( header->mNext )
			header->mNext->mPrev = header->mPrev;
		header->mPrev = 0;
		header->mNext = 0;
	}
	
	b32 tChunkySubHeap::fValidateChunk( tChunkyHeapAllocHeader* header ) const
	{
#ifndef sig_memory_validate
		return true;
#else//sig_memory_validate
		const char* onlyReportThisTextOnce = "(WARNING) tChunkySubHeap::fValidateChunk(): The header at address 0x%p (%s) has failed validation!\n";
		b32 valid = true;
		if( !header || !header->mClientAddr )
		{
			if( valid )
				Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce, header, "unknown");
			Log::fPrintf( Log::cFlagMemory, "\tthe header address is null!\n");
			valid = false;
		}
		else
		{
			if( header->mClientAddr < mManagedAddr || header->mClientAddr >= ( byte* )mManagedAddr + mManagedBytes )
			{
				if( valid )
					Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce, header, header->mStamp->mCppFile );
				Log::fPrintf( Log::cFlagMemory, "\tthe header memory is outside of the valid address space!\n");
				valid = false;
			}
			if( header->mClientByteCount < mMinUserMemSize )
			{
				if( valid )
					Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce, header, header->mStamp->mCppFile);
				Log::fPrintf( Log::cFlagMemory, "\tthe header's user memory size is smaller than the minimum!\n");
				valid = false;
			}
			if( mOccupyingHeaderSize > 0 && !fIsAligned( reinterpret_cast<u32>( header ), mAlignment ) )
			{
				if( valid )
					Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce, header, header->mStamp->mCppFile);
				Log::fPrintf( Log::cFlagMemory, "\tthe header is not aligned!\n");
				valid = false;
			}
			if( header->mClientByteCount > mManagedBytes )
			{
				if( valid )
					Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce, header, header->mStamp->mCppFile);
				Log::fPrintf( Log::cFlagMemory, "\tthe header's size is out of range (claims to be larger than is possible)!\n");
				valid = false;
			}
			if( (header->mNext && header->mNext->mPrev != header) || (header->mPrev && header->mPrev->mNext != header) )
			{
				if( valid )
					Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce, header, header->mStamp->mCppFile);
				Log::fPrintf( Log::cFlagMemory, "\tthe linked list of chunks has been corrupted (likely a buffer overrun)!\n");
				valid = false;
			}
			if( header->mTrampleCheck != tChunkyHeapAllocHeader::cDontTrampleValue )
			{
				if( valid )
					Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce, header, header->mStamp->mCppFile);
				Log::fPrintf( Log::cFlagMemory, "\tthe header's magic number has been over-written!\n");
				valid = false;
			}
			if( (int)header->mState < 0 || (int)header->mState >= tChunkyHeapAllocHeader::cStateCount )
			{
				if( valid )
					Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce, header, header->mStamp->mCppFile);
				Log::fPrintf( Log::cFlagMemory, "\tthe header is in an invalid state!\n");
				valid = false;
			}
#if defined(sig_memory_validate_tokens) && defined(sig_memory_tokenfill)
			// check each byte of the user memory to ensure it matches it's specified state;
			// this is obviously extremely slow and should only be done when trying to hunt down a nasty bug!
			if( mOccupyingHeaderSize > 0 && header->mState != tChunkyHeapAllocHeader::cStateAllocd )
			{
				byte* ptr=header->mClientAddr;
				u32 tokenUninit = 0;
				u32 tokenFreed = 0;
				fMemSet(&tokenUninit, cMemTokenGarbage, sizeof(tokenUninit));
				fMemSet(&tokenFreed, cMemTokenFreed, sizeof(tokenFreed));
				for( u32 i=0; i < header->mClientByteCount; i+=4 )
				{
					if( (*(u32*)&ptr[i]) != tokenUninit && (*(u32*)&ptr[i]) != tokenFreed )
					{
						if( valid )
							Log::fPrintf( Log::cFlagMemory, onlyReportThisTextOnce, header, header->mStamp->mCppFile);
						Log::fPrintf( Log::cFlagMemory, "\tthe header memory debug tokens have been overwritten (likely a buffer overrun)!\n");
						valid = false;
						break;
					}
				}
			}
#endif//defined(sig_memory_validate_tokens) && defined(sig_memory_tokenfill)
		}

		assert_mem_valid( valid );
		return valid;
#endif//sig_memory_validate
	}

}}
