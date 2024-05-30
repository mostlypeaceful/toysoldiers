#include "BasePch.hpp"
#include "tSteamServices.hpp"

#if defined( sig_use_steam_services )

#include <sstream>

namespace Sig
{
	tSteamServices* service_instance = NULL;

	tSteamServices* tSteamServices::fInstance( )
	{
		if( !service_instance )
			service_instance = new tSteamServices( );

		return service_instance;
	}


	//private CTOR
	tSteamServices::tSteamServices( ) : mSteamRequestHandleGenerator( 0 )
	{
		mSteamRequests.fResize( 20 );
	}

	b32  tSteamServices::fAuthenticate( )
	{
		return SteamAPI_Init( );
	}

	u64  tSteamServices::fGetCurrentUserID( )
	{
		return SteamUser()->GetSteamID( ).ConvertToUint64( );
	}

	b32  tSteamServices::fGetUserLoggedOn( )
	{
		return SteamUser()->BLoggedOn();
	}

	void tSteamServices::fUpdateSteamEngine( )
	{
		SteamAPI_RunCallbacks();

		if( !mActivityThread.fRunning( ) )
		{
			if( mSteamRequests.fNumItems() > 0 )
			{
				tSteamRequest* req = NULL;//mLeaderboardSubmissions.fBack( );
				mSteamRequests.fTryPopFirst( req );
				req->service = this;
				mActivityThread.fStart( &tSteamServices::fRequestMain_MT, "&tSteamServices::fRequestMain_MT", req );
			}
		}
	}


	//Request leaderboard information.
	void tSteamServices::fFindLeaderboard( const u32& leaderboardID )
	{
		std::stringstream ss;
		ss << leaderboardID;
		SteamAPICall_t hSteamAPICall = SteamUserStats()->FindOrCreateLeaderboard( ss.str().c_str(),
			k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
		mSteamCallResultCreateLeaderboard.Set( hSteamAPICall, this, &tSteamServices::OnFindLeaderboard );
	}	

	// Called when SteamUserStats()->FindOrCreateLeaderboard() returns asynchronously
	void tSteamServices::OnFindLeaderboard( LeaderboardFindResult_t *pFindLearderboardResult, bool bIOFailure )
	{
	}

	//	Request entries for a specific leader board.
	void tSteamServices::fGetLeaderboardEntries( SteamLeaderboard_t handle )
	{
		SteamAPICall_t hSteamAPICall = SteamUserStats()->DownloadLeaderboardEntries( handle, k_ELeaderboardDataRequestGlobal, 1, 2 );
		mSteamCallResultGetLeaderboardEntries.Set( hSteamAPICall, this, &tSteamServices::OnGetLeaderboardEntries );
	}

	// Called when SteamUserStats()->DownloadLeaderboardEntries () returns asynchronously
	void tSteamServices::OnGetLeaderboardEntries( LeaderboardScoresDownloaded_t *pLeaderboardScoresResult, bool bIOFailure )
	{
		if( pLeaderboardScoresResult )
		{
			log_line( 0, "LEADERBOARD: " << SteamUserStats()->GetLeaderboardName( pLeaderboardScoresResult->m_hSteamLeaderboard )  << "\n====================");
			for( int i = 0; i < pLeaderboardScoresResult->m_cEntryCount; ++i )
			{
				log_line( 0, "----------\nENTRY\n----------");
				s32 details[3] = {0};
				LeaderboardEntry_t leaderboardEntry;
				SteamUserStats()->GetDownloadedLeaderboardEntry( pLeaderboardScoresResult->m_hSteamLeaderboardEntries, i, &leaderboardEntry, details, 3 );
				log_line( 0, "RANK: " << leaderboardEntry.m_nGlobalRank );
				log_line( 0, "NAME: " << SteamFriends()->GetFriendPersonaName( leaderboardEntry.m_steamIDUser ) );
				log_line( 0, "SKOR: " << leaderboardEntry.m_nScore );
				log_line( 0, "DETAILS: " << details[0] << " " << details[1] << " " << details[2] );
			}
			log_line( 0,"=====================");
		}
	}

	void tSteamServices::OnUploadScore( LeaderboardScoreUploaded_t *pResult, bool bIOFailure)
	{
		log_line(0, "OnUploadScore" );
	}

	//  Return a completed steam request.
	b32 tSteamServices::fReturnSteamRequest( tSteamRequest* request )
	{
		if( request->mRequestHandle == cInvalidSteamRequestHandle )
			return false;

		Threads::tMutex lock( mReturnedSteamRequestsMTProtection );
		mReturnedSteamRequests[ request->mRequestHandle ] = request;

		return true;
	}

	//  Query whether an issued, returnable steam request has returned to the service.
	b32 tSteamServices::fRequestHasReturned( tSteamRequestHandle handle )
	{
		Threads::tMutex lock( mReturnedSteamRequestsMTProtection );
		if( mReturnedSteamRequests.fFind( handle ) != NULL )
			return true;

		return false;
	}

	//  Obtain a returned steam request from the service.
	tSteamRequest* tSteamServices::fGetReturnedSteamRequest( tSteamRequestHandle handle )
	{
		Threads::tMutex lock( mReturnedSteamRequestsMTProtection );
		tSteamRequest** ppRequest = mReturnedSteamRequests.fFind( handle );
		
		if( NULL == ppRequest )
			return NULL;

		tSteamRequest* pRequest = *ppRequest;

		mReturnedSteamRequests.fRemove( ppRequest );
		return pRequest;
	}

	u32 tSteamServices::fRequestMain_MT( void* param )
	{
		tSteamRequest* request = static_cast<tSteamRequest*>(param);
		
		if( !request )
			return 0;

		request->fSendRequest( );

		while( !request->fDone( ) )
			fSleep( 5 );

		if( request->mRequestHandle != cInvalidSteamRequestHandle )
			request->service->fReturnSteamRequest( request );
		else
			delete request;

		return 0;
	}

	//  Upload a score for the current user.
	void tSteamServices::fUploadLeaderboardScoreMT( const u32& leaderboardID, const s32& score, const tDynamicArray<s32>& details)
	{
		mSteamRequests.fPushLast( new tSteamLeaderboardSubmissionRequest( leaderboardID, score, details) );
	}

	//	Request entries for a specific leader board.
	tSteamRequestHandle tSteamServices::fGetLeaderboardEntriesMT( const u32& leaderboardID, const u32& numEntriesToRead, const u32& startReadingFromRank, const b32& readFriendsOnly)
	{
		tSteamLeaderboardRecordsRequest* request = new tSteamLeaderboardRecordsRequest( leaderboardID, 0, numEntriesToRead, startReadingFromRank, readFriendsOnly  );
		request->mRequestHandle = mSteamRequestHandleGenerator++;
		mSteamRequests.fPushLast( request );
		return request->mRequestHandle;
	}

	// ----------------------------------------------------------------------------- //
	

	// ----------------------------------------------------------------------------- //
	tSteamLeaderboardSubmissionRequest::tSteamLeaderboardSubmissionRequest( const u32& leaderboardID, const s32& score, const tDynamicArray<s32>& details)
	{
		mScore = score;
		mDetails = details;
		mLeaderboardID = leaderboardID;
		mSubmissionComplete = false;
	}

	b32  tSteamLeaderboardSubmissionRequest::fDone( )
	{
		return mSubmissionComplete;
	}

	//	Request leaderboard information.
	void tSteamLeaderboardSubmissionRequest::fFindLeaderboard( const u32& leaderboardID )
	{
		std::stringstream ss;
		ss << leaderboardID;
		SteamAPICall_t hSteamAPICall = SteamUserStats()->FindOrCreateLeaderboard(	ss.str().c_str(),
																					k_ELeaderboardSortMethodDescending, 
																					k_ELeaderboardDisplayTypeNumeric );

		m_callResultFindLeaderboard.Set( hSteamAPICall, this, &tSteamLeaderboardSubmissionRequest::fOnFindLeaderboard );
	}	

    void tSteamLeaderboardSubmissionRequest::fOnFindLeaderboard( LeaderboardFindResult_t *pFindLeaderboardResult, bool bIOFailure )
	{
		if( !pFindLeaderboardResult->m_bLeaderboardFound )
		{
			mSubmissionComplete = true;
			return;
		}

		SteamAPICall_t hSteamAPICall = 
		SteamUserStats()->UploadLeaderboardScore( pFindLeaderboardResult->m_hSteamLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, mScore, mDetails.fBegin(), mDetails.fCount() );
		m_callResultUploadScore.Set( hSteamAPICall, this, &tSteamLeaderboardSubmissionRequest::fOnReceivedSubmissionResponse );
	}

	void tSteamLeaderboardSubmissionRequest::fSendRequest( )
	{
		fFindLeaderboard( mLeaderboardID );
	}

	void tSteamLeaderboardSubmissionRequest::fOnReceivedSubmissionResponse( LeaderboardScoreUploaded_t *pResult, bool bIOFailure)
	{
		mSubmissionComplete = true;
	}

	// ---------------------------------------------------------------------------------------------- //

	tSteamLeaderboardRecordsRequest::tSteamLeaderboardRecordsRequest( const u32& leaderboardID, const u32& numDetailsToRead, const u32& numEntriesToRead, const u32& startReadingFromRank, const b32& readFriendsOnly) : 
		mLeaderboardID( leaderboardID ) , 
		mNumDetailsToRead( numDetailsToRead ),
		mNumEntriesToRead( numEntriesToRead ), 
		mStartReadingFromRank( startReadingFromRank ), 
		mReadFriendsOnly( readFriendsOnly ), 
		mRequestComplete( false )
	{

	}

	b32 tSteamLeaderboardRecordsRequest::fDone( )
	{
		return mRequestComplete;
	}

	void tSteamLeaderboardRecordsRequest::fSendRequest( )
	{
		fFindLeaderboard( mLeaderboardID );
	}

	//	Request leaderboard information.
	void tSteamLeaderboardRecordsRequest::fFindLeaderboard( const u32& leaderboardID )
	{
		std::stringstream ss;
		ss << leaderboardID;
		SteamAPICall_t hSteamAPICall = SteamUserStats()->FindOrCreateLeaderboard(	ss.str().c_str(),
																					k_ELeaderboardSortMethodDescending, 
																					k_ELeaderboardDisplayTypeNumeric );

		m_callResultFindLeaderboard.Set( hSteamAPICall, this, &tSteamLeaderboardRecordsRequest::fOnFindLeaderboard );
	}

	void tSteamLeaderboardRecordsRequest::fOnFindLeaderboard( LeaderboardFindResult_t *pFindLeaderboardResult, bool bIOFailure )
	{
		if( !pFindLeaderboardResult->m_bLeaderboardFound )
		{
			mRequestComplete = true;
			return;
		}

		SteamAPICall_t hSteamAPICall = SteamUserStats()->DownloadLeaderboardEntries( pFindLeaderboardResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobal, 1, 2 );
		mSteamCallResultGetLeaderboardEntries.Set( hSteamAPICall, this, &tSteamLeaderboardRecordsRequest::fOnGetLeaderboardEntries );
	}

	void tSteamLeaderboardRecordsRequest::fOnGetLeaderboardEntries( LeaderboardScoresDownloaded_t *pLeaderboardScoresResult, bool bIOFailure )
	{
		if( pLeaderboardScoresResult )
		{
			for( int i = 0; i < pLeaderboardScoresResult->m_cEntryCount; ++i )
			{
				LeaderboardEntry_t leaderboardEntry;
				s32 details[10]; //we never have even close to 10 details on a single page. 10 is safe without being retarded.  it's also a nice round number.    
				SteamUserStats()->GetDownloadedLeaderboardEntry( pLeaderboardScoresResult->m_hSteamLeaderboardEntries, i, &leaderboardEntry, details, 10 );

				tReturnedSteamLeaderboardEntry returned_entry;
				returned_entry.mRank = leaderboardEntry.m_nGlobalRank;
				returned_entry.mUserName = StringUtil::fStringToWString( SteamFriends()->GetFriendPersonaName( leaderboardEntry.m_steamIDUser ) );
				returned_entry.mScore = leaderboardEntry.m_nScore;
				for( s32 p = 0; p < leaderboardEntry.m_cDetails; p++ )
					returned_entry.mDetails.fPushBack( details[ p ]);

				mReturnedEntries.fPushBack( returned_entry );
			}
		}
		mRequestComplete = true;
	}

	tDynamicArray< tSteamLeaderboardRecordsRequest::tReturnedSteamLeaderboardEntry > tSteamLeaderboardRecordsRequest::fGetReturnedEntries( )
	{
		return mReturnedEntries;
	}

	u32 tSteamLeaderboardRecordsRequest::fGetLeaderboardID( ) { return mLeaderboardID; }

}

#else
// !sig_use_steam_services

void fDontWhineAboutMySteamServicesObjectFileBro() {}

#endif
