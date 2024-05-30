//------------------------------------------------------------------------------
// \file tCloudStorage.cpp - 17 Jul 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tCloudStorage.hpp"
#include "tUrlEncode.hpp"

#if defined( platform_xbox360 )
#include <xgetserviceendpoint.h>
#endif

namespace Sig
{
	namespace
	{
		//--------------------------------------------------------------------------------------
		// Constants
		//--------------------------------------------------------------------------------------
		const tStringPtr cFieldSize( "size" );
		const tStringPtr cFieldEtag( "etag" );
		const tStringPtr cFieldFileName( "fileName" );

		//--------------------------------------------------------------------------------------
		// Helper functions
		//--------------------------------------------------------------------------------------
		Time::tDateTime fGetClientFileDateTime( const std::string& clientFileTimeStr )
		{
			if( !clientFileTimeStr.empty( ) )
			{
				char delim = 0;
				u32 year, month, day, hour, minute, second;
				std::stringstream ss( clientFileTimeStr );
				ss >> year >> delim;
				ss >> month >> delim;
				ss >> day >> delim;
				ss >> hour >> delim;
				ss >> minute >> delim;
				ss >> second >> delim;

				return Time::tDateTime::fFromJulianDateMilitaryTime( year, month, day, hour, minute, second );
			}
			else
			{
				return Time::tDateTime::fMin( );
			}
		}
	}

	//------------------------------------------------------------------------------
	// iCloudStorage
	//------------------------------------------------------------------------------
	const tStringPtr iCloudStorage::cFieldClientFileTime( "clientFileTime" );

	//------------------------------------------------------------------------------
	// iCloudStorage::tFileBlob
	//------------------------------------------------------------------------------
	iCloudStorage::tFileBlob::tFileBlob( )
		: mSize( 0 )
	{ }

	//------------------------------------------------------------------------------
	b32 iCloudStorage::tFileBlob::fLoad( tJsonReader& reader )
	{
		tVarPropertyBag fields;
		if( reader.fGetObject( fields ) )
		{
			std::string fileTimeStr;
			if( tVarProperty* fileTimeProp = fields.fFind( cFieldClientFileTime ) )
				fileTimeStr = *fileTimeProp->fValueAs<std::string>( );
			mTimeStamp = fGetClientFileDateTime( fileTimeStr );

			if( tVarProperty* sizeProp = fields.fFind( cFieldSize ) )
				mSize = *sizeProp->fValueAs<s32>( );
			if( tVarProperty* etagProp = fields.fFind( cFieldEtag ) )
				mEtag = *etagProp->fValueAs<std::string>( );
			if( tVarProperty* fileNameProp = fields.fFind( cFieldFileName ) )
				mFileName = *fileNameProp->fValueAs<std::string>( );

			// mFileName is formatted as "{path/file},binary" or "{path/file},json"
			// We need mFilePath formatted as "{path\file}b" or "{path\file}j".
			mFilePath = fCloudStoragePartialUrlToPartialLocalPath( mFileName.c_str( ) );
			mFileType = fGetCloudStorageFileTypeFromPartialUrl( mFileName.c_str( ) );
			mFileName = fCloudStoragePartialUrlToRemoteFilename( mFileName.c_str( ), mFileName.length( ) );
			return true;
		}
		else
		{
			return false;
		}
	}

	//------------------------------------------------------------------------------
	std::string iCloudStorage::tFileBlob::fFileNameWithPostfix( ) const
	{
		std::stringstream ss;
		ss << mFileName << "," << fGetCloudStoragePostFix( mFileType );
		return ss.str( );
	}

	//------------------------------------------------------------------------------
	void iCloudStorage::tFileBlob::fSetFromPathAndSize( const tFilePathPtr& filePath, u32 size )
	{
		mFilePath = filePath;
		mSize = size;

		// mFilePath is formatted as "{path\file}b" or "{path\file}j".
		// We need mFileName formatted as "{path/file}" or "{path/file}".
		mFileName = fCloudStoragePartialLocalPathToPartialUrl( filePath.fCStr( ) );
		mFileType = fGetCloudStorageFileTypeFromPartialLocalPath( filePath.fCStr( ) );
	}

	//------------------------------------------------------------------------------
	// iCloudStorage::tFileBlobList
	//------------------------------------------------------------------------------
	b32 iCloudStorage::tFileBlobList::fLoad( const byte* buffer, u32 bufferSize )
	{
		fSetCount( 0 );

		tJsonReader reader( ( const char* )buffer, bufferSize );
		if( reader.fBeginObject( ) && reader.fGetField( "blobs" ) )
		{
			if( reader.fBeginArray( ) ) do
			{
				tFileBlob newItem;
				if( newItem.fLoad( reader ) )
					fPushBack( newItem );
				else
					break;

			} while( true );
		}

		return true;
	}

	//------------------------------------------------------------------------------
	// iCloudStorage::tQuotaInfo
	//------------------------------------------------------------------------------
	iCloudStorage::tQuotaInfo::tQuotaInfo( )
		: mQuotaBytesForUnverifiedStorage( 0 )
		, mQuotaBytesForVerifiedStorage( 0 )
		, mUsedBytesInUnverifiedStorage( 0 )
		, mUsedBytesInVerifiedStorage( 0 )
	{
	}

	//------------------------------------------------------------------------------
	b32 iCloudStorage::tQuotaInfo::fLoad( const byte* buffer, u32 bufferSize )
	{
		tJsonReader reader( ( const char* )buffer, bufferSize );
		if( reader.fBeginObject( ) && reader.fGetField( "quotaInfo" ) )
		{
			return(
				reader.fBeginObject( ) &&
				reader.fGetField( "QuotaBytesForUnverifiedStorage", mQuotaBytesForUnverifiedStorage ) &&
				reader.fGetField( "QuotaBytesForVerifiedStorage", mQuotaBytesForVerifiedStorage ) &&
				reader.fGetField( "UsedBytesInUnverifiedStorage", mUsedBytesInUnverifiedStorage ) &&
				reader.fGetField( "UsedBytesInVerifiedStorage", mUsedBytesInVerifiedStorage ) &&
				reader.fEndObject( )
				);
		}

		return false;
	}

} // ::Sig
