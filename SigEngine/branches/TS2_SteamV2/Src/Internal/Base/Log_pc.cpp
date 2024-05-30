#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )

namespace Sig { namespace Log
{
	void fFatalError( const char* lastTextUserWillSee )
	{
		if( lastTextUserWillSee )
		{
			fPrintf( 0, "@ERROR@ %s@END@\n", ( lastTextUserWillSee ? lastTextUserWillSee : "" ) );
			MessageBox( 0, lastTextUserWillSee, "Fatal Error", MB_OK|MB_APPLMODAL|MB_ICONEXCLAMATION );
		}
#ifdef sig_devmenu
		__debugbreak( );
#else//sig_devmenu
		*( int* )0 = 0;
#endif//sig_devmenu
	}

#ifdef sig_logging

	void fStandardOutputFunction( const char* text, u32 flag )
	{
		OutputDebugString( text );
	}

#endif//sig_logging

}}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
