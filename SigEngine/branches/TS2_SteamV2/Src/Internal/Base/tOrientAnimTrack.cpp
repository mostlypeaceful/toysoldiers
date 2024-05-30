#include "BasePch.hpp"
#include "tOrientAnimTrack.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"

namespace Sig
{
	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;
	}

	tOrientAnimTrack::tOrientAnimTrack( const tOrientAnimDesc& desc )
		: tAnimTrack( desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, 0.f, Math::cInfinity, 0.f, cFlagPartial )
		, mSourceVector( desc.mSourceVector )
		, mTargetVector( desc.mTargetVector )
		, mAlwaysSmooth( desc.mAlwaysSmooth )
		, mRotateSpeed( desc.mRotateSpeed )
		, mErrorAngle( 0.f )
	{
	}

	void tOrientAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		if( forceFullBlend || fBlendStrength( ) > cBlendEpsilon ) // only compute delta if we're "enough" blended-in
		{
			Math::tPRSXformf delta = Math::tPRSXformf::cZeroXform;

			// Since we are always rotating about the y axis, project these vectors to the XZ plane to try and get a better angle.
			mTargetVector.y = 0;
			mSourceVector.y = 0;

			const f32 dist = mTargetVector.fLength( );
			
			if( dist > 0 )	mTargetVector /= dist;
			else			mTargetVector = Math::tVec3f::cZAxis;

			mSourceVector.fNormalizeSafe( Math::tVec3f::cZAxis );
			
			mErrorAngle = Math::fAcos( mTargetVector.fDot( mSourceVector ) );

			Math::tVec3f cross = mSourceVector.fCross( mTargetVector );

			if( mAlwaysSmooth || fAbs( mErrorAngle ) > Math::fToRadians( 5.f ) || dist > 2.f )
				mErrorAngle = fClamp( mErrorAngle * dt * mRotateSpeed, -mErrorAngle, mErrorAngle );

			if( cross.y < 0.f )
				mErrorAngle *= -1.f;

			if( fAbs( mErrorAngle ) > 0.f )
			{
				Math::tQuatf r( Math::tAxisAnglef( Math::tVec3f::cYAxis, mErrorAngle ) );
				delta.mR = r - Math::tQuatf::cIdentity;
			}

			if( forceFullBlend )	refFrameDelta.mR = delta.mR;
			else					refFrameDelta.fBlendLerpR( delta, fBlendStrength( ) );
		}
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tOrientAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fRemoveTracksOfType<tOrientAnimTrack>( );
			stack->fPushTrack( tAnimTrackPtr( NEW tOrientAnimTrack( *desc ) ) );
		}
	}
	void tOrientAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tOrientAnimDesc,Sqrat::DefaultAllocator<tOrientAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("BlendIn"), &tOrientAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tOrientAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tOrientAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tOrientAnimDesc::mBlendScale)
			.Var(_SC("SourceVector"), &tOrientAnimDesc::mSourceVector)
			.Var(_SC("TargetVector"), &tOrientAnimDesc::mTargetVector)
			.Var(_SC("RotateSpeed"), &tOrientAnimDesc::mRotateSpeed)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("OrientTrack"), classDesc );
	}
}
