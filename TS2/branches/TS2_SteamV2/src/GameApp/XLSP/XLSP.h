#ifndef _tXLSP_
#define _tXLSP_

#include "tUser.hpp"
#include "XlspManager.h"

namespace Sig
{
	
	class tXLSP
	{
		declare_singleton( tXLSP );
	public:

		// Minigame leaderboards
		static const u32 cMachineID_Fly = -189968617;
		static const u32 cMachineID_Hallway = 132471559;
		static const u32 cMachineID_TrialMG2 = -695010488;
		static const u32 cMachineID_Totals = -342538395;

		// check ownership record to unlock special feature
		static const u32 cTitleID_MobileOwnership = 0x4D5313B0;
		static const u32 cContentID_MobileOwnership = -342538395;


		void fInitialize( );
		void fSetFrontEndPlayer( const tUserPtr& user );
		void fUpdate( f32 dt );

		void fWriteTestValues( tUser* user );
		void fWriteTestOwnership( tUser* user );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};


	class tXLSPLeaderboard
	{
	public:
		tXLSPLeaderboard( )
			: mMode( cModeIdle )
			, mRequestedMode( cModeIdle )
			, mBoardID( ~0 )
			, mReadSize( 1 )

#ifdef platform_xbox360
			, mFriendReadStart( 0 )
			, mRequestingUser( ~0 )
			, mFriendRequestSuccess( false )
#endif
		{ }

		b32 fIdle( ) const { return mMode == cModeIdle; }

		// base leaderboard boilerplate
		void fAddBoardToRead( u32 board );
		void fReadByFriends( tUser* user, u32 start );
		void fReadByRank( u32 start, u32 amount );
		void fReadByRankAround( tUser * user, u32 toRead );
		b32  fAdvanceRead( );
		void fCancelRead( );
		void fSelectBoard( u32 board );

		tLocalizedString fBoardName( );

		u32  fColumnWidth( u32 col );
		u32  fColumnAlignment( u32 col );
		tLocalizedString fColumnName( u32 col );
		u32 fColumnUserData( u32 col );
		u32 fRowRank( u32 row );
		u32 fRowUserIdFromScript( u32 row );
		std::string fRowGamerName( u32 row );
		u32 fColumnCount( );
		u32 fRowsAvailable( );
		u32 fRowsTotal( );

		tUserData fData( u32 col, u32 row );

		// ownership
		void fRequestMobileOwnership( tUser& user );
		b32	 fHasOwnerShip( ) const { return mHasOwnerShip; }

#ifdef platform_xbox360
		tGrowableArray<XLSP::XlspManager::Score> mScores;

		//getting friends
		tGrowableBuffer mFriendBuffer;
		HANDLE mEnumerator;
		XOVERLAPPED mOverlapped;
		tPlatformUserId mRequestingUser;
		u32 mFriendReadStart;
		b32 mFriendRequestSuccess;
#endif

	private: 
		enum tMode
		{
			cModeRequestingFriends,
			cModeRequestingRank,
			cModeRequestingRankAround,
			cModeRequestingOwnership,
			cModeWaitingResult,
			cModeIdle
		};

		u32 mMode;
		u32 mRequestedMode;
		u32 mBoardID;
		u32 mReadSize;
		tUser* mRequestingUserPtr;
		b32 mHasOwnerShip;

		b32  fCheckReceivedOwnership( );

		// Score stuff
		void fRequestFriendHighScores( u32 leaderboardMachine, u32 numResults );
		b32 fCheckReceivedFriendHighScores( );

		void fRequestMobileHighScores( u32 leaderboardMachine, u32 numResults );
		b32 fCheckReceivedHighScores( );

		void fRequestMobileHighScoresAround( u32 leaderboardMachine, tUser* user, u32 numResults );
		b32 fCheckReceivedHighScoresAround( );


		b32  fFriendsOverlapComplete( b32 & success );
		void fCloseEnumerator( );
		void fResetOverlapped( );
	};


		//void fWriteScore( u32 boardMachineID, u32 value, tUser& user )
		//{
		//	g_XlspManager.RequestPostScore( user.fPlatformId( ), boardMachineID, value, XlspManager::PlatformXbox);
		//}

	//		const int fakeMachineId = 13;
	//		const int otherMachineId = 14;
	//		const int yetAnotherMachineId = 10;
	//		const int fakeTitleId = 12345678;
	//		const int fakeContentId = 0;
	//		const int anotherFakeContentId = 1;
	//		const int pageSize = 20;

	//		//#define POPULATE_SCORES
	//		//#define POPULATE_OWNERSHIP_RECORDS

	//		//test the xlsp code a few seconds after init to give time for everything to settle in with player profile
	//		//stuff a bunch of request in the queue at once
	//		static bool tested = false;
	//		if (!tested && g_Time.fAppTime > 5.0f)
	//		{
	//#ifdef POPULATE_OWNERSHIP_RECORDS
	//			//populate mobile ownership record
	//			g_XlspManager.RequestSetOwnershipRecord(0xfeeb1e1, fakeTitleId, fakeContentId, XlspManager::PlatformMobile);

	//			//populate xbox ownership records
	//			g_XlspManager.RequestSetOwnershipRecord(0xfeeb1e1, fakeTitleId, fakeContentId, XlspManager::PlatformXbox);
	//			g_XlspManager.RequestSetOwnershipRecord(0xfeeb1e1, fakeTitleId, anotherFakeContentId, XlspManager::PlatformXbox);
	//#endif

	//#ifdef POPULATE_SCORES
	//			//populate fake xbox scores
	//			g_XlspManager.RequestPostScore(0xfeeb1e0, fakeMachineId, 31500, XlspManager::PlatformXbox);
	//			g_XlspManager.RequestPostScore(0xfeeb1e1, fakeMachineId, 31500, XlspManager::PlatformXbox);
	//			g_XlspManager.RequestPostScore(0xfeeb1e2, fakeMachineId, 3600, XlspManager::PlatformXbox);
	//			g_XlspManager.RequestPostScore(0xfeeb1e3, fakeMachineId, 3500, XlspManager::PlatformXbox);
	//			g_XlspManager.RequestPostScore(0xfeeb1e4, fakeMachineId, 3500, XlspManager::PlatformXbox);
	//			g_XlspManager.RequestPostScore(0xfeeb1e5, fakeMachineId, 33750, XlspManager::PlatformXbox);
	//			g_XlspManager.RequestPostScore(0xfeeb1e6, fakeMachineId, 33750, XlspManager::PlatformXbox);
	//			g_XlspManager.RequestPostScore(0xfeeb1e7, fakeMachineId, 31505, XlspManager::PlatformXbox);
	//			g_XlspManager.RequestPostScore(0xfeeb1e7, otherMachineId, 32200, XlspManager::PlatformXbox);
	//			g_XlspManager.RequestPostScore(0xfeeb1e7, yetAnotherMachineId, 3150, XlspManager::PlatformXbox);

	//			//populate fake mobile scores
	//			g_XlspManager.RequestPostScore(0xfeeb1e1, fakeMachineId, 41030, XlspManager::PlatformMobile);
	//			g_XlspManager.RequestPostScore(0xfeeb1e3, fakeMachineId, 41730, XlspManager::PlatformMobile);
	//			g_XlspManager.RequestPostScore(0xfeeb1e8, fakeMachineId, 32000, XlspManager::PlatformMobile);
	//#endif

	//			//create list of fake friend xuids 
	//			std::list<XUID> xuids;
	//			xuids.push_back(0xfeeb1e1);
	//			xuids.push_back(0xfeeb1e3);
	//			xuids.push_back(0xfeeb1e4);
	//			xuids.push_back(0xfeeb1e2);
	//			xuids.push_back(0xfeeb1e8);
	//			xuids.push_back(0xfeeb1e5);
	//			//add myself too
	//			xuids.push_back(0xfeeb1e7);

	//			//getscores for friend xuids 
	//			g_XlspManager.RequestGetScoresForXuids(0xfeeb1e7, fakeMachineId, pageSize, xuids);

	//			//get top scores
	//			g_XlspManager.RequestTopScores(fakeMachineId, pageSize);

	//			//get top scores on mobile 
	//			g_XlspManager.RequestTopScoresPlatform(fakeMachineId, XlspManager::PlatformMobile, pageSize);

	//			//get scores around a score
	//			g_XlspManager.RequestGetScoresAroundScore(0xfeeb1e0, fakeMachineId, pageSize, pageSize);

	//			//get rank of a new score (does not post the score)
	//			g_XlspManager.RequestGetRankForScore(fakeMachineId, 50000, XlspManager::PlatformXbox, 0xfeeb1e1);

	//			//get mobile ownership records
	//			g_XlspManager.RequestGetOwnershipRecords(0xfeeb1e1, fakeTitleId, XlspManager::PlatformMobile);

	//			tested = true;
	//		}

	//		//note: results will not contain valid gamertags due to the way the test cases above use fake xuids

	//		char buffer[1000];
	//		XlspManager::LspStatus status;

	//		std::list<XlspManager::OwnershipRecord> records;
	//		static bool tested0 = false;
	//		if (!tested0)
	//		{
	//			status = g_XlspManager.GetOwnershipRecords(records);
	//			if (status == XlspManager::LspStatusUpdated)
	//			{
	//				OutputDebugString("===== ownershipRecords Mobile =====\n");
	//				for (std::list<XlspManager::OwnershipRecord>::const_iterator iter = records.begin(); iter != records.end(); ++iter)
	//				{
	//					XlspManager::OwnershipRecord record = XlspManager::OwnershipRecord(*iter);
	//					sprintf(buffer, "contentId: %d contentType: %d platform: %d purchaseType: %c\n", record.contentId, record.contentType, record.platform, record.purchaseType);
	//					OutputDebugString(buffer);
	//				}
	//				OutputDebugString("=============================\n");
	//				tested0 = true;
	//			}
	//			else if (status == XlspManager::LspStatusFailure)
	//			{
	//				OutputDebugString("===== GetOwnershipRecords Mobile Failed =====\n");
	//				tested0 = true;
	//			}

	//			if (tested0)
	//			{
	//				//add in next ownership request after this one has completed
	//				g_XlspManager.RequestGetOwnershipRecords(0xfeeb1e1, fakeTitleId, XlspManager::PlatformXbox);
	//			}
	//		}

	//		static bool tested00 = false;
	//		if (!tested00)
	//		{
	//			status = g_XlspManager.GetOwnershipRecords(records);
	//			if (status == XlspManager::LspStatusUpdated)
	//			{
	//				OutputDebugString("===== ownershipRecords =====\n");
	//				for (std::list<XlspManager::OwnershipRecord>::const_iterator iter = records.begin(); iter != records.end(); ++iter)
	//				{
	//					XlspManager::OwnershipRecord record = XlspManager::OwnershipRecord(*iter);
	//					sprintf(buffer, "contentId: %d contentType: %d platform: %d purchaseType: %c\n", record.contentId, record.contentType, record.platform, record.purchaseType);
	//					OutputDebugString(buffer);
	//				}
	//				OutputDebugString("=============================\n");
	//				tested00 = true;
	//			}
	//			else if (status == XlspManager::LspStatusFailure)
	//			{
	//				OutputDebugString("===== GetOwnershipRecords Failed =====\n");
	//				tested00 = true;
	//			}
	//		}

	//		std::list<XlspManager::Score> scores;

	//		static bool tested1 = false;
	//		if (!tested1)
	//		{
	//			status = g_XlspManager.GetScoresAroundScore(scores);
	//			if (status == XlspManager::LspStatusUpdated)
	//			{
	//				OutputDebugString("===== scoresAroundScore =====\n");
	//				for (std::list<XlspManager::Score>::const_iterator iter = scores.begin(); iter != scores.end(); ++iter)
	//				{
	//					XlspManager::Score score = XlspManager::Score(*iter);
	//					sprintf(buffer, "xuid: %I64u gamertag: '%s' score: %d rank: %d\n", score.xuid, score.gamertag, score.score, score.rank);
	//					OutputDebugString(buffer);
	//				}
	//				OutputDebugString("=============================\n");
	//				tested1 = true;
	//			}
	//			else if (status == XlspManager::LspStatusFailure)
	//			{
	//				OutputDebugString("===== GetScoresAroundScore Failed =====\n");
	//				tested1 = true;
	//			}
	//		}

	//		static bool tested2 = false;
	//		if (!tested2)
	//		{
	//			status = g_XlspManager.GetScoresForXuids(scores);
	//			if (status == XlspManager::LspStatusUpdated)
	//			{
	//				OutputDebugString("===== scoresForXuids =====\n");
	//				for (std::list<XlspManager::Score>::const_iterator iter = scores.begin(); iter != scores.end(); ++iter)
	//				{
	//					XlspManager::Score score = XlspManager::Score(*iter);
	//					sprintf(buffer, "xuid: %I64u gamertag: '%s' score: %d rank: %d platform:%d\n", score.xuid, score.gamertag, score.score, score.rank, score.platform);
	//					OutputDebugString(buffer);
	//				}
	//				OutputDebugString("=============================\n");
	//				tested2 = true;
	//			}
	//			else if (status == XlspManager::LspStatusFailure)
	//			{
	//				OutputDebugString("===== GetScoresForXuids Failed =====\n");
	//				tested2 = true;
	//			}
	//		}

	//		static bool tested3 = false;
	//		if (!tested3)
	//		{
	//			status = g_XlspManager.GetTopScores(scores);
	//			if (status == XlspManager::LspStatusUpdated)
	//			{
	//				OutputDebugString("===== topScores =====\n");
	//				for (std::list<XlspManager::Score>::const_iterator iter = scores.begin(); iter != scores.end(); ++iter)
	//				{
	//					XlspManager::Score score = XlspManager::Score(*iter);
	//					sprintf(buffer, "xuid: %I64u gamertag: '%s' score: %d rank: %d platform: %d\n", score.xuid, score.gamertag, score.score, score.rank, score.platform);
	//					OutputDebugString(buffer);
	//				}
	//				OutputDebugString("=============================\n");
	//				tested3 = true;
	//			}
	//			else if (status == XlspManager::LspStatusFailure)
	//			{
	//				OutputDebugString("===== GetTopScores Failed =====\n");
	//				tested3 = true;
	//			}
	//		}

	//		static bool tested4 = false;
	//		if (!tested4)
	//		{
	//			status = g_XlspManager.GetTopScoresPlatform(scores);
	//			if (status == XlspManager::LspStatusUpdated)
	//			{
	//				OutputDebugString("===== topScoresPlatform =====\n");
	//				for (std::list<XlspManager::Score>::const_iterator iter = scores.begin(); iter != scores.end(); ++iter)
	//				{
	//					XlspManager::Score score = XlspManager::Score(*iter);
	//					sprintf(buffer, "xuid: %I64u gamertag: '%s' score: %d rank: %d platform: %d\n", score.xuid, score.gamertag, score.score, score.rank, score.platform);
	//					OutputDebugString(buffer);
	//				}
	//				OutputDebugString("=============================\n");
	//				tested4 = true;
	//			}
	//			else if (status == XlspManager::LspStatusFailure)
	//			{
	//				OutputDebugString("===== GetTopScoresPlatform Failed =====\n");
	//				tested4 = true;
	//			}
	//		}

	//		static bool tested5 = false;
	//		bool isNewHighScore;
	//		XlspManager::Score score;
	//		if (!tested5)
	//		{
	//			status = g_XlspManager.GetRankForScore(score, isNewHighScore);
	//			if (status == XlspManager::LspStatusUpdated)
	//			{
	//				OutputDebugString("===== rankForScore =====\n");
	//				sprintf(buffer, "xuid: %I64u score: %d rank: %d platform: %d isNewHighScore: %s\n", score.xuid, score.score, score.rank, score.platform, isNewHighScore?"true":"false");
	//				OutputDebugString(buffer);
	//				OutputDebugString("=============================\n");
	//				tested5 = true;
	//			}
	//			else if (status == XlspManager::LspStatusFailure)
	//			{
	//				OutputDebugString("===== GetRankForScore Failed =====\n");
	//				tested5 = true;
	//			}
	//		}
	//	}

}

#endif _tXLSP_