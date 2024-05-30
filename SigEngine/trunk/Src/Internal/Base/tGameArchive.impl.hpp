//------------------------------------------------------------------------------
// \file tGameArchive.impl.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tGameArchive_impl__
#define __tGameArchive_impl__

#include "tGameArchiveLoad.hpp"
#include "tGameArchiveSave.hpp"

namespace Sig
{
	template< class t >
	void tGameArchive::fSaveLoad( t& object )
	{
		switch( mMode )
		{
		case cGameArchiveModeSave: static_cast<tGameArchiveSave*>(this)->fSaveLoad( object ); break;
		case cGameArchiveModeLoad: static_cast<tGameArchiveLoad*>(this)->fSaveLoad( object ); break;
		}
	}
}

#endif //ndef __tGameArchive_impl__
