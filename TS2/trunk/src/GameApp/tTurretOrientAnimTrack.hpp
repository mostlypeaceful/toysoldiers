#ifndef __tTurretOrientAnimTrack__
#define __tTurretOrientAnimTrack__
#include "tOrientAnimTrack.hpp"

namespace Sig
{
	class tTurretLogic;

	struct tTurretOrientAnimDesc
	{
		f32			mBlendIn;
		f32			mBlendOut;
		f32			mTimeScale;
		f32			mBlendScale;
		tTurretLogic* mTurret;

		tTurretOrientAnimDesc( )
			: mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
			, mTurret( 0 )
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tTurretOrientAnimTrack : public tOrientAnimTrack
	{
		define_dynamic_cast( tTurretOrientAnimTrack, tOrientAnimTrack );
	public:
		tTurretOrientAnimTrack( const tTurretOrientAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	private:
		tTurretLogic* mTurret;
	};
}

#endif//__tTurretOrientAnimTrack__

