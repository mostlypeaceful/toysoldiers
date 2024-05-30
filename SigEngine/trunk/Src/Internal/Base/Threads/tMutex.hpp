#ifndef __tMutex__
#define __tMutex__
#include "tCriticalSection.hpp"

namespace Sig { namespace Threads
{

	class base_export tMutex
	{
	public:
		typedef Threads::tCriticalSection tCriticalSection;
	private:
		tCriticalSection& mCritical;
	public:
		tMutex( tCriticalSection& critical );
		~tMutex( );
	};

	class base_export tDummyMutex
	{
	public:
		struct tCriticalSection { };
	public:
		inline tDummyMutex( tCriticalSection& critical ) { }
	};

}}


#endif//__tMutex__
