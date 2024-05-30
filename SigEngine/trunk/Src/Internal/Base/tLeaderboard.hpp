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
	enum { cUserAttributesInStatsSpec = 1 };
	struct tUserStatsSpec { };
#endif
	enum tLeaderBoardVisiblity
	{
		cLeaderBoardVisible,
		cLeaderBoardNotVisible
	};

	///
	/// \class tLeaderboardColumnDesc
	/// \brief 
	struct tLeaderboardColumnDesc
	{
		u32 mDisplayNameId;
		u32 mWidth;
		Gui::tText::tAlignment mAlignment;
		tLeaderBoardVisiblity mVisibility;
		u32 mUserData; //user supplied value to help format the data

		tLeaderboardColumnDesc( u32 displayNameID = ~0, u32 width = ~0, Gui::tText::tAlignment alignment = Gui::tText::cAlignRight, tLeaderBoardVisiblity visibil = cLeaderBoardNotVisible, u32 userData = ~0 )
			: mDisplayNameId( displayNameID ), mWidth( width ), mAlignment( alignment ), mVisibility( visibil ), mUserData( userData )
		{ }
	};

	///
	/// \class tLeaderboardDesc
	/// \brief 
	struct tLeaderboardDesc
	{
		u32 mTitleDisplayId;
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
		static void fResetLeaderBoards( );

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
			cStateReadingFriends,
			cStateReading,
			cStateSuccess,
			cStateFail
		};

	public:

		tLeaderboard( );
		~tLeaderboard( );

		// State
		tState fState( ) const { return mState; }
		b32 fReading( ) const;
		b32 fDataReady( ) const { return mState == cStateSuccess; }

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
			~tReadData( );

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

#else
		struct tReadData{ };
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
