#ifndef __tBase64__
#define __tBase64__

namespace Sig
{
	class base_export tBase64
	{
	public:

		///
		/// \brief Encode a binary sequence into a base-64, text-safe string (suitable for serializing in an xml file for instance).
		static void fEncode( const Sig::byte* toEncode, u32 toEncodeLength, std::string& encodedOutput );

		///
		/// \brief Decode a character string (convert back to binary). Assumes all the characters in 'toDecode' were
		/// encoded using fEncode.
		static void fDecode( const char* toDecode, u32 toDecodeLength, tGrowableArray<Sig::byte>& decodedOutput );

	private:
		static char fEncode( Sig::byte uc );
		static Sig::byte fDecode( char c );
		static b32 fIsBase64( char c );
	};
}

#endif//__tBase64__