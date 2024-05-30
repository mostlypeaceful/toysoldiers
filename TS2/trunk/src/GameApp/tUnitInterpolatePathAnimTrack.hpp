#ifndef __tUnitInterpolatePathAnimTrack__
#define __tUnitInterpolatePathAnimTrack__
#include "tOrientAnimTrack.hpp"
#include "tUnitPath.hpp"

namespace Sig
{
	struct tUnitInterpolatePathAnimDesc
	{
		f32			mBlendIn;
		f32			mBlendOut;
		f32			mTimeScale;
		f32			mBlendScale;
		tUnitPath*	mUnitPath;

		tUnitInterpolatePathAnimDesc( )
			: mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tUnitInterpolatePathAnimTrack : public tAnimTrack
	{
		define_dynamic_cast( tUnitInterpolatePathAnimTrack, tAnimTrack );
	private:
		tUnitPath*	mUnitPath;
		Math::tVec3f mOffset;
		b32			mFirstTick;

	public:
		tUnitInterpolatePathAnimTrack( const tUnitInterpolatePathAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	};
}

#endif//__tUnitInterpolatePathAnimTrack__

