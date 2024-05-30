//------------------------------------------------------------------------------
// \file tGameSessionSearch_xbox360.cpp - 13 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tGameSessionSearch.hpp"
#include "tGameSession.hpp"
#include "tApplication.hpp"

namespace Sig
{

	//------------------------------------------------------------------------------
	// tSearchData
	//------------------------------------------------------------------------------
	tGameSessionSearch::tSearchData::tSearchData( )
		: mEnumerator( INVALID_HANDLE_VALUE )
		, mResults( NULL )
		, mUserIndex( 0 )
		, mNumUsers( 0 )
	{
		fZeroOut( mOverlapped );
	}

	tGameSessionSearch::tSearchData::~tSearchData( )
	{
		sigcheckfail_xoverlapped_done_else_wait_cancel( &mOverlapped );
	}

	//------------------------------------------------------------------------------
	// tQosLookup
	//------------------------------------------------------------------------------
	tGameSessionSearch::tQosLookup::tQosLookup( ) : mXnQos( NULL )
	{
	}

	//------------------------------------------------------------------------------
	tGameSessionSearch::tQosLookup::~tQosLookup( )
	{
		fReset( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::tQosLookup::fInit( const tGameSessionSearch& sessionSearch )
	{
		fReset( );

		const u32 numResults = sessionSearch.fUnconfirmedResultCount( );

		// Set up our arrays
		mXnAddrs.fResize( numResults );
		mXnKIds.fResize( numResults );
		mXnKeys.fResize( numResults );
		for( u32 i = 0; i < numResults; ++i )
		{
			const tGameSessionSearchResult& curResult = sessionSearch.fUnconfirmedResult( i );
			mXnAddrs[ i ] = &curResult.info.hostAddress;
			mXnKIds[ i ] = &curResult.info.sessionID;
			mXnKeys[ i ] = &curResult.info.keyExchangeKey;
		}

		int result = XNetQosLookup(
			numResults,
			mXnAddrs.fBegin( ),
			mXnKIds.fBegin( ),
			mXnKeys.fBegin( ),
			0,
			NULL,
			NULL,
			0,
			0,
			0,
			NULL,
			&mXnQos );

		if( result != 0 )
		{
			log_warning( "XNetQosLookup failed with result " << result );
			fReset( );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::tQosLookup::fReset( )
	{
		if( mXnQos )
		{
			XNetQosRelease( mXnQos );
			mXnQos = NULL;
		}
		mXnAddrs.fDeleteArray( );
		mXnKIds.fDeleteArray( );
		mXnKeys.fDeleteArray( );
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionSearch::tQosLookup::fValid( ) const
	{
		return mXnQos != NULL;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionSearch::tQosLookup::fPending( ) const
	{
		sigassert( fValid( ) );
		return mXnQos->cxnqosPending > 0;
	}

	//------------------------------------------------------------------------------
	// tGameSessionSearch
	//------------------------------------------------------------------------------
	b32 tGameSessionSearch::fBegin( 
		u32 userIndex,
		u32 gameType,
		u32 gameMode,
		u32 procedure, 
		u32 maxResultCount, 
		u32 numUsers )
	{
		sigassert( mState == cStateNull && "Begin can only be called from null state" );
		sigassert( maxResultCount >= 1 && "Begin called with invalid maxResultCount" );
		sigassert( !mData.mResults && "Sanity: Results should be nulled if state if null" );

		// Contexts
		{
			XUSER_CONTEXT context;

			// GameType
			context.dwContextId = X_CONTEXT_GAME_TYPE;
			context.dwValue = gameType;
			mData.mContexts.fPushBack( context);

			// GameMode
			context.dwContextId = X_CONTEXT_GAME_MODE;
			context.dwValue = gameMode;
			mData.mContexts.fPushBack( context );
		}

		DWORD resultsSize = 0;

		// Initial query for size
		{
			DWORD status = XSessionSearchEx( 
				procedure, 
				userIndex, 
				maxResultCount, 
				numUsers, 
				0, 0, NULL, NULL, // No props or contexts for size call
				&resultsSize,
				NULL, NULL );

			if( status != ERROR_INSUFFICIENT_BUFFER )
			{
				log_warning( "Session search failed because call for result size failed" );
				fDestroy( );
				return false;
			}
		}

		// Allocate results
		mData.mResults = reinterpret_cast<XSESSION_SEARCHRESULT_HEADER*>( NEW BYTE[resultsSize] );
		fMemSet( mData.mResults, 0, resultsSize );

		// Start real query
		DWORD status = XSessionSearchEx( 
			procedure, 
			userIndex, 
			maxResultCount, 
			numUsers, 
			mData.mProperties.fCount( ), 
			mData.mContexts.fCount( ), 
			mData.mProperties.fBegin( ), 
			mData.mContexts.fBegin( ), 
			&resultsSize, 
			mData.mResults, 
			&mData.mOverlapped );

		if( status != ERROR_IO_PENDING && status != ERROR_SUCCESS )
		{
			log_warning( "Session search failed because call to start search failed" );
			fDestroy( );
			return false;
		}

		mState = cStateSearching;
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionSearch::fBegin( u32 userIndex, const tGameSessionInfo & info )
	{
		sigassert( mState == cStateNull && "Begin can only be called from null state" );
		sigassert( !mData.mResults && "Sanity: Results should be nulled if state if null" );

		DWORD resultsSize = 0;

		// Initial query for size
		{
			DWORD status = XSessionSearchByID(
				info.sessionID, 
				userIndex,
				&resultsSize,
				NULL, NULL );

			if( status != ERROR_INSUFFICIENT_BUFFER )
			{
				log_warning( "Session search failed because call for result size failed" );
				fDestroy( );
				return false;
			}
		}

		// Allocate results
		mData.mResults = reinterpret_cast<XSESSION_SEARCHRESULT_HEADER*>( NEW BYTE[resultsSize] );
		fMemSet( mData.mResults, 0, resultsSize );

		// Start real query
		DWORD status = XSessionSearchByID( 
			info.sessionID, 
			userIndex, 
			&resultsSize, 
			mData.mResults, 
			&mData.mOverlapped ); 

		if( status != ERROR_IO_PENDING && status != ERROR_SUCCESS )
		{
			log_warning( "Session search failed because call to start search failed" );
			fDestroy( );
			return false;
		}

		mState = cStateSearching;
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionSearch::fBeginFriendsOnly( 
		u32 userIndex,
		u32 gameType,
		u32 gameMode,
		u32 numUsers )
	{
		sigassert( mState == cStateNull && "Begin can only be called from null state" );
		sigassert( !mData.mResults && "Sanity: Results should be nulled if state if null" );

		// Create an an enumerator to retrieve our friends
		DWORD bufferSize = 0;
		DWORD result = XFriendsCreateEnumerator( 
			userIndex, 0, MAX_FRIENDS, &bufferSize, &mData.mEnumerator );

		// Success?
		if( result != ERROR_SUCCESS )
		{
			log_warning( "Session search failed. Error " << std::hex << result << " creating friend enumerator" );
			return false;
		}

		// Allocate
		if( mData.mFriendBuffer.fCount( ) < bufferSize )
			mData.mFriendBuffer.fSetCount( bufferSize );

		// Enumerate friends
		result = XEnumerate(
			mData.mEnumerator,
			mData.mFriendBuffer.fBegin( ),
			mData.mFriendBuffer.fCount( ),
			NULL,
			&mData.mOverlapped );

		// Success?
		if( result != ERROR_IO_PENDING && result != ERROR_SUCCESS )
		{
			log_warning( "Session search failed. Error " << std::hex << result << " enumerating friends" );
			XCloseHandle( mData.mEnumerator );
			mData.mEnumerator = INVALID_HANDLE_VALUE;
			return false;
		}

		// Setup mData for when we start the actual search
		mData.mUserIndex = userIndex;
		mData.mNumUsers = numUsers;

		mState = cStateReadingFriends;
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionSearch::fBegin(
		u32 userIndex,
		const tGameSessionId* ids,
		u32 numIds,
		u32 numUsers )
	{
		sigassert( mState == cStateNull && "Begin can only be called from null state" );
		sigassert( !mData.mResults && "Sanity: Results should be nulled if state if null" );

		mData.mSessionIds.fSetCount( 0 );
		mData.mSessionIds.fInsert( 0, ids, numIds );

		DWORD resultsSize = 0;

		// Initial query for size
		{
			DWORD status = XSessionSearchByIds(
				mData.mSessionIds.fCount( ),
				mData.mSessionIds.fBegin( ),
				userIndex,
				&resultsSize,
				NULL, NULL );

			if( status != ERROR_INSUFFICIENT_BUFFER )
			{
				log_warning( "Session search failed because call for result size failed" );
				fDestroy( );
				return false;
			}
		}

		// Allocate results
		mData.mResults = reinterpret_cast<XSESSION_SEARCHRESULT_HEADER*>( NEW BYTE[resultsSize] );
		fMemSet( mData.mResults, 0, resultsSize );

		// Start real query
		DWORD status = XSessionSearchByIds(
			mData.mSessionIds.fCount( ),
			mData.mSessionIds.fBegin( ),
			userIndex,
			&resultsSize,
			mData.mResults,
			&mData.mOverlapped );

		if( status != ERROR_IO_PENDING && status != ERROR_SUCCESS )
		{
			log_warning( "Session search failed because call to start search failed" );
			fDestroy( );
			return false;
		}

		mState = cStateSearching;
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fSetContext( u32 id, u32 value )
	{
		if( id == tUser::cUserContextGameMode || id == tUser::cUserContextGameType )
			return;

		XUSER_CONTEXT context;
		context.dwContextId = id;
		context.dwValue = value;
		mData.mContexts.fPushBack( context );
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fSetProperty( u32 id, const tUserData & value )
	{
		XUSER_PROPERTY prop;
		prop.dwPropertyId = id;
		value.fGetPlatformSpecific( &prop.value );

		mData.mProperties.fPushBack( prop );
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fTick( )
	{
		if( mState == cStateTestingResults )
		{
			if( !mQosLookup.fPending( ) )
			{
				const XNQOS* qos = mQosLookup.fXnQos( );
				for( u32 i = 0; i < qos->cxnqos; ++i )
				{
					const tGameSessionSearchResult& searchResult = fUnconfirmedResult( i );

					// If the host was successfully contacted, add the result to the confirmed list
					const XNQOSINFO& qosInfo = qos->axnqosinfo[ i ];
					if( qosInfo.bFlags & XNET_XNQOSINFO_TARGET_CONTACTED &&
						!( qosInfo.bFlags & XNET_XNQOSINFO_TARGET_DISABLED ) )
					{
						tConfirmedResult& confirmedResult = mConfirmedResults.fPushBack( );
						confirmedResult.mResult = &searchResult;
						confirmedResult.mCustomData = qosInfo.pbData;
						confirmedResult.mCustomDataSize = qosInfo.cbData;
					}
				}

				mState = cStateSuccess;
			}

			return;
		}

		if( mState == cStateReadingFriends )
		{
			// If the operation hasn't completed then we must wait longer
			if( !XHasOverlappedIoCompleted( &mData.mOverlapped ) )
				return;

			XCloseHandle( mData.mEnumerator );
			mData.mEnumerator = INVALID_HANDLE_VALUE;

			const DWORD result = XGetOverlappedExtendedError( &mData.mOverlapped );
			if( SUCCEEDED( result ) )
			{
				// Shrink to correct size for friends found
				u32 friendsRead = mData.mOverlapped.InternalHigh;
				mData.mFriendBuffer.fSetCount( friendsRead * sizeof( XONLINE_FRIEND ) );

				// Build the list of session ids to search for
				tGameSessionId invalidSessionId;
				fZeroOut( invalidSessionId );
				XONLINE_FRIEND* friendsList = ( XONLINE_FRIEND* )mData.mFriendBuffer.fBegin( );
				tGrowableArray< tGameSessionId > sessionIds;
				for( u32 friendIdx = 0; friendIdx < friendsRead; ++friendIdx )
				{
					// TODO: Make sure the curFriend.dwTitleID matches our current title id
					XONLINE_FRIEND& curFriend = friendsList[ friendIdx ];
					if( fMemCmp( &curFriend.sessionID, &invalidSessionId, sizeof( tGameSessionId ) ) != 0 )
						sessionIds.fPushBack( curFriend.sessionID );
				}

				// Clean up after cStateReadingFriends
				fZeroOut( mData.mOverlapped );
				mState = cStateNull;

				if( sessionIds.fCount( ) )
				{
					fBegin(
						mData.mUserIndex,
						sessionIds.fBegin( ),
						sessionIds.fCount( ),
						mData.mNumUsers );
				}
				else
				{
					mState = cStateFail;
				}
			}
			else
			{
				log_warning( "Session search failed with error: " << std::hex << result );
				mState = cStateFail;
			}
		}

		// We only need to process while we're searching
		if( mState != cStateSearching )
			return;

		// If the operation hasn't completed then we must wait longer
		if( !XHasOverlappedIoCompleted( &mData.mOverlapped ) )
			return;

		const DWORD result = XGetOverlappedExtendedError( &mData.mOverlapped );
		if( SUCCEEDED( result ) )
		{
			mState = cStateTestingResults;
			mQosLookup.fInit( *this );

			if( !mQosLookup.fValid( ) )
				mState = cStateFail;
		}
		else
		{
			log_warning( "Session search failed with error: " << std::hex << result );
			mState = cStateFail;
		}

		fZeroOut( mData.mOverlapped );
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fCancel( )
	{
		if( mState == cStateSearching )
		{
			const DWORD result = XCancelOverlapped( &mData.mOverlapped );
			sigassert( result == ERROR_SUCCESS && "Attempt to cancel session search failed" );
			mState = cStateFail;
		}
		
		fReset( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fReset( )
	{
		sigassert( mState != cStateSearching && "Reset called during active session search" );

		if( mState == cStateNull )
			return;

		mState = cStateNull;
		fDestroy( );
	}

	//------------------------------------------------------------------------------
	u32 tGameSessionSearch::fUnconfirmedResultCount( ) const
	{
		if( mState != cStateSuccess && mState != cStateTestingResults )
			return 0;

		return mData.mResults->dwSearchResults;
	}
	
	//------------------------------------------------------------------------------
	u32 tGameSessionSearch::fFindOpenResult( u32 slotsNeeded ) const
	{
		if( mState == cStateSuccess )
		{
			for( u32 r = 0; r < mConfirmedResults.fCount( ); ++r )
			{
				if( mConfirmedResults[ r ].mResult->dwOpenPublicSlots >= slotsNeeded )
					return r;
			}
		}

		return ~0;
	}

	//------------------------------------------------------------------------------
	const tGameSessionSearchResult & tGameSessionSearch::fUnconfirmedResult( u32 idx ) const
	{
		sigassert( mState == cStateSuccess || mState == cStateTestingResults && "Can only access session search results on successful searches" );
		sigassert( idx < mData.mResults->dwSearchResults && "Result index out of bounds" );

		return ( const tGameSessionSearchResult & )mData.mResults->pResults[ idx ];
	}

	//------------------------------------------------------------------------------
	u32 tGameSessionSearch::fResultContext( u32 idx, u32 contextId ) const
	{
		const tGameSessionSearchResult & result = fResult( idx );

		for( u32 c = 0; c < result.cContexts; ++c )
		{
			if( result.pContexts[ c ].dwContextId == contextId )
			{
				return result.pContexts[ c ].dwValue;
			}
		}

		return ~0;
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fResultProperty( u32 idx, tUserProperty & prop ) const
	{
		prop.mData.fReset( );

		const tGameSessionSearchResult & result = fResult( idx );
		for( u32 p = 0; p < result.cProperties; ++p )
		{
			if( result.pProperties[ p ].dwPropertyId == prop.mId )
			{
				prop.mData.fSetPlatformSpecific( &result.pProperties[ p ].value );
				return;
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fResultCustomData( u32 idx, const byte*& data, u32& dataSize ) const
	{
		const tConfirmedResult & result = mConfirmedResults[ idx ];
		data = result.mCustomData;
		dataSize = result.mCustomDataSize;
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fDestroy( )
	{
		sigassert( mState == cStateNull && "Sanity: Destroy can only act when the state is null" );

		mQosLookup.fReset( );
		mConfirmedResults.fDeleteArray( );

		mData.mContexts.fDeleteArray( );
		mData.mProperties.fDeleteArray( );
		fZeroOut( &mData.mOverlapped );

		if( mData.mEnumerator != INVALID_HANDLE_VALUE )
		{
			XCloseHandle( mData.mEnumerator );
			mData.mEnumerator = INVALID_HANDLE_VALUE;
		}
		
		if( mData.mResults )
		{
			delete mData.mResults;
			mData.mResults = NULL;
		}

		mData.mFriendBuffer.fDeleteArray( );
		mData.mSessionIds.fDeleteArray( );

		mData.mUserIndex = 0;
		mData.mNumUsers = 0;
	}
}
#endif//#if defined( platform_xbox360 )

