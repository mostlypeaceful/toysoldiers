#ifndef __tFrontEndCamera__
#define __tFrontEndCamera__
#include "Gfx/tCameraController.hpp"

namespace Sig
{
	class tPlayer;

	///
	/// \brief Default debug/free-look camera.
	class base_export tFrontEndCamera : public Gfx::tCameraController
	{
		define_dynamic_cast( tFrontEndCamera, Gfx::tCameraController );
	public:
		tFrontEndCamera( tPlayer& player );
		virtual void fOnTick( f32 dt );
	private:
		tPlayer& mPlayer;
	};
}


#endif//__tFrontEndCamera__

