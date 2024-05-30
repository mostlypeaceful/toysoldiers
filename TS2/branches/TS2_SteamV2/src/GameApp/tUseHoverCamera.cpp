#include "GameAppPch.hpp"
#include "tUseHoverCamera.hpp"
#include "tGameApp.hpp"
#include "tHoverLogic.hpp"

using namespace Sig::Math;

namespace Sig
{

	tUseHoverCamera::tUseHoverCamera( tPlayer& player, tHoverLogic& HoverLogic )
		: tUseUnitCamera( player, &HoverLogic, false )
		, mHover( HoverLogic )
		, mInitialized( false )
	{
		mTargetZoom = HoverLogic.fUnitAttributeCameraFOV( );
	}

	void tUseHoverCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
		fUserTick( dt, tripod );
	}

	void tUseHoverCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
		Gfx::tTripod newTripod;
		fKeepCameraAlignedToUnit( newTripod, mHover.fCameraMovement( ).fCameraBasis( ) );

		if( !mInitialized )
		{
			mInitialized = true;
			mOldTripod = newTripod;
		}
		
		const f32 frameRateCompensation = dt / (1.f/30.f);
		const f32 camBlendT = frameRateCompensation * mHover.fCameraLerp( );
		fBlendTripods( camBlendT, newTripod, mOldTripod );
		tripod = mOldTripod;
	}

}

