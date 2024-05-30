#include "BasePch.hpp"
#include "tUberBreakableDebris.hpp"
#include "tSceneGraph.hpp"
#include "tUberBreakableLogic.hpp"
#include "tSceneGraphCollectTris.hpp"

#include "tProfiler.hpp"
#include "tRandom.hpp"
#include "tSync.hpp"

namespace Sig
{
	tUberBreakableDebris::tUberBreakableDebrisOnBounce tUberBreakableDebris::gOnDebrisBounce = 0;

	u32 tUberBreakableDebris::sDebrisCount = 0;

	devvar( u32, Perf_Debris_MaxDebrisUber, 120 );

	namespace
	{
		struct tDebrisRayCastCallback
		{
			mutable Math::tRayCastHit		mHit;

			inline void operator( )( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				tSpatialEntity* spatial = static_cast<tSpatialEntity*>( i );
				if( !spatial->fHasGameTagsAny( tUberBreakableLogic::cGroundFlags | tUberBreakableLogic::cCollisionFlags | tUberBreakableLogic::cDebrisFlags ) )
					return;
				if( spatial->fToSpatialSetObject( )->fQuickRejectByBox( ray ) )
					return;

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );

				if( hit.fHit( ) && hit.mT < mHit.mT )
					mHit = hit;
			}
		};
		static void fSpawn( tEntity& ent, tEntity& newParent, const Math::tVec3f& velocity )
		{
			ent.fReparent( newParent );

			tUberBreakableDebris *dl = NEW tUberBreakableDebris( );
			tLogicPtr *dlp = NEW tLogicPtr( dl );

			ent.fAcquireLogic( dlp );
			dl->fPhysicsSpawn( velocity );
		}
	}

	void tUberBreakableDebris::fSpawnFromMesh( tMeshEntity& mesh, tEntity& newParent, const Math::tVec3f& velocity )
	{
		fSpawn( mesh, newParent, velocity );
	}

	b32 tUberBreakableDebris::fTooMuchDebris( )
	{
		return sDebrisCount >= Perf_Debris_MaxDebrisUber;
	}

	tUberBreakableDebris::tUberBreakableDebris( )
		: mCOMOffset( Math::tVec3f::cZeroVector )
		, mScale( Math::tVec3f::cOnesVector )
		, mPhysicsPos( Math::tMat3f::cIdentity )
		, mVelocity( Math::tVec3f::cZeroVector )
		, mRotation( Math::tQuatf::cIdentity )
		, mRotationDelta( Math::tQuatf::cZeroQuat )
		, mBounceCount( 0 )
		, mActive( true )
		, mBounced( false )
		, mDormantDueToPerf( false )
		, mDeathHeight( 0.f )
		, mTotalTimer( 0.f )

		, mMaxBounces( 6 )
		, mGravity( -9.8f * 2 )
		, mMassCoeff( 0.25f ) // [0,1] zero is infinite mass, 1 is no resistance
		, mEnergyLossNormCoeff( 0.50f ) // [0,1] zero is total energy loss, one is no energy loss
		, mEnergyLossTangCoeff( 0.50f ) // [0,1] zero is total energy loss, one is no energy loss
		, mRandVelocityCoeff( 0.50f ) // [0,1] zero is no randomness, 1 is lots of randomness
		, mRandRotationCoeff( 0.50f ) // [0,1] zero is no randomness, 1 is lots of randomness
		, mRandom( sync_rand( fUInt( ) ) )
	{
		mBounds.fInvalidate( );
	}

	void tUberBreakableDebris::fOnSpawn( )
	{
		fIncDebrisCount( );

		fOnPause( false );
		fOwnerEntity( )->fRemoveGameTags( ~0 );
		fOwnerEntity( )->fAddGameTags( tUberBreakableLogic::cDebrisFlags );
	}

	void tUberBreakableDebris::fOnDelete( )
	{
		fDecDebrisCount( );
		tLogic::fOnDelete( );
	}

	void tUberBreakableDebris::fPhysicsSpawn( const Math::tVec3f& velocity )
	{
		// compute object space bounds of mesh
		mBounds = tMeshEntity::fCombinedObjectSpaceBox( *fOwnerEntity( ) );

		// compute COM offset
		const Math::tVec3f center = mBounds.fComputeCenter( );
		mCOMOffset = -center;

		// set physics position, and compensate for renderable COM offset
		mPhysicsPos = fOwnerEntity( )->fObjectToWorld( );
		mPhysicsPos.fTranslateLocal( -mCOMOffset );

		// store and remove scale so we can restore it later
		mScale = mPhysicsPos.fGetScale( );
		mPhysicsPos.fNormalizeBasis( );

		// initialize current rotation
		mRotation = Math::tQuatf( mPhysicsPos );
		mRotation.fNormalizeSafe( Math::tQuatf::cIdentity );

		// initial linear velocity
		mVelocity = velocity * mMassCoeff;
		const f32 vLen = mVelocity.fLength( );

		if( vLen > 0.f )
		{
			// compute random velocity delta in the plane tangent to the actual velocity

			Math::tVec3f x = mVelocity.fCross( Math::tVec3f::cXAxis );
			if( x.fIsZero( ) )
				x = mVelocity.fCross( Math::tVec3f::cYAxis );
			sigassert( !x.fIsZero( ) );

			Math::tVec3f y = x.fCross( mVelocity );
			sigassert( !y.fIsZero( ) );

			const f32 rX = mRandom.fFloatInRange( -1.f, +1.f );
			const f32 rY = mRandom.fFloatInRange( -1.f, +1.f );

			x.fNormalize( ); x *= rX * mRandVelocityCoeff * vLen;
			y.fNormalize( ); y *= rY * mRandVelocityCoeff * vLen;

			// apply random delta to initial velocity
			mVelocity += x + y;
		}

		// compute random rotation vector - this is not really correct in any pure "physics" sense,
		// we're just treating quaternions as vectors (which they are) and applying a small increment each frame
		mRotationDelta = mRandRotationCoeff * mRandom.fVecNorm<Math::tQuatf>( );
	}

	void tUberBreakableDebris::fOnPause( b32 paused )
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

	void tUberBreakableDebris::fCoRenderMT( f32 dt )
	{
		profile( cProfilePerfDebrisLogicCoRenderMT );

		const Math::tMat3f prevPos = mPhysicsPos;

		mVelocity.y += mGravity * dt;

		// integrate position
		Math::tVec3f pos = mPhysicsPos.fGetTranslation( );
		pos += mVelocity * dt;

		mPhysicsPos.fSetTranslation( pos );

		// "integrate" rotation - as mentioned above, this is h4x0r but looks good
		mRotation += mRotationDelta * dt;
		mRotation.fNormalizeSafe( Math::tQuatf::cIdentity );
		mRotation.fToMatrix( mPhysicsPos );
		mPhysicsPos.fScaleLocal( mScale );

		if( !fTooMuchDebris( ) )
		{
			if( mDormantDueToPerf )
			{
				// reactivate
				mDormantDueToPerf = false;
				mActive = true;
			}

			fCollideAndRespond( dt, prevPos );
		}
		else if( mActive )
		{
			// deactivate, die
			mDormantDueToPerf = true;
			mActive = false;

			// this will basically cause an insta delete
			//mDeathHeight = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ).y;
		}
	}
	
	void tUberBreakableDebris::fMoveST( f32 dt )
	{
		profile( cProfilePerfDebrisLogicMoveST );

		if( mBounced )
		{
			mBounced = false;
			if( gOnDebrisBounce )
				gOnDebrisBounce( *this );
		}

		if( mActive )
			mTotalTimer += dt;

		const f32 lifeMaxTime = 10.f;

		if( (!mActive && mPhysicsPos.fGetTranslation( ).y < mDeathHeight) || mTotalTimer > lifeMaxTime )
			fOwnerEntity( )->fDelete( );
		else
		{
			Math::tMat3f xform = mPhysicsPos;
			xform.fTranslateLocal( mCOMOffset );
			fOwnerEntity( )->fMoveTo( xform );
		}
	}
	void tUberBreakableDebris::fCollideAndRespond( f32 dt, const Math::tMat3f& prevPos )
	{
		sync_event_v_c( fGuid( ), tSync::cSCDebris );

		if( mActive && mBounceCount < mMaxBounces )
		{
			const f32 bounceFactor = ( 1.f - ( mBounceCount / ( mMaxBounces - 1.f ) ) );

			const Math::tVec3f pos0 = prevPos.fGetTranslation( );
			const Math::tVec3f pos1 = mPhysicsPos.fGetTranslation( );
			const Math::tVec3f vel = pos1 - pos0;

			Math::tVec3f shiftAxis = vel;
			shiftAxis.fNormalizeSafe( Math::tVec3f::cZeroVector );

			//start ray at projected radius of box along velocity
			const Math::tObbf worldObb( mBounds, fOwnerEntity( )->fObjectToWorld( ) );
			const f32 offset = worldObb.fProjectedRadius( shiftAxis );

			// only use a portion, to get the best of both worlds
			// between full penetration, and full imaginary shape
			const f32 fudgeFact = bounceFactor;
			shiftAxis *= offset * fudgeFact;

			const Math::tRayf ray( pos0 + shiftAxis, vel );

			tDebrisRayCastCallback rayCastCb;

			if( tUberBreakableLogic::cCollideWithShapeSpatialSet )
				fOwnerEntity( )->fSceneGraph( )->fRayCast( ray, rayCastCb, tShapeEntity::cSpatialSetIndex );
			fOwnerEntity( )->fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastCb );

			if( rayCastCb.mHit.fHit( ) )
			{
				const Math::tVec3f hitPos = ray.fEvaluate( rayCastCb.mHit.mT );
				sigassert( !hitPos.fIsNan( ) );

				Math::tVec3f normal = rayCastCb.mHit.mN;
				sigassert( !normal.fIsNan( ) );
				sigassert( !normal.fIsZero( ) );
				normal.fNormalize( );

				// compute normal velocity taking friction into account
				const Math::tVec3f reflectedV = Math::fReflect( mVelocity, normal );
				const Math::tVec3f vNorm = Math::fProject( normal, reflectedV ) * mEnergyLossNormCoeff;

				// compute tangential velocity taking friction into account
				const Math::tVec3f tangent = reflectedV - Math::fProject( normal, reflectedV );
				const Math::tVec3f vTang = tangent * mEnergyLossTangCoeff;

				// new velocity (combination of tangent + normal velocities)
				mVelocity = vNorm + vTang;
				sigassert( !mVelocity.fIsNan( ) );
				mVelocity.fClampLength( tUberBreakableLogic::cDebrisMaxV );

				// correct position to be at the point of intersection
				mPhysicsPos.fSetTranslation( hitPos - shiftAxis );

				// recompute a new rotation delta - again, h4x0r, the point is that it changes when it bounces
				mRotationDelta = mRandRotationCoeff * mRandom.fVecNorm<Math::tQuatf>( );

				// track bounces
				++mBounceCount;
				mBounced = true;

				mDeathHeight = ray.fEvaluate( rayCastCb.mHit.mT ).y - worldObb.fExtents( ).fMax( ) * 2;
			}
		}
		else
			mActive = false;
	}
}
