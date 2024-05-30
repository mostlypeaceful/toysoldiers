#include "GameAppPch.hpp"
#include "tShellCamera.hpp"
#include "tGameApp.hpp"
#include "tProjectileLogic.hpp"
#include "tRtsCamera.hpp"
#include "tWeaponStation.hpp"
#include "tRtsCursorLogic.hpp"
#include "Wwise_IDs.h"
#include "tSceneGraphCollectTris.hpp"

using namespace Sig::Math;

namespace Sig
{
	devvar( f32, Gameplay_Weapon_ShellCam_DefaultFollowDistance, 18.0f );
	devvar( f32, Gameplay_Weapon_ShellCam_LeanBlend, 0.1f );
	devvar( f32, Gameplay_Weapon_ShellCam_LeanAngle, 20.f );
	devvar( f32, Gameplay_Weapon_ShellCam_DeathTimer, 1.1f );
	devvar( f32, Gameplay_Weapon_ShellCam_BurstCameraSetBack, 10.0f );
	devvar( f32, Gameplay_Weapon_ShellCam_BurstCameraSetBackLerp, 0.1f );
	devvar( f32, Gameplay_Weapon_ShellCam_MinDistFromTerrain, 18.0f );
	devvar( f32, Gameplay_Weapon_ShellCam_DetonateTimer, 0.5f );
	devvar( f32, Gameplay_Weapon_ShellCam_DeathExposureScale, 2.5f );
	devvar( f32, Gameplay_Weapon_ShellCam_MinDistBeforeCorrection, 18.0f );


	namespace
	{
		const tStringPtr cShellCamName( "shellCam" );

		struct tShellCamProjectileRayCastCallback
		{
			mutable Math::tRayCastHit		mHit;
			mutable tEntity*		mHitEntity;
			tEntity*				mOwnerEnt;
			tEntity*				mIgnore;

			mutable tGrowableArray<tEntity*> mProxyEntsSearched;

			explicit tShellCamProjectileRayCastCallback( tEntity* ownerEnt, tEntity* ignore = 0 ) 
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
				if( spatial->fHasGameTagsAny( GameFlags::cFLAG_DUMMY | GameFlags::cFLAG_DONT_STOP_BULLETS ) )
					return;
				if( mOwnerEnt && (mOwnerEnt == spatial || spatial->fIsAncestorOfMine( *mOwnerEnt )) )
					return;
				if( mIgnore && (mIgnore == spatial || spatial->fIsAncestorOfMine( *mIgnore )) )
					return;

				tEntity* logicEnt = spatial->fFirstAncestorWithLogic( );

				if( logicEnt && logicEnt->fQueryEnumValue( GameFlags::cENUM_PICKUPS ) != ~0 )
					return;

				if( logicEnt && logicEnt->fHasGameTagsAll( tEntityTagMask( GameFlags::cFLAG_PROXY_COLLISION_ROOT ) ) )
				{
					//this object contains proxy geometry
					fTestProxyShapes( ray, logicEnt );
				}
				else
					fRayCastAgainstSpatial( ray, spatial );
			}

			inline void fTestProxyShapes( const Math::tRayf& ray, tEntity* root ) const
			{
				if( !mProxyEntsSearched.fFind( root ) )
				{
					mProxyEntsSearched.fPushBack( root );

					for( u32 i = 0; i < root->fChildCount( ); ++i )
					{
						const tEntityPtr& e = root->fChild( i );
						if( e->fHasGameTagsAll( tEntityTagMask( GameFlags::cFLAG_PROXY_COLLISION_ROOT ) ) )
							fTestProxyShapes( ray, root );

						if( e->fHasGameTagsAll( tEntityTagMask( GameFlags::cFLAG_PROXY_COLLISION_SHAPE ) ) )
							fRayCastAgainstSpatial( ray, e->fStaticCast<tSpatialEntity>( ) );
					}
				}
			}
		};
	}

	tShellCamera::tShellCamera( tPlayer& player, const tGrowableArray<tProjectileLogic*>& projLogics, tWeapon* weapon )
		: tUseUnitCamera( player, NULL, false )
		, mPlayer( player )
		, mProjLogics( projLogics )
		, mFollowProj( NULL )
		, mDeathTimer( Gameplay_Weapon_ShellCam_DeathTimer )
		, mBursted( false )
		, mPushed( true )
		, mInitialized( false )
		, mDepthLimited( false )
		, mOverlayShown( false )
		, mDoRayCastCorrect( false )
		, mBurstBlend( 0.f )
		, mDetonateTimer( 0.f )
		, mOriginalPos( tVec3f::cZeroVector )
	{
		if( weapon )
		{
			mBlendLerpStart = weapon->fDesc( ).mShellCamBlendInDepart;
			mBlendLerpEnd = weapon->fDesc( ).mShellCamBlendInArrive;
		}
		else
		{
			mBlendLerpStart = 0.01f;
			mBlendLerpEnd = 0.25f;
		}

		sigassert( mProjLogics.fCount( ) );

		mFollowProj = mProjLogics.fBack( );
		mWeapon = mFollowProj->fID( ).mWeapon;
		tEntity* owner = mFollowProj->fOwnerEntity( );
		sigassert( owner );

		tEntity* shellCamPoint = owner->fFirstDescendentWithName( cShellCamName );
		if( shellCamPoint )
			mShellCamOffset = shellCamPoint->fParentRelative( );
		else
		{
			mShellCamOffset = tMat3f::cIdentity;
			mShellCamOffset.fSetTranslation( tVec3f( 0, 0, -Gameplay_Weapon_ShellCam_DefaultFollowDistance ) );
		}

		for( u32 i = 0; i < mProjLogics.fCount( ); ++i )
			mProjLogics[ i ]->fSetShellCam( tShellCameraPtr( this ) );

		// while we're in shell cam, we dont want the rts cam to acquire the current position if something happens to the turret.
		tRtsCamera* rtsCamera = mPlayer.fCameraStack( ).fFindCameraOfType<tRtsCamera>( );
		sigassert( rtsCamera );
		rtsCamera->fAcquireCurrentCameraPos( );
		rtsCamera->fSetPreventPositionAcquisition( true );

		// Show alternate controls
		if( mWeapon && mWeapon->fStation( ).fUI( ) )
		{
			mWeapon->fStation( ).fUI( )->fShowShellCam( true, owner, &mPlayer );
			mOverlayShown = true;
		}

		// Show the alternate money/combo notification
		if( mPlayer.fScreenSpaceNotification( ) )
			mPlayer.fScreenSpaceNotification( )->fEnable( true );
	}

	tShellCamera::~tShellCamera( )
	{
		for( u32 i = 0; i < mProjLogics.fCount( ); ++i )
			mProjLogics[ i ]->fSetTimeMultiplier( 1.0f );
	}

	void tShellCamera::fProjectileDied( tProjectileLogic* proj )
	{
		mProjLogics.fFindAndErase( proj );
		if( proj == mFollowProj )
			mFollowProj = NULL;
		else
			mDeathSpots.fPushBack( proj->fOwnerEntity( )->fObjectToWorld( ) );

		if( mProjLogics.fCount( ) == 0 )
		{
			fHideOverlay( true );
		}
	}

	void tShellCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
		fUserTick( dt, tripod );
	}

	void tShellCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
		Gfx::tTripod newTripod;

		if( mProjLogics.fCount( ) )
		{
			f32 lean = 0.f;
			tVec3f center = tVec3f::cZeroVector;
			tVec3f z = tVec3f::cZeroVector;
			u32 count = 0;

			for( u32 i = 0; i < mProjLogics.fCount( ); ++i )
			{
				lean = mProjLogics[ i ]->fUpdateShellCam( mPlayer, dt );
				if( mProjLogics[ i ]->fSceneGraph( ) )
				{
					center += mProjLogics[ i ]->fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
					z += mProjLogics[ i ]->fOwnerEntity( )->fObjectToWorld( ).fZAxis( );
					++count;
				}
			}

			for( u32 i = 0; i < mDeathSpots.fCount( ); ++i )
			{
				center += mDeathSpots[ i ].fGetTranslation( );
				z += mDeathSpots[ i ].fZAxis( );
				++count;
			}

			if( count == 0 )
			{
				newTripod = mLastTripod;
			}
			else
			{
				sigassert( count );
				center /= (f32)count;
				z.fNormalizeSafe( tVec3f::cZAxis );

				mLeanBlend.fSetBlends( Gameplay_Weapon_ShellCam_LeanBlend );
				mLeanBlend.fStep( lean, dt );
				f32 leanAngle = mLeanBlend.fValue( ) * -fToRadians( (f32)Gameplay_Weapon_ShellCam_LeanAngle );

				// actual camera stuff
				tMat3f afterBurstOffset = tMat3f::cIdentity;
				if( mBursted )
					mBurstBlend = fLerp( mBurstBlend, 1.f, (f32)Gameplay_Weapon_ShellCam_BurstCameraSetBackLerp );

				afterBurstOffset.fTranslateLocal( tVec3f( 0, 0, -mBurstBlend * (f32)Gameplay_Weapon_ShellCam_BurstCameraSetBack ) );

				tMat3f xform;
				xform.fOrientZAxis( z, tVec3f::cYAxis );
				xform.fSetTranslation( center );
				xform *= mShellCamOffset * afterBurstOffset;

				tVec3f zAxis = xform.fZAxis( );
				newTripod.mEye = xform.fGetTranslation( );
				newTripod.mLookAt = newTripod.mEye + zAxis;
				newTripod.mUp = tQuatf( tAxisAnglef( zAxis, leanAngle ) ).fRotate( tVec3f::cYAxis );

				mPlayer.fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_IN_SHELLCAM, dt );
			}

			mLastTripod = newTripod;

			if( mDepthLimited && mDetonateTimer > Gameplay_Weapon_ShellCam_DetonateTimer )
			{
				//kill them all
				for( s32 i = mProjLogics.fCount( ) - 1; i >= 0 ; --i )
					mProjLogics[ i ]->fHitSomething( tEntityPtr( ) );
			}
		}
		else
		{
			newTripod = mLastTripod;
			mDeathTimer -= dt;

			f32 lived = (Gameplay_Weapon_ShellCam_DeathTimer - mDeathTimer)/Gameplay_Weapon_ShellCam_DeathTimer;
			lived = fClamp( lived, 0.f, 1.f );
			fSetExposureMult( 1.f + lived * Gameplay_Weapon_ShellCam_DeathExposureScale );
		}

		if( !mInitialized )
		{
			mInitialized = true;
			mRealTripod = newTripod;
			mOriginalPos = newTripod.mEye;
		}

		f32 overAllBlend = 0.2f;
		if( mWeapon )
			overAllBlend = mWeapon->fDesc( ).mShellCamOverallBlend;

		fShellCamRayCastCorrectNewTripod( newTripod, dt );
		fBlendTripods( overAllBlend, newTripod, mRealTripod );
		tripod = mRealTripod;
	}

	// Same as the base but looks forward and does other projectily things
	void tShellCamera::fShellCamRayCastCorrectNewTripod( Gfx::tTripod& tripod, f32 dt )
	{
		if( !mDoRayCastCorrect )
		{
			const f32 cMinDeltaSqr = Gameplay_Weapon_ShellCam_MinDistBeforeCorrection * Gameplay_Weapon_ShellCam_MinDistBeforeCorrection;
			tVec3f delta = tripod.mEye - mOriginalPos;
			if( delta.fLengthSquared( ) > cMinDeltaSqr )
				mDoRayCastCorrect = true;

			return;
		}

		const f32 cBackLen = 0.25f; //extra ray length from behind camera

		tVec3f dir = tripod.mLookAt - tripod.mEye;
		f32 len;
		dir.fNormalizeSafe( tVec3f::cZAxis, len );

		if( !mDepthLimited )
		{			
			tRayf ray( tripod.mEye - dir * cBackLen, dir );
			ray.mExtent *= cBackLen + Gameplay_Weapon_ShellCam_MinDistFromTerrain;

			tShellCamProjectileRayCastCallback cb( mFollowProj ? mFollowProj->fID( ).mUnit.fGetRawPtr( ) : NULL, NULL );
			tGameApp::fInstance( ).fSceneGraph( )->fRayCastAgainstRenderable( ray, cb );

			// dont get stopped by pickups
			if( cb.mHit.fHit( ) )
			{
				mDepthLimited = true;
				mDepthLimitPos = ray.fEvaluate( cb.mHit.mT );			
			}
		}

		if( mDepthLimited )
		{
			tripod.mEye = mDepthLimitPos - dir * (Gameplay_Weapon_ShellCam_MinDistFromTerrain - cBackLen);
			tripod.mLookAt = tripod.mEye + dir * len;
			mDetonateTimer += dt;
		}
	}

	void tShellCamera::fBurst( ) 
	{ 
		mBursted = true; 
	}

	b32 tShellCamera::fExit( ) const
	{
		return (mWeapon && !mWeapon->fShouldShellCam( ));
	}
	
	void tShellCamera::fOnRemove( ) 
	{ 
		{
			for( u32 i = 0; i < mProjLogics.fCount( ); ++i )
			{
				mProjLogics[ i ]->fSetShellCam( tShellCameraPtr( ) );
				mProjLogics[ i ]->fSetTimeMultiplier( 1.0f );
			}
		
			fHideOverlay( false );

			// Hide the alternate money/combo notification
			if( mPlayer.fScreenSpaceNotification( ) )
				mPlayer.fScreenSpaceNotification( )->fEnable( false );

			if( mWeapon )
				mWeapon->fShellCamDied( );

			//blend back in to the other
			u32 myIndex = mPlayer.fCameraStack( ).fIndexOfType<tShellCamera>( );
			sigassert( myIndex != ~0 );

			if( myIndex != ~0 && myIndex > 0 )
			{
				tRtsCamera* belowMe = mPlayer.fCameraStack( )[ myIndex - 1 ].fDynamicCast<tRtsCamera>( );
				if( !belowMe )
				{
					// rts camera is not below me, so the turret must still be alive, and it's ok to disable position acquisistion disabling
					tRtsCamera* rts = mPlayer.fCameraStack( ).fFindCameraOfType<tRtsCamera>( );
					sigassert( rts );
					rts->fSetPreventPositionAcquisition( false );
				}
			}
		}

		tUseUnitCamera::fOnRemove( );
	}

	void tShellCamera::fHideOverlay( b32 keepEffect )
	{
		if( mOverlayShown )
		{
			// Hide the alternate controls
			if( mWeapon && mWeapon->fValid( ) && mWeapon->fStation( ).fUI( ) )
			{
				mWeapon->fStation( ).fUI( )->fShowShellCam( false, NULL, &mPlayer );
				if( keepEffect )
					mWeapon->fStation( ).fUI( )->fActivateScreenEffect( Gui::tWeaponUI::cScreenEffectShellCam );
				else
				{
					fSetExposureMult( 1.f );
					mOverlayShown = false;
				}
			}
		}
	}

	void tShellCamera::fSetExposureMult( f32 mult )
	{
		tGameApp::fInstance( ).fPostEffectsManager( )->fSetFilmGrainExposureMult( mPlayer.fUser( )->fViewportIndex( ), mult );
	}

	b32 tShellCamera::fCleanup( )
	{
		if( mDeathTimer <= 0.f || fExit( ) )
		{
			tShellCameraPtr me( this );
			mPlayer.fCameraStack( ).fPopCamerasOfType<tShellCamera>( );
			mPushed = false;

			return true;
		}

		return false;
	}

}

