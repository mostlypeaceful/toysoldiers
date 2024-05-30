#ifndef __tVehiclePassengerAnimTrack__
#define __tVehiclePassengerAnimTrack__
#include "tBlendAnimTrack.hpp"
#include "tKeyFrameAnimTrack.hpp"

namespace Sig
{
	class tVehiclePassengerLogic;

	struct tSprungMassRef
	{
		const Math::tVec3f* mAcc;

		tSprungMassRef( const Math::tVec3f* ptr = NULL )
			: mAcc( ptr )
		{ }
	};

	struct tVehiclePassengerAnimDesc
	{
		tKeyFrameAnimation* mFrontLeftAnim;
		tKeyFrameAnimation* mFrontRightAnim;
		tKeyFrameAnimation* mBackLeftAnim;
		tKeyFrameAnimation* mBackRightAnim;

		f32					mBlendIn;
		f32					mBlendOut;
		f32					mTimeScale;
		f32					mBlendScale;
		tSprungMassRef		mAcc;

		tVehiclePassengerAnimDesc( )
			: mFrontLeftAnim( NULL )
			, mFrontRightAnim( NULL )
			, mBackLeftAnim( NULL )
			, mBackRightAnim( NULL )

			, mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tVehiclePassengerAnimTrack : public tBlendAnimTrack
	{
		define_dynamic_cast( tVehiclePassengerAnimTrack, tBlendAnimTrack );
	public:
		tVehiclePassengerAnimTrack( 
			tBlendAnimTrack::tTrackList& tracksToBlend,
			f32 blendIn, 
			f32 blendOut, 
			f32 timeScale = 1.f, 
			f32 blendScale = 1.f,
			tSprungMassRef acc = tSprungMassRef( ) );

		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
	private:
		tSprungMassRef mAcc;
	};
}

#endif//__tVehiclePassengerAnimTrack__

