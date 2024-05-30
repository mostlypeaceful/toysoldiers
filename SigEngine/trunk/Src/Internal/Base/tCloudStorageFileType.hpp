//------------------------------------------------------------------------------
// \file tCloudStorageFileType.hpp - 7 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tCloudStorageFileType__
#define __tCloudStorageFileType__

namespace Sig
{
	/// \class	tCloudStorageFileType
	/// \brief	Describes Microsoft recognized file types in their REST APIs, such as referenced in the docs
	///			Example docs: https://developer.xboxlive.com/en-us/live/development/documentation/software/Pages/URI_UsersXuidStorageTitleGroupsDataPathandfilename_jan2013.aspx
	enum tCloudStorageFileType
	{
		cCloudStorageFileTypeBinary,	///< ",binary"	postfix/type
		cCloudStorageFileTypeConfig,	///< ",config"	postfix/type
		cCloudStorageFileTypeJson,		///< ",json"	postfix/type

		cCloudStorageFileTypeInvalid = 0xFF,
	};

	/// \brief		Categorize a cloud storage file by remote URL fragment.
	/// \example	fCategorizeUrl( "foo/bar.ext,json" ) == cCloudStorageFileTypeJson
	/// \example	fCategorizeUrl( "foo/bar.ext,binary" ) == cCloudStorageFileTypeBinary
	tCloudStorageFileType fGetCloudStorageFileTypeFromPartialUrl( const char* urlFragment );

	/// \brief		Categorize a cloud storage file by local path fragment.
	/// \example	fCategorizeUrl( "foo\\bar.extj" ) == cCloudStorageFileTypeJson
	/// \example	fCategorizeUrl( "foo\\bar.extb" ) == cCloudStorageFileTypeBinary
	tCloudStorageFileType fGetCloudStorageFileTypeFromPartialLocalPath( const char* localPathFragment );

	/// \brief		Get the type string without the comma from the cloud type	
	/// \example	fGetCloudStoragePostFix( cCloudStorageFileTypeBinary ) == "binary"
	/// \example	fGetCloudStoragePostFix( cCloudStorageFileTypeJson ) == "json"
	std::string fGetCloudStoragePostFix( tCloudStorageFileType type );

	/// \brief		Convert from a remote URL fragment to a local path fragment.
	/// \example	fCloudStorageLocalPathToUrlFragment( "foo\\bar.extj" ) == "foo/bar.ext,json"
	/// \example	fCloudStorageLocalPathToUrlFragment( "foo\\bar.extb" ) == "foo/bar.ext,binary"
	std::string fCloudStoragePartialLocalPathToPartialUrl( const char* localPath );

	/// \brief		Convert from a remote URL fragment to a local path fragment.
	/// \example	fCloudStorageLocalPathToUrlFragment( "foo/bar.ext,json" ) == "foo\\bar.extj"
	/// \example	fCloudStorageLocalPathToUrlFragment( "foo/bar.ext,binary" ) == "foo\\bar.extb"
	tFilePathPtr fCloudStoragePartialUrlToPartialLocalPath( const char* urlFragment );

	/// \brief		Convert from a remote URL fragment to a remote file name
	/// \example	fCloudStorageLocalPathToUrlFragment( "foo/bar.ext,json" ) == "foo/bar.ext"
	/// \example	fCloudStorageLocalPathToUrlFragment( "foo/bar.ext,binary" ) == "foo/bar.ext"
	std::string fCloudStoragePartialUrlToRemoteFilename( const char* urlFragment, u32 urlFragLen = ~0 );
}

#endif //ndef __tCloudStorageFileType__
