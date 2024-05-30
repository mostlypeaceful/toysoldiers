#include "BasePch.hpp"
#include "tCameraEntity.hpp"

namespace Sig
{
	tCameraEntityDef::tCameraEntityDef( )
	{
	}

	tCameraEntityDef::tCameraEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mLensProperties( cNoOpTag )
	{
	}

	tCameraEntityDef::~tCameraEntityDef( )
	{
	}

	void tCameraEntityDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		tCameraEntity* entity = NEW tCameraEntity( mObjectToLocal, *this );
		fApplyPropsAndSpawnWithScript( *entity, params );
	}
}

namespace Sig
{

	tCameraEntity::tCameraEntity( const Math::tMat3f& objectToWorld, const tCameraEntityDef& def )
		: mLensProperties( def.mLensProperties )
		, mEntityDef( &def )
	{
		fMoveTo( objectToWorld );
	}

	void tCameraEntity::fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton )
	{
		fPropagateSkeletonInternal( skeleton, mEntityDef );
	}

}
