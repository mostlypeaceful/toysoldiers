#include "ToolsPch.hpp"
#include "Animap.hpp"
#include "tAnimapAssetPlugin.hpp"
#include "tAniMapFile.hpp"
#include "tFileWriter.hpp"
#include "tLoadInPlaceSerializer.hpp"

namespace Sig
{

	///
	/// \section tAnimapAssetGenPlugin
	///

    tLoadInPlaceResourcePtr* tAnimapAssetGenPlugin::fAddDependencyInternal( 
        tLoadInPlaceFileBase& outputFileObject, 
        const tFilePathPtr& dependencyPath 
    )
    {
        tLoadInPlaceResourcePtr* o = 0;

        if( Animap::fIsAnimapFile( dependencyPath ) )
        {
            const tFilePathPtr filePath = tAniMapFile::fConvertToBinary( dependencyPath );
            o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tAniMapFile>( filePath ) );
        }

        return o;
    }

	void tAnimapAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Animap::fIsAnimapFile( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( tAniMapFile::fConvertToBinary( immediateInputFiles[ i ] ) );
			}
		}
	}

	void tAnimapAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		Animap::tFile file;
		const tFilePathPtr& inputFileName = inOut.mInputs.fFront( );
		if( !file.fLoadXml( inputFileName ) )
			return;

		tAniMapFile* output = file.fMakeAnimapFile( );

		// for each platform
		for( u32 i = 0; i < inOut.mPlatformsToConvert.fCount( ); ++i )
		{
			const tPlatformId pid = inOut.mPlatformsToConvert[ i ];
			const tFilePathPtr ofileName = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, inOut.mOutput );

			// open the output file; if we can't open it, then no need to convert the platform-specific stuff
			tFilePathPtr outputPath = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, inOut.mOutput );
			tFileWriter ofile( outputPath );
			if( !ofile.fIsOpen( ) )
			{
				log_warning( "Couldn't open output file [" << outputPath << "] for writing." );
				continue;
			}

			// set the binary file signature
			output->fSetSignature<tAniMapFile>( pid );

			// output
			tLoadInPlaceSerializer ser;
			ser.fSave( static_cast<tAniMapFile&>( *output ), ofile, pid );
		}

		delete output;

		file.fGatherAnimPacks( indirectGenFilesAddTo );
	}

}
