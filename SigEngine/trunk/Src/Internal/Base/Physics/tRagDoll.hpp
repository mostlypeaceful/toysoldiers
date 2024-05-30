#ifndef __tRagDoll__
#define __tRagDoll__

#include "tConstraint.hpp"
#include "tRigidBody.hpp"
#include "tRagDollConstraint.hpp"
#include "tShapeEntity.hpp"

namespace Sig { 

	class tKeyFrameAnimation;

namespace Physics
{
	struct tRagDollBody
	{
		// Skeleton mapping stuff
		tStringPtr mName;
		u32 mMasterBoneIndex;

		// computed from setup rig
		Math::tMat3f mBoneToBody;
		Math::tMat3f mBodyToBone;

		tRigidBodyPtr mBody;
		tRagDollConstraintPtr mConstraint; //will not be set for root
		u32 mParentBody; //will be set if constraint is set
		tGrowableArray<u32> mConnectedTo;

		tRagDollBody( tRigidBody* body = NULL )
			: mMasterBoneIndex( ~0 )
			, mBody( body )
			, mParentBody( ~0 )
		{ }

		b32 operator == ( const tStringPtr& name ) const { return mName == name; }
		b32 operator == ( const u32 masterBoneIndex ) const { return mMasterBoneIndex == masterBoneIndex; }
	};

	struct tRagDollBuilder : public tRefCounterCopyable
	{
		tRagDollBuilder( )
			: mInverse_RootBody_ObjectSpaceXform( Math::tMat3f::cIdentity )
		{ }

		virtual ~tRagDollBuilder( ) 
		{ }

		tGrowableArray< tRagDollBody > mBodies;
		tGrowableArray< tConstraintPtr > mWorldConstraints;
		tGrowableArray< tRagDollConstraintPtr > mConstraints;
		Math::tMat3f mInverse_RootBody_ObjectSpaceXform;
	};

	// Mostly for testing
	//	This builds a really ghetto but functional ragdoll, like out of trashcans or something.
	struct tRagDollBuilderBasic : public tRagDollBuilder
	{
		void fAddJoint( const Math::tMat3f& jointLocation, tRigidBody* b1, tRigidBody* b2 );
	};

	// built from skeleton data and shapes
	struct tRagDollBuilderFromRig : public tRagDollBuilder
	{
		// 1.) call this, it will collect shape entities with a given tag mask and will map their names to skeleton bones.
		//  ~0 to accept all shapes.
		void fCollectBodies( const tEntity& entity, u32 tags );

		// 2.) The configure for a rig.
		void fConfigure( const Math::tMat3f& location, const Anim::tAnimatedSkeleton& skeleton );

	private:
		// in parent relative space, will create bodies for you, determining hierarchical order
		void fAddShape( tShapeEntity* se, const Math::tMat3f& parentInvXform, const tStringPtr& boneName );

		struct tBone
		{
			tCollisionShape* mShape;
			Math::tMat3f mObjectSpaceBodyPosition;
			Math::tMat3f mBoneToShape;

			tStringPtr mBoneName;
			u32 mMasterBoneID;

			tBone( tCollisionShape* shape = NULL, const tStringPtr& boneName = tStringPtr::cNullPtr )
				: mShape( shape )
				, mBoneName( boneName )
				, mMasterBoneID( ~0 )
			{ }

			b32 operator < ( const tBone& right ) const { return mMasterBoneID < right.mMasterBoneID; }
		};

		tGrowableArray<tBone> mBones;
	};

	struct tPose
	{
		tGrowableArray<Math::tMat3f> mXforms;
	};

	struct tLaunchData
	{
		f32		mTimeRemaining;
		u32		mStartBody;
		f32		mFallOff; //0.f only all bodies, 1.f is only this body. How much of the velocity is lost going to the next body.
		Math::tVec3f mVelocity;

		tLaunchData( ) 
			: mTimeRemaining( -1.f ) 
		{ }
	};

	class tRagDoll : public tConstraint
	{
	public:
		tRagDoll( );

		void fSetup( const tRagDollBuilder& builder );

		void fSetFriction( f32 friction );
		void fSetLocation( const Math::tMat3f& entityXform, const Anim::tAnimatedSkeleton& skeleton );
		void fSetConstraintLimits( const tKeyFrameAnimation* anim );

		void fBeginComputingDelta( );
		void fEndComputingDelta( f32 dt );

		void fInsertWorldConstraint( tConstraintPtr& constraint ) { mData.mWorldConstraints.fPushFront( constraint ); }

		const tRagDollBuilder&	fData( ) const { return mData; }
		tRigidBody*				fRootBody( ) const { return mData.mBodies[ 0 ].mBody.fGetRawPtr( ); }

		// Feed these into the character entity
		void fSetApplyExpectedLocation( b32 apply ) { mApplyExpectedLocation = apply; }
		b32 fApplyExpectedLocation( ) const { return mApplyExpectedLocation; }
		Math::tMat3f fExpectedEntityXform( ) const { return fRootBody( )->fTransform( ) * mData.mInverse_RootBody_ObjectSpaceXform; }

		tSleepingParams& fSleepingParams( ) { return mSleepingParams; }

		u32 fFindBody( const tStringPtr& name ) const { return mData.mBodies.fIndexOf( name ); }
		void fLaunch( u32 bodyIndex, const Math::tVec3f& velocity, f32 fallOff, f32 duration );
		void fApplyLaunch( f32 dt );

	protected:
		virtual void fStepConstraintInternal( f32 dt, f32 percentage );
		virtual void fDebugDraw( tPhysicsWorld& world );
		virtual void fSetWorld( tPhysicsWorld* world );

	private:
		tRagDollBuilder mData;
		tLaunchData mLaunch;

		b32 mApplyExpectedLocation;
		b32 mComputingDelta;
		tPose mDeltaPose1;

		void fAddEverything( tPhysicsWorld& world );
		void fRemoveEverything( tPhysicsWorld& world );

		// these treat the ragdoll as a whole, and not per rigid body.
		tSleepingParams mSleepingParams;
		void fUpdateSleeping( f32 dt );

		void fCreatePose( tPose& output ); //off the current state
		void fInitializeVelocities( const tPose& a, const tPose& b, f32 dt );

		void fApplyLaunchRecursive( tRagDollBody& body, tGrowableArray<tRagDollBody>& bodies, const Math::tVec3f& baseVel, f32 weight, f32 fallOff );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	};

	typedef tRefCounterPtr< tRagDoll > tRagDollPtr;

	
}}

#endif//__tRagDoll__
