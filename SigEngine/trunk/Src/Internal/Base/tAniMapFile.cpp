#include "BasePch.hpp"
#include "tAniMapFile.hpp"

namespace Sig
{
	define_lip_version( tAniMapFile, 1, 1, 1 );

	const char*	tAniMapFile::fGetFileExtension( )
	{
		return ".animab";
	}

	tFilePathPtr tAniMapFile::fPathToAnimab( const char* path )
	{
		return fPathToAnimab( tFilePathPtr( path ) );
	}

	tFilePathPtr tAniMapFile::fPathToAnimab( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, fGetFileExtension( ) );
	}



}

