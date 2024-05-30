//------------------------------------------------------------------------------
// \file tPixProfiler.cpp - 07 Sep 2010
// \author cbramwell
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "Fui.hpp"

#if defined( sig_profile ) && defined( platform_xbox360 )
#define sig_use_pix
#endif

#ifdef sig_use_pix

#include <pix.h>

namespace Sig { namespace PixProfiler
{
	tScopedPixProfile::tScopedPixProfile( const char* name )
	{
		PIXBeginNamedEvent( 0, name );
	}

	tScopedPixProfile::~tScopedPixProfile( )
	{
		PIXEndNamedEvent( );
	}
}} // namespace Sig::PixProfiler

#endif // sig_use_pix

namespace Sig { namespace PixProfiler
{
	namespace
	{
		void fScopedPixProfilePush( Sig::Fui::tFuiFuncParams& params )
		{
#ifdef sig_use_pix
			const char* message = NULL;
			sigassert( params.fCount( ) == 1 );
			params.fGet( message );

			PIXBeginNamedEvent( 0, message );
#endif // sig_use_pix
		}

		void fScopedPixProfilePop( Sig::Fui::tFuiFuncParams& params )
		{
#ifdef sig_use_pix
			sigassert( params.fCount( ) == 0 );
			PIXEndNamedEvent( );
#endif // sig_use_pix
		}
	}

	void fExportFuiInterface( )
	{
		Sig::Fui::tFuiSystem::fInstance( ).fRegisterFunc( "tScopedPixProfile.fPush", fScopedPixProfilePush );
		Sig::Fui::tFuiSystem::fInstance( ).fRegisterFunc( "tScopedPixProfile.fPop", fScopedPixProfilePop );
	}
}} // namespace Sig::PixProfiler
