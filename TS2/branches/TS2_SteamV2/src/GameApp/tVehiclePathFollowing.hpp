#ifndef __tVehiclePathFollowing__
#define __tVehiclePathFollowing__

#include "Gfx/tDebugGeometry.hpp"
#include "Math/tDamped.hpp"
#include "Physics/tVehiclePhysics.hpp"

namespace Sig
{
	class tWheeledVehicleLogic;
	class tUnitPath;
	class tVehicleOffender;

	class tVehiclePathFollower
	{
	public:
		bool fComputeInput( Physics::tVehicleInput &inputOut, const tWheeledVehicleLogic& vehicle, tUnitPath& path, const tGrowableArray<tVehicleOffender>& offenders, f32 dt, Gfx::tDebugGeometryContainer *debugging );

	private:
		Math::tDampedFloat mIntendedSpeed;
	};

}

#endif//__tVehiclePathFollowing__
