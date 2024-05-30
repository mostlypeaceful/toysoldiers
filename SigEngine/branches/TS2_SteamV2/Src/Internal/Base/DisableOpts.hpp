#ifndef __DisableOpts__
#define __DisableOpts__

#if 0 // if you want to disable optimizations, change this to #if 1 locally, but don't submit

#	if defined( platform_pcdx9 ) || defined( platform_pcdx10 ) || defined( platform_xbox360 )
#		pragma optimize( "", off )
#	endif//MS platforms

#endif//#if 0

#endif//__DisableOpts__
