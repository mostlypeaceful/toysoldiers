#include "GameAppPch.hpp"
#include "tUseTurretCamera.hpp"
#include "tGameApp.hpp"
#include "tTurretLogic.hpp"
#include "tRtsCamera.hpp"

namespace Sig
{
	using namespace Sig::Math;

	devvar( f32, Gameplay_Turrets_UseCam_ScopeBlend, 0.2f );

	tUseTurretCamera::tUseTurretCamera( tPlayer& player, tTurretLogic& turretLogic )
		: tUseUnitCamera( player, &turretLogic, (!player.fIsLoading( ) && !turretLogic.fOnVehicle( ) && !turretLogic.fDontAlign( )) )
		, mTurret( turretLogic )
	{
		Math::tVec3f constrainedUserDir = turretLogic.fConstrainFacingDirection( turretLogic.fWorldToUser( mBlendInUnitMatrix.fZAxis( ).fNormalizeSafe( Math::tVec3f::cZAxis ) ) );
		mBlendInUnitMatrix.fOrientZAxis( turretLogic.fUserToWorld( constrainedUserDir ) );
		turretLogic.fSetUserDirection( constrainedUserDir );
		mTargetZoom = turretLogic.fUnitAttributeCameraFOV( );

		if( player.fIsLoading( ) || (turretLogic.fQuickSwitchCamera( ) && player.fQuickSwitchTurret( )) )
		{
			player.fResetQuickSwitchTurret( );
			mBlendLerpStart = -1.f;

			tRtsCamera* camera = player.fCameraStack( ).fFindCameraOfType<tRtsCamera>( );
			sigassert( camera );
			camera->fSetPreventPositionAcquisition( true );
		}
	}
	void tUseTurretCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
		if( mAlignUnitToCamera )
		{
			fUpdateZoom( dt );
			fKeepCameraAlignedToUnit( tripod, mBlendInUnitMatrix );
		}
		else
		{
			mHasBlendedInPercentage = 0.9f;
			fUserTick( dt, tripod );	
		}
	}
	void tUseTurretCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
		fUpdateZoom( dt );

		tMat3f basis;
		basis.fOrientYWithZAxis( mTurret.fUserToWorld( tVec3f::cYAxis ), fUnitEntity( )->fObjectToWorld( ).fZAxis( ) );
		basis.fSetTranslation( fUnitEntity( )->fObjectToWorld( ).fGetTranslation( ) );
		fKeepCameraAlignedToUnit( tripod, basis );

		if( mTurret.fUseCamScopePt( ) )
		{
			f32 targetScope = 0.f;
			if( fPlayer( ).fGamepad( ).fLeftTriggerHeld( ) )
				targetScope = 1.f;

			mScopeBlend.fSetBlends( Gameplay_Turrets_UseCam_ScopeBlend );
			mScopeBlend.fStep( targetScope, dt );

			Gfx::tTripod scopeTri( basis * mTurret.fUseCamScopePt( )->fParentRelative( ) );
			fBlendTripods( mScopeBlend.fValue( ), scopeTri, tripod );

			mTargetZoom = Math::fLerp( mTurret.fUnitAttributeCameraFOV( ), mTurret.fUnitAttributeScopeZoom( ), mScopeBlend.fValue( ) );
			mTurret.fWeaponStation( 0 )->fSetScopeBlend( mScopeBlend.fValue( ) );
		}
	}
	void tUseTurretCamera::fUpdateZoom( f32 dt )
	{
		if( fUnitLogic( ).fUnitAttributeAutoNearFarInterpolate( ) )
			fUnitLogic( ).fSetUseCamZoom( 1.0f - mTurret.fPitchBlendValue( ) );
		else
		{
			const b32 inversion = fPlayer( ).fProfile( )->fInversion( tUserProfile::cProfileTurrets );
			tVec2f stick = fPlayer( ).fMoveStick( tUserProfile::cProfileTurrets );

			f32 yVal = stick.y;
			if( inversion )
				yVal *= -1.f;

			fUnitLogic( ).fUseCamZoomFromGamepad( dt, yVal );
		}
	}
	tPlayer& tUseTurretCamera::fPlayer( )
	{
		return mTurret.fControllingPlayer( ) ? *mTurret.fControllingPlayer( ) : tUseUnitCamera::fPlayer( );
	}

}

