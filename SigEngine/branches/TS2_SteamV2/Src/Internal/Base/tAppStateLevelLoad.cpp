#include "BasePch.hpp"
#include "tAppStateLevelLoad.hpp"
#include "tAppStateDefault.hpp"
#include "tGameAppBase.hpp"
#include "tSgFileRefEntity.hpp"
#include "Gfx/tLightEntity.hpp"
#include "Gfx/tGeometryBufferVRamSlice.hpp"

// for fast reload, reserve these resource types
#include "tSceneGraphFile.hpp"
#include "Gfx/tGeometryFile.hpp"
#include "Gfx/tTextureFile.hpp"
#include "FX/tFxFile.hpp"

namespace Sig
{
	devvar( bool, Resources_ReserveAllTypes, true );

	namespace
	{
		static void fAcquireReservedResourceTypes( tGrowableArray<Rtti::tClassId>& cids )
		{
			if( Resources_ReserveAllTypes )
			{
				cids.fPushBack( Rtti::fGetClassId<FX::tFxFile>( ) );
				cids.fPushBack( Rtti::fGetClassId<tSceneGraphFile>( ) );
				cids.fPushBack( Rtti::fGetClassId<Gfx::tGeometryFile>( ) );
			}

			cids.fPushBack( Rtti::fGetClassId<Gfx::tTextureFile>( ) );
		}
	}

	//------------------------------------------------------------------------------
	tAppStateLevelLoad::tAppStateLevelLoad( 
		const tFilePathPtr& levelPath, 
		const tFilePathPtr& loadScriptPath, 
		u32 numLoadCalls, 
		b32 isAssetEquivalent )
		: tApplicationState( tGameAppBase::fInstance( ) )
		, mLevelPath( levelPath )
		, mLoadCompleteCount( 0 )
		, mFinalLoadCompleteCount( numLoadCalls )
		, mRetriggerLoad( false )
		, mIsAssetEquivalent( isAssetEquivalent )
		, mLoadingFromSave( false )
	{
		fInitLoadScreen( loadScriptPath );
		Gfx::tGeometryBufferVRamAllocator::fEnableBufferLocking( true );
	}

	//------------------------------------------------------------------------------
	tAppStateLevelLoad::tAppStateLevelLoad( 
		const tUserPtr& user,
		const tSaveGameStorageDesc & saveGameDesc, 
		const tFilePathPtr& loadScriptPath, 
		u32 numLoadCalls)
		: tApplicationState( tGameAppBase::fInstance( ) )
		, mLoadCompleteCount( 0 )
		, mFinalLoadCompleteCount( numLoadCalls )
		, mRetriggerLoad( false )
		, mIsAssetEquivalent( false )
		, mLoadingFromSave( true )
	{
		fInitLoadScreen( loadScriptPath );
		Gfx::tGeometryBufferVRamAllocator::fEnableBufferLocking( true );

		mSaveGameReader.fBeginReading( user, saveGameDesc );
	}

	//------------------------------------------------------------------------------
	tAppStateLevelLoad::tAppStateLevelLoad( 
		tGameArchive & saveGameArchive, 
		const tFilePathPtr& loadScriptPath, 
		u32 numLoadCalls )
		: tApplicationState( tGameAppBase::fInstance( ) )
		, mLoadCompleteCount( 0 )
		, mFinalLoadCompleteCount( numLoadCalls )
		, mRetriggerLoad( false )
		, mIsAssetEquivalent( false )
		, mLoadingFromSave( false )
	{
		fInitLoadScreen( loadScriptPath );
		Gfx::tGeometryBufferVRamAllocator::fEnableBufferLocking( true );
	}

	//------------------------------------------------------------------------------
	void tAppStateLevelLoad::fOnTick( )
	{
		tGameAppBase::fInstance( ).fSetLastLevelLoadTime( fTimer( ).fGetElapsedS( ) );

		switch( mLoadScreen->fState( ) )
		{
		case Gui::tLoadScreen::cStateLoading:
			if( mLoadScreen->fLoadComplete( ) )
			{
				mLoadScreen->fBegin( );
			}
			break;
		case Gui::tLoadScreen::cStateBeginning: 
			// We break the beginning state into two phases
			// if we're loading the level info from file
			if( mLoadingFromSave )
			{
				if( mSaveGameReader.fIsFinished( ) )
				{
					if( mSaveGameReader.fIsErrored( ) )
						fSaveGameLoadError( mSaveGameReader );
					else
						fParseSaveGameData( );
					mLoadingFromSave = false;
				}
			}
			else if( fCanLoad( ) && mLoadScreen->fTryAdvanceToPlaying( ) )
			{
				if( mIsAssetEquivalent )
				{
					log_line( 0, "Next level is asset-equivalent to previous level: reserving resources so they aren't reloaded..." );
					tGrowableArray<Rtti::tClassId> cids;
					fAcquireReservedResourceTypes( cids );
					tGameAppBase::fInstance( ).fResourceDepot( )->fReserveAll( cids );
				}

				fClearOldResources( );
				fLoadResources( );
			}
			break;
		case Gui::tLoadScreen::cStatePlaying:
			if( mRetriggerLoad )
			{
				mRetriggerLoad = false;
				fLoadResources( );
			}
			else if( fCanSpawn( ) && ( mLoadCompleteCount == mFinalLoadCompleteCount ) && mLoadScreen->fCanProceedToNewLevel( ) )
			{
				++mLoadCompleteCount; // so we don't get back in here
				mLoadScreen->fSetNewLevelIsLoaded( ); // May have been missed if we couldn't spawn when all the resources finished
				mLoadScreen->fEnd( );
				fSpawnLevel( );
			}
			break;
		case Gui::tLoadScreen::cStateEnding:
			if( mLoadScreen->fIsComplete( ) )
			{
				if( mIsAssetEquivalent )
				{
					log_line( 0, "Removing temporary resource load references for level asset-equivalence..." );
					tGrowableArray<Rtti::tClassId> cids;
					fAcquireReservedResourceTypes( cids );
					tGameAppBase::fInstance( ).fResourceDepot( )->fReserveAll( cids, true );
				}

				fOnCompletion( );
			}
			break;
		}
	}
	void tAppStateLevelLoad::fClearOldResources( )
	{
		tGameAppBase::fInstance( ).fSetDefaultLight( 0 );
		tGameAppBase::fInstance( ).fSceneGraph( )->fRemoveUnpreserved( );		
		tScriptVm::fInstance( ).fGarbageCollect( );
		Memory::tPool::fCleanAll( );
	}
	void tAppStateLevelLoad::fLoadResources( )
	{
		if( mLoadCompleteCount == mFinalLoadCompleteCount - 1 )
		{
			// reset and remove any previously loaded resources
			fUnloadAll( );
			fAddLevelToLoadList( );
			fLoadAll( );
		}
	}
	void tAppStateLevelLoad::fOnAllResourcesLoaded( )
	{
		++mLoadCompleteCount;
		if( mLoadCompleteCount < mFinalLoadCompleteCount )
			fRetriggerLoad( );
		else if( mLoadCompleteCount == mFinalLoadCompleteCount )
		{
			mTimer.fStop( );
			log_line( 0, "Level load time (seconds) = " << mTimer.fGetElapsedS( ) );
			if( fCanSpawn( ) )
				mLoadScreen->fSetNewLevelIsLoaded( );
		}
	}
	void tAppStateLevelLoad::fOnCompletion( )
	{
		fNewApplicationState( tApplicationStatePtr( NEW tAppStateDefault( ) ) );
	}
	void tAppStateLevelLoad::fAddLevelToLoadList( )
	{
		sigassert( !mLevelResource );
		mLevelResource = tGameAppBase::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< tSceneGraphFile >( mLevelPath ) );
		fAddToLoadList( mLevelResource );
	}

	//------------------------------------------------------------------------------
	void tAppStateLevelLoad::fInitLoadScreen( const tFilePathPtr& loadScriptPath )
	{
		tResourcePtr res;
		if( !loadScriptPath.fNull( ) ) {
			res = tGameAppBase::fInstance( ).fResourceDepot( )->fQuery( 
						tResourceId::fMake<tScriptFile>( loadScriptPath ) );
		}

		mLoadScreen = Gui::tLoadScreenPtr( NEW Gui::tLoadScreen( res ) );
	}

	//------------------------------------------------------------------------------
	void tAppStateLevelLoad::fSpawnLevel( )
	{
		if( mLevelResource->fLoaded( ) )
		{
			tEntity& rootEntity = tGameAppBase::fInstance( ).fSceneGraph( )->fRootEntity( );
			fOnLevelSpawnBegin( );
			tSgFileRefEntityPtr	worldEntity = tSgFileRefEntityPtr( NEW tSgFileRefEntity( mLevelResource ) );
			worldEntity->fSpawnImmediate( rootEntity );
			fOnLevelSpawnEnd( );
			Gfx::tLightEntity* defaultLight = worldEntity->fSpawnDefaultLight( rootEntity, tGameAppBase::fInstance( ).fScreen( ) );
			tGameAppBase::fInstance( ).fSetDefaultLight( defaultLight );
			Gfx::tGeometryBufferVRamAllocator::fEnableBufferLocking( false );
		}
		else
		{
			log_warning( 0,
				"Error trying to load level [" << mLevelResource->fGetPath( ) << "] - the game is probably going to hang out on a black screen until you reboot." );
		}
	}

	//------------------------------------------------------------------------------
	void tAppStateLevelLoad::fParseSaveGameData( tGameArchive & archive )
	{
		// This expects the data to be decrypted
		// Parse the save game and retrieve the level path and asset equivalence
		fParseSaveGameData( archive, mLevelPath, mIsAssetEquivalent );
	}

	//------------------------------------------------------------------------------
	void tAppStateLevelLoad::fParseSaveGameData( )
	{
		sigassert( mLoadingFromSave );
		
		tDynamicArray<byte> buffer;
		mSaveGameReader.fGetBuffer( buffer );

		tGameArchiveLoad archive( buffer.fBegin( ), buffer.fCount( ) );

		//// Data is stored on disk as encrypted
		//if( !archive.fDecrypt( ) )
		//{
		//	log_line( 0, "Could not decrypt save game data" );
		//}

		fParseSaveGameData( archive );
	}

}
