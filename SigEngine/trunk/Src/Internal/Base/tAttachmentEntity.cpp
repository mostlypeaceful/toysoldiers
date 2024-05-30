#include "BasePch.hpp"
#include "tAttachmentEntity.hpp"

namespace Sig
{
	tAttachmentEntityDef::tAttachmentEntityDef( )
		: mStateMask( tStateableEntity::cMaxStateMaskValue )
	{
	}

	tAttachmentEntityDef::tAttachmentEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
	{
	}

	tAttachmentEntityDef::~tAttachmentEntityDef( )
	{
	}

	void tAttachmentEntityDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		tAttachmentEntity* entity = NEW_TYPED( tAttachmentEntity )( this, mBounds, mObjectToLocal );
		fApplyPropsAndSpawnWithScript( *entity, params );
	}

	tAttachmentEntity::tAttachmentEntity( const tAttachmentEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld )
		: tStateableEntity( entityDef ? entityDef->mStateMask : tStateableEntity::cMaxStateMaskValue )
		, mEntityDef( entityDef )
	{
		fMoveTo( objectToWorld );
	}

	void tAttachmentEntity::fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton )
	{
		fPropagateSkeletonInternal( skeleton, mEntityDef );
	}
}

