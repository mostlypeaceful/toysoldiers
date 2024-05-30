//------------------------------------------------------------------------------
// \file tContent.cpp - 24 Mar 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tContent.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tContent::tContent( )
		: mState( cStateNull )
	{
	}

	tContent::tContent( const tStringPtr & root )
		: mState( cStateCreated )
		, mRootName( root )
	{
		sigassert( !root.fNull( ) && "Content roots cannot be null" );
	}

	tContent::~tContent( )
	{
		fClose( );
	}
}
