#ifndef __tShellCamera__
#define __tShellCamera__
#include "tUseUnitCamera.hpp"
#include "tWeapon.hpp"
#include "Math/tDamped.hpp"

namespace Sig
{
	class tPlayer;
	class tProjectileLogic;
	class tWeapon;

	class base_export tShellCamera : public tUseUnitCamera
	{
		define_dynamic_cast( tShellCamera, tUseUnitCamera );
	public:
		explicit tShellCamera( tPlayer& player, const tGrowableArray<tProjectileLogic*>& projLogics, tWeapon* weapon );
		~tShellCamera( );

		void fProjectileDied( tProjectileLogic* proj );
		void fAddProjectile( tProjectileLogic* proj ) { mProjLogics.fPushBack( proj ); }
		void fBurst( );
		b32 fPushed( ) const { return mPushed; }
		b32 fExit( ) const;

		u32 fInputFilterLevel( ) { return mInputFilter; }

	protected:
		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );
		virtual b32 fCleanup( );
		virtual void fOnRemove( );

		void fShellCamRayCastCorrectNewTripod( Gfx::tTripod& tripod, f32 dt );
		void fHideOverlay( b32 keepEffect );
		void fSetExposureMult( f32 mult );

		tPlayer& mPlayer;
		tGrowableArray<tProjectileLogic*> mProjLogics;
		tProjectileLogic* mFollowProj;
		Gfx::tTripod mLastTripod; //last 100% accurate tripod
		Gfx::tTripod mRealTripod; //last/current blended tripod

		tWeaponPtr mWeapon;

		Math::tMat3f mShellCamOffset;
		Math::tDampedFloat mLeanBlend;
		tGrowableArray<Math::tMat3f> mDeathSpots;

		f32 mDeathTimer;
		f32 mDetonateTimer;

		b8 mBursted;
		b8 mPushed;
		b8 mInitialized;
		b8 mDepthLimited;

		b8 mOverlayShown;
		b8 mDoRayCastCorrect;
		b8 pad1;
		b8 pad2;

		Math::tVec3f mOriginalPos;
		Math::tVec3f mDepthLimitPos;

		f32 mBurstBlend;

		u32 mInputFilter;
	};

	typedef tRefCounterPtr<tShellCamera> tShellCameraPtr;
}


#endif//__tShellCamera__

