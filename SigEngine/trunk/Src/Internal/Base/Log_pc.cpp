#include "BasePch.hpp"
#if defined( platform_pcdx )

namespace Sig { namespace Log
{
	void fFatalError( const char* lastTextUserWillSee )
	{
		fPrintf( 0, "@ERROR@ %s@END@\n", ( lastTextUserWillSee ? lastTextUserWillSee : "" ) );

		if( Log::tAssert::fShouldShowDialog() && lastTextUserWillSee )
		{
#if defined(target_game) && defined(build_release)
			MessageBox( 0, lastTextUserWillSee, "Fatal Error", MB_OK|MB_APPLMODAL|MB_ICONEXCLAMATION );
#else
			std::stringstream ss;
			ss << lastTextUserWillSee << "\nCrash?";
			if( MessageBox( 0, ss.str().c_str(), "Fatalish Error", MB_YESNO|MB_APPLMODAL|MB_ICONEXCLAMATION ) == IDNO )
				return; // don't actually crash
#endif
		}

		if( Log::tAssert::fShouldSetBreakpoint() )
			__debugbreak( );

		if( Log::tAssert::fShouldCrashHard() )
			*( int* )0 = 0;

		if( !Log::tAssert::fShouldAllowContinue() )
			abort();
	}

#ifdef sig_logging

	void fStandardOutputFunction( const char* text, u32 flag )
	{
		OutputDebugString( text );
	}

#endif//sig_logging

}}
#endif//#if defined( platform_pcdx )
