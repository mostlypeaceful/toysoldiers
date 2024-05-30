#include "MaxClassDesc.hpp"
#include <vector>
#include <algorithm>

namespace Sig { namespace MaxPlugin
{
	typedef std::vector<tBaseClassDesc*> tClassDescList;
	typedef tSingleton<tClassDescList> tGlobalClassDescList;

	int tBaseClassDesc::fNumberOfClasses( )
	{
		return ( int )tGlobalClassDescList::fGet( ).size( );
	}

	ClassDesc* tBaseClassDesc::fGetClassDesc( int i )
	{
		if( i < 0 || i >= tGlobalClassDescList::fGet( ).size( ) )
			return 0;
		return tGlobalClassDescList::fGet( )[i];
	}

	tBaseClassDesc::tBaseClassDesc( )
	{
		tGlobalClassDescList::fGet( ).push_back( this );
	}

	tBaseClassDesc::~tBaseClassDesc( )
	{
		tClassDescList::iterator i = std::find( tGlobalClassDescList::fGet( ).begin( ), tGlobalClassDescList::fGet( ).end( ), this );
		if( i != tGlobalClassDescList::fGet( ).end( ) )
			tGlobalClassDescList::fGet( ).erase( i );
	}

}}

