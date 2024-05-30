//------------------------------------------------------------------------------
// \file GameUser_xbox360.cpp - 28 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "GameAppPch.hpp"
#include "GameSession.hpp"
#include "../../XBLA/FullGame/LiveFiles/ts2.spa.h"

namespace Sig { namespace GameSession
{
	const u32 cLeaderboardArcade = STATS_VIEW_ARCADE_LEADERBOARD;
	const u32 cLeaderboardLevels[cCAMPAIGN_LEVEL_COUNT][GameFlags::cDIFFICULTY_COUNT] = 
	{
		{ 
			STATS_VIEW_LEVEL0_CASUAL,
			STATS_VIEW_LEVEL0_NORMAL,
			STATS_VIEW_LEVEL0_HARD,
			STATS_VIEW_LEVEL0_ELITE,
			STATS_VIEW_LEVEL0_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL1_CASUAL,
			STATS_VIEW_LEVEL1_NORMAL,
			STATS_VIEW_LEVEL1_HARD,
			STATS_VIEW_LEVEL1_ELITE,
			STATS_VIEW_LEVEL1_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL2_CASUAL,
			STATS_VIEW_LEVEL2_NORMAL,
			STATS_VIEW_LEVEL2_HARD,
			STATS_VIEW_LEVEL2_ELITE,
			STATS_VIEW_LEVEL2_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL3_CASUAL,
			STATS_VIEW_LEVEL3_NORMAL,
			STATS_VIEW_LEVEL3_HARD,
			STATS_VIEW_LEVEL3_ELITE,
			STATS_VIEW_LEVEL3_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL4_CASUAL,
			STATS_VIEW_LEVEL4_NORMAL,
			STATS_VIEW_LEVEL4_HARD,
			STATS_VIEW_LEVEL4_ELITE,
			STATS_VIEW_LEVEL4_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL5_CASUAL,
			STATS_VIEW_LEVEL5_NORMAL,
			STATS_VIEW_LEVEL5_HARD,
			STATS_VIEW_LEVEL5_ELITE,
			STATS_VIEW_LEVEL5_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL6_CASUAL,
			STATS_VIEW_LEVEL6_NORMAL,
			STATS_VIEW_LEVEL6_HARD,
			STATS_VIEW_LEVEL6_ELITE,
			STATS_VIEW_LEVEL6_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL7_CASUAL,
			STATS_VIEW_LEVEL7_NORMAL,
			STATS_VIEW_LEVEL7_HARD,
			STATS_VIEW_LEVEL7_ELITE,
			STATS_VIEW_LEVEL7_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL8_CASUAL,
			STATS_VIEW_LEVEL8_NORMAL,
			STATS_VIEW_LEVEL8_HARD,
			STATS_VIEW_LEVEL8_ELITE,
			STATS_VIEW_LEVEL8_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL9_CASUAL,
			STATS_VIEW_LEVEL9_NORMAL,
			STATS_VIEW_LEVEL9_HARD,
			STATS_VIEW_LEVEL9_ELITE,
			STATS_VIEW_LEVEL9_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL10_CASUAL,
			STATS_VIEW_LEVEL10_NORMAL,
			STATS_VIEW_LEVEL10_HARD,
			STATS_VIEW_LEVEL10_ELITE,
			STATS_VIEW_LEVEL10_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL11_CASUAL,
			STATS_VIEW_LEVEL11_NORMAL,
			STATS_VIEW_LEVEL11_HARD,
			STATS_VIEW_LEVEL11_ELITE,
			STATS_VIEW_LEVEL11_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL12_CASUAL,
			STATS_VIEW_LEVEL12_NORMAL,
			STATS_VIEW_LEVEL12_HARD,
			STATS_VIEW_LEVEL12_ELITE,
			STATS_VIEW_LEVEL12_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL13_CASUAL,
			STATS_VIEW_LEVEL13_NORMAL,
			STATS_VIEW_LEVEL13_HARD,
			STATS_VIEW_LEVEL13_ELITE,
			STATS_VIEW_LEVEL13_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL14_CASUAL,
			STATS_VIEW_LEVEL14_NORMAL,
			STATS_VIEW_LEVEL14_HARD,
			STATS_VIEW_LEVEL14_ELITE,
			STATS_VIEW_LEVEL14_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL15_CASUAL,
			STATS_VIEW_LEVEL15_NORMAL,
			STATS_VIEW_LEVEL15_HARD,
			STATS_VIEW_LEVEL15_ELITE,
			STATS_VIEW_LEVEL15_GENERAL
		},
		{ 
			STATS_VIEW_LEVEL16_CASUAL,
			STATS_VIEW_LEVEL16_NORMAL,
			STATS_VIEW_LEVEL16_HARD,
			STATS_VIEW_LEVEL16_ELITE,
			STATS_VIEW_LEVEL16_GENERAL
		}
	};

	const u32 cLeaderboardH2H[cH2H_LEVEL_COUNT] = 
	{
		STATS_VIEW_H2H_LEVEL1,
		STATS_VIEW_H2H_LEVEL2,
		STATS_VIEW_H2H_LEVEL3,
		STATS_VIEW_H2H_LEVEL4,
		STATS_VIEW_H2H_LEVEL5,
	};

	const u32 cLeaderBoardMiniGameScoreColumnID = STATS_COLUMN_MINIGAME_AC130_SCORE; // THIS MAY NOT BE SAFE FOR all minigames :/

	const u32 cLeaderboardH2HTotals = STATS_VIEW_H2H_TOTALS;
	const u32 cLeaderboardTrialMiniGame[2] = 
	{
		STATS_VIEW_TRIAL_MINIGAME1,
		STATS_VIEW_TRIAL_MINIGAME2,
	};

	const u32 cLeaderboardMiniGame[cMINIGAME_LEVEL_COUNT] = 
	{
		STATS_VIEW_TRIAL_MINIGAME1,
		STATS_VIEW_TRIAL_MINIGAME2,
		STATS_VIEW_MINIGAME_DX,
		STATS_VIEW_MINIGAME_AC130,
		STATS_VIEW_MINIGAME_HALLWAY,
		STATS_VIEW_MINIGAME_FLY,
		STATS_VIEW_MINIGAME_F14,
		STATS_VIEW_MINIGAME_ATV,
	};

	const u32 cLeaderboardSurvival[cSURVIVAL_LEVEL_COUNT][GameFlags::cCHALLENGE_MODE_COUNT] =
	{
		{ 
			STATS_VIEW_SURVIVAL0_SURVIVAL,
			STATS_VIEW_SURVIVAL0_LOCKDOWN,
			STATS_VIEW_SURVIVAL0_HARDCORE,
			STATS_VIEW_SURVIVAL0_TRAUMA,
			STATS_VIEW_SURVIVAL0_HARDLOCK,
		},
		{ 
			STATS_VIEW_SURVIVAL1_SURVIVAL,
			STATS_VIEW_SURVIVAL1_LOCKDOWN,
			STATS_VIEW_SURVIVAL1_HARDCORE,
			STATS_VIEW_SURVIVAL1_TRAUMA,
			STATS_VIEW_SURVIVAL1_HARDLOCK,
		},
		{ 
			STATS_VIEW_SURVIVAL2_SURVIVAL,
			STATS_VIEW_SURVIVAL2_LOCKDOWN,
			STATS_VIEW_SURVIVAL2_HARDCORE,
			STATS_VIEW_SURVIVAL2_TRAUMA,
			STATS_VIEW_SURVIVAL2_HARDLOCK,
		},
		{ 
			STATS_VIEW_SURVIVAL3_SURVIVAL,
			STATS_VIEW_SURVIVAL3_LOCKDOWN,
			STATS_VIEW_SURVIVAL3_HARDCORE,
			STATS_VIEW_SURVIVAL3_TRAUMA,
			STATS_VIEW_SURVIVAL3_HARDLOCK,
		},
		{ 
			STATS_VIEW_SURVIVAL4_SURVIVAL,
			STATS_VIEW_SURVIVAL4_LOCKDOWN,
			STATS_VIEW_SURVIVAL4_HARDCORE,
			STATS_VIEW_SURVIVAL4_TRAUMA,
			STATS_VIEW_SURVIVAL4_HARDLOCK,
		},
	};

	const u32 cLeaderboardPlayerStats = STATS_VIEW_PLAYER_STATS;
	const u32 cLeaderboardPlayerStats2 = STATS_VIEW_PLAYER_STATS2;
	const u32 cLeaderboardPlayerStats2StartID = GameFlags::cSESSION_STATS_TIME_USING_TURRETS;

	const u32 cLeaderboardPlayerStatsColumnIdByStat[GameFlags::cSESSION_STATS_COUNT] = 
	{
		STATS_COLUMN_PLAYER_STATS_SCORE,
		STATS_COLUMN_PLAYER_STATS_ENEMIES_REACHED_GOAL,
		STATS_COLUMN_PLAYER_STATS_INFANTRY_REACHED_GOAL,
		STATS_COLUMN_PLAYER_STATS_TOTAL_MONEY,
		STATS_COLUMN_PLAYER_STATS_MONEY_EARNED,
		STATS_COLUMN_PLAYER_STATS_UNITS_PURCHASED,
		STATS_COLUMN_PLAYER_STATS_UNITS_UPGRADED,
		STATS_COLUMN_PLAYER_STATS_UNITS_REPAIRED,
		STATS_COLUMN_PLAYER_STATS_KILLS,
		STATS_COLUMN_PLAYER_STATS_HIGHEST_COMBO,
		STATS_COLUMN_PLAYER_STATS_AMMO_EXPENDED,
		STATS_COLUMN_PLAYER_STATS_BARRAGES_CALLED_IN,
		STATS_COLUMN_PLAYER_STATS_KILLS_WITH_BARRAGES,
		STATS_COLUMN_PLAYER_STATS_TOTAL_TIME,
		STATS_COLUMN_PLAYER_STATS_TIME_IN_UNITS,
		STATS_COLUMN_PLAYER_STATS_TIME_IN_VEHICLES,
		STATS_COLUMN_PLAYER_STATS_TIME_IN_SHELLCAM,
		STATS_COLUMN_PLAYER_STATS_TRIES_BEFORE_COMPLETION,
		STATS_COLUMN_PLAYER_STATS_KILLS_IN_NIGHT_VISION,
		STATS_COLUMN_PLAYER_STATS_HIGHEST_FLYING_SOLDIER,
		STATS_COLUMN_PLAYER_STATS_SECONDS_SKIPPED,
		STATS_COLUMN_PLAYER_STATS_OVERKILL,
		STATS_COLUMN_PLAYER_STATS_MONEY_SPENT,
		STATS_COLUMN_PLAYER_STATS_DEFENSE_SUB_SCORE,
		STATS_COLUMN_PLAYER_STATS_TIME_SUB_SCORE,
		STATS_COLUMN_PLAYER_STATS_MONEY_SUB_SCORE,
		STATS_COLUMN_PLAYER_STATS_MINIGAME_META_STAT,
		STATS_COLUMN_PLAYER_STATS_ASSISTS,
		STATS_COLUMN_PLAYER_STATS_OVERCHARGE,
		STATS_COLUMN_PLAYER_STATS_WAVE_BONUS,
		STATS_COLUMN_PLAYER_STATS_WAVE_CHAIN,
		STATS_COLUMN_PLAYER_STATS_PAYBACK,
		STATS_COLUMN_PLAYER_STATS_CLOSE_CALL,
		STATS_COLUMN_PLAYER_STATS_BOMBING_RUN,
		STATS_COLUMN_PLAYER_STATS_SPEED_BONUS,
		STATS_COLUMN_PLAYER_STATS_KILLS_WHILE_USING_TURRETS,
		STATS_COLUMN_PLAYER_STATS_BASIC_INFANTRY_KILLED,
		STATS_COLUMN_PLAYER_STATS_ELITE_INFANTRY_KILLED,
		STATS_COLUMN_PLAYER_STATS_ATVS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_CARS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_APCS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_IFVS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_MEDIUM_TANKS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_HEAVY_TANKS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_TRANSPORT_COPTERS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_GUNSHIPS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_ATTACK_COPTERS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_FIGHTER_PLANES_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_TRANSPORT_PLANES_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_BOMBERS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS_TANKS_REACHED_TOY_BOX,
		STATS_COLUMN_PLAYER_STATS_APCS_REACHED_TOY_BOX,
		STATS_COLUMN_PLAYER_STATS_PLANES_REACHED_TOY_BOX,
		STATS_COLUMN_PLAYER_STATS_HELICOPTERS_REACHED_TOY_BOX,
		STATS_COLUMN_PLAYER_STATS_CARS_REACHED_TOY_BOX,
		STATS_COLUMN_PLAYER_STATS_ATVS_REACHED_TOY_BOX,
		STATS_COLUMN_PLAYER_STATS_TURRETS_LOST,
		STATS_COLUMN_PLAYER_STATS_VEHICLES_PURCHASED,
		STATS_COLUMN_PLAYER_STATS_VEHICLES_LOST,
		STATS_COLUMN_PLAYER_STATS_VEHICLES_DRIVEN_INTO_GOAL,
		STATS_COLUMN_PLAYER_STATS_VERSUS_WAVES_LAUNCHED,

		// this where the table splits
		STATS_COLUMN_PLAYER_STATS2_TIME_USING_TURRETS,
		STATS_COLUMN_PLAYER_STATS2_TIME_USING_AC130,
		STATS_COLUMN_PLAYER_STATS2_TIME_USING_COMMANDO,
		STATS_COLUMN_PLAYER_STATS2_TIME_USING_IVAN,
		STATS_COLUMN_PLAYER_STATS2_TIME_USING_MEDIUM_TANK,
		STATS_COLUMN_PLAYER_STATS2_TIME_USING_HEAVY_TANK,
		STATS_COLUMN_PLAYER_STATS2_TIME_USING_ATTACK_HELICOPTER,
		STATS_COLUMN_PLAYER_STATS2_TIME_USING_HELICOPTER_GUNSHIP,
		STATS_COLUMN_PLAYER_STATS2_TIME_USING_FIGHTER_PLANE,
		STATS_COLUMN_PLAYER_STATS2_MOST_KILLS_WITH_A_NUKE,
		STATS_COLUMN_PLAYER_STATS2_TIME_SPENT_IN_TURBOCHARGE,
		STATS_COLUMN_PLAYER_STATS2_TURBOCHARGE_KILLS,
		STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_MACHINEGUNS,
		STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_HOWITZERS,
		STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_MORTARS,
		STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_ANTI_TANK,
		STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_MAKESHIFT,
		STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_ANTI_AIR,
		STATS_COLUMN_PLAYER_STATS2_MOST_HOTSWAPS_IN_A_SINGLE_TURBOCHARGE,
		STATS_COLUMN_PLAYER_STATS2_TICKETS_LOST_IN_HEAD_2_HEAD,
		STATS_COLUMN_PLAYER_STATS2_TICKETS_DEALT_IN_HEAD_2_HEAD,
		STATS_COLUMN_PLAYER_STATS2_VERSUS_WINS,
		STATS_COLUMN_PLAYER_STATS2_VERSUS_LOSSES,
		STATS_COLUMN_PLAYER_STATS2_VERSUS_WIN_LOSS_RATIO,
		STATS_COLUMN_PLAYER_STATS2_VERSUS_SHUT_OUTS,
		STATS_COLUMN_PLAYER_STATS2_VERSUS_TURRETS_DESTROYED,
		STATS_COLUMN_PLAYER_STATS2_VERSUS_PLATFORMS_CAPTURED,
		STATS_COLUMN_PLAYER_STATS2_DECORATIONS_ACQUIRED,
		STATS_COLUMN_PLAYER_STATS2_TOTAL_BRONZE_MEDALS,
		STATS_COLUMN_PLAYER_STATS2_TOTAL_SILVER_MEDALS,
		STATS_COLUMN_PLAYER_STATS2_TOTAL_GOLD_MEDALS,
		STATS_COLUMN_PLAYER_STATS2_TOTAL_PLATINUM_MEDALS,
		STATS_COLUMN_PLAYER_STATS2_TOTAL_CHALLENGES_EARNED,
		STATS_COLUMN_PLAYER_STATS2_5X_COMBOS,
		STATS_COLUMN_PLAYER_STATS2_10X_COMBOS,
		STATS_COLUMN_PLAYER_STATS2_20X_COMBOS,
		STATS_COLUMN_PLAYER_STATS2_40X_COMBOS,
		STATS_COLUMN_PLAYER_STATS2_80X_COMBOS,
		STATS_COLUMN_PLAYER_STATS2_100X_COMBOS,
		STATS_COLUMN_PLAYER_STATS2_150X_COMBOS,
		STATS_COLUMN_PLAYER_STATS2_200X_COMBOS,
		STATS_COLUMN_PLAYER_STATS2_250X_COMBOS,
		STATS_COLUMN_PLAYER_STATS2_300X_COMBOS,
		STATS_COLUMN_PLAYER_STATS2_BULLETS_FIRED,
		STATS_COLUMN_PLAYER_STATS2_MORTARS_SHELLS_FIRED,
		STATS_COLUMN_PLAYER_STATS2_HOWITZER_SHELLS_FIRED,
		STATS_COLUMN_PLAYER_STATS2_MISSILES_FIRED,
		STATS_COLUMN_PLAYER_STATS2_BARRAGES_EARNED_FROM_RED_STARS,
		STATS_COLUMN_PLAYER_STATS2_BARRAGES_EARNED_FROM_TURBOCHARGE,
		STATS_COLUMN_PLAYER_STATS2_BARRAGES_PURCHASED_IN_VERSUS,
		STATS_COLUMN_PLAYER_STATS2_INFANTRY_GASSED,
		STATS_COLUMN_PLAYER_STATS2_INFANTRY_SET_ON_FIRE,
		STATS_COLUMN_PLAYER_STATS2_HIGHEST_WITH_A_SINGLE_BARRAGE,
		STATS_COLUMN_PLAYER_STATS2_MOST_BARRAGES_IN_A_SINGLE_MAP,
		STATS_COLUMN_PLAYER_STATS2_HIGHEST_SURVIVAL_ROUND,
		STATS_COLUMN_PLAYER_STATS2_HIGHEST_LOCKDOWN_ROUND,
		STATS_COLUMN_PLAYER_STATS2_HIGHEST_HARDCORE_ROUND,
		STATS_COLUMN_PLAYER_STATS2_GOLDEN_FLIES_EXPLODED,
		STATS_COLUMN_PLAYER_STATS2_TURRETS_RESCUED,
		~0, //NO LB
		~0,
	};

	const tLeaderBoardColumns cLevelLeaderBoardScoreColumnID[GameFlags::cMAP_TYPE_COUNT][cMAX_LEVEL_COUNT] = 
	{
		{ tLeaderBoardColumns( ) }, // front end, none
		{ 
			tLeaderBoardColumns( STATS_COLUMN_LEVEL0_CASUAL_SCORE, STATS_COLUMN_LEVEL0_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL0_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL1_CASUAL_SCORE, STATS_COLUMN_LEVEL1_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL1_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL2_CASUAL_SCORE, STATS_COLUMN_LEVEL2_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL2_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL3_CASUAL_SCORE, STATS_COLUMN_LEVEL3_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL3_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL4_CASUAL_SCORE, STATS_COLUMN_LEVEL4_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL4_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL5_CASUAL_SCORE, STATS_COLUMN_LEVEL5_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL5_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL6_CASUAL_SCORE, STATS_COLUMN_LEVEL6_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL6_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL7_CASUAL_SCORE, STATS_COLUMN_LEVEL7_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL7_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL8_CASUAL_SCORE, STATS_COLUMN_LEVEL8_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL8_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL9_CASUAL_SCORE, STATS_COLUMN_LEVEL9_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL9_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL10_CASUAL_SCORE, STATS_COLUMN_LEVEL10_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL10_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL11_CASUAL_SCORE, STATS_COLUMN_LEVEL11_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL11_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL12_CASUAL_SCORE, STATS_COLUMN_LEVEL12_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL12_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL13_CASUAL_SCORE, STATS_COLUMN_LEVEL13_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL13_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL14_CASUAL_SCORE, STATS_COLUMN_LEVEL14_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL14_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL15_CASUAL_SCORE, STATS_COLUMN_LEVEL15_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL15_CASUAL_MEDAL ),
			tLeaderBoardColumns( STATS_COLUMN_LEVEL16_CASUAL_SCORE, STATS_COLUMN_LEVEL16_CASUAL_CHALLENGE_PROGRESS, STATS_COLUMN_LEVEL16_CASUAL_MEDAL ),
			tLeaderBoardColumns( )
		},
		{ tLeaderBoardColumns( ) }, // head to head, none
		{
			tLeaderBoardColumns( STATS_COLUMN_SURVIVAL0_SURVIVAL_SCORE, STATS_COLUMN_SURVIVAL0_SURVIVAL_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_SURVIVAL1_SURVIVAL_SCORE, STATS_COLUMN_SURVIVAL1_SURVIVAL_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_SURVIVAL2_SURVIVAL_SCORE, STATS_COLUMN_SURVIVAL2_SURVIVAL_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_SURVIVAL3_SURVIVAL_SCORE, STATS_COLUMN_SURVIVAL3_SURVIVAL_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_SURVIVAL4_SURVIVAL_SCORE, STATS_COLUMN_SURVIVAL4_SURVIVAL_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( )
		},
		{
			tLeaderBoardColumns( STATS_COLUMN_TRIAL_MINIGAME1_SCORE, STATS_COLUMN_TRIAL_MINIGAME1_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_TRIAL_MINIGAME2_SCORE, STATS_COLUMN_TRIAL_MINIGAME1_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_MINIGAME_DX_SCORE, STATS_COLUMN_MINIGAME_DX_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_MINIGAME_AC130_SCORE, STATS_COLUMN_MINIGAME_AC130_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_MINIGAME_HALLWAY_SCORE, STATS_COLUMN_MINIGAME_HALLWAY_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_MINIGAME_FLY_SCORE, STATS_COLUMN_MINIGAME_FLY_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_MINIGAME_F14_SCORE, STATS_COLUMN_MINIGAME_F14_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( STATS_COLUMN_MINIGAME_ATV_SCORE, STATS_COLUMN_MINIGAME_ATV_CHALLENGE_PROGRESS ),
			tLeaderBoardColumns( )
		},
		{ tLeaderBoardColumns( ) }, // dev, none
	};

	// Context Game Mode
	const u32 cContextGameModeCampaign		= CONTEXT_GAME_MODE_CAMPAIGN;
	const u32 cContextGameModeSurvival		= CONTEXT_GAME_MODE_SURVIAL;
	const u32 cContextGameModeMinigame		= CONTEXT_GAME_MODE_MINIGAME;
	const u32 cContextGameModeVersus		= CONTEXT_GAME_MODE_VERSUS;
	const u32 cContextGameModeReplay		= CONTEXT_GAME_MODE_REPLAY;

	// Property
	const u32 cPropertyScore			= PROPERTY_SCORE;
	const u32 cPropertyWins				= PROPERTY_WINS;
	const u32 cPropertyLosses			= PROPERTY_LOSSES;
	const u32 cPropertyWinLose			= PROPERTY_WINLOSE;
	const u32 cPropertyMoney			= PROPERTY_MONEY;
	const u32 cPropertyKills			= PROPERTY_KILLS;
	const u32 cPropertyTime				= PROPERTY_TIME;
	const u32 cPropertyReplayHostId		= PROPERTY_REPLAYHOSTID;
	const u32 cPropertyReplayClientId	= PROPERTY_REPLAYCLIENTID;
	const u32 cPopertyPlayerStats[GameFlags::cSESSION_STATS_COUNT] = 
	{
		PROPERTY_STATS_SCORE,
		PROPERTY_STATS_ENEMIES_REACHED_GOAL,
		PROPERTY_STATS_INFANTRY_REACHED_GOAL,
		PROPERTY_STATS_TOTAL_MONEY,
		PROPERTY_STATS_MONEY_EARNED,
		PROPERTY_STATS_UNITS_PURCHASED,
		PROPERTY_STATS_UNITS_UPGRADED,
		PROPERTY_STATS_UNITS_REPAIRED,
		PROPERTY_STATS_KILLS,
		PROPERTY_STATS_HIGHEST_COMBO,
		PROPERTY_STATS_AMMO_EXPENDED,
		PROPERTY_STATS_BARRAGES_CALLED_IN,
		PROPERTY_STATS_KILLS_WITH_BARRAGES,
		PROPERTY_STATS_TOTAL_TIME,
		PROPERTY_STATS_TIME_IN_UNITS,
		PROPERTY_STATS_TIME_IN_VEHICLES,
		PROPERTY_STATS_TIME_IN_SHELLCAM,
		PROPERTY_STATS_TRIES_BEFORE_COMPLETION,
		PROPERTY_STATS_KILLS_IN_NIGHT_VISION,
		PROPERTY_STATS_HIGHEST_FLYING_SOLDIER,
		PROPERTY_STATS_SECONDS_SKIPPED,
		PROPERTY_STATS_OVER_KILL,
		PROPERTY_STATS_MONEY_SPENT,
		PROPERTY_STATS_DEFENSE_SUB_SCORE,
		PROPERTY_STATS_TIME_SUB_SCORE,
		PROPERTY_STATS_MONEY_SUB_SCORE,
		PROPERTY_STATS_MINIGAME_META_STAT,
		PROPERTY_STATS_ASSISTS,
		PROPERTY_STATS_OVERCHARGE,
		PROPERTY_STATS_WAVE_BONUS,
		PROPERTY_STATS_WAVE_CHAIN,
		PROPERTY_STATS_PAYBACK,
		PROPERTY_STATS_CLOSE_CALL,
		PROPERTY_STATS_BOMBING_RUN,
		PROPERTY_STATS_SPEED_BONUS,
		PROPERTY_STATS_KILLS_WHILE_USING_TURRETS,
		PROPERTY_STATS_BASIC_INFANTRY_KILLED,
		PROPERTY_STATS_ELITE_INFANTRY_KILLED,
		PROPERTY_STATS_ATVS_DESTROYED,
		PROPERTY_STATS_CARS_DESTROYED,
		PROPERTY_STATS_APCS_DESTROYED,
		PROPERTY_STATS_IFVS_DESTROYED,
		PROPERTY_STATS_MEDIUM_TANKS_DESTROYED,
		PROPERTY_STATS_HEAVY_TANKS_DESTROYED,
		PROPERTY_STATS_TRANSPORT_COPTERS_DESTROYED,
		PROPERTY_STATS_GUNSHIPS_DESTROYED,
		PROPERTY_STATS_ATTACK_COPTERS_DESTROYED,
		PROPERTY_STATS_FIGHTER_PLANES_DESTROYED,
		PROPERTY_STATS_TRANSPORT_PLANES_DESTROYED,
		PROPERTY_STATS_BOMBERS_DESTROYED,
		PROPERTY_STATS_TANKS_REACHED_TOY_BOX,
		PROPERTY_STATS_APCS_REACHES_TOY_BOX,
		PROPERTY_STATS_PLANES_REACHED_TOY_BOX,
		PROPERTY_STATS_HELICOPTERS_REACHED_TOY_BOX,
		PROPERTY_STATS_CARS_REACHED_TOY_BOX,
		PROPERTY_STATS_ATVS_REACHED_TOY_BOX,
		PROPERTY_STATS_TURRETS_LOST,
		PROPERTY_STATS_VEHICLES_PURCHASED,
		PROPERTY_STATS_VEHICLES_LOST,
		PROPERTY_STATS_VEHICLES_DRIVEN_INTO_GOAL,
		PROPERTY_STATS_VERSUS_WAVES_LAUNCHED,

		// this is where the ble splits
		PROPERTY_STATS_TIME_USING_TURRETS,
		PROPERTY_STATS_TIME_USING_AC130,
		PROPERTY_STATS_TIME_USING_COMMANDO,
		PROPERTY_STATS_TIME_USING_IVAN,
		PROPERTY_STATS_TIME_USING_MEDIUM_TANK,
		PROPERTY_STATS_TIME_USING_HEAVY_TANK,
		PROPERTY_STATS_TIME_USING_ATTACK_HELICOPTER,
		PROPERTY_STATS_TIME_USING_HELICOPTER_GUNSHIP,
		PROPERTY_STATS_TIME_USING_FIGHTER_PLANE,
		PROPERTY_STATS_MOST_KILLS_WITH_A_NUKE,
		PROPERTY_STATS_TIME_SPENT_IN_TURBOCHARGE,
		PROPERTY_STATS_TURBOCHARGE_KILLS,
		PROPERTY_STATS_TURBOCHARGES_WITH_MACHINEGUNS,
		PROPERTY_STATS_TURBOCHARGES_WITH_HOWITZERS,
		PROPERTY_STATS_TURBOCHARGES_WITH_MORTARS,
		PROPERTY_STATS_TURBOCHARGES_WITH_ANTI_TANK,
		PROPERTY_STATS_TURBOCHARGES_WITH_MAKESHIFT,
		PROPERTY_STATS_TURBOCHARGES_WITH_ANTI_AIR,
		PROPERTY_STATS_MOST_HOTSWAPS_IN_A_SINGLE_TURBOCHARGE,
		PROPERTY_STATS_TICKETS_LOST_IN_HEAD_2_HEAD,
		PROPERTY_STATS_TICKETS_DEALT_IN_HEAD_2_HEAD,
		PROPERTY_STATS_VERSUS_WINS,
		PROPERTY_STATS_VERSUS_LOSSES,
		PROPERTY_STATS_VERSUS_WIN_LOSS_RATIO,
		PROPERTY_STATS_VERSUS_SHUT_OUTS,
		PROPERTY_STATS_VERSUS_TURRETS_DESTROYED,
		PROPERTY_STATS_VERSUS_PLATFORMS_CAPTURED,
		PROPERTY_STATS_DECORATIONS_ACQUIRED,
		PROPERTY_STATS_TOTAL_BRONZE_MEDALS,
		PROPERTY_STATS_TOTAL_SILVER_MEDALS,
		PROPERTY_STATS_TOTAL_GOLD_MEDALS,
		PROPERTY_STATS_TOTAL_PLATINUM_MEDALS,
		PROPERTY_STATS_TOTAL_CHALLENGES_EARNED,
		PROPERTY_STATS_5X_COMBOS,
		PROPERTY_STATS_10X_COMBOS,
		PROPERTY_STATS_20X_COMBOS,
		PROPERTY_STATS_40X_COMBOS,
		PROPERTY_STATS_80X_COMBOS,
		PROPERTY_STATS_100X_COMBOS,
		PROPERTY_STATS_150X_COMBOS,
		PROPERTY_STATS_200X_COMBOS,
		PROPERTY_STATS_250X_COMBOS,
		PROPERTY_STATS_300X_COMBOS,
		PROPERTY_STATS_BULLETS_FIRED,
		PROPERTY_STATS_MORTARS_SHELLS_FIRED,
		PROPERTY_STATS_HOWITZER_SHELLS_FIRED,
		PROPERTY_STATS_MISSILES_FIRED,
		PROPERTY_STATS_BARRAGES_EARNED_FROM_RED_STARS,
		PROPERTY_STATS_BARRAGES_EARNED_FROM_TURBOCHARGE,
		PROPERTY_STATS_BARRAGES_PURCHASED_IN_VERSUS,
		PROPERTY_STATS_INFANTRY_GASSED,
		PROPERTY_STATS_INFANTRY_SET_ON_FIRE,
		PROPERTY_STATS_HIGHEST_WITH_A_SINGLE_BARRAGE,
		PROPERTY_STATS_MOST_BARRAGES_IN_A_SINGLE_MAP,
		PROPERTY_STATS_HIGHEST_SURVIVAL_ROUND,
		PROPERTY_STATS_HIGHEST_LOCKDOWN_ROUND,
		PROPERTY_STATS_HIGHEST_HARDCORE_ROUND,
		PROPERTY_STATS_GOLDEN_FLIES_EXPLODED,
		PROPERTY_STATS_TURRETS_RESCUED,
		~0, //NO LB
		~0
	};

	const u32 cPropertyMedal			= PROPERTY_OVERALL_MEDAL;
	const u32 cPropertyChallengeProgress = PROPERTY_CHALLENGE_PROGRESS;

	// Match Query
	const u32 cMatchQueryStandard	= SESSION_MATCH_QUERY_STANDARD;
	const u32 cMatchQueryReplay		= SESSION_MATCH_QUERY_REPLAY;

	// Achievements
	u32 cAchievementIds[GameFlags::cACHIEVEMENTS_COUNT] = 
	{
		ACHIEVEMENT_SHOCKING_RESULTS,
		ACHIEVEMENT_SUNK,
		ACHIEVEMENT_A_FEW_LOOSE_SCREWS,
		ACHIEVEMENT_WITH_DISTINCTION,
		ACHIEVEMENT_HIGHLY_DECORATED,
		ACHIEVEMENT_LIKE_IT_NEVER_HAPPENED,
		ACHIEVEMENT_A_JOB_WELL_DONE,
		ACHIEVEMENT_EFFECTIVE_TACTICIAN,
		ACHIEVEMENT_DEMOLITION_MAN,
		ACHIEVEMENT_CONCENTRATED_FIRE,
		ACHIEVEMENT_I_CANT_GET_A_TONE,
		ACHIEVEMENT_BRUTE_FORCE,
		ACHIEVEMENT_RESOLUTE,
		ACHIEVEMENT_SYNERGY,
		ACHIEVEMENT_IN_SYNC,
		ACHIEVEMENT_CLUTCH_REPAIRS,
		ACHIEVEMENT_AGGRESSIVE_INVESTMENT_STRATEGY,
		ACHIEVEMENT_KING_OF_THE_HILL,
		ACHIEVEMENT_PERSISTENCE,
		~0, //dlc 0 - 0 //these will be set by teh game app
		~0, //dlc 0 - 1
		~0, //dlc 0 - 2
		~0, //dlc 1 - 0
		~0, //dlc 1 - 1
		~0, //dlc 1 - 2
	};	


	// These arrays are sort of unioned together.
	const u32 cAvatarAwardIds[GameFlags::cAVATAR_AWARDS_COUNT] = 
	{
		AVATARASSETAWARD_JACKET,
		AVATARASSETAWARD_MULLET,
		AVATARASSETAWARD_TSHIRT,
		~0, // DIVIDER
		~0, // EMPTY
		~0  // EMPTY
	};	

	const u32 cGamerPicsIds[GameFlags::cAVATAR_AWARDS_COUNT] = 
	{
		~0, // EMPTY
		~0, // EMPTY
		~0, // EMPTY
		~0, // DIVIDER
		GAMER_PICTURE_GO_COMMANDO,
		GAMER_PICTURE_CRAZY_IVAN
	};	

	// Rich Presence
	namespace RichPresence
	{
		const u32 cContextSurvivalName = CONTEXT_CHALLENGE_NAME;
		const u32 cContextCampaignName2 = CONTEXT_CAMPAIGN_NAME2;
		const u32 cContextH2HName = CONTEXT_VERSUS_NAME;
		const u32 cContextMinigameName = CONTEXT_MINIGAME_NAME;

		const u32 cContextCampaignName2Values[cCAMPAIGN_LEVEL_COUNT] =
		{
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_00,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_01,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_02,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_03,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_04,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_05,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_06,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_07,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_08,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_09,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_10,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_11,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_12,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_13,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_14,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_15,
			CONTEXT_CAMPAIGN_NAME2_CAMPAIGN_16
		};

		const u32 cContextSurvivalNameValues[cSURVIVAL_LEVEL_COUNT] =
		{
			CONTEXT_CHALLENGE_NAME_CH01,
			CONTEXT_CHALLENGE_NAME_CH02,
			CONTEXT_CHALLENGE_NAME_CH03,
			CONTEXT_CHALLENGE_NAME_CH04,
			CONTEXT_CHALLENGE_NAME_CH05
		};

		const u32 cContextMinigameNameValues[cMINIGAME_LEVEL_COUNT] =
		{
			CONTEXT_MINIGAME_NAME_MINIGAME_00,
			CONTEXT_MINIGAME_NAME_MINIGAME_01,
			CONTEXT_MINIGAME_NAME_MINIGAME_02,
			CONTEXT_MINIGAME_NAME_MINIGAME_03,
			CONTEXT_MINIGAME_NAME_MINIGAME_04,
			CONTEXT_MINIGAME_NAME_MINIGAME_05,
			CONTEXT_MINIGAME_NAME_MINIGAME_06,
			CONTEXT_MINIGAME_NAME_MINIGAME_07
		};

		const u32 cContextH2HNameValues[cH2H_LEVEL_COUNT] =
		{
			CONTEXT_VERSUS_NAME_VS00,
			CONTEXT_VERSUS_NAME_VS01,
			CONTEXT_VERSUS_NAME_VS02,
			CONTEXT_VERSUS_NAME_VS03,
			CONTEXT_VERSUS_NAME_VS04,
			CONTEXT_VERSUS_NAME_VS05
		};

		const u32 cRichPresenceMenu				= CONTEXT_PRESENCE_MENU;
		const u32 cRichPresenceNotParticipating = CONTEXT_PRESENCE_NOT_PARTICIPATING;
		const u32 cRichPresenceCampaignSP		= CONTEXT_PRESENCE_CAMPAIGN_SP;
		const u32 cRichPresenceCampaignMP		= CONTEXT_PRESENCE_CAMPAIGN_MP;
		const u32 cRichPresenceSurvivalSP		= CONTEXT_PRESENCE_CHALLENGE_SP;
		const u32 cRichPresenceSurvivalMP		= CONTEXT_PRESENCE_CHALLENGE_MP;
		const u32 cRichPresenceLobby			= CONTEXT_PRESENCE_LOBBY;
		const u32 cRichPresenceHeadToHeadUSA	= CONTEXT_PRESENCE_HEAD_TO_HEAD_USA;
		const u32 cRichPresenceHeadToHeadUSSR	= CONTEXT_PRESENCE_HEAD_TO_HEAD_USSR;
		const u32 cRichPresenceMinigameSP		= CONTEXT_PRESENCE_MINIGAME_SP;
		const u32 cRichPresenceMinigameMP		= CONTEXT_PRESENCE_MINIGAME_MP;

	}
}}		   