//------------------------------------------------------------------------------
// \file tGameSessionSearch.hpp - 13 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tGameSessionSearch__
#define __tGameSessionSearch__
#include "tGameSessionSearchResult.hpp"
#include "tUser.hpp"

namespace Sig
{
	class tGameInvite;

	///
	/// \class tGameSessionSearch
	/// \brief 
	class tGameSessionSearch : public tRefCounter
	{
	public:

		enum tState
		{
			cStateNull = 0,
			cStateSearching,
			cStateSuccess,
			cStateFail
		};

	public:

		tGameSessionSearch( );
		~tGameSessionSearch( );

		inline tState fState( ) const { return mState; }
		inline b32 fSearching( ) const { return mState == cStateSearching; }
		inline b32 fFinished( ) const { return mState == cStateSuccess || mState == cStateFail; }
		inline b32 fSucceeded( ) const { return mState == cStateSuccess; }
		inline b32 fFailed( ) const { return mState == cStateFail; }

		///
		/// \brief Start a matchmaking search, returns whether the search has begun or not
		b32 fBegin( 
			u32 userIndex,				// The user used for matchmaking
			u32 gameType,				// The desired game type
			u32 gameMode,				// The desired game mode
			u32 procedure = 0,			// The search procedure used
			u32 maxResultCount = 10,	// The maximum number of returned results
			u32 numUsers = 1			// The number of users participating
		);

		///
		/// \brief Start a search for a game that we were invited to
		b32 fBegin( u32 userIndex, const tGameSessionInfo & info );

		///
		/// \brief Sets an extended context, contextIds equalling cUserContextGameMode or cUserContextGameType
		/// are ignored. 
		/// \note This can only be called before the call to fBegin
		void fSetContext( u32 contextId, u32 contextValue );

#if defined( use_steam )
		///
		/// \brief Sets an extended property.
		/// \note This can only be caled before the call to fBegin
		void fSetProperty( u32 propertyId, const tUserData & data, tSearchFilter filter = cSearchFilterExact );
#else
		///
		/// \brief Sets an extended property.
		/// \note This can only be caled before the call to fBegin
		void fSetProperty( u32 propertyId, const tUserData & data );
#endif

		///
		/// \brief Handle search progress
		void fTick( );

		///
		/// \brief Cancels the active search if there is one and resets
		void fCancel( );

		///
		/// \brief Reset to ready for a new search
		void fReset( );

		///
		/// \brief Returns the number of sessions found
		u32 fResultCount( ) const;

		///
		/// \brief Returns the index of the first session with at least 'slotsNeeded'
		/// public slots available or ~0 if none are available
		u32 fFindOpenResult( u32 slotsNeeded = 1 ) const;

		///
		/// \brief Returns the search result at the specified index
		const tGameSessionSearchResult & fResult( u32 idx ) const;

		u32 fResultContext( u32 idx, u32 contextId ) const;
		void fResultProperty( u32 idx, tUserProperty & prop ) const;

	private:

		void fDestroy( );

	private:

#ifdef platform_xbox360
		struct tSearchData
		{
			tSearchData( );

			XOVERLAPPED mOverlapped;
			XSESSION_SEARCHRESULT_HEADER * mResults;

			tGrowableArray< XUSER_CONTEXT>		mContexts;
			tGrowableArray< XUSER_PROPERTY >	mProperties;
		};
#elif defined( use_steam )
		struct tSearchData
		{
			tSearchData( );

			b32 fOverlapComplete( b32 & success, b32 wait );

			SteamAPICall_t mApiCall;
			u32 mResultsCount;
			tGrowableArray< tGameSessionSearchResult > mResults;
			ESteamAPICallFailure mLastError;
			tGrowableArray< tUserProperty >	mContexts;
			tGrowableArray< tUserProperty >	mProperties;
		};

		STEAM_CALLBACK( tGameSessionSearch, fOnRequestLobbyList, LobbyMatchList_t, mCallbackRequestLobbyList );
#else
		struct tSearchData{ };
#endif

	private:

		tState mState;
		tSearchData mData;
		
	};

	typedef tRefCounterPtr<tGameSessionSearch> tGameSessionSearchPtr;
}

#endif//__tGameSessionSearch__
