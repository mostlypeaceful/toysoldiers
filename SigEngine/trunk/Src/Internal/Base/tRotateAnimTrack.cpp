#include "BasePch.hpp"
#include "tRotateAnimTrack.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"

namespace Sig { namespace Anim
{
	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;
	}

	tRotateAnimTrack::tRotateAnimTrack( const tRotateAnimDesc& desc )
		: tAnimTrack( desc )
		, mDesc( desc )
	{
	}

	void tRotateAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		if( forceFullBlend || fBlendStrength( ) > cBlendEpsilon ) // only compute delta if we're "enough" blended-in
		{
			Math::tPRSXformf delta = Math::tPRSXformf::cZeroXform;

			if( fAbs( mDesc.mYaw ) > 0.f )
			{
				Math::tQuatf r( Math::tAxisAnglef( Math::tVec3f::cYAxis, mDesc.mYaw * dt ) );
				delta.mRotation = r - Math::tQuatf::cIdentity;
			}

			if( forceFullBlend )	refFrameDelta = delta;
			else					refFrameDelta.fBlendLerp( delta, fBlendStrength( ) );
		}
	}
} }


namespace Sig { namespace Anim
{
	namespace
	{
		static void fPushAnim( const tRotateAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fPushTrack( tAnimTrackPtr( NEW tRotateAnimTrack( *desc ) ) );
		}
	}
	void tRotateAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass< tRotateAnimDesc, tAnimTrackDesc, Sqrat::DefaultAllocator<tRotateAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("Yaw"), &tRotateAnimDesc::mYaw)
			.Var(_SC("Pitch"), &tRotateAnimDesc::mPitch)
			.Var(_SC("Roll"), &tRotateAnimDesc::mRoll)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("RotateTrack"), classDesc );
	}
} }
