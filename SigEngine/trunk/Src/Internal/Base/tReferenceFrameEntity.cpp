#include "BasePch.hpp"
#include "tReferenceFrameEntity.hpp"
#include "tEntityDef.hpp"

namespace Sig
{

	tReferenceFrameEntity::tReferenceFrameEntity( const tEntityDefProperties* entityDef )
		: mEntityDef( entityDef )
	{
	}
	tReferenceFrameEntity::tReferenceFrameEntity( const tEntityDefProperties* entityDef, const Math::tMat3f& objectToWorld )
		: mEntityDef( entityDef )
	{
		fMoveTo( objectToWorld );
	}
	void tReferenceFrameEntity::fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton )
	{
		fPropagateSkeletonInternal( skeleton, mEntityDef );
	}

}

