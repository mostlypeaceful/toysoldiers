#include "BasePch.hpp"
#if defined( platform_ios )
#include "LocalStorage.hpp"
#include <pthread.h>

namespace Sig { namespace Threads { namespace LocalStorage
{
	//------------------------------------------------------------------------------
	tLocalStorageHandle Allocate( )
	{
		pthread_key_t key;
		int error = pthread_key_create(&key, NULL);
		sigassert(!error);
		return key;
	}

	//------------------------------------------------------------------------------
	b32 Free( tLocalStorageHandle index )
	{
		int error = pthread_key_delete( index );
		return !error;
	}

	//------------------------------------------------------------------------------
	b32 SetValue( tLocalStorageHandle index, void * value )
	{
		int error = pthread_setspecific( index, value );
		return !error;
	}

	//------------------------------------------------------------------------------
	void * GetValue( tLocalStorageHandle index )
	{
		return pthread_getspecific( index );
	}
} } }
#endif // defined( platform_ios )
