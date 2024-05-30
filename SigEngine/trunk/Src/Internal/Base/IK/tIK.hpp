#ifndef _tIK_
#define _tIK_

namespace Sig 
{
	namespace IK
	{

	enum tSolveResult { cSolveResultFailed, cSolveResultSolved, cSolvedResultSolvedNotIdeal };
	
	struct base_export tIKJoint
	{
		enum tFlags 
		{
			cHingeJointFlag = 1<<0, 
			cConeJointFlag = 1<<1,
			cFixedJointFlag = 1<<2,
			cFreeJointFlag = 1<<3
		};

		// Setup Params
		u8				mID; //this just makes debugging easier
		u8				mFlags;
		u8				mAxesLimitsOrder; //this value maps to cml::EulerOrder
		u8				mHingeAxis; //if hinge is set

		f32				mLength;
		f32				mLenSqr;
		Math::tMat3f	mBindParentRelative;
		Math::tVec3f	mAxesLimitsLow;  //In parent space
		Math::tVec3f	mAxesLimitsHigh; //In parent space
		Math::tQuatf	mBindPoseToLocal; //rotate the bone space back into world aligned space.
		Math::tQuatf	mLocalToBindPose;
		Math::tQuatf	mLimbToBindSpace; //Limb space has Z pointing down all the bones, this is the delta from that to the bind pose

		// Output Params
		Math::tQuatf	mLocalDelta;  //In parent space
		Math::tMat3f	mWorldTransform;

		tIKJoint( const Math::tMat3f& bindParentRel = Math::tMat3f::cIdentity )
			: mID( 0 )
			, mFlags( 0 )
			, mAxesLimitsOrder( Math::cEulerOrderXYZ )
			, mLength( 1.f )
			, mLenSqr( mLength * mLength )
			, mBindParentRelative( bindParentRel )
			, mAxesLimitsLow( Math::tVec3f( -Math::cInfinity ) )
			, mAxesLimitsHigh( Math::tVec3f( Math::cInfinity ) )
			, mBindPoseToLocal( Math::tQuatf::cIdentity )
			, mLocalToBindPose( Math::tQuatf::cIdentity )
			, mLimbToBindSpace( Math::tQuatf::cIdentity )
			, mLocalDelta( Math::tQuatf::cIdentity )
			, mWorldTransform( Math::tMat3f::cIdentity )
		{ }

		// This is basically used for the effector, to reorient the transform so that it is object aligned in bind pose, and back again.
		//  this allows the artist to orient the effector bone how ever they like, and the programmer to treat it as object space in the callback.
		void fToObjectSpace( Math::tMat3f& mat ) const;
		void fToOriginalSpace( Math::tMat3f& mat ) const;
		void fDetermineFlags( );
		void fSetLength( f32 len );
		void fIncLength( f32 incLen );
	};

	// Arbitrary N chain
	class base_export tIKChain
	{
	public:
		tIKChain( );

		void fAddJoint( const tIKJoint& joint );
		const tGrowableArray< tIKJoint >& fJoints( ) const { return mJoints; }
		inline f32 fLimbLength( ) const { return mLimbLength; }

		tSolveResult fSolve( const Math::tMat3f& root, const Math::tMat3f& endEffector, f32 dt, tLogic& logic );
		void fRender( const Math::tMat3f& xForm, const Math::tMat3f& root, const Math::tMat3f& endEffector, tLogic& logic );

		void fSetupTestData( ); // for debugging only!

	private:
		tGrowableArray< tIKJoint >	mJoints;
		tGrowableArray< u32 >		mPriority;
		f32				mLimbLength;
		Math::tMat3f	mWorldRoot;
		Math::tMat3f	mEndEffector;

		void fForwardSolveToEffector( u32 bone );
		void fForwardSolveBone( u32 bone );

		void fClampLocal( tIKJoint& joint );
		Math::tMat3f fGetBoneParentWorldSpace( const tIKJoint& joint ) const;
		void fSetWorldOrientation( const Math::tMat3f& worldR, tIKJoint& joint );
	};

	// Human arm or leg.
	class base_export tIKLimb
	{
	public:
		tIKLimb( );

		void fAddJoint( const tIKJoint& joint );
		const tGrowableArray< tIKJoint >& fJoints( ) const { return mJoints; }
		inline f32 fLimbLength( ) const { return mLimbLength; }

		tSolveResult fSolve( const Math::tMat3f& root, const Math::tMat3f& endEffector, f32 dt, tLogic& logic );
		void fRender( const Math::tMat3f& xForm, const Math::tMat3f& root, const Math::tMat3f& endEffector, tLogic& logic );

		void fSetupTestData( ); // for debugging only!

	private:
		enum tJoints { cUpperJoint, cLowerJoint, cEndJoint, cJointCount };
		u32 mJointIndices[ cJointCount ];

		tGrowableArray< tIKJoint >	mJoints;
		f32				mLimbLength;
		Math::tMat3f	mWorldRoot;
		Math::tMat3f	mEndEffector;

		tIKJoint& fJoint( tJoints joint ) { return mJoints[ mJointIndices[ joint ] ]; }
		void fComputeCombinedJointInfo( tJoints from, tJoints to );
		void fForwardSolveToEffector( u32 bone );
		void fForwardSolveBone( u32 bone );
	};

	// When tIKCallback is called, mObjectSpaceEffectorTarget contains the animated effector position.
	//  The user is expected to modify the mObjectSpaceEffectorTarget which the ik system will use to manipulate the chain.
	struct base_export tIKCallbackArgs
	{
		Math::tMat3f mObjectSpaceEffectorTarget;
		const Math::tMat3f mObjectSpaceBoneFirstBone;
		u32			 mChannel; //User defined id number
		b16          mChanged; //true if the user changed the object space effector target
		u16			 mSolveResult; //from pervious frame
		f32			 mIKBlendStrength;
		f32			 mDT;

		tIKCallbackArgs( const Math::tMat3f effectorTarget = Math::tMat3f::cIdentity
			, const Math::tMat3f& objectSpaceBoneFirstBone = Math::tMat3f::cIdentity
			, u32 channel = 0 ) 

			: mObjectSpaceEffectorTarget( effectorTarget )
			, mObjectSpaceBoneFirstBone( objectSpaceBoneFirstBone )
			, mChannel( channel )
			, mChanged( false )
		{ }
	};
	
	class base_export tIKCallback : public tRefCounter
	{
	public:
		virtual ~tIKCallback( ) { }

		virtual void fCallbackMT( tIKCallbackArgs& args ) = 0;
		virtual void fDebugData( std::stringstream& ss, u32 indentDepth, u32 channel ) const { ss << std::endl; }
	};
	typedef tRefCounterPtr< tIKCallback > tIKCallbackPtr;

	class base_export tCharacterLegTargets : public tIKCallback
	{
	public:
		tCharacterLegTargets( ) : mOwner( NULL ) { }
		tCharacterLegTargets( tLogic *owner, tEntityTagMask ignoreTags, tEntityTagMask requireTags, b32 doLock );

		void fSetIdle( b32 idle );

		b32 fHandleLogicEvent( const Logic::tEvent& e );

		static void fExportScriptInterface( tScriptVm& vm );

	private:
		enum tTargetChannel { cTargetChannelLeftLeg, cTargetChannelRightLeg, cTargetChannelCount };

		enum tLockMode 
		{ 
			cLockModeDetect,	//Don't penetrate the ground
			cLockModeLock,		//Lock to the ground when you get a chance. will transition to Detect if the lock can't be solved.
			cLockModeFree		//No detecting behavior at all
		};
		enum tLocks { cLockNone, cLockPrimary, cLockSecondary };
		
		// Setup data
		f32 mRayLength;  //Distance back from the target pos ray casted to the target, to find penetration
		f32 mFootHeight; //Height above terrain to place target (along terrain normal direction)
		f32 mExtraRay;   //Extra "reaching" distance the target will do, added to foot height
		tEntityTagMask mIgnoreTags;
		tEntityTagMask mRequireTags;

		// Running Data
		tLogic				*mOwner;
		b8					mIdle;   //Character is standing still
		b8					mIdleLastFrame;
		b8					mDoLock; // This is setup data, but here for packing
		b8					mResetLocks;
		f32					mResetLocksTimer;
		tFixedArray<u16,cTargetChannelCount> mLockMode;
		tFixedArray<u16,cTargetChannelCount> mLockedStatus;

		tFixedArray<Math::tPRSXformf,cTargetChannelCount>	mLockDownPos;   // This is the intended target output
		tFixedArray<Math::tPRSXformf,cTargetChannelCount>	mLockDownBlend; // This is the true blended target output
		tFixedArray<Math::tDampedFloat,cTargetChannelCount>	mBlendSrc;
		tFixedArray<Math::tDampedFloat,cTargetChannelCount>	mBlendDst;

		void fSetTarget( u32 channel, f32 ikBlend, const Math::tMat3f& target );
		Math::tMat3f fTarget( u32 channel ) const { return Math::tMat3f( mLockDownBlend[ channel ] ); }

		virtual void fCallbackMT( tIKCallbackArgs& args );
		virtual void fDebugData( std::stringstream& ss, u32 indentDepth, u32 channel ) const;

		void fSetDetectMode( u32 channel );
		void fSetLockOnFirstContactMode( u32 channel );
		void fSetFreeAfterLostContactMode( u32 channel );
	};

	void fExportScriptInterface( tScriptVm& vm );
	b32 fOverride( ); //true if ik should be overridden

} }

#endif //_tIK_
