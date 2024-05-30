//------------------------------------------------------------------------------
// \file tCallStack_pc.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )

#include "tCallStack.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	void tCallStack::fCaptureCallStackForPlatform( )
	{
		mData.mDepth = CaptureStackBackTrace( 0, mData.mDepth, mData.mAddresses.fBegin( ), NULL );
	}
}

#endif // #if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
