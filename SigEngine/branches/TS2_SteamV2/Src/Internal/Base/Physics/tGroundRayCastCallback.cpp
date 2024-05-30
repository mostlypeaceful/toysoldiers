#include "BasePch.hpp"
#include "tGroundRayCastCallback.hpp"
#include "tShapeEntity.hpp"
#include "tSceneGraphCollectTris.hpp"

namespace Sig { namespace Physics
{
	b32 tGroundRayCastCallback::cShapesEnabledAsGround = false;

	void tGroundRayCastCallback::fRayCast( tSceneGraph& sg, const Math::tRayf& ray )
	{
		sg.fRayCastAgainstRenderable( ray, *this );
		if( cShapesEnabledAsGround )
			sg.fRayCast( ray, *this, tShapeEntity::cSpatialSetIndex );
	}
}}

