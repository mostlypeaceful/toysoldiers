//------------------------------------------------------------------------------
// \file tAchievements.cpp - 20 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tAchievements.hpp"

namespace Sig
{

	//------------------------------------------------------------------------------
	// tAchievementData
	//------------------------------------------------------------------------------
	void tAchievementData::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tAchievementData, Sqrat::DefaultAllocator<tAchievementData> > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("Label"),			&tAchievementData::mLabel)
			.Var(_SC("Description"),	&tAchievementData::mDescription)
			.Var(_SC("ImageId"),		&tAchievementData::mImageId)
			;

		vm.fRootTable( ).Bind( _SC("AchievementData"), classDesc );
	}

	//------------------------------------------------------------------------------
	// tAchievementsReader
	//------------------------------------------------------------------------------
	tAchievementsReader::tAchievementsReader( )
		: mUserIndex( tUser::cMaxLocalUsers )
		, mState( cStateNull )
		, mReadCount( 0 )
	{

	}

	//------------------------------------------------------------------------------
	tAchievementsReader::~tAchievementsReader( )
	{
		fCancelRead( );
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::fSelectUser( u32 localHwIndex )
	{
		sigassert( mState != cStateReading && "Cancel any pending ops before changing users" );
		sigassert( localHwIndex < tUser::cMaxLocalUsers );

		mState = cStateNull;
		mUserIndex = localHwIndex;
		mReadCount = 0;
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::fWait( )
	{
		while( !fAdvanceRead( ) )
			fSleep( 10 );
	}
}
