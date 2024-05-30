//------------------------------------------------------------------------------
// \file tFuiUserProfilePicService.cpp - 22 Aug 2012
// \author pwalker
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tFuiUserProfilePicService.hpp"

namespace Sig
{
	namespace
	{
		static const tStringPtr cProfilePicPrepend( "gamerpic_" );
	}

	//------------------------------------------------------------------------------
	// tFuiUserProfilePic
	//------------------------------------------------------------------------------
	tFuiUserProfilePic::tFuiUserProfilePic( )
		: mExplicitUserId( tUser::cInvalidUserId )
		, mSmall( false )
	{
	}

	tFuiUserProfilePic::tFuiUserProfilePic( const tStringPtr& id, const tUserProfilePicRef& profilePicRef, b32 _small )
		: mId( id )
		, mProfilePicRef( profilePicRef )
		, mSmall( _small )
	{
	}

	tFuiUserProfilePic::tFuiUserProfilePic( const tStringPtr& id, const tPlatformUserId & userId, const Gfx::tTextureReference& pic, b32 _small )
		: mId( id )
		, mExplicitUserId( userId )
		, mExplicitPic( pic )
		, mSmall( _small )
	{
	}

	b32 tFuiUserProfilePic::fGetTexture( Gfx::tTextureReference & tex, Math::tVec2f * outDims ) const
	{
		if( mProfilePicRef.fNull( ) )
		{
			if( !mExplicitPic.fNull( ) )
			{
				tex = mExplicitPic;
				if( outDims )
					outDims->x = outDims->y = ( mSmall ? 32.f : 64.f ); // Dims from XDK docs

				return true;
			}
		}
		else
		{
			return mProfilePicRef.fPic( ).fGetTexture( tex, outDims );
		}

		return false;
	}

	tPlatformUserId tFuiUserProfilePic::fUserId( ) const
	{
		if( mProfilePicRef.fNull( ) )
			return mExplicitUserId;

		return mProfilePicRef.fProfile( ).fUserId( );
	}

	b32 tFuiUserProfilePic::fIsReady( ) const
	{
		if( mProfilePicRef.fNull( ) )
		{
			Gfx::tTextureReference tex;
			return fGetTexture( tex, NULL );
		}

		return mProfilePicRef.fIsReady( );
	}

	void tFuiUserProfilePic::fUpdateST( )
	{
		if( !mProfilePicRef.fNull( ) )
			mProfilePicRef.fUpdateST( );
	}

	//------------------------------------------------------------------------------
	// tFuiUserProfilePicService
	//------------------------------------------------------------------------------
	tStringPtr tFuiUserProfilePicService::fAddUser( u32 requesterIdx, const tPlatformUserId & userId, b32 _small )
	{
		if( const tFuiUserProfilePic* userProfile = fFindUserInternal( userId, _small ) )
			return userProfile->fId( );

		tUserProfilePicRef profilePicRef;
		profilePicRef.fReset( requesterIdx, userId, _small );
		mProfilePicRefs.fPushBack( tFuiUserProfilePic( fGenerateId( _small ), profilePicRef, _small ) );

		return mProfilePicRefs.fBack( ).fId( );
	}

	tStringPtr tFuiUserProfilePicService::fAddUser( u32 requesterIdx, const tPlatformUserId & userId, const Gfx::tTextureReference& pic, b32 _small )
	{
		if( const tFuiUserProfilePic* userProfile = fFindUserInternal( userId, _small ) )
			return userProfile->fId( );

		mProfilePicRefs.fPushBack( tFuiUserProfilePic( fGenerateId( _small ), userId, pic, _small ) );

		return mProfilePicRefs.fBack( ).fId( );
	}

	const tFuiUserProfilePic* tFuiUserProfilePicService::fFindUser( const tStringPtr& id )
	{
		for( u32 i = 0; i < mProfilePicRefs.fCount( ); ++i )
		{
			if( mProfilePicRefs[ i ].fId( ) == id )
				return &mProfilePicRefs[ i ];
		}
		return NULL;
	}

	void tFuiUserProfilePicService::fProcessST( )
	{
		for( u32 i = 0; i < mProfilePicRefs.fCount( ); ++i )
			mProfilePicRefs[ i ].fUpdateST( );
	}

	const tFuiUserProfilePic* tFuiUserProfilePicService::fFindUserInternal( const tPlatformUserId & userId, b32 _small )
	{
		for( u32 i = 0; i < mProfilePicRefs.fCount( ); ++i )
		{
			const tFuiUserProfilePic& profilePicRef = mProfilePicRefs[ i ];
			if( profilePicRef.fSmall( ) == _small && profilePicRef.fUserId( ) == userId )
				return &profilePicRef;
		}
		return NULL;
	}

	tStringPtr tFuiUserProfilePicService::fGenerateId( b32 _small )
	{
		//Generate a string formated like so
		// (0|1 - is small)(numProfilesCreated)
		std::stringstream ss;
		ss << cProfilePicPrepend;
		if( _small )
			ss << "1";
		else
			ss << "0";
		ss << mProfilePicRefs.fCount( );
		return tStringPtr( ss.str( ) );
	}

} // Sig