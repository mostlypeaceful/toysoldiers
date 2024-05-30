#include "BasePch.hpp"
#include "tLightProbeEntity.hpp"
#include "tSceneGraph.hpp"
#include "Gfx/tRenderContext.hpp"

namespace Sig
{
	const u32 tLightProbeEntity::cSpatialSetIndex = tSceneGraph::fNextSpatialSetIndex( );

	register_rtti_factory( tLightProbeEntityDef, true );

	tLightProbeEntityDef::tLightProbeEntityDef( )
	{
	}

	tLightProbeEntityDef::tLightProbeEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mHarmonics( cNoOpTag )
	{
	}

	tLightProbeEntityDef::~tLightProbeEntityDef( )
	{
	}

	void tLightProbeEntityDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		tLightProbeEntity* entity = NEW tLightProbeEntity( this, Math::tAabbf::cZeroSized, mObjectToLocal );
		fApplyPropsAndSpawnWithScript( *entity, params );
	}


	tLightProbeEntity::tLightProbeEntity( const tLightProbeEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld )
		: tSpatialEntity( objectSpaceBox )
		, mEntityDef( entityDef )
	{
		fMoveTo( objectToWorld );
	}

	void tLightProbeEntity::fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton )
	{
		fPropagateSkeletonInternal( skeleton, mEntityDef );
	}

	void tLightProbeEntity::fOnSpawn( )
	{
		// for lack of a better system. last probe spawned will apply itself ot the gloabl settings.
		Gfx::tRenderContext::gSphericalHarmonics = mEntityDef->mHarmonics;

		tSpatialEntity::fOnSpawn( );
	}
}

