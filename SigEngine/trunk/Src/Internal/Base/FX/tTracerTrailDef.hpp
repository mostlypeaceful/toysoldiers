#ifndef __tTracerTrailDef__
#define __tTracerTrailDef__
#include "Gfx/tVertexFormat.hpp"
#include "Gfx/tRenderState.hpp"

namespace Sig { namespace FX
{
	class base_export tTracerTrailDef : public tRefCounter
	{
	public:
		b8 mRealTime;
		b8 mSmooth;
		u8 mStepCount;
		b8 mBeefyQuads;

		f32 mSpawnRate; //time in between trail elements
		f32 mLifeSpan;
		f32 mLeadDistance;
		f32 mWidth;
		f32 mSpinRate;

		tResourcePtr mTexture;
		Gfx::tVertexColor mTint;
		Gfx::tRenderState mRenderState;

		tTracerTrailDef( )
			: mRealTime( false )
			, mSmooth( false )
			, mStepCount( 1 ) // number of steps per game step
			, mBeefyQuads( false )
			, mSpawnRate( 0.075f )
			, mLifeSpan( 1.5f )
			, mLeadDistance( 0.0f )
			, mWidth( 0.25f )
			, mSpinRate( 2.5f )
			, mTint( 1.0f, 1.0f, 1.0f, 1.0f )
			, mRenderState( Gfx::tRenderState::cDefaultColorTransparent )
		{
			mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );
		}
	};

	define_smart_ptr( base_export, tRefCounterPtr, tTracerTrailDef );
}}

#endif //__tTracerTrailDef__
