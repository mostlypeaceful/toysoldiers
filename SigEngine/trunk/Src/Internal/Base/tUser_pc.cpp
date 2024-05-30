#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tUser.hpp"
#include "tFileWriter.hpp"
#include "tFileReader.hpp"
#include "tEncryption.hpp"

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

	void tUserData::fSetPlatformSpecific( const void * platformSpecific )
	{
		log_warning_unimplemented( );
	}

	void tUserData::fGetPlatformSpecific( void * platformSpecific ) const
	{
		log_warning_unimplemented( );
	}

	void tUserInfo::fRefresh( u32 localHwIndex )
	{
#ifndef target_tools
		// Check if we should generate a user id
		if( localHwIndex >= 0 && mUserId == tUser::cInvalidUserId )
		{
			// Create a 128-bit UUID as a basis for our 64-bit user id.
			// Note the use of the "sequential" version of the function, which contains a mac address.
			UUID uuid;
			RPC_STATUS status = UuidCreateSequential( &uuid );
			if( status != RPC_S_OK )
				log_warning( "UuidCreateSequential failed with status " << status );

			// Set bytes 0-1 of mUserId to a sequential number, which is stored in uuid.Data1.
			// This gives us an id that's unique wrt other users within the application instance,
			// and usually unique wrt other instances of the application.
			*( u16* )&mUserId = ( u16 )uuid.Data1;

			// Set bytes 2-8 of mUserId to our mac address, which is stored in bytes 2-8 of uuid.Data4.
			// This ensures that the id is unique to this PC.
			byte* userIdBytes = ( byte* )&mUserId;
			for( u32 i = 2; i < 8; ++i )
				userIdBytes[ i ] = uuid.Data4[ i ];

			log_line( 0, "hw" << localHwIndex << " user id: " << std::hex << mUserId );

			fRefreshGamerTag( localHwIndex );
		}
#endif
	}

	void tUserInfo::fRefreshGamerTag( u32 localHwIndex )
	{
		// Generate a gamer tag using the platform id
		std::wstringstream ss;
		ss << "User ";
		ss << std::setfill( L'0' ) << std::setw( 8 );
		ss << std::hex << ( u32 )mUserId;
		mGamerTag = tLocalizedString( ss.str( ).c_str( ) );
	}

	void tUser::fRefreshPlatformId( )
	{
		mUserInfo.fRefresh( mLocalHwIndex );
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

	void tUser::fShowSignInUI( bool onlineOnly ) const
	{
	}

	void tUser::fShowAchievementsUI( ) const
	{
	}

	b32 tUser::fShowMarketplaceUI( u64 offerId ) const
	{
		return false;
	}

	void tUser::fShowGamerCardUI( tPlatformUserId toShow ) const
	{
	}

	void tUser::fShowFriendsUI( ) const
	{
	}

	void tUser::fShowInviteUI( ) const
	{
	}

	void tUser::fShowCommunitySessionsUI( ) const
	{
	}

	b32 tUser::fIsInActiveParty( ) const
	{
		return false;
	}

	namespace
	{
		static const tFilePathPtr cDebugSavePath( "c:/toyboxdata/savetest.txt" ); 

		b32 fTestCheckSum( byte* data, u32 byteCount, u8 sum )
		{
			if( byteCount == 0 ) 
				return true;

			b32 valid = (tEncryption::fCheckSum( data, byteCount, sum ) == 0);

			return valid;
		}

		u32 fComputeCheckSum( byte* data, u32 byteCount )
		{
			return tEncryption::fCheckSum( data, byteCount, 0 );
		}
	}

	b32 tUser::fReadProfile( tDynamicArray<byte>& dataOut )
	{
		tFileReader reader;
		if( !reader.fOpen( cDebugSavePath ) )
		{
			log_warning( "Failed to open user profile file path." );
			return false;
		}

		u32 sizePlusCheckSum = reader.fFileSize( cDebugSavePath );
		if( sizePlusCheckSum == 0 )
		{
			log_warning( "User profile is 0 bytes." );
			return false;
		}

		u32 dataSize = sizePlusCheckSum - 1;

		dataOut.fNewArray( dataSize );
		reader( dataOut.fBegin( ), dataSize );

		u8 checkSum = 0;
		reader( &checkSum, 1 );

		return fTestCheckSum( dataOut.fBegin( ), dataSize, checkSum );
	}

	class tUserProfileSavePC : public tUserProfileSaveBase
	{
	public:
		tUserProfileSavePC( )
			: mFileWriter( cDebugSavePath, false )
		{
		}

		virtual b32 fSaveDataMT( byte* data, u32 size )
		{
			u8 checkSum = fComputeCheckSum( data, size );

			mFileWriter( data, size );
			mFileWriter( &checkSum, 1 );

			//TODO: check storage capacity of mFileWriter, return true/false accordingly.
			return true;
		}

	private:
		tFileWriter mFileWriter;
	};

	tUserProfileSaveBase* tUser::fCreateUserProfileSave( )
	{
		return NEW tUserProfileSavePC( );
	}

	b32 tUser::fCanWriteToProfile( )
	{
		if( !fSignedIn( ) )
			return false;

		return true;
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

	//------------------------------------------------------------------------------
	b32 tUser::fIsGuest( u32 localHwIndex )
	{
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tUser::fSignedIn( u32 localHwIndex )
	{
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tUser::fIsMuted( u32 localHwIndex, tPlatformUserId test )
	{
		return false;
	}

	//------------------------------------------------------------------------------
	tPlatformUserId tUser::fGetUserId( u32 localHwIndex )
	{
		return cInvalidUserId;
	}
}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
