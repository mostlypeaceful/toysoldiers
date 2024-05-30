#ifndef __tWaveManager__
#define __tWaveManager__

#include "tGeneratorLogic.hpp"
#include "tWaveList.hpp"
#include "tSinglePlayerWaveList.hpp"
#include "tEnemiesAliveList.hpp"
#include "tVersusWaveList.hpp"
#include "tSaveGame.hpp"

namespace Sig
{
	class tWaveManager : public tRefCounter
	{
	public:
		tWaveManager( );
		virtual ~tWaveManager( );

		void fUpdate( f32 dt );

		tWaveList* fAddWaveList( const tStringPtr& waveListName );
		tWaveList* fAddCommonWaveList( const tStringPtr& waveListName );
		tWaveList* fAddWaveList( const tDataTableFile& dataTableFile, const tStringPtr& waveListName );

		u32 fWaveListCount( ) const { return mWaveLists.fCount( ); }
		tWaveList* fWaveList( u32 i ) { return i < mWaveLists.fCount( ) ? mWaveLists[ i ].fGetRawPtr( ) : 0; }
		tWaveList* fWaveList( const tStringPtr& name );
		void fSetUIWaveList( const tStringPtr& listName );

		void fRemoveWaveList( tWaveList* list ) { mWaveLists.fFindAndEraseOrdered( list ); }

		void fShow( b32 show );
		void fShowEnemiesAliveList( b32 show );
		u32	 fTotalEnemyCount( ) const { return mAliveNonLoopingUnitsCount; }
		

		u32 fAddEnemyAliveUnit( u32 unitID, u32 country, b32 fromLooping );
		u32 fRemoveEnemyAliveUnit( u32 unitID, u32 country, b32 fromLooping );

		void fSkipWave( );
		tWaveList* fDisplayedWaveList( ) const;
		tWaveList* fCurrentOrLastDisplayedWaveList( ) const;

		virtual void fOnLevelUnloadBegin( );

		void fStopAllWaves( );

		virtual void fReactivateWaveLists( const tGrowableArray< tSaveGameData::tWaveID >& waveData );

	protected:
		b32 fAreWaveListsDone( b32 versus );
		u32 fUpdateEnemiesAliveCount( u32& loopingUnitsCountOut );
		void fUpdateEnemiesAliveUI( );

		void fFillWaveDesc( const tWaveListPtr& waveList, const tDataTable& table, u32 row, b32 popWhenDone = false );
		void fFindGenerators( const tWaveDescPtr& wave, const tStringPtr& generators );
	protected:
		tGrowableArray<tWaveListPtr> mWaveLists;
		u32	mCurrentWaveID;
		u32 mAliveNonLoopingUnitsCount;
		b16 mUIHidden;
		b16 mEnded;
		tWaveList* mLastDisplayedWavelist;

		Gui::tSinglePlayerWaveListPtr	mWaveListUI;
		Gui::tEnemiesAliveListPtr		mEnemiesAliveListUI;
		Gui::tEnemiesAliveListPtr		mSecondaryEnemiesAliveListUI;

		struct tAliveUnit
		{
			tAliveUnit( u32 id = GameFlags::cUNIT_ID_COUNT, u32 country = GameFlags::cCOUNTRY_COUNT, b32 fromLooping = false )
				: mID( id )
				, mCountry( country )
				, mFromLoopingList( fromLooping )
			{ }

			b32 operator == ( const tAliveUnit& other ) const { return mID == other.mID && mCountry == other.mCountry && mFromLoopingList == other.mFromLoopingList; }

			u32 mID;
			u32 mCountry;
			b32 mFromLoopingList;
		};

		typedef tHashTable< tAliveUnit, u32 > tEnemyAliveCountTable;
		tEnemyAliveCountTable mEnemiesAliveCounts; // Key:, Value: Count
	};

	define_smart_ptr( base_export, tRefCounterPtr, tWaveManager );
}

#endif//__tWaveManager__
