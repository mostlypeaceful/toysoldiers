//------------------------------------------------------------------------------
// \file LocalStorage.hpp - 13 Apr 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __LocalStorage__
#define __LocalStorage__

#if defined( platform_ios )
#include <pthread.h>
#endif // defined( platform_ios )

namespace Sig { namespace Threads { namespace LocalStorage
{

#if defined( platform_msft )
	typedef u32 tLocalStorageHandle;
#elif defined( platform_ios )
	typedef pthread_key_t tLocalStorageHandle;
#else
	#error Unknown platform
#endif

	extern const LocalStorage::tLocalStorageHandle cInvalidHandle;

	tLocalStorageHandle Allocate( );
	b32 Free( tLocalStorageHandle index );
	b32 SetValue( tLocalStorageHandle index, void * value );
	void * GetValue( tLocalStorageHandle index );

} } }

#endif//__LocalStorage__
