#include "GameAppPch.hpp"
#include "XlspManager.h"
#include "tXmlFile.hpp"

using namespace Sig;

namespace XLSP
{

#ifdef platform_xbox360

#define ENABLE_LSP

	namespace
	{
		void fScoresToGrowable( Sig::tGrowableArray<XLSP::XlspManager::Score>& output, std::list<XLSP::XlspManager::Score>& input )
		{
			output.fSetCount( 0 );

			for (std::list<XLSP::XlspManager::Score>::const_iterator iter = input.begin(); iter != input.end(); ++iter)
			{
				XLSP::XlspManager::Score score = XLSP::XlspManager::Score(*iter);
				output.fPushBack( score );
			}
		}
	}

	XlspManager::XlspManager() :
		requestID(0),
		scoresAroundScoreStatus(LspStatusUndefined),
		scoresForXuidsStatus(LspStatusUndefined),
		postScoreStatus(LspStatusUndefined),
		topScoresStatus(LspStatusUndefined),
		topScoresPlatformStatus(LspStatusUndefined),
		rankForScoreStatus(LspStatusUndefined),
		rankForScoreIsNewHighScore(false),
		mobileOwnershipRecordsStatus(LspStatusUndefined),
		setOwnershipStatus(LspStatusUndefined)
	{
	}

	void XlspManager::Initialize()
	{
	#ifdef ENABLE_LSP
		LSPConnection::LspGroup lspGroup = LSPConnection::LspGroup_Production; //todo: switch to the correct group you want the code to run against
		sigassert(lspGroup >= 0 && lspGroup < LSPConnection::LspGroup_Count);
		if (lspGroup < 0 || lspGroup >= LSPConnection::LspGroup_Count)
		{
			lspGroup = LSPConnection::LspGroup_Test;
		}
		lspConnection.Initialize(lspGroup);
	#endif
	}

	void XlspManager::Update(float elapsedSeconds)
	{
	#ifdef ENABLE_LSP
		//if (lspConnection.IsServiceAvailable())
		{
			//if requestID is non-zero, we are processing a request
			if (requestID != 0)
			{
				UpdateRequest();
			}
			else 
			{
				SendNextRequest();
			}
		}

		lspConnection.Update();
	#endif
	}

	void XlspManager::Cleanup()
	{
	#ifdef ENABLE_LSP
		lspConnection.Cleanup();
	#endif
	}

	void XlspManager::UpdateRequest()
	{
#ifdef ENABLE_LSP
		//wait for repsonse
		if (!lspConnection.IsBusy(requestID))
		{
			bool succeeded = lspConnection.GetResult(requestID, &requestData.pResultBuffer, &requestData.resultSize);

			if (succeeded)
			{
				//log_line(Tag_XLSP, "XlspManager::UpdateRequest() %s", requestData.pResultBuffer);

				//check for valid data
				//response should be a string containing http headers + an xml document.
				//the GetResult call guarantees that the returned buffer is null terminated.
				int responseCode = 0;
				sscanf_s(requestData.pResultBuffer, "HTTP/1.1 %d", &responseCode ); 
				const int httpCodeOK = 200;
				succeeded = (responseCode == httpCodeOK);

				if (succeeded)
				{
					//TCR: can't use LSP unless an online player
					//toss out results if we happened to fire off a request that succeeded 
					//also toss out resutls if xuids don't match
					Player *pPlayer = g_PlayerManager.GetLocalPlayer();
					succeeded = pPlayer != NULL && (XUserGetSigninState(pPlayer->GetUserIndex()) == eXUserSigninState_SignedInToLive);
					if (!succeeded)
					{
						log_line(Tag_XLSP, "XlspManager::UpdateRequest() error: results came back ok, but local player was not signed in to live or did not have multiplayer priviledge");
					}
				}
				else
				{
					log_line(Tag_XLSP, "XlspManager::UpdateRequest() error: responseCode " << responseCode);
				}
			}
			else
			{
				log_line(Tag_XLSP, "XlspManager::UpdateRequest() lspConnection.GetResult() error");
			}

			//handle result, successful or not
			switch (requestData.requestType)
			{
			case RequestTypeGetScoresAroundScore:
				GetScoresAroundScoreCompleted(succeeded);
				break;
			case RequestTypeGetScoresForXuids:
				GetScoresForXuidsCompleted(succeeded);
				break;
			case RequestTypePostScore:
				PostScoreCompleted(succeeded);
				break;
			case RequestTypeTopScores:
				TopScoresCompleted(succeeded);
				break;
			case RequestTypeTopScoresPlatform:
				TopScoresPlatformCompleted(succeeded);
				break;
			case RequestTypeGetRankForScore:
				GetRankForScoreCompleted(succeeded);
				break;
			case RequestTypeGetOwnershipRecords:
				GetOwnershipRecordsCompleted(succeeded);
				break;
			case RequestTypeSetOwnershipRecord:
				SetOwnershipRecordCompleted(succeeded);
				break;
			}

			//done with request
			requestID = 0;
		}
#endif
	}

	void XlspManager::SendNextRequest()
	{
	#ifdef ENABLE_LSP
		//send next request in queue
		if (requests.size() > 0)
		{
			Request request = requests.front();
			RequestType requestType = request.requestType;

			switch (requestType)
			{
			case RequestTypeGetScoresAroundScore:
				SendGetScoresAroundScore(request.xuid, request.machineId, request.data1, request.data2);
				break;
			case RequestTypeGetScoresForXuids:
				SendGetScoresForXuids(request.xuid, request.machineId, request.data1, request.xuids);
				break;
			case RequestTypePostScore:
				SendPostScore(request.xuid, request.machineId, request.data1, request.data2);
				break;
			case RequestTypeTopScores:
				SendTopScores(request.machineId, request.data1);
				break;
			case RequestTypeTopScoresPlatform:
				SendTopScoresPlatform(request.machineId, request.data1, request.data2);
				break;
			case RequestTypeGetRankForScore:
				SendGetRankForScore(request.machineId, request.data1, request.data2, request.xuid);
				break;
			case RequestTypeGetOwnershipRecords:
				SendGetOwnershipRecords(request.xuid, request.machineId, request.data1);
				break;
			case RequestTypeSetOwnershipRecord:
				SendSetOwnershipRecord(request.xuid, request.machineId, request.data1, request.data2);
				break;
			}

			requests.pop_front();
		}
	#endif
	}

	XlspManager::LspStatus XlspManager::GetScoresAroundScore(Sig::tGrowableArray<Score> &scores)
	{
		fScoresToGrowable( scores, scoresAroundScore );
		return scoresAroundScoreStatus;
	}

	void XlspManager::RequestGetScoresAroundScore(XUID playerXuid, int machineId, int scoresAbove, int scoresBelow)
	{
		Request request(RequestTypeGetScoresAroundScore, playerXuid, machineId);
		request.data1 = scoresAbove;
		request.data2 = scoresBelow;
		requests.push_back(request);
		scoresAroundScoreStatus = LspStatusUpdating;
		scoresAroundScore.clear();
	}

	void XlspManager::SendGetScoresAroundScore(XUID playerXuid, int machineId, int scoresAbove, int scoresBelow)
	{
		//http://localhost/XLSP_api.do?req=GetScoresAroundScore&machineId=%d&platform=%d&above=%d&below=%d&xuid=%I64d
		requestData.sendSize = sprintf_s(requestData.sendBuffer, _countof(requestData.sendBuffer), 
			"GET /api_hamster.do?req=GetScoresAroundScore&machineId=%d&above=%d&below=%d&xuid=%I64d HTTP/1.1\r\nHost: %s\r\n\r\n",
			machineId,
			scoresAbove,
			scoresBelow,	
			playerXuid, 
			lspConnection.GetServerName()
			);

		requestData.requestType = RequestTypeGetScoresAroundScore;

		requestID = lspConnection.Send(requestData.sendBuffer, requestData.sendSize, true);
	}

	void XlspManager::GetScoresAroundScoreCompleted(bool succeeded)
	{
		/*sample result
		<?xml version="1.0" encoding="utf-8" ?> 
		- <scores>
		<rank rank="4" forScore="21500" XUID="267301344" gamertag="??????" /> 
		<score score="2500" XUID="267301348" gamertag="??????" /> 
		<score score="2600" XUID="267301346" gamertag="??????" /> 
		<score score="21000" XUID="267301345" gamertag="??????" /> 
		<score score="21500" XUID="267301344" gamertag="??????" /> 
		<score score="21505" XUID="267301351" gamertag="??????" /> 
		</scores>
		*/

#ifdef ENABLE_LSP
		scoresAroundScore.clear();


		if (succeeded)
		{
			log_line(Tag_XLSP, "XlspManager::GetScoresAroundScoreCompleted() succeeded");
			scoresAroundScoreStatus = LspStatusUpdated;
			log_line( 0, "data: " << requestData.pResultBuffer );

			std::string results( requestData.pResultBuffer );
			u32 beginning = results.find( "<scores>" );

			tXmlFile file;

			if( beginning != -1 )
			{
				results = results.substr( beginning );
				log_line( 0, "trimmed: " << results );
				file.fConstructTree( results );
			}


			tXmlNode root = file.fGetRoot( );
			tXmlNode rank;

			if( !root.fNull( ) && root.fFindChild( rank, "rank" ) )
			{
				u32 rankNumber;
				tPlatformUserId rankXuid;

				if( rank.fGetAttribute( "rank", rankNumber ) 
					&& rank.fGetAttribute( "XUID", rankXuid ) )
				{
					tXmlNodeList children;
					root.fFindChildren( children, "score" );

					for( u32 i = 0; i < children.fCount( ); ++i )
					{
						u32 score;
						tPlatformUserId xuid;
						std::string gamertag;

						tXmlNode& child = children[ i ];
						if( child.fGetAttribute( "score", score ) && child.fGetAttribute( "XUID", xuid ) && child.fGetAttribute( "gamertag", gamertag ) )
						{
							Score xScore;
							xScore.score = score;
							xScore.xuid = xuid;
							xScore.rank = (xuid == rankXuid) ? rankNumber : 0;
							xScore.platform = -1;

							u32 strLen = fMin( gamertag.length( ), array_length( xScore.gamertag ) - 1 ); // (?)
							memcpy( xScore.gamertag, gamertag.c_str( ), strLen );
							xScore.gamertag[ strLen ] = 0; //null terminate (?)

							scoresAroundScore.push_back( xScore );
						}
					}
				}

			}
		}
		else
		{
			log_line(Tag_XLSP, "XlspManager::GetScoresAroundScoreCompleted() failed");
			scoresAroundScoreStatus = LspStatusFailure;
		}
	#endif
	}


	XlspManager::LspStatus XlspManager::GetScoresForXuids(Sig::tGrowableArray<Score> &scores)
	{
		fScoresToGrowable( scores, scoresForXuids );
		return scoresForXuidsStatus;
	}

	void XlspManager::RequestGetScoresForXuids(XUID playerXuid, int machineId, int number, std::list<XUID> &xuids)
	{
		Request request(RequestTypeGetScoresForXuids, playerXuid, machineId);
		request.data1 = number;
		request.xuids = xuids;
		requests.push_back(request);
		scoresForXuidsStatus = LspStatusUpdating;
		scoresForXuids.clear();
	}

	void XlspManager::SendGetScoresForXuids(XUID playerXuid, int machineId, int number, std::list<XUID> &xuids)
	{
		//create an xml doc that represents a list of xuids and a list of machineIds (but we only use one machineId currently)
		char xmlDoc[8192] = "<?xml version=\"1.0\" encoding=\"utf-8\"?><request>";  //8k is enough for 256 xuids + xml formatting
		char temp[100];
		for (std::list<XUID>::const_iterator iter = xuids.begin(); iter != xuids.end(); ++iter)
		{
			XUID xuid = XUID(*iter);
			sprintf_s(temp, _countof(temp), "<xuid>%I64d</xuid>", xuid); 
			strcat(xmlDoc, temp);
		}
		sprintf_s(temp, _countof(temp), "<machine>%d</machine></request>", machineId); 
		strcat(xmlDoc, temp);
		
		//http://localhost/XLSP_api.do?req=LeaderboardGetTopScoresForXuids&xuid=%I64d&number=%d
		requestData.sendSize = sprintf_s(requestData.sendBuffer, _countof(requestData.sendBuffer), 
			"POST /api_hamster.do?req=LeaderboardGetTopScoresForXuids&xuid=%I64d&number=%d HTTP/1.1\r\nHost: %s\r\nContent-Type: text/xml\r\nContent-Length: %d\r\n\r\n%s",
			playerXuid,
			number,
			lspConnection.GetServerName(),
			strlen(xmlDoc),
			xmlDoc
			);

		requestData.requestType = RequestTypeGetScoresForXuids;

		requestID = lspConnection.Send(requestData.sendBuffer, requestData.sendSize, true);
	}

	void XlspManager::GetScoresForXuidsCompleted(bool succeeded)
	{
		/* sample result
		<?xml version="1.0" encoding="utf-8" ?> 
		<leaderboard>
		<entry xuid="267301349" gamertag="??????" machineId="13" rank="1" score="3750" platform="10" replayId="15532" replaySize="3" /> 
		<entry xuid="267301347" gamertag="??????" machineId="13" rank="2" score="1750" platform="10" replayId="15530" replaySize="3" /> 
		<entry xuid="267301351" gamertag="??????" machineId="13" rank="3" score="1505" platform="11" replayId="15593" replaySize="3" /> 
		<entry xuid="267301344" gamertag="??????" machineId="13" rank="4" score="1500" platform="10" replayId="15526" replaySize="3" /> 
		<entry xuid="267301345" gamertag="??????" machineId="13" rank="5" score="1000" platform="10" replayId="15528" replaySize="3" /> 
		<entry xuid="267301346" gamertag="??????" machineId="13" rank="6" score="600" platform="10" replayId="15529" replaySize="3" /> 
		<entry xuid="267301348" gamertag="??????" machineId="13" rank="7" score="500" platform="10" replayId="15531" replaySize="3" /> 
		</leaderboard>
		*/
	#ifdef ENABLE_LSP
		if (succeeded)
		{
			log_line(Tag_XLSP, "XlspManager::GetScoresForXuidsCompleted() succeeded");

			ParseScoresIntoList(scoresForXuids);

			scoresForXuidsStatus = LspStatusUpdated;
		}
		else
		{
			log_line(Tag_XLSP, "XlspManager::GetScoresForXuidsCompleted() failed");
			scoresForXuidsStatus = LspStatusFailure;
		}
	#endif
	}

	XlspManager::LspStatus XlspManager::GetPostScoreStatus()
	{
		return postScoreStatus;
	}

	void XlspManager::RequestPostScore(XUID playerXuid, int machineId, int score, int platform)
	{
		Request request(RequestTypePostScore, playerXuid, machineId);
		request.data1 = score;
		request.data2 = platform;
		requests.push_back(request);
		postScoreStatus = LspStatusUpdating;
	}

	void XlspManager::SendPostScore(XUID playerXuid, int machineId, int score, int platform)
	{
		///XLSP_api.do?req=LeaderboardPostScore&xuid=%I64d&machineId=%d&score=%I64d&platform=%d

		//send in the smallest chunk of data we can send for the bidy with base64 encoding, since this server method wants a replay file, but we don't have one to send
		//we'll use the base64 code for "NIL" ("TklM")
		requestData.sendSize = sprintf_s(requestData.sendBuffer, _countof(requestData.sendBuffer), 
			"GET /api_hamster.do?req=LeaderboardPostScore&xuid=%I64d&machineId=%d&score=%d&platform=%d&body=TklM HTTP/1.1\r\nHost: %s\r\n\r\n",
			playerXuid, 
			machineId,
			score,
			platform,
			lspConnection.GetServerName()
			);

		requestData.requestType = RequestTypePostScore;

		requestID = lspConnection.Send(requestData.sendBuffer, requestData.sendSize, true);
	}

	void XlspManager::PostScoreCompleted(bool succeeded)
	{
#ifdef ENABLE_LSP
		if (succeeded)
		{
			log_line(Tag_XLSP, "XlspManager::PostScoreCompleted() succeeded");
			postScoreStatus = LspStatusUpdated;
		}
		else
		{
			log_line(Tag_XLSP, "XlspManager::PostScoreCompleted() failed");
			postScoreStatus = LspStatusFailure;
		}
#endif
	}

	XlspManager::LspStatus XlspManager::GetTopScores(Sig::tGrowableArray<Score> &scores)
	{
		fScoresToGrowable( scores, topScores );
		return topScoresStatus;
	}

	void XlspManager::RequestTopScores(int machineId, int number)
	{
		Request request(RequestTypeTopScores, machineId);
		request.data1 = number;
		requests.push_back(request);
		topScoresStatus = LspStatusUpdating;
		topScores.clear();
	}

	void XlspManager::SendTopScores(int machineId, int number)
	{
		//http://localhost/XLSP_api.do?req=LeaderboardGetTopScores&machineId=%d&number=%d

		requestData.sendSize = sprintf_s(requestData.sendBuffer, _countof(requestData.sendBuffer), 
			"GET /api_hamster.do?req=LeaderboardGetTopScores&machineId=%d&number=%d HTTP/1.1\r\nHost: %s\r\n\r\n",
			machineId,
			number, 
			lspConnection.GetServerName()
			);

		requestData.requestType = RequestTypeTopScores;

		requestID = lspConnection.Send(requestData.sendBuffer, requestData.sendSize, true);
	}

	void XlspManager::TopScoresCompleted(bool succeeded)
	{
		/*
		<?xml version="1.0" encoding="utf-8" ?> 
		<leaderboard>
		<entry xuid="267301349" gamertag="??????" machineId="13" rank="1" score="3750" platform="10" replayId="15532" replaySize="3" /> 
		<entry xuid="267301347" gamertag="??????" machineId="13" rank="2" score="1750" platform="10" replayId="15530" replaySize="3" /> 
		<entry xuid="267301351" gamertag="??????" machineId="13" rank="3" score="1505" platform="11" replayId="15593" replaySize="3" /> 
		<entry xuid="267301344" gamertag="??????" machineId="13" rank="4" score="1500" platform="10" replayId="15526" replaySize="3" /> 
		<entry xuid="267301345" gamertag="??????" machineId="13" rank="5" score="1000" platform="10" replayId="15528" replaySize="3" /> 
		<entry xuid="267301346" gamertag="??????" machineId="13" rank="6" score="600" platform="10" replayId="15529" replaySize="3" /> 
		<entry xuid="267301348" gamertag="??????" machineId="13" rank="7" score="500" platform="10" replayId="15531" replaySize="3" /> 
		</leaderboard>
		*/
	#ifdef ENABLE_LSP
		if (succeeded)
		{
			log_line(Tag_XLSP, "XlspManager::TopScoresCompleted() succeeded");

			ParseScoresIntoList(topScores);

			topScoresStatus = LspStatusUpdated;
		}
		else
		{
			log_line(Tag_XLSP, "XlspManager::TopScoresCompleted() failed");
			topScoresStatus = LspStatusFailure;
		}
	#endif
	}

	XlspManager::LspStatus XlspManager::GetTopScoresPlatform(Sig::tGrowableArray<Score> &scores)
	{
		scores.fSetCount( 0 );

		for (std::list<XlspManager::Score>::const_iterator iter = topScoresPlatform.begin(); iter != topScoresPlatform.end(); ++iter)
		{
			XlspManager::Score score = XlspManager::Score(*iter);
			scores.fPushBack( score );
		}

		return topScoresPlatformStatus;
	}

	void XlspManager::RequestTopScoresPlatform(int machineId, int platform, int number)
	{
		Request request(RequestTypeTopScoresPlatform, machineId);
		request.data1 = platform;
		request.data2 = number;
		requests.push_back(request);
		topScoresPlatformStatus = LspStatusUpdating;
		topScoresPlatform.clear();
	}

	void XlspManager::SendTopScoresPlatform(int machineId, int platform, int number)
	{
		//http://localhost/XLSP_api.do?req=LeaderboardGetTopScoresForPlatform&machineId=%d&platform=%d&number=%d

		requestData.sendSize = sprintf_s(requestData.sendBuffer, _countof(requestData.sendBuffer), 
			//"GET /api_hamster.do?req=NativeSproc&sproc=TopScoresPlatformSproc&machineId=%d&platform=%d&number=%d HTTP/1.1\r\nHost: %s\r\n\r\n",
			"GET /api_hamster.do?req=LeaderboardGetTopScoresForPlatform&machineId=%d&platform=%d&number=%d HTTP/1.1\r\nHost: %s\r\n\r\n",
			machineId,
			platform, 
			number,
			lspConnection.GetServerName()
			);

		requestData.requestType = RequestTypeTopScoresPlatform;

		requestID = lspConnection.Send(requestData.sendBuffer, requestData.sendSize, true);
	}

	//get top score for a given machineID on a given platform
	void XlspManager::TopScoresPlatformCompleted(bool succeeded)
	{
		/*		  
		<?xml version="1.0" encoding="utf-8" ?> 
		- <leaderboard>
		<entry xuid="267301349" gamertag="??????" machineId="13" rank="1" score="23750" platform="10" replayId="15616" replaySize="3" /> 
		<entry xuid="267301347" gamertag="??????" machineId="13" rank="2" score="21750" platform="10" replayId="15614" replaySize="3" /> 
		<entry xuid="267301351" gamertag="??????" machineId="13" rank="3" score="21505" platform="10" replayId="15608" replaySize="3" /> 
		</leaderboard>  
		*/

	#ifdef ENABLE_LSP
		if (succeeded)
		{
			ParseScoresIntoList(topScoresPlatform);

			log_line(Tag_XLSP, "XlspManager::TopScoresPlatformCompleted() succeeded, records: " << topScoresPlatform.size( ) );


			topScoresPlatformStatus = LspStatusUpdated;
		}
		else
		{
			log_line(Tag_XLSP, "XlspManager::TopScoresPlatformCompleted() failed");
			topScoresPlatformStatus = LspStatusFailure;
		}
	#endif


	}

	XlspManager::LspStatus XlspManager::GetRankForScore(Score &score, bool &isNewHighScore)
	{
		score = rankForScore;
		isNewHighScore = rankForScoreIsNewHighScore;

		return rankForScoreStatus;
	}

	void XlspManager::RequestGetRankForScore(int machineId, int score, int platform, XUID playerXuid)
	{
		Request request(RequestTypeGetRankForScore, playerXuid, machineId);
		request.data1 = score;
		request.data2 = platform;
		requests.push_back(request);
		rankForScoreStatus = LspStatusUpdating;
	}

	void XlspManager::SendGetRankForScore(int machineId, int score, int platform, XUID playerXuid)
	{
		///XLSP_api.do?req=LeaderboardGetRankForScore&platform=%d&machineId=%d&score=%d&xuid=%I64d

		requestData.sendSize = sprintf_s(requestData.sendBuffer, _countof(requestData.sendBuffer), 
			"GET /api_hamster.do?req=LeaderboardGetRankForScore&platform=%d&machineId=%d&score=%d&xuid=%I64d HTTP/1.1\r\nHost: %s\r\n\r\n",
			platform,
			machineId,
			score, 
			playerXuid,	
			lspConnection.GetServerName()
			);

		requestData.requestType = RequestTypeGetRankForScore;

		//clear/init current data
		rankForScore.gamertag[0] = 0;
		rankForScore.platform = platform;
		rankForScore.rank = -1;
		rankForScore.score = score;
		rankForScore.xuid = playerXuid;
		rankForScoreIsNewHighScore = false;

		requestID = lspConnection.Send(requestData.sendBuffer, requestData.sendSize, true);
	}

	void XlspManager::GetRankForScoreCompleted(bool succeeded)
	{
		/*	<?xml version="1.0" encoding="utf-8" ?> 
			<leaderboardPosition rank="8" isNewHighScore="False" requiresReplay="False" /> 
		*/
	#ifdef ENABLE_LSP
		if (succeeded)
		{
			log_line(Tag_XLSP, "XlspManager::GetRankForScoreCompleted() succeeded");

			const char *pStr = requestData.pResultBuffer;
			pStr = ParseForIntAttribute(pStr, "rank=\"", rankForScore.rank);
			if (pStr != NULL)
			{
				pStr = ParseForBooleanAttribute(pStr, "isNewHighScore=\"", rankForScoreIsNewHighScore);
			}

			log_line(Tag_XLSP, "XlspManager::GetRankForScoreCompleted() rank: " << rankForScore.rank << " isNewHighScore: " << (rankForScoreIsNewHighScore?"true":"false") );
			rankForScoreStatus = LspStatusUpdated;
		}
		else
		{
			log_line(Tag_XLSP, "XlspManager::GetRankForScoreCompleted() failed");
			rankForScoreStatus = LspStatusFailure;
		}
	#endif
	}


	XlspManager::LspStatus XlspManager::GetOwnershipRecords(std::list<OwnershipRecord> &records)
	{
		records = mobileOwnershipRecords;

		return mobileOwnershipRecordsStatus;
	}

	void XlspManager::RequestGetOwnershipRecords(XUID playerXuid, int titleId, int platform)
	{
		Request request(RequestTypeGetOwnershipRecords, playerXuid, titleId);
		request.data1 = platform;
		requests.push_back(request);
		mobileOwnershipRecordsStatus = LspStatusUpdating;
		mobileOwnershipRecords.clear();
	}

	void XlspManager::SendGetOwnershipRecords(XUID playerXuid, int titleId, int platform)
	{
		//XLSP_api.do?req=BopaGetOwnershipRecords&xuid=267301344&titleId=12345678
		//http://localhost//XLSP_api.do?req=BopaGetOwnershipRecords&xuid=%I64d&titleId=%d

		requestData.sendSize = sprintf_s(requestData.sendBuffer, _countof(requestData.sendBuffer), 
			"GET /XLSP_api.do?req=%s&xuid=%I64d&titleId=%d HTTP/1.1\r\nHost: %s\r\n\r\n",
			((platform == PlatformMobile)?"BopaGetMobileOwnershipRecords":"BopaGetOwnershipRecords"),
			playerXuid,	
			titleId,
			lspConnection.GetServerName()
			);

		requestData.requestType = RequestTypeGetOwnershipRecords;

		requestID = lspConnection.Send(requestData.sendBuffer, requestData.sendSize, true);
	}

	void XlspManager::GetOwnershipRecordsCompleted(bool succeeded)
	{
		/*	<?xml version="1.0" encoding="utf-8" ?> 
		<content>
			<item contentId="1234" contentType="2" platform="1" purchaseType="S"/>
		</content>
		*/

	#ifdef ENABLE_LSP
		if (succeeded)
		{
			log_line(Tag_XLSP, "XlspManager::GetOwnershipRecordsCompleted() succeeded");

			mobileOwnershipRecords.clear();
	
			OwnershipRecord record;

			const char *pStr = requestData.pResultBuffer;
			while (pStr != NULL)
			{
				pStr = ParseForIntAttribute(pStr, " contentId=\"", record.contentId);
				if (pStr != NULL)
				{
					pStr = ParseForIntAttribute(pStr, " contentType=\"", record.contentType);
					if (pStr != NULL)
					{
						/*pStr = */ ParseForIntAttribute(pStr, " platform=\"", record.platform);
						if (record.platform == PlatformUnknown)
						{
							record.platform = PlatformMobile; //assume mobile platform, since the other ownership request will return platform ids
						}

						//if (pStr != NULL)
						{
							pStr = ParseForCharAttribute(pStr, " purchaseType=\"", record.purchaseType);
							if (pStr != NULL)
							{
								//add record to list
								log_line(Tag_XLSP, "XlspManager::GetOwnershipRecordsCompleted() contentId: " << record.contentId << " contentType: " << record.contentType << " platform: " << record.platform << " purchaseType: " << record.purchaseType << "\n" );
								mobileOwnershipRecords.push_back(record);
							}
						}
					}
				}
			}

			mobileOwnershipRecordsStatus = LspStatusUpdated;
		}
		else
		{
			log_line(Tag_XLSP, "XlspManager::GetOwnershipRecordsCompleted() failed");
			mobileOwnershipRecordsStatus = LspStatusFailure;
		}
	#endif
	}

	XlspManager::LspStatus XlspManager::GetSetOwnershipStatus()
	{
		return setOwnershipStatus;
	}

	void XlspManager::RequestSetOwnershipRecord(XUID playerXuid, int titleId, int contentId, int platform)
	{
		Request request(RequestTypeSetOwnershipRecord, playerXuid, titleId);
		request.data1 = contentId;
		request.data2 = platform;
		requests.push_back(request);
		setOwnershipStatus = LspStatusUpdating;
	}

	void XlspManager::SendSetOwnershipRecord(XUID playerXuid, int titleId, int contentId, int platform)
	{
		/*			
		/XLSP_api.do?req=BopaAddOwnershipRecords&xuid=%I64d&platform=%d&titleId=%d
		<request>
		<content>
		<id>123</id>
		<type>1</type>
		<purchaseType>S</purchaseType>
		</content>
		</request>
		*/

		if (platform == PlatformMobile)
		{
		//create an xml doc that represents the ownership record
		char xmlDoc[1024];
		sprintf_s(xmlDoc, _countof(xmlDoc), "<?xml version=\"1.0\" encoding=\"utf-8\"?><request><content><id>%d</id><type>0</type></content></request>", contentId);
		
		//http://localhost/XLSP_api.do?req=BopaAddMobileOwnershipRecords&xuid=%I64d&platform=%d&titleId=%d
		requestData.sendSize = sprintf_s(requestData.sendBuffer, _countof(requestData.sendBuffer), 
			"POST /api_hamster.do?req=BopaAddOwnershipRecordsForMobile&xuid=%I64d&titleId=%d HTTP/1.1\r\nHost: %s\r\nContent-Type: text/xml\r\nContent-Length: %d\r\n\r\n%s",
			playerXuid,
			titleId,
			lspConnection.GetServerName(),
			strlen(xmlDoc),
			xmlDoc
			);
		}
		else
		{
		//create an xml doc that represents the ownership record
		char xmlDoc[1024];
		sprintf_s(xmlDoc, _countof(xmlDoc), "<?xml version=\"1.0\" encoding=\"utf-8\"?><request><content><id>%d</id><type>0</type><purchaseType>S</purchaseType></content></request>", contentId);
		
		//http://localhost/XLSP_api.do?req=BopaAddOwnershipRecords&xuid=%I64d&platform=%d&titleId=%d
		requestData.sendSize = sprintf_s(requestData.sendBuffer, _countof(requestData.sendBuffer), 
			"POST /api_hamster.do?req=BopaAddOwnershipRecords&xuid=%I64d&platform=%d&titleId=%d HTTP/1.1\r\nHost: %s\r\nContent-Type: text/xml\r\nContent-Length: %d\r\n\r\n%s",
			playerXuid,
			platform,
			titleId,
			lspConnection.GetServerName(),
			strlen(xmlDoc),
			xmlDoc
			);
		}

		requestData.requestType = RequestTypeSetOwnershipRecord;

		requestID = lspConnection.Send(requestData.sendBuffer, requestData.sendSize, true);
	}

	void XlspManager::SetOwnershipRecordCompleted(bool succeeded)
	{
	#ifdef ENABLE_LSP
		if (succeeded)
		{
			log_line( Tag_XLSP, "XlspManager::SetOwnershipRecordCompleted() succeeded");
			setOwnershipStatus = LspStatusUpdated;
		}
		else
		{
			log_line( Tag_XLSP, "XlspManager::SetOwnershipRecordCompleted() failed");
			setOwnershipStatus = LspStatusFailure;
		}
	#endif
	}

	void XlspManager::ParseScoresIntoList(std::list<Score> &scores)
	{
		//parse scores from result string
		//we could convert the xml to a doc, but our data is short
		//simply use string parsing for now

		scores.clear();

		Score score;

		const char *pStr = requestData.pResultBuffer;
		while (pStr != NULL)
		{
			pStr = ParseForULongLongAttribute(pStr, "xuid=\"", score.xuid);
			if (pStr != NULL)
			{
				pStr = ParseForGamertagAttribute(pStr, "gamertag=\"", score.gamertag);
				if (pStr != NULL)
				{
					pStr = ParseForIntAttribute(pStr, "rank=\"", score.rank);
					if (pStr != NULL)
					{
						pStr = ParseForIntAttribute(pStr, "score=\"", score.score);
						if (pStr != NULL)
						{
							pStr = ParseForIntAttribute(pStr, "platform=\"", score.platform);
							if (pStr != NULL)
							{
								sigassert(score.xuid != INVALID_XUID);
								//record score
								//log_line(Tag_XLSP, "XlspManager::TopScoresCompleted() xuid: " << score.xuid << " gamertag: " << score.gamertag << " score: " << score.score << " rank: " << score.rank << " platform: " << score.platform );
								scores.push_back(score);
							}
						}
					}
				}
			}
		}
	}

	//return pointer to beginning of pAttribute
	const char *XlspManager::ParseForCharAttribute(const char * const pString, const char * const pAttribute, char &value)
	{
		const char *pTag = NULL;
	#ifdef ENABLE_LSP
		value = 0;
		pTag = strstr(pString, pAttribute);
		if (pTag)
		{
			sscanf_s(pTag + strlen(pAttribute), "%c", &value); 
		}
	#endif
		return pTag;
	}

	//return pointer to beginning of pAttribute
	const char *XlspManager::ParseForIntAttribute(const char * const pString, const char * const pAttribute, int &value)
	{
		const char *pTag = NULL;
	#ifdef ENABLE_LSP
		value = 0;
		pTag = strstr(pString, pAttribute);
		if (pTag)
		{
			sscanf_s(pTag + strlen(pAttribute), "%d", &value); 
		}
	#endif
		return pTag;
	}

	//return pointer to beginning of pAttribute
	const char *XlspManager::ParseForHexIntAttribute(const char * const pString, const char * const pAttribute, int &value)
	{
		const char *pTag = NULL;
	#ifdef ENABLE_LSP
		value = 0;
		pTag = strstr(pString, pAttribute);
		if (pTag)
		{
			sscanf_s(pTag + strlen(pAttribute), "%x", &value); 
		}
	#endif
		return pTag;
	}

	//return pointer to beginning of pAttribute
	const char *XlspManager::ParseForHexULongLongAttribute(const char * const pString, const char * const pAttribute, ULONGLONG &value)
	{
		const char *pTag = NULL;
	#ifdef ENABLE_LSP
		value = 0;
		pTag = strstr(pString, pAttribute);
		if (pTag)
		{
			sscanf_s(pTag + strlen(pAttribute), "%I64x", &value); 
		}
	#endif
		return pTag;
	}

	//return pointer to beginning of pAttribute
	const char *XlspManager::ParseForULongLongAttribute(const char * const pString, const char * const pAttribute, ULONGLONG &value)
	{
		const char *pTag = NULL;
	#ifdef ENABLE_LSP
		value = 0;
		pTag = strstr(pString, pAttribute);
		if (pTag)
		{
			sscanf_s(pTag + strlen(pAttribute), "%I64d", &value); 
		}
	#endif
		return pTag;
	}

	const char *XlspManager::ParseForGamertagAttribute(const char * const pString, const char * const pAttribute, char *gamertag)
	{
		const char *pTag = NULL;
	#ifdef ENABLE_LSP
		if (gamertag != NULL)
		{
			int count = 0;
			pTag = strstr(pString, pAttribute);
			if (pTag)
			{
				pTag += strlen(pAttribute);

				while (count < XUSER_MAX_NAME_LENGTH && *pTag != '\"' && *pTag != 0)
				{
					gamertag[count] = *pTag;
					pTag++;
					count++;
				}
			}
			gamertag[count] = 0;
		}
	#endif
		return pTag;
	}

	//return pointer to beginning of pAttribute
	const char *XlspManager::ParseForBooleanAttribute(const char * const pString, const char * const pAttribute, bool &value)
	{
		const char *pTag = NULL;
	#ifdef ENABLE_LSP
		value = 0;
		pTag = strstr(pString, pAttribute);
		if (pTag)
		{
			pTag += strlen(pAttribute);

			if (*pTag == 'T' || *pTag == 't')
			{
				value = true;
			}
			else
			{
				value = false;
			}

			while (*pTag != '\"' && *pTag != 0)
			{
				pTag++;
			}
		}
	#endif
		return pTag;
	}

	/// Helper to convert data into base64
	/// <returns>the length of the output data, in base 64 encoding, appropriate for encapsulation in a MIME part</returns>
	int XlspManager::EncodeBase64(char *inputBuffer, int inputBufferLength, char *outputBuffer, int outputBufferLength)
	{
		const char *encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		const int k_charsPerLine = 76;      // Per RFC, no more than 76 chars per line (19 groups)

		int outLength = 0;

		// precompute size
		int outputBytes = (inputBufferLength * 4 + 2) / 3; // 3 bytes translate into 4, rounded up
		outputBytes += 2 * (1 + (outputBytes + k_charsPerLine - 1) / k_charsPerLine); // plus space for newlines, rounded up
		outputBytes++; //add room for final 0

		//if result can fit in the output buffer, then start the conversion
		if (outputBytes <= outputBufferLength)
		{

			// quick part: turn groups of 3 into groups of 4
			int inLine = 0;
			int triple;
			for (int i = 0; i < inputBufferLength-2; i += 3)
			{
				// swizzle 3x8 bit objects into 4x6 bit objects
				//
				triple = (inputBuffer[i] << 16) | (inputBuffer[i + 1] << 8) | inputBuffer[i + 2];

				outputBuffer[outLength++] = encoding[(triple >> (3 * 6)) & 0x3f];
				outputBuffer[outLength++] = encoding[(triple >> (2 * 6)) & 0x3f];
				outputBuffer[outLength++] = encoding[(triple >> (1 * 6)) & 0x3f];
				outputBuffer[outLength++] = encoding[(triple >> (0 * 6)) & 0x3f];

				inLine += 4;
				if (inLine >= k_charsPerLine)
				{
					outputBuffer[outLength++]='\r';
					outputBuffer[outLength++]='\n';
					inLine = 0;
				}

			}

			// end case: if there are not a multiple of 3 octets
			//
			switch (inputBufferLength % 3)
			{
			case 0:
				break;

			case 1:
				// 1 extra octet - 2 encoded characters plus two ='s
				//
				triple = inputBuffer[inputBufferLength - 1] << 16;
				outputBuffer[outLength++]=encoding[(triple >> (3 * 6)) & 0x3f];
				outputBuffer[outLength++]=encoding[(triple >> (2 * 6)) & 0x3f];
				outputBuffer[outLength++]='=';
				outputBuffer[outLength++]='=';
				inLine += 4;
				break;

			case 2:
				// 2 extra octets - 3 encoded characters plus one =
				//
				triple = (inputBuffer[inputBufferLength - 2] << 16) | (inputBuffer[inputBufferLength - 1] << 8);
				outputBuffer[outLength++]=encoding[(triple >> (3 * 6)) & 0x3f];
				outputBuffer[outLength++]=encoding[(triple >> (2 * 6)) & 0x3f];
				outputBuffer[outLength++]=encoding[(triple >> (1 * 6)) & 0x3f];
				outputBuffer[outLength++]='=';
				inLine += 4;
				break;
			}

			// always end with a newline
			//
			if (inLine != 0)
			{
				outputBuffer[outLength++]='\r';
				outputBuffer[outLength++]='\n';
			}

			//add final 0
			outputBuffer[outLength] = 0;
		}
		else
		{
			//no room for conversion, tu see if we can null-terminate the string anyway, just for safety's sake
			if (outputBufferLength > 0)
			{
				outputBuffer[0] = 0;
			}	
		}

		return outLength;
	}

	void XlspManager::GuidToULongLongs(GUID guid, ULONGLONG &upper, ULONGLONG &lower)
	{
		upper = guid.Data1;
		upper <<= 16;
		upper |= guid.Data2;
		upper <<= 16;
		upper |= guid.Data3;

		memcpy(&lower, guid.Data4, 8);
	}

	void XlspManager::ULongLongsToGuid(GUID &guid, ULONGLONG upper, ULONGLONG lower)
	{
		guid.Data3 = upper & 0xffff;
		upper >>= 16;
		guid.Data2 = upper & 0xffff;
		upper >>= 16;
		guid.Data1 = upper & 0xffffffff;

		memcpy(guid.Data4, &lower, 8);
	}

	XlspManager::RequestData::RequestData()
	{
		Clear();
	}

	void XlspManager::RequestData::Clear()
	{
		requestType = RequestTypeUndefined;
		sendSize = 0;
		sendBuffer[0] = 0;
		pResultBuffer = NULL;
		resultSize = 0;
	}

	XlspManager g_XlspManager;

#endif
}

