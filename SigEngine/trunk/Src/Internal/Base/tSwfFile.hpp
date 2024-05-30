#ifndef __tSwfFile__
#define __tSwfFile__
#include "tByteFile.hpp"

namespace Sig
{
	/*
		Warning if you add any other data here, you should not be dependent on tByteFile, and should become your own unique resource.
	*/
	class base_export tSwfFile : public tByteFile
	{
		declare_reflector( );
		implement_rtti_serializable_base_class(tSwfFile, 0x65FDEE83);

	public:
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathAddB( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathSubB( path ); }

	public:
		tSwfFile( ) { }
		tSwfFile( tNoOpTag ) : tByteFile( cNoOpTag ) { }
	};

}

#endif //__tSwfFile__
