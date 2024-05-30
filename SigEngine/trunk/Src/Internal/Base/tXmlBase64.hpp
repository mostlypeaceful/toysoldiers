//------------------------------------------------------------------------------
// \file tXmlBase64.hpp - 17 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tXmlBase64__
#define __tXmlBase64__
#include "tBase64.hpp"

namespace Sig
{
	///
	/// \class tXmlBase64DynamicArraySerializer
	/// \brief 
	template<class t>
	class tools_export tXmlBase64DynamicArraySerializer
	{
	public:

		tXmlBase64DynamicArraySerializer( tDynamicArray<t> & data ) : mData( data ) { }

		template<class tSerializer>
		void fSerializeXml( tSerializer& s )
		{
			if( s.fIn( ) )
			{
				std::string inEncoded;
				s( fTitle( ), inEncoded );

				u32 numData = 0;
				s( fNumTitle( ), numData );

				tGrowableArray<Sig::byte> decoded;
				tBase64::fDecode( inEncoded.c_str( ), inEncoded.length( ), decoded );

				mData.fNewArray( numData );
				sigassert( decoded.fCount( ) == mData.fTotalSizeOf( ) );

				fMemCpy( mData.fBegin( ), decoded.fBegin( ), decoded.fCount( ) );
			}
			else
			{
				std::string outEncoded;
				tBase64::fEncode( (const Sig::byte*)mData.fBegin( ), mData.fTotalSizeOf( ), outEncoded );
				s( fTitle( ), outEncoded );

				u32 numData = mData.fCount( );
				s( fNumTitle( ), numData );
			}
		}

	private:

		const char * fTitle( ) const { return "EncodedData"; }
		const char * fNumTitle( ) const { return "DecodedCount"; }

		tDynamicArray<t> & mData;
	};
}

#endif//__tXmlBase64__
