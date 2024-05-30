#ifndef __tSaveGame__
#define __tSaveGame__
#include "tGameArchive.hpp"
#include "GameArchiveString.hpp"
#include "Gfx/tCamera.hpp"
#include "tPlayer.hpp"
#include "tGameAppBase.hpp"
#include "tLevelLoadInfo.hpp"

namespace Sig
{
	namespace { static const u32 cSaveDataVersion = 3; }

	struct tPlayerSaveData
	{
		tPlayerSaveData( )
			: mMoney( 0 )
			, mTickets( 0 )
			, mBarrage( -1 )
		{ 
		}

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mMoney );
			archive.fSaveLoad( mTickets );
			archive.fSaveLoad( mCamera );
			archive.fSaveLoad( mSkippableSeconds );
			archive.fSaveLoad( mStats );
			archive.fSaveLoad( mWaveKills );
			archive.fSaveLoad( mSpeedKills );
			archive.fSaveLoad( mBombingRuns );
			archive.fSaveLoad( mBarrage );
		}

		u32				mMoney;
		u32				mTickets;
		Gfx::tTripod	mCamera;
		f32				mSkippableSeconds;
		tPlayer::tWaveKillList	mWaveKills;
		tPlayer::tSpeedKillList mSpeedKills;
		tPlayer::tBombingRunList mBombingRuns;
		tFixedArray< f32, GameFlags::cSESSION_STATS_COUNT > mStats;
		s32				mBarrage;
	};

	struct tLevelSaveData
	{
		enum tGameModeMode
		{
			cGameModeSinglePlayer = 1,
			cGameModeCoOp,
		};

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mLoadInfo );
			archive.fSaveLoad( mRewindCount );
			archive.fSaveLoad( mPlatformPrice );
			archive.fSaveLoad( mEntsFromLevel );
			archive.fSaveLoad( mEntsDynamic );
		}

		tLevelLoadInfo mLoadInfo;

		u32 mRewindCount;
		f32 mPlatformPrice;

		tGrowableArray< tRefCounterPtr< tEntitySaveData > > mEntsFromLevel;
		tGrowableArray< tRefCounterPtr< tEntitySaveData > > mEntsDynamic;
	};

	// The rewind preview is the smallest amount of data possible we can keep in the game to enumerate the rewind screen.
	struct tTurretRewindPreview
	{
		u32				mUnitID;
		Math::tVec2f	mPosition;
		u32				mCountry;

		tTurretRewindPreview( u32 uid = ~0, const Math::tVec2f& pos = Math::tVec2f::cZeroVector, u32 country = GameFlags::cCOUNTRY_USA )
			: mUnitID( uid ), mPosition( pos ), mCountry( country )
		{ }
	};

	struct tSaveGameRewindPreview : public tRefCounter
	{
		u32 mMoney;
		u32 mMoneyPlayer2;
		u32 mTickets;
		
		tGrowableArray<tTurretRewindPreview> mTurrets;

		// script stuff
		u32 fTurretCount( ) const { return mTurrets.fCount( ); }
		tTurretRewindPreview* fTurret( u32 index ) { return &mTurrets[ index ]; }

		static void fExportScriptInterface( tScriptVm& vm );
	};

	typedef tRefCounterPtr<tSaveGameRewindPreview> tSaveGameRewindPreviewPtr;

	struct tSaveGameData
	{
		struct tWaveID
		{
			tStringPtr  mWaveName;
			s32			mWaveID; // total wave table index, dont use u32 because this gets converted to a string number.
			s32			mWaveIndex; //UI Index
			b32			mActive;
			u32			mCountry;
			b32			mIsLooping;

			tWaveID( const tStringPtr& waveName = tStringPtr::cNullPtr
				, s32 waveID = -1
				, s32 waveIndex = -1
				, b32 active = false
				, u32 country = GameFlags::cCOUNTRY_USSR
				, u32 looping = false )
				: mWaveName( waveName )
				, mWaveID( waveID )
				, mWaveIndex( waveIndex )
				, mActive( active )
				, mCountry( country )
				, mIsLooping( false )
			{ }

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				archive.fSaveLoad( mWaveName );
				archive.fSaveLoad( mWaveID );
				archive.fSaveLoad( mActive );
				archive.fSaveLoad( mCountry );
				archive.fSaveLoad( mIsLooping );
			}
		};


		static std::string fGenerateFileName( u32 waveIndex, b32 multiPlayer ) 
		{
			std::string res( "temp_save_" );
			if( multiPlayer )
				res += "mp_";
			res += "_" + StringUtil::fToString( waveIndex );
			res += ".bin";

			return res;
		}

		tSaveGameData( )
			: mVersion( cSaveDataVersion ) 
			, mId( 0 )
		{ }

		b32 fVersionOutOfDate( ) const { return mVersion != cSaveDataVersion; }

		u32 mVersion;
		u32 mId;
		tFixedArray< tPlayerSaveData, 2 > mPlayerData;
		tLevelSaveData mLevelData;
		tGrowableArray<tWaveID> mWaveLists;
		tGrowableArray<tWaveID> mOffensiveWaveLists;

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mVersion );

			if( archive.fMode( ) == tGameArchive::cModeLoad )
			{
				// check version before going any further - if it doesn't match the current version, then abort
				if( fVersionOutOfDate( ) )
				{
					log_warning( 0, "cSaveDataVersion doesn't match tSaveGameData::mVersion - aborting load bcz this probably means the save data format has changed." );
					return;
				}
			}

			archive.fSaveLoad( mId );
			archive.fSaveLoad( mPlayerData );
			archive.fSaveLoad( mLevelData );
			archive.fSaveLoad( mWaveLists );
			archive.fSaveLoad( mOffensiveWaveLists );
		}

		void fCreateRewindPreview( tSaveGameRewindPreview& preview );
	};

	class tSaveGame : public tRefCounter, public tSaveGameData
	{
	public:
		const tEntitySaveData* fEntFromLevel( u32 index ) const;
		void fOnLevelSpawnEnd( );
		u32 fGetWaveID( const tStringPtr& name ) const;
	private:
		void fSpawnDynamicSavedEnts( );
		void fActivateWaveLists( );
	};

}


#endif//__tSaveGame__
