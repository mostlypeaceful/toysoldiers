#include "ToolsPch.hpp"
#include "tFilePackageCreator.hpp"
#include "FileSystem.hpp"
#include "tFileWriter.hpp"
#include "tCompressor.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "EndianUtil.hpp"

//for resize function
#include "Gfx/tGeometryFile.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tTextureFile.hpp"

namespace Sig
{
	class tBinaryFileBaseReader : public tBinaryFileBase
	{
	public:
		static Rtti::tClassId fExtractClassId( const tDynamicBuffer& buffer, tPlatformId pid )
		{
			if( buffer.fCount( ) < sizeof(tBinaryFileBase) )
				return 0; // too small, not of tBinaryFileBase

			tFixedArray<Sig::byte,sizeof(tBinaryFileBase)> binFileMem;
			tBinaryFileBase* binFile = ( tBinaryFileBase* )binFileMem.fBegin( );
			fMemCpy( binFile, buffer.fBegin( ), sizeof(*binFile) );

			tSignature expectedSig;
			fCreateSignature( expectedSig, pid );

			if( fMemCmp( &binFile->fSignature( ), &expectedSig, sizeof( tSignature ) ) != 0 )
				return 0; // signature is wrong, not of tBinaryFileBase

			Rtti::tClassId cid = binFile->fFileClassId( );
			EndianUtil::fSwapForTargetPlatform( &cid, sizeof( cid ), pid );
			return cid;
		}

		static b32 fWillResize( const Rtti::tClassId& id  )
		{
			if( id == Gfx::tGeometryFile::cClassId 
				|| id == Gfx::tMaterialFile::cClassId
				|| id == Gfx::tTextureFile::cClassId )
				return true;
			else
				return false;
		}
	};

	const char* tFilePackageCreator::fGetDefaultPackageFileName( )
	{
		return "files";
	}
	const char* tFilePackageCreator::fGetDefaultPackageFileExt( )
	{
		return ".pkg";
	}
	tFilePathPtr tFilePackageCreator::fGetDefaultPackagePath( )
	{
		std::string o = fGetDefaultPackageFileName( );
		o += fGetDefaultPackageFileExt( );
		return tFilePathPtr( o.c_str( ) );
	}

	void tFilePackageCreator::fCreate( tPackagingConfig& config )
	{
		tFileWriter fileWriter( config.mOutputPath );
		if( !fileWriter.fIsOpen( ) )
		{
			log_warning( Log::cFlagFile, "Failed to open package file [" << config.mOutputPath << "] for writing." );
			return;
		}

		if( config.mIncludeFilesInPackageFolder )
		{
			FileSystem::fGetFileNamesInFolder( 
				config.mFilesToAdd, config.mPackageFolder, true, config.mIncludeFilesInPackageFolderRecursive );
		}

		for( u32 i = 0; i < config.mFoldersToAdd.fCount( ); ++i )
		{
			FileSystem::fGetFileNamesInFolder( 
				config.mFilesToAdd, config.mFoldersToAdd[ i ], true, config.mIncludeFilesInPackageFolderRecursive );
		}

		// remove files we should ignore
		for( u32 i = 0; i < config.mFilesToIgnore.fCount( ); ++i )
			config.mFilesToAdd.fFindAndEraseOrdered( config.mFilesToIgnore[ i ] );

		for( u32 i = 0; i < config.mFilesToAdd.fCount( ); ++i )
		{
			b32 del = false;

			// cull zero-sized files
			if( !del && FileSystem::fGetFileSize( config.mFilesToAdd[i] ) == 0 )
				del = true;

			// cull other package files
			if( !del && StringUtil::fCheckExtension( config.mFilesToAdd[i].fCStr( ), fGetDefaultPackageFileExt( ) ) )
				del = true;

			// cull files in dev folders
			if( !del && config.mIgnoreDevFolders && ToolsPaths::fIsUnderDevFolder( config.mFilesToAdd[i] ) )
				del = true;

			// cull files in ignored folders
			for( u32 j = 0; !del && j < config.mFoldersToIgnore.fCount( ); ++j )
			{
				if( _strnicmp( config.mFilesToAdd[i].fCStr( ), config.mFoldersToIgnore[j].fCStr( ), config.mFoldersToIgnore[j].fLength( ) ) == 0 )
					del = true;
			}

			// do custom user-supplied filter function
			if( !del && config.mPathFilter && config.mPathFilter( config.mFilesToAdd[i] ) )
				del = true;

			if( del )
			{
				log_line( 0, "excluding file " << config.mFilesToAdd[ i ] << " from pack file." );
				config.mFilesToAdd.fEraseOrdered( i );
				--i;
			}
		}

		if( config.mFilesToAdd.fCount( )==0 )
		{
			// no files to put in package, return
			log_warning( Log::cFlagFile, "No files to include in package file [" << config.mOutputPath.fCStr( ) << "]; aborting package creation." );
			return;
		}

		// TODO consider a way to specify a more optimal ordering of the files in the package

		// create file headers for each file...

		mFileHeaders.fNewArray( config.mFilesToAdd.fCount( ) );

		for( u32 i = 0; i < config.mFilesToAdd.fCount( ); ++i )
		{
			tFilePathPtr simpleName;
			if( config.mConvertFullPath )
				simpleName = config.mConvertFullPath( config.mFilesToAdd[i], config.mDestPlatform );
			else
				simpleName = config.mFilesToAdd[i];

			mFileHeaders[i].mFileName.fCreateNullTerminated( simpleName.fCStr( ) );
			mFileHeaders[i].mNumRawFileBytesUncompressed = 
				FileSystem::fGetFileSize( config.mFilesToAdd[i] );
			if( config.mCompressFiles )
				mFileHeaders[i].mFlags |= tFilePackageFile::tFileHeader::cCompressed;
		}

		//
		// prior to serialization, be sure to set base binary file settings

		tBinaryFileBase::fSetSignature( config.mDestPlatform, Rtti::fGetClassId<tFilePackageFile>( ), tFilePackageFile::cVersion );

		//
		// save out the file package's table of headers using class serializer

		tLoadInPlaceSerializer serializer;
		mHeaderTableSize = serializer.fSave( *static_cast<tFilePackageFile*>( this ), fileWriter, config.mDestPlatform );

		//
		// now save out each file's actual raw bytes, after compression (if applicable);
		// we use standard file writing (appending using the same fileWriter object as previous);
		// at the end, we will have to re-write the table header, as it will only contain proper
		// offsets after we've written all the actual raw file data.

		const u32 cFileAlignment = 16;
		tFixedArray<u8,16> cDummyValue; cDummyValue.fFill( 0 );

		// compute aligned initial offset for first file (begins after header + padding)
		u32 offsetFromStartOfFile = fAlignHigh( mHeaderTableSize, cFileAlignment );

		// write padding
		fileWriter( &cDummyValue, offsetFromStartOfFile - mHeaderTableSize );

		for( u32 i = 0; i < config.mFilesToAdd.fCount( ); ++i )
		{
			tDynamicBuffer fileBuffer;
			const b32 readSuccess = FileSystem::fReadFileToBuffer( fileBuffer, config.mFilesToAdd[i] );
			sigassert( readSuccess );
			sigassert( fileBuffer.fCount( ) == mFileHeaders[i].mNumRawFileBytesUncompressed );

			mFileHeaders[i].mClassId = tBinaryFileBaseReader::fExtractClassId( fileBuffer, config.mDestPlatform );
			if( tBinaryFileBaseReader::fWillResize( mFileHeaders[ i ].mClassId ) )
				mFileHeaders[i].mFlags |= tFilePackageFile::tFileHeader::cWillResizeAfterLoad;

			u32 numBytesForFile = 0;

			if( mFileHeaders[i].mFlags & tFilePackageFile::tFileHeader::cCompressed )
			{
				// caller wants to compress this file, but there's a chance that after
				// compressing, the size will actually be equivalent or larger...

				tCompressor compressor;
				tDynamicBuffer compressedFileBuffer( fileBuffer.fCount( ) + tCompressor::cCompressionOverhead );
				numBytesForFile = compressor.fCompress( fileBuffer.fBegin( ), fileBuffer.fCount( ), compressedFileBuffer.fBegin( ) );
				sigassert( numBytesForFile <= compressedFileBuffer.fCount( ) );

				if( numBytesForFile < fileBuffer.fCount( ) )
				{
					// write compressed buffer to file
					fileWriter( compressedFileBuffer.fBegin( ), numBytesForFile );
				}
				else
				{
					// might as well not compress the file, as it's larger than the uncompressed version
					mFileHeaders[i].mFlags &= ~tFilePackageFile::tFileHeader::cCompressed;
				}
			}

			if( !(mFileHeaders[i].mFlags & tFilePackageFile::tFileHeader::cCompressed) )
			{
				// write un-compressed buffer to file
				numBytesForFile = fileBuffer.fCount( );
				fileWriter( fileBuffer.fBegin( ), numBytesForFile );
			}

			sigassert( numBytesForFile <= mFileHeaders[i].mNumRawFileBytesUncompressed );

			mFileHeaders[i].mNumRawFileBytes = numBytesForFile;
			mFileHeaders[i].mRawFileOffset = offsetFromStartOfFile;
			offsetFromStartOfFile += numBytesForFile;

			// write padding
			const u32 paddedNumBytesForFile = fAlignHigh( numBytesForFile, cFileAlignment );
			sigassert( paddedNumBytesForFile >= numBytesForFile );
			fileWriter( &cDummyValue, paddedNumBytesForFile - numBytesForFile );

			// increment current file pointer
			offsetFromStartOfFile += paddedNumBytesForFile - numBytesForFile;
		}

		// seek back to the start of the file
		fileWriter.fSeek( 0 );

		// re-write the table of header information
		const size_t tableHeaderSize = serializer.fSave( *static_cast<tFilePackageFile*>( this ), fileWriter, config.mDestPlatform );

		// sanity check
		sigassert( mHeaderTableSize == tableHeaderSize );
	}

}

