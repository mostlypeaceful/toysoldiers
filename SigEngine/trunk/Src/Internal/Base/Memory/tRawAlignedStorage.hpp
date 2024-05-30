#ifndef __Memory_tAlignment__
#define __Memory_tAlignment__

#include "Metaprogramming/Conditionals.hpp"

namespace Sig { namespace Memory
{
	namespace Detail
	{
		template<typename t,size_t N,typename alignT> struct tRawAlignedStorageImpl
		{
			union
			{
				alignT _DontUse_ForcemItemsToAlign;
				byte mItems[sizeof(t)*N];
			};
		};
	}

	template<typename t,size_t N> struct tRawMaxAlignedStorage :
		Metaprogramming::SelectIfC<
			// sizeof(t[N]) == sizeof(Detail::tRawAlignedStorageImpl<t,N,u64>), Detail::tRawAlignedStorageImpl<t,N,u64>, // Don't do 64-bit alignment yet
			sizeof(t[N]) == sizeof(Detail::tRawAlignedStorageImpl<t,N,u32>), Detail::tRawAlignedStorageImpl<t,N,u32>,
			sizeof(t[N]) == sizeof(Detail::tRawAlignedStorageImpl<t,N,u16>), Detail::tRawAlignedStorageImpl<t,N,u16>,
			sizeof(t[N]) == sizeof(Detail::tRawAlignedStorageImpl<t,N,u8 >), Detail::tRawAlignedStorageImpl<t,N,u8 >
		 >::tResult
	{
		// has byte mItems[sizeof(t)*N];
	};

	template<typename t,size_t N> struct tRawMinAlignedStorage :
		Metaprogramming::SelectIfC<
			// sizeof(t) % 8 == 0, Detail::tRawAlignedStorageImpl<t,N,u64>, // Don't do 64-bit alignment yet
			sizeof(t) % 4 == 0, Detail::tRawAlignedStorageImpl<t,N,u32>,
			sizeof(t) % 2 == 0, Detail::tRawAlignedStorageImpl<t,N,u16>,
			sizeof(t) % 1 == 0, Detail::tRawAlignedStorageImpl<t,N,u8 >
		 >::tResult
	{
	};
}} // namespace Sig::Memory

#endif //ndef __Memory_tAlignment__
