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
	// Based on my knowledge of C++, this shouldn't be necessary.
	// Yet for some reason GCC for iOS is erroring out with "undefined symbol" when trying to link.
	// Given that MSVC actually complains if we define this... we're probably working around a compiler bug.
	// So it goes here, in the platform specific implementation.
	const u32 tCallStackData::cMaxDepth;

	//------------------------------------------------------------------------------
	void tCallStack::fCaptureCallStackForPlatform( )
	{
		log_warning_once( "tCallStack::fCaptureCallStackForPlatform not implemented for iOS" );
		mData.mAddresses.fFill( NULL );
	}
}

#endif // #if defined( platform_ios )
