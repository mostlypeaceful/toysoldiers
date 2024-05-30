#include "ToolsPch.hpp"
#include "Sacml.hpp"
#include "tFilePackageFile.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "tFilePackageCreator.hpp"

namespace Sig { namespace Sacml
{
	const char* fGetFileExtension( )
	{
		return ".sacml";
	}

	b32 fIsSacmlFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	tFilePathPtr fSacmlPathToSacb( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, tFilePackageFile::fGetFileExtension( ) );
	}

	///
	/// \section tFile
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s( "DoFolder", o.mDoFolder );
		s( "Recurse", o.mRecurse );
		s( "Compress", o.mCompress );
		s( "IgnoreDevFiles", o.mIgnoreDevFiles );
		s( "FilesToIgnore", o.mFilesToIgnore );
		s( "FilesToAdd", o.mFilesToAdd );
		s( "FoldersToAdd", o.mFoldersToAdd );
		s( "FoldersToIgnore", o.mFoldersToIgnore );
	}

	tFile::tFile( )
		: mDoFolder( true )
		, mRecurse( true )
		, mCompress( true )
		, mIgnoreDevFiles( true )
	{
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Sacml", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Sacml file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Sacml", *this ) )
		{
			log_warning( "Couldn't load Sacml file [" << path << "]" );
			return false;
		}

		return true;
	}

	namespace
	{
		static tFilePathPtr fConvertPath( const tFilePathPtr& path, tPlatformId pid )
		{
			return ToolsPaths::fMakeGameRelative( path, pid );
		}

		static b32 fFilterPath( const tFilePathPtr& path )
		{
			if( ToolsPaths::fIsUnderSourceFolder( path ) )
				return true;
			if( StringUtil::fCheckExtension( path.fCStr( ), tFilePackageFile::fGetFileExtension( ) ) )
				return true;
			return false;
		}
	}

	void tFile::fGeneratePackFile( const tFilePathPtr& myPath, u32 platforms, const std::string& outputFile )
	{
		tFilePackageCreator pkgCreator;

		const tFilePathPtr myName = fSacmlPathToSacb( tFilePathPtr( StringUtil::fNameFromPath( myPath.fCStr( ) ) ) );
		const tFilePathPtr myFolder = tFilePathPtr( StringUtil::fDirectoryFromPath( myPath.fCStr( ) ) );
		const tFilePathPtr myFolderResRelative = ToolsPaths::fMakeResRelative( myFolder );

		tFilePackageCreator::tPackagingConfig cfg;
		cfg.mCompressFiles = mCompress;
		cfg.mIgnoreDevFolders = mIgnoreDevFiles;
		cfg.mIncludeFilesInPackageFolder = mDoFolder;
		cfg.mIncludeFilesInPackageFolderRecursive = mRecurse;
		cfg.mConvertFullPath = fConvertPath;
		cfg.mPathFilter = fFilterPath;

		for( tPlatformIdIterator pid; !pid.fDone( ); pid.fNext( ) )
		{
			if( !( platforms & fPlatformIdFlag( pid ) ) )
				continue;

			const tFilePathPtr myOutputFolder = ToolsPaths::fMakeGameAbsolute( myFolderResRelative, pid );
			cfg.mPackageFolder = myOutputFolder;
			cfg.mOutputPath = outputFile.length( ) > 0 ? tFilePathPtr( outputFile ) : tFilePathPtr::fConstructPath( myOutputFolder, myName );
			cfg.mFilesToAdd = mFilesToAdd;
			cfg.mFilesToIgnore = mFilesToIgnore;
			cfg.mFoldersToAdd = mFoldersToAdd;
			cfg.mFoldersToIgnore = mFoldersToIgnore;

			// make add/ignore files game absolute
			for( u32 i = 0; i < cfg.mFilesToAdd.fCount( ); ++i )
				cfg.mFilesToAdd[ i ] = ToolsPaths::fMakeGameAbsolute( cfg.mFilesToAdd[ i ], pid );
			for( u32 i = 0; i < cfg.mFilesToIgnore.fCount( ); ++i )
				cfg.mFilesToIgnore[ i ] = ToolsPaths::fMakeGameAbsolute( cfg.mFilesToIgnore[ i ], pid );
			for( u32 i = 0; i < cfg.mFoldersToAdd.fCount( ); ++i )
				cfg.mFoldersToAdd[ i ] = ToolsPaths::fMakeGameAbsolute( cfg.mFoldersToAdd[ i ], pid );
			for( u32 i = 0; i < cfg.mFoldersToIgnore.fCount( ); ++i )
				cfg.mFoldersToIgnore[ i ] = ToolsPaths::fMakeGameAbsolute( cfg.mFoldersToIgnore[ i ], pid );

			cfg.mDestPlatform = pid;
			pkgCreator.fCreate( cfg );
		}
	}

}}

