#ifndef __tCharacterMoveAnimTrackFPS__
#define __tCharacterMoveAnimTrackFPS__
#include "tAnimTrackInputs.hpp"
#include "tKeyFrameAnimTrack.hpp"
#include "tBlendAnimTrack.hpp"

namespace Sig { namespace Anim
{

	struct tCharacterMoveFPSAnimDesc : public tAnimTrackDesc
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

		f32 mIdleBlendSpring;
		f32 mSprintBlendSpring;

		Anim::tTrackInput mInputSpeed;
		Anim::tTrackInput mInputSprinting;
		Anim::tTrackInput mInputMoveVec;
		Anim::tTrackInput mInputAimBlendMain;
		Anim::tTrackInput mInputAimBlendUpDown;

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
			, mIdleBlendSpring( 0.2f )
			, mSprintBlendSpring( 0.8f )
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};


	class base_export tCharacterMoveAnimTrackFPS : public tBlendAnimTrack
	{
		define_dynamic_cast( tCharacterMoveAnimTrackFPS, tBlendAnimTrack );
	public:
		tCharacterMoveAnimTrackFPS( tCharacterMoveAnimTrackFPS* prev,
			tBlendAnimTrack::tTrackList& tracksToBlend,
			const tCharacterMoveFPSAnimDesc& desc );

		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	
	private:
		const tCharacterMoveFPSAnimDesc mDesc;

		Math::tDampedFloat mIdleBlend;
		Math::tDampedFloat mSprintBlend;
		f32 mCurrentRevereseBlend; //1.0f if fully reversed, 0.0f if not reversed
		f32 mTargetReverseBlend; //1.0f if we intend to be reversing the left right anims
	};

} }

#endif//__tCharacterMoveAnimTrackFPS__

