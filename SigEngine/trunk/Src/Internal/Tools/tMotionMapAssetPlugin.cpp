#include "ToolsPch.hpp"
#include "Momap.hpp"
#include "tMotionMapAssetPlugin.hpp"
#include "tMotionMapFile.hpp"
#include "tFileWriter.hpp"
#include "tLoadInPlaceSerializer.hpp"

namespace Sig
{

	///
	/// \section tMotionMapAssetGenPlugin
	///
    
    tLoadInPlaceResourcePtr* tMotionMapAssetGenPlugin::fAddDependencyInternal( 
        tLoadInPlaceFileBase& outputFileObject, 
        const tFilePathPtr& dependencyPath 
    )
    {
        tLoadInPlaceResourcePtr* o = 0;

        if( Momap::fIsMomapFile( dependencyPath ) )
        {
            const tFilePathPtr filePath = tMotionMapFile::fConvertToBinary( dependencyPath );
            o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tMotionMapFile>( filePath ) );
        }

        return o;
    }

	void tMotionMapAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Momap::fIsMomapFile( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( tMotionMapFile::fConvertToBinary( immediateInputFiles[ i ] ) );
			}
		}
	}

	void tMotionMapAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		Momap::tFile file;
		const tFilePathPtr& inputFileName = inOut.mInputs.fFront( );
		if( !file.fLoadXml( inputFileName ) )
			return;

		tMotionMapFile* output = file.fBuildMoMapFile( true );

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
			output->fSetSignature<tMotionMapFile>( pid );

			// output
			tLoadInPlaceSerializer ser;
			ser.fSave( static_cast<tMotionMapFile&>( *output ), ofile, pid );
		}

		delete output;
	}

}
