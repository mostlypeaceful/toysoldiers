//------------------------------------------------------------------------------
// \file tDebugger.hpp - 9 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tDebugger__
#define __tDebugger__

namespace Sig { namespace Debug
{
	/// \class	tDebugger
	/// \brief	Expose various functionality for interacting with an attached debugger.
	class base_export tDebugger
	{
	public:
		/// \brief	Does the current platform & configuration allow the debugger to be detected?
		static b32 fCanBeDetected( );

		/// \brief	Returns true if anything has been successfully detected "attached" to the current program.
		///			This should include non-interactive debuggers such as SDL MiniFuzz.
		///			This may obviously return false if one is attached but undetected or undetectable in the current config.
		static b32 fIsDebuggerAttached( );

		/// \brief	Is an "interactive" debugger attached?  Unlike fIsDebuggerAttached, this could exclude
		///			"debuggers" such as SDL MiniFuzz running the code automatically overnight collecting info
		///			without human intervention.
		static b32 fIsInteractiveDebuggerAttached( );

		/// \brief	Should input-conditional breaks be triggered?
		static b32 fIsBreakHeld( );
	};
}}

/// \brief	Set a permanent breakpoint for debuggers.
#if defined( platform_msft )
#	define debug_break( ) __debugbreak( )
#else
#	define debug_break( ) raise(SIGTRAP)
#endif

/// \brief	Break here if a debugger is attached.
#define break_if_debugger( ) if( ::Sig::Debug::tDebugger::fIsDebuggerAttached( ) ) debug_break( ); else ((void)0)

/// \brief	Break here if Ctrl+Pause (Break) is held.
#define keyboard_debug_break( ) if( ::Sig::Debug::tDebugger::fIsBreakHeld( ) ) debug_break( ); else ((void)0)


#endif //ndef __tDebugger__
