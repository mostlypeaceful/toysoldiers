#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tUser.hpp"
#include <xparty.h>
#include "tApplication.hpp"

namespace Sig
{
	devvar( bool, AAACheats_FullVersionOverride, false );
	devvar( bool, AAACheats_FullVersionValue, true );
	devvar( bool, AAACheats_SimulatePurchase, false );

	namespace
	{
		XUSER_SIGNIN_STATE fGetSigninState( u32 userIndex )
		{
			if( userIndex >= tUser::cMaxLocalUsers )
				return eXUserSigninState_NotSignedIn;
			return XUserGetSigninState( userIndex );
		}
	}

	const u32 tUserData::cTypeS32		= XUSER_DATA_TYPE_INT32;
	const u32 tUserData::cTypeS64		= XUSER_DATA_TYPE_INT64;
	const u32 tUserData::cTypeF32		= XUSER_DATA_TYPE_FLOAT;
	const u32 tUserData::cTypeF64		= XUSER_DATA_TYPE_DOUBLE;
	const u32 tUserData::cTypeUnicode	= XUSER_DATA_TYPE_UNICODE;
	const u32 tUserData::cTypeBinary	= XUSER_DATA_TYPE_BINARY;
	const u32 tUserData::cTypeNull		= XUSER_DATA_TYPE_NULL;

	const tPlatformUserId tUser::cInvalidUserId = INVALID_XUID;
	//const u32 tUser::cMaxLocalUsers = XUSER_MAX_COUNT;
	const u32 tUser::cUserContextPresence = X_CONTEXT_PRESENCE;
	const u32 tUser::cUserContextGameMode = X_CONTEXT_GAME_MODE;
	const u32 tUser::cUserContextGameType = X_CONTEXT_GAME_TYPE;
	const u32 tUser::cUserContextGameTypeRanked = X_CONTEXT_GAME_TYPE_RANKED;
	const u32 tUser::cUserContextGameTypeStandard = X_CONTEXT_GAME_TYPE_STANDARD;

	const u32 tUser::cPrivilegeMultiplayer = XPRIVILEGE_MULTIPLAYER_SESSIONS;

	//------------------------------------------------------------------------------
	// tUserPic
	//------------------------------------------------------------------------------
	b32 tUserPic::tReadData::fOperationComplete( b32 & success )
	{
		if( !XHasOverlappedIoCompleted( &mOverlapped ) )
			return false;

		DWORD result = XGetOverlappedExtendedError( &mOverlapped );
		success = (result == ERROR_SUCCESS);
		return true;
	}
	//------------------------------------------------------------------------------
	void tUserPic::fStartReadKey( )
	{
		sigassert( mState == cStateNull );
		sigassert( mRequest.mRequester < tUser::cMaxLocalUsers );
		sigassert( mRequest.mGamer != tUser::cInvalidUserId );

		u32 operationSize = sizeof( XUID ) + sizeof( DWORD );
		mReadData.mOperationData.fNewArray( operationSize );

		DWORD resultsSize = 0;

		// Query for size of results buffer
		{
			byte * data = mReadData.mOperationData.fBegin( );

			XUID * xuid = (XUID *)data; 
			data += sizeof( XUID );

			DWORD * setting = (DWORD*)data; 
			data += sizeof( DWORD );

			*xuid = mRequest.mGamer;
			*setting = XPROFILE_GAMERCARD_PICTURE_KEY;

			DWORD result = XUserReadProfileSettingsByXuid( 
				0,						// titleId
				mRequest.mRequester,	// local hw index of requester
				1, xuid,				// xuids to query for, max 15
				1, setting,				// settings to query, max 32
				&resultsSize, NULL,		// result buffer
				NULL );					// overlapped

			if( result != ERROR_INSUFFICIENT_BUFFER )
			{
				log_warning( 0, "Error " << result << " querying result buffer size for gamer picture key query" );
				return;
			}
		}

		// Resize the op buffer
		operationSize += resultsSize;
		mReadData.mOperationData.fResize( operationSize );

		byte * data = mReadData.mOperationData.fBegin( );

		// The xuid and setting were set before the resultSize query
		const XUID * xuid = (XUID *)data;
		data += sizeof( XUID );

		const DWORD * setting = (DWORD*)data;
		data += sizeof( DWORD );

		XUSER_READ_PROFILE_SETTING_RESULT * results = (XUSER_READ_PROFILE_SETTING_RESULT *)data;

		DWORD result = XUserReadProfileSettingsByXuid( 
			0,							// titleId
			mRequest.mRequester,		// local hw index of requester
			1, xuid,					// xuids to query for, max 15
			1, setting,					// settings to query, max 32
			&resultsSize, results,		// result buffer
			&mReadData.mOverlapped );	// overlapped

		if( result != ERROR_IO_PENDING && result != ERROR_SUCCESS )
		{
			log_warning( 0, "Error " << result << "querying gamer picture key" );
			return;
		}

		mState = cStateReadingKey;
	}

	//------------------------------------------------------------------------------
	void tUserPic::fStartReadPicture( )
	{
		sigassert( mState == cStateReadingKey );

		const byte * data = mReadData.mOperationData.fBegin( );
		data += sizeof(XUID) + sizeof(DWORD);

		const XUSER_READ_PROFILE_SETTING_RESULT * result = (const XUSER_READ_PROFILE_SETTING_RESULT *)data;
		sigassert( result->dwSettingsLen == 1 );

		const Gfx::tDevicePtr & device = Gfx::tDevice::fGetDefaultDevice( );

		IDirect3DDevice9* d3ddev = device->fGetDevice( );
		IDirect3DTexture9* d3dtex = 0;

		const u32 dim = mRequest.mSmall ? 32 : 64; // From XDK docs
		mNextTextureDims.x = mNextTextureDims.y = (f32)dim;

		// create the texture
		HRESULT hr = d3ddev->CreateTexture( dim, dim, 1, 0, D3DFMT_LIN_A8R8G8B8, 0, &d3dtex, 0 );
		sigassert( SUCCEEDED( hr ) && d3dtex );

		// lock the top mip
		D3DLOCKED_RECT rect;
		d3dtex->LockRect( 0, &rect, 0, 0 );

		DWORD error = XUserReadGamerPictureByKey( 
			&result->pSettings->data,			// XUSER_DATA with gamer picture key
			mRequest.mSmall ? TRUE : FALSE,		// Request small(32x32) instead of large(64x64)
			(PBYTE)rect.pBits,					// Texture bits to copy into
			rect.Pitch,							// Texture pitch
			dim,								// Height of the texture
			&mReadData.mOverlapped );			// Overlapped

		if( error != ERROR_IO_PENDING && error != ERROR_SUCCESS )
		{
			log_warning( 0, "Failed to start gamer picture read by key: " << error );
			Gfx::fReleaseComPtr( d3dtex );
			return;
		}

		mNextTexture = d3dtex;
		mState = cStateReadingPicture;
	}

	//------------------------------------------------------------------------------
	void tUserPic::fFinishReadPicture( )
	{
		Gfx::fReleaseComPtr((IDirect3DTexture9*&)mTexture);

		// Unlock the rect if the next texture is valid
		sigassert( mNextTexture );
		if( IDirect3DTexture9* d3dtex = (IDirect3DTexture9*)mNextTexture )
			d3dtex->UnlockRect( 0 );

		mTexture = mNextTexture;
		mTextureDims = mNextTextureDims;
		mNextTexture = NULL;
		mNextTextureDims = Math::tVec2f::cZeroVector;
	}

	//------------------------------------------------------------------------------
	void tUserPic::fReleaseTextures( )
	{
		Gfx::fReleaseComPtr((IDirect3DTexture9*&)mTexture);
		Gfx::fReleaseComPtr((IDirect3DTexture9*&)mNextTexture);
	}

	//------------------------------------------------------------------------------
	void tUserPic::tReadData::fReset( )
	{
		fZeroOut( mOverlapped );
	}

	//------------------------------------------------------------------------------
	// tUserData
	//------------------------------------------------------------------------------
	void tUserData::fSetPlatformSpecific( const void * platformUserData )
	{
		const XUSER_DATA * data = (const XUSER_DATA *)platformUserData;

		mType = data->type;
		
		if( mType == cTypeS32 )
			mS32Data = data->nData;
		else if( mType == cTypeS64 )
			mS64Data = data->i64Data;
		else if( mType == cTypeF32 )
			mF32Data = data->fData;
		else if( mType == cTypeF64 )
			mF64Data = data->dblData;
		else if( mType == cTypeUnicode )
		{
			mUnicodeData.mCharCount = data->string.cbData;
			mUnicodeData.mChars = data->string.pwszData;
		}
		else if( mType == cTypeBinary )
		{
			mBinaryData.mByteCount = data->binary.cbData;
			mBinaryData.mBytes = data->binary.pbData;
		}
	}

	//------------------------------------------------------------------------------
	void tUserData::fGetPlatformSpecific( void * platformSpecific ) const
	{
		XUSER_DATA * data = (XUSER_DATA *)platformSpecific;
		data->type = (BYTE)mType;
		
		if( mType == cTypeS32 )
			data->nData = mS32Data;
		else if( mType == cTypeS64 )
			data->i64Data = mS64Data;
		else if( mType == cTypeF32 )
			data->fData = mF32Data;
		else if( mType == cTypeF64 )
			data->dblData = mF64Data;
		else if( mType == cTypeUnicode )
		{
			data->string.cbData = mUnicodeData.mCharCount;
			data->string.pwszData = mUnicodeData.mChars;
		}
		else if( mType == cTypeBinary )
		{
			data->binary.cbData = mBinaryData.mByteCount;
			data->binary.pbData = (PBYTE)mBinaryData.mBytes;
		}

	}

	//------------------------------------------------------------------------------
	// tUserInfo
	//------------------------------------------------------------------------------
	void tUserInfo::fRefresh( u32 localHwIndex )
	{
		// Not local, do nothing
		if( localHwIndex >= tUser::cMaxLocalUsers )
			return;

		if( fGetSigninState( localHwIndex ) != eXUserSigninState_NotSignedIn )
		{
#ifdef sig_logging
			tPlatformUserId oldId = mUserId;
			tPlatformUserId oldOfflineId = mOfflineId;
			tLocalizedString oldTag = mGamerTag;
#endif//sig_logging


			DWORD result = XUserGetXUID( localHwIndex, &mUserId );
			sigassert( result == ERROR_SUCCESS );
			sigassert( mUserId != tUser::cInvalidUserId );

			XUSER_SIGNIN_INFO signinInfo;
			result = XUserGetSigninInfo( localHwIndex, XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY , &signinInfo );
			sigassert( result == ERROR_SUCCESS );
			mOfflineId = signinInfo.xuid;

			fRefreshGamerTag( localHwIndex );

#ifdef sig_logging
			bool changeOccurred = false;
			std::stringstream ss;
			ss << "hw" << localHwIndex << " user change:" << std::endl;
			if( mUserId != oldId )
			{
				changeOccurred = true;
				ss << std::hex << "  online  id changed from 0x" << oldId << " to 0x" << mUserId;
			}
			if( mOfflineId != oldOfflineId )
			{
				if( changeOccurred )
					ss << std::endl;
				changeOccurred = true;
				ss << std::hex << "  offline id changed from 0x" << oldOfflineId << " to 0x" << mOfflineId;
			}
			if( mGamerTag.fToCString( ) != oldTag.fToCString( ) )
			{
				if( changeOccurred )
					ss << std::endl;
				changeOccurred = true;
				ss << "  gamertag changed from '" << oldTag << "' to '" << mGamerTag << "'";
			}

			if( changeOccurred )
				log_warning( 0, ss.str( ) );
			else
				log_warning( 0, "hw" << localHwIndex << "'s user has not changed." );
#endif//sig_logging
		}
		else
		{
			fReset( );
#ifdef sig_logging
			log_warning( 0, "hw" << localHwIndex << "'s user reset." );
#endif//sig_logging
		}
	}
	//------------------------------------------------------------------------------
	void tUserInfo::fRefreshGamerTag( u32 localHwIndex )
	{
		char szGamerTag[ XUSER_NAME_SIZE ] = { 0 };
		XUserGetName( localHwIndex, szGamerTag, XUSER_NAME_SIZE );
		szGamerTag[ XUSER_NAME_SIZE - 1 ] = '\0';

		wchar_t wideGamerTag[ XUSER_NAME_SIZE ] = { 0 };
		MultiByteToWideChar( CP_ACP, 0, szGamerTag, -1, wideGamerTag, XUSER_NAME_SIZE );
		wideGamerTag[ XUSER_NAME_SIZE - 1 ] = '\0';
		mGamerTag.fFromCStr( wideGamerTag );
	}

	//------------------------------------------------------------------------------
	// tUser
	//------------------------------------------------------------------------------
	void tUser::fRefreshPlatformId( )
	{
		sigassert( !tApplication::fInstance( ).fSceneGraph( )->fInMTRunList( ) );
		tPlatformUserId beforeId = mUserInfo.mUserId;
		mUserInfo.fRefresh( mLocalHwIndex );
		if( beforeId != mUserInfo.mUserId )
		{
			mUserPic.fRelease( );
		}
	}

	//------------------------------------------------------------------------------
	void tUser::fSetContext( u32 id, u32 value )
	{
		sigassert( fIsLocal( ) && "Cannot set contexts on non-local users" );

		//tOperationPtr operation = fNewOperation( );
		//DWORD result = XUserSetContextEx( mLocalHwIndex, id, value, &operation->mOverlapped );
		//sigassert( result == ERROR_IO_PENDING || result == ERROR_SUCCESS );
		
		XUserSetContext( mLocalHwIndex, id, value );
	}

	//------------------------------------------------------------------------------
	void tUser::fSetProperty( u32 id, const tUserData & value )
	{
		sigassert( fIsLocal( ) && "Cannot set properties on non-local users" );

		tOperationPtr operation = fNewOperation( );
		const u32 dataSize = value.fDataSize( );
		const void * data = value.fDataPtr( );

		if( operation->mData.fCount( ) < dataSize )
			operation->mData.fInitialize( (const byte *)data, dataSize );
		else
			fMemCpy( operation->mData.fBegin( ), data, dataSize );

		DWORD result = XUserSetPropertyEx( 
			mLocalHwIndex, 
			id, 
			dataSize, 
			operation->mData.fBegin( ), 
			&operation->mOverlapped );

		sigassert( result == ERROR_IO_PENDING || result == ERROR_SUCCESS );

		//XUserSetProperty( mLocalHwIndex, id, value.fDataSize( ), value.fDataPtr( ) );
	}

	b32 tUser::fIsGuest( ) const
	{
		if( mLocalHwIndex < cMaxLocalUsers )
			return fIsGuest( mLocalHwIndex );

		return false;
	}

	b32 tUser::fIsOnlineEnabled( ) const
	{
		if( mLocalHwIndex < cMaxLocalUsers )
			return fIsOnlineEnabled( mLocalHwIndex );

		return false;
	}

	b32 tUser::fSignedIn( ) const
	{
		// Report remote users as always signed in
		if( !fIsLocal( ) )
			return true;

		return fSignedIn( mLocalHwIndex );
	}

	b32 tUser::fSignedInOnline( ) const
	{
		// Report remote users as always signed in online
		if( !fIsLocal( ) )
			return true;

		return fGetSigninState( mLocalHwIndex ) == eXUserSigninState_SignedInToLive;
	}

	//------------------------------------------------------------------------------
	tUser::tSignInState tUser::fSignInState( ) const
	{
		switch( fGetSigninState( mLocalHwIndex ) )
		{
		case eXUserSigninState_NotSignedIn:
			return cSignInStateNotSignedIn;
		case eXUserSigninState_SignedInLocally:
			return cSignInStateSignedInLocally;
		case eXUserSigninState_SignedInToLive:
			return cSignInStateSignedInOnline;
		default:
			return cSignInStateNotSignedIn;
		}
	}

	b32 tUser::fHasPrivilege( u32 privilege ) const
	{
		log_assert( fSignedInOnline( ), "tUser::fHasPrivilege() - xbox docs say: 'Specifying an offline user will result in a return value of ERROR_NOT_LOGGED_ON'" );
		BOOL result;
		DWORD error = XUserCheckPrivilege( mLocalHwIndex, ( XPRIVILEGE_TYPE )privilege, &result );
		if( error != ERROR_SUCCESS )
		{
			log_warning( 0, "XUserCheckPrivilege Failed with error code: " << error );
			return false;
		}

		return ( result == TRUE );
	}

	void tUser::fShowSignInUI( ) const
	{
		const DWORD result = XShowSigninUI( 1, XSSUI_FLAGS_ADDUSER );
		if( result != ERROR_SUCCESS	)
		{
			const DWORD lastError = GetLastError( );
			log_warning( 0, "XShowSigninUI Failed with error code: " << result << ", GetLastError(): " << lastError );
		}
		sigassert( result == ERROR_SUCCESS || result == ERROR_ACCESS_DENIED );
	}

	void tUser::fShowAchievementsUI( ) const
	{
		sigassert( fSignedIn( ) );
		const DWORD result = XShowAchievementsUI( mLocalHwIndex );
		sigassert( result == ERROR_SUCCESS );
	}

	void tUser::fShowMarketplaceUI( bool dlc ) const
	{
		sigassert( fSignedIn( ) );
		DWORD result;

		if( dlc )
		{
			// Show offering for new DLCS. (0x5841125F is new additional title id)
			result = XShowMarketplaceUI( mLocalHwIndex, XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTLIST, 0x5841125F4de39000, -1 );

			//ULONGLONG offerIds[ 1 ] = { 0x5841125F00000001 };
			//HRESULT result2;
			//result = XShowMarketplaceDownloadItemsUI( mLocalHwIndex, XSHOWMARKETPLACEDOWNLOADITEMS_ENTRYPOINT_PAIDITEMS, 
			//	offerIds, array_length( offerIds ), &result2, NULL );
		}
		else
		{
			// Show offering for original full version of game. (0x58410AAC is original title id)
			result = XShowMarketplaceUI( mLocalHwIndex, XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTITEM, 0x58410AAC00000001, -1 ); // 0x[TITLEID]00000001 is the full version offer id
		}

		sigassert( result == ERROR_SUCCESS );
	}

	void tUser::fShowGamerCardUI( tPlatformUserId toShow ) const
	{
		sigassert( fSignedIn( ) );
		DWORD result = XShowGamerCardUI( mLocalHwIndex, toShow );
		if( result != ERROR_SUCCESS )
		{
			tScriptVm::fDumpCallstack( );
			log_warning( result == ERROR_SUCCESS, "XShowGamerCardUI ERROR: " << result << " with hw" << mLocalHwIndex << " id: " << toShow );
		}
	}

	void tUser::fShowFriendsUI( ) const
	{
		sigassert( fSignedIn( ) );
		DWORD result = XShowFriendsUI( mLocalHwIndex );
		sigassert( result == ERROR_SUCCESS );
	}

	void tUser::fShowInviteUI( ) const
	{
		sigassert( fSignedIn( ) );
		
		DWORD result;
		if( fIsInActiveParty( ) )
			result = XShowPartyUI( mLocalHwIndex );
		else
			result = XShowFriendsUI( mLocalHwIndex );
		sigassert(result == ERROR_SUCCESS );
	}

	void tUser::fShowCommunitySessionsUI( ) const
	{
		sigassert( fSignedIn( ) );
		sigassert( fIsLocal( ) );

		DWORD result = XShowCommunitySessionsUI( mLocalHwIndex, XSHOWCOMMUNITYSESSION_SHOWPARTY );
		sigassert( SUCCEEDED( result ) );
	}

	b32 tUser::fIsInActiveParty( ) const
	{
		XPARTY_USER_LIST list;
		HRESULT result = XPartyGetUserList( &list );
		
		// Not in a party
		if( result == XPARTY_E_NOT_IN_PARTY )
			return false;

		// Party only has one member, which means it's non-active
		if( list.dwUserCount <= 1 )
			return false;

		// Search for the user index
		for( u32 u = 0; u < list.dwUserCount; ++u )
		{
			// Is a local user and is the correct one
			if( list.Users[u].dwFlags & XPARTY_USER_ISLOCAL )
				if( list.Users[u].dwUserIndex == mLocalHwIndex )
					return true;
		}

		// This user is not in a party
		return false;
	}

	b32 tUser::fReadProfile( byte* data, u32 size )
	{
		if( !fSignedIn( ) )
			return false;

		/*if( size > cSettingsMaxSize )
		{
			log_warning( 0, "Max size that can be read/written to profile settings is: " << cSettingsMaxSize );
			return false;
		}*/

		b32 retval = false;

		const DWORD settingIDs[ ] = { XPROFILE_TITLE_SPECIFIC1, XPROFILE_TITLE_SPECIFIC2, XPROFILE_TITLE_SPECIFIC3 };
		const u32 settingCount = array_length( settingIDs );
		sig_static_assert( settingCount == cTotalSettings );
		DWORD bufferSize = 0;

		//first XUserReadProfileSettings determines buffer size we need
		u32 err = XUserReadProfileSettings( 0, mLocalHwIndex, settingCount, settingIDs, &bufferSize, NULL, NULL );
		sigassert( err == ERROR_INSUFFICIENT_BUFFER );
		sigassert( bufferSize > 0 );

		PXUSER_READ_PROFILE_SETTING_RESULT results = reinterpret_cast< PXUSER_READ_PROFILE_SETTING_RESULT >( NEW byte[ bufferSize ] );

		//second read gets us the data
		err = XUserReadProfileSettings( 0, mLocalHwIndex, settingCount, settingIDs, &bufferSize, results, NULL );
		if( err != ERROR_SUCCESS )
			return false;

		sigassert( results->pSettings );

		u32 start = 0;
		retval = true;
		for( u32 i = 0; i < settingCount; ++i )
		{
			if( results->pSettings[ i ].source == XSOURCE_TITLE )
			{
				const XUSER_DATA& xdata = results->pSettings[ i ].data;		
				if( xdata.type == XUSER_DATA_TYPE_BINARY )
				{		
					u32 size = cSettingsMaxSize;

					sigassert( start + size <= bufferSize );
					fMemCpy( data + start, xdata.binary.pbData, size );
					start += size;
				}
				else
				{
					log_warning( 0, "Error reading profile. Data did not exist." );
					retval = false;
				}
			}
			else
			{
				log_warning( 0, "Error reading profile. Not from title." );
				retval = false;
			}
		}

		delete[] results;

		return retval;
	}

	b32 tUser::fWriteToProfile( byte* data, u32 size )
	{
		if( !fSignedIn( ) )
			return false;

		//if( size > cSettingsMaxSize )
		//{
		//	log_warning( 0, "Max size that can be read/written to profile settings is: " << cSettingsMaxSize << "; tried to write: " << size );
		//	return false;
		//}

		//sigassert( index < cTotalSettings );

		const DWORD settingIDs[ ] = { XPROFILE_TITLE_SPECIFIC1, XPROFILE_TITLE_SPECIFIC2, XPROFILE_TITLE_SPECIFIC3 };

		XUSER_PROFILE_SETTING setting[ cTotalSettings ];

		fZeroOut( setting );

		u32 writeOffset = 0;
		s32 leftToWrite = size;
		u32 index = cSettingsIndex0;

		while( leftToWrite > 0 )
		{
			u32 write = fMin( (s32)cSettingsMaxSize, leftToWrite );
			
			// split up data into each setting
			setting[ index ].source = XSOURCE_TITLE;
			setting[ index ].user.dwUserIndex = mLocalHwIndex;
			setting[ index ].dwSettingId = settingIDs[ index ];
			setting[ index ].data.type = XUSER_DATA_TYPE_BINARY;
			setting[ index ].data.binary.cbData = write;
			setting[ index ].data.binary.pbData = data + writeOffset;

			writeOffset += write;
			leftToWrite -= write;

			if( ++index == cTotalSettings )
				break;
		}

		if( leftToWrite != 0 )
		{
			log_warning( 0, "Error writing to profile!" );
			return false;
		}

		// ship off all settings
		u32 err = XUserWriteProfileSettings( mLocalHwIndex, cTotalSettings, setting, NULL );
		
		if( err != ERROR_SUCCESS )
		{
			log_warning( 0, "Error returned: XUserWriteProfileSettings, " << err );
		}

		return err == ERROR_SUCCESS;
	}

	b32 tUser::fCanWriteToProfile( ) 
	{
		if( !fSignedIn( ) )
			return false;

		tFixedArray<BYTE, 4000> data;
		b32 canRead = fReadProfile( data.fBegin( ), data.fCount( ) );

		if( canRead )
			return true;

		// we can't read, see if we can write (it's safe since there is no valid data to read anyways, we wont be nuking anything.)

		XUSER_PROFILE_SETTING setting[ 1 ];
		fZeroOut( setting );

		BYTE dummyData = 0;

		setting[ 0 ].source = XSOURCE_TITLE;
		setting[ 0 ].user.dwUserIndex = mLocalHwIndex;
		setting[ 0 ].dwSettingId = XPROFILE_TITLE_SPECIFIC1;
		setting[ 0 ].data.type = XUSER_DATA_TYPE_BINARY;
		setting[ 0 ].data.binary.cbData = 1;
		setting[ 0 ].data.binary.pbData = &dummyData;

		// ship off all settings
		u32 err = XUserWriteProfileSettings( mLocalHwIndex, 1, setting, NULL );

		if( err != ERROR_SUCCESS )
		{
			// we were unable to read or write.
			log_warning( 0, "Error returned: XUserWriteProfileSettings, " << err );
			return false;
		}

		// we were unable to read, but able to write, this is a first time player.
		return true;
	}

	b32 tUser::fYAxisInvertedDefault( )
	{
		if( !fSignedIn( ) )
			return false;

		const DWORD settings[ ] = { XPROFILE_GAMER_YAXIS_INVERSION };
		DWORD bufferSize = 0;

		u32 result = XUserReadProfileSettings( 0, mLocalHwIndex, array_length( settings ), settings, &bufferSize, NULL, NULL );
		sigassert( result == ERROR_INSUFFICIENT_BUFFER );

		XUSER_READ_PROFILE_SETTING_RESULT* results = reinterpret_cast<XUSER_READ_PROFILE_SETTING_RESULT*>( NEW byte[ bufferSize ] );

		b32 ret = true;
		result = XUserReadProfileSettings( 0, mLocalHwIndex, array_length( settings ), settings, &bufferSize, results, NULL );

		if( result == ERROR_SUCCESS )
		{
			if( results->pSettings && results->pSettings[ 0 ].data.type == XUSER_DATA_TYPE_INT32 )
				ret = ( results->pSettings[ 0 ].data.nData == XPROFILE_YAXIS_INVERSION_ON );
		}
		else
			ret = false;

		delete[] results;

		return ret;
	}

	b32 tUser::fSouthpawDefault( )
	{
		if( !fSignedIn( ) )
			return false;

		const DWORD settings[ ] = { XPROFILE_GAMER_ACTION_MOVEMENT_CONTROL };
		DWORD bufferSize = 0;

		u32 result = XUserReadProfileSettings( 0, mLocalHwIndex, array_length( settings ), settings, &bufferSize, NULL, NULL );
		sigassert( result == ERROR_INSUFFICIENT_BUFFER );

		XUSER_READ_PROFILE_SETTING_RESULT* results = reinterpret_cast<XUSER_READ_PROFILE_SETTING_RESULT*>( NEW byte[ bufferSize ] );

		b32 ret = true;
		result = XUserReadProfileSettings( 0, mLocalHwIndex, array_length( settings ), settings, &bufferSize, results, NULL );

		if( result == ERROR_SUCCESS )
		{
			if( results->pSettings && results->pSettings[ 0 ].data.type == XUSER_DATA_TYPE_INT32 )
				ret = ( results->pSettings[ 0 ].data.nData == XPROFILE_ACTION_MOVEMENT_CONTROL_R_THUMBSTICK );
		}
		else
			ret = false;

		delete[] results;

		return ret;
	}

	b32 tUser::fAwardAvatar( u32 avatarId )
	{
		if( !fIsLocal( ) )
			return false;

		if( !fSignedIn( ) )
			return false;

		tOperationPtr operation = fNewOperation( );
		if( operation->mData.fCount( ) < sizeof( XUSER_AVATARASSET ) )
			operation->mData.fNewArray( sizeof( XUSER_AVATARASSET ) );
		
		XUSER_AVATARASSET * asset = (XUSER_AVATARASSET *)operation->mData.fBegin( );
		asset->dwAwardId = avatarId;
		asset->dwUserIndex = mLocalHwIndex;

		DWORD result = XUserAwardAvatarAssets( 1, asset, &operation->mOverlapped );
		return ( result == ERROR_IO_PENDING );
	}

	b32 tUser::fAwardGamerPicture( u32 pictureId )
	{
		if( !fIsLocal( ) )
			return false;

		if( !fSignedIn( ) )
			return false;

		tOperationPtr operation = fNewOperation( );

		DWORD result = XUserAwardGamerPicture( mLocalHwIndex, pictureId, 0, &operation->mOverlapped );
		return ( result == ERROR_IO_PENDING );
	}

	b32 tUser::fCheckFullLicenseFlag( )
	{
		u32 mask = 0;
		b32 fullVersion = false;

		// TODO replace NULL with a XOVERLAPPED structure for non blocking calls
		if( XContentGetLicenseMask( (PDWORD)&mask, NULL ) == ERROR_SUCCESS )
		{
			if( (mask & 0x01) == 0x01 )
				fullVersion = true;
		}
		//else
		//	log_warning_nospam( 0, "Could not read license flag!" );

		if( AAACheats_FullVersionOverride )
			fullVersion = AAACheats_FullVersionValue;

		if( AAACheats_SimulatePurchase )
			fullVersion = true;

		return fullVersion;
	}

	//------------------------------------------------------------------------------
	b32 tUser::fIsGuest( u32 localHwIndex )
	{
		sigassert( localHwIndex < cMaxLocalUsers );

		XUSER_SIGNIN_INFO info = { 0 };
		DWORD result = XUserGetSigninInfo( localHwIndex, 0, &info );

		if( result == ERROR_NO_SUCH_USER )
			log_warning( 0, __FUNCTION__ << " error: ERROR_NO_SUCH_USER for hw index " << localHwIndex );
		else if( result != ERROR_SUCCESS )
			log_warning( 0, __FUNCTION__ << " error: " << std::hex << result );

		return ( ( info.dwInfoFlags & XUSER_INFO_FLAG_GUEST ) != 0 );
	}

	//------------------------------------------------------------------------------
	b32 tUser::fIsOnlineEnabled( u32 localHwIndex )
	{
		sigassert( localHwIndex < cMaxLocalUsers );

		XUSER_SIGNIN_INFO info = { 0 };
		DWORD result = XUserGetSigninInfo( localHwIndex, 0, &info );

		if( result != ERROR_SUCCESS )
			log_warning( 0, __FUNCTION__ << " error: " << std::hex << result );

		return ( ( info.dwInfoFlags & XUSER_INFO_FLAG_LIVE_ENABLED ) != 0 );
	}

	//------------------------------------------------------------------------------
	b32 tUser::fSignedIn( u32 localHwIndex )
	{
		// Treat guests as though they're not signed in either
		if( fGetSigninState( localHwIndex ) != eXUserSigninState_NotSignedIn )
			return !fIsGuest( localHwIndex );
		
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tUser::fIsMuted( u32 localHwIndex, tPlatformUserId test )
	{
		sigassert( localHwIndex < tUser::cMaxLocalUsers );

		BOOL isMuted = FALSE;
		DWORD result = XUserMuteListQuery( localHwIndex, test, &isMuted );
		if( result != ERROR_SUCCESS )
			log_warning( 0, __FUNCTION__ << " error: " << std::hex << result );

		return isMuted == TRUE;
	}

	//------------------------------------------------------------------------------
	tPlatformUserId tUser::fGetUserId( u32 localHwIndex )
	{
		sigassert( localHwIndex < tUser::cMaxLocalUsers );

		tPlatformUserId id;
		const DWORD result = XUserGetXUID( localHwIndex, &id );
		sigassert( result == ERROR_SUCCESS );
		
		return id;
	}

	

}
#endif//#if defined( platform_xbox360 )
