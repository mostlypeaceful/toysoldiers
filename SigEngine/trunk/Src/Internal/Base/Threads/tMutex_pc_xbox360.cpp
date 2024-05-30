#include "BasePch.hpp"
#if defined( platform_msft )
#include "tMutex.hpp"

namespace Sig { namespace Threads
{

	tMutex::tMutex( tCriticalSection& critical )
		: mCritical( critical )
	{
		EnterCriticalSection( &mCritical.mPlatformCritSec );
	}

	tMutex::~tMutex( )
	{
		LeaveCriticalSection( &mCritical.mPlatformCritSec );
	}

}}
#endif//#if defined( platform_msft )
