#ifndef __tEncryption__
#define __tEncryption__

namespace Sig
{
	class base_export tEncryption
	{
	public:
		static void fEncrypt( const tGrowableArray<byte>& src, tGrowableArray<byte>& output );
		static void fDecrypt( const tGrowableArray<byte>& src, tGrowableArray<byte>& output );

		static void fTest( );
	};

}


#endif//__tEncryption__
