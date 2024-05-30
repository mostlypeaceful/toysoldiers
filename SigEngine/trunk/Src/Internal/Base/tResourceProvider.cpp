//------------------------------------------------------------------------------
// \file tResourceProvider.cpp - 31 Jul 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tCloudStorage.hpp"
#include "FileSystem.hpp"
#include "tSceneGraphFile.hpp"
#include "Scripts/tScriptFile.hpp"
#include "Gfx/tTextureFile.hpp"
#include "tFilePackage.hpp"
#include "tFilePackageFile.hpp"

#include "tStandardResourceLoader.hpp"
#include "tDirectResourceLoader.hpp"
#include "tCloudStorageResourceLoader.hpp"

namespace Sig
{
	devvar( bool, Debug_ResourceProviders_PackFileOnly, false );
	devvar( bool, Debug_ResourceProviders_PackFileNever, false );
	devvar( bool, TimeResources, false );
	devvar( f32, TimeResourcesThreshMs, 1.0f );
	b32 fTimeResources( )
	{
		return TimeResources;
	}
	f32 fTimeResourcesThreshMs( )
	{
		return TimeResourcesThreshMs;
	}
}

namespace Sig
{
	//------------------------------------------------------------------------------
	// tResourceProvider
	//------------------------------------------------------------------------------
	tResourceProvider::~tResourceProvider( )
	{
	}

	//------------------------------------------------------------------------------
	void tResourceProvider::fSetRootPath( const tFilePathPtr& rootPath )
	{
		mRootPath = rootPath;
	}





	//------------------------------------------------------------------------------
	// tFileSystemResourceProvider
	//------------------------------------------------------------------------------
	tResourceLoaderPtr tFileSystemResourceProvider::fCreateResourceLoader( const tResourcePtr& resourcePtr )
	{
		tResource::tTypeSpecificProperties* props = tResource::tTypeSpecificPropertiesTable::fInstance( ).fFind( resourcePtr->fGetClassId( ) );
		if( props && ( props->mFlags & tResource::tTypeSpecificProperties::cIsDirectResource )  )
		{
			return tResourceLoaderPtr( NEW_TYPED( tDirectResourceLoader )( props->mOffsetToHeaderSize, resourcePtr.fGetRawPtr( ) ) );
		}
		else
		{
			return tResourceLoaderPtr( NEW_TYPED( tStandardResourceLoader )( resourcePtr.fGetRawPtr( ) ) );
		}
	}

	//------------------------------------------------------------------------------
	b32 tFileSystemResourceProvider::fResourceExists( const tFilePathPtr& path ) const
	{
		if( Debug_ResourceProviders_PackFileOnly )
		{
			const std::string ext = StringUtil::fGetExtension( path.fCStr( ), false );
			if( ext != "sacb" )
				return false; 
		}

		if_logging( Time::tStopWatch sw );

		const b32 exists = FileSystem::fFileExists( tFilePathPtr::fConstructPath( fRootPath( ), path ) );

		if_logging
		( 
			const f32 ms = sw.fGetElapsedMs( );
			if( fTimeResources( ) && ms > fTimeResourcesThreshMs( ) )
				log_line( 0, "tFileSystemResourceProvider::fResourceExists(" << path << ") took " << ms << "ms." );
		)

			return exists;
	}

	//------------------------------------------------------------------------------
	void tFileSystemResourceProvider::fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount ) const
	{
		const tFilePathPtr root = fRootPath( );

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

	//------------------------------------------------------------------------------
	void tFileSystemResourceProvider::fQueryResourcesByFolder( 
		tFilePathPtrList& filePaths, 
		const tFilePathPtr & ext, 
		const tFilePathPtr resourceFolders[], 
		u32 resourceFolderCount ) const
	{
		const tFilePathPtr root = fRootPath( );

		u32 startFile = 0;
		for( u32 i = 0; i < resourceFolderCount; ++i )
		{
			FileSystem::fGetFileNamesInFolder( 
				filePaths, 
				tFilePathPtr::fConstructPath( root, resourceFolders[ i ] ), 
				true, true, 
				ext );

			const u32 fileCount = filePaths.fCount( );
			for( u32 f = startFile; f < fileCount; ++f )
			{
				filePaths[ f ] = tFilePathPtr( StringUtil::fFirstCharAfter( filePaths[ f ].fCStr( ), root.fCStr( ) ) );
			}
			startFile = fileCount;
		}
	}





	//------------------------------------------------------------------------------
	// tFilePackageResourceProvider
	//------------------------------------------------------------------------------
	tFilePackageResourceProvider::tFilePackageResourceProvider( )
	{ }

	//------------------------------------------------------------------------------
	tFilePackageResourceProvider::~tFilePackageResourceProvider( )
	{ }

	//------------------------------------------------------------------------------
	tResourceLoaderPtr tFilePackageResourceProvider::fCreateResourceLoader( const tResourcePtr& resourcePtr )
	{
		sigassert( resourcePtr );
		const tFilePackage* fp = NULL;
		u32 headerIndex = ~0;

		if( !fFindPackage( resourcePtr->fGetPath( ), fp, headerIndex ) )
			return tResourceLoaderPtr( ); // couldn't find any packages to load from

		return fp->fCreateResourceLoader( resourcePtr, fp->fAccessFile( ), headerIndex );
	}

	//------------------------------------------------------------------------------
	b32 tFilePackageResourceProvider::fFindPackage( const tFilePathPtr& path, const tFilePackage*& packageOut, u32& headerIndexOut ) const
	{
		// These are iterated backwards so that file packages added later will take precedence.
		for( s32 ipkg = mFilePackages.fCount( ) - 1; ipkg >= 0; --ipkg )
		{
			const tFilePackage* package = mFilePackages[ ipkg ].fGetRawPtr( );
			u32 headerIndex = package->fAccessFile( )->fFindFile( path );
			if( headerIndex != ~0 )
			{
				packageOut = package;
				headerIndexOut = headerIndex;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------
	void tFilePackageResourceProvider::fRemove( const tGrowableArray< tFilePackagePtr >& packages )
	{
		for( u32 i = 0; i < packages.fCount( ); ++i )
		{
			const b32 found = mFilePackages.fFindAndErase( packages[ i ] );
			log_assert( found, "kinda silly requirement but you should probably only remove what you know is there" );
			log_assert( packages[ i ]->fRefCount( ) == 1, "if this isnt the last ref count then someone else is keeping this package around which could result in stale data" );
		}
	}

	//------------------------------------------------------------------------------
	b32 tFilePackageResourceProvider::fResourceExists( const tFilePathPtr& path ) const
	{
		if( Debug_ResourceProviders_PackFileNever )
			return false;

		if_logging( Time::tStopWatch sw );

		const tFilePackage* fp = NULL;
		u32 headerIndex = ~0;
		const b32 exists = fFindPackage( path, fp, headerIndex );

		if_logging
		( 
			const f32 ms = sw.fGetElapsedMs( );
			if( fTimeResources( ) && ms > fTimeResourcesThreshMs( ) )
			log_line( 0, "tFilePackageResourceProvider::fResourceExists(" << path << ") took " << ms << "ms." );
		)

		return exists;
	}

	//------------------------------------------------------------------------------
	void tFilePackageResourceProvider::fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount ) const
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

	//------------------------------------------------------------------------------
	void tFilePackageResourceProvider::fQueryResourcesByFolder( 
		tFilePathPtrList& filePaths, 
		const tFilePathPtr & ext, 
		const tFilePathPtr resourceFolders[], 
		u32 resourceFolderCount ) const
	{
		for( u32 ipkg = 0; ipkg < mFilePackages.fCount( ); ++ipkg )
		{
			const tFilePackageFile* package = mFilePackages[ ipkg ]->fAccessFile( );

			tFilePathPtrList extList; extList.fPushBack( ext );
			for( u32 i = 0; i < resourceFolderCount; ++i )
				package->fQueryFilePaths( filePaths, resourceFolders[ i ], extList );
		}
	}

	//------------------------------------------------------------------------------
	void tFilePackageResourceProvider::fAddPackage( tFilePackage& package )
	{
		mFilePackages.fPushBack( tFilePackagePtr( &package ) );
	}






	//------------------------------------------------------------------------------
	// tCloudStorageResourceProvider
	//------------------------------------------------------------------------------
	tCloudStorageResourceProvider::tCloudStorageResourceProvider( const iCloudStoragePtr& cloudStorage )
	{
		mCloudStorage = cloudStorage;
	}

	//------------------------------------------------------------------------------
	tCloudStorageResourceProvider::~tCloudStorageResourceProvider( )
	{
	}

	//------------------------------------------------------------------------------
	b32 tCloudStorageResourceProvider::fResourceExists( const tFilePathPtr& path ) const
	{
		if_logging( Time::tStopWatch sw );

		const b32 exists = mCloudStorage->fFileExists( path );

		if_logging
		( 
			const f32 ms = sw.fGetElapsedMs( );
			if( fTimeResources( ) && ms > fTimeResourcesThreshMs( ) )
				log_line( 0, "tCloudStorageResourceProvider::fResourceExists(" << path << ") took " << ms << "ms." );
		)

		return exists;
	}

	//------------------------------------------------------------------------------
	void tCloudStorageResourceProvider::fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount ) const
	{
	}

	//------------------------------------------------------------------------------
	void tCloudStorageResourceProvider::fQueryResourcesByFolder( 
		tFilePathPtrList& filePaths, 
		const tFilePathPtr & ext, 
		const tFilePathPtr resourceFolders[], 
		u32 resourceFolderCount ) const
	{

	}

	//------------------------------------------------------------------------------
	void tCloudStorageResourceProvider::fUpdateForBlockingLoad( )
	{
		Http::tSystem::fInstance( ).fTick( );
	}

	//------------------------------------------------------------------------------
	tResourceLoaderPtr tCloudStorageResourceProvider::fCreateResourceLoader( const tResourcePtr& resourcePtr )
	{
		return tResourceLoaderPtr( NEW_TYPED( tCloudStorageResourceLoader )( resourcePtr.fGetRawPtr( ), mCloudStorage ) );
	}
}
