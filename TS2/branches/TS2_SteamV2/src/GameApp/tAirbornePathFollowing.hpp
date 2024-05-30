#ifndef __tAirbornePathFollowing__
#define __tAirbornePathFollowing__

#include "tUnitPath.hpp"
#include "tAirborneLogic.hpp"
#include "Gfx/tDebugGeometry.hpp"

namespace Sig
{
	class tAirbornePathFollower
	{
	public:
		static bool fComputeInput( Physics::tAirborneInput &inputOut, const tAirborneLogic& vehicle, tUnitPath& path, const tGrowableArray<tVehicleOffender>& offenders, f32 dt, Gfx::tDebugGeometryContainer *debugging );

	};

}

#endif//__tAirbornePathFollowing__
