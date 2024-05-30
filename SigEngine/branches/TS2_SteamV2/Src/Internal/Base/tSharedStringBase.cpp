#include "BasePch.hpp"
#include "tSharedStringBase.hpp"
#include "tSharedStringTable.hpp"

namespace Sig
{
	tSharedStringBase::tSharedStringBase( const char* str )
	{
		mCharBuffer.fCreateNullTerminated( str );
	}
	
	void tSharedStringBase::fOnDestroy( tSharedStringTable& stringTable )
	{
		stringTable.fRemoveString( mCharBuffer.fBegin( ) );
	}

}
