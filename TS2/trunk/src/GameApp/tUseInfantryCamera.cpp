#include "GameAppPch.hpp"
#include "tUseInfantryCamera.hpp"
#include "tGameApp.hpp"
#include "tUserControllableCharacterLogic.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( bool, Gameplay_Character_DebugCam, false );
	devvar( f32, Gameplay_Character_CamShakeScale, 0.25f );

	devvar( Math::tVec4f, Renderer_PostEffects_CharacterCam_DOF, Math::tVec4f( 0.994f, 0.298f, 3.4f, 0.5f ) );


	tUseInfantryCamera::tUseInfantryCamera( tPlayer& player, tUserControllableCharacterLogic& charLogic )
		: tUseUnitCamera( player, &charLogic, false )
		, mCharacter( charLogic )
		, mPreviousZoom( 0.f )
	{
		if( !tGameApp::fInstance( ).fExtraDemoMode( ) )
			mTargetDOF = Renderer_PostEffects_CharacterCam_DOF;
		mCameraShakeScale = Gameplay_Character_CamShakeScale;
		mTargetZoom = charLogic.fUnitAttributeCameraFOV( );
	}

	void tUseInfantryCamera::fOnActivate( b32 active )
	{
		if( active )
		{
			fStartBlendIn( );
			mPreviousZoom = fUnitLogic( ).fUseCamZoom( );
		}
		else
			fUnitLogic( ).fSetUseCamZoom( mPreviousZoom );

		tUseUnitCamera::fOnActivate( active );
	}

	void tUseInfantryCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
		fUserTick( dt, tripod );
	}

	void tUseInfantryCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
		mCharacter.fSetUseCamZoom( Gameplay_Character_DebugCam ? 1.0f : 0.0f );

		// keep the camera aligned to unit
		const tMat3f& unitXform = fUnitEntity( )->fObjectToWorld( );

		// calculate new tripod
		fKeepCameraAlignedToUnit( tripod, unitXform );

		tQuatf pitch( tAxisAnglef( unitXform.fXAxis( ), mCharacter.fGetAimBlendDownUp( ) * mCharacter.fGetAimPitchMax( ) ) );
		
		tVec3f viewDir = tripod.mEye - tripod.mLookAt;
		viewDir = pitch.fRotate( viewDir );

		tripod.mEye = tripod.mLookAt + viewDir;

		// keep setting this so value can be tweaked
		mTargetDOF = Renderer_PostEffects_CharacterCam_DOF;
	}

}

