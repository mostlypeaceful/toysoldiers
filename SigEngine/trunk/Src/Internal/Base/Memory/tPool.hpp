#ifndef __tPool__
#define __tPool__

#include "MemoryUtil.hpp"

namespace Sig { namespace Memory
{
	class tHeap;
	struct tMemoryDumpOptions;

	class base_export tPool : public tUncopyable
	{
	private:
		struct base_export tObjectHeader
		{
			tObjectHeader*	mNext;
#ifdef sig_memory_dump
			tAllocStampPtr		mStamp;
#endif
		};

		struct base_export tPage
		{
			void*		mMemory;
			tPage*		mNext;
		};

	private:
		tPool*			mNextPool;
		tPool*			mPrevPool;
		const char*		mName;
		tHeap*			mHeap;
		u32				mObjectHeaderSize;	// only non-zero with sig_memory_dump
		u32				mObjectStride;		// only different than object size for sig_memory_dump
		u32				mObjectSize;
		u32				mNumObjectsPerPage;
		tPage*			mPageList;
		tObjectHeader*	mFreeList;

		u32				mNumObjectsAllocd;

	public:
		tPool( );
		tPool( u32 objectSize, u32 numObjectsPerPage, u32 numInitialPages, const char* name, tHeap* heap = 0 /*defaults to global heap if null is specified*/ );
		~tPool( );
		void fCreate( u32 objectSize, u32 numObjectsPerPage, u32 numInitialPages, const char* name, tHeap* heap = 0 /*defaults to global heap if null is specified*/ );

		u32 fObjectSize( ) const { return mObjectSize; }
		u32 fNumObjectsPerPage( ) const { return mNumObjectsPerPage; }
		u32 fNumObjectsAllocd( ) const { return mNumObjectsAllocd; }
		u32 fNumPagesAllocd( ) const;

		void  fClearPageMemory( ); // should only be called if you're sure that fNumObjectsAllocd( ) == 0
		b32   fClean( ); // releases any unused pages, returns true if anything was actually cleaned
		void* fAlloc( const tAllocStamp& stamp );
		void  fFree( void* object );

		inline b32 fContains( void* object ) const
		{
			const u32 numPageBytes = fComputeBytesForPage( );
			for( tPage* i = mPageList; i; i = i->mNext )
			{
				if( object >= ( Sig::byte* )i->mMemory + mObjectHeaderSize && object < ( Sig::byte* )i->mMemory + numPageBytes )
					return true;
			}

			return false;
		}

		static void fCleanAll( ); // releases any unused pages on all existing pools

		static void fDumpGlobalPoolAllocations( tMemoryDumpOptions& memoryDumpOptions );

		void fDumpNamedAllocations( tMemoryDumpOptions& memoryDumpOptions );

	private:
		inline b32 fComputeBytesForPage( ) const {  return mObjectStride * mNumObjectsPerPage; }
		void fNewPage( );
		void fLinkUp( );
		void fUnLink( );

		b32 fFoundInFreeList( void* memory );
	};


#define define_class_pool_new_delete( className, numObjectsPerPage ) \
	public: static ::Sig::Memory::tPool& fClassPool( ) \
		{ static ::Sig::Memory::tPool gClassPool( sizeof( className ), numObjectsPerPage, 1, #className ); return gClassPool; } \
	static void fAllocClassPoolPage( ) { fClassPool( ); } \
	static void* operator new ( size_t numBytes ) \
		{ sigassert( numBytes <= fClassPool( ).fObjectSize( ) && "derived type of "#className" needs its own class pool!" ); return fClassPool( ).fAlloc( Memory::tAllocStamp::fNoContextStamp( ) ); } \
	static void* operator new( size_t numBytes, const Memory::tAllocStamp& stamp ) \
		{ sigassert( numBytes <= fClassPool( ).fObjectSize( ) && "derived type of "#className" needs its own class pool!" ); return fClassPool( ).fAlloc( stamp ); } \
	static void* operator new( size_t numBytes, void* mem ) \
		{ sigassert( numBytes <= fClassPool( ).fObjectSize( ) && "derived type of "#className" needs its own class pool!" ); return mem; } \
	static void operator delete( void* object, const Memory::tAllocStamp& stamp ) \
		{ fClassPool( ).fFree( object ); } \
	static void operator delete( void* object ) \
		{ fClassPool( ).fFree( object ); } \
	static void operator delete( void* object, void* mem ) \
		{  }

#define define_class_pool_new_delete_mt( className, numObjectsPerPage ) \
	public: static ::Sig::Memory::tPool& fClassPool( ) \
		{ static ::Sig::Memory::tPool gClassPool( sizeof( className ), numObjectsPerPage, 1, #className ); return gClassPool; } \
	static ::Sig::Threads::tCriticalSection& fClassPoolCriticalSection( ) \
		{ static ::Sig::Threads::tCriticalSection gClassPoolCriticalSection; return gClassPoolCriticalSection; } \
	static void fAllocClassPoolPage( ) { fClassPool( ); } \
	static void* operator new( size_t numBytes ) \
		{ ::Sig::Threads::tMutex threadLock( fClassPoolCriticalSection( ) ); sigassert( numBytes <= fClassPool( ).fObjectSize( ) && "derived type of "#className" needs its own class pool!" ); return fClassPool( ).fAlloc( Memory::tAllocStamp::fNoContextStamp( ) ); } \
	static void* operator new( size_t numBytes, void* mem ) \
		{ sigassert( numBytes <= fClassPool( ).fObjectSize( ) && "derived type of "#className" needs its own class pool!" ); return mem; } \
	static void* operator new( size_t numBytes, const Memory::tAllocStamp& stamp ) \
		{ ::Sig::Threads::tMutex threadLock( fClassPoolCriticalSection( ) ); sigassert( numBytes <= fClassPool( ).fObjectSize( ) && "derived type of "#className" needs its own class pool!" ); return fClassPool( ).fAlloc( stamp ); } \
	static void operator delete( void* object, const Memory::tAllocStamp& stamp ) \
		{ ::Sig::Threads::tMutex threadLock( fClassPoolCriticalSection( ) ); fClassPool( ).fFree( object ); } \
	static void operator delete( void* object ) \
		{ ::Sig::Threads::tMutex threadLock( fClassPoolCriticalSection( ) ); fClassPool( ).fFree( object ); } \
	static void operator delete( void* object, void* mem ) \
		{  }

}}

#endif//__tPool__
