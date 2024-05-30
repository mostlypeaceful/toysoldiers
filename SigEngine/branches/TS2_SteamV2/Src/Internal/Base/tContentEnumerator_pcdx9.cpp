//------------------------------------------------------------------------------
// \file tContentEnumerator_pcdx9.cpp - 28 Mar 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tContentEnumerator.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tContentEnumerator
	//------------------------------------------------------------------------------
	b32 tContentEnumerator::fCreate( u32 userIndex, u32 deviceId, u32 contentType, b32 userSpecificOnly, u32 numItems )
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tContentEnumerator::fEnumerate( )
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tContentEnumerator::fAdvance( )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	void tContentEnumerator::fWait( )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	u32 tContentEnumerator::fResultCount( )
	{
		log_warning_unimplemented( 0 );
		return 0;
	}

	//------------------------------------------------------------------------------
	void tContentEnumerator::fResult( u32 idx, tContentData & data, XCONTENT_CROSS_TITLE_DATA& dataOut )
	{
		sigassert( idx < fResultCount( ) );
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	void tContentEnumerator::fDestroy( )
	{
		log_warning_unimplemented( 0 );
	}
}

#endif // defined( platform_pcdx9 ) || defined( platform_pcdx10 )