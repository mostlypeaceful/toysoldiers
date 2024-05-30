#ifndef __tDebugFreeCamera__
#define __tDebugFreeCamera__
#include "tFreeCamera.hpp"

namespace Sig { namespace Gfx
{
	///
	/// \brief Default debug/free-look camera.
	class base_export tDebugFreeCamera : public tFreeCamera
	{
		define_dynamic_cast( tDebugFreeCamera, tFreeCamera );
	public:
		static void fGlobalAppTick( );
		static tEntityPtr fGetDebugEntity( );
		static tLogic* fGetDebugEntityLogic( );
		static void fReleaseDebugEntity( );
		static void fSetDebugEntity( const tEntityPtr& e );
	public:
		tDebugFreeCamera( const tUserPtr& user );
		virtual void fOnTick( f32 dt );
		virtual void fOnActivate( b32 active );
	private:
		tEntityPtr		mFollowingEntity;
		Math::tVec3f	mFollowingEye;
		Math::tVec3f	mFollowingLookAt;
		Math::tVec2f	mPanVel;
		Math::tVec2f	mRotateVel;
		f32				mUpVel;
	};
}}


#endif//__tDebugFreeCamera__

