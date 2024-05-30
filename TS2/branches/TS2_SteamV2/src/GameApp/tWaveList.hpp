#ifndef __tWaveList__
#define __tWaveList__

#include "tDataTableFile.hpp"
#include "tSinglePlayerWaveList.hpp"
#include "tPlayer.hpp"

namespace Sig
{
	class tGeneratorLogic;
	class tWaveList;

	define_smart_ptr( base_export, tRefCounterPtr, tWaveList );
	
	struct tUnitDesc
	{
		tUnitDesc::tUnitDesc( u32 unitID = ~0
			, u32 uiUnitID = ~0
			, u32 country = ~0
			, f32 interval = 0.f
			, const tStringPtr& name = tStringPtr::cNullPtr )
			: mUnitID( unitID )
			, mUIUnitID( uiUnitID )
			, mCountry( country )
			, mSpawnInterval( interval )
			, mName( name )
			, mHealthModifier( 1.f )
			, mTimeMultiplier( 1.f )
		{ }

		u32	mUnitID;
		u32	mUIUnitID; //the unit id used in the ui, to alias unit types together
		u32 mCountry;
		f32 mSpawnInterval;
		tStringPtr mName;
		f32 mHealthModifier;
		f32 mTimeMultiplier;
	};

	struct tWaveDesc : public tRefCounter
	{
		enum tWaveParameters
		{
			cWaveParameterGenerator,
			cWaveParameterUnitCount,
			cWaveParameterReadyTime,
			cWaveParameterCountdown,
			cWaveParameterInterval,
			cWaveParameterTotalLaunchTime,
			cWaveParameterPathName,
			cWaveParameterCount
		};

		enum tGroupParameters
		{
			cGroupParameterUnitCount,
			cGroupParameterInterval,
			cGroupParameterCount
		};

		tWaveDesc( tWaveList* list, u32 count = 0, f32 readyTime = 0.f, f32 countdown = 0.f, f32 spawnInterval = 0.f, f32 launchTime = 0.f, u32 waveID = 0 );

		u32 fUnitDescCount( ) const { return mUnitDescs.fCount( ); }
		const tUnitDesc& fUnitDesc( u32 index ) const;
		u32	fUnitID( u32 index ) const;
		u32	fUIUnitID( u32 index ) const;
		u32 fCountry( u32 index ) const;
		u32 fWaveID( ) const { return mWaveID; }
		b32 fIsFunction( ) const { return mFunctionCallName.fExists( ); }

		b32 fConsiderPreviousWave( ) const { return (mReadyTime == 0.f && mCountdown == 0.f); }

		static b32 fIsBarrage( tWaveDesc* desc );
		static void fExportScriptInterface( tScriptVm& vm );

		tGrowableArray< tUnitDesc > mUnitDescs;
		tGrowableArray< tGeneratorLogic* > mGenerators;

		tWaveList* mList;
		u32 mCount;
		f32 mReadyTime;
		f32 mCountdown;
		f32 mSpawnInterval;
		f32 mTotalLaunchTime;
		u32 mWaveID;
		b32 mPopWhenFinished;
		tStringPtr mPathName;
		tStringPtr mFunctionCallName;
		tStringPtr mVersusName;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tWaveDesc );

	class tWaveList : public tRefCounter
	{
	public:
		enum tWaveMetaDataParameters
		{
			cWaveMetaDataCountry,
			cWaveMetaDataLooping,
			cWaveMetaDataAllowReadySkip,
			cWaveMetaDataCount
		};

		enum tState
		{
			cReadying,
			cWaiting,
			cLaunching,
			cInactive,
			cStateCount
		};

		tWaveList( );
		~tWaveList( );

		void		fUpdate( f32 dt );

		void		fSetTotalWaveIndex( u32 index );

		void		fAddWave( const tWaveDescPtr& wave ) { mWaveList.fPushBack( wave ); }
		tWaveDesc*	fCurrentWave( ) { return fWave( mCurrentWaveIndex ); }
		const tWaveDesc* fCurrentWave( ) const { return fWave( mCurrentWaveIndex ); }
		u32			fTotalWaveID( ) const { return mTotalWaveIndex; }
		void		fNextWave( );
		tWaveDesc*	fWave( u32 index ) { return ( index < mWaveList.fCount( ) ) ? mWaveList[ index ].fGetRawPtr( ) : 0; }
		u32			fWaveCount( ) const { return mWaveList.fCount( ); }
		u32			fSurvivalWaveCount( ) const { return mSurvivalMode ? mSurvivalWaveCount : mWaveList.fCount( ); }
		u32			fNonSimultaneousWaveCount( ) const;
		u32			fUIWaveIDToTotalWaveIndex( u32 index ) const;
		u32			fCurrentUIWaveListID( ) const;

		u32 fCountry( ) const { return mCountry; }
		void fSetCountry( u32 country ) { mCountry = country; }

		const tWaveDesc*	fWave( u32 index ) const { return ( index < mWaveList.fCount( ) ) ? mWaveList[ index ].fGetRawPtr( ) : 0; }

		tWaveDesc* fWaveForScript( u32 index ) { return ( index < mWaveList.fCount( ) ) ? mWaveList[ index ].fGetRawPtr( ) : 0; }
		tWaveDesc* fCurrentWaveForScript( ) { return fWave( mCurrentWaveIndex ); }

		void		fActivate( ) { fSetActive( true ); }
		void		fPause( ) { fSetActive( false ); }
		void		fSetActive( bool activate );
		b32			fIsActive( ) const { return mState != cInactive; }
		void		fSetLooping( bool looping );
		b32			fIsLooping( ) const { return mLooping; }
		void		fReset( );
		void		fKillEveryone( );
		void		fReleaseTrenched( );
		b32			fLaunching( ) const { return mState == cLaunching; }
		void		fSetAllowReadySkip( b32 allow ) { mAllowReadySkip = allow; }
		void		fSetHealthModifier( f32 modifier );
		
		b32			fFinishedLaunching( ) const;
		void		fBeginReadying( );
		void		fFinishReadying( ); // Called when there are no units alive and forces this wave to skip it's ready time
		void		fFinishReadyingForScript( ); //this will force the skip, regardless of internal state. very explicit
		
		const tStringPtr&	fName( ) const { return mName; }
		void				fSetName( const tStringPtr& name ) { mName = name; }

		b32			fMakeSelectableUnits( ) const { return mMakeSelectableUnits; }
		void		fSetMakeSelectableUnits( b32 selectable ) { mMakeSelectableUnits = selectable; }

		u32			fAssignPickup( ) const { return mAssignPickup; }
		void		fSetAssignPickup( u32 pickup ) { mAssignPickup = pickup; }

		void fSetUI( const Gui::tSinglePlayerWaveListPtr& ui );
		const Gui::tSinglePlayerWaveListPtr& fGetUI( ) const { return mWaveListUI; }

		static void fExportScriptInterface( tScriptVm& vm );

		b32 fSaveable( ) const { return mSaveable; }
		void fSetSaveable( b32 saveable ) { mSaveable = saveable; }

		// this is used for loading from a savegame
		void fDontWaitAtAll( );

		b32 fSurvivalMode( ) const { return mSurvivalMode; }

		// indicates this IS a bonus wavelist
		b32 fBonusWave( ) const { return mBonusWave; }
		void fSetBonusWave( b32 bonus ) { mBonusWave = bonus; }

		// indicates we want to start seeing bonus waves (on main wave list )
		b32 fBonusWaveActive( ) const { return mBonusWavesActive; }
		void fSetBonusWaveActive( b32 active ) { mBonusWavesActive = active; }

		b32 fDisableAIFire( ) const { return mDisableAIFire; }
		void fSetDisableAIFire( b32 disable ) { mDisableAIFire = disable; }

		u32 fSurvivalRound( ) const { return mSurvivalRound; }

		void fSetIsVersus( tPlayer* player ) { mVersusPlayer.fReset( player ); }
		f32 fTimeRemaining( ) const;
		void fReactivateReadying( f32 time );

	private:
		void fStartLaunching( );
		void fFillUIWaves( );
		void fRampUpCurrentWaves( );
	private:
		tGrowableArray< tWaveDescPtr > mWaveList;

		u32 mCountry;
		u32 mCurrentWaveIndex;
		u32 mTotalWaveIndex; //keeps counting when mCurrentWaveIndex wraps
		
		tState	mState;
		f32		mTimer;
		b8		mLooping;
		b8		mAllowReadySkip;
		b8		mVeryFirstWave;
		b8		mMakeSelectableUnits;

		b8 mSaveable;
		b8 mSurvivalMode;
		b8 mBonusWave; //for survival
		b8 mBonusWavesActive;

		b8 mDisableAIFire;
		u8 pad0;
		b8 pad1;
		b8 pad2;

		u32 mAssignPickup; //to all units spawned
		u32 mSurvivalRound;
		u32 mSurvivalWaveCount;

		tStringPtr mName;
		Gui::tSinglePlayerWaveListPtr mWaveListUI;

		tPlayerPtr mVersusPlayer;
	};
}

#endif//__tWaveList__
