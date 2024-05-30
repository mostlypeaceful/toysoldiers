#include "GameAppPch.hpp"
#include "tSaveGame.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"

namespace Sig
{
	register_rtti_factory( tEntitySaveData, false );

	const tEntitySaveData* tSaveGame::fEntFromLevel( u32 index ) const
	{
		if( index < mLevelData.mEntsFromLevel.fCount( ) )
			return mLevelData.mEntsFromLevel[ index ].fGetRawPtr( );
		log_warning( 0, "Invalid index to tSaveGame::fEntFromLevel - means your Save Game is corrupt or out of date." );
		return 0;
	}

	void tSaveGame::fOnLevelSpawnEnd( )
	{
		tGameApp::fInstance( ).fCurrentLevel( )->fSetRewindCount( mLevelData.mRewindCount + 1 );

		fSpawnDynamicSavedEnts( );

		tGameApp & gameApp = tGameApp::fInstance( );
		const u32 playerCount = gameApp.fPlayerCount( );
		for( u32 p = 0; p < playerCount; ++p )
		{
			// complimentary to tGameApp::fBuildSaveData
			tPlayer* player = gameApp.fGetPlayer( p );

			tPlayerSaveData & psd = mPlayerData[ p ];

			player->fSetInGameMoney( psd.mMoney);
			player->fSetTicketsLeft( psd.mTickets );
			player->fAddSkippableSeconds( psd.mSkippableSeconds );
			player->fSetWaveKillList( psd.mWaveKills );
			player->fSetSpeedKillList( psd.mSpeedKills );
			player->fSetBombingRunList( psd.mBombingRuns );
			player->fStats( ).fSetStats( psd.mStats );
			player->fBarrageController( )->fReactivateBarrage( psd.mBarrage );
		}

		fActivateWaveLists( );
		tGameApp::fInstance( ).fCurrentLevel( )->fSetPlatformPrice( mLevelData.mPlatformPrice );
	}

	void tSaveGame::fActivateWaveLists( )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( level );

		level->fWaveManager( )->fReactivateWaveLists( mWaveLists );
		if( level->fVersusWaveManager( ) )
			level->fVersusWaveManager( )->fReactivateWaveLists( mOffensiveWaveLists );
	}

	void tSaveGame::fSpawnDynamicSavedEnts( )
	{
		for( u32 i = 0; i < mLevelData.mEntsDynamic.fCount( ); ++i )
		{
			if( mLevelData.mEntsDynamic[ i ]->mDeleted )
				continue; // shouldn't get here, but just in case
			mLevelData.mEntsDynamic[ i ]->fSpawnSavedEntity( );
		}
	}

	u32 tSaveGame::fGetWaveID( const tStringPtr& name ) const
	{
		for( u32 i = 0; i < mWaveLists.fCount( ); ++i )
		{
			if( mWaveLists[ i ].mWaveName == name )
				return mWaveLists[ i ].mWaveID;
		}

		return ~0;
	}
}


namespace Sig
{

	void tSaveGameRewindPreview::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tTurretRewindPreview, Sqrat::NoCopy<tTurretRewindPreview> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("UnitID"),			&tTurretRewindPreview::mUnitID)
				.Var(_SC("Position"),		&tTurretRewindPreview::mPosition)
				.Var(_SC("Country"),		&tTurretRewindPreview::mCountry)
				;
			vm.fRootTable( ).Bind(_SC("SaveGameRewindPreviewTurret"), classDesc);
		}
		{
			Sqrat::Class<tSaveGameRewindPreview, Sqrat::NoCopy<tSaveGameRewindPreview> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("Money"),			&tSaveGameRewindPreview::mMoney)
				.Var(_SC("MoneyPlayer2"),	&tSaveGameRewindPreview::mMoneyPlayer2)
				.Var(_SC("Tickets"),		&tSaveGameRewindPreview::mTickets)
				.Func(_SC("Turret"),		&tSaveGameRewindPreview::fTurret)
				.Prop(_SC("TurretCount"),	&tSaveGameRewindPreview::fTurretCount)
				;
			vm.fRootTable( ).Bind(_SC("SaveGameRewindPreview"), classDesc);
		}
	}

}