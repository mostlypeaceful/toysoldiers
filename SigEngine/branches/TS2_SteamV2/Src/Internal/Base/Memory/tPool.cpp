#include "BasePch.hpp"
#include "tPool.hpp"
#include "tMainMemoryProvider.hpp"

// Dumpability consumes extra memory. Only enable while debugging.
// undef sig_memory_dump during "good times"

// *update the consumption from this appears to be negligible. ( < 0.1mb )
//#undef sig_memory_dump 

namespace Sig { namespace Memory
{
	namespace
	{
		static tPool* gGlobalPoolList = NULL;
		static tAllocStamp cFreeStamp( "Free" );
	}

	tPool::tPool( )
	{
		fZeroOut( this );
		fLinkUp( );
	}

	tPool::tPool( u32 objectSize, u32 numObjectsPerPage, u32 numInitialPages, const char* name, tHeap* heap )
	{
		fZeroOut( this );
		fLinkUp( );
		fCreate( objectSize, numObjectsPerPage, numInitialPages, name, heap );
	}

	void tPool::fCreate( u32 objectSize, u32 numObjectsPerPage, u32 numInitialPages, const char* name, tHeap* heap )
	{
		fClearPageMemory( );

		mName = name;
		mHeap = heap ? heap : &::Sig::Memory::tMainMemoryProvider::fInstance( );

#ifdef sig_memory_dump
		mObjectSize = objectSize;
		mObjectHeaderSize = sizeof( tObjectHeader );
		mObjectStride = mObjectSize + mObjectHeaderSize;
#else
		mObjectSize = fMax( ( u32 )sizeof( tObjectHeader ), objectSize );
		mObjectHeaderSize = 0;
		mObjectStride = mObjectSize;
#endif

		mNumObjectsPerPage = numObjectsPerPage;
		mPageList = 0;
		mFreeList = 0;
		mNumObjectsAllocd = 0;

		for( u32 i = 0; i < numInitialPages; ++i )
			fNewPage( );
	}

	tPool::~tPool( )
	{
		fClearPageMemory( );
		fUnLink( );
	}

	void tPool::fClearPageMemory( )
	{
#ifndef target_tools // TODO shouldn't be doing this
		if( mNumObjectsAllocd != 0 )
			log_warning( Log::cFlagMemory, "tPool::fClearPageMemory() [" << mName << "]: objects still allocated (leaking memory)" );
#endif//target_tools

		tPage* next = mPageList;
		while( next )
		{
			tPage* kill = next;
			next = kill->mNext;
			mHeap->fFree( kill->mMemory );
			mHeap->fFree( kill );
		}

		mPageList = 0;
		mFreeList = 0;
		mNumObjectsAllocd = 0;
	}

	b32 tPool::fClean( )
	{
		if( !mPageList /*|| !mPageList->mNext*/ )
			return false; // i.e., we only clear pools with more than one page

		if( mNumObjectsAllocd == 0 )
		{
			// if no objects are allocated, we can simply clear all page memory
			fClearPageMemory( );
			return true;
		}
		else
		{
			// TODO count up objects in each page,
			//  see if all are in free list;
			//   if so, we can safely delete that page;
			//     we will need to rebuild the free list when we're done

			//for( tPage* page = mPageList; page; page = page->mNext )
			//{
			//	tObjectHeader* obj = ( tObjectHeader* )page->mMemory;
			//	for( u32 ithObj = 0; ithObj < mNumObjectsPerPage; ++ithObj )
			//	{
			//		obj = ( tObjectHeader* )( ( Sig::byte* )obj + mObjectSize );
			//	}
			//}
		}

		return false;
	}

	void* tPool::fAlloc( const tAllocStamp& stamp )
	{
		if( !mFreeList )
			fNewPage( );

		sigassert( mFreeList );
		tObjectHeader* o = mFreeList;

#ifdef sig_logging
		if( !o )
		{
			Memory::tPool::fDumpGlobalPoolAllocations( Memory::tMemoryDumpOptions( ) );
			sigassert( o && "out of memory!" );
		}
#endif

		mFreeList = mFreeList->mNext;
		++mNumObjectsAllocd;

#ifdef sig_memory_dump
		o->mStamp = stamp;
		return ( void* )(((Sig::byte*)o) + sizeof(tObjectHeader));
#endif

		return ( void* )o;
	}

	void tPool::fFree( void* object )
	{
		sigassert( mNumObjectsAllocd > 0 );
		--mNumObjectsAllocd;

#ifdef sig_memory_dump
		tObjectHeader* i = ( tObjectHeader* )(((Sig::byte*)object) - sizeof(tObjectHeader));
		i->mStamp = cFreeStamp;
#else
		tObjectHeader* i = ( tObjectHeader* )object;
#endif
		sigassert( i );
		i->mNext = mFreeList;
		mFreeList = i;
	}

	void tPool::fNewPage( )
	{
		const u32 numPageBytes = fComputeBytesForPage( );
		if( numPageBytes == 0 ) return;

		void* rawPageMemory = mHeap->fAlloc( sizeof( tPage ), tAllocStamp( mName, 0, "Page Header" ) );
		if( !rawPageMemory )
		{
			log_warning( Log::cFlagMemory, 
				"Attempting to allocate new pool page (alloc size = " << mObjectSize << "), but out of memory - will try to clean pools as a last ditch effort." );
			fCleanAll( );
			rawPageMemory = mHeap->fAlloc( sizeof( tPage ), tAllocStamp( mName, 0, "Page Header" ) );
			sigassert( rawPageMemory && "out of memory in tPool::fNewPage" );
		}

		tPage* newPage = new ( rawPageMemory ) tPage;
		newPage->mNext = mPageList;
		mPageList = newPage;

		newPage->mMemory = mHeap->fAlloc( numPageBytes, tAllocStamp( mName, 0, "Page", NULL, numPageBytes ) );
		sigassert( newPage->mMemory && "out of memory in tPool::fNewPage" );

		tObjectHeader* obj = ( tObjectHeader* )newPage->mMemory;
		tObjectHeader* next = ( tObjectHeader* )( ( Sig::byte* )obj + mObjectStride );
		for( u32 ithObj = 0; ithObj < mNumObjectsPerPage - 1; ++ithObj )
		{
			obj->mNext = next;
#ifdef sig_memory_dump
			obj->mStamp = cFreeStamp;
#endif

			obj = next;
			next = ( tObjectHeader* )( ( Sig::byte* )obj + mObjectStride );
		}

		obj->mNext = mFreeList;
#ifdef sig_memory_dump
		obj->mStamp = cFreeStamp;
#endif
		mFreeList = ( tObjectHeader* )newPage->mMemory;
	}
	
	void tPool::fLinkUp( )
	{
		if( gGlobalPoolList )
		{
			mNextPool = gGlobalPoolList;
			gGlobalPoolList->mPrevPool = this;
		}
		gGlobalPoolList = this;
	}

	void tPool::fUnLink( )
	{
		if( mPrevPool )
			mPrevPool->mNextPool = mNextPool;
		if( mNextPool )
			mNextPool->mPrevPool = mPrevPool;
		if( gGlobalPoolList == this )
		{
			sigassert( !mPrevPool );
			gGlobalPoolList = mNextPool;
		}
	}

	void tPool::fCleanAll( )
	{
		u32 numPools = 0, numCleaned = 0;
		for( tPool* i = gGlobalPoolList; i; i = i->mNextPool )
		{
			if( i->fClean( ) )
				++numCleaned;
			++numPools;
		}

		//log_line( Log::cFlagMemory, "Cleaned " << numCleaned << " out of " << numPools << " pools." );
	}

	void tPool::fDumpGlobalPoolAllocations( tMemoryDumpOptions& memoryDumpOptions )
	{
#ifdef sig_memory_dump
		for( tPool* i = gGlobalPoolList; i; i = i->mNextPool )
		{
			i->fDumpNamedAllocations( memoryDumpOptions );
		}
#else
		log_warning( 0, "App not compiled with sig_memory_dump!" );
#endif
	}

	b32 tPool::fFoundInFreeList( void* memory )
	{
		for( tObjectHeader* freed = mFreeList; freed; freed = freed->mNext )
		{
			if( freed == memory )
				return true;
		}

		return false;
	}

#ifdef sig_memory_dump
	struct NamedAllocCount
	{
		const char* mName;
		u32   mBytesUsed;
		u32   mCount;
	};
#endif//sig_memory_dump

	void tPool::fDumpNamedAllocations( tMemoryDumpOptions& memoryDumpOptions )
	{
#ifdef sig_memory_dump
		{
			if( memoryDumpOptions.mPost )
			{
				Log::fPrintf( Log::cFlagMemory,  "Pool Name: %s\n NumObjectPerPage: %d \n", mName, mNumObjectsPerPage );
			}

			tHeapDump* heapDump = NULL;
			tPageDump* pageDump = NULL;
			if( memoryDumpOptions.mDump )
			{
				memoryDumpOptions.mDump->mHeaps.fPushBack( tHeapDump( mName, Memory::tHeapDump::cTypePool ) );
				heapDump = &memoryDumpOptions.mDump->mHeaps.fBack( );
			}

			u32 pages = 0;
			for( tPage* page = mPageList; page; page = page->mNext )
			{
				++pages;

				if( heapDump )
				{
					heapDump->mPages.fPushBack( tPageDump( fComputeBytesForPage( ), 0, 0, (u32)page->mMemory ) );
					pageDump = &heapDump->mPages.fBack( );
					pageDump->mAllocations.fSetCapacity( mNumObjectsPerPage );
				}

				tObjectHeader* obj = ( tObjectHeader* )page->mMemory;

				u32 numObjInPageUsed = 0;
				for( u32 ithObj = 0; ithObj < mNumObjectsPerPage; ++ithObj )
				{
					if( !fFoundInFreeList( obj ) )
					{
						++numObjInPageUsed;

						if( pageDump )
							pageDump->mAllocations.fPushBack( tAllocDump( tStampDump( obj->mStamp, (u32)obj ) ) );
					}

					obj = ( tObjectHeader* )( ( Sig::byte* )obj + mObjectStride );
				}

				u32 totalChunks   = mNumObjectsPerPage;
				u32 emptyChunks   = totalChunks - numObjInPageUsed;
				f32 wastedMB   = emptyChunks * mObjectSize / (1024.f * 1024.f);
				f32 wastedPercent = ( f32 )emptyChunks / totalChunks * 100.0f;

				if( memoryDumpOptions.mPost )
				{
					Log::fPrintf( Log::cFlagMemory, " - Page: %d  Wasted: %d  Space: %.2f%% \n", pages, emptyChunks, wastedPercent );
					//Log::fPrintf( Log::cFlagMemory,  "##----------------- End Page -----------------##\n");
				}

				if( pageDump )
					pageDump->mPercentUsed = tMemoryDump::fComputePercentage( numObjInPageUsed, mNumObjectsPerPage );
			}

			u32 totalChunks = pages * mNumObjectsPerPage;
			u32 emptyChunks = totalChunks - mNumObjectsAllocd;
			f32 wastedMB   = emptyChunks * mObjectSize / (1024.f * 1024.f);
			f32 wastedPercent = ( f32 )emptyChunks / totalChunks * 100.0f;

			if( memoryDumpOptions.mPost )
			{
				Log::fPrintf( Log::cFlagMemory, " - Pages: %d  Wasted: %d  Space: %.2f%% %.2f MB \n", pages, emptyChunks, wastedPercent, wastedMB );

				//Log::fPrintf( Log::cFlagMemory,  "Total From Named Objects=%f MB, Total Otherwise=%f MB\n", total/cOneMB, fNumBytesAllocd()/cOneMB );
				Log::fPrintf( Log::cFlagMemory,  "##----------------- End Pool -----------------##\n");
			}

			if( heapDump )
				heapDump->mPercentUsed = tMemoryDump::fComputePercentage( mNumObjectsAllocd, totalChunks );
		}
#endif
	}

}}


