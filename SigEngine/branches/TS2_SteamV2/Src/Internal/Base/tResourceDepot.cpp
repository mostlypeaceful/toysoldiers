#include "BasePch.hpp"
#include "tResourceDepot.hpp"

// for querying common resources
#include "FileSystem.hpp"
#include "tFilePackage.hpp"
#include "tFilePackageFile.hpp"
#include "tSceneGraphFile.hpp"
#include "Scripts/tScriptFile.hpp"
#include "Gfx/tTextureFile.hpp"

namespace Sig
{

	tResourceDepot::tResourceDepot( )
		: mResourceTable( 32 )
	{
	}

	tResourceDepot::~tResourceDepot( )
	{
		for( tResourceTable::tIterator i = mResourceTable.fBegin( ), iend = mResourceTable.fEnd( );
			 i != iend;
			 ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			i->mValue->mOwner = 0;
		}
	}

	void tResourceDepot::fSetRootPath( const tFilePathPtr& rootPath )
	{
		mRootPath = rootPath;
	}

	b32 tResourceDepot::fResourceExistsOnFileSystem( const tFilePathPtr& path ) const
	{
		return FileSystem::fFileExists( tFilePathPtr::fConstructPath( fRootPath( ), path ) );
	}

	b32 tResourceDepot::fResourceExists( const tFilePathPtr& path ) const
	{
		if( mFilePackages.fCount( ) > 0 )
		{
			for( u32 ipkg = 0; ipkg < mFilePackages.fCount( ); ++ipkg )
			{
				const tFilePackageFile* package = mFilePackages[ ipkg ]->fAccessFile( );
				if( package->fFindFile( path ) != ~0 )
					return true;
			}

			return false;
		}
		else
		{
			return FileSystem::fFileExists( tFilePathPtr::fConstructPath( fRootPath( ), path ) );
		}
	}

	tResourcePtr tResourceDepot::fQuery( const tResourceId& rid )
	{
		tResource** find = mResourceTable.fFind( rid );

		if( !find )
		{
			// it doesn't exist, so create it
			find = mResourceTable.fInsert( rid, NEW tResource( rid, this ) );
		}

		sigassert( find );
		return tResourcePtr( *find );
	}

	tResourcePtr tResourceDepot::fQueryLoadBlock( const tResourceId& rid, const tResource::tLoadCallerId& lcid )
	{
		tResourcePtr o = fQuery( rid );
		o->fLoadDefault( lcid );
		o->fBlockUntilLoaded( );
		return o;
	}

	tResourcePtr tResourceDepot::fQueryLoadBlockNoDependencies( const tResourceId& rid, const tResource::tLoadCallerId& lcid )
	{
		tResourcePtr o = fQuery( rid );
		o->fLoadDefault( lcid );
		o->fBlockUnitLoadedNoDependencies( );
		return o;
	}

	void tResourceDepot::fUpdateLoadingResources( f32 timeAllottedMs )
	{
		profile_pix("fUpdateLoadingResources");
		Time::tStopWatch timer;

		for( u32 i = 0; i < mLoadingResources.fCount( ); ++i )
		{
			// in order to prevent initiating "too many" loads at once, we continue to cycle through the first N resources;
			// this is just an empirically-arrived-at number which seems to yield a decent combination of memory/speed etc,
			// with the idea being we want to maximize our IO throughput while not allocating too many read buffers at once
			i = ( i % 16 );

			tResourcePtr resource( mLoadingResources[ i ] );
			sigassert( !resource.fNull( ) );

			const tResourceLoader::tLoadResult loadResult = resource->fUpdateLoad( );
			if( loadResult != tResourceLoader::cLoadPending )
			{
				// load was either successful or failed, either way it's done
				mLoadingResources.fEraseOrdered( i );
				--i;
			}

			// check timer to see if we should continue updating resources
			if( timer.fGetElapsedMs( ) >= timeAllottedMs )
				break;
		}
	}

	void tResourceDepot::fUpdateLoadingOneResource( tResource* res )
	{
		// this will not try to load dependencies, only call it on resources with no dependencies
		tResourcePtr resource( res );
		sigassert( !resource.fNull( ) );

		const tResourceLoader::tLoadResult loadResult = resource->fUpdateLoad( );
		if( loadResult != tResourceLoader::cLoadPending )
		{
			// load was either successful or failed, either way it's done
			mLoadingResources.fFindAndEraseOrdered( resource );
		}
	}

	void tResourceDepot::fQueryByType( Rtti::tClassId cid, tGrowableArray<tResourcePtr>& resources )
	{
		for( tResourceTable::tIterator i = mResourceTable.fBegin( ), iend = mResourceTable.fEnd( );
			 i != iend;
			 ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			if( cid != i->mValue->fGetClassId( ) )
				continue;
			if( !i->mValue->fLoaded( ) )
				continue;
			resources.fPushBack( tResourcePtr( i->mValue ) );
		}
	}

	void tResourceDepot::fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount )
	{
		const tFilePathPtr root = fRootPath( );

		if( mFilePackages.fCount( ) > 0 )
		{
			for( u32 ipkg = 0; ipkg < mFilePackages.fCount( ); ++ipkg )
			{
				const tFilePackageFile* package = mFilePackages[ ipkg ]->fAccessFile( );

				// get all gui/hud textures
				{
					tFilePathPtrList files;
					tFilePathPtrList extList; extList.fPushBack( tFilePathPtr( ".pngb" ) ); extList.fPushBack( tFilePathPtr( ".tgab" ) );
					for( u32 i = 0; i < resourceFolderCount; ++i )
						package->fQueryFilePaths( files, resourceFolders[ i ], extList );
					for( u32 i = 0; i < files.fCount( ); ++i )
						resourceIds.fPushBack( tResourceId::fMake<Gfx::tTextureFile>( files[ i ] ) );
				}
				//sigmls and mshmls
				{
					tFilePathPtrList files;
					tFilePathPtrList extList; extList.fPushBack( tFilePathPtr( ".sigb" ) ); extList.fPushBack( tFilePathPtr( ".mshb" ) );
					for( u32 i = 0; i < resourceFolderCount; ++i )
						package->fQueryFilePaths( files, resourceFolders[ i ], extList );
					for( u32 i = 0; i < files.fCount( ); ++i )
						resourceIds.fPushBack( tResourceId::fMake<tSceneGraphFile>( files[ i ] ) );
				}
				//scripts (nut)
				{
					tFilePathPtrList files;
					tFilePathPtrList extList; extList.fPushBack( tFilePathPtr( ".nutb" ) );
					for( u32 i = 0; i < resourceFolderCount; ++i )
						package->fQueryFilePaths( files, resourceFolders[ i ], extList );
					for( u32 i = 0; i < files.fCount( ); ++i )
						resourceIds.fPushBack( tResourceId::fMake<tScriptFile>( files[ i ] ) );
				}
			}
		}
		else
		{
			// get all gui/hud textures
			{
				tFilePathPtrList files;
				for( u32 i = 0; i < resourceFolderCount; ++i )
				{
					FileSystem::fGetFileNamesInFolder( files, tFilePathPtr::fConstructPath( root, resourceFolders[ i ] ), true, true, tFilePathPtr( ".pngb" ) );
					FileSystem::fGetFileNamesInFolder( files, tFilePathPtr::fConstructPath( root, resourceFolders[ i ] ), true, true, tFilePathPtr( ".tgab" ) );
				}
				for( u32 i = 0; i < files.fCount( ); ++i )
					resourceIds.fPushBack( tResourceId::fMake<Gfx::tTextureFile>( StringUtil::fFirstCharAfter( files[ i ].fCStr( ), root.fCStr( ) ) ) );
			}
			//sigmls and mshmls
			{
				tFilePathPtrList files;
				for( u32 i = 0; i < resourceFolderCount; ++i )
				{
					FileSystem::fGetFileNamesInFolder( files, tFilePathPtr::fConstructPath( root, resourceFolders[ i ] ), true, true, tFilePathPtr( ".sigb" ) );
					FileSystem::fGetFileNamesInFolder( files, tFilePathPtr::fConstructPath( root, resourceFolders[ i ] ), true, true, tFilePathPtr( ".mshb" ) );
				}
				for( u32 i = 0; i < files.fCount( ); ++i )
					resourceIds.fPushBack( tResourceId::fMake<tSceneGraphFile>( StringUtil::fFirstCharAfter( files[ i ].fCStr( ), root.fCStr( ) ) ) );
			}
			//scripts (nut)
			{
				tFilePathPtrList files;
				for( u32 i = 0; i < resourceFolderCount; ++i )
				{
					FileSystem::fGetFileNamesInFolder( files, tFilePathPtr::fConstructPath( root, resourceFolders[ i ] ), true, true, tFilePathPtr( ".nutb" ) );
				}
				for( u32 i = 0; i < files.fCount( ); ++i )
					resourceIds.fPushBack( tResourceId::fMake<tScriptFile>( StringUtil::fFirstCharAfter( files[ i ].fCStr( ), root.fCStr( ) ) ) );
			}
		}
	}

	void tResourceDepot::fReloadResourcesByType( const tGrowableArray<Rtti::tClassId>& cids )
	{
		for( tResourceTable::tIterator i = mResourceTable.fBegin( ), iend = mResourceTable.fEnd( );
			 i != iend;
			 ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			if( !i->mValue->fLoaded( ) )
				continue;
			if( !cids.fFind( i->mValue->fGetClassId( ) ) )
				continue;
			i->mValue->fReload( );
		}
	}

	void tResourceDepot::fRemoveResource( tResource* res )
	{
		sigassert( res );
		sigassert( res->mOwner == this );
		sigassert( !res->mLoader );

		res->mOwner = 0;
		tResource** find = mResourceTable.fFind( res->fGetResourceId( ) );
		sigassert( find );
		mResourceTable.fRemove( find );
	}

	void tResourceDepot::fRemoveLoadingResource( tResource* res )
	{
		mLoadingResources.fFindAndErase( res );
	}

	void tResourceDepot::fAddLoadingResource( tResource* res )
	{
		sigassert( res );
		sigassert( !mLoadingResources.fFind( res ) );
		mLoadingResources.fPushBack( res );
	}

	void tResourceDepot::fReserveAll( const tGrowableArray<Rtti::tClassId>& cids, b32 unreserve )
	{
		for( tResourceTable::tIterator i = mResourceTable.fBegin( ), iend = mResourceTable.fEnd( );
			 i != iend;
			 ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			if( !i->mValue->fLoaded( ) )
				continue;
			if( !cids.fFind( i->mValue->fGetClassId( ) ) )
				continue;

			if( unreserve )
				i->mValue->fUnload( this );
			else
			{
				i->mValue->fAddRef( );
				i->mValue->fAddLoadCallerId( this );
			}
		}
	}

	u32 tResourceDepot::fResourcesWithLoadRefsCount( ) const
	{
		u32 o = 0;
		for( tResourceTable::tConstIterator i = mResourceTable.fBegin( ), iend = mResourceTable.fEnd( );
			 i != iend;
			 ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			if( i->mValue->fGetLoadCallerIds( ).fCount( ) == 0 )
				continue;
			++o;
		}
		return o;
	}
	
}

