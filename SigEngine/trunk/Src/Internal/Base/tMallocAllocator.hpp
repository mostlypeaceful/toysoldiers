//------------------------------------------------------------------------------
// \file tMallocAllocator.hpp - 06 Aug 2013
// \author cbramwell
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tMallocAllocator__
#define __tMallocAllocator__
#include "IAllocator.hpp"

namespace Sig
{
	class tMallocAllocator : public IAllocator
	{
		declare_uncopyable( tMallocAllocator );
	public:
		tMallocAllocator();
		virtual void* fAlloc(u32 size) OVERRIDE;
		virtual void* fRealloc(void* mem, u32 size) OVERRIDE;
		virtual void fFree(void* mem) OVERRIDE;
		virtual u32 fSize(void* mem) const OVERRIDE;
		virtual u32 fTotalBytes() const OVERRIDE;
		virtual void fLog(std::wstringstream& log) const OVERRIDE;
	};
}//Sig

#endif//__tMallocAllocator__
