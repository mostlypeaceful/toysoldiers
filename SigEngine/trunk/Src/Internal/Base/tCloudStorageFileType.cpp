//------------------------------------------------------------------------------
// \file tCloudStorageFileType.cpp - 7 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tCloudStorageFileType.hpp"

namespace Sig
{
	tCloudStorageFileType fGetCloudStorageFileTypeFromPartialUrl( const char* urlFragment )
	{
		if		( StringUtil::fEndsWithI( urlFragment, ",json" ) )		return cCloudStorageFileTypeJson;
		else if	( StringUtil::fEndsWithI( urlFragment, ",config" ) )	return cCloudStorageFileTypeConfig;
		else if	( StringUtil::fEndsWithI( urlFragment, ",binary" ) )	return cCloudStorageFileTypeBinary;

		sigassert( !"URL fragment contains no valid type postfix" );
		return cCloudStorageFileTypeInvalid;
	}

	tCloudStorageFileType fGetCloudStorageFileTypeFromPartialLocalPath( const char* localPathFragment )
	{
		if		( StringUtil::fEndsWithI( localPathFragment, "j" ) )	return cCloudStorageFileTypeJson;
		else if	( StringUtil::fEndsWithI( localPathFragment, "c" ) )	return cCloudStorageFileTypeConfig;
		else if	( StringUtil::fEndsWithI( localPathFragment, "b" ) )	return cCloudStorageFileTypeBinary;

		sigassert( !"Local path extension postfix invalid" );
		return cCloudStorageFileTypeInvalid;
	}

	std::string fGetCloudStoragePostFix( tCloudStorageFileType type )
	{
		switch( type )
		{
		case cCloudStorageFileTypeBinary: return "binary";
		case cCloudStorageFileTypeConfig: return "config";
		case cCloudStorageFileTypeJson: return "json";
		default: sig_nodefault( );
		}

		return std::string( );
	}

	std::string fCloudStoragePartialLocalPathToPartialUrl( const char* localPath )
	{
		std::string t( localPath );
		sigcheckfail( t.size( ) > 0, return "" );
		t.pop_back( );
		StringUtil::fReplaceAllOf( t, "\\", "/" );

		return t;
	}

	tFilePathPtr fCloudStoragePartialUrlToPartialLocalPath( const char* urlFragment )
	{
		std::string t( urlFragment );
		const std::size_t comma = t.find_last_of(',');
		sigcheckfail( comma != std::string::npos, return tFilePathPtr::cNullPtr );
		t = t.substr( 0, comma );

		switch( fGetCloudStorageFileTypeFromPartialUrl( urlFragment ) )
		{
		case cCloudStorageFileTypeBinary:	return tResource::fConvertPathAddB( tFilePathPtr( t ) );
		case cCloudStorageFileTypeConfig:	return tFilePathPtr( t + "c" );
		case cCloudStorageFileTypeJson:		return tFilePathPtr( t + "j" );
		default:							return tFilePathPtr::cNullPtr; // Already asserted in fGetCloudStorageFileTypeFromUrlFragment...
		}
	}

	std::string fCloudStoragePartialUrlToRemoteFilename( const char* urlFragment, u32 urlFragLen )
	{
		if( urlFragLen == ~0 )
			urlFragLen = StringUtil::fStrLen( urlFragment );

		if		( StringUtil::fEndsWithI( urlFragment, ",json" ) )		return std::string( urlFragment, urlFragment + urlFragLen - 5 ) ;
		else if	( StringUtil::fEndsWithI( urlFragment, ",config" ) )	return std::string( urlFragment, urlFragment + urlFragLen - 7 ) ;
		else if	( StringUtil::fEndsWithI( urlFragment, ",binary" ) )	return std::string( urlFragment, urlFragment + urlFragLen - 7 ) ;
		else return std::string( urlFragment );
	}
}
