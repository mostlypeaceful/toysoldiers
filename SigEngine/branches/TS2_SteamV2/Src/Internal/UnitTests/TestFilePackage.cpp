#include "UnitTestsPch.hpp"
#include "tRandom.hpp"
#include "tFileWriter.hpp"
#include "FileSystem.hpp"
#include "tFilePackageCreator.hpp"
#include "tFilePackage.hpp"
#include "tResourceDepot.hpp"
#include "tLoadInPlaceDeserializer.hpp"


using namespace Sig;
	
void fCreateRandomFiles( )
{
	const u32 numFiles = 121;

	for( u32 i = 0; i < numFiles; ++i )
	{
		const tFilePathPtr path = ToolsPaths::fCreateTempEngineFilePath( );

		tFileWriter fw( path );
		sigassert( fw.fIsOpen( ) );

		tRandom r;
		const u32 numBytes = 11 + fRound<u32>( r.fFloatZeroToOne( ) * 7 * 1024 );
		tDynamicBuffer buf( numBytes );
		fw( buf.fBegin( ), buf.fCount( ) );
	}
}

define_unittest( TestFilePackageFile )
{
	fCreateRandomFiles( );

	tFilePackageCreator pkgCreator;

	tFilePackageCreator::tPackagingConfig cfg;
	cfg.mPackageFolder = ToolsPaths::fGetEngineTempFolder( );
	cfg.mOutputPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), tFilePackageCreator::fGetDefaultPackagePath( ) );

	cfg.mCompressFiles = true;

	pkgCreator.fCreate( cfg );


	tDynamicBuffer dataBuffer;
	FileSystem::fReadFileToBuffer( dataBuffer, cfg.mOutputPath );
	tLoadInPlaceDeserializer des;
	des.fLoad<tFilePackageFile>( dataBuffer.fBegin( ) );

	tFilePackageFile* loadedIn = ( tFilePackageFile* )dataBuffer.fBegin( );
	fAssertEqual( loadedIn->fGetNumFiles( ), pkgCreator.fGetNumFiles( ) );
	fAssertEqual( loadedIn->fGetHeaderTableSize( ), pkgCreator.fGetHeaderTableSize( ) );

	for( u32 i = 0; i < loadedIn->fGetNumFiles( ); ++i )
	{
		fAssertEqual( loadedIn->fGetFileHeader( i ).mLastModifiedTimeStamp, pkgCreator.fGetFileHeader( i ).mLastModifiedTimeStamp );
		fAssertEqual( loadedIn->fGetFileHeader( i ).mClassId, pkgCreator.fGetFileHeader( i ).mClassId );
		fAssertEqual( loadedIn->fGetFileHeader( i ).mFlags, pkgCreator.fGetFileHeader( i ).mFlags );
		fAssertEqual( loadedIn->fGetFileHeader( i ).mRawFileOffset, pkgCreator.fGetFileHeader( i ).mRawFileOffset );
		fAssertEqual( loadedIn->fGetFileHeader( i ).mNumRawFileBytes, pkgCreator.fGetFileHeader( i ).mNumRawFileBytes );
		fAssertEqual( loadedIn->fGetFileHeader( i ).mNumRawFileBytesUncompressed, pkgCreator.fGetFileHeader( i ).mNumRawFileBytesUncompressed );
		fAssert( strcmp( loadedIn->fGetFileHeader( i ).mFileName.fBegin( ), pkgCreator.fGetFileHeader( i ).mFileName.fBegin( ) ) == 0 );
	}

	FileSystem::fDeleteAllFilesInFolder( ToolsPaths::fGetEngineTempFolder( ) );
}

define_unittest( TestFilePackage )
{
	fCreateRandomFiles( );

	tFilePackageCreator pkgCreator;

	tFilePackageCreator::tPackagingConfig cfg;
	cfg.mPackageFolder = ToolsPaths::fGetEngineTempFolder( );
	cfg.mOutputPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), tFilePackageCreator::fGetDefaultPackagePath( ) );

	cfg.mCompressFiles = true;

	pkgCreator.fCreate( cfg );

	tResourceDepot resDepot;

	tFilePackage thePkg;

	thePkg.fLoad( this, resDepot, cfg.mOutputPath, true );

	thePkg.fBlockUntilLoaded( );

	fAssert( thePkg.fLoaded( ) );

	const tFilePackageFile* loadedIn = thePkg.fAccessFile( );

	fAssertEqual( loadedIn->fGetNumFiles( ), pkgCreator.fGetNumFiles( ) );
	fAssertEqual( loadedIn->fGetHeaderTableSize( ), pkgCreator.fGetHeaderTableSize( ) );

	for( u32 i = 0; i < loadedIn->fGetNumFiles( ); ++i )
	{
		fAssertEqual( loadedIn->fGetFileHeader( i ).mLastModifiedTimeStamp, pkgCreator.fGetFileHeader( i ).mLastModifiedTimeStamp );
		fAssertEqual( loadedIn->fGetFileHeader( i ).mClassId, pkgCreator.fGetFileHeader( i ).mClassId );
		fAssertEqual( loadedIn->fGetFileHeader( i ).mFlags, pkgCreator.fGetFileHeader( i ).mFlags );
		fAssertEqual( loadedIn->fGetFileHeader( i ).mRawFileOffset, pkgCreator.fGetFileHeader( i ).mRawFileOffset );
		fAssertEqual( loadedIn->fGetFileHeader( i ).mNumRawFileBytes, pkgCreator.fGetFileHeader( i ).mNumRawFileBytes );
		fAssertEqual( loadedIn->fGetFileHeader( i ).mNumRawFileBytesUncompressed, pkgCreator.fGetFileHeader( i ).mNumRawFileBytesUncompressed );
		fAssert( strcmp( loadedIn->fGetFileHeader( i ).mFileName.fBegin( ), pkgCreator.fGetFileHeader( i ).mFileName.fBegin( ) ) == 0 );
	}

	thePkg.fBlockUntilSubFilesLoaded( );

	fAssert( thePkg.fQuerySubFilesLoaded( ) );

	fAssertEqual( thePkg.fGetNumSubFiles( ), cfg.mFilesToAdd.fCount( ) );

	for( u32 i = 0; i < cfg.mFilesToAdd.fCount( ); ++i )
	{
		tResourcePtr resource = resDepot.fQuery( tResourceId::fMake( 0, cfg.mFilesToAdd[i], false ) );
		fAssert( !resource.fNull( ) );
		fAssert( resource->fLoaded( ) );
		fAssert( !resource->fGetFilePackageLink( &thePkg ).fNull( ) );
		fAssertEqual( resource, thePkg.fGetIthSubFile( i ) );
	}

	thePkg.fUnload( );
	FileSystem::fDeleteAllFilesInFolder( ToolsPaths::fGetEngineTempFolder( ) );
}

