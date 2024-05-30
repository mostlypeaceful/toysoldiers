#include "BasePch.hpp"
#include "tRagDoll.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tSkeletonFile.hpp"
#include "tPhysicsWorld.hpp"
#include "tShapeEntity.hpp" //for builder
#include "tKeyFrameAnimation.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{
	devvar( u32, Physics_Ragdoll_Steps, 4 );
	devvar( bool, Physics_Ragdoll_LogLimits, false );

	void tRagDollBuilderBasic::fAddJoint( const Math::tMat3f& jointLocation, tRigidBody* b1, tRigidBody* b2 )
	{
		tMat3f aRelPt = b1->fTransform( ).fInverse( ) * jointLocation;
		mConstraints.fPushBack( tRagDollConstraintPtr( NEW tRagDollConstraint( b1, b2, aRelPt ) ) );

		tRigidBody* b = b1;
		for( u32 i = 0; i < mBodies.fCount( ); ++i )
			if( mBodies[ i ].mBody == b )
			{	
				b = NULL;
				break;
			}

		if( b )
			mBodies.fPushBack( tRagDollBody( b ) );

		b = b2;
		for( u32 i = 0; i < mBodies.fCount( ); ++i )
			if( mBodies[ i ].mBody == b )
			{
				b = NULL;
				break;
			}

		if( b )
			mBodies.fPushBack( tRagDollBody( b ) );
	}

	void tRagDollBuilderFromRig::fCollectBodies( const tEntity& entity, u32 tags )
	{
		for( u32 i = 0; i < entity.fChildCount( ); ++i )
		{
			const tEntityPtr& e = entity.fChild( i );
			if( tags != ~0 && !e->fHasGameTagsAll( tags ) )
				continue;

			tShapeEntity* se = e->fDynamicCast<tShapeEntity>( );
			if( se && se->fName( ).fExists( ) )
				fAddShape( se, entity.fWorldToObject( ), se->fName( ) );
		}
	}

	void tRagDollBuilderFromRig::fAddShape( tShapeEntity* se, const Math::tMat3f& parentInvXform, const tStringPtr& boneName )
	{
		sigassert( se );
		mBones.fPushBack( tBone( tCollisionShape::fFromShapeEntity( se, parentInvXform ), boneName ) );

		tBone& newBone = mBones.fBack( );
		newBone.mObjectSpaceBodyPosition = se->fParentRelative( );
		newBone.mObjectSpaceBodyPosition.fNormalizeBasis( );

		newBone.mShape->fTransform( newBone.mObjectSpaceBodyPosition.fInverse( ) );
	}

	void tRagDollBuilderFromRig::fConfigure( const Math::tMat3f& location, const Anim::tAnimatedSkeleton& skeleton )
	{
		log_assert( skeleton.fSkeletonResource( ), "Skeleton file must be set!" );
		tSkeletonFile* skelFile = skeleton.fSkeletonResource( )->fCast< tSkeletonFile >( );

		// find owner bones
		for( u32 i = 0; i < mBones.fCount( ); ++i )
		{
			tBone& out = mBones[ i ];
			const Sig::tBone* skeletonBone = skelFile->fFindBone( out.mBoneName );
			log_assert( skeletonBone, "Bone not found with name: " << out.mBoneName );

			out.mMasterBoneID = skeletonBone->mMasterIndex;
			out.mBoneToShape = skeletonBone->mRefPoseInv * out.mObjectSpaceBodyPosition;
		}

		// sort them by hierarchy
		std::sort( mBones.fBegin( ), mBones.fEnd( ) );

		// Create output bodies and constraints
		mBodies.fSetCount( mBones.fCount( ) );
		for( u32 i = 0; i < mBones.fCount( ); ++i )
		{
			tBone& input = mBones[ i ];
			tRagDollBody& out = mBodies[ i ];
			const Sig::tBone& skeletonBone = skelFile->fBone( input.mMasterBoneID );

			out.mName = input.mBoneName;
			out.mMasterBoneIndex = input.mMasterBoneID;
			out.mBoneToBody = input.mBoneToShape;
			out.mBodyToBone = input.mBoneToShape.fInverse( );
			
			out.mBody.fReset( NEW tRigidBody( ) );
			out.mBody->fAddShape( input.mShape );
			// todo set mass from shapes
			out.mBody->fSetTransform( location * input.mObjectSpaceBodyPosition );
			out.mBody->fSetMassPropertiesFromShape( 1.f, 3.f );

			if( i == 0 )
			{
				//parent, no constraint
				mInverse_RootBody_ObjectSpaceXform = input.mObjectSpaceBodyPosition.fInverse( );
			}
			else
			{
				// We need to add a constraint
				u32 parentBodyIndex = ~0;

				// traverse up the skeleton until we find one of the bones in our list.
				s32 parentBone = skeletonBone.mParent;
				while( parentBone >= 0 )
				{
					// see if bone is in our list
					for( u32 b = 0; b < mBones.fCount( ); ++b )
						if( mBones[ b ].mMasterBoneID == parentBone )
						{
							// it is
							parentBodyIndex = b;
							break;
						}

					if( parentBodyIndex != ~0 )
						break;

					// keep looking
					const Sig::tBone& parentSBone = skelFile->fBone( parentBone );
					parentBone = parentSBone.mParent;
				}

				// Now we know which body is our parent, we can add a constraint to it and us.
				sigassert( parentBodyIndex != ~0 );
				const tBone& parentBodyData = mBones[ parentBodyIndex ];
				tRigidBody* parentBody = mBodies[ parentBodyIndex ].mBody.fGetRawPtr( );
				sigassert( parentBody );

				// give parent a link to us
				mBodies[ parentBodyIndex ].mConnectedTo.fPushBack( i );

				tMat3f aRelPt = parentBodyData.mObjectSpaceBodyPosition.fInverse( ) * skeletonBone.mRefPose;
				mConstraints.fPushBack( tRagDollConstraintPtr( NEW tRagDollConstraint( parentBody, out.mBody.fGetRawPtr( ), aRelPt ) ) );
				out.mConstraint = mConstraints.fBack( );
				out.mParentBody = parentBodyIndex;
			}
		}		
	}


	tRagDoll::tRagDoll( )
		: mApplyExpectedLocation( false )
		, mComputingDelta( false )
	{ 
	}

	void tRagDoll::fSetup( const tRagDollBuilder& builder )
	{
		sigassert( !mData.mBodies.fCount( ) && "Already setup, resetup not supported yet." );
		mData = builder;
	}

	void tRagDoll::fSetWorld( tPhysicsWorld* world )
	{
		tPhysicsWorld* prevWorld = mInWorld;

		if( prevWorld )
		{
			tConstraint::fSetWorld( NULL );
			fRemoveEverything( *prevWorld );
		}

		if( world )
		{
			// Setting the main ragdoll constraint BodyA will include this constraint in its island.
			sigassert( mData.mBodies.fCount( ) && "Ragdoll had no bodies?!" );
			mBodyA = mData.mBodies[ 0 ].mBody; //root body is our primary constraint body, for island stuff

			fAddEverything( *world );
			tConstraint::fSetWorld( world );
		}
	}

	void tRagDoll::fAddEverything( tPhysicsWorld& world )
	{
		// add bodies first, then inform constraints
		for( u32 i = 0; i < mData.mBodies.fCount( ); ++i )
			world.fAddObject( mData.mBodies[ i ].mBody.fGetRawPtr( ) );

		for( u32 i = 0; i < mData.mWorldConstraints.fCount( ); ++i )
			mData.mWorldConstraints[ i ]->fSetWorld( &world );

		for( u32 i = 0; i < mData.mConstraints.fCount( ); ++i )
			mData.mConstraints[ i ]->fSetWorld( &world );
	}

	void tRagDoll::fRemoveEverything( tPhysicsWorld& world )
	{
		// inform constraints, then remove bodies
		for( u32 i = 0; i < mData.mConstraints.fCount( ); ++i )
			mData.mConstraints[ i ]->fSetWorld( NULL );

		for( u32 i = 0; i < mData.mWorldConstraints.fCount( ); ++i )
			mData.mWorldConstraints[ i ]->fSetWorld( NULL );

		for( u32 i = 0; i < mData.mBodies.fCount( ); ++i )
			world.fRemoveObject( mData.mBodies[ i ].mBody.fGetRawPtr( ) );
	}

	void tRagDoll::fSetFriction( f32 friction )
	{
		for( u32 i = 0; i < mData.mBodies.fCount( ); ++i )
			mData.mBodies[ i ].mBody->fSetFriction( friction );
	}

	void tRagDoll::fSetLocation( const Math::tMat3f& entityXform, const Anim::tAnimatedSkeleton& skeleton )
	{
		const Anim::tAnimatedSkeleton::tMatrixPalette& pallette = skeleton.fMatrixPalette( );

		sigassert( skeleton.fSkeletonResource( ) );
		const tSkeletonFile* skelFile = skeleton.fSkeletonResource( )->fCast< tSkeletonFile >( );

		for( u32 i = 0; i < mData.mBodies.fCount( ); ++i )
		{
			tRagDollBody& body = mData.mBodies[ i ];
			const tBone& bone = skelFile->fBone( body.mMasterBoneIndex );

			// assuming this was called outside of a fStepPostTracks function, 
			//  these will be in ref space, ie deltas.
			//  convert to object space.
			body.mBody->fReset( entityXform * pallette[ body.mMasterBoneIndex ] * bone.mRefPose * body.mBoneToBody );
		}
	}

	void tRagDoll::fSetConstraintLimits( const tKeyFrameAnimation* anim )
	{
		sigassert( anim );

		for( u32 i = 0; i < anim->mBones.fCount( ); ++i )
		{
			const tKeyFrameAnimation::tBone& kfaBone = anim->mBones[ i ];
			tRagDollBody* body = mData.mBodies.fFind( kfaBone.mMasterBoneIndex );

			if( body && body->mConstraint )
			{
				if( Physics_Ragdoll_LogLimits )
					log_line( 0, "Setting limits: " << body->mName << " order: " << kfaBone.mIKAxisLimitsOrder << " values: " << kfaBone.mIKAxisLimits );
				body->mConstraint->fSetConstraints( kfaBone.mIKAxisLimitsOrder, kfaBone.mIKAxisLimits );
			}
		}
	}

	void tRagDoll::fBeginComputingDelta( )
	{
		mComputingDelta = true;
		fCreatePose( mDeltaPose1 );
	}

	void tRagDoll::fEndComputingDelta( f32 dt )
	{
		sigassert( mComputingDelta );
		mComputingDelta = false;
		tPose pose2;
		fCreatePose( pose2 );
		fInitializeVelocities( mDeltaPose1, pose2, dt );
	}

	void tRagDoll::fCreatePose( tPose& output )
	{
		output.mXforms.fSetCount( mData.mBodies.fCount( ) );
		for( u32 i = 0; i < mData.mBodies.fCount( ); ++i )
			output.mXforms[ i ] = mData.mBodies[ i ].mBody->fTransform( );
	}

	void tRagDoll::fInitializeVelocities( const tPose& a, const tPose& b, f32 dt )
	{
		sigassert( a.mXforms.fCount( ) == b.mXforms.fCount( ) );
		for( u32 i = 0; i < a.mXforms.fCount( ); ++i )
		{
			tVec3f p1 = a.mXforms[ i ].fGetTranslation( );
			tQuatf r1( a.mXforms[ i ] );
			tVec3f p2 = b.mXforms[ i ].fGetTranslation( );
			tQuatf r2( b.mXforms[ i ] );

			tRigidBody* body = mData.mBodies[ i ].mBody.fGetRawPtr( );
			body->fSetVelocity( (p2 - p1) / dt );
			body->fSetAngularVelocity( tRigidBody::fWFromDeltaR( r1, r2, dt ) );
		}
	}

	void tRagDoll::fStepConstraintInternal( f32 dt, f32 percentage )
	{
		fUpdateSleeping( dt );

		// sub step the ragdoll constraints
		u32 steps = Physics_Ragdoll_Steps;
		f32 rate = 1.f / steps;
		f32 subDt = dt * rate;

		for( u32 s = 0; s < steps; ++s )
		{
			//f32 subPercent = (s+1) * rate * percentage;
			for( u32 i = 0; i < mData.mWorldConstraints.fCount( ); ++i )
			{
				mData.mWorldConstraints[ i ]->fStepConstraintInternal( subDt, 1.0f );
			}

			for( u32 i = 0; i < mData.mConstraints.fCount( ); ++i )
			{
				//mData.mConstraints[ i ]->fSetChildWeightScale( (s==0) ? 1.f : 0.f );
				mData.mConstraints[ i ]->fStepConstraintInternal( subDt, 1.0f );
			}
		}
	}

	void tRagDoll::fUpdateSleeping( f32 dt )
	{
		mSleepingParams.fApplyOverride( );

		tVec3f w = tVec3f::cZeroVector;
		tVec3f v = tVec3f::cZeroVector;

		u32 bodies = mData.mBodies.fCount( );
		for( u32 i = 0; i < bodies; ++i )
		{
			tRigidBody& body = *mData.mBodies[ i ].mBody;
			w += body.mW;
			v += body.mV;
		}

		w /= (f32)bodies;
		v /= (f32)bodies;

		f32 wLenSqr = w.fLengthSquared( );
		f32 vLenSqr = v.fLengthSquared( );

		b32 sleeping = (wLenSqr <= mSleepingParams.mWLenSqrd && vLenSqr <= mSleepingParams.mVLenSqrd);
		
		if( sleeping )
			mIslandData.mIdleTime += dt;
		else
			mIslandData.mIdleTime = 0.f;

		sleeping = ( mIslandData.mIdleTime >= mSleepingParams.mIdleTime );
		if( !sleeping )
			mIslandData.fAwaken( );

		for( u32 i = 0; i < bodies; ++i )
		{
			tRigidBody& body = *mData.mBodies[ i ].mBody;
			body.fSleepingParams( ).mIdleTime = sleeping ? -1.f : mSleepingParams.mIdleTime; // -1 force sleeps
		}
	}

	void tRagDoll::fDebugDraw( tPhysicsWorld& world )
	{
		for( u32 i = 0; i < mData.mConstraints.fCount( ); ++i )
			mData.mConstraints[ i ]->fDebugDraw( world );
		for( u32 i = 0; i < mData.mWorldConstraints.fCount( ); ++i )
			mData.mWorldConstraints[ i ]->fDebugDraw( world );
	}

	void tRagDoll::fLaunch( u32 bodyIndex, const Math::tVec3f& velocity, f32 fallOff, f32 duration )
	{
		mLaunch.mTimeRemaining = duration;
		mLaunch.mStartBody = bodyIndex;
		mLaunch.mVelocity = velocity;
		mLaunch.mFallOff = fallOff;
	}

	void tRagDoll::fApplyLaunch( f32 dt )
	{
		if( mLaunch.mTimeRemaining > 0.f )
		{
			mLaunch.mTimeRemaining -= dt;

			fApplyLaunchRecursive( mData.mBodies[ mLaunch.mStartBody ], mData.mBodies, mLaunch.mVelocity, 1.f, mLaunch.mFallOff );
		}
	}

	void tRagDoll::fApplyLaunchRecursive( tRagDollBody& body, tGrowableArray<tRagDollBody>& bodies, const tVec3f& baseVel, f32 weight, f32 fallOff )
	{
		body.mBody->fSetVelocity( fLerp( body.mBody->fVelocity( ), baseVel, weight ) );

		f32 newWeight = weight - fallOff;
		if( newWeight > 0.f )
		{
			for( u32 i = 0; i < body.mConnectedTo.fCount( ); ++i )
				fApplyLaunchRecursive( bodies[ body.mConnectedTo[ i ] ], bodies, baseVel, newWeight, mLaunch.mFallOff );
		}
	}
	
}}

namespace Sig { namespace Physics
{

	void tRagDoll::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tRagDoll, Sqrat::NoCopy<tRagDoll> > classDesc( vm.fSq( ) );
		classDesc
			;

		vm.fNamespace(_SC("Physics")).Bind( _SC("RagDoll"), classDesc );
	}

} }