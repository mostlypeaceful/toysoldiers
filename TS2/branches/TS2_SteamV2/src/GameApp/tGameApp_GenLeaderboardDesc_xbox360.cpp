#include "GameAppPch.hpp"
#include "tGameApp.hpp"
#include "tLeaderboard.hpp"
#include "../../XBLA/FullGame/LiveFiles/ts2.spa.h"

namespace Sig
{

#define level_leaderboard_diff_spec( title, diff )\
	{ \
		SPASTRING_LB_##title##_##diff##_NAME,\
		{ STATS_VIEW_##title##_##diff##, 5, \
			{ \
				STATS_COLUMN_##title##_##diff##_SCORE, \
				STATS_COLUMN_##title##_##diff##_WINS, \
				STATS_COLUMN_##title##_##diff##_LOSSES, \
				STATS_COLUMN_##title##_##diff##_MEDAL, \
				STATS_COLUMN_##title##_##diff##_CHALLENGE_PROGRESS, \
			} \
		}, \
		{ \
			tLeaderboardColumnDesc( SPASTRING_LB_LEVEL0_##diff##_SCORE_NAME, 170, Gui::tText::cAlignRight ), \
			tLeaderboardColumnDesc( SPASTRING_LB_LEVEL0_##diff##_WINS_NAME, 120, Gui::tText::cAlignCenter ), \
			tLeaderboardColumnDesc( SPASTRING_LB_LEVEL0_##diff##_LOSSES_NAME, 120, Gui::tText::cAlignCenter ), \
			tLeaderboardColumnDesc( SPASTRING_LB_LEVEL0_##diff##_MEDAL_NAME, 100, Gui::tText::cAlignCenter, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_MEDAL ), \
			tLeaderboardColumnDesc( SPASTRING_LB_LEVEL1_CASUAL_CHALLENGE_PROGRESS_NAME, 200, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_CHALLENGEPROGRESS ), \
		}\
	}

#define level_leaderboard_spec( title ) \
	level_leaderboard_diff_spec( title, CASUAL ),\
	level_leaderboard_diff_spec( title, NORMAL ),\
	level_leaderboard_diff_spec( title, HARD ),\
	level_leaderboard_diff_spec( title, ELITE ),\
	level_leaderboard_diff_spec( title, GENERAL )

#define survival_leaderboard_mode_spec( title, mode )\
	{\
		SPASTRING_LB_##title##_##mode##_NAME,\
		{ STATS_VIEW_##title##_##mode##, 4, \
			{ \
				STATS_COLUMN_##title##_##mode##_SCORE, \
				STATS_COLUMN_##title##_##mode##_TIME, \
				STATS_COLUMN_##title##_##mode##_KILLS, \
				STATS_COLUMN_##title##_##mode##_CHALLENGE_PROGRESS, \
			} \
		}, \
		{ \
			tLeaderboardColumnDesc( SPASTRING_LB_##title##_##mode##_SCORE_NAME, 120, Gui::tText::cAlignRight ), \
			tLeaderboardColumnDesc( SPASTRING_LB_PLAYER_STATS_TOTAL_TIME_NAME, 160, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_TIME ), \
			tLeaderboardColumnDesc( SPASTRING_LB_PLAYER_STATS_KILLS_NAME, 160, Gui::tText::cAlignRight ), \
			tLeaderboardColumnDesc( SPASTRING_LB_LEVEL1_CASUAL_CHALLENGE_PROGRESS_NAME, 250, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_CHALLENGEPROGRESS ), \
		}\
	}

#define survival_leaderboard_spec( title ) \
	survival_leaderboard_mode_spec( title, SURVIVAL ),\
	survival_leaderboard_mode_spec( title, LOCKDOWN ),\
	survival_leaderboard_mode_spec( title, HARDCORE ),\
	survival_leaderboard_mode_spec( title, TRAUMA ),\
	survival_leaderboard_mode_spec( title, HARDLOCK )

#define h2h_leaderboard_spec( title ) \
	{\
		SPASTRING_LB_H2H_##title##_NAME, \
		{ STATS_VIEW_H2H_##title##, 2, \
			{ \
				STATS_COLUMN_H2H_##title##_WINS, \
				STATS_COLUMN_H2H_##title##_LOSSES, \
			} \
		}, \
		{ \
			tLeaderboardColumnDesc( SPASTRING_LB_H2H_WINS_NAME, 100, Gui::tText::cAlignCenter ), \
			tLeaderboardColumnDesc( SPASTRING_LB_H2H_LOSSES_NAME, 160, Gui::tText::cAlignCenter ), \
		} \
	}

#define minigame_leaderboard_spec( title ) \
	{  \
		SPASTRING_LB_##title##_NAME, \
		{ STATS_VIEW_##title##, 2,  \
		{  \
			STATS_COLUMN_##title##_SCORE,  \
			STATS_COLUMN_##title##_CHALLENGE_PROGRESS, \
		}  \
		},  \
		{  \
		tLeaderboardColumnDesc( SPASTRING_LB_##title##_SCORE_NAME, 180, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_MINIGAME_META ),  \
		tLeaderboardColumnDesc( SPASTRING_LB_LEVEL1_CASUAL_CHALLENGE_PROGRESS_NAME, 240, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_CHALLENGEPROGRESS ), \
		} \
	}

	static tLeaderboardDesc gLeaderboardSpecs[] = 
	{
		{ 
			SPASTRING_LB_ARCADE_LEADERBOARD_NAME,
			{ STATS_VIEW_ARCADE_LEADERBOARD, 3, 
				{ 
					STATS_COLUMN_ARCADE_LEADERBOARD_TOTAL_SCORE,
					STATS_COLUMN_ARCADE_LEADERBOARD_TOTAL_KILLS, 
					STATS_COLUMN_ARCADE_LEADERBOARD_TOTAL_MONEY, 
				} 
			}, 
			{ 
				tLeaderboardColumnDesc( SPASTRING_LB_ARCADE_LEADERBOARD_TOTAL_SCORE_NAME, 250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( SPASTRING_LB_ARCADE_LEADERBOARD_TOTAL_KILLS_NAME, 200, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( SPASTRING_LB_ARCADE_LEADERBOARD_TOTAL_MONEY_NAME, 200, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_MONEY ),
			}
		},

		{
			SPASTRING_LB_PLAYER_STATS_NAME,
			{ STATS_VIEW_PLAYER_STATS, 61,
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
					STATS_COLUMN_PLAYER_STATS_VERSUS_WAVES_LAUNCHED
				}
			},
			{
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_SCORE,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_ENEMIES_REACHED_GOAL,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_INFANTRY_REACHED_GOAL,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TOTAL_MONEY,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_MONEY_EARNED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_UNITS_PURCHASED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_UNITS_UPGRADED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_UNITS_REPAIRED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_KILLS,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_HIGHEST_COMBO,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_AMMO_EXPENDED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_BARRAGES_CALLED_IN,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_KILLS_WITH_BARRAGES,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TOTAL_TIME,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TIME_IN_UNITS,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TIME_IN_VEHICLES,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TIME_IN_SHELLCAM,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TRIES_BEFORE_COMPLETION,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_KILLS_IN_NIGHT_VISION,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_HIGHEST_FLYING_SOLDIER,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_SECONDS_SKIPPED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_OVERKILL,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_MONEY_SPENT,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_DEFENSE_SUB_SCORE,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TIME_SUB_SCORE,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_MONEY_SUB_SCORE,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_MINIGAME_META_STAT,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_ASSISTS,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_OVERCHARGE,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_WAVE_BONUS,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_WAVE_CHAIN,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_PAYBACK,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_CLOSE_CALL,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_BOMBING_RUN,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_SPEED_BONUS,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_KILLS_WHILE_USING_TURRETS,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_BASIC_INFANTRY_KILLED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_ELITE_INFANTRY_KILLED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_ATVS_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_CARS_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_APCS_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_IFVS_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_MEDIUM_TANKS_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_HEAVY_TANKS_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TRANSPORT_COPTERS_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_GUNSHIPS_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_ATTACK_COPTERS_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_FIGHTER_PLANES_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TRANSPORT_PLANES_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_BOMBERS_DESTROYED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TANKS_REACHED_TOY_BOX,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_APCS_REACHED_TOY_BOX,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_PLANES_REACHED_TOY_BOX,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_HELICOPTERS_REACHED_TOY_BOX,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_CARS_REACHED_TOY_BOX,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_ATVS_REACHED_TOY_BOX,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_TURRETS_LOST,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_VEHICLES_PURCHASED,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_VEHICLES_LOST,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_VEHICLES_DRIVEN_INTO_GOAL,		250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS_VERSUS_WAVES_LAUNCHED,		250, Gui::tText::cAlignRight ), 
			}
		},
		{
			SPASTRING_LB_PLAYER_STATS2_NAME,
			{ STATS_VIEW_PLAYER_STATS2, 59,
			{
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
				}
			},
			{
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TIME_USING_TURRETS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TIME_USING_AC130,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TIME_USING_COMMANDO,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TIME_USING_IVAN,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TIME_USING_MEDIUM_TANK,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TIME_USING_HEAVY_TANK,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TIME_USING_ATTACK_HELICOPTER,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TIME_USING_HELICOPTER_GUNSHIP,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TIME_USING_FIGHTER_PLANE,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_MOST_KILLS_WITH_A_NUKE,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TIME_SPENT_IN_TURBOCHARGE,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TURBOCHARGE_KILLS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_MACHINEGUNS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_HOWITZERS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_MORTARS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_ANTI_TANK,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_MAKESHIFT,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TURBOCHARGES_WITH_ANTI_AIR,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_MOST_HOTSWAPS_IN_A_SINGLE_TURBOCHARGE,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TICKETS_LOST_IN_HEAD_2_HEAD,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TICKETS_DEALT_IN_HEAD_2_HEAD,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_VERSUS_WINS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_VERSUS_LOSSES,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_VERSUS_WIN_LOSS_RATIO,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_VERSUS_SHUT_OUTS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_VERSUS_TURRETS_DESTROYED,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_VERSUS_PLATFORMS_CAPTURED,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_DECORATIONS_ACQUIRED,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TOTAL_BRONZE_MEDALS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TOTAL_SILVER_MEDALS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TOTAL_GOLD_MEDALS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TOTAL_PLATINUM_MEDALS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TOTAL_CHALLENGES_EARNED,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_5X_COMBOS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_10X_COMBOS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_20X_COMBOS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_40X_COMBOS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_80X_COMBOS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_100X_COMBOS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_150X_COMBOS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_200X_COMBOS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_250X_COMBOS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_300X_COMBOS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_BULLETS_FIRED,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_MORTARS_SHELLS_FIRED,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_HOWITZER_SHELLS_FIRED,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_MISSILES_FIRED,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_BARRAGES_EARNED_FROM_RED_STARS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_BARRAGES_EARNED_FROM_TURBOCHARGE,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_BARRAGES_PURCHASED_IN_VERSUS,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_INFANTRY_GASSED,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_INFANTRY_SET_ON_FIRE,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_HIGHEST_WITH_A_SINGLE_BARRAGE,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_MOST_BARRAGES_IN_A_SINGLE_MAP,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_HIGHEST_SURVIVAL_ROUND,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_HIGHEST_LOCKDOWN_ROUND,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_HIGHEST_HARDCORE_ROUND,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_GOLDEN_FLIES_EXPLODED,	250, Gui::tText::cAlignRight ), 
				tLeaderboardColumnDesc( STATS_COLUMN_PLAYER_STATS2_TURRETS_RESCUED,	250, Gui::tText::cAlignRight ), 
				}
			},

		minigame_leaderboard_spec( TRIAL_MINIGAME1 ),
		minigame_leaderboard_spec( TRIAL_MINIGAME2 ),
		minigame_leaderboard_spec( MINIGAME_ATV ),
		minigame_leaderboard_spec( MINIGAME_DX ),
		minigame_leaderboard_spec( MINIGAME_FLY ),
		minigame_leaderboard_spec( MINIGAME_AC130 ),
		minigame_leaderboard_spec( MINIGAME_HALLWAY ),
		minigame_leaderboard_spec( MINIGAME_F14 ),

		level_leaderboard_spec( LEVEL0 ),
		level_leaderboard_spec( LEVEL1 ),
		level_leaderboard_spec( LEVEL2 ),
		level_leaderboard_spec( LEVEL3 ),
		level_leaderboard_spec( LEVEL4 ),
		level_leaderboard_spec( LEVEL5 ),
		level_leaderboard_spec( LEVEL6 ),
		level_leaderboard_spec( LEVEL7 ),
		level_leaderboard_spec( LEVEL8 ),
		level_leaderboard_spec( LEVEL9 ),
		level_leaderboard_spec( LEVEL10 ),
		level_leaderboard_spec( LEVEL11 ),
		level_leaderboard_spec( LEVEL12 ),
		level_leaderboard_spec( LEVEL13 ),
		level_leaderboard_spec( LEVEL14 ),
		level_leaderboard_spec( LEVEL15 ),
		level_leaderboard_spec( LEVEL16 ),

		survival_leaderboard_spec( SURVIVAL0 ),
		survival_leaderboard_spec( SURVIVAL1 ),
		survival_leaderboard_spec( SURVIVAL2 ),
		survival_leaderboard_spec( SURVIVAL3 ),
		survival_leaderboard_spec( SURVIVAL4 ),

		h2h_leaderboard_spec( LEVEL1 ),
		h2h_leaderboard_spec( LEVEL2 ),
		h2h_leaderboard_spec( LEVEL3 ),
		h2h_leaderboard_spec( LEVEL4 ),
		h2h_leaderboard_spec( LEVEL5 ),
		h2h_leaderboard_spec( TOTALS ),

	};

#undef level_leaderboard_spec
#undef level_leaderboard_diff_spec
#undef h2h_leaderboard_spec

	void tGameApp::fSetupLeaderboardData( )
	{
		tLeaderboard::fSetLeaderboards( array_length( gLeaderboardSpecs ), gLeaderboardSpecs );
	}

} //namespace Sig
