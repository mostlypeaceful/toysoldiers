//------------------------------------------------------------------------------
// \file tDebugger_win32.cpp - 9 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#if defined( platform_msft ) && !defined( platform_xbox360 )
#include "Debug/tDebugger.hpp"

namespace Sig { namespace Debug
{
	b32 tDebugger::fCanBeDetected( )
	{
		return true;
	}

	b32 tDebugger::fIsDebuggerAttached( )
	{
		return (b32)IsDebuggerPresent( );
	}

	b32 tDebugger::fIsInteractiveDebuggerAttached( )
	{
		return fIsDebuggerAttached( );
	}
}}

#endif
