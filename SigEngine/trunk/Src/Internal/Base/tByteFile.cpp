#include "BasePch.hpp"
#include "tByteFile.hpp"

namespace Sig
{
	define_lip_version( tByteFile, 0, 0, 0 );

	tByteFile::tByteFile( )
	{
	}

	tByteFile::tByteFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mData( cNoOpTag )
	{
	}

}//Sig
