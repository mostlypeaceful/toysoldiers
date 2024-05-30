#include "BasePch.hpp"
#include "tFilePackage.hpp"
#include "tResourceDepot.hpp"
#include "tFilePackageFile.hpp"
#include "tFilePackageResourceLoader.hpp"
#include "tFilePackageHeaderResourceLoader.hpp"

namespace Sig
{
	tFilePackage::tFilePackage( )
		: mLoadCallerId( 0 )
		, mLoadAllFiles( false )
	{
		mOnLoadComplete.fFromMethod<tFilePackage, &tFilePackage::fOnLoadComplete>( this );
	}

	tFilePackage::~tFilePackage( )
	{
		fUnload( );
	}

	void tFilePackage::fLoad( const tResource::tLoadCallerId& lcid, tResourceDepot& depot, const tFilePathPtr& absPath, b32 ownAllFiles )
	{
		fUnload( );

		depot.mFilePackages.fFindOrAdd( this );

		mLoadCallerId = lcid;
		mLoadAllFiles = ownAllFiles;
		mFileReader = tAsyncFileReader::fCreate( absPath );
		mFilePackage = depot.fQuery( tResourceId::fMake<tFilePackageFile>( absPath ) );
		mFilePackage->fLoad( lcid, tResourceLoaderPtr( NEW tFilePackageHeaderResourceLoader( mFilePackage.fGetRawPtr( ), mFileReader ) ) );
		mFilePackage->fCallWhenLoaded( mOnLoadComplete );
	}

	void tFilePackage::fUnload( )
	{
		if( mFilePackage.fNull( ) )
			return;

		// remove self from resource depot
		mFilePackage->fGetOwner( )->mFilePackages.fFindAndErase( this );

		// cast the resource to a file package file
		const tFilePackageFile* filePackageFile = fAccessFile( );
		sigassert( filePackageFile );

		// go through and unload managed resources
		for( u32 i = 0; i < mSubFiles.fCount( ); ++i )
		{
			mSubFiles[i]->fRemoveFilePackageLink( this );

			// we attempt to unload the file by passing in
			// the load caller id that was passed in to the fLoad
			// method; if in fact this load caller id wasn't a proper
			// load owner for this sub file, then nothing will happen;
			// on the other hand, if this caller id was the owner, then
			// the file be unloaded
			mSubFiles[i]->fUnload( mLoadCallerId );
		}

		mFilePackage->fUnload( mLoadCallerId );
		mFilePackage.fRelease( );
		mFileReader.fRelease( );
		mSubFiles.fDeleteArray( );
		mLoadAllFiles = false;
	}

	void tFilePackage::fBlockUntilLoaded( )
	{
		sigassert( !mFilePackage.fNull( ) );
		tResourceDepot* resDepot = mFilePackage->fGetOwner( );

		while( fLoading( ) )
		{
			resDepot->fUpdateLoadingResources( 1.f );
			fSleep( 1 );
		}
	}

	tResourcePtr tFilePackage::fLoadSubFile( const tResource::tLoadCallerId& lcid, const tResourceId& rid )
	{
		sigassert( !mFilePackage.fNull( ) && mFilePackage->fLoaded( ) && "File package must be loaded prior to loading a sub file" );

		tResourcePtr subFile = mFilePackage->fGetOwner( )->fQuery( rid );

		const tResource::tFilePackageLink fpl = subFile->fGetFilePackageLink( this );
		if( fpl.fNull( ) )
		{
			// apparently this resource isn't contained in this package
			return tResourcePtr( );
		}

		const tFilePackageFile* filePackageFile = fAccessFile( );
		sigassert( filePackageFile );

		// initiate a load for the sub file
		fLoadSubFile( lcid, subFile, filePackageFile, fpl.mIthHeader );

		return subFile;
	}

	void tFilePackage::fUnloadSubFile( const tResourcePtr& subFile )
	{
		sigassert( !mFilePackage.fNull( ) && mFilePackage->fLoaded( ) && "File package must be loaded prior to un-loading a sub file" );
		sigassert( !subFile.fNull( ) );

		const tResource::tFilePackageLink fpl = subFile->fGetFilePackageLink( this );
		if( fpl.fNull( ) )
		{
			// apparently this resource isn't contained in this package
			return;
		}

		subFile->fUnload( this );
	}

	b32 tFilePackage::fQuerySubFilesLoaded( )
	{
		const tFilePackageFile* packageFile = fAccessFile( );
		sigassert( packageFile );

		const u32 numSubFiles = packageFile->fGetNumFiles( );

		for( u32 i = 0; i < numSubFiles; ++i )
		{
			if( !mSubFiles[i]->fIsLoadCaller( mLoadCallerId ) )
				continue;

			if( !mSubFiles[i]->fLoaded( ) )
				return false;
		}

		return true;
	}

	void tFilePackage::fBlockUntilSubFilesLoaded( )
	{
		sigassert( !mFilePackage.fNull( ) );
		tResourceDepot* resDepot = mFilePackage->fGetOwner( );

		while( !fQuerySubFilesLoaded( ) )
		{
			resDepot->fUpdateLoadingResources( 1.f );
			fSleep( 1 );
		}
	}

	const tFilePackageFile* tFilePackage::fAccessFile( ) const
	{
		return mFilePackage.fNull( ) ? 0 : mFilePackage->fCast<tFilePackageFile>( );
	}

	tResourceLoaderPtr tFilePackage::fCreateResourceLoader( const tResourcePtr& resourcePtr, const tFilePackageFile* filePackageFile, u32 ithHeader )
	{
		const tFilePackageFile::tFileHeader& fileHeader = filePackageFile->fGetFileHeader( ithHeader );

		tAsyncFileReaderPtr childFileReader = mFileReader->fSpawnChild( );

#ifdef sig_logging
		childFileReader->fSetDebugContext( std::string( resourcePtr->fGetPath( ).fCStr( ) ) );
#endif//sig_logging

		return tResourceLoaderPtr( NEW tFilePackageResourceLoader( 
			resourcePtr.fGetRawPtr( ),
			childFileReader,
			fileHeader.mRawFileOffset, 
			fileHeader.mNumRawFileBytes,
			fileHeader.mNumRawFileBytesUncompressed,
			fileHeader.mFlags & tFilePackageFile::tFileHeader::cCompressed ) );
	}

	void tFilePackage::fLoadSubFile( const tResource::tLoadCallerId& lcid, const tResourcePtr& resourcePtr, const tFilePackageFile* filePackageFile, u32 ithHeader )
	{
		// TODO test for whether the specified file type is a file package, and instead of
		// loading it directly, store a list of sub packages... ???

		resourcePtr->fLoad( lcid, fCreateResourceLoader( resourcePtr, filePackageFile, ithHeader ) );
	}

	void tFilePackage::fOnLoadComplete( tResource& resource, b32 success )
	{
		// get the depot from the file package resource
		tResourceDepot* depot = resource.fGetOwner( );

		if( !success )
		{
			// remove self from resource depot
			depot->mFilePackages.fFindAndErase( this );

			// release my pointer so as not to leak any memory
			mFilePackage->fUnload( mLoadCallerId );
			mFilePackage.fRelease( );
			return;
		}

		// cast the resource to a file package file
		const tFilePackageFile* filePackageFile = resource.fCast<tFilePackageFile>( );
		sigassert( filePackageFile );

		// create and store resource pointers for all files contained in the file package
		mSubFiles.fNewArray( filePackageFile->fGetNumFiles( ) );
		for( u32 i = 0; i < filePackageFile->fGetNumFiles( ); ++i )
		{
			const tFilePackageFile::tFileHeader& header = filePackageFile->fGetFileHeader( i );
			const tResourceId rid = tResourceId::fMake(
				header.mClassId,
				tFilePathPtr( tFilePathPtr::tGuaranteeTag( ), 
				header.mFileName.fBegin( ) ),
				fTestBits( header.mFlags, tFilePackageFile::tFileHeader::cWillResizeAfterLoad ) );

			mSubFiles[i] = depot->fQuery( rid );

			// register the file header pointer with the resource for fast access
			mSubFiles[i]->fAddFilePackageLink( this, i );

			if( mLoadAllFiles )
			{
				// load the current file
				fLoadSubFile( mLoadCallerId, mSubFiles[i], filePackageFile, i );
			}

			//log_line( 0, "Pack Resource: " << mSubFiles[i]->fGetPath( ) << " sacb: " << resource.fGetPath( ) );
		}
	}

}

