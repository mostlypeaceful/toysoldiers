#include "ToolsPch.hpp"
#include "tTiledbAssetPlugin.hpp"
#include "Tiledb.hpp"
#include "FileSystem.hpp"
#include "tTiledbConverter.hpp"
#include "tFileWriter.hpp"

namespace Sig
{
	///
	/// \section tTiledbAssetGenPlugin
	///

    tLoadInPlaceResourcePtr* tTiledmlAssetGenPlugin::fAddDependencyInternal( 
        tLoadInPlaceFileBase& outputFileObject, 
        const tFilePathPtr& dependencyPath 
        )
    {
        tLoadInPlaceResourcePtr* o = 0;

        if( Tiledml::fIsTiledmlFile( dependencyPath ) )
        {
            const tFilePathPtr filePath = tTilePackage::fConvertToBinary( dependencyPath );
            o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tTilePackage>( filePath ) );
        }

        return o;
    }

	void tTiledmlAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Tiledml::fIsTiledmlFile( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[ i ];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( Tiledml::fTiledmlPathToTiledb( immediateInputFiles[ i ] ) );
			}
		}
	}

	void tTiledmlAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		tTiledmlConverter converter;

		// Load tiledml file
		if( !converter.fLoadTiledbFile( inOut.mInputs.fFront( ) ) )
			return;

		// Convert all the stuff from the tiledml file to a tiledb.
		if( !converter.fConvertPlatformCommon() )
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
				log_warning( "Couldn't open output Tiledb file [" << outputPath << "] for writing." );
				continue;
			}

			// convert tiledml => tiledb for current platform
			converter.fConvertPlatformSpecific( pid );

			// output the tiledb file
			converter.fOutput( ofile, pid );
		}

		fAddDependencies( indirectGenFilesAddTo, converter.fTiledbFile() );
	}

	void tTiledmlAssetGenPlugin::fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Tiledml::tFile& tiledbFile )
	{
	}

}
