//------------------------------------------------------------------------------
// \file tSymbolHelper_xbox360.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )

#include "tSymbolHelper.hpp"

#ifndef build_release
	#include <xbdm.h>
#endif

namespace Sig
{

	//------------------------------------------------------------------------------
	tSymbolHelper::tSymbolHelper( const char * symbolSearchPath )
	{
		log_warning( "Symbols not supported on Xbox 360" );
	}

	//------------------------------------------------------------------------------
	tSymbolHelper::~tSymbolHelper( )
	{

	}

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fLoadSymbolsForModule( 
		const char * moduleName,
		void * baseAddress,
		u32 size,
		u32 timeStamp,
		const tSymbolsSignature & signature )
	{
		log_warning( "Symbols not supported on Xbox 360" );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fGetSymbolSummary( 
		void * address, std::string & symbol, std::string & file )
	{
		log_warning( "Symbols not supported on Xbox 360" );
		return false;
	}
}

#endif // #if defined( platform_xbox360 )