#include "GameAppPch.hpp"
#include "tGameLoadAppState.hpp"
#include "tGameSimulationAppState.hpp"
#include "tDataTableFile.hpp"
#include "tSceneGraphFile.hpp"
#include "Scripts/tScriptFile.hpp"
#include "GameArchiveString.hpp"

#include "Wwise_IDs.h"

// For tSpawnParameters, specifically cSpawnUnitLevel.  Alternatively, add it to GameFlags
#include "tGeneratorLogic.hpp"

namespace Sig
{
	namespace
	{
		static const tStringPtr cStringMetaData( "METADATA" );
		static const tStringPtr cStringVersusUnitGenPreload( "UNIT_GENERATOR_PRELOAD" );

		enum tLoadStates
		{
			cLoadStatePerma,
			cLoadStateWaveList,
			cLoadStateUnits,
			cLoadStateCount,
		};
	}

	//------------------------------------------------------------------------------
	tGameLoadAppState::tGameLoadAppState( const tLevelLoadInfo& levelLoadInfo, b32 allowLoad, b32 allowSpawn )
		: tAppStateLevelLoad( levelLoadInfo.mMapPath, levelLoadInfo.mLoadScript, cLoadStateCount, levelLoadInfo.fIsAssetEquivalent( tGameApp::fInstance( ).fCurrentLevelLoadInfo( ) ) )
		, mLevelLoadInfo( levelLoadInfo )
		, mAllowLoad( allowLoad )
		, mAllowSpawn( false )

	{
		fAllowSpawn( allowSpawn );
		tGameApp::fInstance( ).fSetIngameSimulationState( false );
		//if( fLoadScreen( ) && levelLoadInfo.mDescriptionLocKey.fExists( ) )
			//fLoadScreen( )->fCustomData( ).SetValue( "IntroTextId", levelLoadInfo.mDescriptionLocKey.fCStr( ) );
		fCommonInit( );
	}

	//------------------------------------------------------------------------------
	tGameLoadAppState::tGameLoadAppState( 
		const tFilePathPtr& loadScript, const tSaveGameStorageDesc & saveGameDesc, b32 allowLoad, b32 allowSpawn )
		: tAppStateLevelLoad( tGameApp::fInstance( ).fFrontEndPlayer( )->fUser( ), saveGameDesc, loadScript, cLoadStateCount)
		, mAllowLoad( allowLoad )
		, mAllowSpawn( false )
	{
		fAllowSpawn( allowSpawn );
		tGameApp::fInstance( ).fSetIngameSimulationState( false );

		mLevelLoadInfo.mLoadScript = loadScript;
		fCommonInit( );
	}

	//------------------------------------------------------------------------------
	tGameLoadAppState::tGameLoadAppState( 
		const tFilePathPtr& loadScript, tGameArchive & saveGameArchive, b32 allowLoad, b32 allowSpawn )
		: tAppStateLevelLoad( saveGameArchive, loadScript, cLoadStateCount )
		, mAllowLoad( allowLoad )
		, mAllowSpawn( false )
	{
		fAllowSpawn( allowSpawn );
		tGameApp::fInstance( ).fSetIngameSimulationState( false );
		mLevelLoadInfo.mLoadScript = loadScript;

		tAppStateLevelLoad::fParseSaveGameData( saveGameArchive );
		fCommonInit( );
	}

	void tGameLoadAppState::fCommonInit( )
	{
		if( tGameApp::fInstance( ).fSceneGraph( )->fIsPaused( ) )
		{
			// this cancels the blend out of any world sounds like gun fire
			tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( )->fHandleEvent( AK::EVENTS::STOP_UI_PAUSEGAME );
		}
	}
	//------------------------------------------------------------------------------
	void tGameLoadAppState::fClearOldResources( )
	{
		tGameApp::fInstance( ).fOnLevelUnloadBegin( );
		tAppStateLevelLoad::fClearOldResources( );
		tGameApp::fInstance( ).fOnLevelUnloadEnd( );

		tGameApp::fInstance( ).fConsolidateMemory( );
	}
	void tGameLoadAppState::fLoadResources( )
	{
		switch( fLoadCompleteCount( ) )
		{
		case cLoadStatePerma:
			{
				// ensure the applications perma loaded resources are loaded. this should only get used during the front end.
				fUnloadAll( );
				fAddApplicationResourcesToLoadList( );
				fLoadAll( );
			}
			break;
		case cLoadStateWaveList:
			{
				fUnloadAll( );
				fAddWaveListToLoadList( );
				fLoadAll( );
			}
			break;
		case cLoadStateUnits:
			{
				fUnloadAll( );				
				fAddPlayerTurretsToLoadList( );
				fAddWaveListUnitsToLoadList( );
				fAddVersusUnitGeneratorsToLoadList( );
				fAddLevelToLoadList( );
				fLoadAll( );
				tGameApp::fInstance( ).fOnLevelLoadBegin( );
			}
			break;
		default:
			log_assert( false, "Invalid load stage: " << fLoadCompleteCount( ) );
			break;
		}

	}
	void tGameLoadAppState::fOnLevelSpawnBegin( )
	{
		tGameApp::fInstance( ).fOnLevelSpawnBegin( );
	}
	void tGameLoadAppState::fOnLevelSpawnEnd( )
	{
		tGameApp::fInstance( ).fOnLevelSpawnEnd( );
		tGameApp::fInstance( ).fOnLevelLoadEnd( );
	}
	void tGameLoadAppState::fOnCompletion( )
	{
		sync_line( );
		fNewApplicationState( tApplicationStatePtr( NEW tGameSimulationAppState( ) ) );
	}

	void tGameLoadAppState::fAllowSpawn( b32 allow )
	{
		mAllowSpawn = allow;
		sigassert( fLoadScreen( ) );
		fLoadScreen( )->fSetCanSpawn( allow );
	}

	void tGameLoadAppState::fSaveGameLoadError( tSaveGameStorageReader& reader )
	{
		// Clear the player's data, so we don't try this again
		tPlayer* player = tGameApp::fInstance( ).fFrontEndPlayer( );
		if( player )
		{
			tGameApp::tErrorTypes error = tGameApp::cMissingSaveFile;
			if( reader.fErrorCode( ) == tSaveGameStorageReader::cCorruptData )
			{
				if( player->fProfile( ) )
					player->fProfile( )->fInvalidateLastRewind( );

				error = tGameApp::cCorruptSaveFile;
			}

			tGameApp::fInstance( ).fShowErrorDialog( error, player->fUser( ).fGetRawPtr( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tGameLoadAppState::fParseSaveGameData( 
		tGameArchive & archive, 
		tFilePathPtr & mapPath, 
		b32 & isAssetEquivalent )
	{
		sigassert( archive.fMode() == tGameArchive::cModeLoad );

		// This will organize the next level load info
		
		// Steal the next level load info
		mLevelLoadInfo = tGameApp::fInstance( ).fGetSaveGameLevelLoadInfo( archive );

		// Fill out params
		isAssetEquivalent = mLevelLoadInfo.fIsAssetEquivalent( 
			tGameApp::fInstance( ).fCurrentLevelLoadInfo( ) );

		mapPath = mLevelLoadInfo.mMapPath;
	}

	void tGameLoadAppState::fAddApplicationResourcesToLoadList( )
	{
		const tGrowableArray<tResourcePtr>& res = tGameApp::fInstance( ).fAlwaysLoadedResources( );
		for( u32 i = 0; i < res.fCount( ); ++i )
			if( res[ i ]->fLoading( ) )
				fAddToLoadList( res[ i ] );
	}
	void tGameLoadAppState::fAddWaveListToLoadList( )
	{
		if( mLevelLoadInfo.mWaveList.fExists( ) )
		{
			const tResourcePtr waveListResource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< tDataTableFile >( mLevelLoadInfo.mWaveList ) );
			tGameApp::fInstance( ).fSetWaveListTable( waveListResource );
			fAddToLoadList( waveListResource );
		}
		else
			log_warning( 0, "No WaveList table specified for level " << mLevelLoadInfo.mMapPath );
	}
	void tGameLoadAppState::fAddPlayerTurretsToLoadList( )
	{
		tGameApp& gameApp = tGameApp::fInstance( );
		b32 frontEnd = gameApp.fGameMode( ).fIsFrontEnd( );

		const tGameMode& gm = gameApp.fGameMode( );

		if( frontEnd || tGameApp::fExtraDemoMode( ) )
		{
		}
		else if( gm.fIsSinglePlayerOrCoop( ) && !gameApp.fCurrentLevelLoadInfo( ).fIsDisplayCase( ) )
		{
			u32 playingAsCountry = mLevelLoadInfo.mCountry;
			fAddPlayerTurretsToLoadListByCountry( playingAsCountry );
			fAddBarrageToLoadList( playingAsCountry );
		}
		else
		{
			fAddPlayerTurretsToLoadListByCountry( mLevelLoadInfo.mCountry );
			fAddPlayerTurretsToLoadListByCountry( mLevelLoadInfo.mCountry2 );

			if( gm.fIsVersus( ) )
			{
				fAddBarrageToLoadList( mLevelLoadInfo.mCountry );
				fAddBarrageToLoadList( mLevelLoadInfo.mCountry2 );
			}
		}
	}
	void tGameLoadAppState::fAddBarrageToLoadList( u32 country )
	{
		const tFilePathPtr cUSAPath( "gameplay/barrage/usa_barrage_imports.nut" );
		const tFilePathPtr cUSSRPath( "gameplay/barrage/ussr_barrage_imports.nut" );
		const tFilePathPtr cBRITISHPath( "gameplay/barrage/british_barrage_imports.nut" );
		const tFilePathPtr cGERMANPath( "gameplay/barrage/german_barrage_imports.nut" );
		const tFilePathPtr cFRENCHPath( "gameplay/barrage/french_barrage_imports.nut" );

		tFilePathPtr path;
		
		if( country == GameFlags::cCOUNTRY_USA )
			path = cUSAPath;
		else if( country == GameFlags::cCOUNTRY_USSR )
			path = cUSSRPath;
		else if( country == GameFlags::cCOUNTRY_BRITISH )
			path = cBRITISHPath;
		else if( country == GameFlags::cCOUNTRY_GERMAN )
			path = cGERMANPath;
		else if( country == GameFlags::cCOUNTRY_FRENCH )
			path = cFRENCHPath;

		log_assert( path.fExists( ), "Invalid country passed to fAddBarrageToLoadList" );
		const tResourcePtr resource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< tScriptFile >( path ) );
		fAddToLoadList( resource );
		tGameApp::fInstance( ).fAddLevelResource( resource );
	}
	void tGameLoadAppState::fAddUnitToLoadList( u32 unitID, u32 country )
	{
		const tFilePathPtr turretPath = tGameApp::fInstance( ).fUnitResourcePath( unitID, country );
		if( turretPath.fExists( ) )
		{
			const tResourcePtr turretResource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< tSceneGraphFile >( turretPath ) );
			fAddToLoadList( turretResource );
			tGameApp::fInstance( ).fAddLevelResource( turretResource );
		}
	}
	void tGameLoadAppState::fAddPlayerTurretsToLoadListByCountry( u32 country )
	{
		// Turrets
		for( u32 turretType = GameFlags::cUNIT_ID_TURRET_MG_01; turretType <= GameFlags::cUNIT_ID_TURRET_FLAME_03; ++turretType )
			fAddUnitToLoadList( turretType, country );

		// Walls
		for( u32 turretType = GameFlags::cUNIT_ID_WALL_WIRE; turretType <= GameFlags::cUNIT_ID_WALL_WIRE; ++turretType )
			fAddUnitToLoadList( turretType, country );
	}
	void tGameLoadAppState::fAddWaveListUnitsToLoadList( )
	{
		if( tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
		{
			//fAddWaveListUnitsToLoadListByCountry( tGameApp::fInstance( ).fFrontEndPlayer( )->fEnemyCountry( ) );
			fAddWaveListUnitsToLoadListByCountry( mLevelLoadInfo.mCountry2 );
		}
		else if( tGameApp::fInstance( ).fGameMode( ).fIsSinglePlayerOrCoop( ) )
		{
			//fAddWaveListUnitsToLoadListByCountry( tGameApp::fInstance( ).fFrontEndPlayer( )->fEnemyCountry( ) );
			fAddWaveListUnitsToLoadListByCountry( mLevelLoadInfo.mCountry2 );
		}
		else if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
		{
			fAddVersusWaveListUnitsToLoadListByCountry( mLevelLoadInfo.mCountry );
			fAddVersusWaveListUnitsToLoadListByCountry( mLevelLoadInfo.mCountry2 );
		}
		else
		{
			fAddWaveListUnitsToLoadListByCountry( GameFlags::cCOUNTRY_COUNT );
			//log_warning( 0, "MP game, so skip loading units from the wave list." );
		}
	}
	void tGameLoadAppState::fAddWaveListUnitsToLoadListByCountry( u32 fallbackPlayer )
	{
		if( !tGameApp::fInstance( ).fHasWaveListTable( ) )
			return;

		const tDataTableFile& dataTableFile = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableWaveList );
		const tDataTable* metaDataTable = dataTableFile.fFindTable( cStringMetaData );

		for( u32 i = 0; i < dataTableFile.fTableCount( ); ++i )
		{
			const tDataTable& dataTable = dataTableFile.fIndexTable( i );

			if( cStringMetaData == dataTable.fName( ) )
				continue;

			// Look for country info for this wave list.  Use the passed in country if it doesn't exist.
			u32 unitCountry = fallbackPlayer;
			if( metaDataTable )
			{
				const u32 rowIndex = metaDataTable->fRowIndex( dataTable.fName( ) );
				if( rowIndex != ~0 )
				{
					tStringPtr countryStr = metaDataTable->fIndexByRowCol<tStringPtr>( rowIndex, tWaveList::cWaveMetaDataCountry );
					u32 newCountry = GameFlags::fCOUNTRYValueStringToEnum( countryStr );
					if( newCountry < GameFlags::cCOUNTRY_COUNT )
						unitCountry = newCountry;
					else
						log_warning( 0, "Invalid country type in wavelist metatable: " << countryStr );
				}
			}

			if( unitCountry >= GameFlags::cCOUNTRY_COUNT )
			{
				log_warning( 0, "Country not found for wavelist: " << dataTable.fName( ) << ". If this is a common offensive wave move it to CommonWaves.xlsx, otherwise add it to the meta table." );
				continue;
			}

			for( u32 j = 0; j < dataTable.fRowCount( ); ++j )
			{					
				const tStringPtr& rowName = dataTable.fRowName( j );
				const tDataTable* groupTable = dataTableFile.fFindTable( rowName );
				if( groupTable )
				{
					// not a unit, it's a group name
					continue;
				}

				const GameFlags::tUNIT_ID unitID = ( GameFlags::tUNIT_ID )GameFlags::fUNIT_IDValueStringToEnum( rowName );
				if( unitID < GameFlags::cUNIT_ID_COUNT )
				{
					const tFilePathPtr unitPath = tGameApp::fInstance( ).fUnitResourcePath( unitID, unitCountry );
					if( unitPath.fExists( ) )
					{
						const tResourcePtr unitResource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< tSceneGraphFile >( unitPath ) );
						fAddToLoadList( unitResource );
						tGameApp::fInstance( ).fAddLevelResource( unitResource );
					}

					const tFilePathPtr waveIconPath = tGameApp::fInstance( ).fUnitWaveIconPath( unitID, unitCountry );
					if( waveIconPath.fExists( ) )
					{
						log_line( Log::cFlagWaveIcons, "Loaded: " << waveIconPath );
						const tResourcePtr waveIconResource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< Gfx::tTextureFile >( waveIconPath ) );
						fAddToLoadList( waveIconResource );
						tGameApp::fInstance( ).fAddLevelResource( waveIconResource );
					}
				}
			}		
		}
	}

	void tGameLoadAppState::fAddVersusWaveListUnitsToLoadListByCountry( u32 country )
	{
		if( !tGameApp::fInstance( ).fHasWaveListTable( ) )
			return;

		const tDataTableFile& dataTableFile = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableWaveList );
		const tDataTable* metaDataTable = dataTableFile.fFindTable( cStringMetaData );

		for( u32 i = 0; i < dataTableFile.fTableCount( ); ++i )
		{
			const tDataTable& dataTable = dataTableFile.fIndexTable( i );

			if( cStringMetaData == dataTable.fName( ) )
				continue;

			// Look for country info for this wave list.  Use the passed in country if it doesn't exist.
			u32 unitCountry = GameFlags::cCOUNTRY_COUNT;
			if( metaDataTable )
			{
				const u32 rowIndex = metaDataTable->fRowIndex( dataTable.fName( ) );
				if( rowIndex != ~0 )
				{
					tStringPtr countryStr = metaDataTable->fIndexByRowCol<tStringPtr>( rowIndex, tWaveList::cWaveMetaDataCountry );
					u32 newCountry = GameFlags::fCOUNTRYValueStringToEnum( countryStr );
					if( ( newCountry > GameFlags::cCOUNTRY_DEFAULT ) && ( newCountry < GameFlags::cCOUNTRY_COUNT ) )
					{
						if( newCountry == country )
						{
							unitCountry = newCountry;
						}
					}
					else
						log_warning( 0, "Invalid country type in wavelist metatable: " << countryStr );
				}
			}

			if( unitCountry >= GameFlags::cCOUNTRY_COUNT )
			{
				//log_warning( 0, "Country not found for wavelist: " << dataTable.fName( ) << ". If this is a common offensive wave move it to CommonWaves.xlsx, otherwise add it to the meta table." );
				continue;
			}

			for( u32 j = 0; j < dataTable.fRowCount( ); ++j )
			{
				const tStringPtr& rowName = dataTable.fRowName( j );
				const tDataTable* groupTable = dataTableFile.fFindTable( rowName );
				if( groupTable )
				{
					// not a unit, it's a group name
					continue;
				}

				const GameFlags::tUNIT_ID unitID = ( GameFlags::tUNIT_ID )GameFlags::fUNIT_IDValueStringToEnum( rowName );
				if( unitID < GameFlags::cUNIT_ID_COUNT )
				{
					const tFilePathPtr unitPath = tGameApp::fInstance( ).fUnitResourcePath( unitID, unitCountry );
					if( unitPath.fExists( ) )
					{
						const tResourcePtr unitResource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< tSceneGraphFile >( unitPath ) );
						fAddToLoadList( unitResource );
						tGameApp::fInstance( ).fAddLevelResource( unitResource );
					}

					const tFilePathPtr waveIconPath = tGameApp::fInstance( ).fUnitWaveIconPath( unitID, unitCountry );
					if( waveIconPath.fExists( ) )
					{
						log_line( Log::cFlagWaveIcons, "Loaded: " << waveIconPath );
						const tResourcePtr waveIconResource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< Gfx::tTextureFile >( waveIconPath ) );
						fAddToLoadList( waveIconResource );
						tGameApp::fInstance( ).fAddLevelResource( waveIconResource );
					}
				}
			}
		}
	}

	void tGameLoadAppState::fAddVersusUnitGeneratorsToLoadList( )
	{
		tGameApp& gameApp = tGameApp::fInstance( );
		b32 frontEnd = gameApp.fGameMode( ).fIsFrontEnd( );

		const tGameMode& gm = gameApp.fGameMode( );

		if( gm.fIsVersus( ) )
		{
			fAddVersusUnitGeneratorsToLoadListByCountry( mLevelLoadInfo.mCountry );
			fAddVersusUnitGeneratorsToLoadListByCountry( mLevelLoadInfo.mCountry2 );
		}
	}

	void tGameLoadAppState::fAddVersusUnitGeneratorsToLoadListByCountry( u32 country )
	{
		if( !tGameApp::fInstance( ).fHasWaveListTable( ) )
			return;

		const tDataTableFile& dataTableFile = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableWaveList );
		const tDataTable* unitGenPreloadDataTable = dataTableFile.fFindTable( cStringVersusUnitGenPreload );

		for( u32 unitGeneratorRow = 0; unitGeneratorRow < unitGenPreloadDataTable->fRowCount( ); ++unitGeneratorRow )
		{
			tStringPtr countryStr = unitGenPreloadDataTable->fIndexByRowCol<tStringPtr>( unitGeneratorRow, 0 ); // TODO: convert to constant
			u32 unitGeneratorCountry = GameFlags::fCOUNTRYValueStringToEnum( countryStr );
			if( ( unitGeneratorCountry > GameFlags::cCOUNTRY_DEFAULT ) && ( unitGeneratorCountry < GameFlags::cCOUNTRY_COUNT ) && ( unitGeneratorCountry == country ) )
			{
				const tFilePathPtr unitGeneratorResourcePath = unitGenPreloadDataTable->fIndexByRowCol<tFilePathPtr>( unitGeneratorRow, 1 ); // TODO: convert to constant
				const tResourcePtr unitGeneratorResource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< tSceneGraphFile >( unitGeneratorResourcePath ) );
				fAddToLoadList( unitGeneratorResource );
				tGameApp::fInstance( ).fAddLevelResource( unitGeneratorResource );
			}
		}
	}
}

