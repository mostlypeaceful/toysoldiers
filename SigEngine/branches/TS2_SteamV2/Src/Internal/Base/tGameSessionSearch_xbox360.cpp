//------------------------------------------------------------------------------
// \file tGameSessionSearch_xbox360.cpp - 13 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tGameSessionSearch.hpp"
#include "tApplication.hpp"

namespace Sig
{

	//------------------------------------------------------------------------------
	// tSearchData
	//------------------------------------------------------------------------------
	tGameSessionSearch::tSearchData::tSearchData( )
		: mResults( NULL )
	{
		fZeroOut( mOverlapped );
	}

	//------------------------------------------------------------------------------
	// tGameSessionSearch
	//------------------------------------------------------------------------------
	tGameSessionSearch::tGameSessionSearch( )
		: mState( cStateNull )
	{
	}

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
				log_warning( 0, "Session search failed because call for result size failed" );
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
			log_warning( 0, "Session search failed because call to start search failed" );
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
				log_warning( 0, "Session search failed because call for result size failed" );
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
			log_warning( 0, "Session search failed because call to start search failed" );
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
		// We only need to process while we're searching
		if( mState != cStateSearching )
			return;

		// If the operation hasn't completed then we must wait longer
		if( !XHasOverlappedIoCompleted( &mData.mOverlapped ) )
			return;

		const DWORD result = XGetOverlappedExtendedError( &mData.mOverlapped );
		if( SUCCEEDED( result ) )
			mState = cStateSuccess;
		else
		{
			log_warning( 0, "Session search failed with error: " << std::hex << result );
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
	u32 tGameSessionSearch::fResultCount( ) const
	{
		if( mState != cStateSuccess )
			return 0;

		return mData.mResults->dwSearchResults;
	}
	
	//------------------------------------------------------------------------------
	u32 tGameSessionSearch::fFindOpenResult( u32 slotsNeeded ) const
	{
		if( mState == cStateSuccess )
		{
			for( u32 r = 0; r < mData.mResults->dwSearchResults; ++r )
			{
				if( mData.mResults->pResults[ r ].dwOpenPublicSlots >= slotsNeeded )
					return r;
			}
		}

		return ~0;
	}

	//------------------------------------------------------------------------------
	const tGameSessionSearchResult & tGameSessionSearch::fResult( u32 idx ) const
	{
		sigassert( mState == cStateSuccess && "Can only access session search results on successful searches" );
		sigassert( idx < mData.mResults->dwSearchResults && "Result index out of bounds" );

		return ( const tGameSessionSearchResult & )mData.mResults->pResults[ idx ];
	}

	//------------------------------------------------------------------------------
	u32 tGameSessionSearch::fResultContext( u32 idx, u32 contextId ) const
	{
		sigassert( mState == cStateSuccess && "Can only access session search results on successful searches" );
		sigassert( idx < mData.mResults->dwSearchResults && "Result index out of bounds" );

		const tGameSessionSearchResult & result = ( const tGameSessionSearchResult & )mData.mResults->pResults[ idx ];

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
		sigassert( mState == cStateSuccess && "Can only access session search results on successful searches" );
		sigassert( idx < mData.mResults->dwSearchResults && "Result index out of bounds" );

		prop.mData.fReset( );

		const tGameSessionSearchResult & result = ( const tGameSessionSearchResult & )mData.mResults->pResults[ idx ];
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
	void tGameSessionSearch::fDestroy( )
	{
		sigassert( mState == cStateNull && "Sanity: Destroy can only act when the state is null" );

		mData.mContexts.fDeleteArray( );
		mData.mProperties.fDeleteArray( );
		fZeroOut( &mData.mOverlapped );
		
		if( mData.mResults )
		{
			delete mData.mResults;
			mData.mResults = NULL;
		}
	}
}
#endif//#if defined( platform_xbox360 )

