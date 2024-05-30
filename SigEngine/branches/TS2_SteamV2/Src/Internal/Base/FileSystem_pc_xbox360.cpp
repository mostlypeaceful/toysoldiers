#include "BasePch.hpp"
#if defined( platform_msft )
#include "FileSystem.hpp"

#if defined( platform_xbox360 )
#include "XtlUtil.hpp"
#define OsUtil XtlUtil
#else
#include "Win32Util.hpp"
#define OsUtil Win32Util
#endif

namespace Sig { namespace FileSystem
{
	b32 fFileExists( const tFilePathPtr& path )
	{
		HANDLE hFile = OsUtil::fCreateReadOnlyFileHandle( path.fCStr( ), false );
		if( hFile && hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( hFile );
			return true;
		}

		return false;
	}

	b32 fFolderExists( const tFilePathPtr& path )
	{
		HANDLE hFile = OsUtil::fCreateReadOnlyFileHandle( path.fCStr( ), true );
		if( hFile && hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( hFile );
			return true;
		}

		return false;
	}

	b32 fCreateDirectory( const tFilePathPtr& path )
	{
		if( path.fLength( ) == 0 || fFolderExists( path ) )
			return true; // already exists

		// recursively create all sub-directories
		fCreateDirectory( tFilePathPtr( StringUtil::fUpNDirectories( path.fCStr( ), 1 ).c_str( ) ) );

		return OsUtil::fCreateDirectory( path.fCStr( ) );
	}

	b32 fDeleteFile( const tFilePathPtr& path )
	{
		return DeleteFile( path.fCStr( ) ) ? true : false;
	}

	b32 fCopyFile( const tFilePathPtr& srcPath, const tFilePathPtr& dstPath )
	{
		return CopyFile( srcPath.fCStr( ), dstPath.fCStr( ), FALSE ) ? true : false;
	}

	u32 fGetFileSize( const tFilePathPtr& path )
	{
		HANDLE hFile = OsUtil::fCreateReadOnlyFileHandle( path.fCStr( ), false );
		DWORD size = 0;
		if( hFile && hFile != INVALID_HANDLE_VALUE )
		{
			size = GetFileSize( hFile, 0 );
			CloseHandle( hFile );
		}

		return ( u32 )size;
	}

	u64 fGetLastModifiedTimeStamp( const tFilePathPtr& path )
	{
		HANDLE hFile = OsUtil::fCreateReadOnlyFileHandle( path.fCStr( ), false );
		if( !hFile || hFile == INVALID_HANDLE_VALUE )
			return 0;

		FILETIME lastModified={0};
		const BOOL success = GetFileTime( hFile, 0, 0, &lastModified );

		CloseHandle( hFile );

		if( !success )
			return 0;

		u64 out;
		sigassert( sizeof( out ) == sizeof( lastModified ) );
		fMemCpy( &out, &lastModified, sizeof( out ) );
		
		return out;
	}

	b32 fIsAMoreRecentThanB( u64 A, u64 B )
	{
		FILETIME a,b;
		fMemCpy( &a, &A, sizeof( a ) );
		fMemCpy( &b, &B, sizeof( b ) );

		// return whether a is STRICTLY GREATER THAN b (not equal to)
		return CompareFileTime( &a, &b ) > 0;
	}

	void fGetFileNamesInFolder( 
		tFilePathPtrList& output,
		const tFilePathPtr& path,
		b32 fullPathNames,
		b32 recurse,
		const tFilePathPtr& extensionFilter )
	{
		tFixedArray<tFilePathPtr,2> pathFragments;
		pathFragments[0] = path;

		const tFilePathPtr searchPath = tFilePathPtr::fConstructPath( path, tFilePathPtr( "*" ) );

		b32 skipFiles = false;

		WIN32_FIND_DATA wfd;
#ifdef platform_xbox360
		HANDLE hFile = FindFirstFile( searchPath.fCStr( ), &wfd );
#else
		HANDLE hFile = FindFirstFileEx( searchPath.fCStr( ), FindExInfoStandard/*FindExInfoBasic*/, &wfd, FindExSearchNameMatch, NULL, 0/*FIND_FIRST_EX_LARGE_FETCH*/ );
#endif
		if( hFile == INVALID_HANDLE_VALUE )
		{
			// no files that match criteria
			if( recurse )
			{
				// however, because we're recursing, we need to find the folders and then continue
				skipFiles = true;
				const tFilePathPtr foldersSearchPath = tFilePathPtr::fConstructPath( path, tFilePathPtr( "*" ) );
				hFile = FindFirstFile( foldersSearchPath.fCStr( ), &wfd );
			}

			// if the handle is still invalid, then there are no sub folders and no files matching the criteria
			if( hFile == INVALID_HANDLE_VALUE )
				return;
		}

		tGrowableArray<tFilePathPtr> dirs;
		do
		{
			pathFragments[1] = tFilePathPtr( wfd.cFileName );
			const tFilePathPtr newPath = tFilePathPtr::fConstructPath( pathFragments.fBegin( ), pathFragments.fCount( ) );
			const tFilePathPtr newRelPath = pathFragments[1];

			if( wfd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM) )
				continue;

			if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if( !strcmp( wfd.cFileName, ".." ) ||
					!strcmp( wfd.cFileName, "." ) )
					continue;

				if( recurse )
					dirs.fPushBack( newPath );
			}
			else if( !skipFiles )
			{
				if( extensionFilter.fLength( ) == 0 || StringUtil::fCheckExtension( newRelPath.fCStr( ), extensionFilter.fCStr( ) ) )
				{
					if( fullPathNames )
						output.fPushBack( newPath );
					else
						output.fPushBack( newRelPath );
				}
			}

		} while( FindNextFile( hFile, &wfd ) );

		FindClose( hFile );

		for( u32 i = 0; i < dirs.fCount( ); ++i )
			fGetFileNamesInFolder( output, dirs[ i ], fullPathNames, recurse, extensionFilter );
	}

	void fGetFolderNamesInFolder( 
		tFilePathPtrList& output,
		const tFilePathPtr& path,
		b32 fullPathNames,
		b32 recurse )
	{
		tFixedArray<tFilePathPtr,2> pathFragments;
		pathFragments[0] = path;

		const tFilePathPtr searchPath = tFilePathPtr::fConstructPath( path, tFilePathPtr( "*" ) );

		WIN32_FIND_DATA wfd;
#ifdef platform_xbox360
		HANDLE hFile = FindFirstFile( searchPath.fCStr( ), &wfd );
#else
		HANDLE hFile = FindFirstFileEx( searchPath.fCStr( ), FindExInfoStandard/*FindExInfoBasic*/, &wfd, FindExSearchLimitToDirectories, NULL, 0/*FIND_FIRST_EX_LARGE_FETCH*/ );
#endif
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			// no files that match criteria
			return;
		}

		tGrowableArray<tFilePathPtr> dirs;
		do
		{
			pathFragments[1] = tFilePathPtr( wfd.cFileName );
			const tFilePathPtr newPath = tFilePathPtr::fConstructPath( pathFragments.fBegin( ), pathFragments.fCount( ) );
			const tFilePathPtr newRelPath = pathFragments[1];

			if( wfd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM) )
				continue;

			if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if( !strcmp( wfd.cFileName, ".." ) ||
					!strcmp( wfd.cFileName, "." ) )
					continue;

				if( recurse )
					dirs.fPushBack( newPath );

				if( fullPathNames )
					output.fPushBack( newPath );
				else
					output.fPushBack( newRelPath );
			}

		} while( FindNextFile( hFile, &wfd ) );

		FindClose( hFile );

		for( u32 i = 0; i < dirs.fCount( ); ++i )
			fGetFolderNamesInFolder( output, dirs[ i ], fullPathNames, recurse );
	}

	u32 fMaxFileNameLength( tPlatformId platformId )
	{
		switch( platformId )
		{
		case cPlatformXbox360:
			return 40; // Lowest from the Docs
		default:
			return MAX_PATH;
		}
	}

	u32 fMaxPathLength( tPlatformId platformId )
	{
		switch( platformId )
		{
		case cPlatformXbox360:
			return 240; // Lowest from the Docs
		default:
			return MAX_PATH;
		}
	}

}}

#endif//platform_msft

