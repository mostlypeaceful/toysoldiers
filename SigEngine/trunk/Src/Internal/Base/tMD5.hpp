//------------------------------------------------------------------------------
// \file tMD5.hpp - 12 Apr 2013
// \author colins
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tMD5__
#define __tMD5__

namespace Sig
{
	typedef base_export tFixedArray< u8, 16 > tMD5Hash;

	class base_export tMD5
	{
	public:

		static void fComputeHash( void * data, u32 dataLength, tMD5Hash & out );
		static void fComputeHash( const tFilePathPtr & filePath, tMD5Hash & out );
	};

} // ::Sig

#endif//__tMD5__
