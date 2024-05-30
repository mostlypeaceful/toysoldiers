#ifndef __tLightningLogic__
#define __tLightningLogic__

#include "fx/tQuadTrailEntity.hpp"

namespace Sig
{

	class tLightningBolt : public FX::tQuadTrailEntity
	{
		define_dynamic_cast( tLightningBolt, FX::tQuadTrailEntity );
	public:
		tLightningBolt( const FX::tTracerTrailDef& def );

		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );
		virtual void fEffectsMT( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		void fStopTrackingParent( );

		void fSetTarget( const Math::tVec3f& target );
		void fSetAlphaTarget( f32 alpha );
		void fSetFracs( u32 fracs );

	private:
		void fHeavyLiftingMTSafe( f32 dt, b32 firstFrame = false );
		void fHeavyLiftingMTSafeInternal( f32 dt );

		f32 fComputeAlpha( f32 age ) const;
		tElement fCreateElement( const Math::tVec3f& pos, const Math::tVec3f& axis ) const;

		void fFracture( );
		void fFrac( const Math::tVec3f& p1, const Math::tVec3f& p2, const Math::tVec3f& axis, const Math::tVec3f& otherAxis, u32 frac, f32 displacement, u32 beginSeg, u32 endSeg );

		u32 mSegments;
		u32 mFracs;
		Math::tVec3f mTarget;
		tRandom mRandom;

		f32 mTargetAlpha;
		f32 mCurrentAlpha;

		const FX::tTracerTrailDef& mDef;
	};

	class tLightningEntity : public tEntity
	{
		define_dynamic_cast( tLightningEntity, tEntity );
	public:
		tLightningEntity( u32 tracerIndex );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fMoveST( f32 dt );

		void fSetTarget( const Math::tVec3f& target );
		void fSetAlpha( f32 alpha );
		void fSetAlphaTarget( f32 alpha );
		void fSetFracs( u32 fracs );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
		u32 mTracerIndex;
		Math::tVec3f mTarget;
		u32 mFracs;

		tGrowableArray< tRefCounterPtr< tLightningBolt > > mBolts;
	};

}

#endif//__tLightningLogic__
