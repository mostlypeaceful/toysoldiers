#ifndef __tProjectileLogic__
#define __tProjectileLogic__

#include "tWeapon.hpp"
#include "FX/tTracerTrailEntity.hpp"
#include "tFxFileRefEntity.hpp"
#include "tShellCamera.hpp"

namespace Sig
{

	class tProjectileLogic : public tLogic
	{
		define_dynamic_cast( tProjectileLogic, tLogic );
	public:
		tProjectileLogic( );
		virtual ~tProjectileLogic( ) { }

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fActST( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fCoRenderMT( f32 dt );
		virtual void fComputeNewPosition( f32 dt ) { sigassert( !"Implement fComputeNewPosition." ); }
		virtual void fHitSomething( const tEntityPtr& ent );
		virtual f32 fUpdateShellCam( tPlayer& player, f32 dt, u32 inputFilter ) { return 0.f; } //returns the "lean" factor
		virtual Math::tVec4f fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const;

		void fProjectileDelete( );

		// adds a tracer to the projectile
		void fSpawnTracer( const FX::tTracerTrailDef& def );

		static void fExportScriptInterface( tScriptVm& vm );
	public:
		void fSetIgnoreParent( tEntity* e ) { mIgnoreParent.fReset( e ); }
		void fSetFiredBy( const tDamageID& id );
		virtual void fSetTarget( tEntityPtr& target, const Math::tVec3f& targetPt ) { }
		const tDamageID& fID( ) const { return mDamageID; }

		f32 fSpeed( ) const { return mSpeed; }
		void fSetSpeed( f32 speed ) { mSpeed = speed; }

		const Math::tVec3f& fLaunchVector( ) const { return mLaunchVector; }
		void fSetLaunchVector( const Math::tVec3f& lv ) 
		{
			mLaunchVector = lv; 
			sync_event_v_c( mLaunchVector, tSync::cSCProjectile );
		}

		virtual Math::tVec3f& fCurrentVelocity( ) { return mLaunchVector; }

		f32 fTimeMultiplier( ) const { return mTimeMultiplier; }
		void fSetTimeMultiplier( f32 tm ) { mTimeMultiplier = tm; }
		void fStepTimeMultiplier( f32 throttle, f32 dt );

		// Called after shells physical parameters have been initialized
		virtual void fInitPhysics( ) { }

		void fSpawnTrailEffects( ); //needs a weapon desc

		b32 fDeleteMe( ) const { return mDeleteMe; }

		void fPreSpawnedSpawn( );
		b32 fHasBeenFired( ) const { return mDamageID.mDesc != NULL; }

		void fSetShellCam( const tShellCameraPtr& cam ) { mShellCamera = cam; }
		const tShellCameraPtr& fShellCam( ) const { return mShellCamera; }

		virtual void		fInherit( tProjectileLogic& from, f32 dt );

		// call in response to hitting osmething to not detonate
		void fIgnoreHit( );

		virtual void fBurst( f32 dt ) { }	//inheriting logics can use this to burst into multiple projectiles if wanted...

		f32 fDamageMod( ) const { return mDamageMod; }
		void fSetDamageMod( f32 damageMod ) { mDamageMod = damageMod; }

	protected:
		virtual void fRayCast( );
		void fCheckLevelBounds( );
		void fAddDummyTag( tEntity* ent );

		void fCheckReadyForRealDelete( );
		void fFindParticleSystems( );
		void fEnableParticles( );

		//require a weapon pointer to be set
		void fSpawnHitEffect( const Math::tMat3f& xform, const Math::tVec3f& surfaceNormal );
		void fDealImpactDamage( const tEntityPtr& ent );
		void fSpawnExplosionDamage( const Math::tMat3f& xform );

		void fEnableSound( b32 enable );
		Audio::tSourcePtr mSoundSource;

		// Entity that fired this projectile.  This preserves the ent from being deleted so that we can
		// access the tUnitLogic which we "inherit" properties like team and unit type from.
		tEntityPtr	mEntityHitWithLogic;
		tEntityPtr	mEntityHitReal;
		u32			mSurfaceTypeHit;
		Math::tVec3f mSurfaceNormal;
		
		tShellCameraPtr mShellCamera;

		tEntityPtr	mIgnoreParent;
		tDamageID	mDamageID;
		Math::tMat3f mNextPos; // Gets set in Corender and used to move the projectile in Move
		f32			 mSpeed;
		Math::tVec3f mLaunchVector;
		f32          mTimeMultiplier;
		f32			 mMinTimeMultiplier;
		f32			 mMaxTimeMultiplier;
		f32			 mUserTimeMultiplier;

		b8			mDeleteMe;
		b8			mOutsideLevel;
		b8			mWaitOneFrame;
		b8			mIsBullet;

		b8			mFirstTick;
		u8			mFirstTickOffset; //randomly scale the particles dt by 0-1 on the first frame
		b8			mOrientParticles;
		b8			pad2;

		f32			mDamageMod;

		tStringPtr mHitEffectOverride;

		tGrowableArray<FX::tTracerTrailEntityPtr> mTracers;
		FX::tFxSystemsArray mParticleSystems;

		void fSetOrientParticles( b32 orient ) { mOrientParticles = orient; }
		b32 fOrientParticles( ) const { return mOrientParticles; }
	};

}

#endif//__tProjectileLogic__
