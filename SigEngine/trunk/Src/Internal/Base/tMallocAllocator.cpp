//------------------------------------------------------------------------------
// \file tMallocAllocator.cpp - 06 Aug 2013
// \author cbramwell
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tMallocAllocator.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tMallocAllocator
	//------------------------------------------------------------------------------
	tMallocAllocator::tMallocAllocator()
	{
	}

	//------------------------------------------------------------------------------
	void* tMallocAllocator::fAlloc(u32 size)
	{
		return malloc(size);
	}

	//------------------------------------------------------------------------------
	void* tMallocAllocator::fRealloc(void* mem, u32 size)
	{
		return realloc(mem, size);
	}

	//------------------------------------------------------------------------------
	void tMallocAllocator::fFree(void* mem)
	{
		return free(mem);
	}

	//------------------------------------------------------------------------------
	u32 tMallocAllocator::fSize(void* mem) const
	{
		return _msize(mem);
	}

	//------------------------------------------------------------------------------
	u32 tMallocAllocator::fTotalBytes() const
	{
		return 0;
	}

	//------------------------------------------------------------------------------
	void tMallocAllocator::fLog(std::wstringstream& log) const
	{
	}

}//Sig
