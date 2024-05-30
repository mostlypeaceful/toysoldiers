#ifndef __tCharacterMoveAnimTrackFPS__
#define __tCharacterMoveAnimTrackFPS__
#include "tBlendAnimTrack.hpp"
#include "tKeyFrameAnimTrack.hpp"
#include "Math\tDamped.hpp"

namespace Sig
{
	class tUserControllableCharacterLogic;

	struct tCharacterMoveFPSAnimDesc
	{
		tKeyFrameAnimation* mRunForward;
		tKeyFrameAnimation* mRunBackward;
		tKeyFrameAnimation* mRunLeft;
		tKeyFrameAnimation* mRunRight;
		tKeyFrameAnimation* mRunLeftBack;
		tKeyFrameAnimation* mRunRightBack;
		tKeyFrameAnimation* mIdleAimUp;
		tKeyFrameAnimation* mIdleAimDown;
		tKeyFrameAnimation* mRunAimUp;
		tKeyFrameAnimation* mRunAimDown;
		tKeyFrameAnimation* mIdle;
		tKeyFrameAnimation* mSprint;


		f32			mBlendIn;
		f32			mBlendOut;
		f32			mTimeScale;
		f32			mBlendScale;
		tUserControllableCharacterLogic* mCharacter;

		tCharacterMoveFPSAnimDesc( )
			: mRunForward( NULL )
			, mRunBackward( NULL )
			, mRunLeft( NULL )
			, mRunRight( NULL )
			, mRunLeftBack( NULL )
			, mRunRightBack( NULL )
			, mIdleAimUp( NULL )
			, mIdleAimDown( NULL )
			, mRunAimUp( NULL )
			, mRunAimDown( NULL )
			, mIdle( NULL )
			, mSprint( NULL )
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

	class base_export tCharacterMoveAnimTrackFPS : public tBlendAnimTrack
	{
		define_dynamic_cast( tCharacterMoveAnimTrackFPS, tBlendAnimTrack );
	private:
		tUserControllableCharacterLogic* mCharacter;
		Math::tDampedFloat mIdleBlend;
		Math::tDampedFloat mSprintBlend;

	public:
		tCharacterMoveAnimTrackFPS( tCharacterMoveAnimTrackFPS* prev,
			tBlendAnimTrack::tTrackList& tracksToBlend,
			f32 blendIn, 
			f32 blendOut, 
			f32 timeScale = 1.0f,
			f32 blendScale = 1.0f,
			tUserControllableCharacterLogic* character = NULL );

		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );

	private:
		f32 mCurrentRevereseBlend; //1.0f if fully reversed, 0.0f if not reversed
		f32 mTargetReverseBlend; //1.0f if we intend to be reversing the left right anims
	};
}

#endif//__tCharacterMoveAnimTrackFPS__

