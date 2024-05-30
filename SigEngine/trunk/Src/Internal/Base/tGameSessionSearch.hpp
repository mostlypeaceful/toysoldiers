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
	class base_export tGameSessionSearch : public tRefCounter
	{
		debug_watch( tGameSessionSearch );
		declare_uncopyable( tGameSessionSearch );
	public:

		enum tState
		{
			cStateNull = 0,
			cStateReadingFriends,
			cStateSearching,
			cStateTestingResults,
			cStateSuccess,
			cStateFail
		};

	public:

		tGameSessionSearch( );
		~tGameSessionSearch( );

		inline tState fState( ) const { return mState; }
		inline b32 fPreparingSearch( ) const { return mState == cStateReadingFriends; }
		inline b32 fSearching( ) const { return mState == cStateSearching; }
		inline b32 fTestingResults( ) const { return mState == cStateTestingResults; }
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
		/// \brief Start a search for games that our friends our playing
		b32 fBeginFriendsOnly(
			u32 userIndex,				// The user used for matchmaking
			u32 gameType,				// The desired game type
			u32 gameMode,				// The desired game mode
			u32 numUsers = 1			// The number of users participating
		);

		///
		/// \brief Start a search from a list of session ids, returns whether the search has begun or not
		b32 fBegin(
			u32 userIndex,				// The user used for matchmaking
			const tGameSessionId* ids,	// The list of session ids to search in
			u32 numIds,					// The number of entries in the list
			u32 numUsers = 1			// The number of users participating
		);

		///
		/// \brief Sets an extended context, contextIds equalling cUserContextGameMode or cUserContextGameType
		/// are ignored. 
		/// \note This can only be called before the call to fBegin
		void fSetContext( u32 contextId, u32 contextValue );

		///
		/// \brief Sets an extended property.
		/// \note This can only be caled before the call to fBegin
		void fSetProperty( u32 propertyId, const tUserData & data );
		
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
		/// \brief Returns the number of sessions found *before* testing for connectivity
		u32 fUnconfirmedResultCount( ) const;

		///
		/// \brief Returns the number of sessions found with confirmed connectivity
		u32 fResultCount( ) const;

		///
		/// \brief Returns the index of the first session with at least 'slotsNeeded'
		/// public slots available or ~0 if none are available
		u32 fFindOpenResult( u32 slotsNeeded = 1 ) const;

		///
		/// \brief Returns the search result at the specified index *before* testing for connectivity
		const tGameSessionSearchResult & fUnconfirmedResult( u32 idx ) const;

		///
		/// \brief Returns the search result with confirmed connectivity at the specified index
		const tGameSessionSearchResult & fResult( u32 idx ) const;

		void fEraseResult( u32 idx );

		u32 fResultContext( u32 idx, u32 contextId ) const;
		void fResultProperty( u32 idx, tUserProperty & prop ) const;
		void fResultCustomData( u32 idx, const byte*& data, u32& dataSize ) const;

	private:

		void fDestroy( );

	private:

#ifdef platform_xbox360
		struct tSearchData
		{
			tSearchData( );
			~tSearchData( );

			XOVERLAPPED mOverlapped;
			HANDLE mEnumerator;
			XSESSION_SEARCHRESULT_HEADER * mResults;

			tGrowableArray< XUSER_CONTEXT>		mContexts;
			tGrowableArray< XUSER_PROPERTY >	mProperties;

			tGrowableBuffer mFriendBuffer;

			tGrowableArray< tGameSessionId > mSessionIds;

			u32 mUserIndex;
			u32 mNumUsers;
		};

		class tQosLookup
		{
		public:
			tQosLookup( );
			~tQosLookup( );

			void fInit( const tGameSessionSearch& sessionSearch );
			void fReset( );

			b32 fValid( ) const;
			b32 fPending( ) const;

			const XNQOS* fXnQos( ) const { return mXnQos; }

		private:
			tDynamicArray< const XNADDR* > mXnAddrs;
			tDynamicArray< const XNKID* > mXnKIds;
			tDynamicArray< const XNKEY* > mXnKeys;
			XNQOS* mXnQos;
		};
#else
		struct tSearchData{ };
		class tQosLookup{ };
#endif

		struct tConfirmedResult
		{
			tConfirmedResult( );

			const tGameSessionSearchResult* mResult;
			const byte* mCustomData;
			u32 mCustomDataSize;
		};

	private:

		tState mState;
		tSearchData mData;

		tQosLookup mQosLookup;
		tGrowableArray< tConfirmedResult > mConfirmedResults;
		
	};

	typedef tRefCounterPtr<tGameSessionSearch> tGameSessionSearchPtr;
}

#endif//__tGameSessionSearch__
