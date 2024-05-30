#include "BasePch.hpp"
#if defined( platform_ios )
#include "tCriticalSection.hpp"

namespace Sig { namespace Threads
{
	tCriticalSection::tCriticalSection( )
	{
		pthread_mutex_init( &mPlatformCritSec, NULL );
	}

	tCriticalSection::~tCriticalSection( )
	{
		pthread_mutex_destroy( &mPlatformCritSec );
	}

}}
#endif//#if defined( platform_ios )
