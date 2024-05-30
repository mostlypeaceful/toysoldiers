//------------------------------------------------------------------------------
// \file tSymbolHelper_ios.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_ios )
#include "tSymbolHelper.hpp"
#if defined( sig_use_symbolhelper )

namespace Sig
{
	//------------------------------------------------------------------------------
	tSymbolHelper::tSymbolHelper( const char * symbolSearchPath, b32 currentProcess )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	tSymbolHelper::~tSymbolHelper( )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fLoadSymbolsForModule( 
		const char * moduleName,
		u64 baseAddress,
		u32 size,
		u32 timeStamp,
		const tSymbolsSignature & signature )
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fGetSymbolSummary( 
		u64 address, std::string & symbol, std::string & file )
	{
		log_warning_unimplemented( 0 );
		return false;
	}
}

#endif // #if defined( sig_use_symbolhelper )
#endif // #if defined( platform_ios )
