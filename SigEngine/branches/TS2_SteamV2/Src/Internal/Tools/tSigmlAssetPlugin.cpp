#include "ToolsPch.hpp"
#include "tSigmlAssetPlugin.hpp"
#include "tSigmlConverter.hpp"
#include "FileSystem.hpp"
#include "Tieml.hpp"

namespace Sig
{
	namespace
	{
		static void fSetupSigmlInputOutputs( iAssetGenPlugin::tInputOutputList& inputOutputsOut, const tFilePathPtr& sigmlPath )
		{
			inputOutputsOut.fGrowCount( 1 );
			inputOutputsOut.fBack( ).mOriginalInput = sigmlPath;
			inputOutputsOut.fBack( ).mInputs.fPushBack( sigmlPath );
			inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( Sigml::fSigmlPathToSigb( sigmlPath ) );

			if( StringUtil::fCheckExtension( sigmlPath.fCStr( ), ".mshml" ) )
			{
				const tFilePathPtr skinPath = tFilePathPtr::fSwapExtension( sigmlPath, ".skin" );
				const tFilePathPtr curDir = tFilePathPtr( StringUtil::fDirectoryFromPath( sigmlPath.fCStr( ) ) );

				tFilePathPtrList immediateInputFiles;
				FileSystem::fGetFileNamesInFolder( immediateInputFiles, curDir, true );

				for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
				{
					if( immediateInputFiles[i] == skinPath )
					{
						inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[i] );
						break;
					}
				}
			}
		}
	}

	///
	/// \section tSigmlAssetGenPlugin
	///

	void tSigmlAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Sigml::fIsSigmlFile( immediateInputFiles[ i ] ) && !Tieml::fIsTiemlFile( immediateInputFiles[ i ] ) )
			{
				fSetupSigmlInputOutputs( inputOutputsOut, immediateInputFiles[ i ] );
			}
		}
	}

	void tSigmlAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		tSigmlConverter converter;

		sigassert( inOut.mInputs.fCount( ) >= 1 );
		const tFilePathPtr sigmlPath = inOut.mInputs.fFront( );
		const tFilePathPtr skinPath = tFilePathPtr::fSwapExtension( sigmlPath, ".skin" );
		const b32 applySkin = inOut.mInputs.fFind( skinPath ) != 0; // input processing determined there was a corresponding skin file

		// load sigml file
		if( !converter.fLoadSigmlFile( sigmlPath, applySkin ? skinPath : tFilePathPtr( ), inOut.mOutput ) )
			return;

		// convert sigml => sigb for common platform
		if( !converter.fConvertPlatformCommon( ) )
			return;

		// for each platform
		for( u32 i = 0; i < inOut.mPlatformsToConvert.fCount( ); ++i )
		{
			const tPlatformId pid = inOut.mPlatformsToConvert[ i ];

			// open the output file; if we can't open it, then no need to convert the platform-specific stuff
			tFilePathPtr outputPath = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, inOut.mOutput );
			tFileWriter ofile( outputPath );
			if( !ofile.fIsOpen( ) )
			{
				log_warning( 0, "Couldn't open output Sigb file [" << outputPath << "] for writing." );
				continue;
			}

			// convert sigml => sigb for current platform
			converter.fConvertPlatformSpecific( pid );

			// output the sigb file
			converter.fOutput( ofile, pid );
		}

		fAddDependencies( indirectGenFilesAddTo, converter.fSigmlFile( ) );
	}

	void tSigmlAssetGenPlugin::fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Sigml::tFile& sigmlFile )
	{
		tFilePathPtr globalScript; std::string dummy;
		sigmlFile.fGetScriptSource( globalScript, dummy );
		if( globalScript.fLength( ) > 0 )
			indirectGenFilesAddTo.fFindOrAdd( globalScript );
		tFilePathPtr globalSkeleton = sigmlFile.fGetSkeletonSourcePath( );
		if( globalSkeleton.fLength( ) > 0 )
			indirectGenFilesAddTo.fFindOrAdd( globalSkeleton );

		if( sigmlFile.mDiffuseMapAtlas.fLength( ) > 0 )
			indirectGenFilesAddTo.fFindOrAdd( sigmlFile.mDiffuseMapAtlas );
		if( sigmlFile.mNormalMapAtlas.fLength( ) > 0 )
			indirectGenFilesAddTo.fFindOrAdd( sigmlFile.mNormalMapAtlas );

		for( u32 i = 0; i < sigmlFile.mObjects.fCount( ); ++i )
			sigmlFile.mObjects[ i ]->fGetDependencyFiles( indirectGenFilesAddTo );
		for( u32 i = 0; i < sigmlFile.mMaterials.fCount( ); ++i )
			sigmlFile.mMaterials[ i ]->fGetTextureResourcePaths( indirectGenFilesAddTo );

		for( u32 i = 0; i < sigmlFile.mExplicitDependencies.fCount( ); ++i )
			if( sigmlFile.mExplicitDependencies[ i ].fLength( ) > 0 )
				indirectGenFilesAddTo.fFindOrAdd( sigmlFile.mExplicitDependencies[ i ] );
	}

}
