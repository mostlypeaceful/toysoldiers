#include "BasePch.hpp"
#include "tKeyFrameAnimTrack.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"
#include "Logic/tAnimatable.hpp"
#include "tSkeletonFile.hpp"
#include "tAnimPackFile.hpp"

namespace Sig
{
	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;
		static const tSkeletonMap * cInvalidSkeletonMap = (const tSkeletonMap *)0xffffffff;
	}

	u32 tKeyFrameAnimTrack::cEventKeyFrame = ~0;

	devvar( bool, Debug_Anim_KeyFrameTrack_FixRotationIssue, true );


	f32  tKeyFrameAnimDesc::fScaledOneShotLength( ) const 
	{ 
		return mAnim ? (mAnim->mLengthOneShot / mTimeScale) : 0.f; 
	}

	tKeyFrameAnimTrack::tKeyFrameAnimTrack( const tKeyFrameAnimDesc& desc )
		: tAnimTrack( desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, desc.mMinTime, desc.mMaxTime < 0.f ? desc.mAnim->fLength( desc.mBlendOut == 0.f ) : desc.mMaxTime, 
			desc.mStartTime, desc.mFlags | ( desc.mAnim->fPartial( ) ? cFlagPartial : 0 ) )
		, mKeyFrameAnim( *desc.mAnim )
		, mEulerRotation( desc.mEulerRotation )
		, mSkeletonMap(0)
	{
		tAnimTrack::fSetTag( desc.mTag );

		if( mKeyFrameAnim.fContainsRefFrame( ) )
		{
			if( fCurrentTime( ) == 0.f )
				mKeyFrameAnim.mReferenceFrame.fFirstFrameXform( mLastRefFrameXform );
			else
				mKeyFrameAnim.fSampleBone( mLastRefFrameXform, fNormalizedTime( ), mKeyFrameAnim.mReferenceFrame );
		}
	}

	void tKeyFrameAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		fFireEvents( animSkel, forceFullBlend, dt, wrapSign );

		if( !mKeyFrameAnim.fContainsRefFrame( ) )
			return; // nothing to do if this animation doesn't contain the reference frame

		Math::tPRSXformf current;
		mKeyFrameAnim.fSampleBone( current, fNormalizedTime( ), mKeyFrameAnim.mReferenceFrame );

		if( forceFullBlend || fBlendStrength( ) > cBlendEpsilon ) // only compute delta if we're "enough" blended-in
		{
			Math::tPRSXformf delta;

			if( wrapSign > 0.f ) // wrapped forward
			{
				Math::tPRSXformf x0, x1;
				mKeyFrameAnim.mReferenceFrame.fFirstFrameXform( x0 );
				mKeyFrameAnim.mReferenceFrame.fLastFrameXform( x1 );
				delta = ( current - x0 ) + ( x1 - mLastRefFrameXform );
			}
			else if( wrapSign < 0.f ) // wrapped backward
			{
				Math::tPRSXformf x0, x1;
				mKeyFrameAnim.mReferenceFrame.fFirstFrameXform( x0 );
				mKeyFrameAnim.mReferenceFrame.fLastFrameXform( x1 );
				delta = ( current - x1 ) + ( x0 - mLastRefFrameXform );
			}
			else
			{
				delta = current - mLastRefFrameXform;

				if( Debug_Anim_KeyFrameTrack_FixRotationIssue )
				{
					// This fixes "the" rotation problem. For the most part.
					delta.mR = mLastRefFrameXform.mR.fInverse( ) * current.mR;
					delta.mR.fNormalize( );

					// Shortest way around
					Math::tAxisAnglef aa( delta.mR );
					if( aa.mAngle > Math::cPi )
					{
						aa.mAngle = -Math::c2Pi + aa.mAngle;
					}
					else if( aa.mAngle < -Math::cPi )
					{
						aa.mAngle = Math::c2Pi + aa.mAngle;
					}

					delta.mR = Math::tQuatf( aa );
					delta.mR -= Math::tQuatf::cIdentity;
				}
			}



			// correct the translation delta to account for the fact that 
			// the reference frame has been rotated by successive rotation deltas
			// i.e., the translation delta is stored in model space, but the actual
			// reference frame may be getting rotated each frame
			delta.mP = current.mR.fInverse( ).fRotate( delta.mP );

			if( forceFullBlend )	refFrameDelta = delta;
			else					refFrameDelta.fBlendLerp( delta, fBlendStrength( ) );

			//if( fMaxTime( ) > 0.f && !mEulerRotation.fIsZero( ) )
			//	refFrameDelta.mR = Math::tQuatf( mEulerRotation * ( 2.f * Math::cPi * dt / ( 180.f * fMaxTime( ) ) ) ); // ??? For some inexplicable reason, we have to multiply by 2
		}

		mLastRefFrameXform = current;
	}

	void tKeyFrameAnimTrack::fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend )
	{
		if( mSkeletonMap == cInvalidSkeletonMap )
			return; // Unable to map to target skeleton

		Math::tPRSXformf b, db;

		const f32 normalizedTime = fNormalizedTime( );

		if( forceFullBlend || fBlendStrength( ) >= (1.f-cBlendEpsilon) )
		{
			// this code path is functionally equivalent to the 'else' path, but is optimized
			// using the assumption that we don't have to blend this frame (i.e., overwrite values bcz we're at full)

			for( u32 i = 0; i < mKeyFrameAnim.mBones.fCount( ); ++i )
			{
				const tKeyFrameAnimation::tBone& kfaBone = mKeyFrameAnim.mBones[ i ];

				u32 tgtBoneIndex = kfaBone.mMasterBoneIndex;
				const Math::tPRSXformf * remapXform = 0;
				if( mSkeletonMap )
				{
					const tSkeletonMap::tBoneMap & boneMap = mSkeletonMap->mSrc2TgtBones[ kfaBone.mMasterBoneIndex ];
					tgtBoneIndex = boneMap.mTargetBoneIndex;
					if( boneMap.mSrc2TgtXformIndex != tSkeletonMap::cInvalidIndex )
						remapXform = &mSkeletonMap->mSrc2TgtXforms[ boneMap.mSrc2TgtXformIndex ];
				}

				if( kfaBone.fIsAdditive( ) )
				{
					// sample current frame's delta
					mKeyFrameAnim.fSampleBoneAdditive( db, normalizedTime, kfaBone );
					// blend additively with existing bone result
					result.mBoneResults[ tgtBoneIndex ].fBlendAdditive( db );
				}
				else
				{
					// sample the current frame and overwrite (no blend) the animation result
					mKeyFrameAnim.fSampleBone( result.mBoneResults[ tgtBoneIndex ], normalizedTime, kfaBone, remapXform );
				}
			}
		}
		else if( fBlendStrength( ) > cBlendEpsilon )
		{
			// this code path performs full blend using fBlendStrength( )

			for( u32 i = 0; i < mKeyFrameAnim.mBones.fCount( ); ++i )
			{
				const tKeyFrameAnimation::tBone& kfaBone = mKeyFrameAnim.mBones[ i ];

				u32 tgtBoneIndex = kfaBone.mMasterBoneIndex;
				const Math::tPRSXformf * remapXform = 0;
				if( mSkeletonMap )
				{
					const tSkeletonMap::tBoneMap & boneMap = mSkeletonMap->mSrc2TgtBones[ kfaBone.mMasterBoneIndex ];
					tgtBoneIndex = boneMap.mTargetBoneIndex;
					if( boneMap.mSrc2TgtXformIndex != tSkeletonMap::cInvalidIndex )
						remapXform = &mSkeletonMap->mSrc2TgtXforms[ boneMap.mSrc2TgtXformIndex ];
				}

				if( kfaBone.fIsAdditive( ) )
				{
					// sample current frame's delta
					mKeyFrameAnim.fSampleBoneAdditive( db, normalizedTime, kfaBone );
					// blend additively with existing bone result
					result.mBoneResults[ tgtBoneIndex ].fBlendAdditive( db * fBlendStrength( ) );
				}
				else
				{
					// sample the current frame
					mKeyFrameAnim.fSampleBone( b, normalizedTime, kfaBone, remapXform );
					// blend with animation result
					result.mBoneResults[ tgtBoneIndex ].fBlendNLerp( b, fBlendStrength( ) );
				}
			}
		}
	}

	void tKeyFrameAnimTrack::fEvaluateAdditive( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend )
	{
		if( mSkeletonMap == cInvalidSkeletonMap )
			return; // Unable to map to target skeleton

		Math::tPRSXformf b, db;

		const f32 normalizedTime = fNormalizedTime( );

		if( forceFullBlend || fBlendStrength( ) >= (1.f-cBlendEpsilon) )
		{
			// this code path is functionally equivalent to the 'else' path, but is optimized
			// using the assumption that we don't have to blend this frame (i.e., overwrite values bcz we're at full)

			for( u32 i = 0; i < mKeyFrameAnim.mBones.fCount( ); ++i )
			{
				const tKeyFrameAnimation::tBone& kfaBone = mKeyFrameAnim.mBones[ i ];
				if( kfaBone.fIsAdditive( ) )
				{
					// sample current frame's delta
					mKeyFrameAnim.fSampleBoneAdditive( db, normalizedTime, kfaBone );
					// blend additively with existing bone result
					result.mBoneResults[ kfaBone.mMasterBoneIndex ].fBlendAdditive( db );
				}
				else
				{
					// sample the current frame
					mKeyFrameAnim.fSampleBone( b, normalizedTime, kfaBone );
					// blend additively with existing bone result
					result.mBoneResults[ kfaBone.mMasterBoneIndex ].fBlendAdditive( b );
				}
			}
		}
		else if( fBlendStrength( ) > cBlendEpsilon )
		{
			// this code path performs full blend using fBlendStrength( )

			for( u32 i = 0; i < mKeyFrameAnim.mBones.fCount( ); ++i )
			{
				const tKeyFrameAnimation::tBone& kfaBone = mKeyFrameAnim.mBones[ i ];
				if( kfaBone.fIsAdditive( ) )
				{
					// sample current frame's delta
					mKeyFrameAnim.fSampleBoneAdditive( db, normalizedTime, kfaBone );
					// blend additively with existing bone result
					result.mBoneResults[ kfaBone.mMasterBoneIndex ].fBlendAdditive( db * fBlendStrength( ) );
				}
				else
				{
					// sample the current frame
					mKeyFrameAnim.fSampleBone( b, normalizedTime, kfaBone );
					// blend with animation result
					result.mBoneResults[ kfaBone.mMasterBoneIndex ].fBlendAdditive( b * fBlendStrength( ) );
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	void tKeyFrameAnimTrack::fOnPushed( tAnimatedSkeleton & skeleton )
	{
		tResourcePtr tgtRes = skeleton.fSkeletonResource( );
		tResourcePtr srcRes = mKeyFrameAnim.mPackFile->mSkeletonResource->fGetResourcePtr();

		// No need to map because we're from the same skeleton
		if (tgtRes == srcRes)
			return;

		const tSkeletonFile * tgtSkel = tgtRes->fCast<tSkeletonFile>( );
		mSkeletonMap = tgtSkel->fFindSkeletonMap( tStringPtr( srcRes->fGetPath( ).fCStr( ) ) );

		// If there's no map then set to the invalid map and log a warning
		if( !mSkeletonMap )
		{
			mSkeletonMap = cInvalidSkeletonMap;
			log_warning( Log::cFlagAnimation,  
				   "Cannot play key framed animation \"" 
				<< mKeyFrameAnim.mName->fGetStringPtr( ).fCStr( ) 
				<< "\" because skeleton \"" 
				<< tgtRes->fGetPath( ).fCStr( ) 
				<< "\" contains no mapping from the source skeleton \"" 
				<< srcRes->fGetPath( ).fCStr( ) << "\"" );
		}
	}

#ifdef sig_devmenu
	void tKeyFrameAnimTrack::fDebugTrackName( std::stringstream& ss, u32 indentDepth ) const
	{
		fDebugIndent( ss, indentDepth );
		ss << mKeyFrameAnim.mName->fGetStringPtr( ).fCStr( ) << ".animl";
		if( fTag( ).fExists( ) )
			ss << " - " << fTag( ).fCStr( );
	}
#endif//sig_devmenu

	void tKeyFrameAnimTrack::fFireEvents( tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		if( !forceFullBlend && fBlendStrength( ) <= cBlendEpsilon )
			return; // only report events if we're at least partially blended-in

		sigassert( cEventKeyFrame != ~0 && "Animation key frame event id not initalized." );

		if( wrapSign > 0.f || wrapSign < 0.f ) // wrapped forward or backward
		{
			for( u32 i = 0; i < mKeyFrameAnim.mEvents.fCount( ); ++i )
			{
				const f32 t = mKeyFrameAnim.mEvents[ i ].mTime;
				if( ( t >= fPrevTime( ) && t < fMaxTime( ) ) || ( t >= 0.f && t < fCurrentTime( ) ) )
					animSkel.fQueueEvent( tKeyFrameEvent( t, fBlendStrength( ), cEventKeyFrame, mKeyFrameAnim.mEvents[ i ].mEventTypeCppValue, mKeyFrameAnim.mEvents[ i ].mTag->fGetStringPtr( ) ) );
			}
		}
		else
		{
			for( u32 i = 0; i < mKeyFrameAnim.mEvents.fCount( ); ++i )
			{
				const f32 t = mKeyFrameAnim.mEvents[ i ].mTime;
				if( t >= fPrevTime( ) && t < fCurrentTime( ) )
					animSkel.fQueueEvent( tKeyFrameEvent( t, fBlendStrength( ), cEventKeyFrame, mKeyFrameAnim.mEvents[ i ].mEventTypeCppValue, mKeyFrameAnim.mEvents[ i ].mTag->fGetStringPtr( ) ) );
			}
		}
	}

	f32 tKeyFrameAnimTrack::fCurrentTimeOfTrack( const tKeyFrameAnimation *find, tAnimatedSkeleton* stack )
	{
		for( s32 i = stack->fTrackCount( ) - 1; i >= 0 ; --i )
		{
			tKeyFrameAnimTrack *track = stack->fTrack( i ).fDynamicCast< tKeyFrameAnimTrack >( );
			if( track && &track->fAnim( ) == find )
				return track->fCurrentTime( );
		}

		return 0.f;
	}

}


namespace Sig
{
	namespace
	{

		void fSetStartTime( tKeyFrameAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			if( fTestBits( desc->mFlags, tAnimTrack::cFlagResumeTime ) )
			{
				desc->mStartTime = tKeyFrameAnimTrack::fCurrentTimeOfTrack( desc->mAnim, stack );
			}
		}

		static void fPushAnim( tKeyFrameAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			if( !desc->mAnim )
				return;

			fSetStartTime( desc, stack );

			stack->fPushTrack( tAnimTrackPtr( NEW tKeyFrameAnimTrack( *desc ) ) );
		}

		static void fPushAnimBeforeTag( tKeyFrameAnimDesc* desc, tAnimatedSkeleton* stack, const tStringPtr& tag )
		{
			if( !desc->mAnim )
				return;

			fSetStartTime( desc, stack );

			stack->fPushTrackBeforeTag( tAnimTrackPtr( NEW tKeyFrameAnimTrack( *desc ) ), tag );
		}
		static tKeyFrameAnimDesc fMakeLooping( const tKeyFrameAnimation* anim )
		{
			tKeyFrameAnimDesc desc;
			desc.mAnim = anim;
			desc.mBlendIn = 0.2f;
			desc.mBlendOut = 0.0f;
			return desc;
		}
		static tKeyFrameAnimDesc fMakeOneShot( const tKeyFrameAnimation* anim )
		{
			tKeyFrameAnimDesc desc;
			desc.mAnim = anim;
			desc.mBlendIn = 0.2f;
			desc.mBlendOut = 0.0f; // yes, 0.0f! bcz the expectation is a new state will override
			desc.mFlags |= tAnimTrack::cFlagClampTime;
			return desc;
		}
		static tKeyFrameAnimDesc fMakeOneShotPartial( const tKeyFrameAnimation* anim )
		{
			tKeyFrameAnimDesc desc;
			desc.mAnim = anim;
			desc.mBlendIn = 0.2f;
			desc.mBlendOut = 0.2f;
			desc.mFlags |= tAnimTrack::cFlagClampTime;
			return desc;
		}
	}
	void tKeyFrameAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tKeyFrameAnimDesc,Sqrat::DefaultAllocator<tKeyFrameAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc(_SC("MakeLooping"),				&fMakeLooping)
			.StaticFunc(_SC("MakeOneShot"),				&fMakeOneShot)
			.StaticFunc(_SC("MakeOneShotPartial"),		&fMakeOneShotPartial)
			.StaticFunc(_SC("CurrentTimeOfTrack"),		&tKeyFrameAnimTrack::fCurrentTimeOfTrack)
			.GlobalFunc(_SC("Push"),					&fPushAnim)
			.GlobalFunc(_SC("PushBeforeTag"),			&fPushAnimBeforeTag)
			.Var(_SC("Anim"),							(Sig::tKeyFrameAnimation *Sig::tKeyFrameAnimDesc::*)&tKeyFrameAnimDesc::mAnim)
			.Var(_SC("BlendIn"),						&tKeyFrameAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"),						&tKeyFrameAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"),						&tKeyFrameAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"),						&tKeyFrameAnimDesc::mBlendScale)
			.Var(_SC("MinTime"),						&tKeyFrameAnimDesc::mMinTime)
			.Var(_SC("MaxTime"),						&tKeyFrameAnimDesc::mMaxTime)
			.Var(_SC("StartTime"),						&tKeyFrameAnimDesc::mStartTime)
			.Var(_SC("Flags"),							&tKeyFrameAnimDesc::mFlags)
			.Var(_SC("EulerRotation"),					&tKeyFrameAnimDesc::mEulerRotation)
			.Var(_SC("Tag"),							&tKeyFrameAnimDesc::mTag)
			.Prop(_SC("ScaledOneShotLength"),			&tKeyFrameAnimDesc::fScaledOneShotLength)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("KeyFrameTrack"), classDesc );

		vm.fConstTable( ).Const( "ANIM_TRACK_PARTIAL", ( int )tAnimTrack::cFlagPartial );
		vm.fConstTable( ).Const( "ANIM_TRACK_CLAMP_TIME", ( int )tAnimTrack::cFlagClampTime );
		vm.fConstTable( ).Const( "ANIM_TRACK_RESUME_TIME", ( int )tAnimTrack::cFlagResumeTime );
	}
}
