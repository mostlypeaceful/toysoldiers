#ifndef __tBlendAnimTrack__
#define __tBlendAnimTrack__
#include "tAnimTrack.hpp"

namespace Sig { namespace Anim
{
	class base_export tBlendAnimTrack : public tAnimTrack
	{
		define_dynamic_cast( tBlendAnimTrack, tAnimTrack );
	public:
		typedef tGrowableArray<tAnimTrackPtr> tTrackList;
	protected:
		tTrackList mSubTracks;

	public:
		tBlendAnimTrack( tTrackList& tracksToBlend, const tAnimTrackDesc& desc );

		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
		virtual void fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend );
		
		const tTrackList& fSubTracks( ) const { return mSubTracks; }
		tTrackList& fSubTracks( ) { return mSubTracks; }
		
		virtual void fOnPushed( tAnimatedSkeleton & skeleton );
		virtual void fRestart( );

		if_devmenu( virtual b32 fVisible( ) const );
		if_devmenu( virtual void fDebugTrackName( std::stringstream& ss, u32 indentDepth ) const );
		if_devmenu( virtual void fDebugTrackData( std::stringstream& ss, u32 indentDepth ) const );
	
	private:
		void fEvaluateInternal( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend, b32 additive );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tBlendAnimTrack );
} }

#endif//__tBlendAnimTrack__

