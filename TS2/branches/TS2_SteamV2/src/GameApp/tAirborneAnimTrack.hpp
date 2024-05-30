#ifndef __tAirborneAnimTrack__
#define __tAirborneAnimTrack__
#include "tBlendAnimTrack.hpp"
#include "tKeyFrameAnimTrack.hpp"
#include "Math/tDamped.hpp"

namespace Sig
{
	class tAirborneLogic;

	struct tAirborneAnimDesc
	{
		tKeyFrameAnimation* mFastLeftAnim;
		tKeyFrameAnimation* mFastRightAnim;
		tKeyFrameAnimation* mSlowLeftAnim;
		tKeyFrameAnimation* mSlowRightAnim;
		f32					mBlendIn;
		f32					mBlendOut;
		f32					mTimeScale;
		f32					mBlendScale;
		tAirborneLogic*		mAirborne;

		tAirborneAnimDesc( )
			: mFastLeftAnim( 0 )
			, mFastRightAnim( 0 )
			, mSlowLeftAnim( 0 )
			, mSlowRightAnim( 0 )
			, mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
			, mAirborne( 0 )
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tAirborneAnimTrack : public tBlendAnimTrack
	{
		define_dynamic_cast( tAirborneAnimTrack, tBlendAnimTrack );
	public:
		tAirborneAnimTrack( 
			tBlendAnimTrack::tTrackList& tracksToBlend,
			const tAirborneAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	private:
		tAirborneLogic* mAirborne;
		Math::tDampedFloat mRollDamp;
	};
}

#endif//__tAirborneAnimTrack__

