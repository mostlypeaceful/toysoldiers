//------------------------------------------------------------------------------
// \file tLeaderboard.hpp - 29 Nov 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tLeaderboard__
#define __tLeaderboard__
#include "tUser.hpp"
#include "Gui/tText.hpp"

namespace Sig 
{

#ifdef platform_xbox360
	enum { cUserAttributesInStatsSpec = XUSER_STATS_ATTRS_IN_SPEC };
	typedef XUSER_STATS_SPEC tUserStatsSpec;
#else
	enum { cUserAttributesInStatsSpec = 64 };
	struct tUserStatsSpec
	{
		u32 dwViewId;
		const char* mViewName;
		u32 dwNumColumnIds;
		u16 wColumnId[ cUserAttributesInStatsSpec ];
	};

	struct tUserStatsColumn
	{
		u16 wColumnId;
		tUserData Value;
	};

	struct tUserStatsRow
	{
		u32 dwRank;
		tPlatformUserId mUserId;
		tStringPtr szGamertag;
		u32 dwNumColumns;
		tDynamicArray< tUserStatsColumn > pColumns;
	};

	struct tStatsView
	{
		u32 dwViewId;
		u32 dwNumRows;
		u32 dwTotalViewRows;
		tDynamicArray< tUserStatsRow > pRows;
	};
#endif
	enum tLeaderBoardVisiblity
	{
		cLeaderBoardVisible,
		cLeaderBoardNotVisible
	};

	enum tAggregateType
	{
		cAggregateMax,
		cAggregateLast,
		cAggregateSum,
	};

	///
	/// \class tLeaderboardColumnDesc
	/// \brief 
	struct tLeaderboardColumnDesc
	{
#if defined( platform_xbox360 )
		u32 mDisplayNameId;
#else
		const char* mDisplayNameId;
		tAggregateType mAggregateType;
		u32 mPropertyId;
#endif
		u32 mWidth;
		Gui::tText::tAlignment mAlignment;
		tLeaderBoardVisiblity mVisibility;
		u32 mUserData; //user supplied value to help format the data

#if defined( platform_xbox360 )
		tLeaderboardColumnDesc( u32 displayNameID = ~0, u32 width = ~0, Gui::tText::tAlignment alignment = Gui::tText::cAlignRight, tLeaderBoardVisiblity visibil = cLeaderBoardNotVisible, u32 userData = ~0 )
			: mDisplayNameId( displayNameID ), mWidth( width ), mAlignment( alignment ), mVisibility( visibil ), mUserData( userData )
		{ }
#else
		tLeaderboardColumnDesc( const char* displayNameID = NULL
			, tAggregateType aggregateType = cAggregateMax
			, u32 propertyId = 0
			, u32 width = ~0
			, Gui::tText::tAlignment alignment = Gui::tText::cAlignRight
			, tLeaderBoardVisiblity visibil = cLeaderBoardNotVisible
			, u32 userData = ~0 )
			: mDisplayNameId( displayNameID )
			, mAggregateType( aggregateType)
			, mPropertyId( propertyId )
			, mWidth( width )
			, mAlignment( alignment )
			, mVisibility( visibil )
			, mUserData( userData )
		{ }
#endif
	};

	///
	/// \class tLeaderboardDesc
	/// \brief 
	struct tLeaderboardDesc
	{
#if defined( platform_xbox360 )
		u32 mTitleDisplayId;
#else
		const char* mTitleDisplayId;
#endif
		tUserStatsSpec mSpec;
		tLeaderboardColumnDesc mColumnDescs[ cUserAttributesInStatsSpec ];

		u32 fBoardId( ) const;
	};

	///
	/// \class tLeaderboard
	/// \brief 
	class tLeaderboard : public tRefCounter
	{
	public:

		// Memory for the leaderboards must persist until cleard
		static void fSetLeaderboards( u32 count, const tLeaderboardDesc * leaderboards );
		static const tLeaderboardDesc * fFindLeaderboard( u32 boardId );
		static const tLeaderboardDesc * fFindLeaderboardByViewId( u32 viewId );
		static void fResetLeaderBoards( );
#if defined( use_steam )
		static SteamLeaderboard_t fGetHandle( const char* name );
		static void fAddHandle( const char* name, SteamLeaderboard_t handle );
		static b32 fIsReadActive( );
#endif
		static void fExportScriptInterface( tScriptVm& vm );

	public:

		static const u32 cColumnIndexRank = 0;
		static const u32 cColumnIndexGamerName = 1;
		static const u32 cColumnIndexUserData = 2;
		
		static const u32 cColumnIdRank = 0xffffffff;
		static const u32 cColumnIdGamerName = 0xfffffffe;

		static const u32 cMaxBoardsToRead = 10; // From XDK docs
		static const u32 cMaxRowsToRead = 100; // XDK docs

		enum tState
		{
			cStateNull,
#if defined( platform_xbox360 )
			cStateReadingFriends,
#endif
			cStateReading,
			cStateSuccess,
			cStateFail
		};

	public:

		tLeaderboard( );
		~tLeaderboard( );
		tLeaderboard(const tLeaderboard& leaderboard);

		// State
		tState fState( ) const { return mState; }

		// Reading
		void	fAddBoardToRead( u32 boardId );
		void    fRemoveBoardToRead( u32 boardId );
		u32     fBoardToReadCount( ) const { return mBoardsToReadCount; }
		void	fReadByRank( u32 rankStart, u32 toRead );
		void	fReadByRankAround( tUser * user, u32 toRead );
		void	fReadByFriends( tUser * user, u32 friendStart );
		void	fReadByPlatformId( const tPlatformUserId userIds[], u32 userIdCount );
		b32		fAdvanceRead( );
		void	fCancelRead( );
		void	fRetainZeroRanks( b32 retain ) { mRetainZeroRanks = retain; }

		// Selects the board for data access 
		void	fSelectBoard( u32 boardId );

		tLocalizedString	fBoardName( ) const;

		// Columns
		u32					fColumnCount( ) const;
		u32					fColumnWidth( u32 col ) const;
		u32					fColumnAlignment( u32 col ) const;
		tLocalizedString	fColumnName( u32 col ) const;
		u32					fColumnUserData( u32 col ) const;

		// Rows
		u32 fRowsAvailable( ) const;
		u32 fRowsTotal( ) const;
		u32 fRowRank( u32 r ) const;
		tPlatformUserId fRowUserId( u32 r ) const;
		std::string fRowGamerName( u32 r ) const;
		u32 fRowForUserId( tPlatformUserId userId ) const; // return ~0 if not found

		// Data by index
		b32 fDataIsValid( u32 col, u32 row ) const;
		tUserData fData( u32 col, u32 row ) const;

		// Data by id
		b32 fDataIsValidById( u32 columnId, u32 row ) const;
		tUserData fDataById( u32 columnId, u32 row ) const;

		// Results - returns gamer name as binary data
		b32 fFindResult( u32 col, u32 row, tUserData & data ) const;
		b32 fFindResultById( u32 columnId, u32 row, tUserData & data ) const;

#if defined( use_steam )
		u32 fPropertyIdToColumnId( u32 propertyId );
		u32 fIndexOfColumnId( u32 columnId );
#endif

		template< class t >
		t fTypedData( u32 col, u32 row, const t & defaultValue ) const
		{
			tUserData data;
			if( !fFindResult( col, row, data ) )
				return defaultValue;

			if( data.mType == tUserData::cTypeNull )
				return defaultValue;

			std::stringstream ss;
			ss << data.fDataToString( );
			t o; ss >> o;
			return o;
		}

		template< class t >
		t fTypedDataById( u32 columnId, u32 row, const t & defaultValue ) const
		{
			tUserData data;
			if( !fFindResultById( columnId, row, data ) )
				return defaultValue;

			if( data.mType == tUserData::cTypeNull )
				return defaultValue;

			std::stringstream ss;
			ss << data.fDataToString( );
			t o; ss >> o;
			return o;
		}

	private:

		// platform specific shiz, some may be private but omitted for clarity
#ifdef platform_xbox360
		struct tReadData
		{
			tReadData( );

			void fCloseEnumerator( );
			void fResetOverlapped( );
			b32 fStatsOverlapComplete( b32 & success );
			b32 fFriendsOverlapComplete( b32 & success );

			tPlatformUserId mRequestingUser;
			u32 mFriendReadStart;
			tGrowableBuffer mFriendBuffer;

			HANDLE mEnumerator;
			tFixedArray<tUserStatsSpec, cMaxBoardsToRead> mSpecData;
			tDynamicBuffer mBuffer;
			DWORD mLastError;
			XOVERLAPPED mOverlapped;
		};

		typedef XUSER_STATS_VIEW tStatsView;

		void fOnFriendsRead( );

#elif defined( use_steam )
		static tHashTable< const char*, SteamLeaderboard_t > sHandleCache;

		class tReadData
		{
		public:
			tReadData( );
			virtual ~tReadData();

			void fClear( );
			void fCancel( );

			void fReadByRank( u32 rankStart, u32 toRead );
			void fReadByRankAround( tUser * user, u32 toRead );
			void fReadByFriends( tUser * user, u32 friendStart );
			void fReadByPlatformId( const tPlatformUserId userIds[], u32 userIdCount );

			b32 fOverlapComplete( b32 & success );

			tPlatformUserId mRequestingUser;
			u32 mFriendReadStart;
			tGrowableBuffer mFriendBuffer;

			tFixedArray< const tLeaderboardDesc*, cMaxBoardsToRead > mBoards;
			u32 mBoardsCount;
			u32 mCurrentBoard;
			tFixedArray< tStatsView, cMaxBoardsToRead > mViews;
			u32 mViewsCount;

			// Parameters for reading boards
			u32 mRankStart;
			u32 mToRead;
			tUser* mUser;
			tDynamicArray< tPlatformUserId > mUserIds;
			u32 mUserIdCount;

		private:
			enum tState
			{
				cStateNone,
				cStateWaiting,
				cStateFinding,
				cStateReading,
				cStateDone,
				cStateError,
			};

			enum tRequestType
			{
				cRequestTypeByRank,
				cRequestTypeByRankAround,
				cRequestTypeByFriends,
				cRequestTypeByPlatformId,
			};

			void fAddEntriesToView( LeaderboardScoresDownloaded_t *params );

			tState mState;
			tRequestType mRequestType;

			// Called when SteamUserStats()->FindLeaderboard() returns asynchronously
			void fOnFindLeaderboardByRank( LeaderboardFindResult_t *pResult, bool bIOFailure );
			void fOnFindLeaderboardByRankAround( LeaderboardFindResult_t *pResult, bool bIOFailure );
			void fOnFindLeaderboardByFriends( LeaderboardFindResult_t *pResult, bool bIOFailure );
			void fOnFindLeaderboardByPlatformId( LeaderboardFindResult_t *pResult, bool bIOFailure );
			CCallResult< tReadData, LeaderboardFindResult_t > mCallbackFindLeaderboard;

			// Called when SteamUserStats()->DownloadLeaderboardEntries () returns asynchronously
			void fOnGetLeaderboardEntriesByRank( LeaderboardScoresDownloaded_t *pResult, bool bIOFailure );
			void fOnGetLeaderboardEntriesByRankAround( LeaderboardScoresDownloaded_t *pResult, bool bIOFailure );
			void fOnGetLeaderboardEntriesByFriends( LeaderboardScoresDownloaded_t *pResult, bool bIOFailure );
			void fOnGetLeaderboardEntriesByPlatformId( LeaderboardScoresDownloaded_t *pResult, bool bIOFailure );
			CCallResult< tReadData, LeaderboardScoresDownloaded_t > mCallbackGetLeaderboardEntries;
		};

		static u32 mNumInstances;

		static void fProcessCurrent( );
#else
		struct tReadData { };

		struct tStatsView { };
#endif

	private:
		//this does not own the memory in any way, points to static memory
		static tArraySleeve<const tLeaderboardDesc> mLeaderboards;

		tScript64 fRowUserIdFromScript( u32 r ) const;
		u32 fRowForUserIdFromScript( tScript64 r ) const;
		b32 fAquireSelectedView( ) const;

	private:
		tState mState;
		b32 mRetainZeroRanks;

		struct 
		{
			const tLeaderboardDesc * mBoard;
			mutable const tStatsView * mView; 
		} mSelected;

		u32 mBoardsToReadCount;
		tFixedArray<const tLeaderboardDesc *, cMaxBoardsToRead> mBoardsToRead; //10 is from XDK docs

		tReadData mReadData;
	};

	typedef tRefCounterPtr<tLeaderboard> tLeaderboardPtr;
}

#endif//__tLeaderboard__
