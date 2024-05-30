#include "BasePch.hpp"
#if defined( platform_ios )
#include "Debug/tDebugger.hpp"

namespace Sig { namespace Log
{
	void fFatalError( const char* lastTextUserWillSee )
	{
		if( lastTextUserWillSee )
			fPrintf( 0, "@ERROR@ %s@END@\n", ( lastTextUserWillSee ? lastTextUserWillSee : "" ) );
#ifdef sig_devmenu
		debug_break( );
#else//sig_devmenu
		*( int* )0 = 0;
#endif//sig_devmenu
	}

#ifdef sig_logging

	void fStandardOutputFunction( const char* text, u32 flag )
	{
		printf( "%s", text );
	}

#endif//sig_logging

}}
#endif//#if defined( platform_ios )

