#include "BasePch.hpp"
#include "tMainMemoryProvider.hpp"
#include "tPool.hpp"
#include "tProfiler.hpp"
#include "tApplication.hpp"
#include "Threads/LocalStorage.hpp"

namespace Sig { namespace Memory
{

	const char* tHeap::cVramHeapName = "VRAM";

	tPoolBasedHeap::tPoolBasedHeap( const char* name )
		: tHeap( name )
	{
		const u32 (&initialPageCounts)[ tMemoryOptions::cMemoryPoolCount ] = tApplication::fMemoryOptions( ).mPoolInitialPageCount;

		const u32 cTinyHeapSize   = 1024; //8192;
		const u32 cMediumHeapSize = 512; //2048;
		const u32 cLargeHeapSize  = 256; //256;

		mPools[ 0  ].fCreate( 8,    cTinyHeapSize, initialPageCounts[ tMemoryOptions::c8_BytePool ], "8-Byte Global Pool" );
		mPools[ 1  ].fCreate( 16,   cTinyHeapSize, initialPageCounts[ tMemoryOptions::c16_BytePool ], "16-Byte Global Pool" );
		mPools[ 2  ].fCreate( 32,   cTinyHeapSize, initialPageCounts[ tMemoryOptions::c32_BytePool ], "32-Byte Global Pool" );
		mPools[ 3  ].fCreate( 64,   cTinyHeapSize, initialPageCounts[ tMemoryOptions::c64_BytePool ], "64-Byte Global Pool" );

		mPools[ 4  ].fCreate( 128,  cMediumHeapSize, initialPageCounts[ tMemoryOptions::c128_BytePool ], "128-Byte Global Pool" );
		mPools[ 5  ].fCreate( 256,  cMediumHeapSize, initialPageCounts[ tMemoryOptions::c256_BytePool ], "256-Byte Global Pool" );
		mPools[ 6  ].fCreate( 512,  cMediumHeapSize, initialPageCounts[ tMemoryOptions::c512_BytePool ], "512-Byte Global Pool" );
		mPools[ 7  ].fCreate( 1024, cLargeHeapSize, initialPageCounts[ tMemoryOptions::c1024_BytePool ], "1024-Byte Global Pool" );

		mPools[ 8  ].fCreate( 1280, cLargeHeapSize, initialPageCounts[ tMemoryOptions::c1280_BytePool ], "1280-Byte Global Pool" );
		mPools[ 9  ].fCreate( 1536, cLargeHeapSize, initialPageCounts[ tMemoryOptions::c1536_BytePool ], "1536-Byte Global Pool" );
		mPools[ 10 ].fCreate( 1792, cLargeHeapSize, initialPageCounts[ tMemoryOptions::c1792_BytePool ], "1792-Byte Global Pool" );
		mPools[ 11 ].fCreate( 2048, cLargeHeapSize, initialPageCounts[ tMemoryOptions::c2048_BytePool ], "2048-Byte Global Pool" );
	}

	tPoolBasedHeap::~tPoolBasedHeap( )
	{
	}

	void* tPoolBasedHeap::fAlloc( u32 numBytes, const tAllocStamp& stamp )
	{
		for( tPoolList::tIterator i = mPools.fBegin( ), iend = mPools.fEnd( ); i != iend; ++i )
		{
			if( numBytes <= i->fObjectSize( ) )
			{
				return i->fAlloc( stamp );
			}
		}
		return tMallocHeap::fInstance( ).fAlloc( numBytes, stamp );
	}

	void* tPoolBasedHeap::fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp )
	{
		void* newMem = fAlloc( numBytes, stamp );
		if( memory )
		{
			fMemCpy( newMem, memory, numBytes );
			fFree( memory );
		}
		return newMem;
	}

	void tPoolBasedHeap::fFree( void* memory )
	{
		for( tPoolList::tIterator i = mPools.fBegin( ), iend = mPools.fEnd( ); i != iend; ++i )
		{
			if( i->fContains( memory ) )
			{
				i->fFree( memory );
				return;
			}
		}

		return tMallocHeap::fInstance( ).fFree( memory );
	}
	u32 tPoolBasedHeap::fSizeOfAlloc( void* memory )
	{
		for( tPoolList::tIterator i = mPools.fBegin( ), iend = mPools.fEnd( ); i != iend; ++i )
		{
			if( i->fContains( memory ) )
				return i->fObjectSize( );
		}

		//return tMainMemoryProvider::fInstance( ).fSizeOfAlloc( memory );
		return tMallocHeap::fInstance( ).fSizeOfAlloc( memory );
	}

	void tPoolBasedHeap::fDump( tMemoryDumpOptions& memoryDumpOptions)
	{
		if( memoryDumpOptions.mPost )
		{
			Log::fPrintf( Log::cFlagMemory,  "-----------------------------------------------------\n");
			Log::fPrintf( Log::cFlagMemory,  "tPoolBasedHeap Name: %s \n", mName );
		}

		for( u32 i = 0; i < mPools.fCount( ); ++i )
		{
			mPools[ i ].fDumpNamedAllocations( memoryDumpOptions );
		}

		if( memoryDumpOptions.mPost )
			Log::fPrintf( Log::cFlagMemory,  "#####################################################\n");
	}

	///
	/// \brief Re-implements tPoolBasedHeap but with thread-safe critical sections.
	class tGlobalHeap : public tPoolBasedHeap
	{
		typedef tPoolBasedHeap tBase;
		Threads::tCriticalSection mLock;
	public:
		tGlobalHeap( )
			: tPoolBasedHeap( "tGlobalHeap" )
		{ }

		virtual void* fAlloc( u32 numBytes, const tAllocStamp& stamp )
		{
			Threads::tMutex lock( mLock );
			return tBase::fAlloc( numBytes, stamp );
		}
		virtual void* fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp )
		{
			Threads::tMutex lock( mLock );
			return tBase::fRealloc( memory, numBytes, stamp );
		}
		virtual void fFree( void* memory )
		{
			if( !memory )
				return;
			Threads::tMutex lock( mLock );
			tBase::fFree( memory );
		}
		virtual u32 fSizeOfAlloc( void* memory )
		{
			Threads::tMutex lock( mLock );
			return tBase::fSizeOfAlloc( memory );
		}

		virtual void fAddStats( std::wstringstream& ss )
		{
			u32 totalUsage = 0;
			u32 totalWaste = 0;
			for(u32 i = 0; i < mPools.fCount( ); ++i)
			{
				const u32 objsAllocd = mPools[i].fNumPagesAllocd() * mPools[i].fNumObjectsPerPage();

				totalUsage += objsAllocd * mPools[i].fObjectSize();

				const u32 objsWaste = objsAllocd - mPools[i].fNumObjectsAllocd();
				const u32 bytesWaste = objsWaste * mPools[i].fObjectSize();

				totalWaste += bytesWaste;

				ss << "Pool["	<< std::setfill( L'_' )
								<< std::setw( 4 ) << mPools[i].fObjectSize() << ", " 
								<< std::setw( 4 ) << mPools[i].fNumObjectsPerPage() << "] pages: "
								<< std::setw( 4 ) << mPools[i].fNumPagesAllocd() << " objs: "
								<< std::setw( 8 ) << mPools[i].fNumObjectsAllocd() << "  waste: "
								<< std::setw( -1 ) << bytesWaste << " bytes" << std::endl;
			}

			ss << "Total pool usage: " << std::fixed << std::setprecision( 2 ) << (totalUsage / (1024.f * 1024.f)) << "MB" << std::endl;
			ss << "Total pool waste: " << std::fixed << std::setprecision( 2 ) << (totalWaste / (1024.f * 1024.f)) << "MB" << std::endl;
		}
	};

	namespace
	{
		tGlobalHeap* gGlobalHeap = 0;
	}

	Memory::tAllocStamp tHeap::mVramContext = Memory::tAllocStamp::fNoContextStamp( );

	tHeap& tHeap::fInstance( )
	{
		if( !gGlobalHeap )
		{
			tMallocHeap::fInstance( ); // force tMallocHeap to alloc first, and in turn, deallocate last.
			static tGlobalHeap gGlobalHeapStorage; // moved here to allow it to align itself
			gGlobalHeap = &gGlobalHeapStorage;
		}

		profile_early_init( );

		return *gGlobalHeap;
	}

	b32 tHeap::fInstanceCreated( )
	{
		return gGlobalHeap != NULL;
	}

	void tHeap::fOutOfMemory( const tAllocStamp& stamp )
	{
		Log::fPrintf( Log::cFlagMemory, "-----------------------------------------------------\n");
		Log::fPrintf( Log::cFlagMemory, " Out of memory in heap '%s'. The application is now beyond memory budgets.\n", mName );
		Log::fPrintf( Log::cFlagMemory, "  Trying to allocate: \n" );
		stamp.fLog( Log::cFlagMemory, "  " );
		tApplication::fInstance( ).fLogFreeMemory( );
		Log::fPrintf( Log::cFlagMemory, "-----------------------------------------------------\n" );
		//Log::fFatalError( );
		
		fDump( tMemoryDumpOptions( true ) );
	}

	void tHeap::fPageLimitHit( const tAllocStamp& stamp )
	{
		if( !tApplication::fMemoryOptions( ).mDebugAllowNewPages )
			fOutOfMemory( stamp );
		else
			Log::fPrintf( Log::cFlagMemory, "Debug - Allowing page resize of heap: %s.", mName );
	}



	//------------------------------------------------------------------------------
	// tMallocHeap
	//------------------------------------------------------------------------------
	void* tMallocHeap::fAlloc( u32 size, const tAllocStamp& /*stamp*/ )
	{
		void* mem = malloc( size );
		if( !mem )
			Log::fFatalError( );
		return mem;
	}

	//------------------------------------------------------------------------------
	void* tMallocHeap::fRealloc( void* oldMem, u32 size, const tAllocStamp& /*stamp*/ )
	{
		void* mem = realloc( oldMem, size );
		if( !mem )
			Log::fFatalError( );
		return mem;
	}

	//------------------------------------------------------------------------------
	void tMallocHeap::fFree( void* mem )
	{
		free( mem );
	}

	//------------------------------------------------------------------------------
	u32 tMallocHeap::fSizeOfAlloc( void* mem )
	{
		return 0;
	}

#ifdef sig_memory_dump

	//------------------------------------------------------------------------------
	// tHeapStack
	//------------------------------------------------------------------------------
	namespace
	{
		static Threads::tCriticalSection& fHeapStackCriticalSection( )
		{
			static Threads::tCriticalSection gHeapStackCriticalSection;
			return gHeapStackCriticalSection;
		}

		static Threads::LocalStorage::tLocalStorageHandle& fHeapStackLSH( )
		{
			static Threads::LocalStorage::tLocalStorageHandle gHandle = Threads::LocalStorage::cInvalidHandle;
			return gHandle;
		}
	}


	tHeapStacker::tHeapStacker( tHeap* heap )
	{
		mStack = &tHeapStack::fThreadHeapStack( );
		mPrevHeap = &mStack->fHeapTop( );
		mStack->fSetTop( heap );
	}

	tHeapStacker::~tHeapStacker( )
	{
		mStack->fSetTop( mPrevHeap );
	}

	tHeapStack& tHeapStack::fThreadHeapStack( )
	{
		Threads::tMutex cMut( fHeapStackCriticalSection( ) );

		Threads::LocalStorage::tLocalStorageHandle& handle = fHeapStackLSH( );
		if( handle == Threads::LocalStorage::cInvalidHandle )
		{
			handle = Threads::LocalStorage::Allocate( );
			sigassert( handle != Threads::LocalStorage::cInvalidHandle );
		}

		tHeapStack* threadStack = reinterpret_cast<tHeapStack*>( Threads::LocalStorage::GetValue( handle ) );
		if( !threadStack )
		{
			threadStack = (tHeapStack*)malloc( sizeof( tHeapStack ) );
			if( !threadStack )
				Log::fFatalError( );
			new( threadStack ) tHeapStack( &tHeap::fInstance( ) ); // default to global heap.

			Threads::LocalStorage::SetValue( handle, threadStack );
		}

		sigassert( threadStack );

		fGlobalHeapStackInstantiated( ) = 1;
		return *threadStack;
	}

#endif


}}//Sig::Memory
