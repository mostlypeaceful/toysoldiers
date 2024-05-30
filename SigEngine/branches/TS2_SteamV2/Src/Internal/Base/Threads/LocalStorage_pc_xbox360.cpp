//------------------------------------------------------------------------------
// \file LocalStorage_pc_xbox360.cpp - 13 Apr 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "LocalStorage.hpp"

namespace Sig { namespace Threads { namespace LocalStorage
{
	extern const u32 cOutOfIndices = TLS_OUT_OF_INDEXES;

	//------------------------------------------------------------------------------
	u32 Allocate( )
	{
		return TlsAlloc( );
	}

	//------------------------------------------------------------------------------
	b32 Free( u32 index )
	{
		return ( TlsFree( index ) == TRUE );
	}

	//------------------------------------------------------------------------------
	b32 SetValue( u32 index, void * value )
	{
		return ( TlsSetValue( index, value ) == TRUE );
	}

	//------------------------------------------------------------------------------
	void * GetValue( u32 storage )
	{
		return TlsGetValue( storage );
	}

} } }
