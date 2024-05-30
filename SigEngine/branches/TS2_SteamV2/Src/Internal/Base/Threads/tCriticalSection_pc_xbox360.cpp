#include "BasePch.hpp"
#if defined( platform_msft )
#include "tCriticalSection.hpp"

namespace Sig { namespace Threads
{
	tCriticalSection::tCriticalSection( )
	{
		InitializeCriticalSection( &mPlatformCritSec );
	}

	tCriticalSection::~tCriticalSection( )
	{
		DeleteCriticalSection( &mPlatformCritSec );
	}

}}
#endif//#if defined( platform_msft )
