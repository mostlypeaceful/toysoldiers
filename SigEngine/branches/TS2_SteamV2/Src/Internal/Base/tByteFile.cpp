#include "BasePch.hpp"
#include "tByteFile.hpp"

namespace Sig
{
	tByteFile::tByteFile( )
	{
	}

	tByteFile::tByteFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mData( cNoOpTag )
	{
	}

}//Sig
