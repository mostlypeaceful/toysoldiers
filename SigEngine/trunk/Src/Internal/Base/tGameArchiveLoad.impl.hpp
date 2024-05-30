//------------------------------------------------------------------------------
// \file tGameArchiveLoad.impl.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tGameArchiveLoad_impl__
#define __tGameArchiveLoad_impl__

#include "GameArchiveUtils.hpp"
#include "tGameArchiveLoad.hpp"
#include "tGameArchiveSave.hpp"

namespace Sig
{
	template< class t >
	void tGameArchiveLoad::fLoad( t& object )
	{
		mSafeToSaveLoad = true;
		fSaveLoad( mHeader );
		if( !fFailed() )
			fSaveLoad( object );
		mSafeToSaveLoad = false;
	}

	template< class t >
	void tGameArchiveLoad::fSaveLoad( t& object )
	{
		sigassert( mSafeToSaveLoad && "You must call fLoad before fSaveLoad" );
		tGameArchiveSaveLoad< tIsBuiltInType< t >::cIs >::fLoad( *this, object );
	}

	template< class t >
	b32 tGameArchiveLoad::fSanityCheckArrayAlloc( u32 arrayCount )
	{
		return fSanityCheckArrayAlloc(sizeof(t),arrayCount);
	}
}

#endif //ndef __tGameArchiveLoad_impl__
