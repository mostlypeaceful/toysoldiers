//------------------------------------------------------------------------------
// \file tPixProfiler.hpp - 07 Sep 2010
// \author cbramwell
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tPixProfiler__
#define __tPixProfiler__

#if defined( sig_profile ) && defined( platform_xbox360 )

	struct tScopedPixProfile
	{
		explicit tScopedPixProfile( const char* name );
		~tScopedPixProfile( );
	};
#	define profile_pix(name) tScopedPixProfile _auto_profiler_pix_object_(name)
#   define begin_pix(name) PIXBeginNamedEvent(0,name)
#	define end_pix() PIXEndNamedEvent()

#else

#	define profile_pix(name)
#	define begin_pix(name)
#	define end_pix()

#endif//defined( sig_profile ) && defined( platform_xbox360 )

#endif//__tPixProfiler__
