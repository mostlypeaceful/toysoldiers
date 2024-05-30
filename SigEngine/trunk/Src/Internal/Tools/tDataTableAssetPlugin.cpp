#include "ToolsPch.hpp"
#include "tDataTableAssetPlugin.hpp"
#include "tDataTableConverter.hpp"
#include "tFileWriter.hpp"

namespace Sig
{

	///
	/// \section tDataTableAssetGenPlugin
	///

    tLoadInPlaceResourcePtr* tDataTableAssetGenPlugin::fAddDependencyInternal( 
        tLoadInPlaceFileBase& outputFileObject, 
        const tFilePathPtr& dependencyPath 
        )
    {
        tLoadInPlaceResourcePtr* o = 0;

        if( StringUtil::fCheckExtension( dependencyPath.fCStr( ), ".tab" ) )
        {
            const tFilePathPtr filePath( std::string( dependencyPath.fCStr( ) ) + "b" );
            o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tDataTableFile>( filePath ) );
        }

        return o;
    }

	void tDataTableAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( tDataTableConverter::fIsCsvFile( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[i] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( tDataTableFile::fMakeBinaryPath( immediateInputFiles[i] ) );
			}
		}
	}

	void tDataTableAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		tDataTableConverter converter;

		// parse csv/txt file
		if( !converter.fConvertPlatformCommon( inOut.mInputs.fFront( ) ) )
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
				log_warning( "Couldn't open output Tabb file [" << outputPath << "] for writing." );
				continue;
			}

			// convert Locml => Locb for current platform
			converter.fConvertPlatformSpecific( pid );

			// output the Locb file
			converter.fOutput( ofile, pid );
		}
	}

}
