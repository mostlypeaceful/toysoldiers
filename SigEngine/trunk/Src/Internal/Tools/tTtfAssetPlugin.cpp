//------------------------------------------------------------------------------
// \file tTtfAssetPlugin.cpp - 22 Jan 2013
// \author jwittner
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tTtfAssetPlugin.hpp"
#include "FileSystem.hpp"
#include "tFileReader.hpp"
#include "tFileWriter.hpp"
#include "tLoadInPlaceSerializer.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tTtfFileConverter
	//------------------------------------------------------------------------------
	b32 tTtfFileConverter::fReadFile( const tFilePathPtr& inPath )
	{
		FileSystem::fReadFileToBuffer( mData, inPath );
		return mData.fCount( );
	}

	//------------------------------------------------------------------------------
	b32 tTtfFileConverter::fWriteFile( const tFilePathPtr& outPath, const tPlatformId& pid ) const
	{
		tFileWriter ofile( outPath );
		if( !ofile.fIsOpen( ) )
		{
			log_warning( "Failed to open file " << outPath );
			return false;
		}

		tLoadInPlaceSerializer ser;
		return ser.fSave( static_cast< const tTtfFile& >( *this ), ofile, pid );
	}

	//------------------------------------------------------------------------------
	// tTtfAssetPlugin
	//------------------------------------------------------------------------------
    tLoadInPlaceResourcePtr* tTtfAssetPlugin::fAddDependencyInternal( 
        tLoadInPlaceFileBase& outputFileObject, 
        const tFilePathPtr& dependencyPath )
    {
        tLoadInPlaceResourcePtr* o = 0;

        if( tTtfFile::fIsOperablePath( dependencyPath ) )
        {
            const tFilePathPtr filePath = ToolsPaths::fMakeResRelative
				( tTtfFile::fConvertToBinary( dependencyPath ) );
            o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake< tTtfFile >( filePath ) );
        }

        return o;
    }

	//------------------------------------------------------------------------------
	void tTtfAssetPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( tTtfFile::fIsOperablePath( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[ i ];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative
					( tTtfFile::fConvertToBinary( immediateInputFiles[ i ] ) );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tTtfAssetPlugin::fProcessInputOutputs(
			const tInputOutput& inOut,
			tFilePathPtrList& indirectGenFilesAddTo )
	{
		for( u32 i = 0; i < inOut.mInputs.fCount( ); ++i )
		{
			for( u32 j = 0; j < inOut.mPlatformsToConvert.fCount( ); ++j )
			{
				// Determine the output path by platform
				const tPlatformId pid = inOut.mPlatformsToConvert[ j ];
				const tFilePathPtr outputPath = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, inOut.mOutput );

				// Setup signature for output file
				tTtfFileConverter c;
				c.fSetSignature<tTtfFile>( pid );

				// Read in file
				if( !c.fReadFile( inOut.mInputs[ i ] ) )
				{
					log_warning( "Failed reading file " << inOut.mInputs[ i ] );
					continue;
				}

				// Write out file
				if( !c.fWriteFile( outputPath, pid ) )
				{
					log_warning( "Failed writing file " << outputPath );
					continue;
				}
			}
		}
	}

} // ::Sig