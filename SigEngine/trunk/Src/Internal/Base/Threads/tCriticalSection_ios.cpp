#include "BasePch.hpp"
#if defined( platform_ios )
#include "tCriticalSection.hpp"
#include <sstream>

namespace Sig { namespace Threads
{
	tCriticalSection::tCriticalSection( )
	{
		sigassert( ((size_t)this)%4 == 0 && "Critical section misaligned!  Trying to lock the resulting pthread mutex will cause EXC_BAD_ACCESS on the iPad!" );
		// From observation, a 4 byte alignment appears to be sufficient... for now.
		
		pthread_mutexattr_t attr = {0};
		int error = pthread_mutexattr_init(&attr);
		error = error || pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
		error = error || pthread_mutex_init( &mPlatformCritSec, &attr );
		error = error || pthread_mutexattr_destroy(&attr);
		sigassert(!error);
	}

	tCriticalSection::~tCriticalSection( )
	{
		int error = pthread_mutex_destroy( &mPlatformCritSec );
		sigassert(!error);
	}

}}
#endif//#if defined( platform_ios )
