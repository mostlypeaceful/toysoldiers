#include "GameAppPch.hpp"
#include "tWorldSpaceArc.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "tGameApp.hpp"

using namespace Sig::Math;

namespace Sig
{
	devvar( f32, Gameplay_Weapon_CannonArc_HeightAboveTerrain, 5.0f );
	devvar( f32, Gameplay_Weapon_CannonArc_ArrowLength, 4.7f ); 
	devvar( f32, Gameplay_Weapon_CannonArc_BarrelWidth, 0.05f ); 
	devvar( f32, Gameplay_Weapon_CannonArc_TaperLength, 0.0f );
	devrgba_clamp( Gameplay_Weapon_CannonArc_EdgeColor, Math::tVec4f( 0.0f, 0.1f, 0.0f, 0.5f ), 0.f, 1.f, 2 );
	devrgba_clamp( Gameplay_Weapon_CannonArc_CenterColor, Math::tVec4f( 1.0f, 1.0f, 1.0f, 1.0f ), 0.f, 1.f, 2 );


	tWorldSpaceArc::tWorldSpaceArc( const tEntityPtr& parent )
	{
		mQuads.fReset( NEW Gfx::tWorldSpaceQuads( ) );

		tResourcePtr colorMapResource = tGameApp::fInstance( ).fCannonAimingTexture( );

		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );
		mQuads->fResetDeviceObjectsTexture( tGameApp::fInstance( ).fScreen( )->fGetDevice( )
			, colorMapResource
			, allocator.mFullBrightMaterialFile
			, allocator.mFullBrightGeomAllocator
			, allocator.mIndexAllocator );

		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );
		//mRenderState.fEnableDisable( Gfx::tRenderState::cFillWireFrame, true );
		mQuads->fGeometry( ).fSetRenderStateOverride( &mRenderState );

		mQuads->fSpawn( *parent );
		mQuads->fSetLockedToParent( false );
		mQuads->fMoveTo( Math::tMat3f::cIdentity );

		for( u32 i = 0; i < mLines.fCount( ); ++i )
		{
			mLines[ i ].fReset( NEW Gfx::tWorldSpaceLines( ) );
			mLines[ i ]->fResetDeviceObjects( tGameApp::fInstance( ).fScreen( )->fGetDevice( ), allocator.mSolidColorMaterial, allocator.mSolidColorGeomAllocator, allocator.mIndexAllocator );

			mLines[ i ]->fSetLockedToParent( false );
			mLines[ i ]->fSpawn( *parent );
			mLines[ i ]->fMoveTo( Math::tMat3f::cIdentity );
			mLines[ i ]->fSetDisallowIndirectColorControllers( true );
		}

		mQuads->fSetDisallowIndirectColorControllers( true );
	}

	void tWorldSpaceArc::fPushQuad( Gfx::tFullBrightRenderVertex *verts, tAabbf& bounds, const Math::tVec3f& right, tGrowableArray< Gfx::tSolidColorRenderVertex >& lines1, tGrowableArray< Gfx::tSolidColorRenderVertex >& lines2, const tVec3f& p1, f32 w1, const tVec3f& p2, f32 w2, bool pushDegenerate, u32 centerColor, u32 lineColor )
	{
		f32 texV1 = 0.0f;
		f32 texV2 = 1.0f;

		verts[0].mP = p1 + right * -w1;
		verts[0].mColor = centerColor;
		verts[0].mUv = tVec2f( 1.0f, texV1 );
		bounds |= verts[0].mP;

		verts[1].mP = p1 + right * w1;
		verts[1].mColor = centerColor;
		verts[1].mUv = tVec2f( 0.0f, texV1 );
		bounds |= verts[1].mP;

		verts[2].mP = p2 + right * w2;
		verts[2].mColor = centerColor;
		verts[2].mUv = tVec2f( 0.0f, texV2 );
		bounds |= verts[2].mP;
		lines1.fPushBack( Gfx::tSolidColorRenderVertex( verts[2].mP, lineColor )  );
		if( pushDegenerate ) lines1.fPushBack( lines1.fBack( ) );

		verts[3].mP = p2 + right * -w2;
		verts[3].mColor = centerColor;
		verts[3].mUv = tVec2f( 1.0f, texV2 );
		bounds |= verts[3].mP;
		lines2.fPushBack( Gfx::tSolidColorRenderVertex( verts[3].mP, lineColor ) );
		if( pushDegenerate ) lines2.fPushBack( lines2.fBack( ) );
	}

	//return false if reached begining of path
	b32 fGetPrevPoint( f32 distance, u32& currentIndex, f32& currentT, const tGrowableArray< Math::tVec3f >& points )
	{
		if( distance == 0.0f )
			return true;

		while( distance > 0 )
		{
			if( currentIndex == 0 ) 
				return false;

			const tVec3f &p1 = points[currentIndex - 1];
			const tVec3f &p2 = points[currentIndex];

			tVec3f diff = p2-p1;
			f32 segLen;
			diff.fNormalizeSafe( tVec3f::cZeroVector, segLen );

			f32 remainingSegLen = segLen * currentT;

			if( remainingSegLen <= distance )
			{
				distance -= remainingSegLen;
				--currentIndex;
				currentT = 1.0f;
			}
			else
			{
				currentT = (remainingSegLen-distance)/segLen;
				return true;
			}
		}

		currentIndex = 0;
		return false;
	}

	f32 fPathLength( u32 currentIndex, f32 currentT, const tGrowableArray< Math::tVec3f >& points )
	{
		f32 dist = 0.0f;

		while( 1 )
		{
			if( currentIndex == 0 ) 
				return dist;

			const tVec3f &p1 = points[currentIndex - 1];
			const tVec3f &p2 = points[currentIndex];

			tVec3f diff = p2-p1;
			f32 segLen;
			diff.fNormalize( segLen );

			segLen *= currentT;

			dist += segLen;
			--currentIndex;
			currentT = 1.0f;
		}

		return dist;
	}
	
	tVec3f fSamplePoints( u32 currentIndex, f32 currentT, const tGrowableArray< Math::tVec3f >& points )
	{
		if( currentIndex == 0 )
			return points.fFront( );

		const tVec3f &p1 = points[currentIndex - 1];
		const tVec3f &p2 = points[currentIndex];

		tVec3f diff = p2-p1;
		return p1 + diff * currentT;
	}

	void tWorldSpaceArc::fSetPoints( const tGrowableArray< Math::tVec3f >& points, f32 arrowWidth )
	{
		profile( cProfilePerfArcGfx );

		u32 lineColor = Gfx::tVertexColor( Gameplay_Weapon_CannonArc_EdgeColor ).fForGpu( );
		//u32 centerColor = Gfx::tVertexColor( Gameplay_Weapon_CannonArc_CenterColor ).fForGpu( );
		u32 centerColor = Gfx::tVertexColor( Math::tVec4f( 1.0f, 1.0f, 1.0f, 1.0f ) ).fForGpu( );

		// TODO this is heavy lifting, move to MT
		sigassert( points.fCount( ) >= 2 );

		const tVec3f right = tVec3f::cYAxis.fCross( points[ 1 ] - points[ 0 ] ).fNormalizeSafe( tVec3f::cZeroVector );

		// these can't be zero!
		const u32 numShaftSegs = points.fCount( );
		const u32 numArrowHeadSegs = 4;
		const u32 numTaperSegs = 2;

		s32 quadCnt = numArrowHeadSegs + numShaftSegs + numTaperSegs;
		mQuads->fSetQuadCount( quadCnt );

		// iterator states
		u32 currentIndex = points.fCount( ) - 1;
		f32 currentT = 1.0f;
		s32 qIndex = 0;
		tVec3f prevPoint;
		f32 step, taperRate;

		//find starting point
		tVec3f barrel = points.fFront( );
		fGetPrevPoint( Gameplay_Weapon_CannonArc_HeightAboveTerrain, currentIndex, currentT, points );
		tVec3f tip = fSamplePoints( currentIndex, currentT, points );

		tAabbf bounds;
		bounds.fInvalidate( );

		tGrowableArray< Gfx::tSolidColorRenderVertex > lineVerts1, lineVerts2;
		lineVerts1.fSetCapacity( quadCnt + 1 + 1 ); //one extra like normal, plus 1 degenerate because the last line is unreliable?
		lineVerts2.fSetCapacity( quadCnt + 1 + 1 );

		lineVerts1.fPushBack( Gfx::tSolidColorRenderVertex( tip, lineColor ) );
		lineVerts2.fPushBack( Gfx::tSolidColorRenderVertex( tip, lineColor ) );

		//push arrow head
		step = Gameplay_Weapon_CannonArc_ArrowLength / numArrowHeadSegs;
		taperRate = arrowWidth / numArrowHeadSegs;
		prevPoint = tip;

		for( u32 s = 0; s < numArrowHeadSegs; ++s )
		{
			fGetPrevPoint( step, currentIndex, currentT, points );
			tVec3f newPoint = fSamplePoints( currentIndex, currentT, points );

			f32 w1 = (s+1) * taperRate ;
			f32 w2 = s * taperRate;

			fPushQuad( mQuads->fQuad( qIndex++ ), bounds, right, lineVerts1, lineVerts2
				, prevPoint, w2
				, newPoint, w1, false, centerColor, lineColor );

			prevPoint = newPoint;
		}

		//push taper
		const f32 shaftWidth = arrowWidth * 0.5f;
		step = Gameplay_Weapon_CannonArc_TaperLength / numTaperSegs;
		taperRate = (shaftWidth - arrowWidth) / numTaperSegs;

		for( u32 s = 0; s < numTaperSegs; ++s )
		{
			fGetPrevPoint( step, currentIndex, currentT, points );
			tVec3f newPoint = fSamplePoints( currentIndex, currentT, points );

			f32 w1 = arrowWidth + (s+1) * taperRate ;
			f32 w2 = arrowWidth + s * taperRate;

			fPushQuad( mQuads->fQuad( qIndex++ ), bounds, right, lineVerts1, lineVerts2
				, prevPoint, w2
				, newPoint, w1, false, centerColor, lineColor );

			prevPoint = newPoint;
		}

		f32 pathRemaining = fPathLength( currentIndex, currentT, points );
		step = pathRemaining / numShaftSegs;
		taperRate = (Gameplay_Weapon_CannonArc_BarrelWidth - shaftWidth) / numShaftSegs;

		for( u32 s = 0; s < numShaftSegs; ++s )
		{
			fGetPrevPoint( step, currentIndex, currentT, points );
			tVec3f newPoint = fSamplePoints( currentIndex, currentT, points );

			f32 w1 = shaftWidth + (s+1) * taperRate;
			f32 w2 = shaftWidth + s * taperRate;

			fPushQuad( mQuads->fQuad( qIndex++ ), bounds, right, lineVerts1, lineVerts2
				, prevPoint, w2
				, newPoint, w1, (s==numShaftSegs-1), centerColor, lineColor );

			prevPoint = newPoint;
		}

		log_assert( qIndex == quadCnt, "Uninitialized quads in tWorldSpaceArc.cpp" );
		
		mQuads->fSetObjectSpaceBox( bounds );

		// this has to be ST, mostly
		mQuads->fCreateGeometry( *tGameApp::fInstance( ).fScreen( )->fGetDevice( ) );	
		
		mLines[ 0 ]->fSetGeometry( lineVerts1, true );
		mLines[ 1 ]->fSetGeometry( lineVerts2, true );
	}

	void tWorldSpaceArc::fSetInvisible( b32 invisible )
	{
		if( mQuads ) mQuads->fSetInvisible( invisible );

		for( u32 i = 0; i < mLines.fCount( ); ++i )
			mLines[ i ]->fSetInvisible( invisible );
	}
	
	void tWorldSpaceArc::fSetRgbaTint( const Math::tVec4f& tint )
	{
		if( mQuads ) mQuads->fSetRgbaTint( tint );

		for( u32 i = 0; i < mLines.fCount( ); ++i )
			mLines[ i ]->fSetRgbaTint( tint );
	}

	void tWorldSpaceArc::fSetViewportMask( u32 mask )
	{
		if( mQuads ) mQuads->fSetViewportMask( mask );

		for( u32 i = 0; i < mLines.fCount( ); ++i )
			mLines[ i ]->fSetViewportMask( mask );
	}
}
