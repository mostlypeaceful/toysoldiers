//------------------------------------------------------------------------------
// \file Http_pc.cpp - 17 Jul 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "Http.hpp"

namespace Sig { namespace Http 
{
	//------------------------------------------------------------------------------
	// tEndpoint
	//------------------------------------------------------------------------------
	void tEndpoint::fPlatformCtor( )
	{
	}

	void tEndpoint::fDestroy()
	{
		log_warning_unimplemented( );
	}

	b32 tEndpoint::fReady( )
	{
		log_warning_unimplemented( );
		return false;
	}

	b32 tEndpoint::fFailed( )
	{
		log_warning_unimplemented( );
		return true;
	}

	tRequestPtr tEndpoint::fOpenRequest( const char* verb, const char* path, const char* contentHeader, const char* data, u32 dataSize, u32 responseBufferSize )
	{
		log_warning_unimplemented( );
		return tRequestPtr( );
	}

	void tEndpoint::fCloseRequest()
	{
		log_warning_unimplemented( );
	}

	void tEndpoint::fTick( )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	// tRequest
	//------------------------------------------------------------------------------
	void tRequest::fPlatformCtor( )
	{
	}

	void tRequest::fClose( )
	{
		log_warning_unimplemented( );
	}

	void tRequest::fTick()
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	// tSystem
	//------------------------------------------------------------------------------
	const u32 tSystem::cStartupBypassSecurity = 0;

	void tSystem::fPlatformCtor( )
	{
	}

	b32 tSystem::fPlatformInitialize( )
	{
		log_warning_unimplemented( );
		return false;
	}

	void tSystem::fPlatformShutdown( )
	{
		log_warning_unimplemented( );
	}

	tEndpointPtr tSystem::fFindOrCreateEndpoint( const tStringPtr& url, b32 requireToken, u32 userIdx )
	{
		log_warning_unimplemented( );
		return tEndpointPtr( );
	}

} } // ::Sig::Http
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
