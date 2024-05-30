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

	void tAttachmentEntityDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		tAttachmentEntity* entity = NEW tAttachmentEntity( this, mBounds, mObjectToLocal );
		fApplyPropsAndSpawnWithScript( *entity, parent, creationFlags );
	}

	tAttachmentEntity::tAttachmentEntity( const tAttachmentEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld )
		: tStateableEntity( entityDef ? entityDef->mStateMask : tStateableEntity::cMaxStateMaskValue )
		, mEntityDef( entityDef )
	{
		fMoveTo( objectToWorld );
	}

	void tAttachmentEntity::fPropagateSkeleton( tAnimatedSkeleton& skeleton )
	{
		fPropagateSkeletonInternal( skeleton, mEntityDef );
	}
}

