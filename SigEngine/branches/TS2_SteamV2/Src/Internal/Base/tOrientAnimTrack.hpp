#ifndef __tOrientAnimTrack__
#define __tOrientAnimTrack__
#include "tAnimTrack.hpp"

namespace Sig
{
	struct tOrientAnimDesc
	{
		f32				mBlendIn;
		f32				mBlendOut;
		f32				mTimeScale;
		f32				mBlendScale;
		Math::tVec3f	mSourceVector;
		Math::tVec3f	mTargetVector;
		f32				mRotateSpeed;
		b32				mAlwaysSmooth;
		b32				mOrientSecondVector;

		tOrientAnimDesc( f32 blendIn = 0.2f, f32 blendOut = 0.2f, f32 timeScale = 1.f, f32 blendScale = 1.f, f32 rotateSpeed = 1.f, b32 orientSecondVector = false )
			: mBlendIn( blendIn )
			, mBlendOut( blendOut )
			, mTimeScale( timeScale )
			, mBlendScale( blendScale )
			, mRotateSpeed( rotateSpeed )
			, mAlwaysSmooth( false )
			, mOrientSecondVector( orientSecondVector )
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	// Odds are, if you inherit from this track you're going to want to call: 
	//  stack->fRemoveTracksOfType<tOrientAnimTrack>( ); before you push your track
	class base_export tOrientAnimTrack : public tAnimTrack
	{
		define_dynamic_cast( tOrientAnimTrack, tAnimTrack );
	protected:
		Math::tVec3f	mSourceVector;
		Math::tVec3f	mTargetVector;
		b32				mAlwaysSmooth;
		f32				mRotateSpeed;

	private:
		f32				mErrorAngle;

	public:
		tOrientAnimTrack( const tOrientAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
		f32 fGetErrorAngle( ) const { return mErrorAngle; }
	};
}

#endif//__tOrientAnimTrack__

