#include "GameAppPch.hpp"
#include "tGameSessionStats.hpp"
#include "tLevelLogic.hpp"
#include "tGameApp.hpp"
#include "tPlayer.hpp"
#include "tUnitLogic.hpp"
#include "tGameEffects.hpp"
#include "tVehicleLogic.hpp"
#include "tPersonalBestUI.hpp"
#include "GameSession.hpp"
#include "tSync.hpp"
#include "tWorldSpaceFloatingText.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( bool, Gameplay_Stats_Combos_ApplyTable, false );

	const f32 tGameSessionStats::cBarrageExtraPercent = 1.0f; //100%

	tComboStatGroup::tComboStatGroup( const Math::tVec3f& color, f32 comboTimer, const tStringPtr& texturePath )
		: mColor( color ), mScreenPos( tVec2f::cZeroVector )
		, mCurrentCombo( 0 )
		, mComboTimer( comboTimer ), mComboTimeRemaining( 0.0f )
		, mTexturePath( texturePath )
		, mRestart( false )
	{
	}

	void tComboStatGroup::fIncrement( u32 count, const Math::tVec2f& screenP )
	{
		if( !fActive( ) ) 
		{
			mRestart = true;
			mRestartPt = screenP;
			mCurrentCombo = 0;
			mComboTimeRemaining = mComboTimer;
		}

		mComboTimeRemaining = mComboTimer;
		mCurrentCombo += count;
	}

	void tComboStatGroup::fStepTimer( tPlayer* player, f32 dt )
	{
		mComboTimeRemaining -= dt;
		if( mComboTimeRemaining <= 0.f && mCurrentCombo > 0 )
		{
			player->fComboLost( );
			mCurrentCombo = 0;
		}
	}

	// See the powerup design doc for how this work
	tPair<b32, f32> tComboStatGroup::fComputePowerUpChance( ) const
	{
		b32 found = false;

		if( mCurrentCombo > 0 )
		{
			if( mModulus > 0 && mCurrentCombo % mModulus == 0 )
			{
				found = true;
			}
			else
			{
				for( u32 i = 0; i < mAdditionalMultipliers.fCount( ); ++i )
				{
					if( mCurrentCombo == mAdditionalMultipliers[ i ] )
					{
						found = true;
						break;
					}
				}
			}
		}

		f32 chance = 0.0f;
		if( found ) chance = mCurrentCombo * mModifier * 0.01f; //0.01 to map from percent to 0-1

		return tPair<b32, f32>( found, chance );
	}

	//////////////////////////////////////////////////////////////////////////////////////

	tGameSessionStats::tGameSessionStats( tPlayer* player )
		: mPlayer( player )
		, mComboMeterValue( 0.0f )
		, mOverChargeComboCost( 20.f )
		, mPauseTimers( false )
		, mFriendStatsInitialized( false )
		, mLevelStatsDirty( false )
		, mLockIncrements( false )
	{
		mStats.fFill( 0.f );
		mStatWasSet.fFill( false );


		if( Gameplay_Stats_Combos_ApplyTable )
			log_warning( 0, "Performance warning! Gameplay_Stats_Combos_ApplyTable enabled" );

		fConfigureCombos( );
	}

	tGameSessionStats::~tGameSessionStats( )
	{
	}

	void tGameSessionStats::fUpdate( f32 dt )
	{
		if( Gameplay_Stats_Combos_ApplyTable )
			fConfigureCombos( ); // keep applying these so the designers can tweek values in real time

		if( !mPauseTimers )
			for( u32 i = 0; i < mCombos.fCount( ); ++i )
				mCombos[ i ].fStepTimer( mPlayer, dt );

		// currently combo meter reflects only first combo group
		if( mOverChargeComboCost == 0 )
			fSetComboMeter( 0 );
		else
		{
			f32 comboMeter = mCombos[ 0 ].mCurrentCombo / mOverChargeComboCost;
			fSetComboMeter( comboMeter );
		}
		
		mComboTimerPercentage = fClamp( mCombos[ 0 ].mComboTimeRemaining / mCombos[ 0 ].mComboTimer, 0.f, 1.f );

#ifdef sync_system_enabled
		for( u32 s = 0; s < mStats.fCount( ); ++s )
			sync_d_event_c( GameFlags::fSESSION_STATSEnumToString( s ).fCStr( ), mStats[ s ], tSync::cSCStats );
		sync_d_event_v_c( mComboMeterValue, tSync::cSCStats );
		sync_d_event_v_c( mOverChargeComboCost, tSync::cSCStats );
		sync_d_event_v_c( mComboTimerPercentage, tSync::cSCStats );
#endif
	}

	s32 tGameSessionStats::fComboIndex( u32 type ) const
	{
		return mComboMapping[ type ];
	}

	void tGameSessionStats::fResetComboMeter( )
	{ 
		mComboMeterValue = 0.f;
		mCombos[ 0 ].fReset( );
	}

	namespace 
	{
		const u32 cComboTable = 0;
		enum tComboTableColumns
		{
			cComboTableColor,
			cComboTableScreenPos,
			cComboTableTimer,
			cComboTableTexture,
			cComboTableUnits,
			cComboTableModifier,
			cComboTableModulus,
			cComboTableMultiplier,
			cComboTableCount
		};

		//const std::string cUnitPrefix( "UNIT_TYPE_" );
	}

	void tGameSessionStats::fConfigureCombos( )
	{
		// Fill out combos table
		//clear mapping table
		mComboMapping.fFill( -1 );

		const tDataTable &comboData = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableCombos ).fIndexTable( cComboTable );
		mCombos.fSetCount( comboData.fRowCount( ) );

		for( u32 i = 0; i < mCombos.fCount( ); ++i )
		{
			tComboStatGroup& group = mCombos[ i ];
			group.mColor = comboData.fIndexByRowCol<tVec4f>( i, cComboTableColor ).fXYZ( );
			group.mScreenPos = comboData.fIndexByRowCol<tVec4f>( i, cComboTableScreenPos ).fXY( );
			group.mComboTimer = comboData.fIndexByRowCol<f32>( i, cComboTableTimer );
			group.mTexturePath = comboData.fIndexByRowCol<tStringPtr>( i, cComboTableTexture );

			// Fill out multiplier table
			group.mModifier = comboData.fIndexByRowCol<f32>( i, cComboTableModifier );
			group.mModulus = (u32)comboData.fIndexByRowCol<f32>( i, cComboTableModulus );
			
			// Parse list of multipliers other than modulus pattern
			std::string multiplierAnomolies( comboData.fIndexByRowCol<tStringPtr>( i, cComboTableMultiplier ).fCStr( ) );
			tGrowableArray< std::string > multsSplit;
			StringUtil::fSplit( multsSplit, multiplierAnomolies.c_str( ), "," );

			for( u32 s = 0; s < multsSplit.fCount( ); ++s )
			{
				u32 value = atoi( StringUtil::fEatWhiteSpace( StringUtil::fStripQuotes( multsSplit[s] ).c_str( ) ).c_str( ) );
				if( value > 0 ) group.mAdditionalMultipliers.fPushBack( value );
			}

			// now find out what units are associated to this combo
			tStringPtr units = comboData.fIndexByRowCol<tStringPtr>( i, cComboTableUnits );
			tGrowableArray< std::string > unitsSplit;
			StringUtil::fSplit( unitsSplit, StringUtil::fStripQuotes( std::string( units.fCStr( ) ) ).c_str( ), "," );
			for( u32 u = 0; u < unitsSplit.fCount( ); ++u )
			{
				u32 type = GameFlags::fUNIT_TYPEValueStringToEnum( tStringPtr( StringUtil::fEatWhiteSpace( unitsSplit[ u ] ) ) );
				if( type < GameFlags::cUNIT_TYPE_COUNT )
					mComboMapping[ type ] = i;
			}
		}
	}

	namespace
	{
		enum tEffectsCombosColumns
		{
			cEffectsComboMult,
			cEffectsComboEffectTurret,
			cEffectsComboEffectVehicle,
			cEffectsComboEffectInfantry,
			cEffectsComboEffectAir,
			cEffectsComboEffectBoss,
			cEffectsComboEffectDefault
		};
	}

	tComboData tGameSessionStats::fComputeCombo( tPlayer& killer, tUnitLogic& killedLogic, b32 incrementCombos )
	{
		tComboData data;

		if( killer.fTeam( ) == killedLogic.fTeam( ) )
			return data;

		u32 comboEnum = killedLogic.fOwnerEntity( )->fQueryEnumValueInherited( GameFlags::cENUM_COMBO );
		if( comboEnum != ~0 )
		{
			// object was tagged as a combo
			data.mComboExists = true;
			data.mSignificantCombo = true;
			data.mCurrentCombo = (u32)tGameApp::fInstance( ).fComboEnumValue( comboEnum );
			fSpawnComboText( data.mCurrentCombo, killedLogic.fOwnerEntity( ), tVec4f( 0.0f, 0.64f, 0.67f, 0.0f ), killer.fHasPowerPoolShown( ) ); // Change the color in the data table too
		}
		else 
		{
			// check combo groups
			s32 comboIndex = fComboIndex( killedLogic.fUnitType( ) );
			if( comboIndex > -1 )
			{
				data.mComboExists = true;

				if( !incrementCombos )
					return data;

				tComboStatGroup& combo = mCombos[ comboIndex ];

				// Increment will shoot the combo ui element from projected world pos to the final screen pos, mostly deprecated (hidden)
				tVec2f screenP; //we only need this if the combo is not active
				if( !combo.fActive( ) ) 
					screenP = mPlayer->fUser( )->fProjectToScreen( killedLogic.fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) ).fXY( );
				combo.fIncrement( killedLogic.fUnitAttributeDestroyedComboMeterValue( ), screenP );
				data.mCurrentCombo = combo.mCurrentCombo; //on reward when we see the combo text

				b32 continuousComboMode = tGameApp::fInstance( ).fCurrentLevelDemand( )->fContinuousCombo( );
				b32 comboReached = false;

				if( continuousComboMode )
				{
					comboReached = (data.mCurrentCombo % 5 == 0);
					data.mCurrentCombo /= 5;
					++data.mCurrentCombo;
					data.mSignificantCombo = true;
				}
				else
					comboReached = combo.fComputePowerUpChance( ).mA;

				if( comboReached )
				{
					// combo reached:					
					data.mSignificantCombo = true;

					fSpawnComboText( data.mCurrentCombo, killedLogic.fOwnerEntity( ), tVec4f( combo.mColor, 0.0f ), killer.fHasPowerPoolShown( ) );

					// spawn a special effect
					const tDataTable& effects = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableEffectsCombos ).fIndexTable( 0 );

					for( u32 i = 0; i < effects.fRowCount( ); ++i )
					{
						if( continuousComboMode || data.mCurrentCombo == (u32)effects.fIndexByRowCol<f32>( i, cEffectsComboMult ) )
						{
							u32 unitType = killedLogic.fUnitType( );
							u32 col = cEffectsComboEffectDefault;

							sigassert( cEffectsComboEffectDefault - cEffectsComboEffectTurret - 1 == GameFlags::cUNIT_TYPE_BOSS - GameFlags::cUNIT_TYPE_TURRET );

							if( unitType >= GameFlags::cUNIT_TYPE_TURRET && unitType <= GameFlags::cUNIT_TYPE_BOSS )
								col = unitType - GameFlags::cUNIT_TYPE_TURRET + cEffectsComboEffectTurret;

							tStringPtr effect = effects.fIndexByRowCol<tStringPtr>( i, col );
							if( effect.fExists( ) )
								tGameEffects::fInstance( ).fPlayEffect( killedLogic.fOwnerEntity( ), effect );
							break;
						}
					}
				}
			}
		}	

		return data;
	}

	void tGameSessionStats::fSpawnComboText( u32 combo, tEntity* entity, const tVec4f& color, b32 flyToCorner )
	{
		tVec3f textSpawn = entity->fObjectToWorld( ).fGetTranslation( ) + Math::tVec3f( 0.f, 3.75f, 0.f );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevelDemand( );
		if( !level->fDisableComboText( ) )
		{
			std::stringstream ss;
			ss << combo << "x";
			
			Gui::tWorldSpaceFloatingText* text = mPlayer->fSpawnText( ss.str( ).c_str( ), textSpawn, color, 1.25f, 2.25f, flyToCorner );
			if( text )
				text->fSetZOffset( -0.001f );
		}
	}

	b32 tGameSessionStats::fCheckStatBeatFriends( u32 statType, f32 newValue, f32 increment )
	{
		sigassert( !tGameApp::fInstance( ).fSceneGraph( )->fInMTRunList( ) );


		if( fIsBonusButNotSpecial( statType ) )
		{
			// wave chains and bonus are handled through a special case since the value is scenario dependent.
			tGameApp::fInstance( ).fCurrentLevelDemand( )->fExtraMiniGamePoints( fBonusValue( statType ) * increment );
		}

		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) && fRealTime( statType ) )
		{

			const tGrowableArray<tFriendStat>& friendsStats = mFriendStats[ statType ].mStats;
			u32& tryingToBeatIndex = mFriendStats[ statType ].mTryingToBeatIndex;
			if( !mPlayer->fPersonalBestUI( ).fNull() && ( friendsStats.fCount( ) && (tryingToBeatIndex != ~0 || ( mPlayer->fPersonalBestUI( )->fShown( ) ) ) ) )
			{
				b32 display = mPlayer->fPersonalBestUI( )->fShown( );
				f32 totalNewValue = newValue + mPlayer->fProfile( )->fAllTimeStat( statType ); //calculate what it would be if it were an all time stat

				// loop incase multiple people have the same stat.
				while( tryingToBeatIndex != ~0  )
				{
					f32 nextScore = friendsStats[ tryingToBeatIndex ].mStat;
					if( totalNewValue < nextScore )
						break;

					// we want to show this stat, todo add threshold so this shows up sooner, 0.5 the distance to the next person?
					display = true;

					// once we pass them, increment
					if( totalNewValue > nextScore )
						--tryingToBeatIndex; //meant to wrap at zero to ~0
					else
						break;
				}

				if( display )
				{
					//log_line( 0, "Best stat: " << GameFlags::fSESSION_STATSEnumToValueString( statType ) << " totalNewValue: " << totalNewValue );
					mPlayer->fPersonalBestUI( )->fNewPersonalBest( statType, totalNewValue );
					return true;
				}
			}
		}

		return false;
	}

	void tGameSessionStats::fUpdateLeaderBoards( )
	{
		fUpdateLevelStats( );
		fUpdateFriendStats( );
	}

	void tGameSessionStats::fUpdateLevelStats( )
	{
		if( !mLevelStatsDirty )
			return;

		for( u32 i = 0; i < mFriendLevelData.fCount( ); ++i )
		{
			if( mFriendLevelData[ i ] )
				mFriendLevelData[ i ]->fStepBoards( mPlayer );
		}
	}

	void tGameSessionStats::tFriendLevelData::fStepBoards( tPlayer* player )
	{
		if( mLeaderboard && mLeaderboard->fAdvanceRead( ) )
		{
			if( mReadingBoard < mBoards.fCount( ) )
			{
				mLeaderboard->fSelectBoard( mBoards[ mReadingBoard ] );

				sigassert( mLevelIndex < cMAX_LEVEL_COUNT );
				const GameSession::tLeaderBoardColumns& columnData = GameSession::cLevelLeaderBoardScoreColumnID[ mMapType ][ mLevelIndex ];
				if( columnData.mScore != ~0 )
				{
					const u32 rowCount = mLeaderboard->fRowsAvailable( );
					if( rowCount > 1 ) // We're in the leaderboard, thus 1 and not 0
					{
						const tPlatformUserId myId = player->fUser( )->fPlatformId( );

						for( u32 r = 0; r < rowCount; ++r )
						{
							// Don't count ourselves
							if( mLeaderboard->fRowUserId( r ) == myId )
								continue;

							tUserData scoreData;
							mLeaderboard->fFindResultById( columnData.mScore, r, scoreData );
							if( !scoreData.fIsSet( ) )
								continue;

							tLocalizedString gt;
							gt.fFromCStr( mLeaderboard->fRowGamerName( r ).c_str( ) );

							u32 scoreVal = (u32)scoreData.fGetS64( );
							if( mMapType == GameFlags::cMAP_TYPE_MINIGAME )
								scoreVal = (u32)fLeaderBoardToStat( GameFlags::cSESSION_STATS_MINIGAME_META_STAT, scoreVal );
							tFriendLevelScore score( gt, scoreVal );

							if( columnData.mMedal != ~0 )
							{
								tUserData medalData;
								mLeaderboard->fFindResultById( columnData.mMedal, r, medalData );
								if( medalData.fIsSet( ) )
									score.mOverallMedal = (u32)medalData.fGetS64( );
							}

							mScores[ mReadingBoard ].fPushBack( score );

							if( mReadingBoard == GameFlags::cDIFFICULTY_NORMAL || mMapType == GameFlags::cMAP_TYPE_MINIGAME )
							{
								// challenge progress is only set in normal (or minigames)
								tUserData challengeProgress;
								mLeaderboard->fFindResultById( columnData.mChallenge, r, challengeProgress );
								if( challengeProgress.fIsSet( ) )
									mChallengeProgresses.fPushBack( tFriendLevelScore(  gt, (u32)challengeProgress.fGetS64( ) ) );
							}
						}
					}
				}

				++mReadingBoard;
			}

			if( mReadingBoard >= mBoards.fCount( ) )
			{
				std::sort( mChallengeProgresses.fBegin( ), mChallengeProgresses.fEnd( ) );
				mLeaderboard.fRelease( );
			}
		}
	}

	tGameSessionStats::tFriendLevelScore* tGameSessionStats::tFriendLevelData::fGetChallengeProgresses( u32 index )
	{
		if( index < mChallengeProgresses.fCount( ) )
			return &mChallengeProgresses[ index ];
		else
			return NULL;
	}

	void tGameSessionStats::fFillFriendsStats( )
	{
		// Skip friends stats for remote users
		if( !mPlayer->fUser( )->fIsLocal( ) )
		{
			mStatsLeaderboard.fRelease( );
			mFriendStatsInitialized = true;
			return;
		}

		mFriendStatsInitialized = false;
		for( u32 s = 0; s < mFriendStats.fCount( ); ++s )
			mFriendStats[ s ] = tFriendStatAndIndex( );

		mStatsLeaderboard.fReset( NEW tLeaderboard( ) );
		mStatsLeaderboard->fAddBoardToRead( GameSession::cLeaderboardPlayerStats );
		mStatsLeaderboard->fAddBoardToRead( GameSession::cLeaderboardPlayerStats2 );
		mStatsLeaderboard->fSelectBoard( GameSession::cLeaderboardPlayerStats );
		mStatsLeaderboard->fRetainZeroRanks( true );
	}

	void tGameSessionStats::fUpdateFriendStats( )
	{
		if( mFriendStatsInitialized || !mStatsLeaderboard )
			return;

		// Kick the boards if they haven't started
		if( mStatsLeaderboard->fState( ) == tLeaderboard::cStateNull )
			mStatsLeaderboard->fReadByFriends( mPlayer->fUser( ).fGetRawPtr( ), 0 );

		if( !mStatsLeaderboard->fAdvanceRead( ) )
			return;

		mFriendStatsInitialized = true;
		const tPlatformUserId myId = mPlayer->fUser( )->fPlatformId( );

		// Helper instance used to sort a list of row indices by a particular column
		struct 
		{
			u32 columnId;
			tLeaderboard * board;
			bool operator( ) ( u32 a, u32 b ) 
			{ 
				tUserData aD, bD;
				board->fFindResultById( columnId, a, aD );
				board->fFindResultById( columnId, b, bD );

				if( aD.fIsSet( ) && !bD.fIsSet( ) )
					return true;
				else if( !aD.fIsSet( ) && bD.fIsSet( ) )
					return false;
				else if( !aD.fIsSet( ) && !bD.fIsSet( ) )
					return false;

				return aD.fGetS64( ) > bD.fGetS64( );
			}
		} mySorter;

		// Set the board
		mySorter.board = mStatsLeaderboard.fGetRawPtr( );

		// Fixed storage for holding the row indices
		tFixedArray<u32, tLeaderboard::cMaxRowsToRead + 1> sortedRows;

		for( ;; )
		{
			if( mStatsLeaderboard->fState( ) == tLeaderboard::cStateFail )
				break;

			const u32 rowCount = mStatsLeaderboard->fRowsAvailable( );
			if( rowCount <= 1 ) // We're in the leaderboard, thus 1 and not 0
				break;

			for( u32 s = 0; s < mFriendStats.fCount( ); ++s )
			{
				if( s == GameSession::cLeaderboardPlayerStats2StartID )
					mStatsLeaderboard->fSelectBoard( GameSession::cLeaderboardPlayerStats2 );

				if( fStatTemp( s ) )
					continue; // no stats expected exist

				u32 columnID = GameSession::cLeaderboardPlayerStatsColumnIdByStat[ s ];

				if( columnID == ~0 )
				{
					log_warning( 0, "No stat column for stat: " << GameFlags::fSESSION_STATSEnumToValueString( s ) );
					continue;
				}

				// Reset the rows in the sort container
				for( u32 r = 0; r < rowCount; ++r )
					sortedRows[ r ] = r;

				mySorter.columnId = columnID;
				std::sort( sortedRows.fBegin( ), sortedRows.fBegin( ) + rowCount, mySorter );

				const f32 myStatVal = mPlayer->fProfile( )->fAllTimeStat( s );

				for( u32 r = 0; r < rowCount; ++r )
				{
					const u32 rowIdx = sortedRows[ r ];

					// Don't count ourselves
					if( mStatsLeaderboard->fRowUserId( rowIdx ) == myId )
						continue;

					tUserData data;
					if( !mStatsLeaderboard->fFindResultById( mySorter.columnId, rowIdx, data ) )
						continue;

					// Null data types were filtered to the end of the sorted indices
					if( !data.fIsSet( ) )
						break;

					mFriendStats[ s ].mStats.fPushBack( tFriendStat( ) );

					tFriendStat & entry = mFriendStats[ s ].mStats.fBack( );

					std::string gamerTag = mStatsLeaderboard->fRowGamerName( rowIdx );
					entry.mGamerTag.fFromCStr( gamerTag.c_str( ) );
					if( s == GameFlags::cSESSION_STATS_SCORE )
						entry.mStat = (f32)data.fGetS64( );
					else
						entry.mStat = fLeaderBoardToStat( s, (u32)data.fGetS64( ) );

					// find our rank amongst friends
					if( entry.mStat >= myStatVal )
						mFriendStats[ s ].mTryingToBeatIndex = mFriendStats[ s ].mStats.fCount( ) - 1;
				}	

				//if( mFriendStats[ s ].mStats.fCount( ) && mFriendStats[ s ].mTryingToBeatIndex != ~0 )
				//{
				//	log_line( 0, "Try to beat." );
				//	log_line( 0, "Stat: " << GameFlags::fSESSION_STATSEnumToValueString( s ) );
				//	log_line( 0, "  Cnt: " << mFriendStats[ s ].mStats.fCount( ) );
				//	log_line( 0, "  Mine: " << myStatVal );
				//	log_line( 0, "  Beat: " << mFriendStats[ s ].mStats[ mFriendStats[ s ].mTryingToBeatIndex ].mStat );
				//}
			}

			break;
		}

		mStatsLeaderboard.fRelease( );
	}

	void tGameSessionStats::fGetStatRange( const tGameSessionStats::tFriendStatAndIndex& stat, u32 ideaCount, u32& startOut, u32& countOut )
	{
		if( stat.mTryingToBeatIndex == ~0 )
		{
			startOut = 0;
			countOut = fMin( stat.mStats.fCount( ), ideaCount );
		}
		else
		{
			startOut = (u32)fMax( 0, (s32)stat.mTryingToBeatIndex - (s32)ideaCount );
			countOut = fMin( (s32)stat.mStats.fCount( ) - (s32)startOut, (s32)ideaCount );
		}

		//log_line( 0, "Stat Range: Start: " << startOut << " Count: " << countOut << " array: " << stat.mStats.fCount( ) );
	}
	
	u32 tGameSessionStats::fGetFirstRelativeFriend( u32 stat, u32 ideaCount ) const
	{
		u32 start, count;
		fGetStatRange( mFriendStats[ stat ], ideaCount, start, count );
		return start;
	}

	u32 tGameSessionStats::fGetRelativeFriendCount( u32 stat, u32 ideaCount ) const
	{
		u32 start, count;
		fGetStatRange( mFriendStats[ stat ], ideaCount, start, count );
		return count;
	}

	b32 tGameSessionStats::fOverChargeAvailable( tUnitLogic* currrentUnit ) const 
	{ 
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevelDemand( );
		if( !currrentUnit || currrentUnit->fUnitType( ) != GameFlags::cUNIT_TYPE_TURRET || mOverChargeComboCost == 0.f )
			return false;
		else
			return (!level->fDisableOvercharge( ) && !level->fForceWeaponOvercharge( ) && mComboMeterValue >= 1.f/* && !fBarrageAvailable( currrentUnit )*/ );
	}

	b32 tGameSessionStats::fBarrageAvailable( tUnitLogic* currrentUnit ) const 
	{ 
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevelDemand( );
		
		if( !currrentUnit || currrentUnit->fUnitType( ) != GameFlags::cUNIT_TYPE_TURRET || mOverChargeComboCost == 0.f )
			return false;
		else
			return (!level->fDisableBarrage( ) && fBarrageMeterPercent( ) >= 1.f );
	}

	namespace 
	{
		enum tSessionStatColumns
		{
			cSessionStatMoreIsBetter,
			cSessionStatDisplayType,
			cSessionStatLocalizedUnitStringID,
			cSessionStatPriority,
			cSessionStatRealTime,
			cSessionStatTemp,
			cSessionStatMinLeaderBoard,
			cSessionStatMaxLeaderBoard,
			cSessionStatThreshold,
			cSessionStatBonus,
			cSessionStatDisplayColor,
			cSessionStatBonusDisplayLocId,
			cSessionStatBonusValue
		};

		enum tDisplayType
		{
			cDisplayTypeInteger,
			cDisplayTypeFloat,
			cDisplayTypeTime,
			cDisplayTypeMoney,
			cDisplayTypeCombo,
			cDisplayTypeCount,
		};
	}

	b32 tGameSessionStats::fMoreIsBetter( u32 statType )
	{ 
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );		
		return (b32)dt.fIndexByRowCol<f32>( statType, cSessionStatMoreIsBetter ); 
	}

	b32 tGameSessionStats::fStatTemp( u32 statType )
	{ 
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );	
		return (b32)dt.fIndexByRowCol<f32>( statType, cSessionStatTemp ); 
	}

	const tLocalizedString& tGameSessionStats::fStatLocName( u32 statType )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );	
		return tGameApp::fInstance( ).fLocString( dt.fRowName( statType ) );
	}

	const tLocalizedString& tGameSessionStats::fBonusDisplayLocString( u32 statType )
	{
		tStringPtr bonusString = fBonusDisplayLocID( statType );
		if( fIsBonus( statType ) && bonusString.fExists( ) )
			return tGameApp::fInstance( ).fLocString( bonusString );
		else
			return tLocalizedString::fNullString( );
	}

	tStringPtr tGameSessionStats::fBonusDisplayLocID( u32 statType )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );
		return (tStringPtr)dt.fIndexByRowCol<tStringPtr>( statType, cSessionStatBonusDisplayLocId );
	}

	tLocalizedString tGameSessionStats::fStatLocValueString( f32 value, u32 statType )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );	

		tLocalizedString retString;

		tDisplayType displayType = (tDisplayType)dt.fIndexByRowCol<u32>( statType, cSessionStatDisplayType );
		tStringPtr unitStringLocID = (tStringPtr)dt.fIndexByRowCol<tStringPtr>( statType, cSessionStatLocalizedUnitStringID );
		
		switch( displayType )
		{
		case cDisplayTypeInteger:
			retString = tLocalizedString::fLocalizeNumber( fRound< s32 >( value ) );
			break;
		case cDisplayTypeFloat:
			retString = tLocalizedString::fLocalizeNumber( value );
			break;
		case cDisplayTypeTime:
			retString = tLocalizedString::fConstructTimeString( value, true );
			break;
		case cDisplayTypeMoney:
			retString = tLocalizedString::fConstructMoneyString( StringUtil::fToString( (u32)value ).c_str( ) );
			break;
		case cDisplayTypeCombo:
			std::stringstream ss;
			ss << (u32)value << "x";
			retString = tLocalizedString::fFromCString( ss.str( ).c_str( ) );
			break;
		}

		if( unitStringLocID.fExists( ) )
		{
			retString.fJoinWithCString( " " );
			retString.fJoinWithLocString( tGameApp::fInstance( ).fLocString( unitStringLocID ) );
		}

		return retString;
	}

	u32 tGameSessionStats::fPriority( u32 statType )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );	

		return dt.fIndexByRowCol<u32>( statType, cSessionStatPriority );
	}

	b32 tGameSessionStats::fIsBonus( u32 statType )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );	

		return (b32)dt.fIndexByRowCol<f32>( statType, cSessionStatBonus );
	}

	// These will be haneded special case.
	b32 tGameSessionStats::fIsBonusButNotSpecial( u32 statType )
	{
		return (statType != GameFlags::cSESSION_STATS_WAVE_CHAIN
			&& statType != GameFlags::cSESSION_STATS_WAVE_BONUS
			&& statType != GameFlags::cSESSION_STATS_SPEED_BONUS
			&& statType != GameFlags::cSESSION_STATS_BOMBING_RUN
			&& fIsBonus( statType ) );
	}

	f32 tGameSessionStats::fThreshold( u32 statType )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );	

		return dt.fIndexByRowCol<f32>( statType, cSessionStatThreshold );
	}

	u32 tGameSessionStats::fBonusValue( u32 statType )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		if( statType > dt.fRowCount( ) )
		{
			log_warning( 0, "Session stat index higher than row count in data table!" );
		}

		return dt.fIndexByRowCol<u32>( statType, cSessionStatBonusValue );
	}

	Math::tVec4f tGameSessionStats::fDisplayColor( u32 statType )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );	

		return dt.fIndexByRowCol<Math::tVec4f>( statType, cSessionStatDisplayColor );
	}

	b32 tGameSessionStats::fRealTime( u32 statType )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );	

		return (b32)dt.fIndexByRowCol<f32>( statType, cSessionStatRealTime );
	}

	u32 tGameSessionStats::fStatToLeaderBoard( u32 statType, f32 value )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );	

		u32 displayType = dt.fIndexByRowCol<u32>( statType, cSessionStatDisplayType );
		if( displayType == cDisplayTypeInteger || displayType == cDisplayTypeMoney || displayType == cDisplayTypeCombo )
			return (u32)value;

		f32 min = dt.fIndexByRowCol<f32>( statType, cSessionStatMinLeaderBoard );
		f32 max = dt.fIndexByRowCol<f32>( statType, cSessionStatMaxLeaderBoard );
		f32 range = max - min;
		
		if( value < min )
		{
			log_warning( 0, "Stat min truncated: " << GameFlags::fSESSION_STATSEnumToValueString( statType ) << " - " << value );
			value = min;
		}
		else if( value > max )
		{
			log_warning( 0, "Stat max truncated: " << GameFlags::fSESSION_STATSEnumToValueString( statType ) << " - " << value );
			value = max;
		}

		f32 zeroToOne = (value - min) / range;
		u32 lb = u32( zeroToOne * std::numeric_limits<u32>( ).max( ) );

		log_line( 0, "s to lb: " << value << " " << lb );
		return lb;
	}

	f32 tGameSessionStats::fLeaderBoardToStat( u32 statType, u32 leaderboardVal )
	{
		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableSessionStats ).fIndexTable( 0 );
		sigassert( statType <= dt.fRowCount( ) && "Session stat index higher than row count in data table!" );	

		u32 displayType = dt.fIndexByRowCol<u32>( statType, cSessionStatDisplayType );
		if( displayType == cDisplayTypeInteger || displayType == cDisplayTypeMoney || displayType == cDisplayTypeCombo )
			return (f32)leaderboardVal;

		f32 min = dt.fIndexByRowCol<f32>( statType, cSessionStatMinLeaderBoard );
		f32 max = dt.fIndexByRowCol<f32>( statType, cSessionStatMaxLeaderBoard );
		f32 range = max - min;

		f32 zeroToOne = leaderboardVal / (f32)std::numeric_limits<u32>( ).max( );
		f32 stat = min + range * zeroToOne;

		if( displayType == cDisplayTypeInteger )
			stat = fRound<f32>( stat );
		return stat;
	}

	void tGameSessionStats::fClearLevelData( )
	{
		mFriendLevelData.fSetCount( 0 );
		mLevelStatsDirty = false;
	}

	tGameSessionStats::tFriendLevelData* tGameSessionStats::fRequestLevelData( u32 mapType, u32 levelIndex )
	{
		if( levelIndex >= mFriendLevelData.fCount( ) )
			mFriendLevelData.fSetCount( levelIndex + 1 );

		const tGameAppSession& gameAppSession = *tGameApp::fInstance( ).fGameAppSession( );

		tFriendLevelDataPtr& ptr = mFriendLevelData[ levelIndex ];
		if( !ptr )
		{
			mLevelStatsDirty = false;

			tLeaderboardPtr lb( NEW tLeaderboard( ) );
			tGrowableArray< u32 > boards;
			u32 modes = 1;

			switch( mapType )
			{
			case GameFlags::cMAP_TYPE_CAMPAIGN:
				{
					modes = GameFlags::cDIFFICULTY_COUNT;
				}
				break;
			case GameFlags::cMAP_TYPE_SURVIVAL:
				{
					modes = GameFlags::cCHALLENGE_MODE_COUNT;
				}
				break;
			}

			for( u32 i = 0; i < modes; ++i )
			{
				u32 board = gameAppSession.fGetLevelLeaderboardId( mapType, levelIndex, i );
				lb->fAddBoardToRead( board );
				boards.fPushBack( board );
			}

			lb->fReadByFriends( mPlayer->fUser( ).fGetRawPtr( ), 0 );
			lb->fRetainZeroRanks( true );
			ptr.fReset( new tFriendLevelData( lb, boards, mapType, levelIndex ) );
			mLevelStatsDirty = true;
		}

		return ptr.fGetRawPtr( );
	}
}

namespace Sig
{
	namespace
	{

		// this class will have the same interface as a game session stats, to show stats for an arbtirary user
		class tArbitraryStatReader
		{
			tLocalizedString mGamerTag;
			tLeaderboardPtr mStatsLeaderboard;
			tFixedArray< f32, GameFlags::cSESSION_STATS_COUNT > mStats;

		public:
			tArbitraryStatReader( )
			{
				mStats.fFill( 0 );
			}

			void fCreate( tScript64 userId )
			{
				mStatsLeaderboard.fReset( NEW tLeaderboard( ) );
				mStatsLeaderboard->fAddBoardToRead( GameSession::cLeaderboardPlayerStats );
				mStatsLeaderboard->fAddBoardToRead( GameSession::cLeaderboardPlayerStats2 );
				mStatsLeaderboard->fSelectBoard( GameSession::cLeaderboardPlayerStats );
				mStatsLeaderboard->fRetainZeroRanks( true );
				
				tPlatformUserId ids[ 1 ];
				ids[ 0 ] = tPlatformUserId( userId.fGet( ) );
				mStatsLeaderboard->fReadByPlatformId( ids, 1 );
			}

			f32 fStat( u32 id ) 
			{ 
				return mStats[ id ]; 
			}

			b32 fStatWasSet( u32 id )
			{
				return (mStats[ id ] != 0.f && !tGameSessionStats::fStatTemp( id ));
			}

			// returns true when finished
			b32 fUpdate( )
			{
				if( mStatsLeaderboard )
				{
					if( mStatsLeaderboard->fAdvanceRead( ) 
						&& mStatsLeaderboard->fState( ) != tLeaderboard::cStateFail 
						&& mStatsLeaderboard->fRowsAvailable( ) )
					{
						mGamerTag.fFromCStr( mStatsLeaderboard->fRowGamerName( 0 ).c_str( )  );

						for( u32 s = 0; s < mStats.fCount( ); ++s )
						{
							if( s == GameSession::cLeaderboardPlayerStats2StartID )
								mStatsLeaderboard->fSelectBoard( GameSession::cLeaderboardPlayerStats2 );

							if( tGameSessionStats::fStatTemp( s ) )
								continue; // no stats expected exist

							u32 columnID = GameSession::cLeaderboardPlayerStatsColumnIdByStat[ s ];

							if( columnID == ~0 )
							{
								log_warning( 0, "No stat column for stat: " << GameFlags::fSESSION_STATSEnumToValueString( s ) );
								continue;
							}

							tUserData data;
							mStatsLeaderboard->fFindResultById( columnID, 0, data );

							if( data.fIsSet( ) )
							{
								if( s != GameFlags::cSESSION_STATS_SCORE )
									mStats[ s ] = tGameSessionStats::fLeaderBoardToStat( s, (u32)data.fGetS64( ) );
								else
									mStats[ s ] = (f32)data.fGetS64( );
							}
						}

						mStatsLeaderboard.fRelease( );
					}
				}

				return !mStatsLeaderboard;
			}
		};
	}

	void tGameSessionStats::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tArbitraryStatReader, Sqrat::DefaultAllocator<tArbitraryStatReader> > classDesc( vm.fSq( ) );

			classDesc
				.Func( _SC( "Create" ),						&tArbitraryStatReader::fCreate )
				.Func( _SC( "Update" ),						&tArbitraryStatReader::fUpdate )
				.Func( _SC( "Stat" ),						&tArbitraryStatReader::fStat )
				.Func( _SC( "StatWasSet" ),					&tArbitraryStatReader::fStatWasSet )
				;

			vm.fRootTable( ).Bind( _SC("ArbitraryStatReader"), classDesc );
		}
		{
			Sqrat::Class<tFriendStat, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

			classDesc
				.Var( _SC( "GamerTag" ),	&tFriendStat::mGamerTag )
				.Var( _SC( "Stat" ),		&tFriendStat::mStat)
				;

			vm.fRootTable( ).Bind( _SC("FriendStat"), classDesc );
		}
		{
			Sqrat::Class<tFriendLevelScore, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

			classDesc
				.Var( _SC( "GamerTag" ),		&tFriendLevelScore::mGamerTag )
				.Var( _SC( "Score" ),			&tFriendLevelScore::mScore )
				.Var( _SC( "OverallMedal" ),	&tFriendLevelScore::mOverallMedal )
				;

			vm.fRootTable( ).Bind( _SC("FriendLevelScore"), classDesc );
		}
		{
			Sqrat::Class<tFriendLevelData, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

			classDesc
				.Prop( _SC( "Initialized" ),	&tFriendLevelData::fInitialized )
				.Func( _SC( "ScoreCount" ),		&tFriendLevelData::fScoreCount)
				.Func( _SC( "GetScore" ),		&tFriendLevelData::fGetScore)
				.Func( _SC( "ChallengeProgressesCount" ),	&tFriendLevelData::fChallengeProgressesCount)
				.Func( _SC( "GetChallengeProgresses" ),		&tFriendLevelData::fGetChallengeProgresses)
				;

			vm.fRootTable( ).Bind( _SC("FriendLevelData"), classDesc );
		}
		{
			Sqrat::Class<tGameSessionStats, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

			classDesc
				.Func( _SC( "IncStat" ),					&tGameSessionStats::fIncStat )
				.Func( _SC( "Stat" ),						&tGameSessionStats::fStatScript )
				.Func( _SC( "StatWasSet" ),					&tGameSessionStats::fStatWasSet )
				.StaticFunc( _SC( "MoreIsBetter" ),			&tGameSessionStats::fMoreIsBetter )
				.StaticFunc( _SC( "StatLocName" ),			&tGameSessionStats::fStatLocName )
				.StaticFunc( _SC( "StatLocValueString" ),	&tGameSessionStats::fStatLocValueString )
				.StaticFunc( _SC( "Priority" ),				&tGameSessionStats::fPriority )
				.StaticFunc( _SC( "StatTemp" ),				&tGameSessionStats::fStatTemp )
				.StaticFunc( _SC( "RealTime" ),				&tGameSessionStats::fRealTime )
				.StaticFunc( _SC( "StatToLeaderBoard" ),	&tGameSessionStats::fStatToLeaderBoard )
				.StaticFunc( _SC( "LeaderBoardToStat" ),	&tGameSessionStats::fLeaderBoardToStat )
				.StaticFunc( _SC( "IsBonus" ),				&tGameSessionStats::fIsBonus )
				.StaticFunc( _SC( "DisplayColor" ),			&tGameSessionStats::fDisplayColor )
				.StaticFunc( _SC( "DisplayLocID" ),			&tGameSessionStats::fBonusDisplayLocID )
				.StaticFunc( _SC( "DisplayLocString" ),		&tGameSessionStats::fBonusDisplayLocString )
				.StaticFunc( _SC( "BonusValue" ),			&tGameSessionStats::fBonusValue )
				.StaticFunc( _SC( "Threshold" ),			&tGameSessionStats::fThreshold )
				.Prop(_SC("FriendDataInitialized"), 		&tGameSessionStats::fFriendDataInitialized )
				.Func(_SC("FriendStatCount"),				&tGameSessionStats::fFriendStatCount )
				.Func(_SC("FriendStat"),					&tGameSessionStats::fFriendStat )
				.Func(_SC("TryingToBeatFriendIndex"),		&tGameSessionStats::fTryingToBeatFriendIndex )
				.Func(_SC("GetFirstRelativeFriend"),		&tGameSessionStats::fGetFirstRelativeFriend )
				.Func(_SC("GetRelativeFriendCount"),		&tGameSessionStats::fGetRelativeFriendCount )
				.Func(_SC("ClearLevelData"),				&tGameSessionStats::fClearLevelData )
				.Func(_SC("RequestLevelData"),				&tGameSessionStats::fRequestLevelData )
				;

			vm.fRootTable( ).Bind( _SC("GameSessionStats"), classDesc );
		}
	}

}
