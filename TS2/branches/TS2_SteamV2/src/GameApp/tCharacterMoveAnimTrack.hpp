#ifndef __tCharacterMoveAnimTrack__
#define __tCharacterMoveAnimTrack__
#include "tBlendAnimTrack.hpp"
#include "tKeyFrameAnimTrack.hpp"
#include "Math\tDamped.hpp"

namespace Sig
{
	class tUserControllableCharacterLogic;

	struct tCharacterMoveAnimDesc
	{
		tKeyFrameAnimation* mWalk;
		tKeyFrameAnimation* mRun;
		tKeyFrameAnimation* mIdle;

		f32			mBlendIn;
		f32			mBlendOut;
		f32			mTimeScale;
		f32			mBlendScale;
		tUserControllableCharacterLogic* mCharacter;

		tCharacterMoveAnimDesc( )
			: mWalk( NULL )
			, mRun( NULL )
			, mIdle( NULL )
			, mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
			, mCharacter( NULL )
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tCharacterMoveAnimTrack : public tBlendAnimTrack
	{
		define_dynamic_cast( tCharacterMoveAnimTrack, tBlendAnimTrack );
	private:
		tUserControllableCharacterLogic* mCharacter;
		Math::tDampedFloat mIdleBlend;
		Math::tDampedFloat mWalkBlend;

		f32 mWalkToRunTimeRatio;
		b32 mFirstTick;

	public:
		tCharacterMoveAnimTrack( 
			tBlendAnimTrack::tTrackList& tracksToBlend,
			const tCharacterMoveAnimDesc& desc );

		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	};
}

#endif//__tCharacterMoveAnimTrack__

