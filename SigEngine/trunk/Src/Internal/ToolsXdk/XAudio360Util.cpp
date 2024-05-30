#include "ToolsXdkPch.hpp"
#include "tXbDm.hpp"
#include "XAudio360Util.hpp"
#include "FileSystem.hpp"
#include "tFileWriter.hpp"
#include "Threads/tProcess.hpp"

namespace Sig { namespace XAudio360Util
{
	namespace
	{
		static const tFilePathPtr cScratchFilePath = ToolsPaths::fCreateTempEngineFilePath( ".wav", tFilePathPtr("xaudio"), "scratch" );
	}

	b32 fWaveFileToXma( tDynamicBuffer& obuffer, const tFilePathPtr& wavePath, s32 quality )
	{
//xma2encode file [/TargetFile xmafile][/PCMOutputFile pcmfile]
//  [/DecodeToPCM pcmfile][/BlockSize][/Quality value]
//  [/FilterHighFrequencies][/LoopWholeFile][/UseLoopPoints]
//  [/Speaker pos][/MeasureTime][/Verbose][/Help]

		{tFileWriter ensureTempFileIsThere( cScratchFilePath );}


		tXbDm& xbdm = tXbDm::fInstance( );
		const tFilePathPtr& xdkBinPath = xbdm.fXdkBinPath( );
		const tFilePathPtr xmaEncoderPath = tFilePathPtr::fConstructPath( xdkBinPath, tFilePathPtr( "xma2encode.exe" ) );

		std::stringstream cmdLineSs;
		cmdLineSs << wavePath.fCStr( ) << " /TargetFile " << cScratchFilePath.fCStr( );
		if( quality >= 0 )
			cmdLineSs << " /Quality " << quality;

		const std::string cmdLine = cmdLineSs.str( );

		if( !Threads::tProcess::fSpawnAndForget( xmaEncoderPath.fCStr( ), cmdLine.c_str( ), 0, true ) )
			return false;

		return FileSystem::fReadFileToBuffer( obuffer, cScratchFilePath );
	}

}}


