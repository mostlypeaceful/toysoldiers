#include "ToolsPch.hpp"
#include "tLocmlAssetPlugin.hpp"
#include "tLocmlConverter.hpp"
#include "tFileWriter.hpp"

namespace Sig
{

	///
	/// \section tLocmlAssetGenPlugin
	///

    tLoadInPlaceResourcePtr* tLocmlAssetGenPlugin::fAddDependencyInternal( 
        tLoadInPlaceFileBase& outputFileObject, 
        const tFilePathPtr& dependencyPath )
    {
        tLoadInPlaceResourcePtr* o = 0;

        return o;
    }

	void tLocmlAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Locml::fIsLocmlFile( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[i] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( Locml::fLocmlPathToLocb( immediateInputFiles[i] ) );
			}
		}
	}

	void tLocmlAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		tLocmlConverter converter;

		// load Locml file
		if( !converter.fLoadLocmlFile( inOut.mInputs.fFront( ) ) )
			return;

		fAddDependencies( indirectGenFilesAddTo, converter.fLocmlFile( ) );

		// convert Locml => Locb for common platform
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
				log_warning( "Couldn't open output Locb file [" << outputPath << "] for writing." );
				continue;
			}

			// convert Locml => Locb for current platform
			converter.fConvertPlatformSpecific( pid );

			// output the Locb file
			converter.fOutput( ofile, pid );
		}
	}

	void tLocmlAssetGenPlugin::fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Locml::tFile& locmlFile )
	{
	}

}
