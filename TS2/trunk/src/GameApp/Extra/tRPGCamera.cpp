#include "GameAppPch.hpp"
#include "tRPGCamera.hpp"
#include "tGameApp.hpp"
#include "tUserControllableCharacterLogic.hpp"
#include "Physics/tGroundRayCastCallback.hpp"

using namespace Sig::Math;

namespace Sig
{

	namespace
	{
		devvar( Math::tVec4f, Renderer_PostEffects_CharacterCam_DOF, Math::tVec4f( 0.994f, 0.298f, 3.2f, 0.5f ) );
		devvar( f32, Gameplay_Character_Camera_Lerp, 0.2f );
		devvar( f32, Gameplay_Character_Camera_OffsetFromCollision, 0.75f );
		devvar( f32, Gameplay_Character_Camera_CollisionSpring, 0.9f );
	}

	tRPGCamera::tRPGCamera( tPlayer& player, tUserControllableCharacterLogic& charLogic )
		: tUseUnitCamera( player, &charLogic, false )
		, mCharacter( charLogic )
		, mFirstTick( true )
		, mCollidedLastFrame( false )
		, mCollisionBlend( 0.f, 0.1f, 0.1f )
		, mCollisionEye( tVec3f::cZeroVector )
	{
		if( !tGameApp::fInstance( ).fExtraDemoMode( ) )
			mTargetDOF = Renderer_PostEffects_CharacterCam_DOF;
	}

	void tRPGCamera::fOnActivate( b32 active )
	{
		if( active ) mFirstTick = true;
		tUseUnitCamera::fOnActivate( active );
	}

	void tRPGCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
		fUserTick( dt, tripod );
	}

	void tRPGCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
		// calculate new tripod

		tVec2f rightStick = fPlayer( ).fAimStick( tUserProfile::cProfileCharacters );
		mCharacter.fUseCamZoomFromGamepad( dt, rightStick.y );

		tMat3f baseXform = tMat3f::cIdentity;
		baseXform.fOrientZAxis( mCharacter.fGetLookDirection( ) );
		baseXform.fSetTranslation( mCharacter.fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) );

		const Math::tVec3f offset = fUnitLogic( ).fUseCamOffset( ); // / fPlayer( ).fProjectionScale( );
		const Math::tVec3f lookDir = fUnitLogic( ).fUseCamLookTarget( ) - offset;

		Gfx::tTripod newTripod;
		newTripod.mUp = tVec3f::cYAxis;
		newTripod.mEye = baseXform.fXformPoint( offset );
		newTripod.mLookAt = newTripod.mEye + baseXform.fXformVector( lookDir );

		if( !mFirstTick )
			fBlendTripods( Gameplay_Character_Camera_Lerp, newTripod, mTripod );
		else
		{
			mFirstTick = false;
			mTripod = newTripod;
		}

		tripod = mTripod;
		fRaycastCorrectCamera( dt, tripod );

		// keep setting this so value can be tweaked
		//mTargetDOF = Renderer_PostEffects_CharacterCam_DOF;
	}

	void tRPGCamera::fRaycastCorrectCamera( f32 dt, Gfx::tTripod& tripod )
	{
		//shoot back from the look at point
		tRayf ray( tripod.mLookAt, tripod.mEye - tripod.mLookAt );
		Physics::tGroundRayCastCallback callback( *mCharacter.fOwnerEntity( ), GameFlags::cFLAG_GROUND );

		mCharacter.fSceneGraph( )->fRayCastAgainstRenderable( ray, callback );
		if( Physics::tGroundRayCastCallback::cShapesEnabledAsGround )
			mCharacter.fSceneGraph( )->fRayCast( ray, callback, tShapeEntity::cSpatialSetIndex );

		f32 blendTarget = 0.f;
		b32 collided = false;
		if( callback.mHit.fHit( ) )
		{
			blendTarget = 1.f;
			collided = true;

			mCollisionEye = ray.fEvaluate( callback.mHit.mT );

			ray.mExtent.fNormalize( );
			mCollisionEye -= ray.mExtent * Gameplay_Character_Camera_OffsetFromCollision;
		}

		if( collided && !mCollidedLastFrame )
			mCollisionBlend.fSetValue( 0.f );
		if( !collided && mCollidedLastFrame )
			mCollisionBlend.fSetValue( 1.f );

		mCollidedLastFrame = collided;

		mCollisionBlend.fSetBlends( Gameplay_Character_Camera_CollisionSpring );
		mCollisionBlend.fStep( blendTarget, dt );

		tripod.mEye = fLerp( tripod.mEye, mCollisionEye, mCollisionBlend.fValue( ) );
		if( collided ) mCollisionEye = tripod.mEye;
	}

}

