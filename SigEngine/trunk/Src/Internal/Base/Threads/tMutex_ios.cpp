#include "BasePch.hpp"
#if defined( platform_ios )
#include "tMutex.hpp"

namespace Sig { namespace Threads
{

	tMutex::tMutex( tCriticalSection& critical )
		: mCritical( critical )
	{
		int error = pthread_mutex_lock( &mCritical.mPlatformCritSec );
		sigassert(!error);
	}

	tMutex::~tMutex( )
	{
		int error = pthread_mutex_unlock( &mCritical.mPlatformCritSec );
		sigassert(!error);
	}

}}
#endif//#if defined( platform_ios )
