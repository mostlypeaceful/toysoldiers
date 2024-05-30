#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tGameSession.hpp"
#include "tApplication.hpp"

namespace Sig
{
	const u32 tGameSession::cCreateUsesPresence					= XSESSION_CREATE_USES_PRESENCE;
	const u32 tGameSession::cCreateUsesStats					= XSESSION_CREATE_USES_STATS;
	const u32 tGameSession::cCreateUsesMatchmaking				= XSESSION_CREATE_USES_MATCHMAKING;
	const u32 tGameSession::cCreateUsesArbitration				= XSESSION_CREATE_USES_ARBITRATION;
	const u32 tGameSession::cCreateUsesPeerNetwork				= XSESSION_CREATE_USES_PEER_NETWORK;
	const u32 tGameSession::cCreateInvitesDisabled				= XSESSION_CREATE_INVITES_DISABLED;
	const u32 tGameSession::cCreateJoinViaPresenceDisabled		= XSESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED;
	const u32 tGameSession::cCreateJoinViaPresenceFriendsOnly	= XSESSION_CREATE_JOIN_VIA_PRESENCE_FRIENDS_ONLY;
	const u32 tGameSession::cCreateJoinInProgressDisabled		= XSESSION_CREATE_JOIN_IN_PROGRESS_DISABLED;

	const u32 tGameSession::cCreateSinglePlayerWithStats	= XSESSION_CREATE_SINGLEPLAYER_WITH_STATS;
	const u32 tGameSession::cCreateMultiplayerStandard		= XSESSION_CREATE_LIVE_MULTIPLAYER_STANDARD;
	const u32 tGameSession::cCreateMultiplayerRanked		= XSESSION_CREATE_LIVE_MULTIPLAYER_RANKED;
	const u32 tGameSession::cCreateSystemLink				= XSESSION_CREATE_SYSTEMLINK;
	const u32 tGameSession::cCreateGroupLobby				= XSESSION_CREATE_GROUP_LOBBY;
	const u32 tGameSession::cCreateGroupGame				= XSESSION_CREATE_GROUP_GAME;

	//------------------------------------------------------------------------------
	// tGameSessionData
	//------------------------------------------------------------------------------
	tGameSession::tGameSessionData::tGameSessionData( )
		: mHandle( INVALID_HANDLE_VALUE )
		, mFlags( 0 )
		, mNonce( 0 )
	{
		fZeroOut( mOverlapped );
		fZeroOut( mInfo );

		mOverlapped.hEvent = CreateEvent( 0, true, false, 0 );
	}

	//------------------------------------------------------------------------------
	tGameSession::tGameSessionData::~tGameSessionData( )
	{
		CloseHandle( mOverlapped.hEvent );
	}

	//------------------------------------------------------------------------------
	void tGameSession::tGameSessionData::fCloseHandle( )
	{
		if( mHandle != INVALID_HANDLE_VALUE )
		{
			XCloseHandle( mHandle );
			mHandle = INVALID_HANDLE_VALUE;
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::tGameSessionData::fOverlapComplete( b32 & success, b32 wait )
	{
		DWORD ignored;
		DWORD testResult = XGetOverlappedResult( &mOverlapped, &ignored, wait ? TRUE : FALSE );
		if( testResult == ERROR_IO_INCOMPLETE )
			return false;

		success = ( testResult == ERROR_SUCCESS );
		if( !success  )
			mLastError = XGetOverlappedExtendedError( &mOverlapped );
		else
			mLastError = ERROR_SUCCESS;

		HANDLE e = mOverlapped.hEvent;
		fZeroOut( mOverlapped );
		mOverlapped.hEvent = e;
		ResetEvent( mOverlapped.hEvent );
		return true;
	}


	//------------------------------------------------------------------------------
	// tGameSession
	//------------------------------------------------------------------------------
	u32 tGameSession::fGetAddress( const tGameSessionInfo & info )
	{
		IN_ADDR addr;
		INT error = XNetXnAddrToInAddr( &info.hostAddress, &info.sessionID, &addr );
		if( error )
		{
			log_warning( 0, "Failed to convert Session info to ip address: " << error );
			return 0;
		}

		return *(u32*)&addr;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fRegisterKey( const tGameSessionInfo & info )
	{
		INT error = XNetRegisterKey( &info.sessionID, &info.keyExchangeKey );
		if( error )
			log_warning( 0, "Failed to register Session info key: " << error );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fUnregisterKey( const tGameSessionInfo & info )
	{
		INT error = XNetUnregisterKey( &info.sessionID );
		if( error )
			log_warning( 0, "Failed to unregister Session info key: " << error );
	}

	//------------------------------------------------------------------------------
	tGameSession::tGameSession( 
		u32 createFlags,
		const tGameSessionInfo & info,
		u32 maxPublicSlots,
		u32 maxPrivateSlots,
		tStateChangedCallback callback )
		: mState( cStateNull )
		, mIsHost( false )
		, mCreateFlags( createFlags )
		, mStateChangedCallback( callback )
		, mStateTimer( 0.f )
	{
		fResetSlots( maxPublicSlots, maxPrivateSlots );
		mData.mInfo = info;
	}

	//------------------------------------------------------------------------------
	tGameSession::~tGameSession( )
	{
		// Ensure the handles are closed
		mState = cStateNull;
		if( mData.mHandle != INVALID_HANDLE_VALUE )
			fCloseSessionData( );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fIsSameSession( const tGameSessionInfo & info ) const
	{
		return ( fMemCmp( &mData.mInfo.sessionID, &info.sessionID, sizeof( XNKID ) ) == 0 );
	}

	//------------------------------------------------------------------------------
	tGameSession::tNonce tGameSession::fNonce( ) const
	{
		sigassert( ( mIsHost && mState == cStateCreated ) && "Session Nonce only available from created host sessions" );
		return mData.mNonce;
	}

	//------------------------------------------------------------------------------
	u32 tGameSession::fAddress( ) const
	{
		// If we're the host and we're not created yet then we don't have an addy
		if( mIsHost && mState < cStateCreated ) 
			return 0;

		return fGetAddress( mData.mInfo );
	}

	//------------------------------------------------------------------------------
	std::string tGameSession::fName( ) const
	{
		sig_static_assert( sizeof( mData.mInfo.sessionID.ab ) % sizeof( u32 ) == 0 );
		
		u32 id = 0;
		const u32 * idData = (const u32 *)mData.mInfo.sessionID.ab;
		const u32 count = sizeof( mData.mInfo.sessionID.ab ) / sizeof( u32 );
		for( u32 b = 0; b < count; ++b )
			id ^= idData[b ];

		std::stringstream ss; ss << std::hex << id;
		return ss.str( );
	}

	//------------------------------------------------------------------------------
	u32 tGameSession::fLastError( ) const
	{
		return mData.mLastError;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fCreate( u32 sessionOwnerId )
	{
		sigassert( mState == cStateNull && "Session can only be created from Null state" );

		DWORD flags = mCreateFlags;
		if( mIsHost )
			flags |= XSESSION_CREATE_HOST;

		log_line( Log::cFlagSession, "XSessionCreate" );
		DWORD result = XSessionCreate( 
			flags, 
			sessionOwnerId, 
			mSlots[ cSlotTotalPublic ], 
			mSlots[ cSlotTotalPrivate ], 
			&mData.mNonce, 
			&mData.mInfo, 
			&mData.mOverlapped, 
			&mData.mHandle );

		if( result != ERROR_IO_PENDING && result != ERROR_SUCCESS )
		{
			log_warning( 0, "Session create failed with error: " << result );
			return false;
		}

		fPushState( cStateCreating );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fDelete( )
	{
		sigassert( ( mState == cStateStarted || mState == cStateCreated ) && "Delete can only be called from Created or Started state" );

		log_line( Log::cFlagSession, "XSessionDelete" );
		DWORD result = XSessionDelete( mData.mHandle, &mData.mOverlapped );
		if( result != ERROR_SUCCESS && result != ERROR_IO_PENDING )
		{
			log_warning( 0, "Session Delete failed with error: " << result );
			return false;
		}

		fPushState( cStateDeleting );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinLocal( u32 userCount, const u32 indices[], const b32 invited[] )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Sessions can only be joined from Created/Started state" );
		u32 additionalPublic = 0;
		u32 additionalPrivate = 0;

		// Build needed slot counts and test that all the users are signed in
		for( u32 u = 0; u < userCount; ++u )
		{
			if( !tUser::fSignedIn( indices[ u ] ) )
			{
				log_warning( 0, "Join local failed because user was not signed in" );
				return false;
			}

			if( invited[ u ] )
				++additionalPrivate;
			else
				++additionalPublic;
		}

		// Try to fill our slots and get corrected slot counts
		if( !fTryFillSlots( additionalPublic, additionalPrivate ) )
		{
			log_warning( 0, "JoinLocal failed because there weren't enough slots" );
			return false;
		}

		mOperationData.fSetCount( sizeof( u32 ) + userCount * ( sizeof( DWORD ) + sizeof( BOOL ) ) );

		/// !!!NB!!! If the format of this array is changed you must update the fOnLeavingJoinLocal
		*(u32 *)mOperationData.fBegin( ) = userCount; // Makes failure case easier
		DWORD * userIds = (DWORD *)( mOperationData.fBegin( ) + sizeof( u32 ) );
		BOOL * privateSlots = (BOOL *)( userIds + userCount );

		// Assign the user ids and the slot types
		for( u32 u = 0; u < userCount; ++u )
		{
			userIds[ u ] = indices[ u ];

			if( invited[ u ] && additionalPrivate )
			{
				--additionalPrivate;
				privateSlots[ u ] = TRUE;
			}
			else
			{
				sigassert( additionalPublic );
				privateSlots[ u ] = FALSE;
				--additionalPublic;
			}
		}

		// Make the session call
		DWORD result = XSessionJoinLocal( 
			mData.mHandle,
			userCount,
			userIds,
			privateSlots,
			&mData.mOverlapped );

		if( result != ERROR_SUCCESS && result != ERROR_IO_PENDING )
		{
			log_warning( 0, "Session JoinLocal failed with error: " << result );
			return false;
		}

		fPushState( cStateJoiningLocalUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveLocal( u32 userCount, const u32 _indices[] )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Session can only be left from Created/Started state" );

		// NB If the format of this changes, you must update the code in fOnLeaveRemovingLocal
		mOperationData.fSetCount( sizeof( DWORD ) * userCount );
		DWORD * indices = ( DWORD * )mOperationData.fBegin( );

		for( u32 u = 0; u < userCount; ++u )
			indices[ u ] = _indices[ u ];

		DWORD error = XSessionLeaveLocal( mData.mHandle, userCount, indices, &mData.mOverlapped );
		if( error != ERROR_IO_PENDING )
		{
			log_warning( 0, "Session LeaveLocal failed with error: " << error );
			return false;
		}

		fPushState( cStateRemovingLocalUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinRemote( u32 userCount, const tPlatformUserId users[], const b32 invited[] )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Sessions can only be joined from Created/Started state" );

		// Build additional slot counts
		u32 additionalPublic = 0;
		u32 additionalPrivate = 0;
		for( u32 u = 0; u < userCount; ++u )
		{
			if( invited[ u ] )
				++additionalPrivate;
			else
				++additionalPublic;
		}

		// Try to fill the slots and get adjusted counts
		if( !fTryFillSlots( additionalPublic, additionalPrivate) ) 
		{
			log_warning( 0, "Session JoinRemote failed because there weren't enough slots" );
			return false;
		}

		mOperationData.fSetCount( sizeof( u32 ) + userCount * ( sizeof( tPlatformUserId ) + sizeof( BOOL ) ) );

		/// !!!NB!!! If the format of this array is changed you must update the fOnLeavingJoinRemote
		*(u32 *)mOperationData.fBegin( ) = userCount; // Makes failure cause easier
		tPlatformUserId * userIds = (tPlatformUserId *)( mOperationData.fBegin( ) + sizeof( u32 ) );
		BOOL * privateSlots = (BOOL *)( userIds + userCount );

		fMemCpy( userIds, users, sizeof( tPlatformUserId ) * userCount );
		for( u32 u = 0; u < userCount; ++u )
		{
			if( invited[ u ] && additionalPrivate )
			{
				--additionalPrivate;
				privateSlots[ u ] = TRUE;
			}
			else
			{
				sigassert( additionalPublic );
				privateSlots[ u ] = FALSE;
				--additionalPublic;
			}
		}

		DWORD result = XSessionJoinRemote( 
			mData.mHandle,
			userCount,
			userIds,
			privateSlots,
			&mData.mOverlapped );

		if( result != ERROR_SUCCESS && result != ERROR_IO_PENDING )
		{
			log_warning( 0, "Session JoinRemote failed with error: " << result );
			return false;
		}

		fPushState( cStateJoiningRemoteUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveRemote( u32 userCount, const tPlatformUserId users[] )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Sessions can only be left from Created/Started state" );

		// NB If the format of this changes, you must update the code in fOnLeaveRemovingRemote
		mOperationData.fSetCount( sizeof( tPlatformUserId ) * userCount );
		fMemCpy( mOperationData.fBegin( ), users, mOperationData.fTotalSizeOf( ) );
		
		DWORD error = XSessionLeaveRemote( 
			mData.mHandle, 
			userCount, 
			( const tPlatformUserId * )mOperationData.fBegin( ), 
			&mData.mOverlapped );

		if( error != ERROR_IO_PENDING )
		{
			log_warning( 0, "Session LeaveRemote failed with error: " << error );
			return false;
		}

		fPushState( cStateRemovingRemoteUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fArbitrationRegister( )
	{
		sigassert( mState == cStateCreated && "Sessions can only be registered from Created state" );

		DWORD bufferSize;
		DWORD result = XSessionArbitrationRegister( 
			mData.mHandle, 0, mData.mNonce, &bufferSize, NULL, NULL );
		sigassert( result == ERROR_INSUFFICIENT_BUFFER );

		mData.mRegistrationResults.fResize( bufferSize );
		mData.mRegistrationResults.fZeroOut( );

		result = XSessionArbitrationRegister( 
			mData.mHandle, 0, 
			mData.mNonce, 
			&bufferSize, 
			(PXSESSION_REGISTRATION_RESULTS)mData.mRegistrationResults.fBegin( ), 
			&mData.mOverlapped );

		if( result != ERROR_SUCCESS && result != ERROR_IO_PENDING )
		{
			log_warning( 0, "ArbitrationRegister failed with: " << std::hex << result );
			return false;
		}

		fPushState( cStateRegistering );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fGetArbitrationResults( tGrowableArray< tPlatformUserId > & out ) const
	{
		sigassert( mState != cStateRegistering );

		// Not registered
		if( !mData.mRegistrationResults.fCount( ) )
			return false;

		const PXSESSION_REGISTRATION_RESULTS results = 
			( const  PXSESSION_REGISTRATION_RESULTS )mData.mRegistrationResults.fBegin( );

		// Registration failed
		if( !results->wNumRegistrants )
			return false;

		// Successfully registered
		for( u32 r = 0; r < results->wNumRegistrants; ++r )
		{
			const XSESSION_REGISTRANT & registrant = results->rgRegistrants[ r ];
			for( u32 u = 0; u < registrant.bNumUsers; ++u )
			{
				out.fPushBack( registrant.rgUsers[ u ] );
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fStart( )
	{
		sigassert( mState == cStateCreated && "Sessions can only be started from the Created state" );

		log_line( Log::cFlagSession, "XSessionStart" );
		DWORD result = XSessionStart( mData.mHandle, 0, &mData.mOverlapped );
		if( result != ERROR_SUCCESS && result != ERROR_IO_PENDING )
		{
			log_warning( 0, "Session Start failed with error: " << std::hex << result );
			return false;
		}

		fPushState( cStateStarting );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fEnd( )
	{
		if( mState != cStateStarted )
		{
			log_warning( 0, __FUNCTION__ << ": Called with state != Started" );
			return false;
		}

		log_line( Log::cFlagSession, "XSessionEnd" );
		DWORD result = XSessionEnd( mData.mHandle, &mData.mOverlapped );
		if( result != ERROR_SUCCESS && result != ERROR_IO_PENDING )
		{
			log_warning( 0, "Session End failed with error: " << result );
			return false;
		}

		fPushState( cStateEnding );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fWriteStats( tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		sigassert( mState == cStateStarted && "Can only write stats on a running session" );

		// Calculate the total size of the operation data
		u32 operationSize = writeCount * sizeof( XSESSION_VIEW_PROPERTIES );
		for( u32 w = 0; w < writeCount; ++w )
			operationSize += sizeof(XUSER_PROPERTY) * writes[ w ].mNumProperties; 

		mOperationData.fSetCount( operationSize );

		// Reserve the properties space
		byte * data = mOperationData.fBegin( );
		XSESSION_VIEW_PROPERTIES * viewProps = (XSESSION_VIEW_PROPERTIES *)data;
		data += sizeof( XSESSION_VIEW_PROPERTIES ) * writeCount;

		// Copy all the properties
		XUSER_PROPERTY * userProps = (XUSER_PROPERTY *)data;
		for( u32 w = 0; w < writeCount; ++w )
		{
			const tGameSessionViewProperties & in = writes[ w ];
			XSESSION_VIEW_PROPERTIES & out = viewProps[ w ];
			
			out.dwViewId = in.mViewId;
			out.dwNumProperties = in.mNumProperties;
			out.pProperties = userProps;

			log_line( Log::cFlagSession, " --, view: " << out.dwViewId );

			// Cast is validated by sig_static_assert in tUser_xbox360.cpp
			for( u32 p = 0; p < in.mNumProperties; ++p )
			{
				const tUserProperty & inProp = in.mProperties[ p ];
				XUSER_PROPERTY & prop = *userProps++;
				prop.dwPropertyId = inProp.mId;

				inProp.mData.fGetPlatformSpecific( &prop.value );
			}
		}

		// Sanity
		sigassert( userProps == (void*)mOperationData.fEnd( ) );

		// Make the call
		log_line( Log::cFlagSession, "XSessionWriteStats, Writes: " << writeCount << " user: " << userId );
		DWORD error = XSessionWriteStats( mData.mHandle, userId, writeCount, viewProps, &mData.mOverlapped );
		if( error != ERROR_IO_PENDING && error != ERROR_SUCCESS )
		{
			log_warning( 0, "Error " << error << " when writing session stats" );
			return false;
		}

		fPushState( cStateWritingStats );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fFlushStats( )
	{
		sigassert( mState == cStateStarted && "Can only flush stats on a running session" );

		log_line( Log::cFlagSession, "XSessionFlushStats" );
		DWORD error = XSessionFlushStats( mData.mHandle, &mData.mOverlapped );
		if( error != ERROR_SUCCESS && error != ERROR_IO_PENDING )
		{
			log_warning( 0, __FUNCTION__ << " failed with error: " << error );
			return false;
		}

		fPushState( cStateFlushingStats );
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fQueryDetails( )
	{
		sigassert( mState != cStateNull && "QueryDetails called on Null session" );
		sigassert( mState == cStateCreated || mState == cStateStarted );
		
		XSESSION_LOCAL_DETAILS deets;
		DWORD size = sizeof( deets );

		DWORD result = XSessionGetDetails( mData.mHandle, &size, &deets, NULL );
		sigassert( result == ERROR_SUCCESS && "Error querying session details" );

		mDetails.mHostUserIndex = deets.dwUserIndexHost;
		mDetails.mGameType = deets.dwGameType;
		mDetails.mGameMode = deets.dwGameMode;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fCheckDetailState( u32 targetState )
	{
		XSESSION_LOCAL_DETAILS deets;
		DWORD size = sizeof( deets );

		DWORD result = XSessionGetDetails( mData.mHandle, &size, &deets, NULL );
		sigassert( result == ERROR_SUCCESS && "Error querying session details" );

		switch( targetState )
		{
		case cStateCreated:
			return deets.eState == XSESSION_STATE_LOBBY;
		case cStateNull:
			return deets.eState == XSESSION_STATE_DELETED;
		default:
			return false;
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fOperationComplete( b32 & success, b32 wait )
	{
		// Check if the operation is complete
		return mData.fOverlapComplete( success, wait );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveJoiningLocal( b32 success )
	{
		sigassert( mOperationData.fCount( ) >= sizeof( u32 ) );

		const byte * data = mOperationData.fBegin( );
		const u32 userCount = *(const u32*)data;

		sigassert( mOperationData.fCount( ) == sizeof( u32 ) + userCount * ( sizeof( DWORD ) + sizeof( BOOL ) ) );

		const DWORD * userIds = (const DWORD*)( data + sizeof( u32 ) );
		const BOOL * privateSlots = (const BOOL*)( userIds + userCount );

		if( success )
		{
			// Store information about the new members
			for( u32 u = 0; u < userCount; ++u )
			{
				tSessionUser newUser;
				newUser.mIsLocal = true;
				newUser.mLocalHwIndex = userIds[ u ];
				newUser.mInPrivateSlot = privateSlots[ u ];

				mUsers.fPushBack( newUser );
			}
		}
		else
		{
			// Update our slots to reflect the failed addition
			for( u32 u = 0; u < userCount; ++u )
				--mSlots[ privateSlots[ u ] ? cSlotFilledPrivate : cSlotFilledPublic ];
		}
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveRemovingLocal( b32 success )
	{
		sigassert( !( mOperationData.fCount( ) % sizeof( DWORD ) ) );

		u32 userCount = mOperationData.fCount( ) / sizeof( DWORD );
		const DWORD * indices = (const DWORD*)mOperationData.fBegin( );

		if( success )
		{
			// Clear out the users and update the slots
			for( u32 u = 0; u < userCount; ++u )
			{
				const u32 sUserCount = mUsers.fCount( );
				for( u32 su = 0; su < sUserCount; ++su )
				{
					if( !mUsers[ su ].mIsLocal )
						continue;

					if( mUsers[ su ].mLocalHwIndex != indices[ u ] )
						continue;

					// Update the slot
					--mSlots[ mUsers[ su ].mInPrivateSlot ? cSlotFilledPrivate : cSlotFilledPublic ];

					// Found it!
					mUsers.fErase( su );
					break;
				}
				if( sUserCount != mUsers.fCount( ) + 1 )
					log_warning( 0, "Couldn't find removed user in user array" );
			}
		}
	}


	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveJoiningRemote( b32 success )
	{
		sigassert( mOperationData.fCount( ) >= sizeof( u32 ) );

		const u32 userCount = *(const u32*)mOperationData.fBegin( );

		sigassert( mOperationData.fCount( ) == sizeof( u32 ) + userCount * ( sizeof( tPlatformUserId ) + sizeof( BOOL ) ) );

		const tPlatformUserId * userIds = (const tPlatformUserId*)( mOperationData.fBegin( ) + sizeof( u32 ) );
		const BOOL * privateSlots = (const BOOL*)( userIds + userCount );

		if( success )
		{
			// Add the new user to the user array
			for( u32 u = 0; u < userCount; ++u )
			{
				tSessionUser newUser;
				newUser.mIsLocal = false;
				newUser.mUserId = userIds[ u ];
				newUser.mInPrivateSlot = privateSlots[ u ];

				mUsers.fPushBack( newUser );
			}
		}
		else
		{
			// Update our slots to reflect the failed addition
			for( u32 u = 0; u < userCount; ++u )
				--mSlots[ privateSlots[ u ] ? cSlotFilledPrivate : cSlotFilledPublic ];
		}
	}
	
	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveRemovingRemote( b32 success )
	{
		sigassert( !( mOperationData.fCount( ) % sizeof( tPlatformUserId )  ) );

		const u32 userCount = mOperationData.fCount( ) /  sizeof( tPlatformUserId );
		const tPlatformUserId * ids = (const tPlatformUserId *)mOperationData.fBegin( );

		if( success )
		{
			for( u32 u = 0; u < userCount; ++u )
			{
				const u32 sUserCount = mUsers.fCount( );
				for( u32 su = 0; su < sUserCount; ++su )
				{
					if( mUsers[ su ].mIsLocal )
						continue;

					if( mUsers[ su ].mUserId != ids[ u ] )
						continue;

					// Update the slot
					--mSlots[ mUsers[ su ].mInPrivateSlot ? cSlotFilledPrivate : cSlotFilledPublic ];

					// Found it!
					mUsers.fErase( su );
					break;
				}
				sigassert( sUserCount == mUsers.fCount( ) + 1 && "Couldn't find removed user in user array" );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGameSession::fCloseSessionData( )
	{
		sigassert( mState == cStateNull && "Sanity: ClearSessionData called with non-Null state" );
		mData.fCloseHandle( );
	}
}
#endif//#if defined( platform_xbox360 )

