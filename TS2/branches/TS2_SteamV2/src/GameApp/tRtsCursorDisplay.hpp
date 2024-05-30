#ifndef __tRtsCursorDisplay__
#define __tRtsCursorDisplay__
#include "Gfx/tWorldSpaceLines.hpp"
#include "Gfx/tWorldSpaceQuads.hpp"
#include "tHoverText.hpp"
#include "tRtsCursorUI.hpp"

namespace Sig
{

	class tRtsCursorDisplay
	{
	public:
		tRtsCursorDisplay( tPlayer& player );
		~tRtsCursorDisplay( );

		enum tVisibility { cShowEverything, cShowJustRings, cShowNothing };
		void fSetVisibility( tVisibility visible );
		void fForceRangeRingUpdate( ) { mRangeRingsDirty = true; }
		void fUpdate( f32 dt, const Math::tMat3f& cursorMatrix, f32 defaultRadius, tEntity* hoverEntity, b32 showStats, b32 isGhost = false );

		Gui::tRtsCursorUIPtr& fUI( ) { return mUI; }

		static f32 fRangeRingFadeLerp( );

	private:
		void fSetupLines( );
		void fSetupQuads( );
		void fSetupRangeQuads( );

		void fAddRing( tGrowableArray< Gfx::tSolidColorRenderVertex >& verts, const Math::tMat3f& xform, f32 radius, const Gfx::tVertexColor& baseColor );
		void fPushIndicatorQuads( Math::tAabbf& bounds, const Math::tMat3f& xform, f32 radius );
		void fPushIndicatorQuad( Gfx::tFullBrightRenderVertex *verts, Math::tAabbf& bounds, const Math::tVec3f& center, const Math::tVec3f& zAxis, const Math::tVec3f& xAxis );

		void fPushRangeQuads( Math::tAabbf& bounds, const Math::tMat3f& xform, f32 minRange, f32 maxRange, tEntity& hover, b32 isGhost = false );
		void fPushRangeArc( Math::tAabbf& bounds, const Math::tMat3f& xform, f32 radius, tEntity& hover, f32 startAngle, f32 angRange, u32& startIndex, tGrowableArray<f32>* samplesOut );
		void fPushRangeLeg( Math::tAabbf& bounds, const Math::tMat3f& xform, f32 radius, f32 angle, tEntity& hover, u32& startIndex );
		void fPushRangeQuad( Gfx::tFullBrightRenderVertex *verts, Math::tAabbf& bounds
			, const Math::tVec3f& p1, const Math::tVec3f& axis1, f32 tv1, f32 alpha1
			, const Math::tVec3f& p2, const Math::tVec3f& axis2, f32 tv2, f32 alpha2 );
		void fPushPulsingRangeQuads( Math::tAabbf& bounds, const Math::tMat3f& xform, f32 minRange, f32 maxRange, tEntity& hover );

	private:
		Gfx::tRenderState mRenderState;
		Gfx::tRenderState mRenderStateRange;
		Gfx::tWorldSpaceLinesPtr mCursorLines;
		Gfx::tWorldSpaceLinesPtr mCursorLinesForOtherPlayer;
		Gfx::tWorldSpaceQuadsPtr mCursorQuads;
		Gfx::tWorldSpaceQuadsPtr mCursorRangeQuads;
		Gui::tHoverTextPtr mHoverText;
		Gui::tRtsCursorUIPtr mUI;
		tPlayer& mPlayer;

		f32 mRingsAlpha;

		f32 mPulseTime;
		f32 mRangePulseTime;
		tVisibility mVisbility;

		// optimization, dont rebuild rings unless necessary
		s32 mLastRangeRingQuadrant;
		b32	mRangeRingsDirty;
		Math::tAabbf mLastRangeRingBounds;

		f32 mGroundHeightStartAngle;
		f32 mGroundHeightAngleRange;
		tGrowableArray<f32> mGroundHeightsNear;
		tGrowableArray<f32> mGroundHeightsFar;
	};
}

#endif //__tRtsCursorDisplay__
