#include "BasePch.hpp"
#if defined( platform_ios )
#include "tMutex.hpp"

namespace Sig { namespace Threads
{

	tMutex::tMutex( tCriticalSection& critical )
		: mCritical( critical )
	{
		pthread_mutex_lock( &mCritical.mPlatformCritSec );
	}

	tMutex::~tMutex( )
	{
		pthread_mutex_unlock( &mCritical.mPlatformCritSec );
	}

}}
#endif//#if defined( platform_ios )
