#include "BasePch.hpp"
#include "tRotateAnimTrack.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"

namespace Sig
{
	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;
	}

	tRotateAnimTrack::tRotateAnimTrack(
		f32 yaw, f32 pitch, f32 roll,
		f32 blendIn, 
		f32 blendOut, 
		f32 timeScale, 
		f32 blendScale )
		: tAnimTrack( blendIn, blendOut, timeScale, blendScale, 0.f, Math::cInfinity, 0.f, cFlagPartial )
		, mYaw( yaw )
		, mPitch( pitch )
		, mRoll( roll )
	{
	}

	void tRotateAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		if( forceFullBlend || fBlendStrength( ) > cBlendEpsilon ) // only compute delta if we're "enough" blended-in
		{
			Math::tPRSXformf delta = Math::tPRSXformf::cZeroXform;

			if( fAbs( mYaw ) > 0.f )
			{
				Math::tQuatf r( Math::tAxisAnglef( Math::tVec3f::cYAxis, mYaw * dt ) );
				delta.mR = r - Math::tQuatf::cIdentity;
			}

			if( forceFullBlend )	refFrameDelta = delta;
			else					refFrameDelta.fBlendLerp( delta, fBlendStrength( ) );
		}
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tRotateAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fPushTrack( tAnimTrackPtr( NEW tRotateAnimTrack( 
				desc->mYaw,
				desc->mPitch,
				desc->mRoll,
				desc->mBlendIn,
				desc->mBlendOut,
				desc->mTimeScale,
				desc->mBlendScale ) ) );
		}
	}
	void tRotateAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tRotateAnimDesc,Sqrat::DefaultAllocator<tRotateAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("BlendIn"), &tRotateAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tRotateAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tRotateAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tRotateAnimDesc::mBlendScale)
			.Var(_SC("Yaw"), &tRotateAnimDesc::mYaw)
			.Var(_SC("Pitch"), &tRotateAnimDesc::mPitch)
			.Var(_SC("Roll"), &tRotateAnimDesc::mRoll)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("RotateTrack"), classDesc );
	}
}
