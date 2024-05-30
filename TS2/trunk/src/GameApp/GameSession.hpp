//------------------------------------------------------------------------------
// \file GameSession.hpp - 28 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __GameSession__
#define __GameSession__

namespace Sig { namespace GameSession
{
	#define cMAX_LEVEL_COUNT 20
	#define cCAMPAIGN_LEVEL_COUNT 17
	#define cH2H_LEVEL_COUNT 6
	#define cSURVIVAL_LEVEL_COUNT 5
	#define cMINIGAME_LEVEL_COUNT 8

	// Leaderboards
	extern const u32 cLeaderboardArcade;
	extern const u32 cLeaderboardLevels[cCAMPAIGN_LEVEL_COUNT][GameFlags::cDIFFICULTY_COUNT]; // [lvl][diff]
	extern const u32 cLeaderboardH2H[cH2H_LEVEL_COUNT]; // [lvl]
	extern const u32 cLeaderboardH2HTotals;
	extern const u32 cLeaderboardTrialMiniGame[2];
	extern const u32 cLeaderboardMiniGame[cMINIGAME_LEVEL_COUNT];
	extern const u32 cLeaderboardSurvival[cSURVIVAL_LEVEL_COUNT][GameFlags::cCHALLENGE_MODE_COUNT];
	extern const u32 cLeaderboardPlayerStats;
	extern const u32 cLeaderboardPlayerStats2;
	extern const u32 cLeaderboardPlayerStats2StartID;
	extern const u32 cLeaderboardPlayerStatsColumnIdByStat[GameFlags::cSESSION_STATS_COUNT];

	// Level boards
	struct tLeaderBoardColumns 
	{
		u32 mScore;
		u32 mMedal;
		u32 mChallenge;

		tLeaderBoardColumns( u32 score = ~0, u32 challenge = ~0, u32 medal = ~0 )
			: mScore( score ), mMedal( medal ), mChallenge( challenge )
		{ }
	};

	extern const tLeaderBoardColumns cLevelLeaderBoardScoreColumnID[GameFlags::cMAP_TYPE_COUNT][cMAX_LEVEL_COUNT]; //big ass number

	// Context Game Mode
	extern const u32 cContextGameModeCampaign;
	extern const u32 cContextGameModeSurvival;
	extern const u32 cContextGameModeMinigame;
	extern const u32 cContextGameModeVersus;
	extern const u32 cContextGameModeReplay;

	// Property
	extern const u32 cPropertyScore;
	extern const u32 cPropertyWins;
	extern const u32 cPropertyLosses;
	extern const u32 cPropertyWinLose;
	extern const u32 cPropertyMoney;
	extern const u32 cPropertyKills;
	extern const u32 cPropertyTime;
	extern const u32 cPropertyReplayHostId;
	extern const u32 cPropertyReplayClientId;
	extern const u32 cPropertyMedal;
	extern const u32 cPropertyChallengeProgress;
	extern const u32 cPopertyPlayerStats[GameFlags::cSESSION_STATS_COUNT];

	// Match Query
	extern const u32 cMatchQueryStandard;
	extern const u32 cMatchQueryReplay;

	// Achievements
	extern u32 cAchievementIds[GameFlags::cACHIEVEMENTS_COUNT];

	extern const u32 cAvatarAwardIds[GameFlags::cAVATAR_AWARDS_COUNT];
	extern const u32 cGamerPicsIds[GameFlags::cAVATAR_AWARDS_COUNT];

	// Rich Presence
	namespace RichPresence
	{
		extern const u32 cContextSurvivalName;
		extern const u32 cContextCampaignName1;
		extern const u32 cContextCampaignName2;
		extern const u32 cContextH2HName;
		extern const u32 cContextMinigameName;

		extern const u32 cContextCampaignName2Values[cCAMPAIGN_LEVEL_COUNT]; // 0 is trial
		extern const u32 cContextSurvivalNameValues[cSURVIVAL_LEVEL_COUNT];
		extern const u32 cContextMinigameNameValues[cMINIGAME_LEVEL_COUNT];
		extern const u32 cContextH2HNameValues[cH2H_LEVEL_COUNT];

		extern const u32 cRichPresenceMenu;
		extern const u32 cRichPresenceNotParticipating;
		extern const u32 cRichPresenceCampaignSP;
		extern const u32 cRichPresenceCampaignMP;
		extern const u32 cRichPresenceSurvivalSP;
		extern const u32 cRichPresenceSurvivalMP;
		extern const u32 cRichPresenceLobby;
		extern const u32 cRichPresenceHeadToHeadUSA;
		extern const u32 cRichPresenceHeadToHeadUSSR;
		extern const u32 cRichPresenceMinigameSP;
		extern const u32 cRichPresenceMinigameMP;

	}

}}

#endif//__GameSession__
