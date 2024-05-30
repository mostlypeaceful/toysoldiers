#ifndef __tRagDollAnimTrack__
#define __tRagDollAnimTrack__

#include "tAnimTrack.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tSkeletonFile.hpp"
#include "Physics/tRagDoll.hpp"

namespace Sig { namespace Anim
{

	struct tRagDollAnimTrackDesc : public tAnimTrackDesc
	{
		Physics::tRagDoll*	mDoll;
		tEntity*			mEntity;
		tKeyFrameAnimation* mLimits; // optional, if null, will leave constraints un touched

		tRagDollAnimTrackDesc( )
			: mDoll( NULL )
			, mEntity( NULL )
			, mLimits( NULL )
		{ }

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tRagDollAnimTrack : public tAnimTrack
	{
		define_dynamic_cast( tRagDollAnimTrack, tAnimTrack );
	private:
		Physics::tRagDollPtr	mDoll;
		tEntityPtr				mOwner;
		b32						mVelocitiesSet;
		b32						mSnapShotting;
		f32						mVelocityTimer;

		tDynamicArray< Math::tPRSXformf > mOutput; 
		tDynamicArray< Math::tPRSXformf > mPallet; 

		void fComputeResult( );
		void fBlendResult( tAnimatedSkeleton& animSkel );

	public:
		tRagDollAnimTrack( const tRagDollAnimTrackDesc& desc, tAnimatedSkeleton& skeleton );
		~tRagDollAnimTrack( );

		virtual void fPostAnimEvaluate( tAnimatedSkeleton& animSkel );

		virtual void fStepST( f32 dt, tAnimatedSkeleton& animSkel );
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign );
		virtual void fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend );

#ifdef sig_devmenu
		virtual void fDebugTrackName( std::stringstream& ss, u32 indentDepth ) const
		{
			fDebugIndent( ss, indentDepth );

			ss << "Ragdoll Track";
			if( fTag( ).fExists( ) )
				ss << " - " << fTag( ).fCStr( );
		}
		//virtual void fDebugTrackData( std::stringstream& ss, u32 indentDepth ) const
		//{
		//	tAnimTrack::fDebugTrackData( ss, indentDepth );
		//}
#endif//sig_devmenu	
	};
} }

#endif//__tRagDollAnimTrack__

