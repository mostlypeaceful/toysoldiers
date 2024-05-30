#include "BasePch.hpp"
#include "tOrientBasisAnimTrack.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"

namespace Sig
{
	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;
	}

	tOrientBasisAnimTrack::tOrientBasisAnimTrack( const tOrientBasisAnimDesc& desc )
		: tAnimTrack( desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, 0.f, Math::cInfinity, 0.f, cFlagPartial )
		, mSource( Math::tQuatf::cIdentity )
		, mTarget( Math::tQuatf::cIdentity )
		, mAlwaysSmooth( desc.mAlwaysSmooth )
		, mRotateSpeed( desc.mRotateSpeed )
		, mErrorAngle( 0.f )
	{
	}

	void tOrientBasisAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		if( forceFullBlend || fBlendStrength( ) > cBlendEpsilon ) // only compute delta if we're "enough" blended-in
		{
			Math::tPRSXformf delta = Math::tPRSXformf::cZeroXform;

			Math::tQuatf deltaQuat = mSource.fInverse( ) * mTarget;
			Math::tAxisAnglef aa( deltaQuat );

			if( aa.mAngle > Math::cPi )
			{
				//reverse rotation but keep angle positive;
				aa.mAngle = Math::c2Pi - aa.mAngle;
				aa.mAxis *= -1.f;
			}
			
			mErrorAngle = aa.mAngle;

			if( mAlwaysSmooth || fAbs( mErrorAngle ) > Math::fToRadians( 5.f ) )
				mErrorAngle = fClamp( mErrorAngle * dt * mRotateSpeed, -mErrorAngle, mErrorAngle );

			aa.mAngle = mErrorAngle;
			deltaQuat = Math::tQuatf( aa );

			if( fAbs( mErrorAngle ) > 0.f )
			{
				delta.mR = deltaQuat - Math::tQuatf::cIdentity;
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
		static void fPushAnim( const tOrientBasisAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fRemoveTracksOfType<tOrientBasisAnimTrack>( );
			stack->fPushTrack( tAnimTrackPtr( NEW tOrientBasisAnimTrack( *desc ) ) );
		}
	}
	void tOrientBasisAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tOrientBasisAnimDesc,Sqrat::DefaultAllocator<tOrientBasisAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("BlendIn"), &tOrientBasisAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tOrientBasisAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tOrientBasisAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tOrientBasisAnimDesc::mBlendScale)
			.Var(_SC("RotateSpeed"), &tOrientBasisAnimDesc::mRotateSpeed)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("OrientBasisTrack"), classDesc );
	}
}
