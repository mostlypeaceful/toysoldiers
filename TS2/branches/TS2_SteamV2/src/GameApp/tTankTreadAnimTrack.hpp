#ifndef __tTankTreadAnimTrack__
#define __tTankTreadAnimTrack__
#include "tBlendAnimTrack.hpp"
#include "tKeyFrameAnimTrack.hpp"

namespace Sig
{
	class tWheeledVehicleLogic;

	struct tTankTreadAnimDesc
	{
		tKeyFrameAnimation* mLeftAnim;
		tKeyFrameAnimation* mRightAnim;
		f32					mBlendIn;
		f32					mBlendOut;
		f32					mTimeScale;
		f32					mBlendScale;
		tWheeledVehicleLogic* mVehicle;
		u32					mFlags;

		tTankTreadAnimDesc( )
			: mLeftAnim( 0 )
			, mRightAnim( 0 )
			, mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
			, mVehicle( 0 )
			, mFlags( 0 )
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tTankTreadAnimTrack : public tBlendAnimTrack
	{
		define_dynamic_cast( tTankTreadAnimTrack, tBlendAnimTrack );
	public:
		tTankTreadAnimTrack( 
			tBlendAnimTrack::tTrackList& tracksToBlend,
			const tTankTreadAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	private:
		tWheeledVehicleLogic* mVehicle;
	};
}

#endif//__tTankTreadAnimTrack__

