//------------------------------------------------------------------------------
// \file tSymbolHelper.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tSymbolHelper.hpp"

namespace Sig
{
#if defined( sig_use_symbolhelper )

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fLoadSymbolsForModule( const tModule & module )
	{
		return fLoadSymbolsForModule( 
			module.mName.c_str( ),
			module.mBaseAddress, 
			module.mSize, 
			module.mTimeStamp, 
			module.mSymbolsSignature );
	}
#endif //defined( sig_use_symbolhelper )
}
