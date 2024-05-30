#ifndef __tUnitPathAnimTrack__
#define __tUnitPathAnimTrack__
#include "tOrientAnimTrack.hpp"
#include "tUnitPath.hpp"

namespace Sig
{
	struct tUnitPathAnimDesc
	{
		f32			mBlendIn;
		f32			mBlendOut;
		f32			mTimeScale;
		f32			mBlendScale;
		tUnitPath*	mUnitPath;
		f32			mRotateSpeed;
		bool		mFireEventWhenAligned;

		tUnitPathAnimDesc( f32 blendIn = 0.2f, f32 blendOut = 0.2f, f32 timeScale = 1.f, f32 blendScale = 1.f
			, tUnitPath* unitPath = NULL, f32 rotateSpeed = 1.f, bool fireEventWhenAligned = false )
			: mBlendIn( blendIn )
			, mBlendOut( blendOut )
			, mTimeScale( timeScale )
			, mBlendScale( blendScale )
			, mUnitPath( unitPath )
			, mRotateSpeed( rotateSpeed )
			, mFireEventWhenAligned( fireEventWhenAligned )
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tUnitPathAnimTrack : public tOrientAnimTrack
	{
		define_dynamic_cast( tUnitPathAnimTrack, tOrientAnimTrack );
	protected:
		tUnitPath*	mUnitPath;
		b32			mFireEventWhenAligned;

	public:
		tUnitPathAnimTrack( const tUnitPathAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );

		virtual const Math::tVec3f fGetTarget( );
	};
}

#endif//__tUnitPathAnimTrack__

