//------------------------------------------------------------------------------
// \file LocalStorage_pc_xbox360.cpp - 13 Apr 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_metro )
#include "LocalStorage.hpp"

namespace Sig { namespace Threads { namespace LocalStorage
{
	//------------------------------------------------------------------------------
	tLocalStorageHandle Allocate( )
	{
		DWORD handle = FlsAlloc( NULL );
		//sigassert( handle != FLS_OUT_OF_INDEXES );
		sigassert( handle != 0xFFFFFFFF );
		return handle;
	}

	//------------------------------------------------------------------------------
	b32 Free( tLocalStorageHandle index )
	{
		return ( FlsFree( index ) == TRUE );
	}

	//------------------------------------------------------------------------------
	b32 SetValue( tLocalStorageHandle index, void * value )
	{
		return ( FlsSetValue( index, value ) == TRUE );
	}

	//------------------------------------------------------------------------------
	void * GetValue( tLocalStorageHandle storage )
	{
		return FlsGetValue( storage );
	}

} } }
#endif // defined( platform_msft )
