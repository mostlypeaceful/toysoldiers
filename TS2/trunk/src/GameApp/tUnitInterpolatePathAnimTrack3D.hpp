#ifndef __tUnitInterpolatePathAnimTrack3D__
#define __tUnitInterpolatePathAnimTrack3D__
#include "tOrientBasisAnimTrack.hpp"
#include "tUnitPath.hpp"

namespace Sig
{
	struct tUnitInterpolatePath3DAnimDesc
	{
		f32			mBlendIn;
		f32			mBlendOut;
		f32			mTimeScale;
		f32			mBlendScale;
		f32			mRotateSpeed;
		tUnitPath*	mUnitPath;
		f32			mBlendInSpring;

		tUnitInterpolatePath3DAnimDesc( )
			: mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
			, mRotateSpeed(1.0f)
			, mBlendInSpring(0.25f)
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tUnitInterpolatePathAnimTrack3D : public tOrientBasisAnimTrack
	{
		define_dynamic_cast( tUnitInterpolatePathAnimTrack3D, tOrientBasisAnimTrack );
	private:
		tUnitPath*	mUnitPath;
		Math::tVec3f mOffset;
		b32			mFirstTick;
		f32			mBlendInSpring;

	public:
		tUnitInterpolatePathAnimTrack3D( const tUnitInterpolatePath3DAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	};
}

#endif//__tUnitInterpolatePathAnimTrack3D__

