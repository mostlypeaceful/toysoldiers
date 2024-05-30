//------------------------------------------------------------------------------
// \file tExactFitAllocator.cpp - 06 Aug 2013
// \author cbramwell
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tExactFitAllocator.hpp"

namespace Sig
{
	namespace
	{
		u32 fCountSize( const tGrowableArray<tExactFitAllocator::tAlloc>& list )
		{
			u32 size = 0;
			for( u32 i = 0; i < list.fCount(); ++i )
				size += list[i].mSize;
			return size;
		}
	}

	//------------------------------------------------------------------------------
	// tExactFitAllocator
	//------------------------------------------------------------------------------
	tExactFitAllocator::tExactFitAllocator(const char* name, IAllocator* fallback)
		: mName(name)
		, mFallback(fallback)
	{
		sigassert_is_main_thread();
		sigassert(name);
		sigassert(fallback);
	}

	//------------------------------------------------------------------------------
	void* tExactFitAllocator::fAlloc(u32 size)
	{
		sigassert_is_main_thread();
		profile_pix("tExactFitAllocator::fAlloc");
		for( u32 i = 0; i < mFrees.fCount(); ++i )
		{
			if(mFrees[i].mSize == size)
			{
				mAllocs.fPushBack(mFrees[i]);
				mFrees.fErase(i);
				return mAllocs.fBack().mMem;
			}
		}
		tAlloc& alloc = mAllocs.fPushBack();
		alloc.mMem = mFallback.fObject()->fAlloc(size);
		alloc.mSize = size;
		return alloc.mMem;
	}

	//------------------------------------------------------------------------------
	void* tExactFitAllocator::fRealloc(void* original, u32 size)
	{
		sigassert_is_main_thread();
		profile_pix("tExactFitAllocator::fRealloc");
		if(original == NULL)
			return fAlloc(size); //behave like malloc

		const u32 originalSize = fSize(original);
		if(size <= originalSize)
			return original; //no need to allocate

		if(void* mem = fAlloc(size))
		{
			memcpy(mem,original,originalSize);
			fFree(original);
			return mem;
		}

		return NULL; //allocation failure
	}

	//------------------------------------------------------------------------------
	void tExactFitAllocator::fFree(void* mem)
	{
		sigassert_is_main_thread();
		profile_pix("tExactFitAllocator::fFree");
		if(mem == NULL)
			return;

		for(u32 i = 0; i < mAllocs.fCount(); ++i)
		{
			if(mAllocs[i].mMem == mem)
			{
				mFrees.fPushBack(mAllocs[i]);
				mAllocs.fErase(i);
				return;
			}
		}
		sigassert( !"fFree() - No matching allocation found!" );
	}

	//------------------------------------------------------------------------------
	u32 tExactFitAllocator::fSize(void* mem) const
	{
		sigassert_is_main_thread();
		profile_pix("tExactFitAllocator::fSize");
		for(u32 i = 0; i < mAllocs.fCount(); ++i)
		{
			if(mAllocs[i].mMem == mem)
				return mAllocs[i].mSize;
		}
		sigassert( !"fSize() - No matching allocation found!" );
		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tExactFitAllocator::fTotalBytes() const
	{
		sigassert_is_main_thread();
		return fCountSize(mAllocs) + fCountSize(mFrees) + fOverhead();
	}

	//------------------------------------------------------------------------------
	void tExactFitAllocator::fLog(std::wstringstream& log) const
	{
		sigassert_is_main_thread();
		profile_pix("tExactFitAllocator::fLog");
		const u32 allocsBytes = fCountSize(mAllocs);
		const u32 freesBytes = fCountSize(mFrees);
		const f32 allocsMegs = Memory::fToMB<f32>(allocsBytes);
		const f32 totalMegs = Memory::fToMB<f32>(allocsBytes+freesBytes);

		log << "(tEFA)[" << mName << "] = " << std::fixed << std::setprecision(2) << allocsMegs << "/" << totalMegs
			<< ", blocks(A/F): " << mAllocs.fCount() << "/" << mFrees.fCount()
			<< ", ohead: " << Memory::fToMB<f32>(fOverhead()) << std::endl;
	}

	//------------------------------------------------------------------------------
	u32 tExactFitAllocator::fOverhead() const
	{
		sigassert_is_main_thread();
		return sizeof(tExactFitAllocator) 
			+ mAllocs.fElementSizeOf() * mAllocs.fCapacity() 
			+ mFrees.fElementSizeOf() * mFrees.fCapacity();
	}

}//Sig
