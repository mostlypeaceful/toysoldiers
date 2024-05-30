#include "BasePch.hpp"
#include "tHashTable.hpp"

//-----------------------------------------------------------------------------
// MurmurHashNeutral2, by Austin Appleby

// Same as MurmurHash2, but endian- and alignment-neutral.
// Half the speed though, alas.

inline unsigned int MurmurHashNeutral2 ( const unsigned char * data, int len, unsigned int seed )
{
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h = seed ^ len;

	while(len >= 4)
	{
		unsigned int k;

		k  = data[0];
		k |= data[1] << 8;
		k |= data[2] << 16;
		k |= data[3] << 24;

		k *= m; 
		k ^= k >> r; 
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}
	
	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
	        h *= m;
	};

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

namespace Sig { namespace Hash
{
	u32 fGenericHash( const Sig::byte* data, const u32 numBytes, const u32 maxHash )
	{
		return MurmurHashNeutral2( data, numBytes, 5381u ) % maxHash;
	}

}}

