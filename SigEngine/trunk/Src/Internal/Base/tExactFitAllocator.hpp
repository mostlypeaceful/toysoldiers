//------------------------------------------------------------------------------
// \file tExactFitAllocator.hpp - 06 Aug 2013
// \author cbramwell
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tExactFitAllocator__
#define __tExactFitAllocator__
#include "IAllocator.hpp"

namespace Sig
{
	class tExactFitAllocator : public IAllocator
	{
		declare_uncopyable( tExactFitAllocator );
	public:
		tExactFitAllocator(const char* name, IAllocator* fallback);
		virtual void* fAlloc(u32 size) OVERRIDE;
		virtual void* fRealloc(void* mem, u32 size) OVERRIDE;
		virtual void fFree(void* mem) OVERRIDE;
		virtual u32 fSize(void* mem) const OVERRIDE;
		virtual u32 fTotalBytes() const OVERRIDE;
		virtual void fLog(std::wstringstream& log) const OVERRIDE;

	public:
		struct tAlloc
		{
			u32 mSize;
			void* mMem;
		};

	private:
		u32 fOverhead() const;

	private:
		const tStringPtr mName;
		tAllocatorPtr mFallback;
		tGrowableArray<tAlloc> mAllocs;
		tGrowableArray<tAlloc> mFrees;
	};
}//Sig

#endif//__tExactFitAllocator__
