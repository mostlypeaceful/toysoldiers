#include "ToolsPch.hpp"
#include "Goaml.hpp"
#include "tGoamlAssetPlugin.hpp"
#include "tScriptFileConverter.hpp"
#include "tFileWriter.hpp"

namespace Sig
{

	///
	/// \section tGoamlAssetGenPlugin
	///

	void tGoamlAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Goaml::fIsGoamlFile( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( Goaml::fGoamlPathToNutb( immediateInputFiles[ i ] ) );
			}
		}
	}

	void tGoamlAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		// read goaml
		Goaml::tFile goamlFile;
		const tFilePathPtr& inputFileName = inOut.mInputs.fFront( );
		if( !goamlFile.fLoadXml( inputFileName ) )
			return;

		// generate script
		const u32 fileBasedUniqueId = tScriptFileConverter::fGenerateUniqueFileBasedId( inputFileName );
		std::string script = goamlFile.fBuildScript( tScriptFileConverter::fGenerateUniqueFileBasedTag( fileBasedUniqueId ) );

		tDynamicBuffer scriptString;
		scriptString.fInsert( 0, reinterpret_cast< const unsigned char* >( script.c_str( ) ), script.length( ) );
		scriptString.fPushBack( 0 );

		// for each platform
		for( u32 i = 0; i < inOut.mPlatformsToConvert.fCount( ); ++i )
		{
			// build script
			const tPlatformId pid = inOut.mPlatformsToConvert[ i ];
			const tFilePathPtr ofileName = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, inOut.mOutput );
			tScriptFileConverter::fConvertScript( scriptString, inputFileName, ofileName, pid, indirectGenFilesAddTo, goamlFile.mExplicitDependencies );
		}

		const b32 dumpNut = false;
		if( dumpNut )
		{
			std::string file( inOut.mOutput.fCStr( ) );
			//hack the 'b' off
			const tFilePathPtr ofileName = ToolsPaths::fMakeResAbsolute( tFilePathPtr( file.substr( 0, file.length( ) - 1 ) ) );
			tFileWriter ofile;
			ofile.fOpen( ofileName );
			ofile( script.c_str( ), script.length( ) );
		}

		fAddDependencies( indirectGenFilesAddTo, goamlFile );
	}

	void tGoamlAssetGenPlugin::fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Goaml::tFile& goamlFile )
	{
		for( u32 i = 0; i < goamlFile.mExplicitDependencies.fCount( ); ++i )
			if( goamlFile.mExplicitDependencies[ i ].fLength( ) > 0 )
				indirectGenFilesAddTo.fFindOrAdd( goamlFile.mExplicitDependencies[ i ] );
	}

}
