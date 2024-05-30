#ifndef __tGameLoadAppState__
#define __tGameLoadAppState__
#include "tAppStateLevelLoad.hpp"
#include "tGameApp.hpp"

namespace Sig
{

	class tGameLoadAppState : public tAppStateLevelLoad
	{
	public:
		explicit tGameLoadAppState( const tLevelLoadInfo& levelLoadInfo, b32 allowLoad = true, b32 allowSpawn = true );
		explicit tGameLoadAppState( 
			const tFilePathPtr& loadScript, 
			const tSaveGameStorageDesc & saveGameDesc, 
			b32 allowLoad = true,
			b32 allowSpawn = true );
		explicit tGameLoadAppState( 
			const tFilePathPtr& loadScript, 
			tGameArchive & saveGameArchive,
			b32 allowLoad = true,
			b32 allowSpawn = true );
		void fCommonInit( );
		virtual void fClearOldResources( );
		virtual void fLoadResources( );
		virtual void fOnLevelSpawnBegin( );
		virtual void fOnLevelSpawnEnd( );
		virtual void fOnCompletion( );
		virtual b32  fCanLoad( ) { return mAllowLoad; }
		virtual b32  fCanSpawn( ) { return mAllowSpawn; }
		void		 fAllowLoad( b32 allow ) { mAllowLoad = allow; }
		void		 fAllowSpawn( b32 allow );
		virtual void fSaveGameLoadError( tSaveGameStorageReader& reader );
	protected:
		virtual void fParseSaveGameData( 
			tGameArchive & archive, 
			tFilePathPtr & mapPath, 
			b32 & isAssetEquivalent );
	private:
		void fAddApplicationResourcesToLoadList( );
		void fAddWaveListToLoadList( );
		void fAddPlayerTurretsToLoadList( );
		void fAddUnitToLoadList( u32 unitID, u32 country );
		void fAddPlayerTurretsToLoadListByCountry( u32 country );
		void fAddWaveListUnitsToLoadList( );
		void fAddWaveListUnitsToLoadListByCountry( u32 country );
		void fAddBarrageToLoadList( u32 country );
	private:
		b32 mAllowLoad;
		b32 mAllowSpawn;
		tLevelLoadInfo mLevelLoadInfo;
	};

}

#endif//__tWorldLoadAppState__
