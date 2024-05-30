#include "ToolsPch.hpp"
#include "tFontGen.hpp"
#include "tFntmlConverter.hpp"
#include "Gui/tFont.hpp"

namespace Sig
{

    tLoadInPlaceResourcePtr* tFontGen::fAddDependencyInternal( 
        tLoadInPlaceFileBase& outputFileObject, 
        const tFilePathPtr& dependencyPath 
    )
    {
        tLoadInPlaceResourcePtr* o = 0;

        if( StringUtil::fCheckExtension( dependencyPath.fCStr( ), ".fntml" ) )
        {
            const tFilePathPtr filePath = tFilePathPtr::fSwapExtension( dependencyPath, ".fntb" );
            o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gui::tFont>( filePath ) );
        }

        return o;
    }

	void tFontGen::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( StringUtil::fCheckExtension( immediateInputFiles[ i ].fCStr( ), Fntml::fGetFileExtension( ) ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = iAssetGenPlugin::fCreateRelativeOutputPath( immediateInputFiles[ i ], Gui::tFont::fGetFileExtension( ) );
			}
		}
	}

	void tFontGen::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		sigassert( inOut.mInputs.fCount( ) == 1 );

		// store input file name in friendly variable
		const tFilePathPtr inputFileName = inOut.mInputs.fFront( );

		tFntmlConverter converter;

		if( !converter.fLoad( inputFileName ) )
		{
			log_warning( "Invalid source font file [" << inputFileName << "], conversion failed." );
			return;
		}

		// for each platform
		for( u32 i = 0; i < inOut.mPlatformsToConvert.fCount( ); ++i )
		{
			const tPlatformId pid = inOut.mPlatformsToConvert[ i ];
			const tFilePathPtr outputPath = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, inOut.mOutput );

			converter.fSaveGameBinary( outputPath, pid );
		}
	}

}

