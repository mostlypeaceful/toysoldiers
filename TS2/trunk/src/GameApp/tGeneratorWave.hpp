#ifndef __tGeneratorWave__
#define __tGeneratorWave__

#include "tWaveList.hpp"

namespace Sig
{
	class tPathEntity;
	class tUnitLogic;
	class tPlayer;

	struct tSpawnPair
	{
		tEntityPtr	mEnt;
		u32			mSlot;

		tSpawnPair( ) : mSlot( 0 ) { }
		tSpawnPair( const tEntityPtr& e, u32 slot ) : mEnt( e ), mSlot( slot ) { }

		b32 operator == (const tEntity* ent ) const { return mEnt == ent; }
	};

	struct tSpawnedUnits : public tRefCounter
	{
		// This is a temporary list where units live until they are launched]
		//  u32 indicates which slot the spawned unit is occupying
		tGrowableArray< tSpawnPair >	mSpawningUnits;
		tGrowableArray< tEntityPtr >	mLaunchedUnits; //alive people in this list who have been launched

		// array with values from 0 to mTotalSpawnCount-1 indicating which positions are still available to stand in.
		tGrowableArray< u32 > mSlots;

		tSpawnedUnits( u32 count )
		{
			mSpawningUnits.fSetCount( 0 );
			mSlots.fSetCount( count );

			for( u32 i = 0; i < count; ++i )
				mSlots[ i ] = i;
		}
	};
	define_smart_ptr( base_export, tRefCounterPtr, tSpawnedUnits );
	typedef tHashTable< u32, tSpawnedUnitsPtr > tSpawnedUnitsMap;


	// RAII lifetime control of the smoke effect. when no one owns this, destroy the smoke effect
	class tSmokeDestroyer : public tRefCounter
	{
	public:
		tSmokeDestroyer( tEntity* smoke, const tPathEntity* from )
			: mSmoke( smoke )
			, mFrom( from )
		{ }

		~tSmokeDestroyer( )
		{
			if( mSmoke )
				mSmoke->fDelete( );
		}

		tEntityPtr mSmoke;
		const tPathEntity* mFrom;
	};

	typedef tRefCounterPtr<tSmokeDestroyer> tSmokeDestroyerPtr;

	struct tGeneratorWave : tRefCounter
	{
		tGrowableArray< tUnitDesc > mUnits;

		tGeneratorWave( tWaveDesc* desc, u32 spawnCount, tGeneratorLogic* generator );

		void fLaunch( );
		b32 fLaunched( ) const { return mLaunched; }

		void fStep( f32 dt, const Math::tMat3f& spawnXform );

		void fEnemyKilled( tUnitLogic* logic, tPlayer* player );
		void fRefreshActiveUnitCounts( );
		void fKillEveryone( );
		void fReleaseTrenched( );

		b32 fDone( ) const { return mLaunched && mKilled && fSpawnedEnough( ); }
		b32 fSpawnedEnough( ) const { return mCurrentSpawnCount >= mTotalSpawnCount; }
		b32 fEquals( const tWaveDesc& desc ) { return mDesc == &desc; }
		const tStringPtr& fPathName( ) const { return mPathName; }

		// Using this to manage prompting ensures only one prompt per wave
		void fPrompt( tUnitLogic* unit );
		b32  fPrompted( ) const { return mPrompted; }

		void fAddCargoUnit( tUnitLogic* unit );

		b32 fDisableAIFire( ) const;

		tWaveList& fList( ) const { sigassert( mDesc ); sigassert( mDesc->mList ); return *mDesc->mList; }

	private:
		void fSpawnUnit( const Math::tMat3f& spawnXform );
		void fConfigureUnitPath( tUnitLogic* logic, const Math::tVec3f& startPt, f32 slotPosition, b32 wait, const tStringPtr& pathName );
		void fDropSmoke( tUnitLogic* logic, const tPathEntity* pathStart );
		void fSpawnArrow( tUnitLogic* logic );

		tSpawnedUnitsPtr* fFindOrAddSpawnedUnits( u32 unitID, u32 slotCount );

		u32 mCurrentUnitIndex;
		u32 mCurrentSpawnCount;
		u32 mCurrentLaunchCount;
		u32 mTotalSpawnCount;
		f32 mSpawnInterval;
		f32 mSpawnTimer;
		f32 mValue;
		b8 mLaunched;
		b8 mGenerating;
		b8 mKilled;
		b8 mPrompted;
		b16 mKillingAll; //this is true if we are killing all, dont give anyone credit and dont let teh arrays change
		b16 mSpawnedArrow;
		tStringPtr mPathName;
		tGeneratorLogic* mGenerator;
		tWaveDescPtr mDesc;

		tSpawnedUnitsMap	mSpawnedUnits;
		tGrowableArray<tSmokeDestroyerPtr> mSmokePtrs;
		tGrowableArray<const tPathEntity*> mDroppedSmokes;
		tFixedArray<u32, 2> mKillsPerPlayer; //matches gameapp player array
	
	};

	typedef tRefCounterPtr<tGeneratorWave> tWavePtr;

}

#endif//__tGeneratorWave__

