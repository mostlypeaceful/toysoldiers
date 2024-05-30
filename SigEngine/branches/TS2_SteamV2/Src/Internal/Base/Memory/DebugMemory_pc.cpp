//------------------------------------------------------------------------------
// \file DebugMemory_pc.cpp - 22 May 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "DebugMemory.hpp"

namespace Sig { namespace Memory 
{

	//------------------------------------------------------------------------------
	b32 fDebugMemEnabled( )
	{
		return false;
	}

	//------------------------------------------------------------------------------
	u32 fDebugMemGetDebugMemorySize( )
	{
		return 0;
	}

	//------------------------------------------------------------------------------
	void * fDebugMemAlloc( u32 size )
	{
		sigassert( 0 && "Not supported" );
		return NULL;
	}

	//------------------------------------------------------------------------------
	void fDebugMemFree( void * mem )
	{

	}

} }
