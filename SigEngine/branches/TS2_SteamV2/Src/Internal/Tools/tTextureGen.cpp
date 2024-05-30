#include "ToolsPch.hpp"
#include "tTextureGen.hpp"
#include "tTextureSysRam.hpp"
#include "FileSystem.hpp"
#include "Tatml.hpp"

namespace Sig
{
	tFilePathPtr tTextureGen::fCreateResourceNameFromInputPath( const tFilePathPtr& inputPath )
	{
		return tTextureSysRam::fCreateBinaryPath( inputPath );
	}


	void tTextureGen::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( tTextureSysRam::fRecognizedExtension( immediateInputFiles[ i ].fCStr( ) ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = fCreateResourceNameFromInputPath( immediateInputFiles[ i ] );

				const tFilePathPtr texturesIni = tTextureSysRam::fTexturesIniPath( immediateInputFiles[ i ] );
				if( FileSystem::fFileExists( texturesIni ) )
					inputOutputsOut.fBack( ).mInputs.fPushBack( texturesIni );
			}
			else if( Tatml::fIsTatmlFile( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = Tatml::fTatmlPathToTatb( ToolsPaths::fMakeResRelative( immediateInputFiles[ i ] ) );

				Tatml::tFile tatml;
				if( tatml.fLoadXml( immediateInputFiles[ i ] ) )
				{
					for( u32 j = 0; j < tatml.mTexturePaths.fCount( ); ++j )
						inputOutputsOut.fBack( ).mInputs.fPushBack( ToolsPaths::fMakeResAbsolute( tatml.mTexturePaths[ j ] ) );
				}
			}
		}
	}

	void tTextureGen::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		sigassert( inOut.mInputs.fCount( ) >= 1 );

		// store input file name in friendly variable
		const tFilePathPtr inputFileName = inOut.mInputs.fFront( );

		tTextureSysRam texDefault;
		tTextureSysRam texArray;
		b32 isAtlas = false;

		if( Tatml::fIsTatmlFile( inputFileName ) )
		{
			tTextureAtlasSysRam atlas;
			Tatml::tFile tatml;
			if( !tatml.fLoadXml( inputFileName ) )
			{
				log_warning( 0, "Invalid source Tatml file [" << inputFileName << "], conversion failed." );
				return;
			}

			isAtlas = true;

			// convert both to texture array and standard 2D atlas:

			tatml.fToTextureArray( texArray );

			tatml.fToTextureAtlas( atlas );
			atlas.fConvertToSysRamTexture( texDefault );
		}
		else
		{
			if( !texDefault.fLoad( inputFileName ) )
			{
				log_warning( 0, "Invalid source texture file [" << inputFileName << "], conversion failed." );
				return;
			}
		}

		tGrowableArray<tPlatformId> arrayTexturePlatforms;
		arrayTexturePlatforms.fPushBack( cPlatformXbox360 );

		// for each platform
		for( u32 i = 0; i < inOut.mPlatformsToConvert.fCount( ); ++i )
		{
			const tPlatformId pid = inOut.mPlatformsToConvert[ i ];
			const tFilePathPtr outputPath = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, inOut.mOutput );

			if( isAtlas && arrayTexturePlatforms.fFind( pid ) )
				texArray.fSaveGameBinary( outputPath, pid );
			else
				texDefault.fSaveGameBinary( outputPath, pid );
		}
	}

}

