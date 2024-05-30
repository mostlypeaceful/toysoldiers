#include "ToolsPch.hpp"
#include "tSwfAssetPlugin.hpp"
#include "FileSystem.hpp"
#include "tFileReader.hpp"
#include "tFileWriter.hpp"
#include "tLoadInPlaceSerializer.hpp"

namespace Sig
{
	b32 tSwfFileConverter::fReadFile( const tFilePathPtr& inPath )
	{
		FileSystem::fReadFileToBuffer( mData, inPath );
		return mData.fCount( );
	}

	b32 tSwfFileConverter::fWriteFile( const tFilePathPtr& outPath, const tPlatformId& pid ) const
	{
		tFileWriter ofile( outPath );
		if( !ofile.fIsOpen( ) )
		{
			log_warning( "Failed to open file " << outPath );
			return 0;
		}
		tLoadInPlaceSerializer ser;
		return ser.fSave( static_cast< const tSwfFile& >( *this ), ofile, pid );
	}

    tLoadInPlaceResourcePtr* tSwfAssetPlugin::fAddDependencyInternal( 
        tLoadInPlaceFileBase& outputFileObject, 
        const tFilePathPtr& dependencyPath )
    {
        tLoadInPlaceResourcePtr* o = 0;

        if( StringUtil::fCheckExtension( dependencyPath.fCStr( ), ".swf" ) )
        {
            const tFilePathPtr filePath = ToolsPaths::fMakeResRelative( tSwfFile::fConvertToBinary( dependencyPath ) );
            o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake< tSwfFile >( filePath ) );
        }

        return o;
    }

	void tSwfAssetPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( StringUtil::fCheckExtension( immediateInputFiles[ i ].fCStr( ), ".swf" ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( tSwfFile::fConvertToBinary( immediateInputFiles[ i ] ) );
			}
		}
	}

	void tSwfAssetPlugin::fProcessInputOutputs(
			const tInputOutput& inOut,
			tFilePathPtrList& indirectGenFilesAddTo )
	{
		//for each input
		for( u32 i = 0; i < inOut.mInputs.fCount( ); ++i )
		{
			// for each platform (PC, Xbox, etc.)
			for( u32 j = 0; j < inOut.mPlatformsToConvert.fCount( ); ++j )
			{
				//determine the output path by platform
				const tPlatformId pid = inOut.mPlatformsToConvert[ j ];
				const tFilePathPtr outputPath = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, inOut.mOutput );

				//setup signature for output file
				tSwfFileConverter c;
				c.fSetSignature<tSwfFile>( pid );

				//read in file
				if( !c.fReadFile( inOut.mInputs[ i ] ) )
				{
					log_warning( "Failed reading file " << inOut.mInputs[ i ] );
					continue;
				}

				//write out file
				if( !c.fWriteFile( outputPath, pid ) )
				{
					log_warning( "Failed writing file " << outputPath );
					continue;
				}
			}
		}
	}

}//sig