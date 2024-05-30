//------------------------------------------------------------------------------
// \file tHeightFieldPaintCursor.cpp - 05 Aprl 2008
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "SigEdPch.hpp"
#include "tHeightFieldPaintCursor.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tTextureSysRam.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "tEditableObjectContainer.hpp"
#include "tHeightFieldMeshEntity.hpp"

namespace Sig
{
	namespace
	{
		static const f32 cCursorHeightOffset = 0.125f;
		static const Math::tVec2f cDefaultRadiusRange( 1.f, 60.f );

		inline void fProjectSquare( u32 i, u32 vertCount, f32& dx, f32& dz )
		{
			const u32 vertCountDiv4 = vertCount / 4;
			const u32 vertCountDiv8 = vertCount / 8;
			i = ( i + vertCountDiv8 ) % vertCount; // shift for easier quadrant detection

			switch( i / vertCountDiv4 )
			{
			case 0: dz =  dz / dx; dx =  1.f; break;
			case 1: dx =  dx / dz; dz =  1.f; break;
			case 2: dz = -dz / dx; dx = -1.f; break;
			case 3: dx = -dx / dz; dz = -1.f; break;

			default: sigassert( !"invalid case" ); break;
			}
		}
	}


	//------------------------------------------------------------------------------
	// tHeightFieldPaintCursor
	//------------------------------------------------------------------------------

	tHeightFieldPaintCursor::tHeightFieldPaintCursor( tEditorCursorControllerButton* button )
		: tEditorButtonManagedCursorController( button )
		, mPainting( false )
		, mRadiusRange( cDefaultRadiusRange )
		, mSize( 0.25f )
		, mStrength( 0.5f )
		, mFalloff( 0.25f )
		, mShape( 0.f )
		, mCursorLines( new Gfx::tSolidColorLines( ) )
	{
		tWxRenderPanelContainer* gfx = fMainWindow( ).fRenderPanelContainer( );
		mCursorLines->fResetDeviceObjects( 
			fMainWindow( ).fGuiApp( ).fGfxDevice( ), gfx->fGetSolidColorMaterial( ), gfx->fGetSolidColorGeometryAllocator( ), gfx->fGetSolidColorIndexAllocator( ) );
	}

	void tHeightFieldPaintCursor::fOnNextCursor( tEditorCursorController* nextController )
	{
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fClear( );
		tEditorButtonManagedCursorController::fOnNextCursor( nextController );
	}

	tEntityPtr tHeightFieldPaintCursor::fFilterHoverObject( const tEntityPtr& newHoverObject )
	{
		if( newHoverObject && !newHoverObject->fDynamicCast< tEditableTerrainGeometry >( ) )
		{
			// current hover object doesn't support editable terrain geometry
			if( mPainting && mPaintEntity )
			{
				// however, we're in the middle of painting, so continue to pretend our
				// current hover object is the terrain itself; this prevents hickups
				// in painting when the cursor goes over an object on the terrain
				return mPaintEntity;
			}
			else
				return tEntityPtr( );
		}

		// this hover object supports editable terrain geometry
		return newHoverObject;
	}

	void tHeightFieldPaintCursor::fOnTick( )
	{
		fHandleHover( );
		fHandleCursor( );
	}

	f32 tHeightFieldPaintCursor::fComputeRadius( ) const
	{
		return Math::fLerp( mRadiusRange.x, mRadiusRange.y, mSize * mSize );
	}

	f32 tHeightFieldPaintCursor::fComputeStrength( ) const
	{
		return Math::fLerp( 0.01f, 1.0f, mStrength );
	}

	f32 tHeightFieldPaintCursor::fComputeFalloff( ) const
	{
		return Math::fLerp( 0.1f, 8.0f, mFalloff * mFalloff );
	}

	f32 tHeightFieldPaintCursor::fComputeShape( ) const
	{
		return std::powf( mShape, 1.5f );
	}


	void tHeightFieldPaintCursor::fBeginPaint( )
	{
		if( !mPainting )
		{
			// we're transitioning from not-painting to painting; do any setup here if necessary
			mPaintEntity = mCurrentHoverObject;
			fMainWindow( ).fGuiApp( ).fSelectionList( ).fAdd( mPaintEntity );
			fBeginAction( );
		}
		mPainting = true;
	}

	void tHeightFieldPaintCursor::fEndPaint( )
	{
		if( mPainting )
		{
			// we're transitioning from painting to not-painting; do any cleanup here if necessary
			fEndAction( );
			fMainWindow( ).fGuiApp( ).fSelectionList( ).fRemove( mPaintEntity );
			mPaintEntity.fRelease( );
		}

		mPainting = false;
	}

	f32 tHeightFieldPaintCursor::fInnerRadius( f32 outerRadius ) const
	{
		return outerRadius * Math::fLerp( 0.05f, 0.95f, mFalloff );
	}

	void tHeightFieldPaintCursor::fHandleCursor( )
	{
		tWxRenderPanel* renderPanel = fMainWindow( ).fRenderPanelContainer( )->fGetActiveRenderPanel( );
		tEditableTerrainGeometry* etg = mCurrentHoverObject ? mCurrentHoverObject->fDynamicCast< tEditableTerrainGeometry >( ) : 0;
		if( !renderPanel || !mCurrentHoverObject || !etg || etg->fIsFrozen( ) )
		{
			fEndPaint( );
			return;
		}

		if( mPaintEntity != mCurrentHoverObject )
			fEndPaint( );

		const f32 radius = fComputeRadius( );
		const Math::tVec3f centerInLocal = etg->fWorldToObject( ).fXformPoint( mLastHoverIntersection );
		const Input::tMouse& mouse = renderPanel->fGetMouse( );

		if( !fMainWindow( ).fPriorityInputActive( ) && mouse.fButtonHeld( Input::tMouse::cButtonLeft ) )// don't edit if other input is active
		{
			fBeginPaint( );
			fDoPaintAction( );
		}
		else
		{
			fEndPaint( );
		}

		// render cursor after editing verts
		fRenderCursor( radius, centerInLocal, etg );
	}
	void tHeightFieldPaintCursor::fRenderCursor( f32 radius, const Math::tVec3f& centerInLocal, tEditableTerrainGeometry* etg )
	{
		tGrowableArray< Gfx::tSolidColorRenderVertex > renderVerts;
		renderVerts.fSetCapacity( 512 );

		// outer ring color; constant
		const Math::tVec3f oc = Math::tVec3f( 0.0f, 0.f, 0.f );

		// inner ring color; this lerps between two colors based on the strength of the brush
		const Math::tVec3f ic = Math::fLerp( Math::tVec3f( 1.0f, 1.0f, 0.f ), Math::tVec3f( 1.f, 0.f, 0.f ), mStrength );

		// compute the inner ring's radius; we scale this based on falloff
		const f32 innerRadius = fInnerRadius( radius );

		// create the brush's outer ring of verts
		fAddRing( etg, radius,      centerInLocal, Gfx::tVertexColor( oc.x, oc.y, oc.z ).fForGpu( ), renderVerts );

		// create the brush's inner ring of verts
		const u32 icGpu = Gfx::tVertexColor( ic.x, ic.y, ic.z ).fForGpu( );
		fAddRing( etg, innerRadius, centerInLocal, icGpu, renderVerts );

		// Let derivatives muck with the geometry
		fOnRenderCursor( radius, centerInLocal, etg, icGpu, renderVerts ); 

		// bake the cursor lines into a vertex buffer
		mCursorLines->fBake( ( Sig::byte* )renderVerts.fBegin( ), renderVerts.fCount( ), false );

		tWxRenderPanelContainer* gfx = fMainWindow( ).fRenderPanelContainer( );
		for( tWxRenderPanel** ipanel = gfx->fRenderPanelsBegin( ); ipanel != gfx->fRenderPanelsEnd( ); ++ipanel )
		{
			if( !*ipanel || !(*ipanel)->fIsVisible( ) )
				continue;

			tWxRenderPanel* panel = *ipanel;
			panel->fGetScreen( )->fAddWorldSpaceDrawCall( Gfx::tRenderableEntityPtr( mCursorLines.fGetRawPtr( ) ) );
		}
	}

	void tHeightFieldPaintCursor::fAddRing( 
		tEditableTerrainGeometry* etg,
		f32 radius, 
		const Math::tVec3f& centerInLocal, 
		u32 color,
		tGrowableArray< Gfx::tSolidColorRenderVertex >& renderVerts )
	{
		tDynamicArray< Math::tVec3f > verts( 256 );

		const f32 squareAmount = fComputeShape( );

		for( u32 i = 0; i < verts.fCount( ); ++i )
		{
			const f32 theta = ( i / ( f32 )( verts.fCount( ) ) ) * Math::c2Pi;

			// compute circle position
			f32 dx = std::cos( theta );
			f32 dz = std::sin( theta );
			f32 sampleX = centerInLocal.x + radius * dx;
			f32 sampleZ = centerInLocal.z + radius * dz;
			const Math::tVec3f pcircle = Math::tVec3f( sampleX, 0.f, sampleZ );

			// compute square position (by projecting circle pos onto square)
			fProjectSquare( i, verts.fCount( ), dx, dz );
			sampleX = centerInLocal.x + radius * dx;
			sampleZ = centerInLocal.z + radius * dz;
			const Math::tVec3f psquare = Math::tVec3f( sampleX, 0.f, sampleZ );

			// lerp between circle and square
			verts[ i ] = Math::fLerp( pcircle, psquare, squareAmount );

			// sample height at lerped location
			verts[ i ].y = etg->fSampleHeight( verts[ i ].x, verts[ i ].z ) + cCursorHeightOffset;
		}

		for( u32 i = 0; i < verts.fCount( ) - 1; ++i )
			fSubDivideCursorLine( etg, verts[ i ], verts[ i + 1 ], renderVerts, color, 0 );
		// add last segment between last control point and first control point
		fSubDivideCursorLine( etg, verts.fBack( ), verts.fFront( ), renderVerts, color, 0 );
	}

	void tHeightFieldPaintCursor::fSubDivideCursorLine( 
		tEditableTerrainGeometry* etg,
		const Math::tVec3f& p0, const Math::tVec3f& p1,
		tGrowableArray< Gfx::tSolidColorRenderVertex >& addTo, 
		u32 vtxColor, u32 currentDepth )
	{
		if( currentDepth < 128 && // ensure we don't overflow stack
			( p1 - p0 ).fLengthSquared( ) > Math::fSquare( 0.125f ) )
		{
			const Math::tVec3f m = 0.5f * ( p0 + p1 );
			const f32 sampleHeight = etg->fSampleHeight( m.x, m.z ) + cCursorHeightOffset;
			if( m.y < sampleHeight ) // only further sub-divide if this point is below the ground
			{
				const Math::tVec3f m0 = Math::tVec3f( m.x, sampleHeight, m.z );
				fSubDivideCursorLine( etg, p0, m0, addTo, vtxColor, currentDepth + 1 );
				fSubDivideCursorLine( etg, m0, p1, addTo, vtxColor, currentDepth + 1 );
				return;
			}
		}

		const Math::tVec3f p0world = etg->fObjectToWorld( ).fXformPoint( p0 );
		const Math::tVec3f p1world = etg->fObjectToWorld( ).fXformPoint( p1 );
		addTo.fPushBack( Gfx::tSolidColorRenderVertex( p0world, vtxColor ) );
		addTo.fPushBack( Gfx::tSolidColorRenderVertex( p1world, vtxColor ) );
	}

	tEntityPtr tHeightFieldPaintCursor::fPick( const Math::tRayf& ray, f32* bestTout, tEntity* const* ignoreList, u32 numToIgnore )
	{
		return fGuiApp( ).fEditableObjects( ).fPickByType< tHeightFieldMeshEntity >( ray, bestTout, ignoreList, numToIgnore );
	}

	
	///
	/// \class tModifyTerrainHeightAction
	/// \brief 
	class tModifyTerrainHeightAction : public tEditorButtonManagedCursorAction
	{
		tEntityPtr mEntity;
		tTerrainGeometry::tHeightField mStart, mEnd;
		u32 mMinX, mMinZ, mMaxX, mMaxZ;
	public:
		tModifyTerrainHeightAction( tToolsGuiMainWindow& mainWindow, const tEntityPtr& entity  )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mEntity( entity )
			, mMinX( ~0 )
			, mMinZ( ~0 )
			, mMaxX( 0 )
			, mMaxZ( 0 )
		{
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fSaveHeightField( mStart );
			fSetIsLive( true );
		}
		virtual void fUndo( )
		{
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fRestoreHeightField( mStart, mMinX, mMinZ, mMaxX, mMaxZ );
		}
		virtual void fRedo( )
		{
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fRestoreHeightField( mEnd, mMinX, mMinZ, mMaxX, mMaxZ );
		}
		virtual void fEnd( )
		{
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fSaveHeightField( mEnd );
			tEditorButtonManagedCursorAction::fEnd( );
		}
		void fUpdateDirtyRect( u32 minx, u32 minz, u32 maxx, u32 maxz )
		{
			if( minx < mMinX ) mMinX = minx;
			if( minz < mMinZ ) mMinZ = minz;
			if( maxx > mMaxX ) mMaxX = maxx;
			if( maxz > mMaxZ ) mMaxZ = maxz;
		}
	};


	//------------------------------------------------------------------------------
	// tHeightFieldVertexPaintCursor
	//------------------------------------------------------------------------------

	tHeightFieldVertexPaintCursor::tHeightFieldVertexPaintCursor( tEditorCursorControllerButton* button )
		: tHeightFieldPaintCursor( button )
	{
	}

	tHeightFieldVertexPaintCursor::~tHeightFieldVertexPaintCursor( )
	{
	}

	void tHeightFieldVertexPaintCursor::fDoPaintAction( )
	{
		tWxRenderPanel* renderPanel = fMainWindow( ).fRenderPanelContainer( )->fGetActiveRenderPanel( );
		tEditableTerrainGeometry* etg = mCurrentHoverObject->fDynamicCast< tEditableTerrainGeometry >( );

		const f32 radius = fComputeRadius( );
		const f32 innerRadius = fInnerRadius( radius );
		const Math::tVec3f centerInLocal = etg->fWorldToObject( ).fXformPoint( mLastHoverIntersection );
		const Input::tMouse& mouse = renderPanel->fGetMouse( );
		const Input::tKeyboard& kb = Input::tKeyboard::fInstance( );
		const b32 shiftHeld = kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) || kb.fButtonHeld( Input::tKeyboard::cButtonRShift );
		const b32 ctrlHeld = kb.fButtonHeld( Input::tKeyboard::cButtonLCtrl ) || kb.fButtonHeld( Input::tKeyboard::cButtonRCtrl );
		const f32 strength = fComputeStrength( );
		const f32 falloff = fComputeFalloff( );
		const f32 shape = fComputeShape( );
		const f32 dt = fMainWindow( ).fGuiApp( ).fGetFrameDeltaTime( );
		const f32 sign = shiftHeld ? -1.f : +1.f;

		tEditableTerrainGeometry::tEditableVertices verts;
		etg->fBeginEditingVerts( verts, centerInLocal, 2.f * radius, 2.f * radius );

		if( verts.fDimZ( ) == 0 || verts.fDimX( ) == 0 )
			return;

		for( u32 z = 0; z < verts.fDimZ( ); ++z )
		{
			for( u32 x = 0; x < verts.fDimX( ); ++x )
			{
				tEditableTerrainGeometry::tEditableVertex& ev = verts.fIndex( x, z );

				const f32 circleDistToCenter = std::sqrt( Math::fSquare( ev.fLocalX( ) - centerInLocal.x ) + Math::fSquare( ev.fLocalZ( ) - centerInLocal.z ) );
				const f32 squareDistToCenter = fMax( fAbs( ev.fLocalX( ) - centerInLocal.x ), fAbs( ev.fLocalZ( ) - centerInLocal.z ) );
				const f32 distToCenter = Math::fLerp( circleDistToCenter, squareDistToCenter, shape );

				const f32 distToCenterNorm = fMin( 1.f, fMax( 0.f, distToCenter / radius ) );
				const f32 distBasedStrength = 1.f - std::powf( distToCenterNorm, falloff );
				const f32 paintStrength = strength * distBasedStrength;
				sigassert( paintStrength >= 0.f );

				fModifyVertex( tModifyVertexArgs( x, z, paintStrength, sign, dt, distToCenterNorm, innerRadius / radius, strength, shiftHeld, ctrlHeld, verts ) );
			}
		}

		mCurrentAction->fUpdateDirtyRect( 
			verts.fIndex( 0, 0 ).fLogicalX( ),
			verts.fIndex( 0, 0 ).fLogicalZ( ),
			verts.fIndex( verts.fDimX( ) - 1, 0 ).fLogicalX( ),
			verts.fIndex( 0, verts.fDimZ( ) - 1 ).fLogicalZ( ) );

		etg->fEndEditingVerts( verts );
	}

	void tHeightFieldVertexPaintCursor::fBeginAction( )
	{
		mCurrentAction.fReset( new tModifyTerrainHeightAction( fMainWindow( ), mPaintEntity ) );
		fMainWindow( ).fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( mCurrentAction.fGetRawPtr( ) ) );
	}

	void tHeightFieldVertexPaintCursor::fEndAction( )
	{
		mCurrentAction->fEnd( );
		mCurrentAction.fRelease( );
	}

	//------------------------------------------------------------------------------
	// tHeightFieldMaterialPaintCursor
	//------------------------------------------------------------------------------

	tHeightFieldMaterialPaintCursor::tHeightFieldMaterialPaintCursor( tEditorCursorControllerButton* button )
		: tHeightFieldPaintCursor( button )
		, mLuminosity( 1.f )
	{
	}

	tHeightFieldMaterialPaintCursor::~tHeightFieldMaterialPaintCursor( )
	{
	}

	void tHeightFieldMaterialPaintCursor::fDoPaintAction( )
	{
		tEditableTerrainGeometry* etg = mCurrentHoverObject->fDynamicCast< tEditableTerrainGeometry >( );

		const Math::tVec3f centerInLocal = etg->fWorldToObject( ).fXformPoint( mLastHoverIntersection );
		const f32 radius = fComputeRadius( );
		const f32 dt = fMainWindow( ).fGuiApp( ).fGetFrameDeltaTime( );

		const f32 centerU = etg->fRez( ).fWorldXUV( centerInLocal.x );
		const f32 centerV = etg->fRez( ).fWorldZUV( centerInLocal.z );
		const f32 minU = etg->fRez( ).fWorldXUV( centerInLocal.x - radius );
		const f32 maxU = etg->fRez( ).fWorldXUV( centerInLocal.x + radius );
		const f32 minV = etg->fRez( ).fWorldZUV( centerInLocal.z - radius );
		const f32 maxV = etg->fRez( ).fWorldZUV( centerInLocal.z + radius );

		fPaintMaterial( tPaintMaterialArgs( 
			etg->fGetMaskTexture( ), 
			etg->fGetMtlIdsTexture( ),
			centerU,
			centerV,
			minU,
			maxU,
			minV,
			maxV,
			dt ) );
	}

	void tHeightFieldMaterialPaintCursor::fBeginAction( )
	{
		mCurrentAction.fReset( new tModifyTerrainMaterialAction( fMainWindow( ), mPaintEntity ) );
		fMainWindow( ).fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( mCurrentAction.fGetRawPtr( ) ) );
	}

	void tHeightFieldMaterialPaintCursor::fEndAction( )
	{
		mCurrentAction->fEnd( );
		mCurrentAction.fRelease( );
	}

	///
	/// \class tModifyTerrainGroundCoverAction
	/// \brief 
	class tModifyTerrainGroundCoverAction : public tEditorButtonManagedCursorAction
	{
	public:

		tModifyTerrainGroundCoverAction( 
			tToolsGuiMainWindow& mainWindow, 
			const tEntityPtr & entity,
			u32 gcId )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mEntity( entity )
			, mStart( gcId )
			, mEnd( gcId )
		{
			mEntity->fDynamicCast<tEditableTerrainGeometry>( )->fSaveGroundCover( mStart );
			fSetIsLive( true );
		}

		virtual void fUndo( )
		{
			mEntity->fDynamicCast<tEditableTerrainGeometry>( )->fRestoreGroundCover( mStart );


		}

		virtual void fRedo( )
		{
			mEntity->fDynamicCast<tEditableTerrainGeometry>( )->fRestoreGroundCover( mEnd );
		}

		virtual void fEnd( )
		{
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fSaveGroundCover( mEnd );
			tEditorButtonManagedCursorAction::fEnd( );
		}

	private:

		tEntityPtr mEntity;
		tTerrainGeometry::tGroundCover mStart, mEnd;
	};

	//------------------------------------------------------------------------------
	// tHeightFieldGroundCoverPaintCursor
	//------------------------------------------------------------------------------
	tHeightFieldGroundCoverPaintCursor::tHeightFieldGroundCoverPaintCursor( 
		tEditorCursorControllerButton* button )
		: tHeightFieldPaintCursor( button )
		, mPaintLayer( 0 )
		, mRenderCursorGrid( false )
	{
	}

	//------------------------------------------------------------------------------
	tHeightFieldGroundCoverPaintCursor::~tHeightFieldGroundCoverPaintCursor( )
	{
	}

	//------------------------------------------------------------------------------
	void tHeightFieldGroundCoverPaintCursor::fOnRenderCursor( 
		f32 radius, 
		const Math::tVec3f & centerInLocal, 
		tEditableTerrainGeometry * etg,
		u32 innerRingColor,
		tGrowableArray< Gfx::tSolidColorRenderVertex > & renderVerts )
	{
		if( !mRenderCursorGrid || !mPaintLayer )
			return;

		const f32 radiusSq = Math::fSquare( radius );
		const f32 unitSize = mPaintLayer->fUnitSize( );
		const u32 paintUnits = mPaintLayer->fPaintUnits( );
		const f32 paintSize = unitSize * paintUnits;

		const u32 dimx = fRound<u32>( etg->fRez( ).fWorldLengthX( ) / unitSize );
		const u32 dimz = fRound<u32>( etg->fRez( ).fWorldLengthZ( ) / unitSize );

		const f32 minLocalX = centerInLocal.x - radius;
		const f32 maxLocalX = centerInLocal.x + radius;
		const f32 minLocalZ = centerInLocal.z - radius;
		const f32 maxLocalZ = centerInLocal.z + radius;

		const u32 centerPX = fRound<u32>( ( etg->fRez( ).fWorldXUV( centerInLocal.x ) * etg->fRez( ).fWorldLengthX( ) - 0.5f * unitSize ) / paintSize );
		const u32 centerPZ = fRound<u32>( ( etg->fRez( ).fWorldZUV( centerInLocal.z ) * etg->fRez( ).fWorldLengthZ( ) - 0.5f * unitSize ) / paintSize );
		const u32 paintRadius = fRoundUp<u32>( radius / paintSize );
		const u32 minPX = centerPX > paintRadius ? centerPX - paintRadius : 0;
		const u32 maxPX = fMin( centerPX + paintRadius, ( dimx / paintUnits ) );
		const u32 minPZ = centerPZ > paintRadius ? centerPZ - paintRadius : 0;
		const u32 maxPZ = fMin( centerPZ + paintRadius, ( dimz / paintUnits ) );

		const Math::tMat3f & worldXform = etg->fObjectToWorld( );

		u32 paintColor = Gfx::tVertexColor( 0, 0, 0xff ).fForGpu( );

		// Paint Unit Square
		for( u32 z = minPZ; z <= maxPZ; ++z )
		{
			f32 zP = z * paintSize - 0.5f * etg->fRez( ).fWorldLengthZ( );
			for( u32 x = minPX; x <= maxPX; ++x )
			{
				f32 xP = x * paintSize - 0.5f * etg->fRez( ).fWorldLengthX( );
		
				Math::tVec3f ul, ur, ll, lr;

				ul.x = xP;
				ul.z = zP;
				ul.y = etg->fSampleHeight( ul.x, ul.z ) + cCursorHeightOffset;

				ur.x = xP + paintSize;
				ur.z = zP;
				ur.y = etg->fSampleHeight( ur.x, ur.z ) + cCursorHeightOffset;

				ll.x = xP;
				ll.z = zP + paintSize;
				ll.y = etg->fSampleHeight( ll.x, ll.z ) + cCursorHeightOffset;

				lr.x = xP + paintSize;
				lr.z = zP + paintSize;
				lr.y = etg->fSampleHeight( lr.x, lr.z ) + cCursorHeightOffset;

				fSubDivideCursorLine( etg, ul, ur, renderVerts, paintColor, 0 );
				fSubDivideCursorLine( etg, ur, lr, renderVerts, paintColor, 0 );
				fSubDivideCursorLine( etg, lr, ll, renderVerts, paintColor, 0 );
				fSubDivideCursorLine( etg, ll, ul, renderVerts, paintColor, 0 );

				// Add interior squares for potentially affected cells
				if( paintUnits > 1 && 

					// Corner inside circle
					( ul - centerInLocal ).fXZLengthSquared( ) < radiusSq ||
					( ur - centerInLocal ).fXZLengthSquared( ) < radiusSq ||
					( ll - centerInLocal ).fXZLengthSquared( ) < radiusSq ||
					( lr - centerInLocal ).fXZLengthSquared( ) < radiusSq ||

					// Center contained
					( centerInLocal.x >= ul.x && centerInLocal.x <= ur.x &&
					  centerInLocal.z >= ul.z && centerInLocal.z <= ll.z  ) ||

					// Circle Extent contained
					( minLocalX >= ul.x && minLocalX <= ur.x &&
					  centerInLocal.z >= ul.z && centerInLocal.z <= ll.z  ) ||
					( maxLocalX >= ul.x && maxLocalX <= ur.x &&
					  centerInLocal.z >= ul.z && centerInLocal.z <= ll.z  ) ||
					( centerInLocal.x >= ul.x && centerInLocal.x <= ur.x &&
					  minLocalZ >= ul.z && minLocalZ <= ll.z  ) ||
					( centerInLocal.x >= ul.x && centerInLocal.x <= ur.x &&
					  maxLocalZ >= ul.z && maxLocalZ <= ll.z  ) )
				{
					
					Math::tVec3f top( ul.x + 0.5f * unitSize, 0, ul.z );
					Math::tVec3f bot( ll.x + 0.5f * unitSize, 0, ll.z );
					for( u32 v = 0; v < paintUnits; ++v, top.x += unitSize, bot.x += unitSize )
					{
						top.y = etg->fSampleHeight( top.x, top.z ) + cCursorHeightOffset;
						bot.y = etg->fSampleHeight( bot.x, bot.z ) + cCursorHeightOffset;

						fSubDivideCursorLine( etg, top, bot, renderVerts, innerRingColor, 0 );			
					}


					Math::tVec3f left( ul.x, 0, ul.z + 0.5f * unitSize );
					Math::tVec3f right( ur.x, 0, ur.z + 0.5f * unitSize );
					for( u32 h = 0; h < paintUnits; ++h, left.z += unitSize, right.z += unitSize )
					{
						left.y = etg->fSampleHeight( left.x, left.z ) + cCursorHeightOffset;
						right.y = etg->fSampleHeight( right.x, right.z ) + cCursorHeightOffset;

						fSubDivideCursorLine( etg, left, right, renderVerts, innerRingColor, 0 );			
					}
				}
			}
		}

	}

	//------------------------------------------------------------------------------
	void tHeightFieldGroundCoverPaintCursor::fDoPaintAction( )
	{
		if( !mPaintLayer )
			return;

		tEditableTerrainGeometry * etg = mCurrentHoverObject->fDynamicCast< tEditableTerrainGeometry >( );
		
		const f32 radius = fComputeRadius( );
		const Math::tVec3f centerInLocal = etg->fWorldToObject( ).fXformPoint( mLastHoverIntersection );

		tEditableTerrainGeometry::tEditableGroundCoverMask mask;
		etg->fBeginEditingGroundCoverMask( *mPaintLayer, mask, centerInLocal, 2 * radius, 2 * radius );

		if( !mask.fDimZ( ) || !mask.fDimX( ) )
		{
			etg->fEndEditingGroundCoverMask( mask );
			return;
		}

		const Input::tKeyboard& kb = Input::tKeyboard::fInstance( );
		const b32 shiftHeld =	kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) || 
								kb.fButtonHeld( Input::tKeyboard::cButtonRShift );
		const f32 strength  =	fComputeStrength( );
		const f32 falloff	=	fComputeFalloff( );
		const f32 shape		=	fComputeShape( );

		tPaintGroundCoverArgs args( mask, shiftHeld ? -1 : 1, fMainWindow( ).fGuiApp( ).fGetFrameDeltaTime( ) );
		
		const u32 xdim = mask.fDimX( );
		const u32 zdim = mask.fDimZ( );
		const f32 unitSize = mPaintLayer->fUnitSize( );
		
		f32 xPos = mask.fMinX( );
		for( args.mX = 0; args.mX < xdim; ++args.mX, xPos += unitSize )
		{
			f32 zPos = mask.fMinZ( );
			for( args.mZ = 0; args.mZ < zdim; ++args.mZ, zPos += unitSize )
			{
				const f32 circleDistToCenter = Math::fSqrt( 
					Math::fSquare( xPos - centerInLocal.x ) + 
					Math::fSquare( zPos - centerInLocal.z ) );
				const f32 squareDistToCenter = fMax( 
					fAbs( xPos - centerInLocal.x ), 
					fAbs( zPos - centerInLocal.z ) );
				const f32 distToCenter = Math::fLerp( circleDistToCenter, squareDistToCenter, shape );

				const f32 distToCenterNorm = fMin( 1.f, fMax( 0.f, distToCenter / radius ) );
				const f32 distBasedStrength = 1.f - std::powf( distToCenterNorm, falloff );
				args.mPaintStrength = strength * distBasedStrength;

				// Do the operation
				fModifyDensityTexel( args );
			}
		}

		etg->fEndEditingGroundCoverMask( mask );
	}

	//------------------------------------------------------------------------------
	void tHeightFieldGroundCoverPaintCursor::fBeginAction( )
	{
		if( !mPaintLayer )
			return;

		mCurrentAction.fReset( 
			new tModifyTerrainGroundCoverAction( fMainWindow( ), mPaintEntity, mPaintLayer->fUniqueId( ) ) );

		fMainWindow( ).fGuiApp( ).fActionStack( ).fAddAction( 
			tEditorActionPtr( mCurrentAction.fGetRawPtr( ) ) );
	}

	//------------------------------------------------------------------------------
	void tHeightFieldGroundCoverPaintCursor::fEndAction( )
	{
		if( mCurrentAction )
		{
			mCurrentAction->fEnd( );
			mCurrentAction.fRelease( );
		}
	}
}

