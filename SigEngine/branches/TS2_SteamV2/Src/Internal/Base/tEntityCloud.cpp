//------------------------------------------------------------------------------
// \file tEntityCloud.cpp - 13 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tEntityCloud.hpp"
#include "tSceneGraph.hpp"

namespace Sig
{
	void tEntityCloud::fOnSpawn( )
	{
		sigassert( fSceneGraph( ) );
		fSceneGraph( )->fCloudInsert( *this );
		tEntity::fOnSpawn( );
	}

	void tEntityCloud::fOnDelete( )
	{
		fSceneGraph( )->fCloudRemove( *this );
		tEntity::fOnDelete( );
	}
}
