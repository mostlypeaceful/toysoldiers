//------------------------------------------------------------------------------
// \file LocalStorage_pc_xbox360.cpp - 13 Apr 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_msft ) && !defined( platform_metro )
#include "LocalStorage.hpp"

namespace Sig { namespace Threads { namespace LocalStorage
{

	const LocalStorage::tLocalStorageHandle cInvalidHandle = TLS_OUT_OF_INDEXES;

	//------------------------------------------------------------------------------
	tLocalStorageHandle Allocate( )
	{
		DWORD handle = TlsAlloc( );
		sigassert( handle != TLS_OUT_OF_INDEXES );
		return handle;
	}

	//------------------------------------------------------------------------------
	b32 Free( tLocalStorageHandle index )
	{
		return ( TlsFree( index ) == TRUE );
	}

	//------------------------------------------------------------------------------
	b32 SetValue( tLocalStorageHandle index, void * value )
	{
		return ( TlsSetValue( index, value ) == TRUE );
	}

	//------------------------------------------------------------------------------
	void * GetValue( tLocalStorageHandle storage )
	{
		return TlsGetValue( storage );
	}

} } }
#endif // defined( platform_msft )
