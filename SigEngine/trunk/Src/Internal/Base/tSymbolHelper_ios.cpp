//------------------------------------------------------------------------------
// \file tSymbolHelper_ios.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_ios )

#include "tSymbolHelper.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tSymbolHelper::tSymbolHelper( const char * symbolSearchPath )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	tSymbolHelper::~tSymbolHelper( )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fLoadSymbolsForModule( 
		const char * moduleName,
		void * baseAddress,
		u32 size,
		u32 timeStamp,
		const tSymbolsSignature & signature )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fGetSymbolSummary( 
		void * address, std::string & symbol, std::string & file )
	{
		log_warning_unimplemented( );
		return false;
	}
}

#endif // #if defined( platform_ios )
