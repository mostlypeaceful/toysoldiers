#include "BasePch.hpp"
#include "tSharedStringTable.hpp"

namespace Sig
{
	tSharedStringTable::tSharedStringTable( )
		: tSharedStringTableBase( 32 )
	{
	}

	tSharedStringTable::~tSharedStringTable( )
	{
	}

	tSharedStringBase* tSharedStringTable::fFindString( tNewSharedStringInstance newSharedString, const char* str )
	{
		tSharedStringBase** find = fFind( str );
		if( !find )
		{
			tSharedStringBase* ss = newSharedString( str ); // i.e., NEW tSharedString( str );
			find = fInsert( ss->fCStr( ), ss );
			sigassert( fFind( ss->fCStr( ) ) );
		}
		sigassert( find );
		return *find;
	}

	void tSharedStringTable::fRemoveString( const char* str )
	{
		tSharedStringBase** find = fFind( str );
		sigassert( find );
		fRemove( find );
	}
}
