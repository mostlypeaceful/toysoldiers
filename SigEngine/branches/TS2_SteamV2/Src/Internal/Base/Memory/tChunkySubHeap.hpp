//------------------------------------------------------------------------------
// \file tChunkySubHeap.hpp - 28 Sep 2010
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tChunkySubHeap__
#define __tChunkySubHeap__
#include "MemoryUtil.hpp"

namespace Sig { namespace Memory
{
	struct tChunkyHeapAllocHeader
	{
		enum tState
		{
			cStateGarbage,
			cStateAllocd,
			cStateFreed,
			cStateCount,
		};

		static const u32 cDontTrampleValue = 0xabcd;

		inline b32 operator==(const tChunkyHeapAllocHeader& other) { return mClientAddr==other.mClientAddr && mClientByteCount==other.mClientByteCount; }
		inline b32 operator!=(const tChunkyHeapAllocHeader& other) { return mClientAddr!=other.mClientAddr || mClientByteCount!=other.mClientByteCount; }

		inline tChunkyHeapAllocHeader( byte* _mem, u32 _bytes, tState _state ) 
			: mTrampleCheck(cDontTrampleValue)
			, mState(_state)
			, mClientAddr(_mem)
			, mClientByteCount(_bytes)
			, mNext(0)
			, mPrev(0)
		{ }
		u16			mTrampleCheck;
		u16			mState;
		byte*		mClientAddr;
		u32			mClientByteCount;
		tAllocStamp mStamp;
		tChunkyHeapAllocHeader* mNext;
		tChunkyHeapAllocHeader* mPrev;
	};

	class base_export tChunkySubHeap
	{
	public:
		tChunkySubHeap( );
		void fInit( void* managedAddr, u32 managedBytes, u32 alignment, u32 maxFreeChunks, u32 minUserMemSize );
		void fShutdown( );
		void* fAlloc( u32 bytes, u32 alignmentIgnored, const tAllocStamp& stamp );
		u32 fFree( void* address );

		tAlignedAllocation fAllocAligned( u32 bytes, u32 alignment, const tAllocStamp& stamp );
		void fFreeAligned( const tAlignedAllocation& alignedAlloc );

		void fSplit( byte* mem, u32 neededBytes );
		virtual void fConsolidate();

		b32 fContains( void* address ) const;
		u32	fGetAllocSize( void* address ) const;
		u32	fGetAlignment( ) const { return mAlignment; }

		// debug/info methods
		b32 fValidate( );
		void fDump( );
		void fDumpNamedAllocations( tMemoryDumpOptions& memoryDumpOptions, tPageDump* page );
		void fDumpSubset( const char* nameToLookFor );
		u32	fLargestFreeChunkSize( ) const;
		void* fManagedAddr( ) const { return mManagedAddr; }
		u32	fNumBytesAllocd( ) const { return mManagedBytes - mFreeBytes; }
		u32	fNumBytesFree( ) const { return mFreeBytes; }
		u32	fNumBytesBeingManaged( ) const { return mManagedBytes; }
		u32 fNumChunksInUse( ) const { return mNumUsedChunks; }
		u32 fNumFreeChunks( ) const { return mNumFreeChunks; }

	protected:
		virtual tChunkyHeapAllocHeader* fHeaderFromClientAddr( byte* memPtr ) const;
		virtual tChunkyHeapAllocHeader* fNewHeader( byte* startOfNewMemory, u32 numBytes, tChunkyHeapAllocHeader::tState state, const tAllocStamp& stamp );
		virtual void fFreeHeader( tChunkyHeapAllocHeader* header );
		virtual u32 fComputeOccupyingHeaderSize( );

		tChunkyHeapAllocHeader*	fAllocInternal( u32 bytes, const tAllocStamp& stamp );
		void fFreeInternal( tChunkyHeapAllocHeader* header );

		tChunkyHeapAllocHeader*	fFindBestFit( u32 bytes );
		void	fSort( tChunkyHeapAllocHeader*& head );
		void	fAddToList( tChunkyHeapAllocHeader* chunk, tChunkyHeapAllocHeader*& head, u32& count );
		void	fInsertIntoList( tChunkyHeapAllocHeader* chunk, tChunkyHeapAllocHeader* insertAfter, tChunkyHeapAllocHeader*& head, u32& count );
		void	fRemoveFromList( tChunkyHeapAllocHeader* chunk, tChunkyHeapAllocHeader*& head, u32& count );
		b32		fValidateChunk( tChunkyHeapAllocHeader* chunk ) const;
		tChunkyHeapAllocHeader*	fSplit( tChunkyHeapAllocHeader* chunk, u32 neededBytes, b32 returnNewHeader );

	protected:
		void* mManagedAddr;
		u32   mManagedBytes;
		u32	  mFreeBytes;

		tChunkyHeapAllocHeader*	mFreeChunkHead;
		tChunkyHeapAllocHeader*	mInUseHead;
		u32 mNumFreeChunks;
		u32 mNumUsedChunks;

		u32	mMinUserMemSize;
		u32	mAlignment;
		u32	mOccupyingHeaderSize;
		u32	mMaxFreeChunks;
	};

}}

#endif//__tChunkySubHeap__
