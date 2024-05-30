//------------------------------------------------------------------------------
// \file tJsonGameArchiveLoader.impl.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tJsonGameArchiveLoader_impl__
#define __tJsonGameArchiveLoader_impl__

#include "tJsonGameArchive_SaveLoadIntrinsics.hpp"

namespace Sig
{
	template< class t > void tJsonGameArchiveLoader::fLoad( t& object )
	{
		fReadHeader( mReader );

		mSafeToSaveLoad = true;
		fSaveLoad( object );
		mSafeToSaveLoad = false;

		fReadFooter( mReader );
	}

	template< class t > void tJsonGameArchiveLoader::fSaveLoad( t& object )
	{
		log_sigcheckfail( mSafeToSaveLoad, "You must call fLoad before fSaveLoad", return );
		if( !fFailed( ) )
			tJsonGameArchive_SaveLoadIntrinsics< t >::fLoad( *this, object );
	}

	template< class t > b32 tJsonGameArchiveLoader::fSanityCheckArrayAlloc( u32 arrayCount )
	{
		return fSanityCheckArrayAlloc( sizeof(t), arrayCount );
	}
}

#endif //ndef __tJsonGameArchiveLoader_impl__
