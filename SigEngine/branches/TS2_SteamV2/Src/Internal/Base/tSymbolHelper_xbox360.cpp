//------------------------------------------------------------------------------
// \file tSymbolHelper_xbox360.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tSymbolHelper.hpp"
#if defined( sig_use_symbolhelper )

#include <xbdm.h>

namespace Sig
{

	//------------------------------------------------------------------------------
	tSymbolHelper::tSymbolHelper( const char * symbolSearchPath, b32 currentProcess )
	{
		log_warning( 0, "Symbols not supported on Xbox 360" );
	}

	//------------------------------------------------------------------------------
	tSymbolHelper::~tSymbolHelper( )
	{

	}

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fLoadSymbolsForModule( 
		const char * moduleName,
		u64 baseAddress,
		u32 size,
		u32 timeStamp,
		const tSymbolsSignature & signature )
	{
		log_warning( 0, "Symbols not supported on Xbox 360" );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fGetSymbolSummary( 
		u64 address, std::string & symbol, std::string & file )
	{
		log_warning( 0, "Symbols not supported on Xbox 360" );
		return false;
	}
}

#endif // #if defined( sig_use_symbolhelper )
#endif // #if defined( platform_xbox360 )