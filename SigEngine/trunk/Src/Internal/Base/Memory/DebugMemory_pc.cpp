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
		return true;
	}

	//------------------------------------------------------------------------------
	u32 fDebugMemGetDebugMemorySize( )
	{
		return std::numeric_limits<u32>::max( );
	}

	//------------------------------------------------------------------------------
	u32 fDebugMemGetAdditionalTitleMemorySize( )
	{
		return std::numeric_limits<u32>::max( );
	}

	//------------------------------------------------------------------------------
	void * fDebugMemAlloc( u32 size )
	{
		return tMallocHeap::fInstance( ).fAlloc( size, tAllocStamp::fNoContextStamp( ) );
	}

	//------------------------------------------------------------------------------
	void fDebugMemFree( void * mem )
	{
		tMallocHeap::fInstance( ).fFree( mem );
	}

} }
