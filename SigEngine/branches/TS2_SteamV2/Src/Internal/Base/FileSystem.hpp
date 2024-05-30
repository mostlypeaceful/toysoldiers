#ifndef __FileSystem__
#define __FileSystem__

namespace Sig { namespace FileSystem
{
	base_export b32 fFileExists( const tFilePathPtr& path );
	base_export b32 fFolderExists( const tFilePathPtr& path );
	base_export b32 fCreateDirectory( const tFilePathPtr& path );
	base_export b32 fDeleteFile( const tFilePathPtr& path );
	base_export b32 fCopyFile( const tFilePathPtr& srcPath, const tFilePathPtr& dstPath );
	base_export void fDeleteAllFilesInFolder( const tFilePathPtr& path );

	base_export u32 fGetFileSize( const tFilePathPtr& path );
	base_export u64 fGetLastModifiedTimeStamp( const tFilePathPtr& path );
	base_export b32 fIsAMoreRecentThanB( u64 A, u64 B );
	base_export b32 fIsAMoreRecentThanB( const tFilePathPtr& A, const tFilePathPtr& B );
	base_export b32 fAreAnyInAMoreRecentThanB( const tFilePathPtrList& A, const tFilePathPtr& B );

	base_export b32 fReadFileToBuffer( void* readInto, u32 numBytes, const tFilePathPtr& path );
	base_export b32 fReadFileToBuffer( tDynamicBuffer& bufferOut, const tFilePathPtr& path, const char* append = 0 );
	base_export b32 fReadFileToString( std::string& stringOut, const tFilePathPtr& path, const char* append = 0 );
	base_export b32 fWriteBufferToFile( const tDynamicBuffer& buffer, const tFilePathPtr& path, const char* append = 0 );
	base_export b32 fWriteBufferToFile( const void* data, u32 numBytes, const tFilePathPtr& path, const char* append = 0 );

	base_export void fGetFileNamesInFolder( 
		tFilePathPtrList& output,
		const tFilePathPtr& path,
		b32 fullPathNames=false,
		b32 recurse=false,
		const tFilePathPtr& extensionFilter = tFilePathPtr( ) );

	base_export void fGetFolderNamesInFolder( 
		tFilePathPtrList& output,
		const tFilePathPtr& path,
		b32 fullPathNames=false,
		b32 recurse=false );

	base_export u32 fMaxFileNameLength( tPlatformId platformId = cCurrentPlatform );
	base_export u32 fMaxPathLength( tPlatformId platformId = cCurrentPlatform );

}}

#endif//__FileSystem__

