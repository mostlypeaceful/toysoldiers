#ifndef __tOrientBasisAnimTrack__
#define __tOrientBasisAnimTrack__
#include "tAnimTrack.hpp"

namespace Sig { namespace Anim
{
	struct tOrientBasisAnimDesc : public tAnimTrackDesc
	{
		Math::tQuatf	mSource;
		Math::tQuatf	mTarget;
		f32				mRotateSpeed;
		b32				mAlwaysSmooth;

		tOrientBasisAnimDesc( f32 rotateSpeed = 1.f )
			: mRotateSpeed( rotateSpeed )
			, mAlwaysSmooth( false )
		{
			mFlags |= tAnimTrack::cFlagPartial;
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
		tOrientBasisAnimDesc mDesc;

	private:
		f32					 mErrorAngle;

	public:
		tOrientBasisAnimTrack( const tOrientBasisAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
		f32 fGetErrorAngle( ) const { return mErrorAngle; }
	};
} }

#endif//__tOrientBasisAnimTrack__

