#include <BasePch.hpp>
//------------------------------------------------------------------------------
// \file tPixProfiler.cpp - 07 Sep 2010
// \author cbramwell
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#if defined( sig_profile ) && defined( platform_xbox360 )

#include <pix.h>

tScopedPixProfile::tScopedPixProfile( const char* name )
{
	PIXBeginNamedEvent( D3DCOLOR_XRGB( 0, 0, 0 ), name );
}
tScopedPixProfile::~tScopedPixProfile( )
{
	PIXEndNamedEvent( );
}

#else

// Fixes no object linker warning
void tPixProfileCPP_NoObjFix( ) { }

#endif//#if defined( sig_profile ) && defined( platform_xbox360 )
