//------------------------------------------------------------------------------
// \file tInterlocked.cpp - 22 Apr 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "Threads/tInterlocked.hpp"
#include "Threads/tCriticalSection.hpp"

namespace Sig { namespace Threads
{

#if defined( sig_interlocked_fake_atomics )
	// Warning so we don't accidentally leave this on
#	pragma message("Threads\\tInterlocked.hpp: WARNING: sig_interlocked_fake_atomics enabled, big 'ole global mutex slowing down absolutely everything.")

	tCriticalSection& base_export fAtomicFakingCriticalSection( )
	{
		static tCriticalSection cs;
		return cs;
	}

#else
	void fDontComplainAboutNoSymbols( )
	{
	}

#endif

}}
