#include "BasePch.hpp"
#if defined( platform_ios )
#include "FileSystem.hpp"
#include "tFileReader.hpp"
#include <dirent.h>
#include <sys/stat.h>

namespace Sig { namespace FileSystem
{
	b32 fFileExists( const tFilePathPtr& path )
	{
		tFileReader f( path );
		return f.fIsOpen( );
	}

	b32 fFolderExists( const tFilePathPtr& path )
	{
		DIR* dir = opendir( path.fCStr( ) );
		if( dir )
		{
			closedir( dir );
			return true;
		}
		return false;
	}

	b32 fCreateDirectory( const tFilePathPtr& path )
	{
		if( !path.fExists( ) || fFolderExists( path ) )
			return true; // already exists

		// recursively create all sub-directories
		fCreateDirectory( tFilePathPtr( StringUtil::fUpNDirectories( path.fCStr( ), 1 ).c_str( ) ) );

		const int result = mkdir( path.fCStr( ), 00777 );
		return result == 0;
	}

	b32 fDeleteFile( const tFilePathPtr& path )
	{
		return unlink( path.fCStr( ) ) == 0;
	}

	b32 fCopyFile( const tFilePathPtr& srcPath, const tFilePathPtr& dstPath )
	{
		log_warning_unimplemented( );
		return false;
	}

	u32 fGetFileSize( const tFilePathPtr& path )
	{
		struct stat s;
		const int result = stat( path.fCStr( ), &s );
		if( result == 0 )
			return s.st_size;
		return 0;
	}

	u64 fGetLastModifiedTimeStamp( const tFilePathPtr& path )
	{
		u64 out = ~0;
		
		struct stat s;
		const int result = stat( path.fCStr( ), &s );
		if( result == 0 )
		{
			static_assert( sizeof( s.st_mtime ) <= sizeof( u64 ) );
			fZeroOut( out );
			fMemCpy( &out, &s.st_mtime, sizeof( s.st_mtime ) );
		}
		
		return out;
	}

	b32 fIsAMoreRecentThanB( u64 A, u64 B )
	{
		time_t a,b;
		fMemCpy( &a, &A, sizeof( a ) );
		fMemCpy( &b, &B, sizeof( b ) );
		return difftime( a, b ) > 0.0;
	}

	void fGetFileNamesInFolder( 
		tFilePathPtrList& output,
		const tFilePathPtr& path,
		b32 fullPathNames,
		b32 recurse,
		const tFilePathPtr& extensionFilter )
	{
		DIR* dir = opendir(path.fCStr( ));
		if (!dir)
			return;

		for ( dirent* entry = readdir(dir) ; entry != NULL ; entry = readdir(dir) )
		{
			if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,".."))
				continue;
			// consider skipping "hidden" dot-files like ".blah" as well

			const tFilePathPtr relPath( entry->d_name );
			const tFilePathPtr absPath = tFilePathPtr::fConstructPath( path, relPath );

			const b32 isFile = fFileExists( absPath );
			const b32 isMatchingFile = isFile && ( extensionFilter.fLength( )==0 || StringUtil::fCheckExtension( relPath.fCStr( ), extensionFilter.fCStr( ) ) );

			if ( isMatchingFile )
				output.fPushBack( fullPathNames ? absPath : relPath );

			const b32 isDir = fFolderExists( absPath );

			if ( recurse && isDir )
				fGetFileNamesInFolder( output, absPath, fullPathNames, recurse, extensionFilter );
		}

		closedir(dir);
	}

	void fGetFolderNamesInFolder( 
		tFilePathPtrList& output,
		const tFilePathPtr& path,
		b32 fullPathNames,
		b32 recurse )
	{
		DIR* dir = opendir(path.fCStr( ));
		if (!dir)
			return;

		for ( dirent* entry = readdir(dir) ; entry != NULL ; entry = readdir(dir) )
		{
			if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,".."))
				continue;
			// consider skipping "hidden" dot-files like ".blah" as well

			const tFilePathPtr relPath( entry->d_name );
			const tFilePathPtr absPath = tFilePathPtr::fConstructPath( path, relPath );

			const b32 isDir = fFolderExists( absPath );

			if ( isDir )
				output.fPushBack( fullPathNames ? absPath : relPath );

			if ( recurse && isDir )
				fGetFolderNamesInFolder( output, absPath, fullPathNames, recurse );
		}

		closedir(dir);
	}

	u32 fMaxFileNameLength( tPlatformId platformId )
	{
		return NAME_MAX;
	}

	u32 fMaxPathLength( tPlatformId platformId )
	{
		return PATH_MAX;
	}

}}

#endif//platform_ios

