#ifndef __tPitchBlendMuzzleAnimTrack__
#define __tPitchBlendMuzzleAnimTrack__
#include "tBlendAnimTrack.hpp"
#include "tKeyFrameAnimTrack.hpp"

namespace Sig
{
	class tWeapon;

	struct tPitchBlendMuzzleAnimDesc
	{
		tKeyFrameAnimation* mLowAnim;
		tKeyFrameAnimation* mHighAnim;
		f32					mLowAngle;
		f32					mHighAngle;
		f32					mBlendIn;
		f32					mBlendOut;
		f32					mTimeScale;
		f32					mBlendScale;
		tWeapon*			mWeapon;
		u32					mFlags;

		tPitchBlendMuzzleAnimDesc( )
			: mLowAnim( 0 )
			, mHighAnim( 0 )
			, mLowAngle( 0 )
			, mHighAngle( 0 )
			, mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
			, mWeapon( 0 )
			, mFlags( 0 )
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tPitchBlendMuzzleAnimTrack : public tBlendAnimTrack
	{
		define_dynamic_cast( tPitchBlendMuzzleAnimTrack, tBlendAnimTrack );
	public:
		tPitchBlendMuzzleAnimTrack( 
			tBlendAnimTrack::tTrackList& tracksToBlend,
			const tPitchBlendMuzzleAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	private:
		tWeapon* mWeapon;
		f32 mLowAngle;
		f32 mHighAngle;
	};
}

#endif//__tPitchBlendMuzzleAnimTrack__

