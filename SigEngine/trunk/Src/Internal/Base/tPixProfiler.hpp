//------------------------------------------------------------------------------
// \file tPixProfiler.hpp - 07 Sep 2010
// \author cbramwell
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tPixProfiler__
#define __tPixProfiler__

namespace Sig { namespace PixProfiler
{

#if defined( sig_profile ) && defined( platform_xbox360 )

	struct tScopedPixProfile
	{
		explicit tScopedPixProfile( const char* name );
		~tScopedPixProfile( );
	};
#	define profile_pix(name) ::Sig::PixProfiler::tScopedPixProfile _auto_profiler_pix_object_(name)
#	define profile_pix_textures

#else

#	define profile_pix(name)

#endif//defined( sig_profile ) && defined( platform_xbox360 )

	base_export void fExportFuiInterface( );

}} // namespace Sig::PixProfiler

#endif//__tPixProfiler__
