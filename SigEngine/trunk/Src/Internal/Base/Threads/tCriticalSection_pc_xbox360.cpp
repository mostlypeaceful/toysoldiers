#include "BasePch.hpp"
#if defined( platform_msft )
#include "tCriticalSection.hpp"

namespace Sig { namespace Threads
{
	tCriticalSection::tCriticalSection( )
	{
#if defined( platform_metro )
#if defined( build_release )
		const DWORD flags = 0; // CRITICAL_SECTION_NO_DEBUG_INFO; // undefined? wut?
#else
		const DWORD flags = 0;
#endif
		InitializeCriticalSectionEx( &mPlatformCritSec, /* spin count, arbitarily chosen */ 10, flags );
#else
		InitializeCriticalSection( &mPlatformCritSec );
#endif
	}

	tCriticalSection::~tCriticalSection( )
	{
		DeleteCriticalSection( &mPlatformCritSec );
	}

}}
#endif//#if defined( platform_msft )
