#include "ToolsPch.hpp"
#include "tSklmlAssetPlugin.hpp"
#include "tSklmlConverter.hpp"
#include "tFileWriter.hpp"

namespace Sig
{

	///
	/// \section tSklmlAssetGenPlugin
	///

	void tSklmlAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Sklml::fIsSklmlFile( immediateInputFiles[ i ] ) )
			{
				Sklml::tFile sklml;
				if( sklml.fLoadXml( immediateInputFiles[ i ] ) )
					sklml.fAddAssetGenInputOutput( inputOutputsOut, immediateInputFiles[ i ] );
			}
		}
	}

	void tSklmlAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		tSklmlConverter converter;

		// load sklml file
		if( !converter.fLoadSklmlFile( inOut.mInputs.fFront( ), inOut.mOutput ) )
			return;

		fAddDependencies( indirectGenFilesAddTo, converter.fSklmlFile( ) );

		// convert sklml => sklb for common platform
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
				log_warning( 0, "Couldn't open output Sklb file [" << outputPath << "] for writing." );
				continue;
			}

			// convert sklml => sklb for current platform
			converter.fConvertPlatformSpecific( pid );

			// output the sklb file
			converter.fOutput( ofile, pid );
		}
	}

	void tSklmlAssetGenPlugin::fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Sklml::tFile& sklmlFile )
	{
	}

}
