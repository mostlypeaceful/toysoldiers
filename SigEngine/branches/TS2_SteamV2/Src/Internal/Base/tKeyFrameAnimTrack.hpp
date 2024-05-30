#ifndef __tKeyFrameAnimTrack__
#define __tKeyFrameAnimTrack__
#include "tAnimTrack.hpp"

namespace Sig
{
	class tKeyFrameAnimation;
	class tSkeletonMap;

	struct tKeyFrameAnimDesc
	{
		const tKeyFrameAnimation* mAnim;
		f32 mBlendIn;
		f32 mBlendOut;
		f32 mTimeScale;
		f32 mBlendScale;
		f32 mMinTime;
		f32 mMaxTime;
		f32 mStartTime;
		u32 mFlags;
		Math::tEulerAnglesf mEulerRotation;
		tStringPtr mTag;

		tKeyFrameAnimDesc( const tKeyFrameAnimation* anim = NULL, f32 blendIn = 0.2f, f32 blendOut = 0.2f
			, f32 timeScale = 1.f, f32 blendScale = 1.f, f32 minTime = 0.f, f32 maxTime = -1.f, f32 startTime = 0.f, u32 flags = 0x0 )
			: mAnim( anim )
			, mBlendIn( blendIn )
			, mBlendOut( blendOut )
			, mTimeScale( timeScale )
			, mBlendScale( blendScale )
			, mMinTime( minTime )
			, mMaxTime( maxTime ) // -1 indicates the track should use the natural length of the key frame animation
			, mStartTime( startTime )
			, mFlags( flags )
			, mEulerRotation( Math::tEulerAnglesf::cZeroAngles )
		{
		}

		f32  fScaledOneShotLength( ) const;

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tKeyFrameAnimTrack : public tAnimTrack
	{
		define_dynamic_cast( tKeyFrameAnimTrack, tAnimTrack );
	private:
		const tKeyFrameAnimation& mKeyFrameAnim;
		Math::tPRSXformf mLastRefFrameXform;
		Math::tEulerAnglesf mEulerRotation;
		const tSkeletonMap * mSkeletonMap;
	public:
		tKeyFrameAnimTrack( const tKeyFrameAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
		virtual void fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend );
		virtual void fEvaluateAdditive( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend );
		virtual void fOnPushed( tAnimatedSkeleton & skeleton );

		const tKeyFrameAnimation& fAnim( ) const { return mKeyFrameAnim; }

		static void fSetKeyFrameEventID( u32 id ) { cEventKeyFrame = id; }
		static u32 fEventKeyFrameID( ) { return cEventKeyFrame; }

		static f32 fCurrentTimeOfTrack( const tKeyFrameAnimation *find, tAnimatedSkeleton* stack );


	public: // debuggish
		if_devmenu( virtual void fDebugTrackName( std::stringstream& ss, u32 indentDepth ) const );

	private:
		void fFireEvents( tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );

		static u32 cEventKeyFrame;
	};
}

#endif//__tKeyFrameAnimTrack__

