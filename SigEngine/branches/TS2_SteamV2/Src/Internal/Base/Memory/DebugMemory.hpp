//------------------------------------------------------------------------------
// \file DebugMemory.hpp - 22 May 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __DebugMemory__
#define __DebugMemory__

namespace Sig { namespace Memory 
{

	b32 fDebugMemEnabled( );
	u32 fDebugMemGetDebugMemorySize( );

	void * fDebugMemAlloc( u32 size );
	void fDebugMemFree( void * mem );

}} // namespace

#endif//__DebugMemory__
