#include "GameAppPch.hpp"
#include "tVersusWaveManager.hpp"
#include "tSinglePlayerWaveList.hpp"
#include "Wwise_IDs.h"

namespace Sig
{

	tOffensiveWaveDesc::tOffensiveWaveDesc()
		: mCost( 0 )
	{
	}

	void tOffensiveWaveDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tOffensiveWaveDesc,Sqrat::DefaultAllocator<tOffensiveWaveDesc> > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("Name"), &tOffensiveWaveDesc::mName)
			.Var(_SC("Desc"), &tOffensiveWaveDesc::mDesc)
			.Var(_SC("Cost"), &tOffensiveWaveDesc::mCost)
			.Var(_SC("Country"), &tOffensiveWaveDesc::mCountry)
			.Var(_SC("WaveID"), &tOffensiveWaveDesc::mWaveID)
			;

		vm.fRootTable( ).Bind( _SC("OffensiveWaveDesc"), classDesc );
	}

}

namespace Sig
{

	namespace
	{
		const tDataTable* fFindTable( const tStringPtr& id )
		{
			tGameApp& game = tGameApp::fInstance( );
			const tDataTableFile& dataTableFile = game.fDataTable( tGameApp::cDataTableWaveList );
			const tDataTableFile& dataTableFileCommon = game.fDataTable( tGameApp::cDataTableCommonWaveList );

			const tDataTable* dt = dataTableFile.fFindTable( id );

			if( !dt || !dt->fRowCount( ) ) 
			{
				dt = dataTableFileCommon.fFindTable( id );
			}

			return dt;
		}
	}

	tVersusWaveManager::tVersusWaveManager( )
	{
		tGameApp& game = tGameApp::fInstance( );

		// mod 2 gives Blue = 0, Red = 1.
		const u32 di = game.fFrontEndPlayer( )->fTeam( ) % 2;// fCountry( ) - 1;
		const u32 si = game.fSecondaryPlayer( )->fTeam( ) % 2;// fCountry( ) - 1;

		tPlayer* player1 = game.fFrontEndPlayer( );
		tPlayer* player2 = game.fSecondaryPlayer( );

		mVersusUi[ di ].fReset( NEW Gui::tVersusWaveList( game.fGlobalScriptResource( tGameApp::cGlobalScriptVersusWaveList ), player1->fUser( ) ) );
		mVersusUi[ si ].fReset( NEW Gui::tVersusWaveList( game.fGlobalScriptResource( tGameApp::cGlobalScriptVersusWaveList ), player2->fUser( ) ) );

		if( game.fGameMode( ).fIsNet( ) )
		{
			mNetVersusUi[ di ].fReset( NEW Gui::tVersusWaveList( game.fGlobalScriptResource( tGameApp::cGlobalScriptVersusWaveList ), player1->fUser( ) ) );
			mNetVersusUi[ si ].fReset( NEW Gui::tVersusWaveList( game.fGlobalScriptResource( tGameApp::cGlobalScriptVersusWaveList ), player2->fUser( ) ) );

			mVersusUi[ di ]->fCanvas( ).fCanvas( )->fSetVirtualMode( player1->fUser( )->fIsViewportVirtual( ) );
			mVersusUi[ si ]->fCanvas( ).fCanvas( )->fSetVirtualMode( player1->fUser( )->fIsViewportVirtual( ) );
			mNetVersusUi[ di ]->fCanvas( ).fCanvas( )->fSetVirtualMode( player2->fUser( )->fIsViewportVirtual( ) );
			mNetVersusUi[ si ]->fCanvas( ).fCanvas( )->fSetVirtualMode( player2->fUser( )->fIsViewportVirtual( ) );

			game.fHudLayer( player1->fHudLayerName( ) ).fToCanvasFrame( ).fAddChild( mVersusUi[ di ]->fCanvas( ) );
			game.fHudLayer( player1->fHudLayerName( ) ).fToCanvasFrame( ).fAddChild( mVersusUi[ si ]->fCanvas( ) );

			game.fHudLayer( player2->fHudLayerName( ) ).fToCanvasFrame( ).fAddChild( mNetVersusUi[ di ]->fCanvas( ) );
			game.fHudLayer( player2->fHudLayerName( ) ).fToCanvasFrame( ).fAddChild( mNetVersusUi[ si ]->fCanvas( ) );
		}
		else
		{
			game.fHudLayer( player1->fHudLayerName( ) ).fToCanvasFrame( ).fAddChild( mVersusUi[ di ]->fCanvas( ) );
			game.fHudLayer( player2->fHudLayerName( ) ).fToCanvasFrame( ).fAddChild( mVersusUi[ si ]->fCanvas( ) );
		}

		mWaveListUI.fRelease( );

		mOffensiveWaveLists[ 0 ] = tWaveListPtr( NEW tWaveList( ) );
		mOffensiveWaveLists[ 1 ] = tWaveListPtr( NEW tWaveList( ) );
		mOffensiveWaveLists[ di ]->fSetUI( mVersusUi[ di ] );
		mOffensiveWaveLists[ si ]->fSetUI( mVersusUi[ si ] );
		mOffensiveWaveLists[ di ]->fSetCountry( player1->fCountry( ) );
		mOffensiveWaveLists[ si ]->fSetCountry( player2->fCountry( ) );
		mOffensiveWaveLists[ di ]->fSetIsVersus( player1 );
		mOffensiveWaveLists[ si ]->fSetIsVersus( player2 );

		mWaveLists.fSetCount( 2 );
		mWaveLists[ di ] = mOffensiveWaveLists[ di ];
		mWaveLists[ si ] = mOffensiveWaveLists[ si ];
	}

	tVersusWaveManager::~tVersusWaveManager( )
	{
	}

	void tVersusWaveManager::fUpdate( f32 dt )
	{
		tWaveManager::fUpdate( dt );
	}

	void tVersusWaveManager::fSetupOffensiveWaveMenu( Gui::tRadialMenuPtr& radialMenu, tPlayer* player )
	{
		Sqrat::Function( radialMenu->fCanvas( ).fScriptObject( ), "DefaultSetup" ).Execute( player );
		
		for( tGrowableArray< tOffensiveWaveDesc >::tIterator i = mOffensiveWaveDescriptions.fBegin( ); i != mOffensiveWaveDescriptions.fEnd( ); ++i )
		{
			const u32 waveCountry = i->mCountry;

			if( player->fCountry( ) == waveCountry )
			{
				const tDataTable* dt = fFindTable( i->mWaveID );
				if( dt )
				{
					u32 count = 0;
					for( u32 row = 0; row < dt->fRowCount( ); ++row )
						count += (u32)dt->fIndexByRowCol<f32>( row, tWaveDesc::cWaveParameterUnitCount );

					// first unitID is sufficient for icons. the icon may contain multiple images.
					const u32 iconUnitID = GameFlags::fUNIT_IDValueStringToEnum( dt->fRowName( 0 ) );

					Sqrat::Function( radialMenu->fCanvas( ).fScriptObject( ), "AddLaunchableWave" ).Execute( (*i), iconUnitID, count );
				}
				else
				{
					log_warning( 0, "could not find offensive wave list: " << i->mWaveID );
					continue;
				}
			}
		}

		Sqrat::Function( radialMenu->fCanvas( ).fScriptObject( ), "FinalizeIconSetup" ).Execute( );
	}

	void tVersusWaveManager::fAddOffensiveWave( const tOffensiveWaveDesc& waveDesc )
	{
		mOffensiveWaveDescriptions.fPushBack( waveDesc );
	}

	b32 tVersusWaveManager::fLaunchOffensiveWave( tOffensiveWaveDesc& waveDesc, tPlayer* player )
	{
		tGameApp& game = tGameApp::fInstance( );
		if( !game.fGameMode( ).fIsVersus( ) )
		{
			log_warning( 0, "Don't call tVersusWaveManager::fLaunchOffensiveWave when not in Versus mode!" );
			return false;
		}

		// Create a wave based on the data from the data table
		const tDataTable* dt = fFindTable( waveDesc.mWaveID );
		if( !dt ) 
			return false;

		// See if this is a barrage
		tWaveListPtr tempList( NEW tWaveList );
		b32 isBarrage = false;
		for( u32 i = 0; i < dt->fRowCount( ); ++i )
		{
			fFillWaveDesc( tempList, *dt, i, true );
			if( tWaveDesc::fIsBarrage( tempList->fWave( i ) ) )
			{
				isBarrage = true;
				break;
			}
		}

		// real wave list stuff
		const u32 index = player->fTeam( ) % 2;// fCountry( ) - 1;
		const tWaveListPtr& realWaveList = mOffensiveWaveLists[ index ];

		// If there are already 3 active waves, do nothing
		if( isBarrage )
		{
			if( !player->fBarrageController( )->fDormant( ) )
				return false;
		}
		else
		{
			if( realWaveList->fNonSimultaneousWaveCount( ) >= 3 )
				return false;
		}

		if( !player->fAttemptPurchase( waveDesc.mCost ) )
			return false;

		if( isBarrage )
		{
			player->fStats( ).fIncStat( GameFlags::cSESSION_STATS_BARRAGES_PURCHASED_IN_VERSUS, 1.f );
			player->fGiveBarrage( false );
			tPlayer* opponent = tGameApp::fInstance( ).fGetPlayerByTeam( player->fEnemyTeam( ) );
			if( opponent )
				opponent->fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_HUD_BARRAGE_OPBOUGHT );
		}
		else
		{
			for( u32 i = 0; i < dt->fRowCount( ); ++i )
			{
				fFillWaveDesc( realWaveList, *dt, i, true );
				realWaveList->fWave( realWaveList->fWaveCount( ) - 1 )->mVersusName = waveDesc.mWaveID;
			}

			if( !realWaveList->fIsActive( ) )
				realWaveList->fActivate( );

			const u32 unitID = GameFlags::fUNIT_IDValueStringToEnum( dt->fRowName( 0 ) );
			if( mVersusUi[ index ] )
				mVersusUi[ index ]->fAddWaveIcon( waveDesc, unitID );
			if( game.fGameMode( ).fIsNet( ) && mNetVersusUi[ index ] ) // repeat for the net one
				mNetVersusUi[ index ]->fAddWaveIcon( waveDesc, unitID );
		}

		return true;
	}

	void tVersusWaveManager::fOnLevelUnloadBegin( )
	{
		tWaveManager::fOnLevelUnloadBegin( );
		for( u32 i = 0; i < mVersusUi.fCount( ); ++i )
		{
			if( mVersusUi[ i ] )
			{
				mVersusUi[ i ]->fCanvas( ).fDeleteSelf( );
				mVersusUi[ i ].fRelease( );
			}
		}
		for( u32 i = 0; i < mNetVersusUi.fCount( ); ++i )
		{
			if( mNetVersusUi[ i ] )
			{
				mNetVersusUi[ i ]->fCanvas( ).fDeleteSelf( );
				mNetVersusUi[ i ].fRelease( );
			}
		}
		for( u32 i = 0; i < mOffensiveWaveLists.fCount( ); ++i )
		{
			mOffensiveWaveLists[ i ].fRelease( );
		}
	}

	tStringPtr tVersusWaveManager::fGetDescWaveID( u32 waveIndex, u32 country )
	{
		//const u32 index = country - 1;
		tGameApp& game = tGameApp::fInstance( );
		const u32 index = ( game.fFrontEndPlayer( )->fCountry( ) == country ) ? game.fFrontEndPlayer( )->fTeam( ) % 2 : game.fSecondaryPlayer( )->fTeam( ) % 2;
		sigassert( index < 2 );
		const tWaveListPtr& realWaveList = mOffensiveWaveLists[ index ];
		sigassert( waveIndex < realWaveList->fWaveCount( ) );
		tWaveDesc* wave = realWaveList->fWave( waveIndex );
		sigassert( wave );

		if( wave->mVersusName.fExists( ) )
			return wave->mVersusName;
		else
			return tStringPtr::cNullPtr;
	}

	void tVersusWaveManager::fRelaunchOffensiveWave( const tStringPtr& waveID, u32 country )
	{
		// Find Wave Desc
		tOffensiveWaveDesc* foundDesc = 0;
		for( u32 i = 0; i < mOffensiveWaveDescriptions.fCount( ); ++i )
		{
			tOffensiveWaveDesc* desc = &mOffensiveWaveDescriptions[ i ];
			if( desc->mWaveID == waveID )
			{
				foundDesc = desc;
				break;
			}
		}

		if( !foundDesc )
		{
			log_warning( 0, "Tried to relaunch offensive wave that doesn't exist: " << waveID );
			return;
		}

		// Create a wave based on the data from the data table
		const tDataTable* dt = fFindTable( foundDesc->mWaveID );
		if( !dt ) 
			return;

		//const u32 index = country - 1;
		tGameApp& game = tGameApp::fInstance( );
		const u32 index = ( game.fFrontEndPlayer( )->fCountry( ) == country ) ? game.fFrontEndPlayer( )->fTeam( ) % 2 : game.fSecondaryPlayer( )->fTeam( ) % 2;
		const tWaveListPtr& realWaveList = mOffensiveWaveLists[ index ];

		// See if this is a barrage
		tWaveListPtr tempList( NEW tWaveList );
		b32 isBarrage = false;
		for( u32 i = 0; i < dt->fRowCount( ); ++i )
		{
			fFillWaveDesc( tempList, *dt, i, true );
			if( tWaveDesc::fIsBarrage( tempList->fWave( i ) ) )
			{
				isBarrage = true;
				break;
			}
		}

		if( !isBarrage )
		{
			for( u32 i = 0; i < dt->fRowCount( ); ++i )
			{
				fFillWaveDesc( realWaveList, *dt, i, true );
				realWaveList->fWave( realWaveList->fWaveCount( ) - 1 )->mVersusName = waveID;
			}

			if( !realWaveList->fIsActive( ) )
				realWaveList->fActivate( );

			const u32 unitID = GameFlags::fUNIT_IDValueStringToEnum( dt->fRowName( 0 ) );
			if( mVersusUi[ index ] )
				mVersusUi[ index ]->fAddWaveIcon( *foundDesc, unitID );
			if( tGameApp::fInstance( ).fGameMode( ).fIsNet( ) && mNetVersusUi[ index ] ) // repeat for the net one
				mNetVersusUi[ index ]->fAddWaveIcon( *foundDesc, unitID );
		}
	}

	void tVersusWaveManager::fReactivateWaveLists( const tGrowableArray< tSaveGameData::tWaveID >& waveData )
	{
		f32 timeLeft[ ] = { 0.0f, 0.0f };
		for( u32 i = 0; i < waveData.fCount( ); ++i )
		{
			// Get time remaining
			const u32 index = waveData[ i ].mCountry - 1;
			timeLeft[ index ] = (f32)waveData[ i ].mActive; // Actually time remaining

			log_line( 0, "Loading:" << waveData[ i ].mWaveName.fCStr( ) << "(" << waveData[ i ].mCountry << ")" );
			fRelaunchOffensiveWave( waveData[ i ].mWaveName, waveData[ i ].mCountry );
		}

		for( u32 i = 0; i < mOffensiveWaveLists.fCount( ); ++i )
		{
			if( mOffensiveWaveLists[ i ]->fWaveCount( ) > 0 )
			{
				mOffensiveWaveLists[ i ]->fReactivateReadying( timeLeft[ i ] );
			}
		}
	}

}