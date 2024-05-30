#include "BasePch.hpp"
#include "tKeyFrameAnimation.hpp"

namespace Sig
{
	///
	/// \section tKeyFrameAnimation::tBone
	///

	tKeyFrameAnimation::tBone::tBone( )
		: mMasterBoneIndex( 0 )
		, mFlags( 0 )
		, mName( 0 )
		, mPMin( 0.f )
		, mPMax( 0.f )
		, mSMin( 0.f )
		, mSMax( 0.f )
		, mParentMasterBoneIndex( ~0 )
		, mIKPriority( 0 )
		, mIKAxisLimits( Math::tAabbf::cZeroSized )
		, mIKAxisLimitsOrder( 0 )
	{
	}

	tKeyFrameAnimation::tBone::tBone( tNoOpTag )
		: mPositionFrameNums( cNoOpTag )
		, mPositionKeys( cNoOpTag )
		, mRotationFrameNums( cNoOpTag )
		, mRotationKeys( cNoOpTag )
		, mScaleFrameNums( cNoOpTag )
		, mScaleKeys( cNoOpTag )
		, mIKAxisLimits( cNoOpTag )
	{
	}

	tKeyFrameAnimation::tBone::~tBone( )
	{
	}


	///
	/// \section tKeyFrameAnimation
	///

	tKeyFrameAnimation::tKeyFrameAnimation( )
		: mName( 0 )
		, mFlags( 0 )
		, mFramesPerSecond( 0 )
		, mLengthOneShot( 0.f )
		, mLengthLooping( 0.f )
	{
	}

	tKeyFrameAnimation::tKeyFrameAnimation( tNoOpTag )
		: mBones( cNoOpTag )
		, mEvents( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	void tKeyFrameAnimation::fSampleBone( 
		Math::tPRSXformf& xformOut, 
		f32 timeInZeroToOne, 
		const tBone& bone ) const
	{
		tBracket bracket;

		if( bone.mPositionFrameNums.fCount( ) == 1 )
			xformOut.mP = fFromPositionKey( bone.mPositionKeys.fFront( ), bone.mPMin, bone.mPMax );
		else
		{
			fBracket( bracket, bone.mPositionFrameNums, timeInZeroToOne );
			const Math::tVec3f a = fFromPositionKey( bone.mPositionKeys[ bracket.mLeft  ], bone.mPMin, bone.mPMax );
			const Math::tVec3f b = fFromPositionKey( bone.mPositionKeys[ bracket.mRight ], bone.mPMin, bone.mPMax );
			xformOut.mP = Math::fLerp( a, b, bracket.mZeroToOne );
		}

		if( bone.mRotationFrameNums.fCount( ) == 1 )
			xformOut.mR = fFromRotationKey( bone.mRotationKeys.fFront( ) );
		else
		{
			fBracket( bracket, bone.mRotationFrameNums, timeInZeroToOne );
			const Math::tQuatf a = fFromRotationKey( bone.mRotationKeys[ bracket.mLeft  ] );
			const Math::tQuatf b = fFromRotationKey( bone.mRotationKeys[ bracket.mRight ] );
			xformOut.mR = Math::fNLerp( a, b, bracket.mZeroToOne );
			sigassert( !xformOut.mR.fIsNan( ) );
		}

		if( bone.mScaleFrameNums.fCount( ) == 1 )
			xformOut.mS = fFromScaleKey( bone.mScaleKeys.fFront( ), bone.mSMin, bone.mSMax );
		else
		{
			fBracket( bracket, bone.mScaleFrameNums, timeInZeroToOne );
			const Math::tVec3f a = fFromScaleKey( bone.mScaleKeys[ bracket.mLeft  ], bone.mSMin, bone.mSMax );
			const Math::tVec3f b = fFromScaleKey( bone.mScaleKeys[ bracket.mRight ], bone.mSMin, bone.mSMax );
			xformOut.mS = Math::fLerp( a, b, bracket.mZeroToOne );
		}
	}

	//------------------------------------------------------------------------------
	void tKeyFrameAnimation::fSampleBone( 
		Math::tPRSXformf& xformOut, 
		f32 timeInZeroToOne, 
		const tBone& bone, 
		const Math::tPRSXformf * remap ) const
	{
		fSampleBone(xformOut, timeInZeroToOne, bone);

		// Remap the bone to the target skeleton
		if( remap )
			xformOut += *remap;
	}

	void tKeyFrameAnimation::fSampleBoneAdditive( Math::tPRSXformf& xformOut, f32 timeInZeroToOne, const tBone& bone ) const
	{
		Math::tPRSXformf b0;

		// sample the first frame
		bone.fFirstFrameXform( b0 );

		// sample the current frame
		fSampleBone( xformOut, timeInZeroToOne, bone );

		// subtract off first frame to get delta (which is used for additive blending)
		xformOut -= b0;
	}

	void tKeyFrameAnimation::fBracket( tBracket& bracketOut, const tKeyFrameNumberArray& keyFrameNums, f32 timeInZeroToOne ) const
	{
		sigassert( keyFrameNums.fCount( ) >= 2 );
		sigassert( fInBounds( timeInZeroToOne, 0.f, 1.f ) );

		const u16* kfs = keyFrameNums.fBegin( );
		const u32 numKfs = keyFrameNums.fCount( );
		const f32 fFrameNum = timeInZeroToOne * ( f32 )keyFrameNums.fBack( );
		const u32 nFrameNum = ( u32 )( fFrameNum );

		u32 lb = 0, ub = keyFrameNums.fCount( ) - 1, mid;

		do
		{
			mid = ( lb + ub ) / 2;
			sigassert( mid + 1 < numKfs );

			const u32 i0 = kfs[ mid + 0 ];
			const u32 i1 = kfs[ mid + 1 ];

			if( i0 <= nFrameNum && nFrameNum < i1 ) // match
			{
				bracketOut.mLeft		= mid;
				bracketOut.mRight		= mid + 1;
				bracketOut.mZeroToOne	= f32( fFrameNum - i0 ) / f32( i1 - i0 );
				return;
			}
			else if( i0 > nFrameNum ) // too high
				ub = mid - 1;
			else//if( i0 < nFrameNum ) // too low
				lb = mid + 1;

		} while( lb <= ub );

		sigassert( !"shouldn't get here (outside the loop in tKeyFrameAnimation::fBracket)" );
	}
}



namespace Sig
{
	void tKeyFrameAnimation::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tKeyFrameAnimation, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("Length"),			&tKeyFrameAnimation::fOneShotLength)
			.Prop(_SC("OneShotLength"),		&tKeyFrameAnimation::fOneShotLength)
			.Prop(_SC("LoopingLength"),		&tKeyFrameAnimation::fLoopingLength)
			;
		vm.fNamespace(_SC("Anim")).Bind(_SC("KeyFrameAnim"), classDesc);
	}
}

