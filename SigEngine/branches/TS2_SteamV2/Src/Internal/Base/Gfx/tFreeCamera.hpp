#ifndef __tFreeCamera__
#define __tFreeCamera__
#include "Math/tIntegratedX.hpp"
#include "tCameraController.hpp"

namespace Sig { namespace Gfx
{
	///
	/// \brief Default debug/free-look camera.
	class base_export tFreeCamera : public tCameraController
	{
		define_dynamic_cast( tFreeCamera, tCameraController );
	public:
		tFreeCamera( const tUserPtr& user );
		const Input::tGamepad& fGamepad( ) const;
		virtual void fOnTick( f32 dt );
		virtual void fOnActivate( b32 active );
	protected:
		tIntegratedV<f32> mYawSpeed;
		tIntegratedV<f32> mPitchSpeed;
		tIntegratedV<Math::tVec3f>	mPanVelocity;
		b32 mLockInXZPlane;
		u32 mGamepadFilterIndex;
	};
}}


#endif//__tFreeCamera__

