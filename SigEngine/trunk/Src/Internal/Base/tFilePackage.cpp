#include "BasePch.hpp"
#include "tFilePackage.hpp"
#include "tResourceDepot.hpp"
#include "tFilePackageFile.hpp"
#include "tFilePackageResourceLoader.hpp"
#include "tFilePackageHeaderResourceLoader.hpp"
#include "tFilePackageDirectResourceLoader.hpp"

namespace Sig
{
	tFilePackage::tFilePackage( )
		: mLoadCallerId( 0 )
	{
	}

	tFilePackage::~tFilePackage( )
	{
		fUnload( );
	}

	void tFilePackage::fLoad( const tResource::tLoadCallerId& lcid, tResourceDepot& depot, const tFilePathPtr& relPath )
	{
		fUnload( );

		mLoadCallerId = lcid;
		
		mFilePackage = depot.fQuery( tResourceId::fMake<tFilePackageFile>( relPath ) );
		log_sigcheckfail( mFilePackage, "Could not load file package from " << relPath, return );

		mFileReader = tAsyncFileReader::fCreate( mFilePackage->fAbsolutePhysicalPath( ) );
		mFilePackage->fLoad( lcid, tResourceLoaderPtr( NEW tFilePackageHeaderResourceLoader( mFilePackage.fGetRawPtr( ), mFileReader ) ) );
	}

	void tFilePackage::fUnload( )
	{
		if( mFilePackage.fNull( ) )
			return;

		mFilePackage.fRelease( );
		mFileReader.fRelease( );
	}

	void tFilePackage::fBlockUntilLoaded( )
	{
		sigassert( !mFilePackage.fNull( ) );
		tResourceDepot* resDepot = mFilePackage->fGetOwner( );

		while( fLoading( ) )
		{
			resDepot->fUpdateLoadingResources( );
			fSleep( 1 );
		}
	}

	const tFilePackageFile* tFilePackage::fAccessFile( ) const
	{
		return mFilePackage.fNull( ) ? 0 : mFilePackage->fCast<tFilePackageFile>( );
	}

	tResourceLoaderPtr tFilePackage::fCreateResourceLoader( const tResourcePtr& resourcePtr, const tFilePackageFile* filePackageFile, u32 ithHeader ) const
	{
		const tFilePackageFile::tFileHeader& fileHeader = filePackageFile->fGetFileHeader( ithHeader );

		tAsyncFileReaderPtr childFileReader = mFileReader->fSpawnChild( fileHeader.mRawFileOffset );

#ifdef sig_logging
		childFileReader->fSetDebugContext( std::string( resourcePtr->fGetPath( ).fCStr( ) ) );
#endif//sig_logging

		const tResource::tTypeSpecificProperties* props = 
			tResource::tTypeSpecificPropertiesTable::fInstance( ).fFind( resourcePtr->fGetClassId( ) );

		if( props && props->mFlags & tResource::tTypeSpecificProperties::cIsDirectResource )
		{
			return tResourceLoaderPtr( NEW tFilePackageDirectResourceLoader(
				resourcePtr.fGetRawPtr( ),
				childFileReader,
				props->mOffsetToHeaderSize,
				fileHeader.mRawFileOffset,
				fileHeader.mNumRawFileBytes,
				fileHeader.mNumRawFileBytesUncompressed,
				fileHeader.mFlags & tFilePackageFile::tFileHeader::cCompressed ) );
		}
		else
		{
			return tResourceLoaderPtr( NEW tFilePackageResourceLoader( 
				resourcePtr.fGetRawPtr( ),
				childFileReader,
				fileHeader.mRawFileOffset, 
				fileHeader.mNumRawFileBytes,
				fileHeader.mNumRawFileBytesUncompressed,
				fileHeader.mFlags & tFilePackageFile::tFileHeader::cCompressed ) );
		}
	}

}

