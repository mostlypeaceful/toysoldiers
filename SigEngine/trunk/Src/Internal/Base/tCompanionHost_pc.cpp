//------------------------------------------------------------------------------
// \file tCompanionHost_pc.cpp - 01 Oct 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#ifdef platform_msft
#include "tCompanionHost.hpp"

namespace Sig
{
	b32 tCompanionHost::fInitialize( )
	{
		log_warning_unimplemented( );
		return false;
	}

	void tCompanionHost::fShutdown( )
	{
		log_warning_unimplemented( );
	}

	void tCompanionHost::fTick( )
	{
		log_warning_unimplemented( );
	}

	void tCompanionHost::fSendJSON( tClientId clientId, const tJsonWriterPtr& writerPtr )
	{
		log_warning_unimplemented( );
	}

	void tCompanionHost::fSendJSONInternal( )
	{
		log_warning_unimplemented( );
	}

} // ::Sig

#endif
