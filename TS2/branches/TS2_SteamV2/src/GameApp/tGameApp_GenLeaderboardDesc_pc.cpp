#include "GameAppPch.hpp"
#include "tGameApp.hpp"
#include "tLeaderboard.hpp"
#include "../../XBLA/FullGame/LiveFiles/ts2.spa.h"

namespace Sig
{

// Two fixes for inconsistent spelling in gameconfig
#define PROPERTY_STATS_OVERKILL PROPERTY_STATS_OVER_KILL
#define PROPERTY_STATS_APCS_REACHED_TOY_BOX PROPERTY_STATS_APCS_REACHES_TOY_BOX

#define spa_string( id ) #id
#define view_desc( id, num ) id, spa_string( id ), num

#define leaderboard_column_spec( id, agg, prop, width, align )\
	tLeaderboardColumnDesc( #id, agg, prop, width, align )

#define leaderboard_column_spec_full( id, agg, prop, width, align, vis, user )\
	tLeaderboardColumnDesc( #id, agg, prop, width, align, vis, user )

#define leaderboard_column_spec_stat( id )\
	tLeaderboardColumnDesc( spa_string( STATS_COLUMN_PLAYER_STATS_##id ), cAggregateMax, PROPERTY_STATS_##id, 250, Gui::tText::cAlignRight )

#define leaderboard_column_spec_stat2( id )\
	tLeaderboardColumnDesc( spa_string( STATS_COLUMN_PLAYER_STATS2_##id ), cAggregateMax, PROPERTY_STATS_##id, 250, Gui::tText::cAlignRight )

#define level_leaderboard_diff_spec( title, diff )\
	{ \
		spa_string( LB_##title##_##diff##_NAME ),\
		{ STATS_VIEW_##title##_##diff##, spa_string( STATS_VIEW_##title##_##diff## ), 5, \
			{ \
				STATS_COLUMN_##title##_##diff##_SCORE, \
				STATS_COLUMN_##title##_##diff##_WINS, \
				STATS_COLUMN_##title##_##diff##_LOSSES, \
				STATS_COLUMN_##title##_##diff##_MEDAL, \
				STATS_COLUMN_##title##_##diff##_CHALLENGE_PROGRESS, \
			} \
		}, \
		{ \
			leaderboard_column_spec( LB_LEVEL0_##diff##_SCORE_NAME, cAggregateMax, PROPERTY_SCORE, 170, Gui::tText::cAlignRight ), \
			leaderboard_column_spec( LB_LEVEL0_##diff##_WINS_NAME, cAggregateSum, PROPERTY_WINS, 120, Gui::tText::cAlignCenter ), \
			leaderboard_column_spec( LB_LEVEL0_##diff##_LOSSES_NAME, cAggregateSum, PROPERTY_LOSSES, 120, Gui::tText::cAlignCenter ), \
			leaderboard_column_spec_full( LB_LEVEL0_##diff##_MEDAL_NAME, cAggregateMax, PROPERTY_OVERALL_MEDAL, 100, Gui::tText::cAlignCenter, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_MEDAL ), \
			leaderboard_column_spec_full( LB_LEVEL1_CASUAL_CHALLENGE_PROGRESS_NAME, cAggregateMax, PROPERTY_CHALLENGE_PROGRESS, 200, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_CHALLENGEPROGRESS ), \
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
		spa_string( LB_##title##_##mode##_NAME ),\
		{ STATS_VIEW_##title##_##mode##, spa_string( STATS_VIEW_##title##_##mode## ), 4, \
			{ \
				STATS_COLUMN_##title##_##mode##_SCORE, \
				STATS_COLUMN_##title##_##mode##_TIME, \
				STATS_COLUMN_##title##_##mode##_KILLS, \
				STATS_COLUMN_##title##_##mode##_CHALLENGE_PROGRESS, \
			} \
		}, \
		{ \
			leaderboard_column_spec( LB_##title##_##mode##_SCORE_NAME, cAggregateMax, PROPERTY_SCORE, 120, Gui::tText::cAlignRight ), \
			leaderboard_column_spec_full( LB_PLAYER_STATS_TOTAL_TIME_NAME, cAggregateMax, PROPERTY_TIME, 160, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_TIME ), \
			leaderboard_column_spec( LB_PLAYER_STATS_KILLS_NAME, cAggregateMax, PROPERTY_KILLS, 160, Gui::tText::cAlignRight ), \
			leaderboard_column_spec_full( LB_LEVEL1_CASUAL_CHALLENGE_PROGRESS_NAME, cAggregateMax, PROPERTY_CHALLENGE_PROGRESS, 250, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_CHALLENGEPROGRESS ), \
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
		spa_string( LB_H2H_##title##_NAME ), \
		{ STATS_VIEW_H2H_##title##, spa_string( STATS_VIEW_H2H_##title## ), 2, \
			{ \
				STATS_COLUMN_H2H_##title##_WINS, \
				STATS_COLUMN_H2H_##title##_LOSSES, \
			} \
		}, \
		{ \
			leaderboard_column_spec( LB_H2H_WINS_NAME, cAggregateSum, PROPERTY_WINS, 100, Gui::tText::cAlignCenter ), \
			leaderboard_column_spec( LB_H2H_LOSSES_NAME, cAggregateSum, PROPERTY_LOSSES, 160, Gui::tText::cAlignCenter ), \
		} \
	}

#define minigame_leaderboard_spec( title ) \
	{  \
		spa_string( LB_##title##_NAME ), \
		{ STATS_VIEW_##title##, spa_string( STATS_VIEW_##title## ), 2,  \
			{  \
				STATS_COLUMN_##title##_SCORE,  \
				STATS_COLUMN_##title##_CHALLENGE_PROGRESS, \
			}  \
		}, \
		{  \
			leaderboard_column_spec_full( LB_##title##_SCORE_NAME, cAggregateMax, PROPERTY_SCORE, 180, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_MINIGAME_META ),  \
			leaderboard_column_spec_full( LB_LEVEL1_CASUAL_CHALLENGE_PROGRESS_NAME, cAggregateMax, PROPERTY_CHALLENGE_PROGRESS, 240, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_CHALLENGEPROGRESS ), \
		} \
	}

	static tLeaderboardDesc gLeaderboardSpecs[] = 
	{
		{ 
			spa_string( LB_ARCADE_LEADERBOARD_NAME ),
			{ STATS_VIEW_ARCADE_LEADERBOARD, spa_string( STATS_VIEW_ARCADE_LEADERBOARD), 3, 
				{ 
					STATS_COLUMN_ARCADE_LEADERBOARD_TOTAL_SCORE,
					STATS_COLUMN_ARCADE_LEADERBOARD_TOTAL_KILLS, 
					STATS_COLUMN_ARCADE_LEADERBOARD_TOTAL_MONEY, 
				}
			},
			{ 
				leaderboard_column_spec( LB_ARCADE_LEADERBOARD_TOTAL_SCORE_NAME, cAggregateMax, PROPERTY_SCORE, 250, Gui::tText::cAlignRight ), 
				leaderboard_column_spec( LB_ARCADE_LEADERBOARD_TOTAL_KILLS_NAME, cAggregateSum, PROPERTY_KILLS, 200, Gui::tText::cAlignRight ), 
				leaderboard_column_spec_full( LB_ARCADE_LEADERBOARD_TOTAL_MONEY_NAME, cAggregateSum, PROPERTY_MONEY, 200, Gui::tText::cAlignRight, cLeaderBoardVisible, GameFlags::cLEADERBOARD_COLUMN_TYPE_MONEY ),
			}
		},

		{
			spa_string( LB_PLAYER_STATS_NAME ),
			{ STATS_VIEW_PLAYER_STATS, spa_string( STATS_VIEW_PLAYER_STATS ), 61,
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
				leaderboard_column_spec_stat( SCORE ), 
				leaderboard_column_spec_stat( ENEMIES_REACHED_GOAL ), 
				leaderboard_column_spec_stat( INFANTRY_REACHED_GOAL ), 
				leaderboard_column_spec_stat( TOTAL_MONEY ), 
				leaderboard_column_spec_stat( MONEY_EARNED ), 
				leaderboard_column_spec_stat( UNITS_PURCHASED ), 
				leaderboard_column_spec_stat( UNITS_UPGRADED ), 
				leaderboard_column_spec_stat( UNITS_REPAIRED ), 
				leaderboard_column_spec_stat( KILLS ), 
				leaderboard_column_spec_stat( HIGHEST_COMBO ), 
				leaderboard_column_spec_stat( AMMO_EXPENDED ), 
				leaderboard_column_spec_stat( BARRAGES_CALLED_IN ), 
				leaderboard_column_spec_stat( KILLS_WITH_BARRAGES ), 
				leaderboard_column_spec_stat( TOTAL_TIME ), 
				leaderboard_column_spec_stat( TIME_IN_UNITS ), 
				leaderboard_column_spec_stat( TIME_IN_VEHICLES ), 
				leaderboard_column_spec_stat( TIME_IN_SHELLCAM ), 
				leaderboard_column_spec_stat( TRIES_BEFORE_COMPLETION ), 
				leaderboard_column_spec_stat( KILLS_IN_NIGHT_VISION ), 
				leaderboard_column_spec_stat( HIGHEST_FLYING_SOLDIER ), 
				leaderboard_column_spec_stat( SECONDS_SKIPPED ), 
				leaderboard_column_spec_stat( OVERKILL ),
				leaderboard_column_spec_stat( MONEY_SPENT ),
				leaderboard_column_spec_stat( DEFENSE_SUB_SCORE ),
				leaderboard_column_spec_stat( TIME_SUB_SCORE ),
				leaderboard_column_spec_stat( MONEY_SUB_SCORE ),
				leaderboard_column_spec_stat( MINIGAME_META_STAT ),
				leaderboard_column_spec_stat( ASSISTS ),
				leaderboard_column_spec_stat( OVERCHARGE ),
				leaderboard_column_spec_stat( WAVE_BONUS ),
				leaderboard_column_spec_stat( WAVE_CHAIN ),
				leaderboard_column_spec_stat( PAYBACK ),
				leaderboard_column_spec_stat( CLOSE_CALL ),
				leaderboard_column_spec_stat( BOMBING_RUN ),
				leaderboard_column_spec_stat( SPEED_BONUS ),
				leaderboard_column_spec_stat( KILLS_WHILE_USING_TURRETS ),
				leaderboard_column_spec_stat( BASIC_INFANTRY_KILLED ),
				leaderboard_column_spec_stat( ELITE_INFANTRY_KILLED ),
				leaderboard_column_spec_stat( ATVS_DESTROYED ),
				leaderboard_column_spec_stat( CARS_DESTROYED ),
				leaderboard_column_spec_stat( APCS_DESTROYED ),
				leaderboard_column_spec_stat( IFVS_DESTROYED ),
				leaderboard_column_spec_stat( MEDIUM_TANKS_DESTROYED ),
				leaderboard_column_spec_stat( HEAVY_TANKS_DESTROYED ),
				leaderboard_column_spec_stat( TRANSPORT_COPTERS_DESTROYED ),
				leaderboard_column_spec_stat( GUNSHIPS_DESTROYED ),
				leaderboard_column_spec_stat( ATTACK_COPTERS_DESTROYED ),
				leaderboard_column_spec_stat( FIGHTER_PLANES_DESTROYED ),
				leaderboard_column_spec_stat( TRANSPORT_PLANES_DESTROYED ),
				leaderboard_column_spec_stat( BOMBERS_DESTROYED ),
				leaderboard_column_spec_stat( TANKS_REACHED_TOY_BOX ),
				leaderboard_column_spec_stat( APCS_REACHED_TOY_BOX ),
				leaderboard_column_spec_stat( PLANES_REACHED_TOY_BOX ),
				leaderboard_column_spec_stat( HELICOPTERS_REACHED_TOY_BOX ),
				leaderboard_column_spec_stat( CARS_REACHED_TOY_BOX ),
				leaderboard_column_spec_stat( ATVS_REACHED_TOY_BOX ),
				leaderboard_column_spec_stat( TURRETS_LOST ),
				leaderboard_column_spec_stat( VEHICLES_PURCHASED ),
				leaderboard_column_spec_stat( VEHICLES_LOST ),
				leaderboard_column_spec_stat( VEHICLES_DRIVEN_INTO_GOAL ),
				leaderboard_column_spec_stat( VERSUS_WAVES_LAUNCHED ),
			}
		},
		{
			spa_string( LB_PLAYER_STATS2_NAME ),
			{ STATS_VIEW_PLAYER_STATS2, spa_string( STATS_VIEW_PLAYER_STATS2 ), 59,
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
				leaderboard_column_spec_stat2( TIME_USING_TURRETS ),
				leaderboard_column_spec_stat2( TIME_USING_AC130 ),
				leaderboard_column_spec_stat2( TIME_USING_COMMANDO ),
				leaderboard_column_spec_stat2( TIME_USING_IVAN ),
				leaderboard_column_spec_stat2( TIME_USING_MEDIUM_TANK ),
				leaderboard_column_spec_stat2( TIME_USING_HEAVY_TANK ),
				leaderboard_column_spec_stat2( TIME_USING_ATTACK_HELICOPTER ),
				leaderboard_column_spec_stat2( TIME_USING_HELICOPTER_GUNSHIP ),
				leaderboard_column_spec_stat2( TIME_USING_FIGHTER_PLANE ),
				leaderboard_column_spec_stat2( MOST_KILLS_WITH_A_NUKE ),
				leaderboard_column_spec_stat2( TIME_SPENT_IN_TURBOCHARGE ),
				leaderboard_column_spec_stat2( TURBOCHARGE_KILLS ),
				leaderboard_column_spec_stat2( TURBOCHARGES_WITH_MACHINEGUNS ),
				leaderboard_column_spec_stat2( TURBOCHARGES_WITH_HOWITZERS ),
				leaderboard_column_spec_stat2( TURBOCHARGES_WITH_MORTARS ),
				leaderboard_column_spec_stat2( TURBOCHARGES_WITH_ANTI_TANK ),
				leaderboard_column_spec_stat2( TURBOCHARGES_WITH_MAKESHIFT ),
				leaderboard_column_spec_stat2( TURBOCHARGES_WITH_ANTI_AIR ),
				leaderboard_column_spec_stat2( MOST_HOTSWAPS_IN_A_SINGLE_TURBOCHARGE ),
				leaderboard_column_spec_stat2( TICKETS_LOST_IN_HEAD_2_HEAD ),
				leaderboard_column_spec_stat2( TICKETS_DEALT_IN_HEAD_2_HEAD ),
				leaderboard_column_spec_stat2( VERSUS_WINS ),
				leaderboard_column_spec_stat2( VERSUS_LOSSES ),
				leaderboard_column_spec_stat2( VERSUS_WIN_LOSS_RATIO ),
				leaderboard_column_spec_stat2( VERSUS_SHUT_OUTS ),
				leaderboard_column_spec_stat2( VERSUS_TURRETS_DESTROYED ),
				leaderboard_column_spec_stat2( VERSUS_PLATFORMS_CAPTURED ),
				leaderboard_column_spec_stat2( DECORATIONS_ACQUIRED ),
				leaderboard_column_spec_stat2( TOTAL_BRONZE_MEDALS ),
				leaderboard_column_spec_stat2( TOTAL_SILVER_MEDALS ),
				leaderboard_column_spec_stat2( TOTAL_GOLD_MEDALS ),
				leaderboard_column_spec_stat2( TOTAL_PLATINUM_MEDALS ),
				leaderboard_column_spec_stat2( TOTAL_CHALLENGES_EARNED ),
				leaderboard_column_spec_stat2( 5X_COMBOS ),
				leaderboard_column_spec_stat2( 10X_COMBOS ),
				leaderboard_column_spec_stat2( 20X_COMBOS ),
				leaderboard_column_spec_stat2( 40X_COMBOS ),
				leaderboard_column_spec_stat2( 80X_COMBOS ),
				leaderboard_column_spec_stat2( 100X_COMBOS ),
				leaderboard_column_spec_stat2( 150X_COMBOS ),
				leaderboard_column_spec_stat2( 200X_COMBOS ),
				leaderboard_column_spec_stat2( 250X_COMBOS ),
				leaderboard_column_spec_stat2( 300X_COMBOS ),
				leaderboard_column_spec_stat2( BULLETS_FIRED ),
				leaderboard_column_spec_stat2( MORTARS_SHELLS_FIRED ),
				leaderboard_column_spec_stat2( HOWITZER_SHELLS_FIRED ),
				leaderboard_column_spec_stat2( MISSILES_FIRED ),
				leaderboard_column_spec_stat2( BARRAGES_EARNED_FROM_RED_STARS ),
				leaderboard_column_spec_stat2( BARRAGES_EARNED_FROM_TURBOCHARGE ),
				leaderboard_column_spec_stat2( BARRAGES_PURCHASED_IN_VERSUS ),
				leaderboard_column_spec_stat2( INFANTRY_GASSED ),
				leaderboard_column_spec_stat2( INFANTRY_SET_ON_FIRE ),
				leaderboard_column_spec_stat2( HIGHEST_WITH_A_SINGLE_BARRAGE ),
				leaderboard_column_spec_stat2( MOST_BARRAGES_IN_A_SINGLE_MAP ),
				leaderboard_column_spec_stat2( HIGHEST_SURVIVAL_ROUND ),
				leaderboard_column_spec_stat2( HIGHEST_LOCKDOWN_ROUND ),
				leaderboard_column_spec_stat2( HIGHEST_HARDCORE_ROUND ),
				leaderboard_column_spec_stat2( GOLDEN_FLIES_EXPLODED ),
				leaderboard_column_spec_stat2( TURRETS_RESCUED ),
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
		level_leaderboard_spec( LEVEL17 ),
		level_leaderboard_spec( LEVEL18 ),
		level_leaderboard_spec( LEVEL19 ),
		level_leaderboard_spec( LEVEL20 ),
		level_leaderboard_spec( LEVEL21 ),
		level_leaderboard_spec( LEVEL22 ),
		level_leaderboard_spec( LEVEL23 ),
		level_leaderboard_spec( LEVEL24 ),
		level_leaderboard_spec( LEVEL25 ),
		level_leaderboard_spec( LEVEL26 ),
		level_leaderboard_spec( LEVEL27 ),
		level_leaderboard_spec( LEVEL28 ),
		level_leaderboard_spec( LEVEL29 ),
		level_leaderboard_spec( LEVEL30 ),
		level_leaderboard_spec( LEVEL31 ),
		level_leaderboard_spec( LEVEL32 ),
		level_leaderboard_spec( LEVEL33 ),
		level_leaderboard_spec( LEVEL34 ),
		level_leaderboard_spec( LEVEL35 ),
		level_leaderboard_spec( LEVEL36 ),
		level_leaderboard_spec( LEVEL37 ),
		level_leaderboard_spec( LEVEL38 ),
		level_leaderboard_spec( LEVEL39 ),
		level_leaderboard_spec( LEVEL40 ),
		level_leaderboard_spec( LEVEL41 ),
		level_leaderboard_spec( LEVEL42 ),
		level_leaderboard_spec( LEVEL43 ),
		level_leaderboard_spec( LEVEL44 ),
		level_leaderboard_spec( LEVEL45 ),
		level_leaderboard_spec( LEVEL46 ),
		level_leaderboard_spec( LEVEL47 ),

		survival_leaderboard_spec( SURVIVAL0 ),
		survival_leaderboard_spec( SURVIVAL1 ),
		survival_leaderboard_spec( SURVIVAL2 ),
		survival_leaderboard_spec( SURVIVAL3 ),
		survival_leaderboard_spec( SURVIVAL4 ),
		survival_leaderboard_spec( SURVIVAL5 ),
		survival_leaderboard_spec( SURVIVAL6 ),
		survival_leaderboard_spec( SURVIVAL7 ),
		survival_leaderboard_spec( SURVIVAL8 ),

		h2h_leaderboard_spec( LEVEL1 ),
		h2h_leaderboard_spec( LEVEL2 ),
		h2h_leaderboard_spec( LEVEL3 ),
		h2h_leaderboard_spec( LEVEL4 ),
		h2h_leaderboard_spec( LEVEL5 ),
		h2h_leaderboard_spec( LEVEL6 ),
		h2h_leaderboard_spec( LEVEL7 ),
		h2h_leaderboard_spec( LEVEL8 ),
		h2h_leaderboard_spec( LEVEL9 ),
		h2h_leaderboard_spec( LEVEL10 ),
		h2h_leaderboard_spec( LEVEL11 ),
		h2h_leaderboard_spec( LEVEL12 ),
		h2h_leaderboard_spec( LEVEL13 ),
		h2h_leaderboard_spec( LEVEL14 ),
		h2h_leaderboard_spec( LEVEL15 ),
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
