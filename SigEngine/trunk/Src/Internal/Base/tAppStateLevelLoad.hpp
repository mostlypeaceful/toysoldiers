#ifndef __tAppStateLevelLoad__
#define __tAppStateLevelLoad__

#include "tApplication.hpp"
#include "tResourceLoadList.hpp"
#include "tUser.hpp"
#include "tSaveGameStorage.hpp"
#include "tGameArchive.hpp"
#include "Gui/tLoadScreen.hpp"

namespace Sig
{
	class base_export tAppStateLevelLoad : public tApplicationState, public tResourceLoadList
	{
	public:
		tAppStateLevelLoad( 
			const tFilePathPtr& levelPath, 
			const tFilePathPtr& loadScriptPath, 
			u32 numLoadCalls = 1, 
			b32 isAssetEquivalent = false );
		tAppStateLevelLoad( 
			const tUserPtr& user,
			const tSaveGameStorageDesc & saveGameDesc, 
			const tFilePathPtr& loadScriptPath, 
			u32 numLoadCalls = 1 );
		tAppStateLevelLoad( 
			tGameArchive & saveGameArchive, 
			const tFilePathPtr& loadScriptPath, 
			u32 numLoadCalls = 1 );
		virtual void fOnTick( );
		virtual void fClearStreamedResources( );
		virtual void fClearOldResources( );
		virtual void fLoadResources( ); // also for derived types, call base only if you want default functionality
		virtual void fOnAllResourcesLoaded( );
		virtual void fOnLevelSpawnBegin( ) { } // notification for derived types
		virtual void fOnLevelSpawnEnd( ) { } // notification for derived types
		virtual void fOnCompletion( ); // default moves to simulation app state
		virtual b32  fCanLoad( ) { return true; }
		virtual b32  fCanSpawn( ) { return true; }
		virtual b32  fCanDropScreen( ) { return true; }
		virtual void fSaveGameLoadError( tSaveGameStorageReader& reader ) { } // reload the front end and display a message
	public:
		const Time::tStopWatch& fTimer( ) const { return mTimer; }
	protected:

		// Derivations are responsible for parsing the save information in the archive
		// and returning the file path
		virtual void fParseSaveGameData( 
			tGameArchiveLoad & archive, 
			tFilePathPtr & mapPath, 
			b32 & isAssetEquivalent ) = 0;

		void fParseSaveGameData( tGameArchiveLoad & archive );

		const Gui::tLoadScreenPtr& fLoadScreen( ) const { return mLoadScreen; }
		u32 fLoadCompleteCount( ) const { return mLoadCompleteCount; }
		void fAddLevelToLoadList( );
	private:
		void fInitLoadScreen( const tFilePathPtr& loadScriptPath );
		void fRetriggerLoad( ) { mRetriggerLoad = true; }
		void fSpawnLevel( );
		void fParseSaveGameData( );
	private:
		Time::tStopWatch		mTimer;
		tFilePathPtr			mLevelPath;
		tResourcePtr			mLevelResource;
		Gui::tLoadScreenPtr		mLoadScreen;
		tSaveGameStorageReader  mSaveGameReader;
		u32						mLoadCompleteCount;
		u32						mFinalLoadCompleteCount;
		b32						mRetriggerLoad;
		b32						mIsAssetEquivalent;
		b32						mLoadingFromSave;
		tGrowableArray<tResourcePtr> mReservedResources;
	};
}

#endif//__tAppStateLevelLoad__
