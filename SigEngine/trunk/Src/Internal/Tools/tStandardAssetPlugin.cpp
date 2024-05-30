#include "ToolsPch.hpp"
#include "tStandardAssetPlugin.hpp"
#include "FileSystem.hpp"

namespace Sig
{
    tLoadInPlaceResourcePtr* tStandardAssetPlugin::fAddDependencyInternal( 
        tLoadInPlaceFileBase& outputFileObject, 
        const tFilePathPtr& dependencyPath )
    {
        tLoadInPlaceResourcePtr* o = 0;

        return o;
    }

	void tStandardAssetPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( StringUtil::fCheckExtension( immediateInputFiles[ i ].fCStr( ), "sigcmd" ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[i] );
				inputOutputsOut.fBack( ).mOutput = immediateInputFiles[i];
			}
		}
	}

	void tStandardAssetPlugin::fProcessInputOutputs(
		const tInputOutput& inputOutput,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		sigassert( inputOutput.mInputs.fCount( ) == 1 );

		tDynamicBuffer text;
		if( FileSystem::fReadFileToBuffer( text, inputOutput.mInputs.fFront( ), "" ) )
		{
			Win32Util::tTemporaryCurrentDirectoryChange changeCurDirToSubFolder( StringUtil::fDirectoryFromPath( inputOutput.mInputs.fFront( ).fCStr( ) ).c_str( ) );

			tGrowableArray<std::string> lines;
			StringUtil::fSplit( lines, ( const char* )text.fBegin( ), "\n" );
			for( u32 i = 0; i < lines.fCount( ); ++i )
				system( lines[ i ].c_str( ) );
		}
	}
}

