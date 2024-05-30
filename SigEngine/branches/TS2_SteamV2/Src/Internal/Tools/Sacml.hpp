#ifndef __Sacml__
#define __Sacml__

namespace Sig { namespace Sacml
{
	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsSacmlFile( const tFilePathPtr& path );
	tools_export tFilePathPtr fSacmlPathToSacb( const tFilePathPtr& path );

	class tools_export tFile
	{
	public:
		b32					mDoFolder;
		b32					mRecurse;
		b32					mCompress;
		b32					mIgnoreDevFiles;
		tFilePathPtrList	mFilesToIgnore;
		tFilePathPtrList	mFilesToAdd;
		tFilePathPtrList	mFoldersToAdd;
		tFilePathPtrList	mFoldersToIgnore;

		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
		void fGeneratePackFile( const tFilePathPtr& myPath, u32 platforms, const std::string& outputFile = std::string( ) );
	};

}}

#endif//__Sacml__
