//------------------------------------------------------------------------------
// \file tTtfFile.hpp - 23 Jan 2013
// \author jwittner
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTtfFile__
#define __tTtfFile__

#include "tByteFile.hpp"

namespace Sig
{
	///
	/// \class tTtfFile
	/// \brief Wraps a ttf file in load in place semantics
	class base_export tTtfFile : public tByteFile
	{
		declare_reflector( );
		implement_rtti_serializable_base_class(tTtfFile, 0x7CE467F5);

	public:
		static b32 fIsOperablePath( const tFilePathPtr& path );
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathAddB( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathSubB( path ); }

	public:
		tTtfFile( ) { }
		tTtfFile( tNoOpTag ) : tByteFile( cNoOpTag ) { }
	};

}

#endif//__tTtfFile__
