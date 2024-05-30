//------------------------------------------------------------------------------
// \file DebugMemory_xbox360.cpp - 22 May 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "DebugMemory.hpp"

#ifndef build_release
	#include <xbdm.h>
#endif

namespace Sig { namespace Memory 
{

#ifndef build_release

	//------------------------------------------------------------------------------
	b32 fDebugMemEnabled( )
	{
		DWORD config;
		HRESULT error = DmGetConsoleDebugMemoryStatus( &config );

		if( error != XBDM_NOERR )
		{
			log_warning( 0, "DmGetConsoleDebugMemoryStatus returned error: " << error );
			return false;
		}

		return config == DM_CONSOLEMEMCONFIG_ADDITIONALMEMENABLED;
	}

	//------------------------------------------------------------------------------
	u32 fDebugMemGetDebugMemorySize( )
	{
		DWORD size;
		HRESULT error = DmGetDebugMemorySize( &size );
		
		if( error != XBDM_NOERR )
		{
			log_warning( 0, "DmGetDebugMemorySize returned error: " << error );
			return 0;
		}

		return size;
	}

	//------------------------------------------------------------------------------
	void * fDebugMemAlloc( u32 size )
	{
		void * mem = DmDebugAlloc( size );
		sigassert( mem && "Out of debug memory" );
		return mem;
	}

	//------------------------------------------------------------------------------
	void fDebugMemFree( void * mem )
	{
		BOOL result = DmDebugFree( mem );
		sigassert( result );
	}

#else

	b32 fDebugMemEnabled( )
	{
		return false;
	}

	u32 fDebugMemGetDebugMemorySize( )
	{
		return 0;
	}

	void * fDebugMemAlloc( u32 /*size*/ )
	{
		return NULL;
	}

	void fDebugMemFree( void * /*mem*/ )
	{

	}

#endif //build_release


} }
