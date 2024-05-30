#include "GameAppPch.hpp"
#include "tUseAirborneCamera.hpp"
#include "tGameApp.hpp"
#include "tAirborneLogic.hpp"

using namespace Sig::Math;

namespace Sig
{


	tUseAirborneCamera::tUseAirborneCamera( tPlayer& player, tAirborneLogic& airborneLogic )
		: tUseUnitCamera( player, &airborneLogic, false )
		, mAirborne( airborneLogic )
		, mInitialized( false )
	{
		mTargetZoom = airborneLogic.fUnitAttributeCameraFOV( );
	}
	void tUseAirborneCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
		Gfx::tTripod newTripod;

		const tMat3f& xform = mAirborne.fOwnerEntity( )->fObjectToWorld( );

		tVec3f flatX = tVec3f::cYAxis.fCross( xform.fZAxis( ) );
		flatX.fNormalizeSafe( xform.fXAxis( ) );

		tMat3f flatXform = xform;
		flatXform.fOrientZWithXAxis( xform.fZAxis( ), flatX );

		f32 bombCamBlend = mAirborne.fInBombCam( ) ? 1.f : 0.f;
		mAirborne.fSetUseCamZoom( bombCamBlend );
		fKeepCameraAlignedToUnit( newTripod, flatXform );

		mTargetZoom = Math::fLerp( mAirborne.fUnitAttributeCameraFOV( ), mAirborne.fUnitAttributeScopeZoom( ), bombCamBlend );
		mTargetZoom += mAirborne.fPhysics( ).fIdealZoom( );

		if( !mInitialized )
		{
			mInitialized = true;
			mOldTripod = newTripod;
		}

		f32 lerpFactor = 0.3f;
		const f32 frameRateCompensation = dt / (1.f/30.f);
		const f32 camBlendT = frameRateCompensation * lerpFactor;
		fBlendTripods( camBlendT, newTripod, mOldTripod );
		tripod = mOldTripod;
	}
	void tUseAirborneCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
		fUserTick( dt, tripod );
	}

}

