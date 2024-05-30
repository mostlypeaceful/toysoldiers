#ifndef __tDecompressor__
#define __tDecompressor__
#include "zlib.h"

namespace Sig
{

	class base_export tDecompressor : public tUncopyable
	{
	    z_stream	mStream;
		b32			mDecompBegan;
		alloc_func	mAllocFunc;
		free_func	mFreeFunc;

	public:

		tDecompressor( b32 begin=true, alloc_func allocFunc=0, free_func freeFunc=0 );
		~tDecompressor( );

		void	fBeginDecompress( );
		void	fEndDecompress( );
		u32		fDecompress( const byte* src, u32 srcBytes, byte* dst, u32 dstBytes );
	};

}


#endif//__tDecompressor__
