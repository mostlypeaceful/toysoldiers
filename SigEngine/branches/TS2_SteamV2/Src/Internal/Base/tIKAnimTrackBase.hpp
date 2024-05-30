#ifndef __tIKAnimTrackBase__
#define __tIKAnimTrackBase__
#include "tAnimTrack.hpp"
#include "IK/tIK.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tSkeletonFile.hpp"
#include "tSceneGraph.hpp"

namespace Sig
{
	class tKeyFrameAnimation;
	class tSkeletonFile;

	struct tIKAnimTrackDesc
	{
		tKeyFrameAnimation* mAnim;
		tLogic* mOwner;
		u32 mIKChannel;
		f32 mBlendIn;
		f32 mBlendOut;
		f32 mTimeScale;
		f32 mBlendScale;
		f32 mMinTime;
		f32 mMaxTime;
		f32 mStartTime;
		u32 mFlags;
		tStringPtr mTag;
		IK::tIKCallback* mTargetCallback;
		u32	mTargetChannel;
		u32	mTargetGroup;
		f32 mIKBlendIn;
		f32 mIKBlendOut;
		f32 mIKBlendInCoolDown;
		bool mReuse;

		tIKAnimTrackDesc( )
			: mAnim( NULL )
			, mOwner( NULL )
			, mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
			, mMinTime(0.0f)
			, mMaxTime(-1.0f) // -1 indicates the track should use the natural length of the key frame animation
			, mStartTime(0.0f)
			, mFlags(0x0)
			, mTargetCallback( NULL )
			, mTargetChannel( ~0 )
			, mTargetGroup( ~0 )
			, mIKBlendIn(0.2f)
			, mIKBlendOut(0.2f)
			, mIKBlendInCoolDown( -1.f ) // negative disables
			, mReuse( true )
		{
		}
		virtual ~tIKAnimTrackDesc( ) {} 

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	//This is used so that all IK tracks can be identified as tIKAnimTrack via RTTI
	class base_export tIKAnimTrack
	{
		define_dynamic_cast_base( tIKAnimTrack );
	public: 
		tIKAnimTrack( u32 channel = ~0, u32 group = ~0 ) : mChannel( channel ), mGroup( group ) { }
		virtual ~tIKAnimTrack( ) { }

		u32 fChannel( ) const { return mChannel; }
		u32 fGroup( ) const { return mGroup; }

	protected:
		u32 mChannel;
		u32 mGroup;
	};

	template< typename tSolver >
	class base_export tIKAnimTrackBase : public tIKAnimTrack, public tAnimTrack
	{
		define_dynamic_cast_double( tIKAnimTrackBase, tIKAnimTrack, tAnimTrack );
	private:
		const tKeyFrameAnimation& mKeyFrameAnim;
		tLogic* mOwner;

		IK::tIKCallbackPtr mCallback;

		tSolver mSolver;
		u32 mParentBone;    //The bone above the ik chain, eg: pelvis
		u32 mFirstBone;     //The fire bone in the ik chain, eg: thigh
		u32 mEffectorBone;  //The target bone of the ik chain, eg: ankle

		f32 mIKBlendIn;		//Blend springs for when there is and isnt an IK result
		f32 mIKBlendOut;
		f32 mBlendInCoolDown; //Don't blend in again if we just blended out (timer > 0)
		f32 mBlendInCoolDownTimer;
		Math::tDampedFloat mIKBlend;

		b16 mFirstTick;
		u16 mResultLastFrame;
		tDynamicArray< Math::tPRSXformf > mOutput; 
		tDynamicArray< Math::tPRSXformf > mPallet; 
		f32 mDT; // store dt from fStepInternal for use in fPostAnimEvaluate

	public:
		tIKAnimTrackBase( const tIKAnimTrackDesc& desc, tSkeletonFile& skelFile )
			: tAnimTrack( desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale
						, desc.mMinTime, desc.mMaxTime < 0.f ? desc.mAnim->fLength( desc.mBlendOut == 0.f ) : desc.mMaxTime
						, desc.mStartTime, desc.mFlags | ( desc.mAnim->fPartial( ) ? cFlagPartial : 0 ) | cFlagPost )
			, tIKAnimTrack( desc.mTargetChannel, desc.mTargetGroup )
			, mKeyFrameAnim( *desc.mAnim )
			, mOwner( desc.mOwner )
			, mParentBone( 0 )
			, mFirstBone( ~0 )
			, mEffectorBone( ~0 )
			, mCallback( desc.mTargetCallback )
			, mIKBlendIn( desc.mIKBlendIn )
			, mIKBlendOut( desc.mIKBlendOut )
			, mIKBlend( 0.f )
			, mFirstTick( true )
			, mResultLastFrame( IK::cSolveResultFailed )
			, mBlendInCoolDown( desc.mIKBlendInCoolDown )
			, mBlendInCoolDownTimer( -1.f )
			, mDT( 1 / 30.0f )
		{
			tAnimTrack::fSetTag( desc.mTag );

			if( mKeyFrameAnim.mBones.fCount( ) == 0 )
			{
				log_warning( Log::cFlagAnimation, "No bones present in IK key frame anim." );
				return;
			}

			// Compute bind poses and initialize ik
			for( u32 i = 0; i < mKeyFrameAnim.mBones.fCount( ); ++i )
			{
				const tKeyFrameAnimation::tBone& kfaBone = mKeyFrameAnim.mBones[ i ];

				s32 bID = kfaBone.mMasterBoneIndex;
				s32 pID = kfaBone.mParentMasterBoneIndex;
				sigassert( pID != -1 );
				sigassert( pID == skelFile.fBone( bID ).mParent );

				const Math::tMat3f& parentInv = skelFile.fBone( pID ).mRefPoseInv;
				const Math::tMat3f& mine = skelFile.fBone( bID ).mRefPose;
				Math::tMat3f bindPose = parentInv * mine;

				if( i == 0 ) 
				{
					mParentBone = pID;
					mFirstBone = bID;
				}

				b32 isDummyLeaf = kfaBone.mIKPriority >= mKeyFrameAnim.mBones.fCount( );
				if( mEffectorBone == ~0 && (i == mKeyFrameAnim.mBones.fCount( ) - 1 || isDummyLeaf) )
				{
					if( isDummyLeaf )
						mEffectorBone = pID;
					else
						mEffectorBone = bID;
				}

				if( isDummyLeaf ) break;

				IK::tIKJoint joint( bindPose );
				joint.mWorldTransform = mine;
				joint.mLocalToBindPose = Math::tQuatf( skelFile.fBone( bID ).mRefPose );
				joint.mBindPoseToLocal = joint.mLocalToBindPose.fInverse( );

				joint.mID = i;
				joint.mAxesLimitsOrder = kfaBone.mIKAxisLimitsOrder;
				joint.mAxesLimitsLow = kfaBone.mIKAxisLimits.mMin;
				joint.mAxesLimitsHigh = kfaBone.mIKAxisLimits.mMax;

				//// some hard coded constraints
				//if( i == 0 )
				//{
				//	joint.mAxesLimitsOrder = cEulerOrderXYZ;
				//	joint.mAxesLimitsLow.x = -c2Pi; joint.mAxesLimitsHigh.x = c2Pi;
				//	joint.mAxesLimitsLow.y = -c2Pi; joint.mAxesLimitsHigh.y = c2Pi;
				//	joint.mAxesLimitsLow.z = -c2Pi; joint.mAxesLimitsHigh.z = c2Pi;
				//}
				//if( i == 1 )
				//{
				//	joint.mAxesLimitsOrder = cEulerOrderXYZ;
				//	joint.mAxesLimitsLow.x = 0; joint.mAxesLimitsHigh.x = 0;
				//	joint.mAxesLimitsLow.y = 0; joint.mAxesLimitsHigh.y = cPi;
				//	joint.mAxesLimitsLow.z = 0; joint.mAxesLimitsHigh.z = 0;
				//}
				//if( i == 2 )
				//{
				//	joint.mAxesLimitsOrder = cEulerOrderXYZ;
				//	joint.mAxesLimitsLow.x = -c2Pi; joint.mAxesLimitsHigh.x = c2Pi;
				//	joint.mAxesLimitsLow.y = 0; joint.mAxesLimitsHigh.y = 0;
				//	joint.mAxesLimitsLow.z = -c2Pi; joint.mAxesLimitsHigh.z = c2Pi;
				//}

				mSolver.fAddJoint( joint );
			}

			mPallet.fResize( mKeyFrameAnim.mBones.fCount( ) );
			mOutput.fResize( mKeyFrameAnim.mBones.fCount( ) );
			mOutput.fFill( Math::tPRSXformf::cIdentity );
		}

		virtual void fPostAnimEvaluate( tAnimatedSkeleton& animSkel )
		{
			if( mEffectorBone == ~0 ) return;

			tSkeletonFile* skelFile = animSkel.fSkeletonResource( )->fCast< tSkeletonFile >( );
			tAnimatedSkeleton::tMatrixPalette& pallette = animSkel.fMatrixPalette( );
			Math::tMat3f objectSpaceParentCur = pallette[ mParentBone ];
			Math::tMat3f objectSpaceTarget = pallette[ mEffectorBone ];
			Math::tMat3f objectSpaceTargetFirstBone = pallette[ mFirstBone ];
			u32 effectorJoint = mSolver.fJoints( ).fCount( ) - 1;
			u32 effectorJointFirstBone = 0;

			mSolver.fJoints( )[ effectorJoint ]			.fToObjectSpace( objectSpaceTarget );
			mSolver.fJoints( )[ effectorJointFirstBone ].fToObjectSpace( objectSpaceTargetFirstBone );

			IK::tIKCallbackArgs args( objectSpaceTarget, objectSpaceTargetFirstBone, mChannel );
			args.mSolveResult = mResultLastFrame;
			args.mIKBlendStrength = mIKBlend.fValue( );
			args.mDT = mDT;

			if( mCallback ) mCallback->fCallbackMT( args );

			objectSpaceTarget = args.mObjectSpaceEffectorTarget;
			mSolver.fJoints( )[ effectorJoint ].fToOriginalSpace( objectSpaceTarget );

			mBlendInCoolDownTimer -= mDT;

			IK::tSolveResult result = IK::cSolveResultFailed;
			if( args.mChanged/* && mBlendInCoolDownTimer <= 0.f*/ )
				result = mSolver.fSolve( objectSpaceParentCur, objectSpaceTarget, 0.1f, *mOwner );
			
			b32 solved = result != IK::cSolveResultFailed;
			b32 solvedLastFrame = mResultLastFrame != IK::cSolveResultFailed;

			mSolver.fRender( mOwner->fOwnerEntity( )->fObjectToWorld( ), objectSpaceParentCur, objectSpaceTarget, *mOwner );

			if( !solved && solvedLastFrame ) 
				mBlendInCoolDownTimer = mBlendInCoolDown;
			mResultLastFrame = result;

			f32 targetBlend = solved ? 1.0f : 0.f;
			mIKBlend.fSetBlends( solved ? mIKBlendIn : mIKBlendOut );
			mIKBlend.fStep( targetBlend, mDT );

			fBlendIKResult( animSkel );
		}

		void fBlendIKResult( tAnimatedSkeleton& animSkel )
		{
			tAnimatedSkeleton::tMatrixPalette& pallette = animSkel.fMatrixPalette( );
			f32 blend = IK::fOverride( ) ? 0.f : fClamp( mIKBlend.fValue( ), 0.f, 1.f );

			//const f32 cBlendEpsilon = 0.0001f;
			{
				s32 lastBoneIndex = mSolver.fJoints( ).fCount( )-1;

				// convert effected bones to delta
				for( s32 i = mKeyFrameAnim.mBones.fCount( )-1; i >= 0; --i )
				{
					const tKeyFrameAnimation::tBone& kfaBone = mKeyFrameAnim.mBones[ i ];
					mPallet[ i ] = Math::tPRSXformf( pallette[ kfaBone.mParentMasterBoneIndex ].fInverse( ) * pallette[ kfaBone.mMasterBoneIndex ] );

					// copy the animation into the output
					mOutput[ i ] = mPallet[ i ];
				}

				mFirstTick = false;

				// apply ik data to pallette as deltas
				for( u32 i = 0; i < mSolver.fJoints( ).fCount( ); ++i )
				{
					const tKeyFrameAnimation::tBone& kfaBone = mKeyFrameAnim.mBones[ i ];

					if( kfaBone.fIsAdditive( ) )
					{
						log_warning( Log::cFlagAnimation, "Additive bone ignored in CCDIKAnimTrack." );
						continue;
					}

					const IK::tIKJoint& joint = mSolver.fJoints( )[ i ];
					Math::tQuatf ikResult = Math::tQuatf( joint.mBindParentRelative ) * joint.mLocalDelta;
					Math::tVec3f bindT = joint.mBindParentRelative.fGetTranslation( );

					mOutput[ i ].fBlendNLerp( Math::tPRSXformf( bindT, ikResult, Math::tVec3f::cOnesVector ), blend );
				}

				for( s32 i = 0; i < (s32)mKeyFrameAnim.mBones.fCount( ); ++i )
				{
					// do stack blending
					mPallet[ i ].fBlendNLerp( mOutput[ i ], fBlendStrength( ) );

					// convert bones from parent relative to object space
					const tKeyFrameAnimation::tBone& kfaBone = mKeyFrameAnim.mBones[ i ];
					pallette[ kfaBone.mMasterBoneIndex ] = pallette[ kfaBone.mParentMasterBoneIndex ] * Math::tMat3f( mPallet[ i ] );
				}
			}
		}

		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign ) 
		{ mDT = dt; }

		virtual void fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend ) 
		{ /*do nothing*/ }

#ifdef sig_devmenu
		virtual void fDebugTrackName( std::stringstream& ss, u32 indentDepth ) const
		{
			fDebugIndent( ss, indentDepth );

			ss << mKeyFrameAnim.mName->fGetStringPtr( ).fCStr( ) << ".animl";
			if( fTag( ).fExists( ) )
				ss << " - " << fTag( ).fCStr( );
		}
		virtual void fDebugTrackData( std::stringstream& ss, u32 indentDepth ) const
		{
			if( mCallback )	
			{
				ss << " (blend = " << fBlendStrength( ) << ", ";

				mCallback->fDebugData( ss, indentDepth, mChannel );

				ss << ") " << std::endl;
			}
			else
				tAnimTrack::fDebugTrackData( ss, indentDepth );
		}
#endif//sig_devmenu	
	};

	class tIKCCDAnimTrackDesc : public tIKAnimTrackDesc { };
	class tIKLimbAnimTrackDesc : public tIKAnimTrackDesc { };

	typedef		tIKAnimTrackBase< IK::tIKChain >	tIKCCDAnimTrack;
	typedef		tIKAnimTrackBase< IK::tIKLimb >		tIKLimbAnimTrack;
}

#endif//__tIKAnimTrackBase__

