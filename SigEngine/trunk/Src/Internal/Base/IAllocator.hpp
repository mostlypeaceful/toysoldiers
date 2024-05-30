//------------------------------------------------------------------------------
// \file IAllocator.hpp - 06 Aug 2013
// \author cbramwell
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __IAllocator__
#define __IAllocator__

namespace Sig
{
	class IAllocator : public tRefCounter
	{
		declare_uncopyable(IAllocator);
	public:
		IAllocator() {}
		virtual ~IAllocator() {}
		virtual void* fAlloc(u32 size) = 0;
		virtual void* fRealloc(void* mem, u32 size) = 0; //Same behavior as: http://www.cplusplus.com/reference/cstdlib/realloc/
		virtual void fFree(void* mem) = 0;
		virtual u32 fSize(void* mem) const = 0;
		virtual u32 fTotalBytes() const = 0;
		virtual void fLog(std::wstringstream& log) const = 0;
	public:
		tWeakPtrHead mWeakPtrHead;
	};
	typedef weak_ptr( IAllocator, mWeakPtrHead ) tAllocatorPtr;
}//Sig

#endif//__IAllocator__
