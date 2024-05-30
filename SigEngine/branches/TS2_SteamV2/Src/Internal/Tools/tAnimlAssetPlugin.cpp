#include "ToolsPch.hpp"
#include "tAnimlAssetPlugin.hpp"
#include "tAnimlConverter.hpp"
#include "tFileWriter.hpp"

namespace Sig
{

	///
	/// \section tAnimlAssetGenPlugin
	///

	void tAnimlAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Anipk::fIsAnipkFile( immediateInputFiles[ i ] ) )
			{
				Anipk::tFile anipk;
				if( anipk.fLoadXml( immediateInputFiles[ i ] ) )
				{
					anipk.fAddAssetGenInputOutput( inputOutputsOut, immediateInputFiles[ i ] );
				}
			}
		}
	}

	void tAnimlAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		tAnimlConverter converter;

		// load animl file
		if( !converter.fLoadAnimlFiles( inOut.mInputs, inOut.mOutput ) )
			return;

		// convert animl => anib for common platform
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
				log_warning( 0, "Couldn't open output Anib file [" << outputPath << "] for writing." );
				continue;
			}

			// convert animl => anib for current platform
			converter.fConvertPlatformSpecific( pid );

			// output the anib file
			converter.fOutput( ofile, pid );
		}
	}

}
