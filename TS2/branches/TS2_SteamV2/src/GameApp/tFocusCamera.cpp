#include "GameAppPch.hpp"
#include "tFocusCamera.hpp"
#include "tGameApp.hpp"
#include "tGamePostEffectMgr.hpp"

using namespace Sig::Math;

namespace Sig
{

	tFocusCamera::tFocusCamera( tPlayer& player, const tEntityPtr target, f32 duration, f32 blendIn )
		: tUseUnitCamera( player, NULL, false )
		, mTarget( target )
		, mTimer( duration )
		, mHasChanged( false )
		, mWaitCount( 0 )
	{
		mTargetDOF = tGameApp::fInstance( ).fPostEffectsManager( )->fDefaultGameCamData( player.fUser( )->fViewportIndex( ) ).mDof;
		mTargetZoom = player.fDefaultZoom( );
		mBlendType = cBlendPureLerp;
		sigassert( target );
		mOverrideBlendDist = blendIn;
	}

	void tFocusCamera::fOnActivate( b32 active )
	{
		tUseUnitCamera::fOnActivate( active );
	}

	void tFocusCamera::fChangeTarget( tEntity* ent )
	{
		mHasChanged = true;
		mTarget.fReset( ent );
		fStartBlendIn( );
	}

	void tFocusCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
		fUserTick( dt, tripod );
	}

	void tFocusCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
		mTimer -= dt;
		tripod = mOriginalCamera.fGetTripod( );

		tVec3f delta = tripod.mLookAt - tripod.mEye;
		f32 originalLen;
		delta.fNormalizeSafe( tVec3f::cZAxis, originalLen );

		tVec3f newDelta = mTarget->fObjectToWorld( ).fGetTranslation( ) - tripod.mEye;

		newDelta.fNormalizeSafe( tVec3f::cZAxis, originalLen );
		newDelta *= originalLen;
		tripod.mLookAt = tripod.mEye + newDelta;

#ifdef sig_logging
		Math::tMat3f debug;
		tripod.fConstructCameraMatrix( debug );
		sigassert( !debug.fIsNan( ) );
#endif
	}

	//b32 tFocusCamera::fWantsAutoPop( ) const 
	//{ 
	//	return mTimer <= 0.f; 
	//} 

}

