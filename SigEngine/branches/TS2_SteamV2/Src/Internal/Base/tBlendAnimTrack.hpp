#ifndef __tBlendAnimTrack__
#define __tBlendAnimTrack__
#include "tAnimTrack.hpp"

namespace Sig
{
	class base_export tBlendAnimTrack : public tAnimTrack
	{
		define_dynamic_cast( tBlendAnimTrack, tAnimTrack );
	public:
		typedef tDynamicArray<tAnimTrackPtr> tTrackList;
	protected:
		tTrackList mSubTracks;
	public:
		tBlendAnimTrack( 
			tTrackList& tracksToBlend,
			f32 blendIn, 
			f32 blendOut, 
			f32 timeScale = 1.f, 
			f32 blendScale = 1.f,
			f32 minTime = 0.f, 
			f32 maxTime = -1.f, 
			u32 flags = 0 );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
		virtual void fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend );
		virtual void fEvaluateAdditive( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend );
		const tTrackList& fSubTracks( ) const { return mSubTracks; }
		virtual void fOnPushed( tAnimatedSkeleton & skeleton );

		if_devmenu( virtual void fDebugTrackData( std::stringstream& ss, u32 indentDepth ) const );
	private:
		void fEvaluateInternal( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend, b32 additive );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tBlendAnimTrack );
}

#endif//__tBlendAnimTrack__

