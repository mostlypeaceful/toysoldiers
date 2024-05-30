#include "GameAppPch.hpp"
#include "tProjectileLogic.hpp"
#include "tGameApp.hpp"
#include "FX/tParticleSystem.hpp"
#include "tExplosionLogic.hpp"
#include "tLevelLogic.hpp"
#include "tGameEffects.hpp"
#include "tUberBreakableLogic.hpp"
#include "tSceneGraphCollectTris.hpp"

#include "Wwise_IDs.h"

using namespace Sig::Math;

namespace Sig
{

	namespace
	{
		static const tStringPtr cSpecialCamTint = tStringPtr( "SpecialCamTint" );

		struct tProjectileRayCastCallback
		{
			mutable Math::tRayCastHit		mHit;
			mutable tEntity*		mHitEntity;
			tEntity*				mIgnore;

			mutable tGrowableArray<tEntity*> mProxyEntsSearched;

			explicit tProjectileRayCastCallback( tEntity* ignore ) 
				: mHitEntity( 0 ), mIgnore( ignore )
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

				if( mIgnore && (mIgnore == spatial || spatial->fIsAncestorOfMine( *mIgnore )) )
					return;

				tEntity* logicEnt = spatial->fFirstAncestorWithLogic( );
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

	devvar_clamp( f32, Gameplay_Weapon_ShellCam_ExtraMinSpeed, 0.0f, -2.f, 2.f, 2 );
	devvar_clamp( f32, Gameplay_Weapon_ShellCam_ExtraMaxSpeed, 0.0f, -2.f, 2.f, 2 );
	devvar( f32, Gameplay_Weapon_ShellCam_SpeedChange, 1.2f );
	devvar( f32, Gameplay_Weapon_ShellCam_SpeedChangeIdle, 1.2f );
	devvar( bool, Gameplay_Weapon_RayTraceBehind, false );
	devvar( bool, Perf_EnableTracers, true );
	devvar( bool, Perf_Audio_DisableProjectileAway, false );
	devvar( f32, Gameplay_Weapon_MaxUberBreakableVel, 100.f );

	tProjectileLogic::tProjectileLogic( ) 
		: mNextPos( tMat3f::cIdentity )
		, mSpeed( 0 )
		, mTimeMultiplier( 1.0f )
		, mMinTimeMultiplier( 0.1f )
		, mMaxTimeMultiplier( 1.2f )
		, mUserTimeMultiplier( 1.f )
		, mLaunchVector( tVec3f::cZeroVector )
		, mDeleteMe( false )
		, mOutsideLevel( false )
		, mWaitOneFrame( false )
		, mIsBullet( false )
		, mFirstTick( true )
		, mFirstTickOffset( 255 )
		, mOrientParticles( false )
		, mSurfaceTypeHit( ~0 )
		, mDamageMod( 1.f )
		, mSurfaceNormal( Math::tVec3f::cYAxis )
	{
	}

	void tProjectileLogic::fOnSpawn( )
	{
		fOnPause( false );

		mNextPos = fOwnerEntity( )->fObjectToWorld( );
		sigassert( !mNextPos.fIsNan( ) );

		fOwnerEntity( )->fAddGameTagsRecursive( GameFlags::cFLAG_DUMMY );
		fFindParticleSystems( );

		fOwnerEntity( )->fAddGameTags( GameFlags::cFLAG_DONT_INHERIT_STATE_CHANGE );
	}
	void tProjectileLogic::fPreSpawnedSpawn( )
	{
		fEnableParticles( );
		fCoRenderMT( 0.f );
		fOnPause( false );
	}
	void tProjectileLogic::fSpawnTracer( const FX::tTracerTrailDef& def )
	{
		if( Perf_EnableTracers )
		{
			FX::tTracerTrailEntity* trail = NEW FX::tTracerTrailEntity( *fOwnerEntity( ), def );

			trail->fSpawn( *fOwnerEntity( ) );

			mTracers.fPushBack( FX::tTracerTrailEntityPtr( trail ) );
		}
	}
	void tProjectileLogic::fFindParticleSystems( )
	{
		fOwnerEntity( )->fForEachDescendent( FX::tAddPausedFxSystem( mParticleSystems, GameFlags::cFLAG_DONT_INHERIT_STATE_CHANGE ) );

		//if we've already been launched before we spawn, enable particles
		if( mDamageID.mDesc ) 
			fEnableParticles( );
	}
	void tProjectileLogic::fEnableParticles( )
	{
		mParticleSystems.fPause( false );
	}
	void tProjectileLogic::fOnDelete( )
	{
		if( mDamageID.mWeapon ) mDamageID.mWeapon->fProjectileDied( this );
		if( mShellCamera ) mShellCamera->fProjectileDied( this );
		mShellCamera.fRelease( );
		mEntityHitWithLogic.fRelease( );
		mEntityHitReal.fRelease( );
		mTracers.fSetCount( 0 );
		mParticleSystems.fSafeCleanup( );
		mSoundSource.fRelease( );
		mDamageID = tDamageID( );
		mIgnoreParent.fRelease( );
		tLogic::fOnDelete( );
	}
	void tProjectileLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListMoveST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else if( fHasBeenFired( ) )
		{
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListMoveST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}
	void tProjectileLogic::fActST( f32 dt )
	{
		profile( cProfilePerfProjectileLogicActST );

		if( mOutsideLevel )
			fProjectileDelete( );

		

		if( mEntityHitWithLogic && !mDeleteMe )
		{
			sync_event_v_c( mEntityHitWithLogic->fGuid( ), tSync::cSCLogic | tSync::cSCParticles | tSync::cSCProjectile );
			sync_event_v_c( mWaitOneFrame, tSync::cSCLogic | tSync::cSCParticles | tSync::cSCProjectile );

			// Delay a frame before registering the hit and dealing damage,
			// so that we can render that the projectile has collided with the entity for one frame
			if( !mWaitOneFrame )
				mWaitOneFrame = true;
			else
			{
				fHitSomething( mEntityHitWithLogic );
			}
		}

		if( mDeleteMe )
			fOwnerEntity( )->fDelete( );
	}
	void tProjectileLogic::fIgnoreHit( )
	{
		mEntityHitWithLogic.fRelease( );
		mEntityHitReal.fRelease( );
		mSurfaceTypeHit = ~0;
		mWaitOneFrame = false;
	}
	void tProjectileLogic::fHitSomething( const tEntityPtr& ent )
	{
		//Math::tMat3f xform;
		
		//if( mOrientParticles )
		//{
		//	tVec3f zDir = tVec3f::cYAxis;
		//	tVec3f yDir = fCurrentVelocity( );
		//	yDir.fNormalizeSafe( tVec3f::cYAxis );

		//	if( yDir.fDot( tVec3f::cYAxis ) > 0.9f )
		//		zDir = tVec3f::cZAxis;

		//	xform.fOrientYWithZAxis( yDir, zDir );
		//}
		//else
		//	xform = tMat3f::cIdentity;

		//xform.fSetTranslation( fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) );
		//sigassert( !xform.fIsNan( ) );


		//xform.fSetTranslation( fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) );
		//sigassert( !xform.fIsNan( ) );

		sync_line_c( tSync::cSCLogic | tSync::cSCParticles | tSync::cSCProjectile );
		tMat3f orient = tMat3f::cIdentity;
		orient.fSetTranslation( fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) );
		sigassert( !orient.fIsNan( ) );
		
		if( ent ) fDealImpactDamage( ent );
		fSpawnHitEffect( orient, mSurfaceNormal );
		fSpawnExplosionDamage( orient );

		fProjectileDelete( );
	}

	void tProjectileLogic::fProjectileDelete( )
	{
		fEnableSound( false );

		mDeleteMe = true;

		if( mDamageID.mWeapon ) 
		{
			mDamageID.mWeapon->fProjectileDied( this );
			mDamageID.mWeapon.fRelease( );
			mDamageID.mDesc = NULL;
		}

		if( mShellCamera ) 
			mShellCamera->fProjectileDied( this );
		mShellCamera.fRelease( );

		for( u32 i = 0; i < mTracers.fCount( ); ++i ) 
			mTracers[ i ]->fStopTrackingParent( );
	}
	void tProjectileLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfProjectileLogicMoveST );

		if( mSoundSource ) mSoundSource->fSetGameParam( AK::GAME_PARAMETERS::SPEED, mTimeMultiplier );

		fOwnerEntity( )->fMoveTo( mNextPos );
	}
	void tProjectileLogic::fCoRenderMT( f32 dt )
	{
		profile( cProfilePerfProjectileLogicCoRenderMT );

		if( !mDeleteMe )
		{
			if( mFirstTick )
			{
				dt *= mFirstTickOffset / 255.f;
				mFirstTick = false;
			}

			fComputeNewPosition( dt );

			if( !mOutsideLevel )
				fRayCast( );
		}
 	}
	void tProjectileLogic::fCheckLevelBounds( )
	{
		if( !tGameApp::fInstance( ).fCurrentLevel( )->fLevelBounds( ).fContainsXZ( mNextPos.fGetTranslation( ) ) )
			mOutsideLevel = true;
	}
	void tProjectileLogic::fRayCast( )
	{
		if( !mEntityHitReal )
		{
			// This happens in MT threads, dont release any pointers. so if it's already set, too bad so sad, it's staying set.

			const tVec3f currPos = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
			const tVec3f delta = mNextPos.fGetTranslation( ) - currPos;
			tRayf ray;
			
			if( Gameplay_Weapon_RayTraceBehind )
				ray = tRayf( currPos - delta, delta );
			else
				ray = tRayf( currPos, delta );

			tProjectileRayCastCallback rayCastcb( mIgnoreParent.fGetRawPtr( ) );

			fOwnerEntity( )->fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastcb );

			if( rayCastcb.mHit.fHit( ) )
			{
				sync_event_v_c( rayCastcb.mHit.mT, tSync::cSCProjectile );
				mSurfaceTypeHit = rayCastcb.mHitEntity->fQueryEnumValue( GameFlags::cENUM_SURFACE_TYPE, ~0 );
				mEntityHitReal.fReset( rayCastcb.mHitEntity );
				mEntityHitWithLogic.fReset( rayCastcb.mHitEntity->fFirstAncestorWithLogic( ) );
				mNextPos.fSetTranslation( ray.fEvaluate( rayCastcb.mHit.mT ) );
				mSurfaceNormal = rayCastcb.mHit.mN;
				sigassert( mSurfaceNormal == mSurfaceNormal );
				mSurfaceNormal.fNormalizeSafe( tVec3f::cYAxis );
			}
		}
	}
	void tProjectileLogic::fAddDummyTag( tEntity* ent )
	{
		sigassert( ent );

		ent->fAddGameTags( GameFlags::cFLAG_DUMMY );
		for( u32 i = 0; i < ent->fChildCount( ); ++i )
			fAddDummyTag( ent->fChild( i ).fGetRawPtr( ) );
	}
	void tProjectileLogic::fSetFiredBy( const tDamageID& id ) 
	{ 
		mDamageID = id; 
		fEnableSound( true ); 
	}
	void tProjectileLogic::fSpawnTrailEffects( )
	{
		sigassert( mDamageID.mDesc );
		b32 overCharged = mDamageID.mOverCharged;
		u32 tracerCnt = tGameApp::fInstance( ).fTracersTable( ).fTable( ).fRowCount( );

		u32 bulletTracer = overCharged ? mDamageID.mDesc->mBulletTracerTypeOverCharged : mDamageID.mDesc->mBulletTracerType;
		if( bulletTracer < tracerCnt )
		{
			const FX::tTracerTrailDef &def = tGameApp::fInstance( ).fTracerTrailDef( bulletTracer );
			fSpawnTracer( def );
		}

		if( mDamageID.mWeapon )
		{
			u32 trailTracer = overCharged ? mDamageID.mDesc->mTracerTrailTypeOverCharged : mDamageID.mDesc->mTracerTrailType;
			if( trailTracer < tracerCnt )
			{
				if( mDamageID.mWeapon->fRequestTrail( ) )
				{
					const FX::tTracerTrailDef &def = tGameApp::fInstance( ).fTracerTrailDef( trailTracer );
					fSpawnTracer( def );
				}
			}
		}
	}
	void tProjectileLogic::fSpawnHitEffect( const tMat3f& xform, const tVec3f& surfaceNormal )
	{		
		sync_event_v_c( xform, tSync::cSCLogic | tSync::cSCParticles | tSync::cSCProjectile );

		const tStringPtr* effect = NULL;
		if( mHitEffectOverride.fExists( ) )
			effect = &mHitEffectOverride;
		else if( mDamageID.mDesc )
			effect = (mDamageID.mOverCharged) ? &mDamageID.mDesc->mHitEffectOverCharged : &mDamageID.mDesc->mHitEffect;
		else
			log_warning( 0, "Tried to fSpawnHitEffect but had no weapon!" );
		
		if( effect )
		{
			tEffectArgs args;
			args.mTransformOverride = &xform;
			args.mInsertParent = (mDamageID.mDesc && mDamageID.mDesc->mWeaponType == tGameApp::cWeaponDerivedTypeMortar) ? tAreaEffectParent::fCreate( mDamageID.mWeapon ) : NULL;
			args.mSurfaceType = mSurfaceTypeHit;

			tVec3f zDir = fOwnerEntity( )->fObjectToWorld( ).fZAxis( );
			tVec3f xDir = fOwnerEntity( )->fObjectToWorld( ).fXAxis( );
			args.mSurfaceNormal = &surfaceNormal;
			args.mInputNormal = &zDir;
			args.mXDir = &xDir;
		
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevelDemand( );
			tGameEffects::fInstance( ).fPlayEffect( level->fOwnerEntity( ), *effect, args );
		}
	}
	void tProjectileLogic::fSpawnExplosionDamage( const tMat3f& xform )
	{
		if( mDamageID.mDesc )
		{			
			if( !fEqual( mDamageID.mDesc->mExplosionFullSize, 0.f ) )
			{
				tEntity *ent = NEW tEntity( );
				ent->fMoveTo( xform );
				ent->fSpawn( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );

				tExplosionLogic *explosionLogic = NEW tExplosionLogic( );
				explosionLogic->fSetFiredBy( mDamageID );
				ent->fAcquireLogic( NEW tLogicPtr( explosionLogic ) );

				if( mIsBullet && mDamageID.mWeapon ) 
					explosionLogic->fReuseLight( mDamageID.mWeapon->fGetBulletExplosionLight( ) );

				f32 fullSize = mDamageID.mDesc->mExplosionFullSize;
				explosionLogic->fSetFullSize( fullSize );
				explosionLogic->fSetGrowRate( mDamageID.mDesc->mExplosionGrowthRate );
				explosionLogic->fSetFalloff( mDamageID.mDesc->mExplosionFallOff );
				explosionLogic->fSetDamageMod( mDamageMod );

				if( mDamageID.mDesc->mImpactLightSize > 0.f )
				{
					explosionLogic->fSetLightValues( mDamageID.mDesc->mImpactLightSize, mDamageID.mDesc->mImpactLightLife, mDamageID.mDesc->mImpactLightColor );

					//spawn the light before the explosion is actually spawned so they both show up together
					explosionLogic->fSpawnLight( xform );
				}
			}
		}
		else
			log_warning( 0, "Tried to fSpawnExplosionDamage but had no weapon!" );
	}

	
	void tProjectileLogic::fDealImpactDamage( const tEntityPtr& ent )
	{
		tUnitLogic* unitLogic = ent->fLogicDerived< tUnitLogic > ( );
		if( unitLogic )
		{
			sigassert( mDamageID.mDesc );
			if( !fEqual( mDamageID.mDesc->mDamageModDirectHit, 0.f ) )
			{
				tDamageContext damageContext;
				damageContext.fSetDamageMultiplier( mDamageMod );

				damageContext.fSetAttacker( mDamageID, GameFlags::cDAMAGE_TYPE_BULLET );

				damageContext.mWorldPosition = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
				sigassert( !damageContext.mWorldPosition.fIsNan( ) );

				unitLogic->fDealDamage( damageContext );
			}
		}
#ifdef __tUberBreakableLogic__
		else
		{
			tUberBreakablePiece* uberBreakPiece = mEntityHitReal->fDynamicCast< tUberBreakablePiece >( );
			if( uberBreakPiece )
			{
				tUberBreakableLogic* parent = uberBreakPiece->fFirstAncestorWithLogicOfType<tUberBreakableLogic>( );
				if( parent )
				{
					if( !parent->fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_BULLET_PROOF ) )
					{
						tVec3f v = fCurrentVelocity( );
						v.fClampLength( Gameplay_Weapon_MaxUberBreakableVel );
						uberBreakPiece->fOnHit( v );
					}
				}
			}
		}
#endif//__tUberBreakableLogic__
	}
	void tProjectileLogic::fEnableSound( b32 enable )
	{
		if( enable && !mSoundSource )
		{
			mSoundSource.fReset( NEW Audio::tSource( "Projectile" ) );
			mSoundSource->fSpawn( *fOwnerEntity( ) );
		}

		if( mSoundSource ) 
		{
			if( !Perf_Audio_DisableProjectileAway || !enable )
			{
				if( enable && mDamageID.mDesc )
					mSoundSource->fSetSwitch( tGameApp::cWeaponTypeSwitchGroup, mDamageID.mDesc->mAudioAlias );
				if( !enable && mSurfaceTypeHit != ~0 )
					mSoundSource->fSetSwitch( tGameApp::cSurfaceTypeSwitchGroup, GameFlags::fSURFACE_TYPEEnumToValueString( mSurfaceTypeHit ) );
				mSoundSource->fHandleEvent( enable ? AK::EVENTS::PLAY_PROJECTILE : AK::EVENTS::STOP_PROJECTILE );
			}
		}
	}
	void tProjectileLogic::fStepTimeMultiplier( f32 throttle, f32 dt )
	{
		f32 currentMultiplier = fTimeMultiplier( );

		f32 target = mUserTimeMultiplier; // return to normal speed if no stick input
		f32 rate = Gameplay_Weapon_ShellCam_SpeedChangeIdle;

		if( throttle < 0.0f )
			target = fLerp( 1.0f, (mMinTimeMultiplier + Gameplay_Weapon_ShellCam_ExtraMinSpeed), -throttle );
		else if( throttle > 0.0f )
			target = fLerp( 1.0f, (mMaxTimeMultiplier + Gameplay_Weapon_ShellCam_ExtraMaxSpeed), throttle );
		else 
			rate = Gameplay_Weapon_ShellCam_SpeedChange;

		f32 delta = target - currentMultiplier;
		f32 effectivedelta = fSign( delta ) * dt * Gameplay_Weapon_ShellCam_SpeedChange;

		if( fAbs( effectivedelta ) > fAbs( delta ) ) effectivedelta = delta;
		currentMultiplier += effectivedelta;

		fSetTimeMultiplier( currentMultiplier );
	}

	void tProjectileLogic::fInherit( tProjectileLogic& from, f32 dt )
	{
		mDamageID = from.mDamageID;
		mIgnoreParent = from.mIgnoreParent;
		mSpeed = from.mSpeed;
		mLaunchVector = from.mLaunchVector;
		mMaxTimeMultiplier = from.mMaxTimeMultiplier;
		mMinTimeMultiplier = from.mMinTimeMultiplier;
		mTimeMultiplier = from.mTimeMultiplier;
		mIsBullet = from.mIsBullet;
		mDamageMod = from.mDamageMod;

		if( from.mSoundSource )
		{
			sigassert( fOwnerEntity( ) );
			// steal the sound for the first guy
			mSoundSource = from.mSoundSource;
			mSoundSource->fReparent( *fOwnerEntity( ) );
			mSoundSource->fSetParentRelativeXform( tMat3f::cIdentity );
			from.mSoundSource.fRelease( );
		}
	}

	Math::tVec4f tProjectileLogic::fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const
	{
		if( varName == cSpecialCamTint )
		{
			return tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamUnitTint( viewportIndex );
		}

		return Math::tVec4f::cZeroVector;
	}


	void tProjectileLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tProjectileLogic, tLogic, Sqrat::NoCopy<tProjectileLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("MinTimeMultiplier"), &tProjectileLogic::mMinTimeMultiplier)
			.Var(_SC("MaxTimeMultiplier"), &tProjectileLogic::mMaxTimeMultiplier)
			.Var(_SC("UserTimeMultiplier"), &tProjectileLogic::mUserTimeMultiplier)
			.Var(_SC("HitEffectOverride"), &tProjectileLogic::mHitEffectOverride)
			.Prop(_SC("OrientParticles"), &tProjectileLogic::fOrientParticles, &tProjectileLogic::fSetOrientParticles)
			
			;

		vm.fRootTable( ).Bind(_SC("ProjectileLogic"), classDesc);
	}
}
