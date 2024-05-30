#ifndef __tOrientBasisAnimTrack__
#define __tOrientBasisAnimTrack__
#include "tAnimTrack.hpp"

namespace Sig
{
	struct tOrientBasisAnimDesc
	{
		f32				mBlendIn;
		f32				mBlendOut;
		f32				mTimeScale;
		f32				mBlendScale;
		f32				mRotateSpeed;
		b32				mAlwaysSmooth;

		tOrientBasisAnimDesc( f32 blendIn = 0.2f, f32 blendOut = 0.2f, f32 timeScale = 1.f, f32 blendScale = 1.f, f32 rotateSpeed = 1.f )
			: mBlendIn( blendIn )
			, mBlendOut( blendOut )
			, mTimeScale( timeScale )
			, mBlendScale( blendScale )
			, mRotateSpeed( rotateSpeed )
			, mAlwaysSmooth( false )
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	// Odds are, if you inherit from this track you're going to want to call: 
	//  stack->fRemoveTracksOfType<tOrientBasisAnimTrack>( ); before you push your track
	class base_export tOrientBasisAnimTrack : public tAnimTrack
	{
		define_dynamic_cast( tOrientBasisAnimTrack, tAnimTrack );
	protected:
		Math::tQuatf	mSource;
		Math::tQuatf	mTarget;
		b32				mAlwaysSmooth;
		f32				mRotateSpeed;

	private:
		f32				mErrorAngle;

	public:
		tOrientBasisAnimTrack( const tOrientBasisAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
		f32 fGetErrorAngle( ) const { return mErrorAngle; }
	};
}

#endif//__tOrientBasisAnimTrack__

