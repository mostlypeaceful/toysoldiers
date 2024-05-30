#include "BasePch.hpp"
#include "tOrientAnimTrack.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"

namespace Sig { namespace Anim
{
	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;
	}

	tOrientAnimTrack::tOrientAnimTrack( const tOrientAnimDesc& desc )
		: tAnimTrack( desc)
		, mDesc( desc )
		, mErrorAngle( 0.f )
	{
	}

	void tOrientAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		if( forceFullBlend || fBlendStrength( ) > cBlendEpsilon ) // only compute delta if we're "enough" blended-in
		{
			Math::tPRSXformf delta = Math::tPRSXformf::cZeroXform;

			// Since we are always rotating about the y axis, project these vectors to the XZ plane to try and get a better angle.
			mDesc.mTargetVector.y = 0;
			mDesc.mSourceVector.y = 0;

			const f32 dist = mDesc.mTargetVector.fLength( );
			
			if( dist > 0 )	mDesc.mTargetVector /= dist;
			else			mDesc.mTargetVector = Math::tVec3f::cZAxis;

			mDesc.mSourceVector.fNormalizeSafe( Math::tVec3f::cZAxis );
			
			mErrorAngle = Math::fAcos( mDesc.mTargetVector.fDot( mDesc.mSourceVector ) );

			Math::tVec3f cross = mDesc.mSourceVector.fCross( mDesc.mTargetVector );

			if( mDesc.mAlwaysSmooth || fAbs( mErrorAngle ) > Math::fToRadians( 5.f ) || dist > 2.f )
				mErrorAngle = fClamp( mErrorAngle * dt * mDesc.mRotateSpeed, -mErrorAngle, mErrorAngle );

			if( cross.y < 0.f )
				mErrorAngle *= -1.f;

			if( fAbs( mErrorAngle ) > 0.f )
			{
				Math::tQuatf r( Math::tAxisAnglef( Math::tVec3f::cYAxis, mErrorAngle ) );
				delta.mRotation = r - Math::tQuatf::cIdentity;
			}

			if( forceFullBlend )	refFrameDelta.mRotation = delta.mRotation;
			else					refFrameDelta.fBlendLerpR( delta, fBlendStrength( ) );
		}
	}
} }


namespace Sig { namespace Anim
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
		Sqrat::DerivedClass< tOrientAnimDesc, tAnimTrackDesc, Sqrat::DefaultAllocator<tOrientAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("SourceVector"), &tOrientAnimDesc::mSourceVector)
			.Var(_SC("TargetVector"), &tOrientAnimDesc::mTargetVector)
			.Var(_SC("RotateSpeed"),  &tOrientAnimDesc::mRotateSpeed)
			.Var(_SC("AlwaysSmooth"), &tOrientAnimDesc::mAlwaysSmooth)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("OrientTrack"), classDesc );
	}
} }
