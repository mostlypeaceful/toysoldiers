#include "BasePch.hpp"
#include "tMotionMapFile.hpp"

namespace Sig
{

	define_lip_version( tMotionMapFile, 1, 1, 1 );

	const char*	tMotionMapFile::fGetFileExtension( )
	{
		return ".momapb";
	}

	tFilePathPtr tMotionMapFile::fPathToMomapb( const char* path )
	{
		return fPathToMomapb( tFilePathPtr( path ) );
	}

	tFilePathPtr tMotionMapFile::fPathToMomapb( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, fGetFileExtension( ) );
	}
}

