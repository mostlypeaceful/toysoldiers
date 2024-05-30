#ifndef __tRotateAnimTrack__
#define __tRotateAnimTrack__
#include "tAnimTrack.hpp"

namespace Sig
{
	struct tRotateAnimDesc
	{
		f32 mBlendIn;
		f32 mBlendOut;
		f32 mTimeScale;
		f32 mBlendScale;
		f32 mYaw;
		f32 mPitch;
		f32 mRoll;

		tRotateAnimDesc( )
			: mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
			, mYaw(0.f)
			, mPitch(0.f)
			, mRoll(0.f)
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tRotateAnimTrack : public tAnimTrack
	{
		define_dynamic_cast( tRotateAnimTrack, tAnimTrack );
	private:
		f32 mYaw, mPitch, mRoll;
	public:
		tRotateAnimTrack( 
			f32 yaw, f32 pitch, f32 roll,
			f32 blendIn, 
			f32 blendOut, 
			f32 timeScale = 1.0f,
			f32 blendScale = 1.0f );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	};
}

#endif//__tRotateAnimTrack__

