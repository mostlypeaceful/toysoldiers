//------------------------------------------------------------------------------
// \file tUserPicServices_pc.cpp - 15 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tUserPicServices.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tUserPicServices
	//------------------------------------------------------------------------------
	void tUserPicServices::fPlatformShutdown( )
	{

	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fQueryPic( const tPictureKey & key, tPicture & picture )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fProcessPic( tPicture & picture )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fCreateTexture( Gfx::tTextureReference & out, b32 _small )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fDestroyTexture( Gfx::tTextureReference & in )
	{
		log_warning_unimplemented( );
	}

} // ::Sig

