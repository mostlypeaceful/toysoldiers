//------------------------------------------------------------------------------
// \file tAnimPackData.cpp - 26 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsGuiPch.hpp"
#include "tAnimPackData.hpp"
#include "FileSystem.hpp"
#include "tAnimPackFile.hpp"
#include "tFileWriter.hpp"
#include "tSkeletonFile.hpp"
#include "tApplication.hpp"

namespace Sig
{
	tAnimPackData::tAnimPackData( const tResourcePtr& resource )
		: mResource( resource ), mDirty( false )
	{
		mAnipkPath = Anipk::fAnibPathToAnipk( mResource->fGetPath( ) );

		mLongLabel = StringUtil::fTrimOversizePath( mAnipkPath.fCStr( ), 48, mAnipkPath.fLength( ) );

		mLabelWithSize = StringUtil::fTrimOversizePath( mAnipkPath.fCStr( ), 22, mAnipkPath.fLength( ) );
		wxString label;
		label.Printf( "%s ~ %.1fmb", mLabelWithSize.c_str( ), 
			FileSystem::fGetFileSize( ToolsPaths::fMakeGameAbsolute( mResource->fGetPath( ) ) )/(1024.f*1024.f) );
		mLabelWithSize = label.c_str( );

		mAnipkFile.fLoadXml( ToolsPaths::fMakeResAbsolute( mAnipkPath ) );
	}



	tAnimPackDataAgent::tAnimPackDataAgent( )
	{
		fLoad( );
	}

	void tAnimPackDataAgent::fRefresh( )
	{
		mAnimPacksBySkeleton.fSetCount( 0 );

		tFilePathPtrList paths;
		FileSystem::fGetFileNamesInFolder(
			paths, 
			ToolsPaths::fGetCurrentProjectResFolder( ),
			true, // full names
			true,  // recurse
			tFilePathPtr( ".anipk") );

		tGrowableArray<tResourcePtr> files;

		// Now kick all the file loads
		u32 pathCount = paths.fCount( );
		for( u32 p = 0; p < pathCount; ++p )
		{
			tResourcePtr resource = tApplication::fInstance( ).fResourceDepot( )->fQuery(
				tResourceId::fMake<tAnimPackFile>( 
				tAnimPackFile::fConvertToBinary( 
				ToolsPaths::fMakeResRelative( paths[ p ] ) ) ) );

			// Resources now come back null when the file doesn't exist.
			if( !resource )
				continue;

			// Start loading all the files
			resource->fLoadDefault( this );
			files.fPushBack(resource);
		}

		// Process them as they load
		while( files.fCount( ) )
		{
			tResourcePtr resource;

			// Find one that's successfully finished
			for( s32 f = files.fCount( ) - 1; f >= 0; --f )
			{
				tResourcePtr res = files[ f ];

				// Still loading
				if( res->fLoading( ) )
					continue;

				// Load failed so destroy it
				if( res->fLoadFailed( ) )
				{
					files.fErase( f );
					continue;
				}

				resource = res;
				files.fErase( f );
				break;
			}

			// No resource so grab any valid one
			if( !resource ){

				for( s32 f = files.fCount( ) - 1; f >= 0; --f )
				{
					tResourcePtr res = files[ f ];
					res->fBlockUntilLoaded( );

					// Failed
					if( res->fLoadFailed( ) )
					{
						files.fErase( f );
						continue;
					}

					resource = res;
					files.fErase( f );
					break;
				}
			}

			sigassert( resource || !files.fCount( ) );

			// Valid resource
			if( resource )
			{
				const tAnimPackFile * packFile = resource->fCast<tAnimPackFile>( );
				sigassert( packFile );

				// Get the list
				tFilePathPtr key = packFile->mSkeletonResource->fGetResourcePtr( )->fGetPath( );
				tAnimPackList * packListPtr = mAnimPacksBySkeleton.fFind( key );

				// No list for this skeleton so build one
				if( !packListPtr )
				{
					mAnimPacksBySkeleton.fPushBack( tAnimPackList( key ) );
					packListPtr = &mAnimPacksBySkeleton.fBack( );
				}

				(*packListPtr).mAnimPacks.fPushBack( resource->fGetPath( ) );

				// add mappings
				tResourcePtr skelResource = tApplication::fInstance( ).fResourceDepot( )->fQuery(
					tResourceId::fMake<tSkeletonFile>( tSkeletonFile::fConvertToBinary( ToolsPaths::fMakeResRelative( key ) ) ) );
				skelResource->fLoadDefault( this );
				skelResource->fBlockUntilLoaded( );

				if( !skelResource->fLoadFailed( ) )
				{
					const tSkeletonFile * skelFile = skelResource->fCast<tSkeletonFile>( ); 
					sigassert( skelFile );
					// Add mapped skeletons for further processing
					const u32 mapCount = skelFile->mSkeletonMaps.fCount( );
					for( u32 m = 0; m < mapCount; ++m )
					{
						packListPtr->mSkeletonMaps.fPushBack( tFilePathPtr( skelFile->mSkeletonMaps[ m ].mSrcSkeletonPath->fGetStringPtr( ) ) );
					}
				}
			}
		}

		fSave( );
	}

	namespace
	{
		static const tFilePathPtr cAnimPackCacheLocation = ToolsPaths::fCreateTempEngineFilePath( ".apd", tFilePathPtr("SigAnim"), "AnimPackData" );
	}

	void tAnimPackDataAgent::fSave( )
	{
		tFileWriter o( cAnimPackCacheLocation );

		tGameArchiveSave save;
		save.fSave( *this, cVersion );
		o( save.fBuffer( ).fBegin( ), save.fBuffer( ).fCount( ) );
	}

	void tAnimPackDataAgent::fLoad( )
	{
		tDynamicBuffer dataBuffer;

		if( !FileSystem::fReadFileToBuffer( dataBuffer, cAnimPackCacheLocation ) )
		{
			fRefresh( );
		}
		else
		{
			tGameArchiveLoad load( dataBuffer.fBegin( ), dataBuffer.fCount( ) );
			load.fLoad( *this );
		}
	}
	namespace
	{
		void fTryAddToPacks( tGrowableArray<tResourcePtr>& animPacks, const tFilePathPtr& packPath )
		{
			const tResourceId rid = tResourceId::fMake<tAnimPackFile>( packPath );
			const tResourcePtr res = tApplication::fInstance( ).fResourceDepot( )->fQuery( rid );
			if( res )
				animPacks.fPushBack( res );
		}
	}

	void tAnimPackDataAgent::fGetAnimPacksForSkeleton( const tFilePathPtr& skelPath, tGrowableArray<tResourcePtr>& animPacks )
	{
		tGrowableArray< tFilePathPtr > processed;
		processed.fPushBack( skelPath );

		// Add anim packs from this skeleton
		const tAnimPackList* list = mAnimPacksBySkeleton.fFind( skelPath );
		
		if( list )
		{
			for( u32 p = 0; p < list->mAnimPacks.fCount( ); ++p )
				fTryAddToPacks( animPacks, list->mAnimPacks[ p ] );

			// now add in mapped skeletons also
			for( u32 m = 0; m < list->mSkeletonMaps.fCount( ); ++m )
			{
				const tFilePathPtr& skel2 = list->mSkeletonMaps[ m ];

				if( processed.fFind( skel2 ) )
					continue;

				processed.fPushBack( skel2 );

				const tAnimPackList* list = mAnimPacksBySkeleton.fFind( skel2 );
				const u32 packCount = list ? list->mAnimPacks.fCount( ) : 0;
				for( u32 p = 0; p < packCount; ++p )
					fTryAddToPacks( animPacks, list->mAnimPacks[ p ] );
			}
		}
	}
}
