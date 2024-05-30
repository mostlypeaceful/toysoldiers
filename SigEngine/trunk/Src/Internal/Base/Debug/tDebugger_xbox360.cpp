//------------------------------------------------------------------------------
// \file tDebugger_xbox360.cpp - 9 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "Debug/tDebugger.hpp"
#include <xbdm.h>

namespace Sig { namespace Debug
{
	b32 tDebugger::fCanBeDetected( )
	{
#if !defined( build_release )
		return true;
#else
		return false; // DmIsDebuggerPresent not present in release XDK
#endif
	}

	b32 tDebugger::fIsDebuggerAttached( )
	{
#if !defined( build_release )
		return (b32)DmIsDebuggerPresent( );
#else
		return false; // DmIsDebuggerPresent not present in release XDK
#endif
	}

	b32 tDebugger::fIsInteractiveDebuggerAttached( )
	{
		return fIsDebuggerAttached( );
	}
}}

#endif
