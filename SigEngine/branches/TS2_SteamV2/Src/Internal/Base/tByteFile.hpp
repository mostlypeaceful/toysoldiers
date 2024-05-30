#ifndef __tByteFile__
#define __tByteFile__
#include "tLoadInPlaceFileBase.hpp"

namespace Sig
{
	/*
		tByteFile is meant to be a reusable mechanism for loading plain byte data into the game.
		 Use this as a quick starter when no platform specific processing is required.

		Useful for:
			Iggy:	.swf files are processed entirely by the Iggy engine.
			xml:	pass the buffer to a tXMLFile to read the data.
			txt:	easily pass byte wide string information such as ini or other simple config data.
	*/

	class base_export tByteFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		//implement_rtti_serializable_base_class(tByteFile, "UserMustSupplyID");
	public:
		tByteFile( );
		tByteFile( tNoOpTag );

		const tDynamicBuffer& fData( ) const { return mData; }

	protected:
		tDynamicBuffer mData;
	};

	template<>
	class tResourceConvertPath<tByteFile>
	{
	public:
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathAddB( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathSubB( path ); }
	};
}

#endif//__tByteFile__
