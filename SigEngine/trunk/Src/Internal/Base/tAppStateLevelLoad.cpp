#include "BasePch.hpp"
#include "tAppStateLevelLoad.hpp"
#include "tAppStateDefault.hpp"
#include "tGameAppBase.hpp"
#include "tSgFileRefEntity.hpp"
#include "Gfx/tLightEntity.hpp"
#include "Gfx/tGeometryBufferVRamSlice.hpp"
#include "tSwfFile.hpp"

// for fast reload, reserve these resource types
#include "tSceneGraphFile.hpp"
#include "Gfx/tGeometryFile.hpp"
#include "Gfx/tTextureFile.hpp"
#include "Gfx/tRenderableEntity.hpp"
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
			if( mLoadScreen->fLoadComplete( ) && tGameAppBase::fInstance( ).fBlackCanvasFullAlpha( ) )
			{
				mLoadScreen->fBegin( );
				tGameAppBase::fInstance( ).fFadeFromBlack( 1.0f );
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
				fClearStreamedResources( ); //do this before reserving data for resourcedepot

				if( mIsAssetEquivalent )
				{
					log_line( 0, "Next level is asset-equivalent to previous level: reserving resources so they aren't reloaded..." );
					tGrowableArray<Rtti::tClassId> cids;
					fAcquireReservedResourceTypes( cids );
					tGameAppBase::fInstance( ).fResourceDepot( )->fQueryByTypes( cids, mReservedResources );
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
				fSpawnLevel( );
			}
			else if( fCanDropScreen( ) && mLoadCompleteCount > mFinalLoadCompleteCount && tGameAppBase::fInstance( ).fBlackCanvasZeroAlpha( ) )
			{
				// Give it a few frames to settle before killing the simulation
				if( mLoadCompleteCount < mFinalLoadCompleteCount + 10 )
					++mLoadCompleteCount;
				else
				{
					tGameAppBase::fInstance( ).fSetSimulate( false );
					tGameAppBase::fInstance( ).fSetSimulate( true );
					mLoadScreen->fSetNewLevelIsLoaded( );
					mLoadScreen->fEnd( );
					tGameAppBase::fInstance( ).fFadeToBlack( 1.0f );

					mTimer.fStop( );
					log_line( 0, "Level load time (seconds) = " << mTimer.fGetElapsedS( ) );
				}
			}
			break;
		case Gui::tLoadScreen::cStateEnding:
			if( mLoadScreen->fIsComplete( ) && tGameAppBase::fInstance( ).fBlackCanvasFullAlpha( ) )
			{
				if( mIsAssetEquivalent )
				{
					log_line( 0, "Removing temporary resource load references for level asset-equivalence..." );
					mReservedResources.fSetCount( 0 );
				}

				tGameAppBase::fInstance( ).fFadeFromBlack( 1.0f );

				fOnCompletion( );
			}
			break;
		default:
			break;
		}
	}
	void tAppStateLevelLoad::fClearStreamedResources( )
	{
	}
	void tAppStateLevelLoad::fClearOldResources( )
	{
		tGameAppBase::fInstance( ).fSetDefaultLight( NULL );
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
		log_line( 0, "tAppStateLevelLoad::fOnAllResourcesLoaded (seconds) = " << mTimer.fGetElapsedS( ) );
		++mLoadCompleteCount;
		if( mLoadCompleteCount < mFinalLoadCompleteCount )
			fRetriggerLoad( );
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
		if( !loadScriptPath.fNull( ) )
		{
			const tResourceDepotPtr& depot = tGameAppBase::fInstance( ).fResourceDepot( );
			if( StringUtil::fCheckExtension( loadScriptPath.fCStr( ), ".swf" ) )
				res = depot->fQuery( tResourceId::fMake<tSwfFile>( loadScriptPath ) );
			else
				res = depot->fQuery( tResourceId::fMake<tScriptFile>( loadScriptPath ) );
		}

		mLoadScreen = Gui::tLoadScreenPtr( NEW Gui::tLoadScreen( res ) );
	}

	//------------------------------------------------------------------------------
	void tAppStateLevelLoad::fSpawnLevel( )
	{
		Time::tStopWatch sw;
		if( mLevelResource->fLoaded( ) )
		{
			fOnLevelSpawnBegin( );
			tEntity& rootEntity = tGameAppBase::fInstance( ).fSceneGraph( )->fRootEntity( );
			tSgFileRefEntityPtr	worldEntity = tSgFileRefEntityPtr( NEW tSgFileRefEntity( mLevelResource ) );
			worldEntity->fSpawnImmediate( rootEntity );
			Gfx::tLightEntity* defaultLight = worldEntity->fSpawnDefaultLight( rootEntity, *tGameAppBase::fInstance( ).fScreen( ) );
			tGameAppBase::fInstance( ).fSetDefaultLight( defaultLight );
			Gfx::tGeometryBufferVRamAllocator::fEnableBufferLocking( false );
			fOnLevelSpawnEnd( );
		}
		else
		{
			log_assert( false, "Error trying to load level [" << mLevelResource->fGetPath( ) << "]" );
		}
		log_line( 0, "tAppStateLevelLoad::fSpawnLevel (seconds) = " << sw.fGetElapsedS( ) );
	}

	//------------------------------------------------------------------------------
	void tAppStateLevelLoad::fParseSaveGameData( tGameArchiveLoad & archive )
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

		// Data is stored on disk as encrypted
		if( !archive.fDecrypt( ) )
		{
			log_line( 0, "Could not decrypt save game data" );
		}

		fParseSaveGameData( archive );
	}

}
