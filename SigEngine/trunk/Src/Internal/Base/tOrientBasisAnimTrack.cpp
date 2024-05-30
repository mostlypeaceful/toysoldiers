#include "BasePch.hpp"
#include "tOrientBasisAnimTrack.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"

namespace Sig { namespace Anim
{
	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;
	}

	tOrientBasisAnimTrack::tOrientBasisAnimTrack( const tOrientBasisAnimDesc& desc )
		: tAnimTrack( desc )
		, mDesc( desc )
		, mErrorAngle( 0.f )
	{
	}

	void tOrientBasisAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		if( forceFullBlend || fBlendStrength( ) > cBlendEpsilon ) // only compute delta if we're "enough" blended-in
		{
			Math::tPRSXformf delta = Math::tPRSXformf::cZeroXform;

			Math::tQuatf deltaQuat = mDesc.mSource.fInverse( ) * mDesc.mTarget;
			Math::tAxisAnglef aa( deltaQuat );

			if( aa.mAngle > Math::cPi )
			{
				//reverse rotation but keep angle positive;
				aa.mAngle = Math::c2Pi - aa.mAngle;
				aa.mAxis *= -1.f;
			}
			
			mErrorAngle = aa.mAngle;

			if( mDesc.mAlwaysSmooth || fAbs( mErrorAngle ) > Math::fToRadians( 5.f ) )
				mErrorAngle = fClamp( mErrorAngle * dt * mDesc.mRotateSpeed, -mErrorAngle, mErrorAngle );

			aa.mAngle = mErrorAngle;
			deltaQuat = Math::tQuatf( aa );

			if( fAbs( mErrorAngle ) > 0.f )
			{
				delta.mRotation = deltaQuat - Math::tQuatf::cIdentity;
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
		static void fPushAnim( const tOrientBasisAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fRemoveTracksOfType<tOrientBasisAnimTrack>( );
			stack->fPushTrack( tAnimTrackPtr( NEW tOrientBasisAnimTrack( *desc ) ) );
		}
	}
	void tOrientBasisAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass< tOrientBasisAnimDesc, tAnimTrackDesc, Sqrat::DefaultAllocator<tOrientBasisAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("RotateSpeed"), &tOrientBasisAnimDesc::mRotateSpeed)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("OrientBasisTrack"), classDesc );
	}
} }
