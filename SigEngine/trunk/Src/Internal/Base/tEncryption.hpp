#ifndef __tEncryption__
#define __tEncryption__

namespace Sig
{
	class base_export tEncryption
	{
	public:
		static void fEncrypt( const tGrowableArray<byte>& src, tGrowableArray<byte>& output );
		static void fDecrypt( const tGrowableArray<byte>& src, tGrowableArray<byte>& output );

		// Pass in zero to sum to start with, and some data to be checksumed
		//  The result is the checksum to store with the file.
		// To validate, pass the checksum to sum, and you should receive zero if data is preserved.
		static u8 fCheckSum( const byte* src, u32 srcBytes, u8 sum );

		static void fTest( );
	};

}


#endif//__tEncryption__
