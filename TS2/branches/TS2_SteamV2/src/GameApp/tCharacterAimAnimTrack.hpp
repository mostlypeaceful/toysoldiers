#ifndef __tCharacterAimAnimTrack__
#define __tCharacterAimAnimTrack__
#include "tBlendAnimTrack.hpp"
#include "tKeyFrameAnimTrack.hpp"

namespace Sig
{
	class tUserControllableCharacterLogic;

	struct tCharacterAimAnimDesc
	{
		tKeyFrameAnimation* mHigh;
		tKeyFrameAnimation* mLow;

		f32			mBlendIn;
		f32			mBlendOut;
		f32			mTimeScale;
		f32			mBlendScale;
		tUserControllableCharacterLogic* mCharacter;
		tStringPtr  mTag;

		tCharacterAimAnimDesc( )
			: mHigh( NULL )
			, mLow( NULL )
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

	class base_export tCharacterAimAnimTrack : public tBlendAnimTrack
	{
		define_dynamic_cast( tCharacterAimAnimTrack, tBlendAnimTrack );
	private:
		tUserControllableCharacterLogic* mCharacter;

	public:
		tCharacterAimAnimTrack( tBlendAnimTrack::tTrackList& tracksToBlend, const tCharacterAimAnimDesc& desc );

		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	};
}

#endif//__tCharacterAimAnimTrack__

