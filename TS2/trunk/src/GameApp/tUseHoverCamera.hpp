#ifndef __tUseHoverCamera__
#define __tUseHoverCamera__
#include "tUseUnitCamera.hpp"
#include "Math/tDamped.hpp"

namespace Sig
{
	class tPlayer;
	class tHoverLogic;

	class base_export tUseHoverCamera : public tUseUnitCamera
	{
		define_dynamic_cast( tUseHoverCamera, tUseUnitCamera );
	public:
		explicit tUseHoverCamera( tPlayer& player, tHoverLogic& HoverLogic );
	protected:
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );

		tHoverLogic &mHover;

		// this stuff should probably be added to the base
		b32 mInitialized;
		Gfx::tTripod mOldTripod;
	};
}


#endif//__tUseHoverCamera__

