//------------------------------------------------------------------------------
// \file tAchievements_pc.cpp - 21 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tAchievements.hpp"

namespace Sig
{
	const u32 tAchievementsReader::cDetailsLabel = ( 1 << 0 );
	const u32 tAchievementsReader::cDetailsDescription = ( 1 << 1 );
	const u32 tAchievementsReader::cDetailsUnachieved = ( 1 << 2 );
	
	//------------------------------------------------------------------------------
	// tAchievementsWriter
	//------------------------------------------------------------------------------
	tAchievementsWriter::tAchievementsWriter( 
		u32 localUserIndex, u32 count, const u32 achievements[] )
		: mState( cStateNull )
	{
		//log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	tAchievementsWriter::~tAchievementsWriter( )
	{
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::fBegin( )
	{
		//log_warning_unimplemented( );

		//TODO: implement fBegin and set mState accordingly.
		tSaveInstance::fBegin( );
		mState = cStateWriting;
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsWriter::fFinished( )
	{
		//log_warning_unimplemented( );

		//TODO: implement fFinished and set mState accordingly.
		mState = cStateSuccess;

		return true;
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::fFinish( b32 wait )
	{
		//log_warning_unimplemented( );

		//TODO: implement fFinish and set mState accordingly.
		mState = cStateSuccess;
	}

	//------------------------------------------------------------------------------
	// tAchievementsReader
	//------------------------------------------------------------------------------
	void tAchievementsReader::fRead( u32 startIdx, u32 toRead, u32 details, tPlatformUserId target )
	{
		//log_warning_unimplemented( );

		//TODO: implement fRead and set mState accordingly.
		mState = cStateSuccess;
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::fCancelRead( )
	{
		//log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	b32	tAchievementsReader::fAdvanceRead( )
	{
		//log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	b32	tAchievementsReader::fIsAwarded( u32 id )
	{
		//log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsReader::fGetData( u32 id, tAchievementData& out )
	{
		//log_warning_unimplemented( );
		return false;
	}
}
