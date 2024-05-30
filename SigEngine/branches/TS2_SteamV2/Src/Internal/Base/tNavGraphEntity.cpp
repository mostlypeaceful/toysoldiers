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

	void tNavGraphEntityDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		tNavGraphEntity* entity = NEW tNavGraphEntity( mNavGraph );

		fApplyProperties( *entity, creationFlags );
		fEntityOnCreate( *entity );
		entity->fSpawn( parent );
		fEntityOnChildrenCreate( *entity );
	}

	//------------------------------------------------------------------------------
	// tNavGraphEntity
	//------------------------------------------------------------------------------
	tNavGraphEntity::tNavGraphEntity( AI::tBuiltNavGraph* graph )
		: mGraphWrapper( NEW AI::tTempNavGraphWrapper( graph ) )
	{ }
}
