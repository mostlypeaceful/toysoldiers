#ifndef __tRPGCamera__
#define __tRPGCamera__
#include "tUseUnitCamera.hpp"
#include "Math/tDamped.hpp"

namespace Sig
{
	class tPlayer;
	class tUserControllableCharacterLogic;

	class base_export tRPGCamera : public tUseUnitCamera
	{
		define_dynamic_cast( tRPGCamera, tUseUnitCamera );
	public:
		explicit tRPGCamera( tPlayer& player, tUserControllableCharacterLogic& charLogic );

		virtual void fOnActivate( b32 active );

	protected:
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );
		void fRaycastCorrectCamera( f32 dt, Gfx::tTripod& tripod );

		tUserControllableCharacterLogic &mCharacter;
		b16				mFirstTick;
		b16				mCollidedLastFrame;
		Gfx::tTripod	mTripod;
		Math::tDampedFloat mCollisionBlend;
		Math::tVec3f	mCollisionEye;
	};
}


#endif//__tRPGCamera__

