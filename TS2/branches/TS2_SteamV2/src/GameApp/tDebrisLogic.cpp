#include "GameAppPch.hpp"
#include "tGameApp.hpp"
#include "tDebrisLogic.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tGamePostEffectMgr.hpp"

#include "tLogic.hpp"
#include "tEntity.hpp"
#include "tRandom.hpp"
#include "tSync.hpp"


using namespace Sig::Math;

namespace Sig
{	
	//devvar( f32, Gameplay_Debris_UpVelMax, 12.0f );
	//devvar( f32, Gameplay_Debris_UpVelMin, 5.0f );
	//devvar( f32, Gameplay_Debris_HorzVelMax, 8.0f );
	//devvar( f32, Gameplay_Debris_TorqueArm, 0.2f );
	//devvar( f32, Gameplay_Debris_BounceRestitution, 0.5f );
	//devvar( f32, Gameplay_Debris_BounceFriction, 0.5f );
	//devvar( u32, Gameplay_Debris_BounceCountMax, 3 );
	//devvar( f32, Gameplay_Debris_LifeAfterFallThrough, 1.0f );
	//devvar( f32, Gameplay_Debris_LifeMax, 6.0f );


	namespace
	{
		static const tStringPtr cSpecialCamTint = tStringPtr( "SpecialCamTint" );
	}

	u32 tDebrisLogic::sDebrisCount = 0;

	devvar( u32, Perf_Debris_MaxDebris, 120 );	
	devvar( bool, Gameplay_Debris_SlowMo, false );
	devvar( bool, Debug_Debris_Render, false );
	

	namespace
	{
		struct tDebrisRayCastCallback
		{
			mutable Math::tRayCastHit		mHit;
			mutable tEntity*				mHitEntity;
			tEntity*						mOwnerEnt;
			tEntity*						mIgnore;

			explicit tDebrisRayCastCallback( tEntity* ownerEnt, tEntity* ignore = 0 ) 
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
				if( spatial->fHasGameTagsAny( GameFlags::cFLAG_DUMMY ) )
					return;
				if( mOwnerEnt && spatial->fIsAncestorOfMine( *mOwnerEnt ) )
					return;
				if( mIgnore && spatial->fIsAncestorOfMine( *mIgnore ) )
					return;

				fRayCastAgainstSpatial( ray, spatial );
			}
		};
	}

	tDebrisLogic::tDebrisLogic( const tDebrisLogicDef& def )
		: mBounds( tAabbf::cZeroSized )
		, mOffset( tMat3f::cIdentity )
		, mRandom( sync_rand( fUInt( ) ) ) 
		, mBounceMeter( 0 )
		, mBounceY( 0 )
		, mActive( true )
		, mFirstTick( true )
		, mDormantDueToPerf( false )
		, mDeathHeight( 0.0f )
		, mTotalTimer( 0.0f )
		, mRandomFactor( 0.f )
		, mCollisionDelay( def.mCollisionDelay )
		, mDef( def )
	{
	}

	void tDebrisLogic::fOnSpawn( )
	{
		fIncDebrisCount( );

		fOnPause( false );
		fOwnerEntity( )->fAddGameTagsRecursive( GameFlags::cFLAG_DUMMY );
		Gfx::tRenderableEntity::fSetRgbaTint( *fOwnerEntity( ), Math::tVec4f::cOnesVector );

		const tMat3f& trans = fOwnerEntity( )->fObjectToWorld( );
		mBounds = tMeshEntity::fCombinedObjectSpaceBox( *fOwnerEntity( ) );
		sync_event_v_c( mBounds.mMin, tSync::cSCDebris );
		sync_event_v_c( mBounds.mMax, tSync::cSCDebris );

		tVec3f scale = trans.fGetScale( );
		tVec3f center = mBounds.fComputeCenter( );
		mOffset = tMat3f(tQuatf::cIdentity, -center * scale);
		mOffset.fScaleGlobal( scale );
		mPhysics.fReset( trans * tMat3f(tQuatf::cIdentity, center) );

		sync_event_v_c( mOffset, tSync::cSCDebris );
	}
	
	void tDebrisLogic::fOnDelete( )
	{
		fDecDebrisCount( );
		mIgnore.fRelease( );
		tLogic::fOnDelete( );
	}

	b32 tDebrisLogic::fTooMuchDebris( )
	{
		return sDebrisCount >= Perf_Debris_MaxDebris;
	}

	void tDebrisLogic::fPhysicsSpawn( const Math::tVec3f& initialVel, const Math::tVec3f& effectVel, f32 dt )
	{
		sync_event_v_c( initialVel, tSync::cSCDebris );
		sync_event_v_c( effectVel, tSync::cSCDebris );

		tVec3f velocity = Math::tVec3f( mRandom.fFloatMinusOneToOne( ), 0.f, mRandom.fFloatMinusOneToOne( ) ).fNormalizeSafe( tVec3f::cZeroVector ) * mDef.mHorzVelMax;
		velocity.y = mRandom.fFloatInRange( mDef.mUpVelMin, mDef.mUpVelMax );

		f32 effectMod = 1.f; //default, all of effect influence.
		if( mDef.mEffectMod > 0.f )
			effectMod = mRandom.fFloatInRange( -mDef.mEffectMod, 1.f - mDef.mEffectMod ); //eg mEffectMod = 0.2f -> (-0.2, 0.8f)
		velocity += initialVel + effectVel * mDef.mInheritedVelocityFactor * effectMod;

		// move them a frame ahead randomly[0, 1]
		tMat3f trans = fOwnerEntity( )->fObjectToWorld( );
		const tVec3f posOffset = velocity * mRandom.fFloatZeroToOne( ) * dt; 
		trans.fTranslateGlobal( posOffset );
		fOwnerEntity( )->fMoveTo( trans );

		mRandomFactor = mRandom.fFloatInRange( 0.4f, 1.f );
		//mBounceCount -= (s32)( mRandom.fFloatZeroToOne( ) * mDef.mBounceCountMax ); //random extra bounces

		f32 torque = mDef.mTorqueArm;
		f32 rX = mRandom.fFloatInRange( -torque, torque );
		f32 rZ = mRandom.fFloatInRange( -torque, torque );
		tVec3f torqueArm( rX, 0.0f, rZ );

		f32 gravity = mDef.mGravity;

		if( Gameplay_Debris_SlowMo )
		{
			gravity = -2.0f;
			velocity *= 0.2f;
		}

		tQuatf drdt = velocity.fCross( torqueArm );
		drdt.fNormalizeSafe( tQuatf::cIdentity );

		mPhysics.mV = velocity;
		mPhysics.mGravity = gravity;
		mPhysics.mDRDT = drdt;
	}

	void tDebrisLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListCoRenderMT );
			fRunListRemove( cRunListMoveST );
		}
		else
		{
			fRunListInsert( cRunListCoRenderMT );
			fRunListInsert( cRunListMoveST );
		}
	}

	void tDebrisLogic::fCoRenderMT( f32 dt )
	{
		profile( cProfilePerfDebrisLogicCoRenderMT );
		if( !fTooMuchDebris( ) )
		{
			if( mDormantDueToPerf )
			{
				// reactivate
				mDormantDueToPerf = false;
				mActive = true;
			}

			fCollideAndRespond( dt );
		}
		else if( mActive )
		{
			// deactivate, die
			mDormantDueToPerf = true;
			mActive = false;

			// this will basically cause an insta delete
			mDeathHeight = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ).y;
		}
		mPhysics.fIntegrate( dt );
	}
	
	void tDebrisLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfDebrisLogicMoveST );

		mTotalTimer += dt;

		if( (!mActive && mDeathHeight > mPhysics.fPosition( ).y)
			|| mTotalTimer > mDef.mLifeMax )
		{
			fOwnerEntity( )->fDelete( );
		}
		else
		{
			tMat3f transform = mPhysics.fTransform( );
			mBounceMeter = fMax( transform.fGetTranslation( ).y - mBounceY, mBounceMeter );

			if( Debug_Debris_Render ) fSceneGraph( )->fDebugGeometry( ).fRenderOnce( transform, 1.0f );

			transform *= mOffset;
			fOwnerEntity( )->fMoveTo( transform );

			if( Debug_Debris_Render ) fSceneGraph( )->fDebugGeometry( ).fRenderOnce( Math::tObbf( fOwnerEntity( )->fCombinedObjectSpaceBox( ),  transform ), tVec4f(1,0,0,0.125f) );
		}

		if( mFirstTick )
		{
			mFirstTick = false;
			Gfx::tRenderableEntity::fSetRgbaTint( *fOwnerEntity( ), Math::tVec4f::cOnesVector );
		}
	}
	void tDebrisLogic::fCollideAndRespond( f32 dt )
	{
		mCollisionDelay -= dt;

		sync_event_v_c( fGuid( ), tSync::cSCDebris );

		if( mActive && mCollisionDelay <= 0.f )
		{	
			tVec3f &vel = mPhysics.fVelocity( );
			const tObbf box( mBounds, fOwnerEntity( )->fObjectToWorld( ) );
			const tVec3f center = mPhysics.fPosition( );
			//fOwnerEntity( )->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( box, tVec4f( 1, 0, 1, 1 ) );

			const b32 lowestCorner = false;
			tRayf ray;
			if( lowestCorner )
			{
				tVec3f probeP = box.fSupportingCorner( -tVec3f::cYAxis );

				f32 radius = box.fProjectedRadius( tVec3f::cYAxis );
				tVec3f offset = tVec3f::cYAxis * (radius * 2);
				ray = tRayf( probeP + offset, -offset ); // + vel * dt * 2.0f );
			}
			else
			{
				tVec3f shiftAxis = vel;
				shiftAxis.fNormalizeSafe( tVec3f::cZeroVector );

				//start ray at projected radius of box along velocity
				f32 offset = box.fProjectedRadius( shiftAxis );

				//only use a portion, to get the best of both worlds
				// between full penetration, and full imaginary shape
				const f32 fudgeFact = 0.75f; 
				shiftAxis *= offset * fudgeFact;

				ray = tRayf( center, vel * dt * 2.0f + shiftAxis );
			}

			tDebrisRayCastCallback rayCastcb( fOwnerEntity( ), mIgnore.fGetRawPtr( ) );

			fOwnerEntity( )->fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastcb );

			if( rayCastcb.mHit.fHit( ) )
			{		
				sync_event_v_c( rayCastcb.mHitEntity->fGuid( ), tSync::cSCDebris );
				sync_event_v_c( rayCastcb.mHit.mT, tSync::cSCDebris );

				tVec3f normal = rayCastcb.mHit.mN;
				normal.fNormalizeSafe( tVec3f::cYAxis );

				f32 normalVMag = vel.fDot( normal );
				if( normalVMag < 0.0f )
				{
					//colliding
					tVec3f normalV = normal * normalVMag;

					// Linear
					vel -= normalV;
					vel *= mDef.mBounceFriction * mRandomFactor;
					vel -= normalV * (mDef.mBounceRestitution * mRandomFactor);

					// Clamp max speed
					f32 len;
					vel.fNormalizeSafe( tVec3f::cZeroVector, len );
					vel *= fMin( mDef.mVelocityMax, len );

					// Angular
					f32 rChange = mDef.mBounceFriction * mRandomFactor;
					f32 roll = mRandom.fFloatInRange( 0.f, 2.f );
					tVec3f dirShift = mPhysics.fDRDT( ).fIm( ) + mRandom.fVecNorm<tVec3f>( ) * roll;
					dirShift.fNormalizeSafe( tVec3f::cZeroVector );

					mPhysics.fDRDT( ).fIm( dirShift );
					mPhysics.fDRDT( ) *= rChange;

					if( mBounceMeter < mDef.mBounceHeight )
					{
						mActive = false;
						mDeathHeight = ray.fEvaluate( rayCastcb.mHit.mT ).y - box.fExtents( ).fMax( ) * 2;
						sync_event_v_c( mBounceMeter, tSync::cSCDebris );
						sync_event_v_c( mDeathHeight, tSync::cSCDebris );
					}

					mBounceY = box.fCenter( ).y;
					mBounceMeter = 0;
				}
			}
		}
	}

	Math::tVec4f tDebrisLogic::fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const
	{
		if( varName == cSpecialCamTint )
		{
			return tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamUnitTint( viewportIndex );
		}

		return Math::tVec4f::cZeroVector;
	}


	void fSpawn( tEntity& ent, const Math::tVec3f& initialVel, const Math::tVec3f& effectVel, f32 dt, tEntity& toParent, const tDebrisLogicDef& debrisDef )
	{
		ent.fReparent( toParent );

		sync_event_v_c( ent.fGuid( ), tSync::cSCDebris );
		sync_event_v_c( toParent.fGuid( ), tSync::cSCDebris );
		sync_event_v_c( initialVel, tSync::cSCDebris );
		sync_event_v_c( effectVel, tSync::cSCDebris );
		sync_event_v_c( ent.fGuid( ), tSync::cSCDebris );

		tDebrisLogic *dl = NEW tDebrisLogic( debrisDef );
		tLogicPtr *dlp = NEW tLogicPtr( dl );

		ent.fAcquireLogic( dlp );
		dl->fPhysicsSpawn( initialVel, effectVel, dt );
	}

	void fSpawnDebris( tGrowableArray< tMeshEntityPtr >& entities, const Math::tVec3f& initialVel, const Math::tVec3f& effectVel, f32 dt, tEntity& toParent, const tDebrisLogicDef& debrisDef )
	{
		for( u32 iDebris = 0; iDebris < entities.fCount( ); ++iDebris )
		{
			entities[ iDebris ]->fSetDisabled( false );
			fSpawn( *entities[ iDebris ], initialVel, effectVel, dt, toParent, debrisDef );
		}
	}

	void fSpawnDebris( tGrowableArray< tEntity* >& entities, const Math::tVec3f& initialVel, const Math::tVec3f& effectVel, f32 dt, tEntity& toParent, const tDebrisLogicDef& debrisDef )
	{
		for( u32 iDebris = 0; iDebris < entities.fCount( ); ++iDebris )
			fSpawn( *entities[ iDebris ], initialVel, effectVel, dt, toParent, debrisDef );
	}

	void fSpawnDebris( tEntity* entity, const Math::tVec3f& initialVel, const Math::tVec3f& effectVel, f32 dt, tEntity& toParent, const tDebrisLogicDef& debrisDef )
	{
		fSpawn( *entity, initialVel, effectVel, dt, toParent, debrisDef );
	}
}


namespace Sig
{
	void tDebrisLogic::fExportScriptInterface( tScriptVm& vm )
	{
		//Sqrat::DerivedClass<tDebrisLogic, tLogic, Sqrat::NoCopy<tDebrisLogic> > classDesc( vm.fSq( ) );
		////classDesc;

		//vm.fRootTable( ).Bind(_SC("DebrisLogic"), classDesc);
	}
}

