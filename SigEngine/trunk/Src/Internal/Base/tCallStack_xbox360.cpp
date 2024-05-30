//------------------------------------------------------------------------------
// \file tCallStack_xbox360.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )

#include "tCallStack.hpp"

#ifndef build_release 
	#include <xbdm.h>
#endif

namespace Sig
{
	//------------------------------------------------------------------------------
	void tCallStack::fCaptureCallStackForPlatform( )
	{
#ifndef build_release

		HRESULT error = DmCaptureStackBackTrace( mData.mDepth, mData.mAddresses.fBegin( ) );

		// Error
		if( error != XBDM_NOERR )
		{
			mData.mDepth = 0;
			return;
		}

		// Count how many were found
		u32 count = 0;
		for( ; count < mData.mDepth && mData.mAddresses[ count ]; ++count );

		// Reset the depth to the final count
		mData.mDepth = count;
#else
		mData.mDepth = 0;
#endif
	}
}

#endif // #if defined( platform_xbox360 )