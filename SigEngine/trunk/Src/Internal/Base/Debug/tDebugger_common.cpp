//------------------------------------------------------------------------------
// \file tDebugger_common.cpp - 9 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "Debug/tDebugger.hpp"
#include "Input/tKeyboard.hpp"

namespace Sig { namespace Debug
{
	b32 tDebugger::fIsBreakHeld( )
	{
		static ::Sig::Input::tKeyboard kb;
		kb.fCaptureState( );

		const b32 ctrl
			=  kb.fButtonHeld( ::Sig::Input::tKeyboard::cButtonLCtrl )
			|| kb.fButtonHeld( ::Sig::Input::tKeyboard::cButtonRCtrl )
			;

		const b32 pause
			= kb.fButtonHeld( ::Sig::Input::tKeyboard::cButtonPause )
			;

		return ctrl && pause;
	}
}}
