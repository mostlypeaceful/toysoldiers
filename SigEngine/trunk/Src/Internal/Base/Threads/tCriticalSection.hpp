#ifndef __tCriticalSection__
#define __tCriticalSection__

namespace Sig { namespace Threads
{
	class tMutex;

	class base_export tCriticalSection
	{
		declare_uncopyable( tCriticalSection );
		friend class tMutex;
	private:

#if defined( platform_msft )
		CRITICAL_SECTION mPlatformCritSec;
#elif defined( platform_ios )
		pthread_mutex_t mPlatformCritSec;
#endif// win32 and xbox360 platforms

	public:
		tCriticalSection( );
		~tCriticalSection( );
	};


}}


#endif//__tCriticalSection__
