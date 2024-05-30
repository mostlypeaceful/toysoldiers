//------------------------------------------------------------------------------
// \file tTtfFile.cpp - 22 Jan 2013
// \author jwittner
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tTtfFile.hpp"

namespace Sig
{
	b32 tTtfFile::fIsOperablePath( const tFilePathPtr& path )
	{
		if( StringUtil::fCheckExtension( path.fCStr( ), ".ttf" ) )
			return true;

		if( StringUtil::fCheckExtension( path.fCStr( ), ".otf" ) )
			return true;

		return false;
	}
}
