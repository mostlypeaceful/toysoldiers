#include "ToolsPch.hpp"
#include "tTiemlAssetPlugin.hpp"
#include "tTiemlConverter.hpp"

namespace Sig
{

	///
	/// \section tTiemlAssetGenPlugin
	///

	void tTiemlAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Tieml::fIsTiemlFile( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( Tieml::fTiemlPathToTieb( immediateInputFiles[ i ] ) );
			}
		}
	}

	void tTiemlAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		tTiemlConverter converter;

		// load Tieml file
		if( !converter.fLoadTiemlFile( inOut.mInputs.fFront( ), inOut.mOutput ) )
			return;

		// convert Tieml => tieb for common platform
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
				log_warning( 0, "Couldn't open output Tieb file [" << outputPath << "] for writing." );
				continue;
			}

			// convert Tieml => tieb for current platform
			converter.fConvertPlatformSpecific( pid );

			// output the tieb file
			converter.fOutput( ofile, pid );
		}

		fAddDependencies( indirectGenFilesAddTo, converter.fTiemlFile( ) );
	}

	void tTiemlAssetGenPlugin::fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Tieml::tFile& TiemlFile )
	{
	}

}
