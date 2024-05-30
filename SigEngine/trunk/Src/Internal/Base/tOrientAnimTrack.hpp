#ifndef __tOrientAnimTrack__
#define __tOrientAnimTrack__
#include "tAnimTrack.hpp"

namespace Sig { namespace Anim
{
	struct tOrientAnimDesc : public tAnimTrackDesc
	{
		Math::tVec3f	mSourceVector;
		Math::tVec3f	mTargetVector;
		f32				mRotateSpeed;
		b32				mOrientSecondVector;
		b32				mAlwaysSmooth;

		tOrientAnimDesc( f32 rotateSpeed = 1.f, b32 orientSecondVector = false )
			: mRotateSpeed( rotateSpeed )
			, mOrientSecondVector( orientSecondVector )
			, mAlwaysSmooth( false )
		{
			mFlags |= tAnimTrack::cFlagPartial;
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
		tOrientAnimDesc mDesc;

	private:
		f32				mErrorAngle;

	public:
		tOrientAnimTrack( const tOrientAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
		f32 fGetErrorAngle( ) const { return mErrorAngle; }
	};
} }

#endif//__tOrientAnimTrack__

