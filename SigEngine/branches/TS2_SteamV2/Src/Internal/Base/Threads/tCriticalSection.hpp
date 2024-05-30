#ifndef __tCriticalSection__
#define __tCriticalSection__

namespace Sig { namespace Threads
{
	class tMutex;

	class base_export tCriticalSection : tUncopyable
	{
		friend class tMutex;
	private:

#if defined( platform_pcdx9 ) || defined( platform_pcdx10 ) || defined( platform_xbox360 )
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
