//------------------------------------------------------------------------------
// \file tUserProfileServices_xbox360.cpp - 14 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tUserProfileServices.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tUserProfileServices
	//------------------------------------------------------------------------------
	const u32 tUserProfileServices::cProfilePictureKey = XPROFILE_GAMERCARD_PICTURE_KEY;
	const u32 tUserProfileServices::cProfileYAxisInversion = XPROFILE_GAMER_YAXIS_INVERSION;
	const u32 tUserProfileServices::cProfileTitleSpecific1 = XPROFILE_TITLE_SPECIFIC1;
	const u32 tUserProfileServices::cProfileTitleSpecific2 = XPROFILE_TITLE_SPECIFIC2;
	const u32 tUserProfileServices::cProfileTitleSpecific3 = XPROFILE_TITLE_SPECIFIC3;

	const u32 tUserProfileServices::cYAxisInversionOff = XPROFILE_YAXIS_INVERSION_OFF;
	const u32 tUserProfileServices::cYAxisInversionOn = XPROFILE_YAXIS_INVERSION_ON;

	//------------------------------------------------------------------------------
	void tUserProfileServices::fProcessLocalUser( u32 u )
	{
		tLocalUser & user = mLocalUsers[ u ];

		DWORD resultsSize = 0;

		// Query for user name
		{
			DWORD result = XUserGetName( u, user.mName.fBegin( ), user.mName.fCount( ) );
			sigassert( result == ERROR_SUCCESS );
		}

		// Query for size of results buffer
		{	
			DWORD result = XUserReadProfileSettings( 
				0,									// titleId, 0 for current
				u,									// local hw index of requester
				mLocalSettings.fCount( ),			// setting count
				(DWORD*)mLocalSettings.fBegin( ),	// setting ids
				&resultsSize, NULL,					// result buffer
				NULL );								// overlapped

			sigassert( result == ERROR_INSUFFICIENT_BUFFER );
			if( user.mSettings.fCount( ) != resultsSize )
				user.mSettings.fNewArray( resultsSize );
		}

		// Query for settings
		{
			XUSER_READ_PROFILE_SETTING_RESULT * results = 
				(XUSER_READ_PROFILE_SETTING_RESULT *)user.mSettings.fBegin( );

			DWORD result = XUserReadProfileSettings( 
				0,									// titleId, 0 for current
				u,									// local hw index of requester
				mLocalSettings.fCount( ),			// setting count
				(DWORD*)mLocalSettings.fBegin( ),	// setting ids
				&resultsSize, results,				// result buffer
				NULL );								// overlapped
			sigassert( result == ERROR_SUCCESS );
		}

		user.mState = cStateReady;
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fStartSettingsProcess( u32 localHwIndex, tRemoteUserQuery & query )
	{
		sigassert( query.mState == cStateWantsProcess && "Sanity!" );

		// Test the user's signin state to see if it can be processed
		if( XUserGetSigninState( localHwIndex ) != eXUserSigninState_SignedInToLive )
		{
			query.mState = cStateError;
			return;
		}

		DWORD resultsSize = 0;

		// Query for size of results buffer
		{	
			DWORD result = XUserReadProfileSettingsByXuid( 
				0,									// titleId, 0 for current
				localHwIndex,						// local hw index of requester
				query.mUserIds.fCount( ),			// user id count
				query.mUserIds.fBegin( ),			// user ids
				mRemoteSettings.fCount( ),			// setting count
				(DWORD*)mRemoteSettings.fBegin( ),	// setting ids
				&resultsSize, NULL,					// result buffer
				NULL );								// overlapped

			sigassert( result == ERROR_INSUFFICIENT_BUFFER );
			query.mSettings.fNewArray( resultsSize );
		}

		// Query for settings
		{
			XUSER_READ_PROFILE_SETTING_RESULT * results = 
				(XUSER_READ_PROFILE_SETTING_RESULT *)query.mSettings.fBegin( );

			fZeroOut( &query.mOverlapped );
			DWORD result = XUserReadProfileSettingsByXuid( 
				0,									// titleId, 0 for current
				localHwIndex,						// local hw index of requester
				query.mUserIds.fCount( ),			// user id count
				query.mUserIds.fBegin( ),			// user ids
				mRemoteSettings.fCount( ),			// setting count
				(DWORD*)mRemoteSettings.fBegin( ),	// setting ids
				&resultsSize, results,				// result buffer
				&query.mOverlapped );				// overlapped
			sigassert( result == ERROR_IO_PENDING );
		}

		query.mState = cStateInSettingsProcess;
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fStartNamesProcess( u32 localHwIndex, tRemoteUserQuery & query )
	{
		sigassert( query.mState == cStateInSettingsProcess && "Sanity!" );

		// Test the user's signin state to see if it can be processed
		if( XUserGetSigninState( localHwIndex ) != eXUserSigninState_SignedInToLive )
		{
			query.mState = cStateError;
			return;
		}

		const u32 userCount = query.mUserIds.fCount( );

		// Get the size and allocate the array
		DWORD resultsSize = XUserFindUsersResponseSize( userCount );
		query.mNames.fNewArray( resultsSize );
		query.mNames.fZeroOut( );

		// Get the xuid for the requester
		XUID localXuid;
		{
			DWORD result = XUserGetXUID( localHwIndex, &localXuid );
			sigassert( result == ERROR_SUCCESS );
		}

		FIND_USERS_RESPONSE * fur = (FIND_USERS_RESPONSE *)query.mNames.fBegin( );
		fur->pUsers = (FIND_USER_INFO *)(fur + 1 );

		for( u32 u = 0; u < userCount; ++u )
			fur->pUsers[ u ].qwUserId = query.mUserIds[ u ];

		// Start the query
		{
			fZeroOut( &query.mOverlapped );
			DWORD result = XUserFindUsers( 
				localXuid, userCount, fur->pUsers, resultsSize, fur, &query.mOverlapped );
			sigassert( result == ERROR_IO_PENDING );
		}

		query.mState = cStateInNamesProcess;
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileServices::fUpdateProcess( tRemoteUserQuery & query, b32 & success )
	{
		if( !XHasOverlappedIoCompleted( &query.mOverlapped ) )
			return false;

		DWORD error = XGetOverlappedExtendedError( &query.mOverlapped );
		success = ( error == ERROR_SUCCESS );
		
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileServices::fSaveLocalSettingsMT( 
		u32 localHwIndex, 
		const tUserProperty settings[], u32 settingCount, 
		const byte * titleBuffer, u32 titleBufferSize )
	{
		u32 finalSettingCount = settingCount;

		// Do we need to split the title buffer?
		const u32 cTitleSettingsNeeded = 
			( titleBufferSize / XPROFILE_SETTING_MAX_SIZE ) + 
			( titleBufferSize % XPROFILE_SETTING_MAX_SIZE ? 1 : 0 );

		if( cTitleSettingsNeeded > cTotalSettings )
		{
			log_warning( "Title Buffer too large to save to profile!" );
			return false;
		}
		
		// Ensure no one is using the setting ids
		if_assert( 
		if( titleBuffer && titleBufferSize )
		{
			for( u32 s = 0; s < settingCount; ++s )
			{
				sigassert( 
					settings[ s ].mId != cProfileTitleSpecific1 &&
					settings[ s ].mId != cProfileTitleSpecific2 &&
					settings[ s ].mId != cProfileTitleSpecific3 );
			}
		} )
		
		const u32 cTotalSettingsCount = settingCount + cTitleSettingsNeeded;
		malloca_array( XUSER_PROFILE_SETTING, platformSettings, cTotalSettingsCount );

		u32 currSetting = 0;
		for( u32 i = 0; i < settingCount; ++i )
		{
			XUSER_PROFILE_SETTING & s = platformSettings[ currSetting++ ];
			s.user.dwUserIndex = localHwIndex;
			s.source = XSOURCE_TITLE;
			s.dwSettingId = settings[ i ].mId;
			settings[ i ].mData.fGetPlatformSpecific( &s.data );
		}

		const u32 cTitleSpecificIds[ cTotalSettings ] = { cProfileTitleSpecific1, cProfileTitleSpecific2, cProfileTitleSpecific3 };
		for( u32 i = 0; i < cTitleSettingsNeeded; ++i )
		{
			XUSER_PROFILE_SETTING & s = platformSettings[ currSetting++ ];
			s.user.dwUserIndex = localHwIndex;
			s.source = XSOURCE_TITLE;
			s.dwSettingId = cTitleSpecificIds[ i ];
			s.data.type = XUSER_DATA_TYPE_BINARY;
			s.data.binary.cbData = fMin<DWORD>( XPROFILE_SETTING_MAX_SIZE, titleBufferSize );
			s.data.binary.pbData = (PBYTE)titleBuffer;

			titleBuffer += s.data.binary.cbData;
			titleBufferSize -= s.data.binary.cbData;
		}

		// Save!
		const DWORD err = XUserWriteProfileSettings( 
			localHwIndex, platformSettings.fCount( ), platformSettings.fBegin( ), NULL );
		if( err != ERROR_SUCCESS )
			log_warning( "Error returned: XUserWriteProfileSettings, " << err );

		mLocalUsers[ localHwIndex ].mState = cStateWantsProcess;

		return err == ERROR_SUCCESS;
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileServices::fGetLocalSettingInternal( 
		u32 user, u32 setting, tUserData & data ) const
	{
		sigassert( mLocalUsers[ user ].mState == cStateReady && "Sanity!" );

		XUSER_READ_PROFILE_SETTING_RESULT * results = 
			(XUSER_READ_PROFILE_SETTING_RESULT *)mLocalUsers[ user ].mSettings.fBegin( );

		for( u32 s = 0; s < results->dwSettingsLen; ++s )
		{
			if( results->pSettings[ s ].dwSettingId == setting )
			{
				data.fSetPlatformSpecific( &results->pSettings[ s ].data );
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------
	const char * tUserProfileServices::fGetRemoteNameInternal( 
		tRemoteUserQuery & query, const tPlatformUserId & userId ) const
	{
		if( query.mState != cStateReady )
			return NULL;

		const FIND_USERS_RESPONSE * results = 
			(const FIND_USERS_RESPONSE*)query.mNames.fBegin( );

		for( u32 u = 0; u < results->dwResults; ++u )
		{
			if( results->pUsers[ u ].qwUserId == userId )
				return results->pUsers[ u ].szGamerTag;
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileServices::fGetRemoteSettingInternal( 
		tRemoteUserQuery & query, const tPlatformUserId & userId, u32 setting, tUserData & data ) const
	{
		if( query.mState != cStateReady )
			return false;

		const XUSER_READ_PROFILE_SETTING_RESULT * results = 
			(const XUSER_READ_PROFILE_SETTING_RESULT *)query.mSettings.fBegin( );

		for( u32 s = 0; s < results->dwSettingsLen; ++s )
		{
			if( results->pSettings[ s ].user.xuid == userId && 
				results->pSettings[ s ].dwSettingId == setting )
			{
				data.fSetPlatformSpecific( &results->pSettings[ s ].data );
				return true;
			}
		}

		return false;
	}

} // ::Sig
