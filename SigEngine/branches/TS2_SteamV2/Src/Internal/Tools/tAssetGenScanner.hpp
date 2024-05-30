#ifndef __tAssetGenScanner__
#define __tAssetGenScanner__
#include "tHashTable.hpp"

namespace Sig
{
	class tAssetPluginDllDepot;

	struct tools_export tAssetGenOptions
	{
		u32 mOutputLevel;
		b32 mRecursive;
		b32 mForce;
		u32 mPlatforms;
		b32 mPauseOnExit;
		b32 mForcePauseOnExit;
		b32 mDisableDependencyFiles;
		b32 mGenPackFiles;
		b32 mGame2Xbox;
		b32 mMultiProcMaster;
		b32 mWorkerProc;
		b32 mSquirrelCheck;
		std::string mFile;
		std::string mExt;
		std::string mSacmlOutput;

		tAssetGenOptions( )
			: mOutputLevel( 2 ) // 0 = none, 1 = important only, 2 = default, 3 = verbose?
			, mRecursive( false )
			, mForce( false )
			, mPlatforms( ~0 ) // default to all platforms
			, mPauseOnExit( true )
			, mForcePauseOnExit( false )
			, mDisableDependencyFiles( false )
			, mGenPackFiles( false )
			, mGame2Xbox( false )
			, mMultiProcMaster( false )
			, mWorkerProc( false )
			, mSquirrelCheck( false )
		{
		}
	};

	struct	tAssetGenPluginInputOutput;
	class	iAssetGenPlugin;
	namespace Threads { class tProcess; }

	typedef tHashTable< tFilePathPtr, tFilePathPtr > tFilesProcessedBase;
	class tools_export tFilesProcessed : public tFilesProcessedBase
	{
	public:
		tFilesProcessed( ) : tFilesProcessedBase( 32 ) { }
	};

	class tools_export tAssetGenScanner : tUncopyable
	{
		tAssetGenOptions	mOpts;
		tFilePathPtrList	mInputsFromFolders;
		tFilesProcessed		mOutputFilesProcessed;
		u32					mDirtyPlatforms;

	public:
		tAssetGenScanner( const tAssetGenOptions& opts );
		~tAssetGenScanner( );
		void fHandleCurrentFolder( tAssetPluginDllDepot& pluginDepot );
		void fAcquireAllInputsFromFolderRecursive( );
		static void fCullByExtFilter( const tAssetGenOptions& opts, tFilePathPtrList& files );
		void fProcessFileList( tAssetPluginDllDepot& pluginDepot, const tFilePathPtrList& files );
		void fDelegateWork( const tFilePathPtrList& inputs, tFilePathPtrList& indirectGenFiles );
		Threads::tProcess* fSpawnWorkerProcess( const tFilePathPtr& inputFile, u32 ithProc );
		void fReadProcessDependencies( Threads::tProcess& proc, tFilePathPtrList& indirectGenFiles );
		static void fWriteProcessDependencies( const std::string& file, const tFilePathPtrList& indirectGenFiles, const tGrowableArray<std::string>& bufferedLogs );
		void fGeneratePackFiles( );
		void fMarkPlatformDirty( tPlatformId pid );
		b32  fIsPlatformDirty( tPlatformId pid ) const;
		static void fProcessSingleFile( const tFilePathPtr& absolutePath, b32 displayResults, u32 platformFlags = ~0 );
	};

}

#endif//__tAssetGenScanner__
