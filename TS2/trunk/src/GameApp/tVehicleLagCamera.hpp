#ifndef __tVehicleLagCamera__
#define __tVehicleLagCamera__
#include "tUseUnitCamera.hpp"
#include "Math/tDamped.hpp"

namespace Sig
{
	class tPlayer;
	class tWheeledVehicleLogic;

	class base_export tVehicleLagCamera : public tUseUnitCamera
	{
		define_dynamic_cast( tVehicleLagCamera, tUseUnitCamera );
	public:
		explicit tVehicleLagCamera( tPlayer& player, u32 seat, tWheeledVehicleLogic& vehicleLogic );
	protected:
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );

		tWheeledVehicleLogic	&mVehicle;
		u32						mSeat;
		Math::tPDDampedFloat	mZoomDist; //distance varies with speed
		Math::tPDDampedFloat	mLook; //camera "looks" when stick is deflected
		Math::tPDDampedFloat	mZoomBlend;
		Math::tPDDampedFloat	mLookTorque; //smooth out changes in yaw velocity
		f32 mInitialZoom;

		b8 mInitialized;
		b8 mCenterLook; //user requested the look dir to center itself
		b8 pad0;
		b8 pad1;

		Gfx::tTripod mOldTripod;

		Math::tVec3f mVel;
	};
}


#endif//__tVehicleLagCamera__

