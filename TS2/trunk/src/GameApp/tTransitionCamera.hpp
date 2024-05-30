#ifndef __tTransitionCamera__
#define __tTransitionCamera__
#include "Gfx/tCameraController.hpp"
#include "tPlayer.hpp"

namespace Sig
{
	///
	/// \brief Default debug/free-look camera.
	class base_export tTransitionCamera : public Gfx::tCameraController
	{
		define_dynamic_cast( tTransitionCamera, Gfx::tCameraController );
	public:
		tTransitionCamera( tPlayer& player, const Math::tMat3f& start, const Math::tMat3f& end, f32 transitionTime );
		virtual ~tTransitionCamera( );
		virtual void fOnTick( f32 dt );
		virtual void fOnActivate( b32 active );
		b32 fTransitionComplete( ) const { return mCounter >= mTransitionTime; }

	private:
		tPlayer&		mPlayer;
		Math::tMat3f	mStartXForm;
		Math::tMat3f	mEndXForm;
		f32				mTransitionTime;
		f32				mCounter;
	};
}


#endif//__tTransitionCamera__

