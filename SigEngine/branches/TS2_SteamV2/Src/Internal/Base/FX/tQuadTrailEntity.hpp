#ifndef __tQuadTrailEntity__
#define __tQuadTrailEntity__
#include "tTracerTrailDef.hpp"
#include "Gfx/tWorldSpaceQuads.hpp"
#include "Gfx/tVertexFormat.hpp"
#include "Gfx/tRenderState.hpp"

namespace Sig { namespace FX
{
	class base_export tQuadTrailEntity : public Gfx::tWorldSpaceQuads
	{
		define_dynamic_cast( tQuadTrailEntity, Gfx::tWorldSpaceQuads );
	public:
		tQuadTrailEntity( const tResourcePtr& texture, const Gfx::tVertexColor& tint, const Gfx::tRenderState* renderState, b32 beefy );

		virtual void fOnSpawn( );
		virtual b32  fReadyForDeletion( );
		virtual void fOnDelete( );
		virtual void fThinkST( f32 dt ); //user must call this

		b32 fAlive( ) const { return mElements.fNumItems( ) > 0; }

		void fFillGraphics( );

	protected:
		struct tElement
		{
			f32 mAge;
			f32 mAgeAlpha;	//alpha multiplier based on the age
			f32 mAlpha;		//desired alpha multiplier at all ages. total alpha  = mAlpha * mAgeAlpha;
			b32 mPredicted;
			Math::tVec3f mPosition;
			Math::tVec3f mAxis;
		};

		static tElement fCatmullRom( const tElement& p0, const tElement& p1, const tElement& p2, const tElement& p3, f32 t );
		static tElement fCatmullRom( const tElement& p0, const tElement& p1, const tElement& p2, f32 t ); // predicts 4th position

		void fPushQuad( Gfx::tFullBrightRenderVertex *verts, Math::tAabbf& bounds
			, const Math::tVec3f& p1, const Math::tVec3f& axis1, f32 alpha1, f32 tv1
			, const Math::tVec3f& p2, const Math::tVec3f& axis2, f32 alpha2, f32 tv2 );

		tElement mHead;
		tRingBuffer< tElement > mElements;

		Gfx::tVertexColor mColor;
		Math::tAabbf mBounds;

		b32 mBeefy;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tQuadTrailEntity );
}}

#endif //__tQuadTrailEntity__
