#ifndef __tSteamServices__
#define __tSteamServices__

#if false //defined( platform_pcdx9 ) && defined( target_game )
// Should probably be moved into a SteamConfig.hpp
#define sig_use_steam_services

#include "steam_api.h"
#include "Threads/tThread.hpp"
#include "tRingBuffer.hpp"

namespace Sig
{
	class tSteamRequest;
	class tSteamLeaderboardSubmissionRequest;
	class tSteamLeaderboardRecordsRequest;

	typedef tPair< u32, LeaderboardEntry_t > tLeaderboardEntryPair;
	typedef u32 tSteamRequestHandle;
	typedef u32 tSteamRequestHandleGenerator;
	
	static const tSteamRequestHandle cInvalidSteamRequestHandle = ~0u;

	class tSteamServices
	{
	public:

		static tSteamServices* fInstance( );

		//  Try to activate steam.
		b32  fAuthenticate( );

		//  Get the currently logged in user.
		u64  fGetCurrentUserID( );

		//  Get whether the user is currently logged in.
		b32  fGetUserLoggedOn( );

		//  Pump the Steam Service engine.
		void fUploadLeaderboardScoreMT( const u32& leaderboardID, const s32& score, const tDynamicArray<s32>& details);
		
		//	Request entries for a specific leader board.
		tSteamRequestHandle fGetLeaderboardEntriesMT( const u32& leaderboardID, const u32& numEntriesToRead, const u32& startReadingFromRank, const b32& readFriendsOnly);

		//  Query whether an issued, returnable steam request has returned to the service.
		b32 fRequestHasReturned( tSteamRequestHandle handle );

		//  Obtain a returned steam request from the service.
		tSteamRequest* fGetReturnedSteamRequest( tSteamRequestHandle handle );
		
		//  Pump the Steam Service engine.
		void fUpdateSteamEngine( );


	private:

		//	Request leaderboard information.
		void fFindLeaderboard( const u32& leaderboardID );

		//	Request entries for a specific leaderboard.
		void fGetLeaderboardEntries( SteamLeaderboard_t handle );

		// Called when SteamUserStats()->FindOrCreateLeaderboard() returns asynchronously
		void OnFindLeaderboard( LeaderboardFindResult_t *pFindLeaderboardResult, bool bIOFailure );
		CCallResult<tSteamServices, LeaderboardFindResult_t> mSteamCallResultCreateLeaderboard;

		// Called when SteamUserStats()->DownloadLeaderboardEntries () returns asynchronously
		void OnGetLeaderboardEntries( LeaderboardScoresDownloaded_t *pLeaderboardScoresResult, bool bIOFailure );
		CCallResult<tSteamServices, LeaderboardScoresDownloaded_t> mSteamCallResultGetLeaderboardEntries;

		// Called when SteamUserStats()->UploadLeaderboardScore () returns asynchronously
		void OnUploadScore( LeaderboardScoreUploaded_t *pResult, bool bIOFailure);
		CCallResult<tSteamServices, LeaderboardScoreUploaded_t> m_callResultUploadScore;

		//  Return a completed steam request.
		b32 fReturnSteamRequest( tSteamRequest* request );

		
		
	private:
		//CTOR
		tSteamServices( );

		//MT Mains			  
		static u32 thread_call fRequestMain_MT( void* );

	private: //data
		tRingBuffer< tSteamRequest* > mSteamRequests;
		Threads::tThread mActivityThread;

		//handles and storage for returning request
		tSteamRequestHandleGenerator mSteamRequestHandleGenerator;
		Threads::tCriticalSection mReturnedSteamRequestsMTProtection;
		tHashTable< tSteamRequestHandle, tSteamRequest* > mReturnedSteamRequests;
	};

	// ---------------------------------------------------------------------------------------------- //

	// Base class for all steam request objects (found below)
	class tSteamRequest
	{
	public:
		tSteamRequest() : mRequestHandle( cInvalidSteamRequestHandle ) { };
		virtual b32 fDone( ) = 0;
		virtual void fSendRequest( ) = 0;
		tSteamServices* service;
		tSteamRequestHandle mRequestHandle;
	};

	// ---------------------------------------------------------------------------------------------- //

	class tSteamLeaderboardSubmissionRequest : public tSteamRequest
	{
	public:
		tSteamLeaderboardSubmissionRequest( const u32& leaderboardID, const s32& score, const tDynamicArray<s32>& details);

		virtual b32 fDone( );

		virtual void fSendRequest( );

		//	Request leaderboard information.
		void fFindLeaderboard( const u32& leaderboardID );

		void fOnFindLeaderboard( LeaderboardFindResult_t *pFindLeaderboardResult, bool bIOFailure );
		CCallResult<tSteamLeaderboardSubmissionRequest, LeaderboardFindResult_t> m_callResultFindLeaderboard;

		void fOnReceivedSubmissionResponse( LeaderboardScoreUploaded_t *pResult, bool bIOFailure);
		CCallResult<tSteamLeaderboardSubmissionRequest, LeaderboardScoreUploaded_t> m_callResultUploadScore;
	private:
		b32 mSubmissionComplete;
		u32 mLeaderboardID;
		s32 mScore;
		tDynamicArray<s32> mDetails;
	};

	// ---------------------------------------------------------------------------------------------- //

	class tSteamLeaderboardRecordsRequest : public tSteamRequest
	{
	public:

		struct tReturnedSteamLeaderboardEntry
		{
			u32						mRank;
			std::wstring			mUserName;
			u32						mScore;
			tDynamicArray< u32 >	mDetails;
		};

		tSteamLeaderboardRecordsRequest( const u32& leaderboardID, const u32& numDetails, const u32& numEntriesToRead, const u32& startReadingFromRank, const b32& readFriendsOnly);

		virtual b32 fDone( );

		virtual void fSendRequest( );

		//	Request leaderboard information.
		void fFindLeaderboard( const u32& leaderboardID );

		void fOnFindLeaderboard( LeaderboardFindResult_t *pFindLeaderboardResult, bool bIOFailure );
		CCallResult<tSteamLeaderboardRecordsRequest, LeaderboardFindResult_t> m_callResultFindLeaderboard;

		void fOnGetLeaderboardEntries( LeaderboardScoresDownloaded_t *pLeaderboardScoresResult, bool bIOFailure );
		CCallResult<tSteamLeaderboardRecordsRequest, LeaderboardScoresDownloaded_t> mSteamCallResultGetLeaderboardEntries;

		tDynamicArray< tReturnedSteamLeaderboardEntry > fGetReturnedEntries( );

		u32 fGetLeaderboardID( );

	private:
		u32 mLeaderboardID;
		u32 mNumEntriesToRead;
		u32 mStartReadingFromRank;
		u32 mNumDetailsToRead;
		b32 mReadFriendsOnly;
		b32 mRequestComplete;

		// returned leaderboard details
		tDynamicArray< tReturnedSteamLeaderboardEntry > mReturnedEntries;
		
	};
}

#endif // using steam
#endif //__tSteamServices__
