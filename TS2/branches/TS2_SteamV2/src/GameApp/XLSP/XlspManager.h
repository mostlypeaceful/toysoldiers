#pragma once

#include "stubs.h"
#include "LSPConnection.h"

namespace XLSP
{
#ifdef platform_xbox360
	class XlspManager
	{
	public:
		enum LspStatus
		{
			LspStatusUndefined = -1,
			LspStatusUpdating,
			LspStatusUpdated,
			LspStatusFailure,
		};

		class Score
		{
		public:
			XUID xuid;
			int score;
			int rank;
			int platform;
			char gamertag[XUSER_NAME_SIZE];
			Sig::tLocalizedString gamerTagW;

			// higher scores move to the front
			bool operator < ( const Score& right ) const { return score > right.score; }
		};

		class OwnershipRecord
		{
		public:
			int contentId;
			int contentType;
			int platform;
			char purchaseType;
		};

		XlspManager();

		static const int PlatformUnknown = 0;
		static const int PlatformXbox = 1;
		static const int PlatformPC = 2;
		static const int PlatformMobile = 3;

		//these methods start a request
		void RequestGetScoresAroundScore(XUID playerXuid, int machineId, int scoresAbove, int scoresBelow);
		void RequestGetScoresForXuids(XUID playerXuid, int machineId, int number, std::list<XUID> &xuids);
		void RequestPostScore(XUID playerXuid, int machineId, int score, int platform = PlatformXbox);
		void RequestTopScores(int machineId, int number);
		void RequestTopScoresPlatform(int machineId, int platform, int number);
		void RequestGetRankForScore(int machineId, int score, int platform, XUID playerXuid);
		void RequestGetOwnershipRecords(XUID playerXuid, int titleId, int platform);
		void RequestSetOwnershipRecord(XUID playerXuid, int titleId, int contentId, int platform);
		
		//these methods get the status and results of a request
		LspStatus GetScoresAroundScore(Sig::tGrowableArray<Score> &scores); //does not fill in platform or global rank
		LspStatus GetScoresForXuids(Sig::tGrowableArray<Score> &scores);
		LspStatus GetPostScoreStatus();
		LspStatus GetTopScores(Sig::tGrowableArray<Score> &scores); 
		LspStatus GetTopScoresPlatform(Sig::tGrowableArray<Score> &scores);
		LspStatus GetRankForScore(Score &score, bool &isNewHighScore); //does not fill in gamertag
		LspStatus GetOwnershipRecords(std::list<OwnershipRecord> &records);
		LspStatus GetSetOwnershipStatus();

		void Initialize();
		void Update(float elapsedSeconds);
		void Cleanup();

		bool fReady( ) const { return requests.size( ) == 0; }

	private:
		enum RequestType
		{
			RequestTypeUndefined = -1,
			RequestTypeGetScoresAroundScore,
			RequestTypeGetScoresForXuids,
			RequestTypePostScore,
			RequestTypeTopScores,
			RequestTypeTopScoresPlatform,
			RequestTypeGetRankForScore,
			RequestTypeGetOwnershipRecords,
			RequestTypeSetOwnershipRecord,

			//must be last
			RequestCount
		};

		struct Request
		{
			Request(RequestType requestType, XUID xuid, int machineId) : requestType(requestType), xuid(xuid), machineId(machineId), data1(0), data2(0) {}
			Request(RequestType requestType, int machineId) : requestType(requestType), xuid(INVALID_XUID), machineId(machineId), data1(0), data2(0) {}
			RequestType requestType;
			XUID xuid;
			int machineId;
			int data1;
			int data2;
			std::list<XUID> xuids;
		};

		class RequestData
		{
		public:
			RequestType requestType;
			unsigned int sendSize;
			char sendBuffer[LSPConnection::bufferSize];
			char *pResultBuffer;
			unsigned int resultSize;

			RequestData();
			void Clear();
		};

		std::deque<Request> requests;
		unsigned int requestID;
		RequestData requestData;
		LSPConnection lspConnection;

		LspStatus scoresAroundScoreStatus;
		std::list<Score> scoresAroundScore;

		LspStatus scoresForXuidsStatus;
		std::list<Score> scoresForXuids;

		LspStatus postScoreStatus;

		LspStatus topScoresStatus;
		std::list<Score> topScores;

		LspStatus topScoresPlatformStatus;
		std::list<Score> topScoresPlatform;

		LspStatus rankForScoreStatus;
		Score rankForScore;
		bool rankForScoreIsNewHighScore;

		std::list<OwnershipRecord> mobileOwnershipRecords;
		LspStatus mobileOwnershipRecordsStatus;

		LspStatus setOwnershipStatus;

		void UpdateRequest();
		void SendNextRequest();

		void SendGetScoresAroundScore(XUID playerXuid, int machineId, int scoresAbove, int scoresBelow);
		void SendGetScoresForXuids(XUID playerXuid, int machineId, int number, std::list<XUID> &xuids);
		void SendPostScore(XUID playerXuid, int machineId, int score, int platform);
		void SendTopScores(int machineId, int number);
		void SendTopScoresPlatform(int machineId, int platform, int number);
		void SendGetRankForScore(int machineId, int score, int platform, XUID playerXuid);
		void SendGetOwnershipRecords(XUID playerXuid, int titleId, int platform);
		void SendSetOwnershipRecord(XUID playerXuid, int titleId, int contentId, int platform);

		void GetScoresAroundScoreCompleted(bool succeeded);
		void GetScoresForXuidsCompleted(bool succeeded);
		void PostScoreCompleted(bool succeeded);
		void TopScoresCompleted(bool succeeded);
		void TopScoresPlatformCompleted(bool succeeded);
		void GetRankForScoreCompleted(bool succeeded);
		void GetOwnershipRecordsCompleted(bool succeeded);
		void SetOwnershipRecordCompleted(bool succeeded);

		void ParseScoresIntoList(std::list<Score> &scores);

		const char *ParseForCharAttribute(const char * const pString, const char * const pAttribute, char &value);
		const char *ParseForIntAttribute(const char * const pString, const char * const pAttribute, int &value);
		const char *ParseForHexIntAttribute(const char * const pString, const char * const pAttribute, int &value);
		const char *ParseForHexULongLongAttribute(const char * const pString, const char * const pAttribute, ULONGLONG &value);
		const char *ParseForULongLongAttribute(const char * const pString, const char * const pAttribute, ULONGLONG &value);
		const char *ParseForGamertagAttribute(const char * const pString, const char * const pAttribute, char *gamertag);
		const char *ParseForBooleanAttribute(const char * const pString, const char * const pAttribute, bool &value);
		void GuidToULongLongs(GUID guid, ULONGLONG &upper, ULONGLONG &lower);
		void ULongLongsToGuid(GUID &guid, ULONGLONG upper, ULONGLONG lower);
		int EncodeBase64(char *inputBuffer, int inputBufferLength, char *outputBuffer, int outputBufferLength);
	};

	extern XlspManager g_XlspManager;

#endif
}