#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#ifndef __Win32Include__
#define __Win32Include__

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif//WIN32_LEAN_AND_MEAN

#ifdef _WIN32_WINNT
#	if _WIN32_WINNT < 0x06000000
#		undef 
#		define _WIN32_WINNT 0x06000000
#	endif//_WIN32_WINNT < 0x06000000
#else
#	define _WIN32_WINNT 0x06000000
#endif

// This is to allow our tools to support Windows Vista when being compiled with a newer Windows SDK.
// http://blogs.msdn.com/b/vcblog/archive/2009/08/27/windows-sdk-v7-0-v7-0a-incompatibility-workaround.aspx
#define PSAPI_VERSION 1

#include <windows.h>

#undef min
#undef max
#undef small
#undef big


#endif//__Win32Include__
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
