#include "GameAppPch.hpp"
#include "tUseUnitCamera.hpp"
#include "tGameApp.hpp"
#include "tUnitLogic.hpp"
#include "tLevelLogic.hpp"
#include "tSceneGraphCollectTris.hpp"

namespace Sig
{

	devvar( f32, Gameplay_Vehicle_Camera_DistFromTerrain, 1.25f );

	namespace
	{

		struct tTerrainCameraRayCastCallback
		{
			mutable Math::tRayCastHit		mHit;
			mutable tEntity*		mHitEntity;
			tEntity*				mOwnerEnt;
			tEntity*				mIgnore;

			tTerrainCameraRayCastCallback( tEntity* ownerEnt, tEntity* ignore = 0 ) 
				: mHitEntity( 0 ), mOwnerEnt( ownerEnt ), mIgnore( ignore )
			{
			}

			inline void fRayCastAgainstSpatial( const Math::tRayf& ray, tSpatialEntity* spatial ) const
			{
				if( spatial->fToSpatialSetObject( )->fQuickRejectByBox( ray ) )
					return;

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( !hit.fHit( ) )
					return;

				if( hit.mT < mHit.mT )
				{
					mHit = hit;
					mHitEntity = spatial;
				}	
			}

			inline void operator( )( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;

				tSpatialEntity* spatial = static_cast<tSpatialEntity*>( i );
				if( !spatial->fHasGameTagsAny( GameFlags::cFLAG_GROUND | GameFlags::cFLAG_COLLISION ) )
					return;
				if( mOwnerEnt && (mOwnerEnt == spatial || spatial->fIsAncestorOfMine( *mOwnerEnt )) )
					return;
				if( mIgnore && (mIgnore == spatial || spatial->fIsAncestorOfMine( *mIgnore )) )
					return;

				fRayCastAgainstSpatial( ray, spatial );
			}
		};
	}

	tUseUnitCamera::tUseUnitCamera( tPlayer& player, tUnitLogic* unitLogic, b32 alignUnitToCamera )
		: Gfx::tCameraController( player.fUser( )->fViewport( ) )
		, mZoomDist( 0.0f )
		, mBlendInUnitMatrix( Math::tMat3f::cIdentity )
		, mAlignUnitToCamera( alignUnitToCamera )
		, mHasBlendedIn( false )
		, mBlendType( cBlendPreferEye )
		, mStepWhenPaused( false )
		, mRaycastTerrainPenetration( false )
		, mBlendLerpStart( 0.02f )
		, mBlendLerpEnd( 0.35f )
		, mHasBlendedInPercentage( 1.0f )
		, mRaycastTerrainPen( 0.f )
		, mPlayer( player )
		, mUnitLogic( unitLogic )
		, mUnitEntity( unitLogic ? unitLogic->fOwnerEntity( ) : NULL )
		, mState( cStateInactive )
		, mBlendInDist( 0.0f )
		, mBlendInStartDist( -1.f )
		, mOriginalDOF( Math::tVec4f::cOnesVector )
		, mTargetDOF( tGameApp::fInstance( ).fPostEffectsManager( )->fDefaultGameCamData( mPlayer.fUser( )->fViewportIndex( ) ).mDof )
		, mTargetZoom( 1.f )
		, mOverrideBlendDist( -1.f ) 
	{
		fAddUser( player.fUser( ) );
		fStartBlendIn( ); // we need the blend in matrices valid here
	}
	void tUseUnitCamera::fOnActivate( b32 active )
	{
		if( active )
		{
			// Blend in again if reactivated
			//  But this line is to prevent, creating and activating, from calling fStartBlendIn twice.
			if( mBlendLerpStart < 0.f )
				fSkipBlendIn( );
			else if( mState != cStateBlendIn ) 
				fStartBlendIn( );
		}
		else
		{
			mState = cStateInactive;
			mHasBlendedIn = false;
		}

		Gfx::tCameraController::fOnActivate( active );
	}
	void tUseUnitCamera::fStartBlendIn( )
	{
		mState = cStateBlendIn;
		mBlendInDist = 0.0f;
		mBlendInStartDist = -1.f;
		mOriginalCamera = fViewport( )->fLogicCamera( );
		mOriginalDOF = tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamData( fViewport( )->fViewportIndex( ) ).mDof;
		mHasBlendedIn = false;

		if( mUnitEntity )
		{
			if( mAlignUnitToCamera )
			{
				mBlendInUnitMatrix.fOrientZAxis( mOriginalCamera.fZAxis( ).fProjectToXZAndNormalize( ) );
				mBlendInUnitMatrix.fSetTranslation( mUnitEntity->fObjectToWorld( ).fGetTranslation( ) );
			}
			else
			{
				mBlendInUnitMatrix.fOrientZAxis( mUnitEntity->fObjectToWorld( ).fZAxis( ).fProjectToXZAndNormalize( ) );
				mBlendInUnitMatrix.fSetTranslation( mUnitEntity->fObjectToWorld( ).fGetTranslation( ) );
			}
		}
	}
	void tUseUnitCamera::fOnTick( f32 dt )
	{
		if( !fIsActive( ) || (tGameApp::fInstance( ).fSceneGraph( )->fIsPaused( ) && !mStepWhenPaused) )
			return;

		if( fCleanup( ) ) 
			return; //camera was deleted, jump out we dont exist anymore.

		Gfx::tCamera camera = fViewport( )->fLogicCamera( );

		switch( mState )
		{
		case cStateBlendIn:
			fOnTickBlendInControl( dt, camera );
			break;
		case cStateUserControl:
			if( fCheckForAndHandleExit( ) )
				return;
			fOnTickUserControl( dt, camera );
			break;
		}

		fStepCameraShake( dt, camera );

		fViewport( )->fSetCameras( camera );
	}
	void tUseUnitCamera::fKeepCameraAlignedToUnit( Gfx::tTripod& tripod, const Math::tMat3f& baseXform )
	{
		const Math::tVec3f lookDir = fUnitLogic( ).fUseCamLookDir( );
		Math::tVec3f viewDir = baseXform.fXformVector( lookDir ).fNormalize( );
		Math::tVec3f zoom = viewDir * mZoomDist;

		const Math::tVec3f offset = fUnitLogic( ).fUseCamOffset( );
		tripod.mEye = baseXform.fXformPoint( offset ) + zoom;
		tripod.mLookAt = tripod.mEye + viewDir * fUnitLogic( ).fDistToUnit( offset, lookDir ) - zoom;
		tripod.mUp = Math::tVec3f::cYAxis;
	}
	b32 tUseUnitCamera::fCheckForAndHandleExit( b32 forceExit )
	{
		b32 exit = forceExit;
		
		if( mUnitLogic && !mUnitLogic->fExtraMode( ) )
		{
			const tGameControllerPtr gc = mPlayer.fGameController( );
			b32 exitButtonPressed = ( gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_ENTER_EXIT_UNIT ) && gc->fMode( ) == tGameController::KeyboardMouse ) || ( gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_CANCEL ) && gc->fMode( ) == tGameController::GamePad );

			exit = exit || ( exitButtonPressed && ( ( !mPlayer.fLockedInUnit( ) && !mUnitLogic->fDisableControl( ) ) || tPlayer::fDebugRandyMode( ) ) );
			if( exit )
			{
				Gfx::tCameraControllerPtr makeSureIDontGetDeletedInNextFunctionCall( this );
				if( mUnitLogic->fEndUse( mPlayer ) )
					mUnitLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_CONTROL_END ) );
			}
		}

		return exit;
	}
	void tUseUnitCamera::fOnTickBlendInControl( f32 dt, Gfx::tCamera& camera )
	{
		const f32 cBlendThresh = 0.025f;
		const f32 fps = 30.0f;

		if( mUnitEntity )
			mBlendInUnitMatrix.fSetTranslation( mUnitEntity->fObjectToWorld( ).fGetTranslation( ) );

		// get target values
		Gfx::tTripod userTarget;
		fUserBlendIn( dt, userTarget );

		if( mRaycastTerrainPenetration )
			fRayCastCorrectNewTripod( userTarget, dt );

		if( mBlendInStartDist < 0.f )
		{
			// initializes blend distance on first tick
			if( mOverrideBlendDist > 0.f )
			{
				mBlendInStartDist = mOverrideBlendDist;
				mBlendInDist = mBlendInStartDist;
			}
			else
			{
				Gfx::tTripod tripod = camera.fGetTripod( );
				f32 dist = ( tripod.mEye - userTarget.mEye ).fLength( );

				if( dist < cBlendThresh * 2 )
					dist = 7.f; //fail safe big blend number

				mBlendInStartDist = dist; 
				mBlendInDist = mBlendInStartDist;
			}
		}

		if( mBlendInDist <= cBlendThresh )
		{
			mState = cStateUserControl;
			camera.fSetTripod( userTarget );
			mAlignUnitToCamera = false; //only do this on the first blend in, for shell cam's sake
			mHasBlendedIn = true;
		}
		else
		{
			// blend camera
			const f32 distZeroToOne = 1.0f - fClamp(mBlendInDist/mBlendInStartDist, 0.0f, 1.0f);
			Gfx::tTripod tripod = mOriginalCamera.fGetTripod( );

			if( distZeroToOne > mHasBlendedInPercentage ) mHasBlendedIn = true;

			switch( mBlendType )
			{
			case cBlendPreferEye: fBlendTripods( distZeroToOne, userTarget, tripod ); break;
			case cBlendPreferLookAt: fBlendTripodsPreferLookAt( distZeroToOne, userTarget, tripod ); break;
			case cBlendPureLerp: fBlendTripodsPureLerp( distZeroToOne, userTarget, tripod ); break;
			default: sigassert( !"Invalid blend type" );
			}

			// blend FOV
			Gfx::tLens lens = mOriginalCamera.fGetLens( );
			lens.fChangeZoom( Math::fLerp( lens.mZoom, mTargetZoom, distZeroToOne ) );
			camera.fSetTripodAndLens( tripod, lens );

			// blend DOF
			tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamData( fViewport( )->fViewportIndex( ) ).mDof = fLerp( mOriginalDOF, mTargetDOF, distZeroToOne );

			// blend distance
			const f32 frameRateCompensation = dt / (1.f/fps);
			f32 distT = frameRateCompensation * Math::fLerp( mBlendLerpStart, mBlendLerpEnd, distZeroToOne );
			mBlendInDist = Math::fLerp(mBlendInDist, 0.0f, distT);

			if( mAlignUnitToCamera )
			{
				// turn turret toward proper facing direction
				Math::tMat3f unitXform = mUnitEntity->fObjectToWorld( );
				const Math::tVec3f currentUnitFacing = unitXform.fZAxis( ).fNormalize( );
				Math::tVec3f targetUnitFacing = mBlendInUnitMatrix.fZAxis( );
				if( fEqual( currentUnitFacing.fDot( targetUnitFacing ), -1.f, 0.1f ) )
					targetUnitFacing = mBlendInUnitMatrix.fXAxis( );
				const Math::tVec3f newUnitFacing = Math::fNLerp( currentUnitFacing, targetUnitFacing, frameRateCompensation * distZeroToOne );
				unitXform.fOrientZAxis( newUnitFacing );
				mUnitEntity->fMoveTo( unitXform );
			}
		}

	}
	void tUseUnitCamera::fOnTickUserControl( f32 dt, Gfx::tCamera& camera )
	{
		Gfx::tTripod tripod = camera.fGetTripod( );
		Gfx::tLens lens = camera.fGetLens( );

		lens.fChangeZoom( mTargetZoom );
		fUserTick( dt, tripod );

		if( mRaycastTerrainPenetration )
			fRayCastCorrectNewTripod( tripod, dt );

		camera.fSetTripodAndLens( tripod, lens );

		tGamePostEffectManagerPtr p = tGameApp::fInstance( ).fPostEffectsManager( );
		p->fCurrentGameCamData( fViewport( )->fViewportIndex( ) ).mDof = mTargetDOF;
	}
	void tUseUnitCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
	}
	void tUseUnitCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
	}
	void tUseUnitCamera::fBlendTripods( f32 lerp, const Gfx::tTripod& newTripod, Gfx::tTripod& oldTripod )
	{
		f32 newDist, oldDist;
		Math::tVec3f newLookDir = ( newTripod.mLookAt - newTripod.mEye ).fNormalize( newDist );
		Math::tVec3f oldLookDir = ( oldTripod.mLookAt - oldTripod.mEye ).fNormalize( oldDist );

		oldTripod.mEye = Math::fLerp( oldTripod.mEye, newTripod.mEye, lerp );
		oldTripod.mLookAt = oldTripod.mEye + Math::fNLerp( oldLookDir, newLookDir, lerp ) * Math::fLerp( oldDist, newDist, lerp );
		oldTripod.mUp = Math::fNLerp( oldTripod.mUp, newTripod.mUp, lerp );
	}
	void tUseUnitCamera::fBlendTripodsPreferLookAt( f32 lerp, const Gfx::tTripod& newTripod, Gfx::tTripod& oldTripod )
	{
		f32 newDist, oldDist;
		Math::tVec3f newLookDir = ( newTripod.mLookAt - newTripod.mEye ).fNormalize( newDist );
		Math::tVec3f oldLookDir = ( oldTripod.mLookAt - oldTripod.mEye ).fNormalize( oldDist );

		oldTripod.mLookAt = Math::fLerp( oldTripod.mLookAt, newTripod.mLookAt, lerp );
		oldTripod.mEye = oldTripod.mLookAt - Math::fNLerp( oldLookDir, newLookDir, lerp ) * Math::fLerp( oldDist, newDist, lerp );
		oldTripod.mUp = Math::fNLerp( oldTripod.mUp, newTripod.mUp, lerp );
	}
	void tUseUnitCamera::fBlendTripodsPureLerp( f32 lerp, const Gfx::tTripod& newTripod, Gfx::tTripod& oldTripod )
	{
		oldTripod.mLookAt = Math::fLerp( oldTripod.mLookAt, newTripod.mLookAt, lerp );
		oldTripod.mEye = Math::fLerp( oldTripod.mEye, newTripod.mEye, lerp );
		oldTripod.mUp = Math::fNLerp( oldTripod.mUp, newTripod.mUp, lerp );
	}

	void tUseUnitCamera::fRayCastCorrectNewTripod( Gfx::tTripod& tripod, f32 dt )
	{
		Math::tVec3f dir = tripod.mEye - tripod.mLookAt;
		f32 len;
		dir.fNormalizeSafe( Math::tVec3f::cZAxis, len );
		
		f32 totalLen = len + Gameplay_Vehicle_Camera_DistFromTerrain;
		Math::tRayf ray( tripod.mLookAt, dir * totalLen );

		tTerrainCameraRayCastCallback cb( mUnitEntity.fGetRawPtr( ), NULL );
		tGameApp::fInstance( ).fSceneGraph( )->fRayCastAgainstRenderable( ray, cb );

		f32 blendTarget = 0.f;

		if( cb.mHit.fHit( ) )
			blendTarget = totalLen - totalLen * cb.mHit.mT;

		mRaycastTerrainPen = Math::fLerp( mRaycastTerrainPen, blendTarget, 0.2f );
		tripod.mEye = tripod.mLookAt + dir * (totalLen - mRaycastTerrainPen);
	}

}

