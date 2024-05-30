#include "BasePch.hpp"
#include "tSgFileRefEntity.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tLightEntity.hpp"

// TODO REFACTOR
// Resolve removal of all children issue (probably should delete itself - same functionality needed for tFxFileRefEntity)

namespace Sig
{
	tSgFileRefEntity::tSgFileRefEntity( const tResourcePtr& sgResource, const tEntity* proxy )
		: tSceneRefEntity( sgResource, proxy )
	{
		mSgResourceLoaded.fFromMethod< tSgFileRefEntity, &tSgFileRefEntity::fOnSgResourceLoaded >( this );

		sigassert( !mSgResource.fNull( ) );
		mSgResource->fLoadDefault( this );
		mSgResource->fCallWhenLoaded( mSgResourceLoaded );
	}
	tSgFileRefEntity::~tSgFileRefEntity( )
	{
		// leave this on for the duration of destructor so that we don't try to re-delete self
		mClearingChildren = true;

		// important: clear children before unloading scene graph file
		fClearChildren( );

		// now unload scene graph file
		mSgResource->fUnload( this );
	}
	Gfx::tLightEntity* tSgFileRefEntity::fSpawnDefaultLight( tEntity& lightParent, const Gfx::tScreenPtr& screen )
	{
		const tSceneGraphFile* sgFile = mSgResource->fCast< tSceneGraphFile >( );
		sigassert( sgFile );

		Gfx::tLightEntityPtr lightEntity( Gfx::tLightEntity::fCreateDefaultLight( screen, *sgFile ) );
		if( lightEntity )
			lightEntity->fSpawn( lightParent );
		return lightEntity.fGetRawPtr( );
	}
	void tSgFileRefEntity::fOnSgResourceLoaded( tResource& theResource, b32 success )
	{
		mClearingChildren = true;
		fClearChildren( );
		mClearingChildren = false;

		if( !success ) return;

		sigassert( mSgResource == &theResource );
		fCollectEntities( tEntityCreationFlags( ) );
	}

}
