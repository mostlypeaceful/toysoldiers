//------------------------------------------------------------------------------
// \file tGameArchiveSave.impl.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tGameArchiveSave_impl__
#define __tGameArchiveSave_impl__

#include "GameArchiveUtils.hpp"

namespace Sig
{
	template< class t >
	void tGameArchiveSave::fSave( t& object, u8 version )
	{
		mHeader.mPlatform = cCurrentPlatform;
		mHeader.mVersion = version;
		mSafeToSaveLoad = true;
		fSaveLoad( mHeader );
		fSaveLoad( object );
		mSafeToSaveLoad = false;
	}

	template< class t >
	void tGameArchiveSave::fSaveLoad( t& object )
	{
		sigassert( mSafeToSaveLoad && "You must call fSave before fSaveLoad" );
		tGameArchiveSaveLoad< tIsBuiltInType< t >::cIs >::fSave( *this, object );
	}
}

#endif //ndef __tGameArchiveSave_impl__
