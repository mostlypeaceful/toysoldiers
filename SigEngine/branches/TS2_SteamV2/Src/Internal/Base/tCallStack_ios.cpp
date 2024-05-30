//------------------------------------------------------------------------------
// \file tCallStack_ios.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_ios )

#include "tCallStack.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	void tCallStack::fCaptureCallStackForPlatform( )
	{
		log_warning_unimplemented( 0 );
		mAddresses.fFill( NULL );
	}
}

#endif // #if defined( platform_ios )