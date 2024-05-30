#ifndef __tCompressor__
#define __tCompressor__
#include "zlib.h"

namespace Sig
{

	class base_export tCompressor : public tUncopyable
	{
	    z_stream mStream;

	public:

		static const u32 cCompressionOverhead = 64;

		u32 fCompress( const byte* src, u32 srcBytes, byte* dst );

	};

}


#endif//__tCompressor__

