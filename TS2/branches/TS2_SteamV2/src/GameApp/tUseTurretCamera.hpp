#ifndef __tUseTurretCamera__
#define __tUseTurretCamera__
#include "tUseUnitCamera.hpp"

namespace Sig
{
	class tPlayer;
	class tTurretLogic;

	class base_export tUseTurretCamera : public tUseUnitCamera
	{
		define_dynamic_cast( tUseTurretCamera, tUseUnitCamera );
	public:
		explicit tUseTurretCamera( tPlayer& player, tTurretLogic& turretLogic );

		f32 fScopeBlend( ) const { return mScopeBlend.fValue( ); }

	private:
		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );

		tTurretLogic& mTurret;
		void fUpdateZoom( f32 dt );

		tPlayer& fPlayer( );

		Math::tDampedFloat mScopeBlend;
	};

	typedef tRefCounterPtr< tUseTurretCamera > tUseTurretCameraPtr;

}

#endif//__tUseTurretCamera__

