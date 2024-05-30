//------------------------------------------------------------------------------
// \file tCallStack_pc.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_pcdx ) || defined( platform_metro )
#include "BannedApiConfig.hpp"
#include "tCallStack.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	void tCallStack::fCaptureCallStackForPlatform( )
	{
#if !defined( platform_metro ) || defined( sig_use_banned_apis )
		mData.mDepth = CaptureStackBackTrace( 0, mData.mDepth, mData.mAddresses.fBegin( ), NULL );
#endif
	}
}

#endif // #if defined( platform_pcdx ) || defined( platform_metro )
