#include "BasePch.hpp"
#if defined( platform_ios )
#include "tUser.hpp"

namespace Sig
{
	const u32 tUserData::cTypeS32		= 0;
	const u32 tUserData::cTypeS64		= 1;
	const u32 tUserData::cTypeF32		= 2;
	const u32 tUserData::cTypeF64		= 3;
	const u32 tUserData::cTypeUnicode	= 4;
	const u32 tUserData::cTypeBinary	= 5;
	const u32 tUserData::cTypeNull		= ~0;

	const tPlatformUserId tUser::cInvalidUserId = 0;
	//const u32 tUser::cMaxLocalUsers = 4;
	const u32 tUser::cUserContextPresence = 0;
	const u32 tUser::cUserContextGameMode = 1;
	const u32 tUser::cUserContextGameType = 2;
	const u32 tUser::cUserContextGameTypeRanked = 0;
	const u32 tUser::cUserContextGameTypeStandard = 1;

	const u32 tUser::cPrivilegeMultiplayer = 0;

	void tUserPic::fStartReadKey( )
	{
		log_warning_unimplemented( 0 );
	}

	void tUserPic::fStartReadPicture( )
	{
		log_warning_unimplemented( 0 );
	}

	void tUserPic::fFinishReadPicture( )
	{
		log_warning_unimplemented( 0 );
	}

	void tUserPic::fReleaseTextures( )
	{
		log_warning_unimplemented( 0 );
	}

	void tUserData::fSetPlatformSpecific( const void * platformSpecific )
	{
		log_warning_unimplemented( 0 );
	}

	void tUserData::fGetPlatformSpecific( void * platformSpecific ) const
	{
		log_warning_unimplemented( 0 );
	}

	void tUserInfo::fRefresh( u32 localHwIndex )
	{
	}

	void tUserInfo::fRefreshGamerTag( u32 localHwIndex )
	{
	}

	void tUser::fRefreshPlatformId( )
	{
	}

	b32 tUser::fIsGuest( ) const
	{
		return false;
	}

	b32 tUser::fIsOnlineEnabled( ) const
	{
		return false;
	}

	b32 tUser::fSignedIn( ) const
	{
		return true;
	}

	b32 tUser::fSignedInOnline( ) const
	{
		return false;
	}

	tUser::tSignInState tUser::fSignInState( ) const
	{
		return cSignInStateNotSignedIn;
	}

	b32 tUser::fHasPrivilege( u32 privilege ) const
	{
		return false;
	}

	void tUser::fShowSignInUI( ) const
	{
	}

	void tUser::fShowAchievementsUI( ) const
	{
	}

	void tUser::fShowMarketplaceUI( bool all ) const
	{
	}

	void tUser::fShowGamerCardUI( tPlatformUserId toShow ) const
	{
	}

	void tUser::fShowFriendsUI( ) const
	{
	}

	void tUser::fShowCommunitySessionsUI( ) const
	{
	}

	void tUser::fShowInviteUI( ) const
	{
	}

	b32 tUser::fIsInActiveParty( ) const
	{
		return false;
	}

	b32 tUser::fReadProfile( byte* data, u32 size )
	{
		return false;
	}

	b32 tUser::fWriteToProfile( byte* data, u32 size )
	{
		return false;
	}

	b32 tUser::fCanWriteToProfile( )
	{
		return false;
	}

	b32 tUser::fYAxisInvertedDefault( )
	{
		return false;
	}

	b32 tUser::fSouthpawDefault( )
	{
		return false;
	}

	b32 tUser::fAwardAvatar( u32 avatarId )
	{
		return false;
	}

	b32 tUser::fAwardGamerPicture( u32 pictureId )
	{
		return false;
	}

	void tUser::fSetContext( u32 id, u32 value )
	{
		
	}

	void tUser::fSetProperty( u32 id, const tUserData & value )
	{

	}
	
	b32 tUser::fCheckFullLicenseFlag( )
	{
		return true;
	}
	
	b32 tUser::fIsMuted( u32 localHwIndex, tPlatformUserId test )
	{
		return false;
	}
	
}
#endif//#if defined( platform_ios )
