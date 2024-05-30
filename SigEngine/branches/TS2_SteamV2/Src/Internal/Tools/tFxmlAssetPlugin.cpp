#include "ToolsPch.hpp"
#include "tFxmlAssetPlugin.hpp"
#include "tFxmlConverter.hpp"

namespace Sig
{

	///
	/// \section tFxmlAssetGenPlugin
	///

	void tFxmlAssetGenPlugin::fDetermineInputOutputs( tInputOutputList& inputOutputsOut, const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Fxml::fIsFxmlFile( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[ i ];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( Fxml::fFxmlPathToFxb( immediateInputFiles[ i ] ) );
			}
		}
	}

	void tFxmlAssetGenPlugin::fProcessInputOutputs( const tInputOutput& inOut, tFilePathPtrList& indirectGenFilesAddTo )
	{
		tFxmlConverter converter;

		// load fxml file
		if( !converter.fLoadFxmlFile( inOut.mInputs.fFront( ), inOut.mOutput ) )
			return;


		// convert fxml => fxb for common platform
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
				log_warning( 0, "Couldn't open output Fxb file [" << outputPath << "] for writing." );
				continue;
			}

			// convert fxml => sigb for current platform
			converter.fConvertPlatformSpecific( pid );

			// output the fxb file
			converter.fOutput( ofile, pid );
		}

		fAddDependencies( indirectGenFilesAddTo, converter.fFxmlFile( ) );
	}

	void tFxmlAssetGenPlugin::fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Fxml::tFile& fxmlFile )
	{
		for( u32 i = 0; i < fxmlFile.mObjects.fCount( ); ++i )
		{
			Fxml::tFxParticleSystemObject* fxParticleSystem = dynamic_cast< Fxml::tFxParticleSystemObject* >( fxmlFile.mObjects[ i ].fGetRawPtr( ) );
			Fxml::tFxAttractorObject* fxAttractor = dynamic_cast< Fxml::tFxAttractorObject* >( fxmlFile.mObjects[ i ].fGetRawPtr( ) );
			Fxml::tFxMeshSystemObject* fxMeshSystem = dynamic_cast< Fxml::tFxMeshSystemObject* >( fxmlFile.mObjects[ i ].fGetRawPtr( ) );

			if( !fxParticleSystem && !fxAttractor && !fxMeshSystem )
				continue;

			fxmlFile.mObjects[ i ]->fGetDependencyFiles( indirectGenFilesAddTo );
		}
	}

}
