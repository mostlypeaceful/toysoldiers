//------------------------------------------------------------------------------
// \file tCallStack.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tCallStack.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tCallStack::tCallStack( u32 depth )
	{
		mData.mDepth = fMin( depth, tCallStackData::cMaxDepth );
		fCaptureCallStackForPlatform( );
	}
}
