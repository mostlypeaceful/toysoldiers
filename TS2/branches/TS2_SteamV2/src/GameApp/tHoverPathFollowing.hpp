#ifndef __tHoverPathFollowing__
#define __tHoverPathFollowing__

#include "tUnitPath.hpp"
#include "tHoverLogic.hpp"
#include "Gfx/tDebugGeometry.hpp"

namespace Sig
{
	class tHoverPathFollower
	{
	public:
		static bool fComputeInput( Physics::tHoverInput &inputOut, const tHoverLogic& vehicle, tUnitPath& path, const tGrowableArray<tVehicleOffender>& offenders, f32 dt, Gfx::tDebugGeometryContainer *debugging );

	};

}

#endif//__tHoverPathFollowing__
