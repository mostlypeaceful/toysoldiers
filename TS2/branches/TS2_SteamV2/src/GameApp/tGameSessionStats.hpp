#ifndef __tGameSessionStats__
#define __tGameSessionStats__

#include "tShapeEntity.hpp"
#include "tLocalizationFile.hpp"
#include "tLeaderBoard.hpp"
#include "tSync.hpp"

namespace Sig
{
	class tPlayer;
	class tUnitLogic;
	
	struct tComboData
	{
		b32 mComboExists; //false if you killed a bush
		b32 mSignificantCombo; //like 5 x, 30x, etc
		u32 mCurrentCombo;

		tComboData( )
			: mComboExists( false )
			, mSignificantCombo( false )
			, mCurrentCombo( 0 ) //preincremented becasue 1x may be a significant combo
		{ }
	};

	struct tComboStatGroup
	{
		// These values are loaded from a table
		Math::tVec3f mColor;
		Math::tVec2f mScreenPos;
		f32          mComboTimer;
		tStringPtr   mTexturePath;
		// See the powerup design doc for how these work
		f32			 mModifier;
		u32			 mModulus; //to establish a pattern of important multipliers, such as every 5x
		tGrowableArray< u32 > mAdditionalMultipliers; //additional powerup chances, not in modulus pattern


		// "Live" values
		u32 mCurrentCombo;
		f32 mComboTimeRemaining;
		b32 mRestart; //time remaining was zero when increment called
		Math::tVec2f mRestartPt;

		tComboStatGroup( const Math::tVec3f& color = Math::tVec3f::cOnesVector, f32 comboTimer = 10.0f, const tStringPtr& texturePath = tStringPtr( ) );

		// Call this when someone dies
		void fIncrement( u32 count, const Math::tVec2f& screenP );
		void fStepTimer( tPlayer* player, f32 dt );
		b32  fActive( ) const { return mComboTimeRemaining > 0.0f; }

		void fReset( ) { mComboTimeRemaining = 0.f; mCurrentCombo = 0; }

		//<Chance Exists/Show Multiplier, ChancePercentatge>
		tPair<b32, f32> fComputePowerUpChance( ) const;
	};

	class tGameSessionStats : public tRefCounter
	{
	public:
		// the percent beyond 100 that the barrage is full
		static const f32 cBarrageExtraPercent;

	public:
		tGameSessionStats( tPlayer* player );
		virtual ~tGameSessionStats( );

		void fUpdate( f32 dt );
		void fUpdateLeaderBoards( );

		inline f32  fStat( u32 statType ) const { return mStats[ statType ]; }
		inline b32  fStatWasSet( u32 statType ) const { return mStatWasSet[ statType ]; }
		inline void fSetLockIncrements( b32 lock ) { mLockIncrements = lock; }

		inline void fSetStat( u32 statType, f32 value ) 
		{
			sync_event_c( "(stat,val)", Math::tVec2f( (f32)statType, value ), tSync::cSCStats );
			mStats[ statType ] = value; 
			mStatWasSet[ statType ] = true;
		}
		inline void fIncStat( u32 statType, f32 inc ) 
		{ 
			sync_event_c( "(stat,inc)", Math::tVec2f( (f32)statType, inc ), tSync::cSCStats );

			if( !mLockIncrements )
			{
				f32 newVal = mStats[ statType ] + inc;
				mStats[ statType ] = newVal; 
				mStatWasSet[ statType ] = true;
				fCheckStatBeatFriends( statType, newVal, inc );
			}
		}
		inline void fIncQuick( u32 statType, f32 inc ) 
		{ 
			sync_event_c( "(stat,inc)", Math::tVec2f( (f32)statType, inc ), tSync::cSCStats );
			if( !mLockIncrements )
			{
				mStats[ statType ] += inc; 
				mStatWasSet[ statType ] = true;
			}
		}
		inline void fMaxStat( u32 statType, f32 value ) 
		{
			sync_event_c( "(stat,val)", Math::tVec2f( (f32)statType, value ), tSync::cSCStats );
			if( !mLockIncrements )
			{
				f32 newVal = fMax( mStats[ statType ], value );
				f32 inc = newVal - mStats[ statType ];
				if( inc > 0.f )
				{
					mStats[ statType ] = newVal; 
					mStatWasSet[ statType ] = true;
					fCheckStatBeatFriends( statType, newVal, inc );
				}
			}
		}
		static b32 fMoreIsBetter( u32 statType );
		static b32 fStatTemp( u32 statType );
		static const tLocalizedString& fStatLocName( u32 statType );
		static tLocalizedString fStatLocValueString( f32 value, u32 statType );
		static u32 fPriority( u32 statType );
		static b32 fRealTime( u32 statType );
		static u32 fStatToLeaderBoard( u32 statType, f32 value );
		static f32 fLeaderBoardToStat( u32 statType, u32 leaderboardVal );
		static b32 fIsBonus( u32 statType );
		static b32 fIsBonusButNotSpecial( u32 statType );
		static f32 fThreshold( u32 statType );
		static Math::tVec4f fDisplayColor( u32 statType );
		static tStringPtr tGameSessionStats::fBonusDisplayLocID( u32 statType );
		static const tLocalizedString& tGameSessionStats::fBonusDisplayLocString( u32 statType );
		static u32 tGameSessionStats::fBonusValue( u32 statType );

		tGrowableArray<tComboStatGroup>& fComboGroups( ) { return mCombos; }

		void fSetComboMeter( f32 value ) { mComboMeterValue = fClamp( value, 0.f, 1.f + cBarrageExtraPercent ); }
		void fResetComboMeter( );
		void fSetOverChargeComboCost( f32 cost ) { mOverChargeComboCost = cost; }
		f32  fOverChargeComboCost( ) const { return mOverChargeComboCost; }
		b32  fOverChargeAvailable( tUnitLogic* currrentUnit ) const;
		b32  fBarrageAvailable( tUnitLogic* currentUnit ) const;

		b32 fCheckStatBeatFriends( u32 statType, f32 newValue, f32 increment );
		tComboData fComputeCombo( tPlayer& killer, tUnitLogic& killedLogic, b32 incrementCombos );
		u32 fCurrentCombo( ) const { return mCombos[ 0 ].mCurrentCombo; }
		f32  fComboMeterValue( ) const { return mComboMeterValue; }
		f32  fComboMeterTimerPercentage( ) const { return mComboTimerPercentage; }
		f32 fBarrageMeterPercent( ) const { return fMax( 0.f, (mComboMeterValue - 1.f) / cBarrageExtraPercent ); }

		void fPauseTimers( b32 pause ) { mPauseTimers = pause; }

		const tFixedArray< f32, GameFlags::cSESSION_STATS_COUNT >& fStats( ) const { return mStats; }
		void fSetStats( const tFixedArray< f32, GameFlags::cSESSION_STATS_COUNT >& stats ) { mStats = stats; }

		void fFillFriendsStats( );
		void fSpawnComboText( u32 combo, tEntity* entity, const Math::tVec4f& color, b32 flyToCorner = false );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
		f32  fStatScript( u32 statType ) const { return fStat( statType ); }


	private:

		struct tFriendStat
		{
			tLocalizedString mGamerTag;
			f32 mStat;
		};

		struct tFriendStatAndIndex
		{
			u32 mTryingToBeatIndex;
			tGrowableArray<tFriendStat> mStats;

			tFriendStatAndIndex( )
				: mTryingToBeatIndex( ~0 )
			{ }
		};

		struct tFriendLevelScore
		{
			tFriendLevelScore( tLocalizedString& gt = tLocalizedString( ), u32 score = ~0 )
				: mGamerTag( gt )
				, mScore( score )
				, mOverallMedal( 0 )
			{ }

			tLocalizedString mGamerTag;
			u32 mScore;
			u32 mOverallMedal;

			b32 operator < ( const tFriendLevelScore& right ) const
			{
				return mScore < right.mScore;
			}
		};

		struct tFriendLevelData : public tRefCounter
		{
			tLeaderboardPtr mLeaderboard;
			tGrowableArray< u32 > mBoards;
			u32 mReadingBoard;
			u32 mMapType;
			u32 mLevelIndex;
			tFixedArray< tGrowableArray< tFriendLevelScore >, GameFlags::cDIFFICULTY_COUNT > mScores;
			tGrowableArray< tFriendLevelScore > mChallengeProgresses;

			tFriendLevelData( const tLeaderboardPtr& lb, const tGrowableArray< u32 >& boards, u32 mapType, u32 levelIndex ) 
				: mReadingBoard( 0 )
				, mLeaderboard( lb )
				, mBoards( boards ) 
				, mMapType( mapType )
				, mLevelIndex( levelIndex )
			{ }

			void fStepBoards( tPlayer* player );

			b32 fInitialized( ) const { return !mLeaderboard; }
			u32 fScoreCount( u32 difficulty ) { return mScores[ difficulty ].fCount( ); }
			tFriendLevelScore* fGetScore( u32 difficulty, u32 index ) { return &mScores[ difficulty ][ index ]; }

			u32 fChallengeProgressesCount( ) { return mChallengeProgresses.fCount( ); }
			tFriendLevelScore* fGetChallengeProgresses( u32 index );
		};

		typedef tRefCounterPtr< tFriendLevelData > tFriendLevelDataPtr;
		
	private:
		void fConfigureCombos( );
		s32  fComboIndex( u32 type ) const;
		void fFillStat( u32 stat, tLeaderboard* friendBoard, tLeaderboard* xblaBoard );

		b32 fFriendDataInitialized( ) const { return mFriendStatsInitialized; }
		u32 fFriendStatCount( u32 stat ) const { return mFriendStats[stat].mStats.fCount( ); }
		tFriendStat * fFriendStat( u32 stat, u32 index ) { return &mFriendStats[stat].mStats[index]; }
		u32 fTryingToBeatFriendIndex( u32 stat ) { return mFriendStats[stat].mTryingToBeatIndex; }
		u32 fGetFirstRelativeFriend( u32 stat, u32 ideaCount ) const; //returns the first friend stat to show in comparison to our stat.
		u32 fGetRelativeFriendCount( u32 stat, u32 ideaCount ) const; //returns the count of friends to show in comparison to our stat.
		static void fGetStatRange( const tGameSessionStats::tFriendStatAndIndex& stat, u32 ideaCount, u32& startOut, u32& countOut );


		void fClearLevelData( );
		tFriendLevelData* fRequestLevelData( u32 mapType, u32 levelIndex );

		void fUpdateLevelStats( );
		void fUpdateFriendStats( );

	private:
		tPlayer*	mPlayer;
		tFixedArray< f32, GameFlags::cSESSION_STATS_COUNT > mStats;
		tFixedArray< b32, GameFlags::cSESSION_STATS_COUNT > mStatWasSet;			//Set true if we've beat our personal best

		tFixedArray< tFriendStatAndIndex, GameFlags::cSESSION_STATS_COUNT > mFriendStats;
		tLeaderboardPtr mStatsLeaderboard;

		tGrowableArray< tFriendLevelDataPtr > mFriendLevelData;

		tGrowableArray<tComboStatGroup> mCombos;
		tFixedArray<s32, GameFlags::cUNIT_TYPE_COUNT> mComboMapping;

		// Power pool
		f32 mComboMeterValue;
		f32 mOverChargeComboCost;
		f32 mComboTimerPercentage; //counts down from 1 to 0 representing combo timer

		b8 mPauseTimers; // user is reloading or cinematics or something, dont decrement combo timers
		b8 mFriendStatsInitialized;
		b8 mLevelStatsDirty;
		b8 mLockIncrements; //after a match has ended
	};

	typedef tRefCounterPtr< tGameSessionStats > tGameSessionStatsPtr;

}

#endif//__tGameSessionStats__

