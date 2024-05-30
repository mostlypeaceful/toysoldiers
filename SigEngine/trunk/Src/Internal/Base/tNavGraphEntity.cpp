//------------------------------------------------------------------------------
// \file tNavGraphEntity.cpp - 13 Dec 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tNavGraphEntity.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tNavGraphEntityDef
	//------------------------------------------------------------------------------
	tNavGraphEntityDef::tNavGraphEntityDef( )
		: mNavGraph( NULL )
	{
	}

	tNavGraphEntityDef::tNavGraphEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
	{
	}

	tNavGraphEntityDef::~tNavGraphEntityDef( )
	{
		delete mNavGraph;
	}

	void tNavGraphEntityDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		tNavGraphEntity* entity = NEW tNavGraphEntity( mNavGraph );

		fApplyProperties( *entity, params.mCreationFlags );
		fEntityOnCreate( *entity );
		entity->fSpawn( params.mParent );
		fEntityOnChildrenCreate( *entity );
	}

	//------------------------------------------------------------------------------
	// tNavGraphEntity
	//------------------------------------------------------------------------------
	tNavGraphEntity::tNavGraphEntity( AI::tBuiltNavGraph* graph )
		: mGraphWrapper( NEW AI::tTempNavGraphWrapper( graph ) )
	{ }
}
