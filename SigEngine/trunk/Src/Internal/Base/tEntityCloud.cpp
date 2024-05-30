//------------------------------------------------------------------------------
// \file tEntityCloud.cpp - 13 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tEntityCloud.hpp"
#include "tSceneGraph.hpp"
#include "tGameAppBase.hpp"

namespace Sig
{
#ifdef platform_xbox360
	devvar( bool, GroundCover_Instanced, true );
#else
	devvar( bool, GroundCover_Instanced, false );
#endif//platform_xbox360

#ifdef sig_devmenu
	void fInitInstanceData( tDevCallback::tArgs& args )
	{
		tGameAppBase::fInstance( ).fSceneGraph( )->fInitGCInstancing( );
	}
	devcb( GroundCover_InstancedInit, "Init", make_delegate_cfn( tDevCallback::tFunction, fInitInstanceData ) );
#endif//sig_devmenu


	b32 tEntityCloud::fInstanced( )
	{
		return GroundCover_Instanced;
	}

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
