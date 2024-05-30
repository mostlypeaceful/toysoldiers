//------------------------------------------------------------------------------
// \file tFuiUserProfilePicService.hpp - 22 Aug 2012
// \author pwalker
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tFuiUserProfilePicService__
#define __tFuiUserProfilePicService__

#include "tUserPicServices.hpp"

namespace Sig
{
	class tFuiUserProfilePic
	{
	public:
		tFuiUserProfilePic( );
		tFuiUserProfilePic( const tStringPtr& id, const tUserProfilePicRef& profilePicRef, b32 _small );
		tFuiUserProfilePic( const tStringPtr& id, const tPlatformUserId & userId, const Gfx::tTextureReference& pic, b32 _small );

		const tStringPtr& fId( ) const { return mId; }
		b32 fGetTexture( Gfx::tTextureReference & tex, Math::tVec2f * outDims ) const;
		tPlatformUserId fUserId( ) const;
		b32 fSmall( ) const { return mSmall; }
		b32 fIsReady( ) const;
		const tUserProfileRef & fProfile( ) const { return mProfilePicRef.fProfile( ); }

		void fUpdateST( );

	private:
		tStringPtr mId;
		tUserProfilePicRef mProfilePicRef;
		Gfx::tTextureReference mExplicitPic;
		tPlatformUserId mExplicitUserId;
		b32 mSmall;
	};

	class tFuiUserProfilePicService
	{
		declare_singleton( tFuiUserProfilePicService );
	public:
		tStringPtr fAddUser( u32 requesterIdx, const tPlatformUserId & userId, b32 _small );
		tStringPtr fAddUser( u32 requesterIdx, const tPlatformUserId & userId, const Gfx::tTextureReference& pic, b32 _small );
		const tFuiUserProfilePic* fFindUser( const tStringPtr& id ); // string id returned from fAddUser
		void fProcessST( );
	private:
		const tFuiUserProfilePic* fFindUserInternal( const tPlatformUserId & userId, b32 _small ); // string id returned from fAddUser
		tStringPtr fGenerateId( b32 _small );
	private:
		tGrowableArray< tFuiUserProfilePic > mProfilePicRefs;
	};
} // Sig

#endif // __tFuiUserProfilePicService__
