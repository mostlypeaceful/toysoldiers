//------------------------------------------------------------------------------
// \file tChunkyHeap.hpp - 28 Sep 2010
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tChunkyHeap__
#define __tChunkyHeap__
#include "tHeap.hpp"

namespace Sig { namespace Memory
{
	template<class tSubHeapType_, class tMutexType_>
	class base_export tChunkyHeap : public tHeap
	{
	public:
		typedef void* (*tExternalAllocFunc)( u32 numBytes );
		typedef void (*tExternalFreeFunc)( void* mem );

	protected:
		typedef tMutexType_ tMutexType;
		typedef tSubHeapType_ tSubHeapType;

		mutable typename tMutexType::tCriticalSection mLock;
		u32 mSubHeapCount;
		Memory::tMemoryOptions::tHeapBehavior mBehavior;

		static const u32 cMaxHeapsEver = 16;
		tFixedArray<tSubHeapType,cMaxHeapsEver> mSubHeaps;

		tExternalAllocFunc mExternalAllocFunc;
		tExternalFreeFunc mExternalFreeFunc;

	public:
		tChunkyHeap( tExternalAllocFunc allocFunc, tExternalFreeFunc freeFunc, const Memory::tMemoryOptions::tHeapBehavior& behavior, const char* name )
			: tHeap( name )
			, mSubHeapCount( 0 )
			, mBehavior( behavior )
			, mExternalAllocFunc( allocFunc )
			, mExternalFreeFunc( freeFunc )
		{
			sigassert( mBehavior.mMaxPages <= cMaxHeapsEver );
			sigassert( mBehavior.mInitialPageCount <= mBehavior.mMaxPages );

			for( u32 i = 0; i < mBehavior.mInitialPageCount; ++i )
				fAddSubHeap( tAllocStamp::cNoContext );
		}

		void* fAllocInternal( u32 numBytes, u32 alignment, const tAllocStamp& stamp )
		{
			tMutexType lock( mLock );

			while( mSubHeapCount < mSubHeaps.fCount( ) )
			{
				for( u32 i = 0; i < mSubHeapCount; ++i )
				{
					void* o = mSubHeaps[ i ].fAlloc( numBytes, alignment, stamp );
					if( o )
					{
						return o;
					}
				}

				fAddSubHeap( stamp );
			}

			sigassert( false && "Out of memory" );
			return 0;
		}
		void* fReallocInternal( void* memory, u32 numBytes, const tAllocStamp& stamp )
		{
			tMutexType lock( mLock );

			void* newMem = fAlloc( numBytes, stamp );
			if( memory )
			{
				fMemCpy( newMem, memory, numBytes );
				fFree( memory );
			}
			return newMem;
		}
		u32 fFreeInternal( void* memory )
		{
			tMutexType lock( mLock );

			sigassert( mSubHeapCount > 0 );

			for( u32 i = 0; i < mSubHeapCount; ++i )
			{
				const u32 numBytes = mSubHeaps[ i ].fFree( memory );
				if( numBytes )
				{
					if( mSubHeaps[ i ].fNumChunksInUse( ) == 0 && mBehavior.mReleaseEmptyPages )
					{
						fFreeSubHeapMemory( mSubHeaps[ i ].fManagedAddr( ), mSubHeaps[ i ].fNumBytesBeingManaged( ) );
						mSubHeaps[ i ] = mSubHeaps[ mSubHeapCount - 1 ];
						--mSubHeapCount;
						mSubHeaps[ mSubHeapCount ] = tSubHeapType( );
					}

					return numBytes;
				}
			}

			sigassert( false && "Invalid address" );
			return 0;
		}
		u32 fSizeOfAlloc( void* memory ) const
		{
			tMutexType lock( mLock );

			sigassert( mSubHeapCount > 0 );

			for( u32 i = 0; i < mSubHeapCount; ++i )
			{
				if( mSubHeaps[ i ].fContains( memory ) )
					return mSubHeaps[ i ].fGetAllocSize( memory );
			}

			sigassert( false && "Invalid address" );
			return 0;
		}
		virtual void fConsolidate( ) 
		{
			for( u32 i = 0; i < mSubHeapCount; ++i )
				mSubHeaps[ i ].fConsolidate( );
		}

		virtual void fDump( tMemoryDumpOptions& memoryDumpOptions )
		{
			if( memoryDumpOptions.mPost )
			{
				Log::fPrintf( Log::cFlagMemory,  "-----------------------------------------------------\n");
				Log::fPrintf( Log::cFlagMemory,  "tChunkyHeap Name: %s SubHeaps: %d \n", mName, mSubHeapCount );
			}

			tHeapDump* heapDump = NULL;
			tPageDump* pageDump = NULL;

			if( memoryDumpOptions.mDump )
			{
				memoryDumpOptions.mDump->mHeaps.fPushBack( tHeapDump( mName, tHeapDump::cTypeChunky, fNumBytesBeingManaged( ), tMemoryDump::fComputePercentage(fNumBytesAllocd( ), fNumBytesBeingManaged( )) ) );
				heapDump = &memoryDumpOptions.mDump->mHeaps.fBack( );
				heapDump->mPages.fSetCapacity( mSubHeapCount );
			}

			for( u32 i = 0; i < mSubHeapCount; ++i )
			{
				tChunkySubHeap& subHeap = mSubHeaps[ i ];

				if( heapDump )
				{
					heapDump->mPages.fPushBack( tPageDump( subHeap.fNumBytesBeingManaged( ), tMemoryDump::fComputePercentage( subHeap.fNumBytesAllocd( ), subHeap.fNumBytesBeingManaged( )), 0.f ) );
					pageDump = &heapDump->mPages.fBack( );
				}

				if( subHeap.fNumBytesBeingManaged( ) )
					subHeap.fDumpNamedAllocations( memoryDumpOptions, pageDump );
			}

			if( memoryDumpOptions.mPost )
				Log::fPrintf( Log::cFlagMemory,  "#####################################################\n");
		}

		virtual void fLogFreeMemory( ) const
		{
			const f32 cOneMb = 1024.f * 1024.f;
			Log::fPrintf( Log::cFlagMemory,  "-----------------------------------------------------\n");
			Log::fPrintf( Log::cFlagMemory,  " Name: %s, SubHeaps: %d, Size: %.2f, Free: %.2f \n", mName, mSubHeapCount, fNumBytesBeingManaged( )/cOneMb, fNumBytesFree( )/cOneMb );
		}

		u32 fNumFreeChunks( ) const
		{
			u32 num = 0;

			for( u32 i = 0; i < mSubHeapCount; ++i )
				num += mSubHeaps[ i ].fNumFreeChunks( );

			return num;
		}

	protected:
		virtual void fAddSubHeap( const tAllocStamp& stamp )
		{
			sigassert( mSubHeapCount + 1 <= mSubHeaps.fCount( ) );

			if( mSubHeapCount == mBehavior.mMaxPages )
				fPageLimitHit( stamp );

			s32 subHeapSize = mBehavior.mPageSize;
			const u32 alignment = mBehavior.mAlignment;
			const u32 maxFreeChunks = mBehavior.mMaxFreeChunks;
			const u32 minUserMemSize = mBehavior.mMinUserMemSize;

			void* mem = 0;
			do
			{
				mem = mExternalAllocFunc( subHeapSize );
				if( mem ) break;
				subHeapSize -= mBehavior.mPageSize / 8;
			} while( subHeapSize > 0 );


			if( !mem )
				fOutOfMemory( stamp );

			mSubHeaps[ mSubHeapCount ].fInit( mem, subHeapSize, alignment, maxFreeChunks, minUserMemSize );
			++mSubHeapCount;
		}

		virtual void fFreeSubHeapMemory( void* mem, u32 numBytes )
		{
			mExternalFreeFunc( mem );
		}

	public:
		u32	fNumBytesAllocd( ) const
		{
			u32 o = 0;
			for( u32 i = 0; i < mSubHeapCount; ++i )
				o += mSubHeaps[ i ].fNumBytesAllocd( );
			return o;
		}
		u32	fNumBytesFree( ) const
		{
			u32 o = 0;
			for( u32 i = 0; i < mSubHeapCount; ++i )
				o += mSubHeaps[ i ].fNumBytesFree( );
			return o;
		}
		u32	fNumBytesBeingManaged( ) const
		{
			u32 o = 0;
			for( u32 i = 0; i < mSubHeapCount; ++i )
				o += mSubHeaps[ i ].fNumBytesBeingManaged( );
			return o;
		}
	};
}}

#endif//__tChunkyHeap__
