#ifndef __tTracerTrailEntity__
#define __tTracerTrailEntity__
#include "tTracerTrailDef.hpp"
#include "Fx/tQuadTrailEntity.hpp"

namespace Sig { namespace FX
{
	class base_export tTracerTrailEntity : public tQuadTrailEntity
	{
		define_dynamic_cast( tTracerTrailEntity, tQuadTrailEntity );
	public:
		tTracerTrailEntity( tEntity& parent, const tTracerTrailDef& def );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );
		virtual void fEffectsMT( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		void fStopTrackingParent( );

		f32 fIntensity( ) const { return mIntensity; }
		void fSetIntensity( f32 intensity ) { mIntensity = intensity; }

	private:
		void fHeavyLiftingMTSafe( f32 dt, b32 firstFrame = false );
		void fHeavyLiftingMTSafeInternal( f32 dt, f32 parentLerp, const Math::tPRSXformf& parentObjToWorld );

		f32 fComputeAlpha( f32 age ) const;
		tElement fCreateElement( const Math::tMat3f& transform ) const;


		f32 mLastSpawn;
		f32 mSpinAngle;
		f32 mSpinVariation;
		f32 mIntensity; //0 to 1 for how much we should be showing the trail

		Math::tPRSXformf mPrevXform;
		tEntityPtr mParent;

		const tTracerTrailDef& mDef;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tTracerTrailEntity );
}}

#endif //__tTracerTrailEntity__
