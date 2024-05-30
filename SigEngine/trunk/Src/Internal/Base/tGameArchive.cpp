//------------------------------------------------------------------------------
// \file tGameArchive.cpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tGameArchive.hpp"

namespace Sig
{
	tGameArchive::tGameArchive( tGameArchiveMode mode ) 
		: mMode( mode )
		, mFailed( false )
		, mSafeToSaveLoad( false )
		, mHeader( cCurrentPlatform, 0xFF )
		, mFilePathArchiver( NULL )
	{
	}



	tGameArchiveMode tGameArchive::fMode( ) const
	{
		return mMode;
	}

	const tGameArchiveHeader& tGameArchive::fHeader( ) const
	{
		return mHeader;
	}

	tFilePathArchiver* tGameArchive::fFilePathArchiver( ) const
	{
		return mFilePathArchiver;
	}

	b32 tGameArchive::fFailed( ) const
	{
		return mFailed;
	}



	void tGameArchive::fFail( )
	{
		mFailed = true;
	}
}
