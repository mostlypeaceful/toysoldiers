#ifndef __tUseAirborneCamera__
#define __tUseAirborneCamera__
#include "tUseUnitCamera.hpp"
#include "Math/tDamped.hpp"

namespace Sig
{
	class tPlayer;
	class tAirborneLogic;

	class base_export tUseAirborneCamera : public tUseUnitCamera
	{
		define_dynamic_cast( tUseAirborneCamera, tUseUnitCamera );
	public:
		explicit tUseAirborneCamera( tPlayer& player, tAirborneLogic& airborneLogic );
	protected:
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );

		tAirborneLogic &mAirborne;

		// this stuff should probably be added to the base
		b32 mInitialized;
		Gfx::tTripod mOldTripod;
	};
}


#endif//__tUseAirborneCamera__

