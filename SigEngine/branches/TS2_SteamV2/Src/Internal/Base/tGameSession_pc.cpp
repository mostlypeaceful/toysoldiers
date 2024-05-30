#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tGameSession.hpp"
#include "tApplication.hpp"
#include "Net/tHost.hpp"
#include "tLeaderboard.hpp"

namespace Sig
{
	const u32 tGameSession::cCreateUsesPresence					= 0x00000002;
	const u32 tGameSession::cCreateUsesStats					= 0x00000004;
	const u32 tGameSession::cCreateUsesMatchmaking				= 0x00000008;
	const u32 tGameSession::cCreateUsesArbitration				= 0x00000010;
	const u32 tGameSession::cCreateUsesPeerNetwork				= 0x00000020;
	const u32 tGameSession::cCreateInvitesDisabled				= 0x00000100;
	const u32 tGameSession::cCreateJoinViaPresenceDisabled		= 0x00000200;
	const u32 tGameSession::cCreateJoinViaPresenceFriendsOnly	= 0x00000800;
	const u32 tGameSession::cCreateJoinInProgressDisabled		= 0x00000400;

	const u32 tGameSession::cCreateSinglePlayerWithStats	= tGameSession::cCreateUsesPresence | tGameSession::cCreateUsesStats | tGameSession::cCreateInvitesDisabled | tGameSession::cCreateJoinViaPresenceDisabled | tGameSession::cCreateJoinInProgressDisabled;
	const u32 tGameSession::cCreateMultiplayerStandard		= tGameSession::cCreateUsesPresence | tGameSession::cCreateUsesStats | tGameSession::cCreateUsesMatchmaking | tGameSession::cCreateUsesPeerNetwork;
	const u32 tGameSession::cCreateMultiplayerRanked		= tGameSession::cCreateMultiplayerStandard | tGameSession::cCreateUsesArbitration;
	const u32 tGameSession::cCreateSystemLink				= tGameSession::cCreateUsesPeerNetwork;
	const u32 tGameSession::cCreateGroupLobby				= tGameSession::cCreateUsesPresence | tGameSession::cCreateUsesPeerNetwork;
	const u32 tGameSession::cCreateGroupGame				= tGameSession::cCreateUsesStats | tGameSession::cCreateUsesMatchmaking | tGameSession::cCreateUsesPeerNetwork;

#if defined( use_steam )
	u32 tGameSession::mNumWritingInstances = 0;

	//------------------------------------------------------------------------------
	// tGameSessionData
	//------------------------------------------------------------------------------
	tGameSession::tGameSessionData::tGameSessionData( )
		: mNonce( 0 )
		, mFlags( 0 )
		, mCreatorHwIndex( 0 )
		, mApiCall( k_uAPICallInvalid )
	{
	}

	//------------------------------------------------------------------------------
	tGameSession::tGameSessionData::~tGameSessionData( )
	{
		fCancel( );
	}

	//------------------------------------------------------------------------------
	void tGameSession::tGameSessionData::fCancel( )
	{
		if( mApiCall != k_uAPICallInvalid )
		{
			// ...
		}
	}

	//------------------------------------------------------------------------------
	void tGameSession::tGameSessionData::fClear( )
	{
		mApiCall = k_uAPICallInvalid;
		mLastError = k_ESteamAPICallFailureNone;
		mSuccess = false;
		mComplete = true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::tGameSessionData::fOverlapComplete( b32 & success, b32 wait )
	{
		success = true;
		if( mApiCall != k_uAPICallInvalid )
		{
			bool failed = false;
			bool completed = false;
			do
			{
				completed = SteamUtils( )->IsAPICallCompleted( mApiCall, &failed );
			} while ( wait && !completed );
			if( !completed )
				return false;

			success = !failed;
			mLastError = SteamUtils( )->GetAPICallFailureReason( mApiCall );
			log_line( Log::cFlagSession, __FUNCTION__ " mLastError=" << mLastError );
			mApiCall = k_uAPICallInvalid;
		}
		return true;
	}

	//------------------------------------------------------------------------------
	// tGameSession
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
		, mCallbackLobbyCreated( this, &tGameSession::fOnLobbyCreated )
		, mCallbackLobbyJoined( this, &tGameSession::fOnLobbyJoined )
		, mCallbackLobbyChatUpdate( this, &tGameSession::fOnLobbyChatUpdate )
		, mCallbackLobbyGameCreated( this, &tGameSession::fOnLobbyGameCreated )
	{
		fResetSlots( maxPublicSlots, maxPrivateSlots );
		mData.mInfo = info;
	}

	//------------------------------------------------------------------------------
	tGameSession::~tGameSession( )
	{
		b32 success;
		mData.fOverlapComplete( success, true );
	}

	//------------------------------------------------------------------------------
	tAddr tGameSession::fGetAddress( const tGameSessionInfo & info )
	{
		CSteamID address;
		if( SteamMatchmaking( )->GetLobbyGameServer( info.mId, NULL, NULL, &address ) )
			return address.ConvertToUint64( );
		return 0;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fRegisterKey( const tGameSessionInfo & info )
	{
	}

	//------------------------------------------------------------------------------
	void tGameSession::fUnregisterKey( const tGameSessionInfo & info )
	{
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fIsWriteActive( )
	{
		return mNumWritingInstances > 0;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fIsSameSession( const tGameSessionInfo & info ) const
	{
		return mData.mInfo == info;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fCloseSessionData( )
	{
		mData.fCancel( );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fStart( )
	{
		fPushState( cStateStarting );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinLocal( u32 userCount, const u32 users[], const b32 invited[] )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );

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
			log_warning( Log::cFlagSession, __FUNCTION__ " failed because there weren't enough slots" );
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
			userIds[ u ] = users[ u ];

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

		fPushState( cStateJoiningLocalUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveLocal( u32 userCount, const u32 users[] )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );

		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Session can only be left from Created/Started state" );

		// NB If the format of this changes, you must update the code in fOnLeaveRemovingLocal
		mOperationData.fSetCount( sizeof( DWORD ) * userCount );
		DWORD * indices = ( DWORD * )mOperationData.fBegin( );

		for( u32 u = 0; u < userCount; ++u )
			indices[ u ] = users[ u ];

		fPushState( cStateRemovingLocalUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinRemote( u32 userCount, const tPlatformUserId users[], const b32 invited[] )
	{
		log_line( Log::cFlagSession, __FUNCTION__ " user " << users[ 0 ].mId );

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

		fPushState( cStateJoiningRemoteUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveRemote( u32 userCount, const tPlatformUserId users[] )
	{
		log_line( Log::cFlagSession, __FUNCTION__ " user " << users[ 0 ].mId );

		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Sessions can only be left from Created/Started state" );

		// NB If the format of this changes, you must update the code in fOnLeaveRemovingRemote
		mOperationData.fSetCount( sizeof( tPlatformUserId ) * userCount );
		fMemCpy( mOperationData.fBegin( ), users, mOperationData.fTotalSizeOf( ) );

		fPushState( cStateRemovingRemoteUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fArbitrationRegister( )
	{
		log_warning_unimplemented( 0 );
		fPushState( cStateRegistering );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fGetArbitrationResults( tGrowableArray< tPlatformUserId > & out ) const
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fCreate( u32 sessionOwnerId )
	{
		sigassert( mState == cStateNull && "Session can only be created from Null state" );
		log_line( Log::cFlagSession, __FUNCTION__ << " host " << mIsHost );
		mLobbyCreated = true;

		mData.mCreatorHwIndex = sessionOwnerId;
		if( mCreateFlags & cCreateUsesPeerNetwork )
		{
			mLobbyCreated = false;
			if( mIsHost )
			{
				mData.mApiCall = SteamMatchmaking( )->CreateLobby( k_ELobbyTypePublic, mSlots[ cSlotTotalPublic ] + mSlots[ cSlotTotalPrivate ] );
			}
			else
			{
				mData.mApiCall = SteamMatchmaking( )->JoinLobby( mData.mInfo.mId );
			}
		}

		fPushState( cStateCreating );
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fQueryDetails( )
	{
		if( mCreateFlags & cCreateUsesPeerNetwork )
		{
			auto gameVersionKey = StringUtil::fToString( tUser::cUserPropertyGameVersion );
			auto data = SteamMatchmaking( )->GetLobbyData( mData.mInfo.mId, gameVersionKey.c_str( ) );
			int gameVersion = atoi( data );
			mDetails.mGameVersion = *( u32* )&gameVersion;
			data = SteamMatchmaking( )->GetLobbyData( mData.mInfo.mId, "gameType" );
			int gameType = atoi( data );
			mDetails.mGameType = *( u32* )&gameType;
			data = SteamMatchmaking( )->GetLobbyData( mData.mInfo.mId, "gameMode" );
			int gameMode = atoi( data );
			mDetails.mGameMode = *( u32* )&gameMode;
			mDetails.mHostUserIndex = mData.mCreatorHwIndex;
		}
		else
		{
			// Set gameType and gameMode for session searches
			auto user = tApplication::fInstance( ).fLocalUsers( )[ mData.mCreatorHwIndex ];
			u32 gameType = 0;
			u32 gameMode = ~0;
			if( user->fGetContext( tUser::cUserContextGameType, gameType ) )
			{
				mDetails.mGameType = gameType;
			}
			if( user->fGetContext( tUser::cUserContextGameMode, gameMode ) )
			{
				mDetails.mGameMode = gameMode;
			}
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fCheckDetailState( u32 targetState )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	tAddr tGameSession::fAddress( ) const
	{
		// If we're the host and we're not created yet then we don't have an addy
		if( mIsHost && mState < cStateCreated ) 
			return 0;

		return fGetAddress( mData.mInfo );
	}

	//------------------------------------------------------------------------------
	std::string tGameSession::fName( ) const
	{
		std::stringstream ss;
		ss << std::hex << mData.mInfo.mId.ConvertToUint64( );
		return ss.str( );
	}

	//------------------------------------------------------------------------------
	u32 tGameSession::fLastError( ) const
	{
		// k_ESteamAPICallFailureNone is -1 and the next values are errors, so to map to ERROR_SUCCESS being zero, we add 1
		return mData.mLastError + 1;
	}

	//------------------------------------------------------------------------------
	u64 tGameSession::fNonce( ) const
	{
		return mData.mNonce;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fEnd( )
	{
		fPushState( cStateEnding );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fWriteStats( 
		tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		log_line( Log::cFlagSession, __FUNCTION__ << " with " << writeCount << " boards" );

		u32 propertyCount = 0;
		for( u32 i = 0; i < writeCount; ++i )
			propertyCount += writes[ i ].mNumProperties;
		mData.mProperties.fNewArray( propertyCount );
		propertyCount = 0;
		mData.mWrites.fNewArray( writeCount );
		for( u32 i = 0; i < writeCount; ++i )
		{
			mData.mWrites[ i ].mViewId = writes[ i ].mViewId;
			mData.mWrites[ i ].mNumProperties = writes[ i ].mNumProperties;
			mData.mWrites[ i ].mProperties = mData.mProperties.fBegin( ) + propertyCount;
			for( u32 j = 0; j < writes[ i ].mNumProperties; ++j )
				mData.mProperties[ propertyCount++ ] = writes[ i ].mProperties[ j ];
		}
		mData.mCurrentWrite = 0;

		// Read the current leaderboard rows
		mLeaderboard.fReset( NEW tLeaderboard( ) );
		for( u32 i = 0; i < writeCount; ++i )
			mLeaderboard->fAddBoardToRead( mData.mWrites[ i ].mViewId );
		mLeaderboard->fSelectBoard( writes[ 0 ].mViewId );
		mLeaderboard->fReadByPlatformId( &userId, 1 );
		mData.mSubState = cSubStateReading;
		fPushState( cStateWritingStats );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fAdvanceLeaderboardRead( )
	{
		if( mData.mSubState == cSubStateReading )
		{
			// Return if we haven't finished reading yet
			if( !mLeaderboard->fAdvanceRead( ) )
				return false;

			if( mLeaderboard->fState( ) == tLeaderboard::cStateSuccess )
			{
				// Aggregate columns for all boards we are writing
				for( u32 i = 0; i < mData.mWrites.fCount( ); ++i )
				{
					auto board = tLeaderboard::fFindLeaderboardByViewId( mData.mWrites[ i ].mViewId );
					if( !board )
					{
						log_warning( Log::cFlagNone, __FUNCTION__ " Failed to find leaderboard for view Id " << mData.mWrites[ i ].mViewId );
					}
					sigassert( board && "Failed to find leaderboard for the view Id" );
					if( !board )
						continue;
					log_line( Log::cFlagNone, "Aggregating board " << board->mSpec.mViewName );
					mLeaderboard->fSelectBoard( board->fBoardId( ) );
					mData.mWrites[ i ].mAggregated.fNewArray( board->mSpec.dwNumColumnIds );
					if( mLeaderboard->fRowsAvailable( ) == 1 )
					{
						// Fill in with current values of leaderboard row
						// -2 and +2 to avoid the virtual rank and gamername columns
						for( u32 j = 0; j < mLeaderboard->fColumnCount( ) - 2; ++j )
							mData.mWrites[ i ].mAggregated[ j ] = mLeaderboard->fData( j + 2, 0 );
						// Aggregate submitted properties
						for( u32 j = 0; j < mData.mWrites[ i ].mNumProperties; ++j )
						{
							auto propertyId = mData.mWrites[ i ].mProperties[ j ].mId;
							auto columnId = mLeaderboard->fPropertyIdToColumnId( propertyId );
							// They write a KILLS property to the level leaderboard that doesn't use the KILLS property on any column. Ignore it.
							if( columnId != ~0 )
							{
								// Find the aggregate type for this column
								for( u32 c = 0; c < board->mSpec.dwNumColumnIds; ++c )
								{
									if( board->mSpec.wColumnId[ c ] == columnId )
									{
										if( board->mColumnDescs[ c ].mUserData == 1 /* GameFlags::cLEADERBOARD_COLUMN_TYPE_TIME in GameApp */ )
										{
											// Property is a float
											auto newValue = mData.mWrites[ i ].mProperties[ j ].mData.fGetAsF64( );
											auto oldValue = mLeaderboard->fDataById( columnId, 0 ).fGetAsF64( );

											switch( board->mColumnDescs[ c ].mAggregateType )
											{
											case cAggregateLast:
												// No change. Simply write the new value
												break;
											case cAggregateMax:
												// Write the highest of the old and new values
												if( oldValue > newValue )
													newValue = oldValue;
												break;
											case cAggregateSum:
												// Add the new value to the old value
												newValue += oldValue;
												break;
											}

											auto index = mLeaderboard->fIndexOfColumnId( columnId );
											if( index != ~0 )
												mData.mWrites[ i ].mAggregated[ index ].fSet( newValue );
										}
										else
										{
											// Property is an integer
											auto newValue = mData.mWrites[ i ].mProperties[ j ].mData.fGetAsS64( );
											auto oldValue = mLeaderboard->fDataById( columnId, 0 ).fGetAsS64( );

											switch( board->mColumnDescs[ c ].mAggregateType )
											{
											case cAggregateLast:
												// No change. Simply write the new value
												break;
											case cAggregateMax:
												// Write the highest of the old and new values
												if( oldValue > newValue )
													newValue = oldValue;
												break;
											case cAggregateSum:
												// Add the new value to the old value
												newValue += oldValue;
												break;
											}

											auto index = mLeaderboard->fIndexOfColumnId( columnId );
											if( index != ~0 )
												mData.mWrites[ i ].mAggregated[ index ].fSet( newValue );
										}
										break;
									}
								}
							}
						}
					}
					else
					{
						// Fill in with current values of leaderboard row
						// Zero out properties. -2 to remove first two virtual columns
						for( u32 j = 0; j < mLeaderboard->fColumnCount( ) - 2; ++j )
							mData.mWrites[ i ].mAggregated[ j ].fSet( ( s64 )0 );
						// Set submitted properties
						for( u32 j = 0; j < mData.mWrites[ i ].mNumProperties; ++j )
						{
							auto propertyId = mData.mWrites[ i ].mProperties[ j ].mId;
							auto columnId = mLeaderboard->fPropertyIdToColumnId( propertyId );
							// They write a KILLS property to the level leaderboard that doesn't use the KILLS property on any column. Ignore it.
							if( columnId != ~0 )
							{
								auto index = mLeaderboard->fIndexOfColumnId( columnId );
								if( index != ~0 )
								{
									// Find the aggregate type for this column
									for( u32 c = 0; c < board->mSpec.dwNumColumnIds; ++c )
									{
										if( board->mSpec.wColumnId[ c ] == columnId )
										{
											if( board->mColumnDescs[ c ].mUserData == 1 /* GameFlags::cLEADERBOARD_COLUMN_TYPE_TIME in GameApp */ )
											{
												auto newValue = mData.mWrites[ i ].mProperties[ j ].mData.fGetAsF64( );
												mData.mWrites[ i ].mAggregated[ index ].fSet( newValue );
											}
											else
											{
												auto newValue = mData.mWrites[ i ].mProperties[ j ].mData.fGetAsS64( );
												mData.mWrites[ i ].mAggregated[ index ].fSet( newValue );
											}
											break;
										}
									}
								}
							}
						}
					}
				}
				mData.mSubState = cSubStateWritePrepared;
			}
			else
			{
				log_warning( Log::cFlagNone, __FUNCTION__ " Leaderboard read failed" );
				mData.mSubState = cSubStateNone;
			}
			mLeaderboard.fRelease( );
		}

		if( mData.mSubState == cSubStateWritePrepared )
		{
			// Wait for leaderboard reads to complete before trying to write
			if( tLeaderboard::fIsReadActive( ) )
				return false;

			// Start write to leaderboards
			auto board = tLeaderboard::fFindLeaderboardByViewId( mData.mWrites[ mData.mCurrentWrite ].mViewId );
			sigassert( board && "Failed to find leaderboard for the view Id" );
			if( board )
			{
				auto handle = tLeaderboard::fGetHandle( board->mSpec.mViewName );
				if( handle )
				{
					log_line( Log::cFlagNone, __FUNCTION__ << " Found cached leaderboard handle for " << board->mSpec.mViewName );
					LeaderboardFindResult_t result;
					result.m_bLeaderboardFound = 1;
					result.m_hSteamLeaderboard = handle;
					fOnFindLeaderboard( &result, false );
				}
				else
				{
					log_line( Log::cFlagNone, __FUNCTION__ << " Finding/creating leaderboard " << board->mSpec.mViewName );
					mData.mApiCall = SteamUserStats( )->FindOrCreateLeaderboard( board->mSpec.mViewName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
					mCallbackFindLeaderboard.Set( mData.mApiCall, this, &tGameSession::fOnFindLeaderboard );
				}
				++mNumWritingInstances;
				mData.mSubState = cSubStateWriting;
			}
			else
			{
				mData.mSubState = cSubStateNone;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fFlushStats( )
	{
		fPushState( cStateFlushingStats );
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnFindLeaderboard( LeaderboardFindResult_t *pResult, bool bIOFailure )
	{
		auto board = tLeaderboard::fFindLeaderboardByViewId( mData.mWrites[ mData.mCurrentWrite ].mViewId );

		if( !pResult->m_bLeaderboardFound )
		{
			mData.mLastError = SteamUtils( )->GetAPICallFailureReason( mData.mApiCall );
			mData.mApiCall = k_uAPICallInvalid;
			log_warning( Log::cFlagNone, "Could not find leaderboard " << board->mSpec.mViewName );
			mData.mWrites.fDeleteArray( );
			mData.mProperties.fDeleteArray( );
			mData.mCurrentWrite = 0;
			mData.mSubState = cSubStateNone;
			--mNumWritingInstances;
			return;
		}
		// Steam appears to lie. It returns true for bIOFailure, but the entry was written to the leaderboard as seen on the Steamworks site.
		// For now we ignore it and carry on.
		/*
		else if( bIOFailure )
		{
			mData.mLastError = SteamUtils( )->GetAPICallFailureReason( mData.mApiCall );
			mData.mApiCall = k_uAPICallInvalid;
			log_warning( Log::cFlagNone, "I/O failure finding leaderboard " << board->mSpec.mViewName );
			mData.mWrites.fDeleteArray( );
			mData.mProperties.fDeleteArray( );
			mData.mCurrentWrite = 0;
			mData.mSubState = cSubStateNone;
			--mNumWritingInstances;
			return;
		}
		*/
		mData.fClear( );

		tLeaderboard::fAddHandle( board->mSpec.mViewName, pResult->m_hSteamLeaderboard );
		const char* boardName = SteamUserStats( )->GetLeaderboardName( pResult->m_hSteamLeaderboard );
		// Board has been found. Fill out the binary data for the row
		s32 score = 0;
		u32 count = mData.mWrites[ mData.mCurrentWrite ].mAggregated.fCount( );
		mData.mWriteBuffer.fNewArray( count );
		for( u32 i = 0; i < count; ++i )
		{
			if( mData.mWrites[ mData.mCurrentWrite ].mAggregated[ i ].fIsInt( ) )
			{
				auto value = ( s32 )mData.mWrites[ mData.mCurrentWrite ].mAggregated[ i ].fGetAsS64( );
				mData.mWriteBuffer[ i ] = value;
				// Use the first property as the score
				if( i == 0 )
					score = value;
			}
			else
			{
				auto value = ( f32 )mData.mWrites[ mData.mCurrentWrite ].mAggregated[ i ].fGetAsF64( );
				mData.mWriteBuffer[ i ] = *( s32* )&value;
			}
		}

		// Upload the score
		log_line( Log::cFlagNone, __FUNCTION__ << " Found leaderboard " << boardName << ". Beginning upload of score " <<
			score << " with " << mData.mWriteBuffer.fCount( ) << " items." );
		mData.mApiCall = SteamUserStats( )->UploadLeaderboardScore( pResult->m_hSteamLeaderboard, k_ELeaderboardUploadScoreMethodForceUpdate, score, mData.mWriteBuffer.fBegin( ), mData.mWriteBuffer.fCount( ) );
		mCallbackLeaderboardUploaded.Set( mData.mApiCall, this, &tGameSession::fOnLeaderboardUploaded );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaderboardUploaded( LeaderboardScoreUploaded_t *pResult, bool bIOFailure )
	{
		auto board = tLeaderboard::fFindLeaderboardByViewId( mData.mWrites[ mData.mCurrentWrite ].mViewId );

		// Steam appears to lie. It returns true for bIOFailure and false for m_bSuccess, but the entry was written
		// to the leaderboard as seen on the Steamworks site. For now we ignore it and carry on.
		//if( pResult->m_bSuccess )
		{
			mData.fClear( );
			log_line( Log::cFlagNone, __FUNCTION__ << " Uploaded score to leaderboard " << board->mSpec.mViewName );
		}
		/*
		else if( bIOFailure )
		{
			mData.mLastError = SteamUtils( )->GetAPICallFailureReason( mData.mApiCall );
			mData.mApiCall = k_uAPICallInvalid;
			log_warning( Log::cFlagNone, "I/O failure uploading to leaderboard " << board->mSpec.mViewName );
		}
		else
		{
			mData.mLastError = SteamUtils( )->GetAPICallFailureReason( mData.mApiCall );
			mData.mApiCall = k_uAPICallInvalid;
			log_warning( Log::cFlagNone, "Steam reported not successful uploading to leaderboard " << board->mSpec.mViewName );
		}
		*/

		mData.mWriteBuffer.fDeleteArray( );
		++mData.mCurrentWrite;
		if( mData.mCurrentWrite < mData.mWrites.fCount( ) )
		{
			// Write the next board
			auto board = tLeaderboard::fFindLeaderboardByViewId( mData.mWrites[ mData.mCurrentWrite ].mViewId );
			sigassert( board && "Failed to find leaderboard for the view Id" );
			if( board )
			{
				auto handle = tLeaderboard::fGetHandle( board->mSpec.mViewName );
				if( handle )
				{
					log_line( Log::cFlagNone, __FUNCTION__ << " Found cached leaderboard handle for " << board->mSpec.mViewName );
					LeaderboardFindResult_t result;
					result.m_bLeaderboardFound = 1;
					result.m_hSteamLeaderboard = handle;
					fOnFindLeaderboard( &result, false );
				}
				else
				{
					log_line( Log::cFlagNone, __FUNCTION__ << " Finding/creating leaderboard " << board->mSpec.mViewName );
					mData.mApiCall = SteamUserStats( )->FindOrCreateLeaderboard( board->mSpec.mViewName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
					mCallbackFindLeaderboard.Set( mData.mApiCall, this, &tGameSession::fOnFindLeaderboard );
				}
			}
		}
		else
		{
			// No more boards to write
			mData.mWrites.fDeleteArray( );
			mData.mProperties.fDeleteArray( );
			mData.mCurrentWrite = 0;
			mData.mSubState = cSubStateNone;
			--mNumWritingInstances;
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fDelete( )
	{
		sigassert( ( mState == cStateStarted || mState == cStateCreated ) && "Delete can only be called from Created or Started state" );
		log_line( Log::cFlagSession, __FUNCTION__ << " lobbyID " << mData.mInfo.mId.ConvertToUint64( ) );

		if( mCreateFlags & cCreateUsesPeerNetwork )
		{
			SteamMatchmaking( )->LeaveLobby( mData.mInfo.mId );
		}

		fPushState( cStateDeleting );

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fOperationComplete( b32 & success, b32 wait )
	{
		if( mState == cStateWritingStats || mState == cStateFlushingStats )
		{
			if( mData.mSubState == cSubStateReading || mData.mSubState == cSubStateWritePrepared )
			{
				fAdvanceLeaderboardRead( );
				return false;
			}
		}

		if( mState == cStateCreating  && ( mIsHost && !mLobbyCreated ) )
		{
			return false;
		}


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
	void tGameSession::fOnLobbyCreated( LobbyCreated_t* param )
	{
		if( param->m_eResult == k_EResultOK )
		{
			log_line( Log::cFlagSession, __FUNCTION__ << " lobbyID " << param->m_ulSteamIDLobby );
			mData.mInfo.mId = param->m_ulSteamIDLobby;

			// Set gameType and gameMode for session searches
			auto user = tApplication::fInstance( ).fLocalUsers( )[ mData.mCreatorHwIndex ];
			u32 gameType = 0;
			u32 gameMode = ~0;
			char buffer[32];
			if( user->fGetContext( tUser::cUserContextGameType, gameType ) )
				SteamMatchmaking( )->SetLobbyData( mData.mInfo.mId, "gameType", _itoa( *( int* )&gameType, buffer, 10 ) );
			if( user->fGetContext( tUser::cUserContextGameMode, gameMode ) )
				SteamMatchmaking( )->SetLobbyData( mData.mInfo.mId, "gameMode", _itoa( *( int* )&gameMode, buffer, 10 ) );
			// Set the properties for other player's game searches
			auto properties = user->fGetProperties( );
			for( auto iter = properties.fBegin( ); iter != properties.fEnd( ); ++iter )
			{
				auto userProp = *iter;
				if( !userProp.fNullOrRemoved( ) )
				{
					auto key = StringUtil::fToString( userProp.mKey );
					std::string value = StringUtil::fToString( ( s32 )userProp.mValue.fGetAsS64( ) );
					SteamMatchmaking( )->SetLobbyData( mData.mInfo.mId, key.c_str( ), value.c_str( ) );
					log_line( Log::cFlagSession, "Session property " << key << " value " << value );
				}
			}
			// Set the host for connection
			SteamMatchmaking( )->SetLobbyGameServer( mData.mInfo.mId, 0, 0, SteamUser( )->GetSteamID( ) );
			mLobbyCreated = true;
		}
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLobbyJoined( LobbyEnter_t* param )
	{
		log_line( Log::cFlagSession, __FUNCTION__ << " lobbyID " << param->m_ulSteamIDLobby );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLobbyChatUpdate( LobbyChatUpdate_t* param )
	{
		log_line( Log::cFlagSession, __FUNCTION__ << " lobbyID " << param->m_ulSteamIDLobby << " change " << param->m_rgfChatMemberStateChange << " steamID " << param->m_ulSteamIDUserChanged );
		if( param->m_rgfChatMemberStateChange == k_EChatMemberStateChangeEntered )
		{
		}
		else if( BChatMemberStateChangeRemoved( param->m_rgfChatMemberStateChange ) )
		{
		}
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLobbyGameCreated( LobbyGameCreated_t* param )
	{
		log_line( Log::cFlagSession, __FUNCTION__ << " lobbyID " << param->m_ulSteamIDLobby << " server " << param->m_ulSteamIDGameServer );
		// Everyone connect to the game server
		if( CSteamID( param->m_ulSteamIDGameServer ) != SteamUser( )->GetSteamID( ) )
		{

		}
	}

#else//#if defined( use_steam )
	u32 tGameSession::fGetAddress( const tGameSessionInfo & info )
	{
		log_warning_unimplemented( 0 );
		return 0;
	}

	void tGameSession::fRegisterKey( const tGameSessionInfo & info )
	{
		log_warning_unimplemented( 0 );
	}

	void tGameSession::fUnregisterKey( const tGameSessionInfo & info )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	tGameSession::tGameSessionData::tGameSessionData( )
		: mNonce( 0 )
		, mFlags( 0 )
	{
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
	{
		log_warning_unimplemented( 0 );
		fResetSlots( maxPublicSlots, maxPrivateSlots );
	}

	//------------------------------------------------------------------------------
	tGameSession::~tGameSession( )
	{
		log_warning_unimplemented( 0 );
	}

	b32 tGameSession::fIsSameSession( const tGameSessionInfo & info ) const
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fCloseSessionData( )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fStart( )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinLocal( u32 userCount, const u32 users[], const b32 invited[] )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveLocal( u32 userCount, const u32 users[] )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinRemote( u32 userCount, const tPlatformUserId users[], const b32 invited[] )
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveRemote( u32 userCount, const tPlatformUserId users[] )
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fArbitrationRegister( )
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fGetArbitrationResults( tGrowableArray< tPlatformUserId > & out ) const
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fCreate( u32 sessionOwnerId )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fQueryDetails( )
	{
		log_warning_unimplemented( 0 );
		mDetails.mGameType = tUser::cUserContextGameTypeStandard;
		mDetails.mGameMode = ~0;
		mDetails.mHostUserIndex = mData.mCreatorHwIndex;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fCheckDetailState( u32 targetState )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	u32 tGameSession::fAddress( ) const
	{
		log_warning_unimplemented( 0 );
		return 0;
	}

	//------------------------------------------------------------------------------
	std::string tGameSession::fName( ) const
	{
		log_warning_unimplemented( 0 );
		return std::string( );
	}

	//------------------------------------------------------------------------------
	u32 tGameSession::fLastError( ) const
	{
		log_warning_unimplemented( 0 );
		return 0;
	}

	//------------------------------------------------------------------------------
	u64 tGameSession::fNonce( ) const
	{
		log_warning_unimplemented( 0 );
		return mData.mNonce;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fEnd( )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fWriteStats( 
		tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		log_warning_unimplemented( 0 );
		return true;
	}


	//------------------------------------------------------------------------------
	b32 tGameSession::fFlushStats( )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fDelete( )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fOperationComplete( b32 & success, b32 wait )
	{
		log_warning_unimplemented( 0 );
		success = true;
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveJoiningLocal( b32 success )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveRemovingLocal( b32 success )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveJoiningRemote( b32 success )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveRemovingRemote( b32 success )
	{
		log_warning_unimplemented( 0 );
	}
#endif//#if defined( use_steam )
}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
