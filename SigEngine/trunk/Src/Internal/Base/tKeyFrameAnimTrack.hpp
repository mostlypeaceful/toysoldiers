#ifndef __tKeyFrameAnimTrack__
#define __tKeyFrameAnimTrack__
#include "tAnimTrack.hpp"

namespace Sig 
{ 
	class tKeyFrameAnimation;
	class tSkeletonMap;
	
namespace Anim
{

	struct tKeyFrameAnimDesc : public tAnimTrackDesc
	{
		const tKeyFrameAnimation* mAnim;

		tKeyFrameAnimDesc( const tKeyFrameAnimation* anim = NULL )
			: mAnim( anim )
		{ }

		tKeyFrameAnimDesc( const tKeyFrameAnimation* anim, const tAnimTrackDesc& desc )
			: tAnimTrackDesc( desc )
			, mAnim( anim )
		{ }

		f32  fScaledOneShotLength( ) const;

		tKeyFrameAnimDesc& fPrepare( );

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};


	class base_export tKeyFrameAnimTrack : public tAnimTrack
	{
		define_dynamic_cast( tKeyFrameAnimTrack, tAnimTrack );
	public:
		tKeyFrameAnimTrack( tKeyFrameAnimDesc& desc );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
		virtual void fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend );
		virtual void fOnPushed( tAnimatedSkeleton & skeleton );

		const tKeyFrameAnimation& fAnim( ) const { return mKeyFrameAnim; }

        void fDisableAnimationEvents( ) { mFireKeyFrameEvents = false; }
        void fEnableAnimationEvents( ) { mFireKeyFrameEvents = true; }
        b32 fCanFireAnimEvents( ) const { return mFireKeyFrameEvents; }

		static void fSetKeyFrameEventID( u32 id ) { cEventKeyFrame = id; }
		static u32 fEventKeyFrameID( ) { return cEventKeyFrame; }

		static f32 fCurrentTimeOfTrack( const tKeyFrameAnimation *find, tAnimatedSkeleton* stack );

	public: // debuggish
		if_devmenu( virtual void fDebugTrackName( std::stringstream& ss, u32 indentDepth ) const );

	private:
		void fFireEvents( tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );

		static u32 cEventKeyFrame;

		const tKeyFrameAnimation& mKeyFrameAnim;
		Math::tPRSXformf mLastRefFrameXform;
		const tSkeletonMap * mSkeletonMap;
        b32 mFireKeyFrameEvents;
	};
} }

#endif//__tKeyFrameAnimTrack__

