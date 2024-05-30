#ifndef __tWorldSpaceArc__
#define __tWorldSpaceArc__

#include "Gfx/tWorldSpaceQuads.hpp"
#include "Gfx/tWorldSpaceLines.hpp"

namespace Sig
{

	class tWorldSpaceArc : public tRefCounter
	{
	public:
		tWorldSpaceArc( const tEntityPtr& parent );

		void fSetPoints( const tGrowableArray< Math::tVec3f >& points, f32 arrowWidth );

		void fSetInvisible( b32 invisible );
		void fSetRgbaTint( const Math::tVec4f& tint );
		void fSetViewportMask( u32 mask );

	private:
		tFixedArray< Gfx::tWorldSpaceLinesPtr, 2 > mLines;
		Gfx::tWorldSpaceQuadsPtr mQuads;
		Gfx::tRenderState mRenderState;

		void fPushQuad( Gfx::tFullBrightRenderVertex *verts, Math::tAabbf& bounds, const Math::tVec3f& right, tGrowableArray< Gfx::tSolidColorRenderVertex >& lines1, tGrowableArray< Gfx::tSolidColorRenderVertex >& lines2, const Math::tVec3f& p1, f32 w1, const Math::tVec3f& p2, f32 w2, bool pushDegenerate, u32 centerColor, u32 lineColor );
	};

	typedef tRefCounterPtr<tWorldSpaceArc> tWorldSpaceArcPtr;
}

#endif //__tWorldSpaceArc__