#ifndef __tPitchBlendAnimTrack__
#define __tPitchBlendAnimTrack__
#include "tBlendAnimTrack.hpp"
#include "tKeyFrameAnimTrack.hpp"

namespace Sig
{
	class tTurretLogic;

	struct tPitchBlendAnimDesc
	{
		tKeyFrameAnimation* mLowAnim;
		tKeyFrameAnimation* mHighAnim;
		f32					mBlendIn;
		f32					mBlendOut;
		f32					mTimeScale;
		f32					mBlendScale;
		tTurretLogic*		mTurret;

		tPitchBlendAnimDesc( )
			: mLowAnim( 0 )
			, mHighAnim( 0 )
			, mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
			, mTurret( 0 )
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tPitchBlendAnimTrack : public tBlendAnimTrack
	{
		define_dynamic_cast( tPitchBlendAnimTrack, tBlendAnimTrack );
	public:
		tPitchBlendAnimTrack( 
			tBlendAnimTrack::tTrackList& tracksToBlend,
			f32 blendIn, 
			f32 blendOut, 
			f32 timeScale = 1.f, 
			f32 blendScale = 1.f,
			tTurretLogic* turret = 0 );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	private:
		tTurretLogic* mTurret;
	};
}

#endif//__tPitchBlendAnimTrack__

