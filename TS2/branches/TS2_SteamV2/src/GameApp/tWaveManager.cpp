#include "GameAppPch.hpp"
#include "tWaveManager.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"

namespace Sig
{
	namespace
	{
		static const tStringPtr cStringMetaData( "METADATA" );

#ifdef sig_devmenu
		void fAAASkipWave( tDevCallback::tArgs& args )
		{
			tWaveManagerPtr& manager = tGameApp::fInstance( ).fCurrentLevel( )->fWaveManager( );
			if( manager )
				manager->fSkipWave( );
		}
#endif //sig_devmenu

	}

	devcb( AAACheats_SkipWave, "1", make_delegate_cfn( tDevCallback::tFunction, fAAASkipWave ) );

	tWaveManager::tWaveManager( ) 
		: mAliveNonLoopingUnitsCount( 0 )
		, mCurrentWaveID( 0 )
		, mUIHidden( false )
		, mEnded( true )
		, mLastDisplayedWavelist( NULL )
	{
		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
		{
			tGameApp& game = tGameApp::fInstance( );

			std::string layer = ( game.fGameMode( ).fIsSplitScreen( ) )? "alwaysShow": "alwaysHide";
			if( game.fGameMode( ).fIsNet( ) )
				layer = game.fFrontEndPlayer( )->fHudLayerName( );

			mWaveListUI.fReset( NEW Gui::tSinglePlayerWaveList( game.fGlobalScriptResource( tGameApp::cGlobalScriptSPWaveList ), game.fFrontEndPlayer( )->fUser( ), tStringPtr( layer ) ) );
			game.fHudLayer( layer ).fToCanvasFrame( ).fAddChild( mWaveListUI->fCanvas( ) );

			mEnemiesAliveListUI.fReset( NEW Gui::tEnemiesAliveList( game.fGlobalScriptResource( tGameApp::cGlobalScriptEnemiesAliveList ), game.fFrontEndPlayer( )->fUser( ), game.fFrontEndPlayer( )->fEnemyCountry( ) ) );

			if( game.fGameMode( ).fIsVersus( ) || ( game.fGameMode( ).fIsSplitScreen( ) && game.fGameMode( ).fIsCoOp( ) ) )
			{
				mSecondaryEnemiesAliveListUI.fReset( NEW Gui::tEnemiesAliveList( game.fGlobalScriptResource( tGameApp::cGlobalScriptEnemiesAliveList ), game.fSecondaryPlayer( )->fUser( ), game.fSecondaryPlayer( )->fEnemyCountry( ) ) );
			}

			fShow( false );
		}
	}

	tWaveManager::~tWaveManager( )
	{
	}

	void tWaveManager::fUpdate( f32 dt )
	{
		// Update wave lists and check if any enemy waves are active
		//b32 someWaveListIsActive = false;
		for( u32 i = 0; i < mWaveLists.fCount( ); ++i )
		{
			mWaveLists[ i ]->fUpdate( dt );
			//someWaveListIsActive = someWaveListIsActive || ( mWaveLists[ i ]->fIsActive( ) && mWaveLists[ i ]->fCountry( ) == tGameApp::fInstance( ).fGetPlayer( 0 )->fEnemyCountry( ) );
		}

		// Possibly put these on timers so it isn't called every frame
		const tGameMode& gm = tGameApp::fInstance( ).fGameMode( );
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );		
		sigassert( level );

		if( /*someWaveListIsActive &&*/ mWaveListUI )
		{
			if( !mWaveListUI->fIsVisible( ) )
				fShow( true );

			if( level->fMapType( ) == GameFlags::cMAP_TYPE_SURVIVAL )
				mWaveListUI->fSurvivalTimer( level->fSurvivalTimer( ) );
		}

		const b32 waveListsDone = !gm.fIsVersus( ) // waves are never done in versus.
			&& fAreWaveListsDone( level->fMapType( ) == GameFlags::cMAP_TYPE_SURVIVAL ); //challenge matches include looping waves
		
		u32 loopingUnitsCount = 0; // looping units still count towards ready skip
		mAliveNonLoopingUnitsCount = fUpdateEnemiesAliveCount( loopingUnitsCount );
		if( mAliveNonLoopingUnitsCount == 0 && loopingUnitsCount == 0 )
			for( u32 i = 0; i < mWaveLists.fCount( ); ++i )
				mWaveLists[ i ]->fFinishReadying( );

		if( waveListsDone && mAliveNonLoopingUnitsCount == 0 )
		{
			if( !mEnded )
			{
				mEnded = true;
				level->fOnNoWavesOrEnemiesLeft( );
				fShow( false );
			}
		}
		else if( !waveListsDone && mAliveNonLoopingUnitsCount > 0 )
			mEnded = false;

	}

	void tWaveManager::fSkipWave( )
	{
		for( u32 i = 0; i < mWaveLists.fCount( ); ++i )
			mWaveLists[ i ]->fNextWave( );	
	}

	tWaveList* tWaveManager::fAddWaveList( const tStringPtr& waveListName )
	{
		// Old level specific datatable
		tGameApp& gameApp = tGameApp::fInstance( );
		const tDataTableFile& dataTableFile = gameApp.fDataTable( tGameApp::cDataTableWaveList );
		return fAddWaveList( dataTableFile, waveListName );
	}

	tWaveList* tWaveManager::fAddCommonWaveList( const tStringPtr& waveListName )
	{
		tGameApp& gameApp = tGameApp::fInstance( );
		const tDataTableFile& dataTableFile = gameApp.fDataTable( tGameApp::cDataTableCommonWaveList );
		return fAddWaveList( dataTableFile, waveListName );
	}

	tWaveList* tWaveManager::fAddWaveList( const tDataTableFile& dataTableFile, const tStringPtr& waveListName )
	{
		tGameApp& gameApp = tGameApp::fInstance( );
		if( !gameApp.fHasWaveListTable( ) )
			return 0;

		const tDataTable* dt = dataTableFile.fFindTable( waveListName );

		if( dt )
		{
			tWaveListPtr waveList = tWaveListPtr( NEW tWaveList( ) );
			waveList->fSetName( waveListName );
			
			// Find meta data and set info
			const tDataTable* metaDataTable = dataTableFile.fFindTable( cStringMetaData );
			u32 country = tGameApp::fInstance( ).fGetPlayer( 0 )->fEnemyCountry( );

			if( metaDataTable )
			{
				const u32 rowIndex = metaDataTable->fRowIndex( dt->fName( ) );
				if( rowIndex != ~0 )
				{
					waveList->fSetLooping( metaDataTable->fIndexByRowCol<bool>( rowIndex, tWaveList::cWaveMetaDataLooping ) );
					if( metaDataTable->fColCount( ) > tWaveList::cWaveMetaDataAllowReadySkip )
						waveList->fSetAllowReadySkip( metaDataTable->fIndexByRowCol<bool>( rowIndex, tWaveList::cWaveMetaDataAllowReadySkip ) );

					u32 metaCountry = GameFlags::fCOUNTRYValueStringToEnum( metaDataTable->fIndexByRowCol<tStringPtr>( rowIndex, tWaveList::cWaveMetaDataCountry ) );
					if( metaCountry < GameFlags::cCOUNTRY_COUNT )
						country = metaCountry;
				}
			}

			waveList->fSetCountry( country );
		
			
			for( u32 i = 0; i < dt->fRowCount( ); ++i )
				fFillWaveDesc( waveList, *dt, i );

			if( !gameApp.fGameMode( ).fIsVersus( ) )
			{
				if( mWaveLists.fCount( ) == 0 )
				{
					waveList->fSetUI( mWaveListUI );
					mLastDisplayedWavelist = waveList.fGetRawPtr( );
				}
			}

			mWaveLists.fPushBack( waveList );
			return mWaveLists.fBack( ).fGetRawPtr( );
		}
		
		log_warning( 0, "No DataTable for wave list named: " << waveListName.fCStr( ) );

		return 0;
	}

	namespace
	{
		b32 fTryParseScriptCall( const tStringPtr& input, tStringPtr& functionName )
		{
			tGrowableArray<std::string> bits;
			StringUtil::fSplit( bits, StringUtil::fStripQuotes( std::string( input.fCStr( ) ) ).c_str( ), "," );

			if( bits.fCount( ) != 2 )
				return false;
			if( bits[ 0 ] != "Script" ) 
				return false;

			functionName = tStringPtr( bits[ 1 ] );
			return true;
		}
	}

	void tWaveManager::fFillWaveDesc( const tWaveListPtr& waveList, const tDataTable& table, u32 row, b32 popWhenDone /*= false*/ )
	{
		const u32 count = (u32)table.fIndexByRowCol<f32>( row, tWaveDesc::cWaveParameterUnitCount );
		const f32 readyTime = table.fIndexByRowCol<f32>( row, tWaveDesc::cWaveParameterReadyTime );
		const f32 countdown = table.fIndexByRowCol<f32>( row, tWaveDesc::cWaveParameterCountdown );
		const f32 interval = table.fIndexByRowCol<f32>( row, tWaveDesc::cWaveParameterInterval );
		const f32 launchtime = table.fIndexByRowCol<f32>( row, tWaveDesc::cWaveParameterTotalLaunchTime );

		const u32 waveId = (row == 0) ? mCurrentWaveID					// if it's the first wave, it's whatever wave we are at
			: ( readyTime != 0 || countdown != 0 ) ? ++mCurrentWaveID	// if it doesnt count as the previous wave, increment
			:   mCurrentWaveID;											// same as previous wave
		
		sigassert( waveList );
		tWaveDescPtr wave = tWaveDescPtr( NEW tWaveDesc( waveList.fGetRawPtr( ), count, readyTime, countdown, interval, launchtime, waveId ) );
		wave->mPopWhenFinished = popWhenDone;
		wave->mPathName = tStringPtr( StringUtil::fEatWhiteSpace( table.fIndexByRowCol<tStringPtr>( row, tWaveDesc::cWaveParameterPathName ).fCStr( ) ) );
		tStringPtr genName = table.fIndexByRowCol<tStringPtr>( row, tWaveDesc::cWaveParameterGenerator );
		fFindGenerators( wave, genName );

		if( genName.fExists( ) && wave->mGenerators.fCount( ) == 0 )
		{
			log_line( 0, "No generators found for wave (" << row << ").  Skipping wave in the list." );
			return;
		}

		const tStringPtr& rowName = table.fRowName( row );
		const u32 unitID = GameFlags::fUNIT_IDValueStringToEnum( rowName );
		const u32 country = wave->mGenerators.fCount( ) ? wave->mGenerators[ 0 ]->fCountry( ) : GameFlags::cCOUNTRY_DEFAULT;

		// Didn't find the name.  Probably a group, or script call
		if( unitID >= GameFlags::cUNIT_ID_COUNT )
		{
			if( !fTryParseScriptCall( rowName, wave->mFunctionCallName ) )
			{
				// group?
				const tDataTable* groupTable = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableWaveList ).fFindTable( rowName );
				if( groupTable )
				{
					for( u32 i = 0; i < groupTable->fRowCount( ); ++i )
					{
						const GameFlags::tUNIT_ID groupUnitID = ( GameFlags::tUNIT_ID )GameFlags::fUNIT_IDValueStringToEnum( groupTable->fRowName( i ) );

						if( groupUnitID != GameFlags::cUNIT_ID_COUNT )
						{
							GameFlags::tUNIT_ID unitIDAlias = (GameFlags::tUNIT_ID)tGameApp::fInstance( ).fUnitIDAlias( groupUnitID, country );
							if( unitIDAlias == ~0 ) unitIDAlias = groupUnitID;

							const f32 groupInterval = groupTable->fIndexByRowCol<f32>( i, tWaveDesc::cGroupParameterInterval );
							const u32 groupCount = (u32)groupTable->fIndexByRowCol<f32>( i, tWaveDesc::cGroupParameterUnitCount );

							// Expand out counts so it's easier to iterate over when launching
							for( u32 j = 0; j < groupCount; ++j )
								wave->mUnitDescs.fPushBack( tUnitDesc( groupUnitID, unitIDAlias, country, groupInterval, waveList->fName( ) ) );
						}
						else
							log_warning( 0, "Invalid unit type ( " << groupTable->fRowName( i ) << " ) in Group ( " << rowName << " )." );
					}
				}
				else
					log_warning( 0, "Row name ( " << rowName << " ) is not a unit type or group name." );
			}
		}
		else
		{
			u32 unitIDAlias = tGameApp::fInstance( ).fUnitIDAlias( unitID, country );
			if( unitIDAlias == ~0 ) unitIDAlias = unitID;
			wave->mUnitDescs.fPushBack( tUnitDesc( unitID, unitIDAlias, country, 0.f, waveList->fName( ) ) );
		}

		// Calculate total launch time
		if( wave->mTotalLaunchTime == 0 )	
		{
			f32 groupInterval = 0.f;
			for( u32 i = 0; i < wave->mUnitDescs.fCount( ); ++i )
				groupInterval += wave->mUnitDescs[ i ].mSpawnInterval;

			wave->mTotalLaunchTime = ( groupInterval + wave->mSpawnInterval ) * wave->mCount;
		}

		waveList->fAddWave( wave );
	}

	void tWaveManager::fFindGenerators( const tWaveDescPtr& wave, const tStringPtr& generators )
	{
		tGrowableArray<std::string> genNames;
		StringUtil::fSplit( genNames, StringUtil::fStripQuotes( std::string( generators.fCStr( ) ) ).c_str( ), "," );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( level );

		for( u32 i = 0; i < genNames.fCount( ); ++i )
		{
			b32 genFound = false;
			for( u32 j = 0; j < level->fGeneratorCount( ); ++j )
			{
				const tStringPtr name = tStringPtr( StringUtil::fEatWhiteSpace( genNames[ i ] ) );
				if( level->fGenerator( j )->fName( ) == name )
				{
					tGeneratorLogic* gen = level->fGenerator( j )->fLogicDerived<tGeneratorLogic>( );
					if( gen )
					{
						wave->mGenerators.fPushBack( gen );
						genFound = true;
						break;
					}
				}
			}

			if( !genFound )
				log_warning( 0, "Generator ( " << genNames[ i ] << " ) not found." );
		}
	}

	tWaveList* tWaveManager::fWaveList( const tStringPtr& name )
	{
		for( u32 i = 0; i < mWaveLists.fCount( ); ++i )
		{
			if( mWaveLists[ i ]->fName( ) == name )
				return mWaveLists[ i ].fGetRawPtr( );
		}

		return NULL;
	}

	void tWaveManager::fSetUIWaveList( const tStringPtr& listName )
	{
		if( mWaveListUI )
		{
			b32 found = false;

			mWaveListUI->fClear( );
			for( u32 i = 0; i < mWaveLists.fCount( ); ++i )
			{
				if( mWaveLists[ i ]->fName( ) == listName )
				{
					found = true;
					mWaveLists[ i ]->fSetUI( mWaveListUI );
					mLastDisplayedWavelist = mWaveLists[ i ].fGetRawPtr( );
				}
				else
					mWaveLists[ i ]->fSetUI( Gui::tSinglePlayerWaveListPtr( ) );
			}

			fShowWaveListUI( found );
		}
	}

	void tWaveManager::fShow( b32 show )
	{
		if( mWaveListUI && !tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
		{
			if( !mWaveLists.fCount( ) || mUIHidden )
				show = false;

			mWaveListUI->fShow( show );
		}
	}

	void tWaveManager::fShowWaveListUI( b32 show )
	{
		mUIHidden = !show;
		fShow( show );
	}

	void tWaveManager::fShowEnemiesAliveList( b32 show )
	{
		if( mEnemiesAliveListUI )
			mEnemiesAliveListUI->fShow( show );
	}

	b32 tWaveManager::fAreWaveListsDone( b32 countLooping )
	{
		u32 enemyCountry = tGameApp::fInstance( ).fGetPlayer( 0 )->fEnemyCountry( );
		b32 done = true;

		for( u32 i = 0; i < mWaveLists.fCount( ); ++i )
		{
			if( mWaveLists[ i ]->fCountry( ) == enemyCountry  
				&& (countLooping || !mWaveLists[ i ]->fIsLooping( ))
				&& mWaveLists[ i ]->fIsActive( ) )
			{
				done = false;
				break;
			}
		}

		return done;
	}

	u32 tWaveManager::fUpdateEnemiesAliveCount( u32& loopingUnitsCountOut )
	{
		//the total count here should only matter for singleplayer/co-op (not versus) matches, so only total up enemies
		u32 totalCount = 0;

		for( tEnemyAliveCountTable::tIterator unitCount = mEnemiesAliveCounts.fBegin( ); unitCount != mEnemiesAliveCounts.fEnd( ); ++unitCount )
		{
			if( unitCount->fNullOrRemoved( ) )
				continue;

			u32 unitID = unitCount->mKey.mID;
			u32 unitCountry = unitCount->mKey.mCountry;

			if( unitCountry == tGameApp::fInstance( ).fGetPlayer( 0 )->fEnemyCountry( ) )
			{
				if( unitCount->mKey.mFromLoopingList )
					loopingUnitsCountOut += unitCount->mValue;
				else
					totalCount += unitCount->mValue;
			}
		}

		return totalCount;
	}

	void tWaveManager::fUpdateEnemiesAliveUI( )
	{
		for( tEnemyAliveCountTable::tIteratorNoNullOrRemoved unitCount( mEnemiesAliveCounts.fBegin( ), mEnemiesAliveCounts.fEnd( ) ); unitCount != mEnemiesAliveCounts.fEnd( ); ++unitCount )
		{
			const u32 unitID = unitCount->mKey.mID;
			const u32 unitCountry = unitCount->mKey.mCountry;
			const u32 count = unitCount->mValue;

			if( mEnemiesAliveListUI )
				mEnemiesAliveListUI->fSetCount( unitID, unitCountry, count );

			if( mSecondaryEnemiesAliveListUI )
				mSecondaryEnemiesAliveListUI->fSetCount( unitID, unitCountry, count );
		}
	}

	/// Add a unit to the EnemiesAliveList
	u32 tWaveManager::fAddEnemyAliveUnit( u32 unitID, u32 country, b32 fromLooping )
	{
		// Only add to the ENEMIES count if the unit is on the enemy's team
		if( unitID > GameFlags::cUNIT_ID_NONE && unitID < GameFlags::cUNIT_ID_COUNT )
		{
			u32& count = mEnemiesAliveCounts[ tAliveUnit( unitID, country, fromLooping ) ];
			count++;
			fUpdateEnemiesAliveUI( );
			return count;
		}
		else
		{
			return 0;
		}
	}

	/// Remove a unit from the EnemiesAliveList. If the unit type doesn't exist, fail silently
	u32 tWaveManager::fRemoveEnemyAliveUnit( u32 unitID, u32 country, b32 fromLooping )
	{
		if( unitID > GameFlags::cUNIT_ID_NONE && unitID < GameFlags::cUNIT_ID_COUNT )
		{
			u32* count = mEnemiesAliveCounts.fFind( tAliveUnit( unitID, country, fromLooping ) );

			if( count )
			{
				if( (*count) > 0 )
					(*count)--;
				
				fUpdateEnemiesAliveUI( );

				if( (*count) <= 0 )
					mEnemiesAliveCounts.fRemove( count );

				return *count;
			}
		}

		return 0;
	}

	tWaveList* tWaveManager::fDisplayedWaveList( ) const
	{
		if( mWaveListUI )
		{
			tWaveList* found = NULL;

			for( u32 i = 0; i < mWaveLists.fCount( ); ++i )
			{
				if( !mWaveLists[ i ]->fGetUI( ).fNull( ) && mWaveListUI == mWaveLists[ i ]->fGetUI( ) )
				{
					found = mWaveLists[ i ].fGetRawPtr( );
				}
			}

			return found;
		}
		return NULL;
	}

	tWaveList* tWaveManager::fCurrentOrLastDisplayedWaveList() const
	{
		if( mLastDisplayedWavelist )
			return mLastDisplayedWavelist;
		else
			return fDisplayedWaveList( );
	}

	void tWaveManager::fOnLevelUnloadBegin( )
	{
		if( mWaveListUI )
		{
			mWaveListUI->fCanvas( ).fRemoveFromParent( );
			mWaveListUI->fCanvas( ).fDeleteSelf( );
			mWaveListUI->fReleaseCanvas( );
		}

		if( mEnemiesAliveListUI )
		{
			mEnemiesAliveListUI->fCanvas( ).fRemoveFromParent( );
			mEnemiesAliveListUI->fCanvas( ).fDeleteSelf( );
			mEnemiesAliveListUI->fReleaseCanvas( );
		}

		if( mSecondaryEnemiesAliveListUI )
		{
			mSecondaryEnemiesAliveListUI->fCanvas( ).fRemoveFromParent( );
			mSecondaryEnemiesAliveListUI->fCanvas( ).fDeleteSelf( );
			mSecondaryEnemiesAliveListUI->fReleaseCanvas( );
		}
	}

	void tWaveManager::fStopAllWaves( )
	{
		for( u32 i = 0; i < mWaveLists.fCount( ); ++i )
			mWaveLists[ i ]->fPause( );
	}

	void tWaveManager::fReactivateWaveLists( const tGrowableArray< tSaveGameData::tWaveID >& waveData )
	{
		for( u32 i = 0; i < waveData.fCount( ); ++i )
		{
			tWaveList* list = fWaveList( waveData[ i ].mWaveName );
			if( list )
			{
				list->fReset( );
				list->fSetLooping( waveData[ i ].mIsLooping ? true : false );
				list->fSetTotalWaveIndex( waveData[ i ].mWaveID );
				list->fSetActive( waveData[ i ].mActive != 0 );
				list->fDontWaitAtAll( );
			}
			else
				log_warning( 0, "Wave list nto found when game was loaded!" );
		}
	}

}
