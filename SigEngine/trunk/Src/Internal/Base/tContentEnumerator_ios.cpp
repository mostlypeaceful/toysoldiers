//------------------------------------------------------------------------------
// \file tContentEnumerator_ios.cpp - 28 Mar 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_ios )
#include "tContentEnumerator.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tContentEnumerator
	//------------------------------------------------------------------------------
	b32 tContentEnumerator::fCreate( u32 userIndex, u32 deviceId, u32 contentType, b32 userSpecificOnly, u32 numItems )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tContentEnumerator::fEnumerate( )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tContentEnumerator::fAdvance( )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	void tContentEnumerator::fWait( )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	u32 tContentEnumerator::fResultCount( )
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	void tContentEnumerator::fResult( u32 idx, tContentData & data )
	{
		sigassert( idx < fResultCount( ) );
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tContentEnumerator::fDestroy( )
	{
		log_warning_unimplemented( );
	}
}


#endif // defined( platform_ios )
