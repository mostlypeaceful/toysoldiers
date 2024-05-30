//------------------------------------------------------------------------------
// \file tJsonGameArchive.cpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tJsonGameArchive.hpp"
#include "Debug/tDebugger.hpp"

namespace Sig
{
	namespace
	{
		const u8 cInvalidVersionByte = 0xFF;
	}

	tJsonGameArchive::~tJsonGameArchive( )
	{
	}



	tGameArchiveMode tJsonGameArchive::fMode( ) const
	{
		return mMode;
	}

	b32 tJsonGameArchive::fFailed( ) const
	{
		return mFailed;
	}

	const tGameArchiveHeader& tJsonGameArchive::fHeader( ) const
	{
		return mHeader;
	}



	void tJsonGameArchive::fFail( )
	{
		mFailed = true;
		break_if_debugger( );
	}

	tJsonGameArchive::tJsonGameArchive( tGameArchiveMode mode )
		: mMode( mode )
		, mFailed( false )
		, mSafeToSaveLoad( false )
		, mHeader( cCurrentPlatform, cInvalidVersionByte )
	{
	}
}
