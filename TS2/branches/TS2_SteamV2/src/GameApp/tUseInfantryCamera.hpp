#ifndef __tUseInfantryCamera__
#define __tUseInfantryCamera__
#include "tUseUnitCamera.hpp"
#include "Math/tDamped.hpp"

namespace Sig
{
	class tPlayer;
	class tUserControllableCharacterLogic;

	class base_export tUseInfantryCamera : public tUseUnitCamera
	{
		define_dynamic_cast( tUseInfantryCamera, tUseUnitCamera );
	public:
		explicit tUseInfantryCamera( tPlayer& player, tUserControllableCharacterLogic& charLogic );
	protected:

		virtual void fOnActivate( b32 active );
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );

		tUserControllableCharacterLogic &mCharacter;
		f32 mPreviousZoom;
	};
}


#endif//__tUseVehicleCamera__

