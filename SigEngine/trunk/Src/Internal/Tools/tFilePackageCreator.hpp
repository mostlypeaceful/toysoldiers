#ifndef __tFilePackageCreator__
#define __tFilePackageCreator__
#include "tFilePackageFile.hpp"

namespace Sig
{
	class tools_export tFilePackageCreator : public tFilePackageFile
	{
	public:

		typedef tFilePathPtr (*tConvertFullPath)( const tFilePathPtr& path, tPlatformId pid );
		typedef b32 (*tPathFilter)( const tFilePathPtr& path ); // return true to discard file

		struct tools_export tPackagingConfig
		{
			tPackagingConfig( ) 
				: mDestPlatform( cCurrentPlatform )
				, mConvertFullPath( 0 )
				, mPathFilter( 0 )
				, mIgnoreDevFolders( true )
				, mIncludeFilesInPackageFolder( true )
				, mIncludeFilesInPackageFolderRecursive( false )
				, mCompressFiles( false ) 
			{ }

			tPlatformId					mDestPlatform;
			tConvertFullPath			mConvertFullPath; // provides optional user-callback to modify file names (for example, make relative to 'game' folder)
			tPathFilter					mPathFilter;
			tFilePathPtr				mPackageFolder;
			tFilePathPtr				mOutputPath;
			tFilePathPtrList			mFilesToAdd;
			tFilePathPtrList			mFilesToIgnore;
			tFilePathPtrList			mFoldersToAdd;
			tFilePathPtrList			mFoldersToIgnore;
			b32							mIgnoreDevFolders;
			b32							mIncludeFilesInPackageFolder;
			b32							mIncludeFilesInPackageFolderRecursive;
			b32							mCompressFiles;
		};

		static const char*			fGetDefaultPackageFileName( );
		static const char*			fGetDefaultPackageFileExt( );
		static tFilePathPtr			fGetDefaultPackagePath( );

		void fCreate( tPackagingConfig& config );

	};

}



#endif//__tFilePackageCreator__
