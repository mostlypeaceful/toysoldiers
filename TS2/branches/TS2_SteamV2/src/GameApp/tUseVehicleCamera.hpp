#ifndef __tUseVehicleCamera__
#define __tUseVehicleCamera__
#include "tUseUnitCamera.hpp"
#include "Math/tDamped.hpp"

namespace Sig
{
	class tPlayer;
	class tWheeledVehicleLogic;

	class base_export tUseVehicleCamera : public tUseUnitCamera
	{
		define_dynamic_cast( tUseVehicleCamera, tUseUnitCamera );
	public:
		explicit tUseVehicleCamera( tPlayer& player, u32 seat, tWheeledVehicleLogic& vehicleLogic );
	protected:
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );

		tWheeledVehicleLogic	&mVehicle;
		u32						mSeat;
		Math::tPDDampedFloat	mZoomDist; //distance varies with speed
		Math::tPDDampedFloat	mLookIn; //camera "looks in" to the corners

		b32 mInitialized;
		Gfx::tTripod mOldTripod;
	};
}


#endif//__tUseVehicleCamera__

