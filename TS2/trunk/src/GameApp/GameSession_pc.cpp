//------------------------------------------------------------------------------
// \file GameSession_pc.cpp - 28 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "GameAppPch.hpp"
#include "GameSession.hpp"

namespace Sig { namespace GameSession
{
	const u32 cLeaderboardArcade = 0;
	const u32 cLeaderboardTotal[GameFlags::cDIFFICULTY_COUNT] = { 0 };
	const u32 cLeaderboardLevels[cCAMPAIGN_LEVEL_COUNT][GameFlags::cDIFFICULTY_COUNT] = { 0 };
	const u32 cLeaderboardH2H[cH2H_LEVEL_COUNT] = { 0 };
	const u32 cLeaderboardSurvival[cSURVIVAL_LEVEL_COUNT][GameFlags::cCHALLENGE_MODE_COUNT] = { 0 };
	const u32 cLeaderboardH2HTotals = 0;
	const u32 cLeaderboardTrialMiniGame[2] = { 0 };
	const u32 cLeaderboardMiniGame[cMINIGAME_LEVEL_COUNT] = { 0 };
	const u32 cLeaderboardPlayerStats = 0;
	const u32 cLeaderboardPlayerStats2 = 0;
	const u32 cLeaderboardPlayerStats2StartID = GameFlags::cSESSION_STATS_TIME_USING_TURRETS;
	const u32 cLeaderboardPlayerStatsColumnIdByStat[GameFlags::cSESSION_STATS_COUNT] = { 0 };


	const tLeaderBoardColumns cLevelLeaderBoardScoreColumnID[GameFlags::cMAP_TYPE_COUNT][cMAX_LEVEL_COUNT] = 
	{
		tLeaderBoardColumns( )
	};

	// Context Game Mode
	const u32 cContextGameModeCampaign		= 0;
	const u32 cContextGameModeSurvival		= 1;
	const u32 cContextGameModeMinigame		= 2;
	const u32 cContextGameModeVersus		= 3;
	const u32 cContextGameModeReplay		= 4;

	// Property
	const u32 cPropertyScore			= 0;
	const u32 cPropertyWins				= 1;
	const u32 cPropertyLosses			= 2;
	const u32 cPropertyWinLose			= 3;
	const u32 cPropertyMoney			= 4;
	const u32 cPropertyKills			= 5;
	const u32 cPropertyTime				= 5;
	const u32 cPropertyReplayHostId		= 6;
	const u32 cPropertyReplayClientId	= 7;
	const u32 cPopertyPlayerStats[GameFlags::cSESSION_STATS_COUNT] = { 0 };
	const u32 cPropertyMedal			= 0;
	const u32 cPropertyChallengeProgress = 0;

	// Match Query
	const u32 cMatchQueryStandard	= 0;
	const u32 cMatchQueryReplay		= 1;

	// Achievements
	u32 cAchievementIds[GameFlags::cACHIEVEMENTS_COUNT] = { 0 };	

	const u32 cAvatarAwardIds[GameFlags::cAVATAR_AWARDS_COUNT] = { ~0 };

	const u32 cGamerPicsIds[GameFlags::cAVATAR_AWARDS_COUNT] = { ~0 };	

	// Rich Presence
	namespace RichPresence
	{
		const u32 cContextSurvivalName = 0;
		const u32 cContextCampaignName1 = 1;
		const u32 cContextCampaignName2 = 2;
		const u32 cContextH2HName = 3;
		const u32 cContextMinigameName = 4;

		const u32 cContextCampaignName1Values[cCAMPAIGN_LEVEL_COUNT] =
		{ 
			0 
		};

		const u32 cContextCampaignName2Values[cCAMPAIGN_LEVEL_COUNT] =
		{
			0
		};

		const u32 cContextSurvivalNameValues[cSURVIVAL_LEVEL_COUNT] =
		{
			0
		};

		const u32 cContextMinigameNameValues[cMINIGAME_LEVEL_COUNT] =
		{
			0
		};

		const u32 cContextH2HNameValues[cH2H_LEVEL_COUNT] =
		{
			0
		};

		const u32 cRichPresenceMenu				= 0;
		const u32 cRichPresenceInactive			= 1;
		const u32 cRichPresenceNotParticipating = 2;
		const u32 cRichPresenceCampaignSP		= 3;
		const u32 cRichPresenceCampaignMP		= 4;
		const u32 cRichPresenceSurvivalSP		= 5;
		const u32 cRichPresenceSurvivalMP		= 6;
		const u32 cRichPresenceLobby			= 7;
		const u32 cRichPresenceHeadToHeadUSA	= 8;
		const u32 cRichPresenceHeadToHeadUSSR	= 9;
		const u32 cRichPresenceDemo				= 10;
		const u32 cRichPresenceMinigameSP		= 11;
		const u32 cRichPresenceMinigameMP		= 12;

	}
}}

