#include "GameAppPch.hpp"
#include "tDecalManager.hpp"
#include "tGameApp.hpp"

namespace Sig
{
	Gfx::tWorldSpaceDecalPtr tDecalManager::fPlace( const tResourcePtr& colorMap, const Math::tObbf& projectionBox )
	{
		// TODO performance heuristics, i.e., check how many decals we have, maybe see if there's already decals near/intersecting
		// the projection box, etc, and don't spawn if so

		mDecals.fPushBack( Gfx::tWorldSpaceDecalPtr( NEW Gfx::tWorldSpaceDecal( colorMap, projectionBox, *tGameApp::fInstance( ).fScreen( )->fGetDevice( ), GameFlags::cFLAG_GROUND ) ) );

		mDecals.fBack( )->fSpawn( tGameApp::fInstance( ).fSceneGraph( )->fRootEntity( ) );

		return mDecals.fBack( );
	}

	void tDecalManager::fUpdate( f32 dt )
	{
		profile_pix("tDecalMan*::fUpdate");
		for( u32 i = 0; i < mDecals.fCount( ); ++i )
		{
			if( !mDecals[ i ]->fSceneGraph( ) )
			{
				mDecals.fErase( i );
				--i;
			}
		}
	}

}

