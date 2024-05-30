#ifndef __tRotateAnimTrack__
#define __tRotateAnimTrack__
#include "tAnimTrack.hpp"

namespace Sig { namespace Anim
{
	struct tRotateAnimDesc : public tAnimTrackDesc
	{
		f32 mYaw;
		f32 mPitch;
		f32 mRoll;

		tRotateAnimDesc( )
			: tAnimTrackDesc( )
			, mYaw( 0 )
			, mPitch( 0.f )
			, mRoll( 0.f )
		{
			mFlags |= tAnimTrack::cFlagPartial;
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tRotateAnimTrack : public tAnimTrack
	{
		define_dynamic_cast( tRotateAnimTrack, tAnimTrack );
	public:
		tRotateAnimTrack( const tRotateAnimDesc& desc );

		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );

	private:
		const tRotateAnimDesc mDesc;
	};
} }

#endif//__tRotateAnimTrack__

