#include "GameAppPch.hpp"
#include "tRtsCursorDisplay.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "tGameApp.hpp"
#include "tUnitLogic.hpp"
#include "tLevelLogic.hpp"
#include "tTurretLogic.hpp"
#include "tSceneGraphCollectTris.hpp"

using namespace Sig::Math;
namespace Sig
{
	devvar( f32, Gameplay_RtsCursor_ArrowOffset, 0.25f );
	devvar( f32, Gameplay_RtsCursor_ArrowWidth, 3.5f );
	devvar( f32, Gameplay_RtsCursor_ArrowLength, 3.5f );
	devvar( f32, Gameplay_RtsCursor_ArrowPulseMag, 0.7f );
	devvar( f32, Gameplay_RtsCursor_ArrowPulseFreq, 1.0f );
	devvar( f32, Gameplay_RtsCursor_RangeRingWidth, 0.3f );
	devvar( u32, Gameplay_RtsCursor_RangeRingSegments, 50 );
	devvar( u32, Gameplay_RtsCursor_RangeLegSegments, 15 );
	devvar( f32, Gameplay_RtsCursor_RangeFadeDist, 5.0f );
	devvar( f32, Gameplay_RtsCursor_RangeFadeStart, 3.0f );

	devvar( f32, Gameplay_RtsCursor_RangeFadeLerp, 0.25f );
	devvar( bool, Gameplay_RtsCursor_UseTerrainIndex, false );


	namespace
	{
		struct tRtsCursorDisplayRayCastCallback
		{
			mutable Math::tRayCastHit	mHit;
			mutable tEntity*			mFirstEntity;
			tEntity*					mIgnoreEntity;
			u32							mGroundMask;

			explicit tRtsCursorDisplayRayCastCallback( tEntity& ignore, u32 groundMask ) : mFirstEntity( 0 ), mIgnoreEntity( &ignore ), mGroundMask( groundMask )
			{ }

			inline void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;
				tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i );

				if( !Gameplay_RtsCursor_UseTerrainIndex && !spatial->fHasGameTagsAny( mGroundMask ) )
					return;

				//if( spatial == mIgnoreEntity || spatial->fIsAncestorOfMine( *mIgnoreEntity ) )
				//	return;
				if( i->fQuickRejectByBox( ray ) )
					return;

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( hit.fHit( ) && hit.mT < mHit.mT )
				{
					mHit			= hit;
					mFirstEntity	= spatial;
				}
			}
		};
	}

	f32 tRtsCursorDisplay::fRangeRingFadeLerp( ) 
	{ 
		return Gameplay_RtsCursor_RangeFadeLerp; 
	}

	tRtsCursorDisplay::tRtsCursorDisplay( tPlayer& player )
		: mRenderState( Gfx::tRenderState::cDefaultColorTransparent )
		, mRenderStateRange( Gfx::tRenderState::cDefaultColorTransparent )
		, mRingsAlpha( 0.f )
		, mPulseTime( 0.f )
		, mRangePulseTime( 0.f )
		, mPlayer( player )
		, mGroundHeightStartAngle( 0 )
		, mGroundHeightAngleRange( 1 )
		, mRangeRingsDirty( false )
	{
		fSetupLines( );
		fSetupQuads( );
		fSetupRangeQuads( );
		mRenderState.fEnableDisable( Gfx::tRenderState::cDepthBuffer, false );

		//mHoverText.fReset( NEW Gui::tHoverText( tGameApp::fInstance( ).fLocalUsers( ) ) );
		mHoverText.fReset( NEW Gui::tHoverText( player.fUser( ) ) );
		mHoverText->fSpawn( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );

		mUI.fReset( NEW Gui::tRtsCursorUI( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptRtsCursorUI ) ) );
		tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ).fAddChild( mUI->fCanvas( ) );	
		mUI->fCanvas( ).fCanvas( )->fSetPosition( Math::tVec3f( mPlayer.fViewportRect( ).fCenter( ), 0.4f ) );
		mUI->fCanvas( ).fCanvas( )->fSetVirtualMode( player.fUser( )->fIsViewportVirtual( ) );
	}
	tRtsCursorDisplay::~tRtsCursorDisplay( )
	{
	}
	void tRtsCursorDisplay::fSetupLines( )
	{
		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );

		mCursorLines.fReset( NEW Gfx::tWorldSpaceLines( ) );
		mCursorLines->fResetDeviceObjects( tGameApp::fInstance( ).fScreen( )->fGetDevice( ), allocator.mSolidColorMaterial, allocator.mSolidColorGeomAllocator, allocator.mIndexAllocator );
		mCursorLines->fSetLockedToParent( false );
		mCursorLines->fSpawn( tGameApp::fInstance( ).fSceneGraph( )->fRootEntity( ) );
		mCursorLines->fMoveTo( tMat3f::cIdentity );
		mCursorLines->fSetRenderStateOverride( &mRenderState );

		mCursorLinesForOtherPlayer.fRelease( );
		if( tGameApp::fInstance( ).fGameMode( ).fIsMultiPlayer( ) ) 
		{
			mCursorLinesForOtherPlayer.fReset( NEW Gfx::tWorldSpaceLines( ) );
			mCursorLinesForOtherPlayer->fResetDeviceObjects( tGameApp::fInstance( ).fScreen( )->fGetDevice( ), allocator.mSolidColorMaterial, allocator.mSolidColorGeomAllocator, allocator.mIndexAllocator );
			mCursorLinesForOtherPlayer->fSetLockedToParent( false );
			mCursorLinesForOtherPlayer->fSpawn( tGameApp::fInstance( ).fSceneGraph( )->fRootEntity( ) );
			mCursorLinesForOtherPlayer->fMoveTo( tMat3f::cIdentity );
			mCursorLinesForOtherPlayer->fSetRenderStateOverride( &mRenderState );
		}
	}
	void tRtsCursorDisplay::fSetupQuads( )
	{
		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );
		tResourcePtr colorMapResource = tGameApp::fInstance( ).fRtsCursorTexture( tGameApp::cRtsCursorTextureIDArrow );

		mCursorQuads.fReset( NEW Gfx::tWorldSpaceQuads( ) );
		mCursorQuads->fResetDeviceObjectsTexture( tGameApp::fInstance( ).fScreen( )->fGetDevice( )
			, colorMapResource
			, allocator.mFullBrightMaterialFile
			, allocator.mFullBrightGeomAllocator
			, allocator.mIndexAllocator );

		mCursorQuads->fSetLockedToParent( false );
		mCursorQuads->fSpawn( tGameApp::fInstance( ).fSceneGraph( )->fRootEntity( ) );
		mCursorQuads->fMoveTo( tMat3f::cIdentity );
		mCursorQuads->fGeometry( ).fSetRenderStateOverride( &mRenderState );
	}
	void tRtsCursorDisplay::fSetupRangeQuads( )
	{
		mRenderStateRange.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );
		mRenderStateRange.fSetDepthBias( std::numeric_limits<s8>::min( ) );
		//mRenderStateRange.fEnableDisable( Gfx::tRenderState::cDepthBuffer, false );

		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );
		tResourcePtr colorMapResource = tGameApp::fInstance( ).fRtsCursorTexture( tGameApp::cRtsCursorTextureIDRange );

		mCursorRangeQuads.fReset( NEW Gfx::tWorldSpaceQuads( ) );
		mCursorRangeQuads->fResetDeviceObjectsTexture( tGameApp::fInstance( ).fScreen( )->fGetDevice( )
			, colorMapResource
			, allocator.mFullBrightMaterialFile
			, allocator.mFullBrightGeomAllocator
			, allocator.mIndexAllocator );

		mCursorRangeQuads->fSetLockedToParent( false );
		mCursorRangeQuads->fSpawn( tGameApp::fInstance( ).fSceneGraph( )->fRootEntity( ) );
		mCursorRangeQuads->fMoveTo( tMat3f::cIdentity );
		mCursorRangeQuads->fGeometry( ).fSetRenderStateOverride( &mRenderStateRange );
	}
	void tRtsCursorDisplay::fSetVisibility( tVisibility visible )
	{
		mVisbility = visible;
		b32 displayCaseMode = tGameApp::fInstance( ).fCurrentLevel( ) && tGameApp::fInstance( ).fCurrentLevel( )->fIsDisplayCase( );
		if( visible == cShowNothing )
		{
			mCursorLines->fSetInvisible( true );
			if( mCursorLinesForOtherPlayer ) 
				mCursorLinesForOtherPlayer->fSetInvisible( true );
			mCursorQuads->fSetInvisible( true );
			mCursorRangeQuads->fSetInvisible( true );
			mHoverText->fSetVisibility( false );
		}
		else if ( visible == cShowJustRings )
		{
			mCursorLines->fSetInvisible( true );
			if( mCursorLinesForOtherPlayer ) 
				mCursorLinesForOtherPlayer->fSetInvisible( true );
			mCursorQuads->fSetInvisible( true );
			mHoverText->fSetVisibility( false );

			if( !displayCaseMode )
				mCursorRangeQuads->fSetInvisible( false );
			else
				mCursorRangeQuads->fSetInvisible( true );
		}
		else if ( visible == cShowEverything )
		{
			mCursorLines->fSetInvisible( false );
			if( mCursorLinesForOtherPlayer ) 
				mCursorLinesForOtherPlayer->fSetInvisible( false );
			mCursorQuads->fSetInvisible( false );
			mHoverText->fSetVisibility( true );

			if( !displayCaseMode )
				mCursorRangeQuads->fSetInvisible( false );
			else
				mCursorRangeQuads->fSetInvisible( true );
		}

	}
	void tRtsCursorDisplay::fUpdate( f32 dt, const tMat3f& cursorMatrix, f32 defaultRadius, tEntity* hover, b32 showStats, b32 isGhost )
	{
		tUnitLogic* hoverLogic = hover ? hover->fLogicDerived<tUnitLogic>( ) : 0;

		mHoverText->fSetHoverUnit( showStats && !mPlayer.fUser( )->fViewport( )->fIsVirtual( ) ? hoverLogic : NULL );
		
		tAabbf bounds;
		bounds.fInvalidate( );

		const Gfx::tVertexColor centerRingColor = Gfx::tVertexColor( 0x00, 0x00, 0x00 );
		const Gfx::tVertexColor currentPlayerRingColor = Gfx::tVertexColor( 0xff, 0xff, 0xff );
		const Gfx::tVertexColor allyPlayerRingColor = Gfx::tVertexColor( 0x0c, 0xff, 0x0c ); // nice bright green
		const Gfx::tVertexColor enemyPlayerRingColor = Gfx::tVertexColor( 0xff, 0x0c, 0x0c ); // angry red!

		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		tGrowableArray< Gfx::tSolidColorRenderVertex > altVerts;
		Gfx::tVertexColor otherPlayerRingColor;
		f32 targetAlpha = 0.0f;

		if( tGameApp::fInstance( ).fGameMode( ).fIsCoOp( ) )
		{
			otherPlayerRingColor = allyPlayerRingColor;
		}
		else if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
		{
			otherPlayerRingColor = enemyPlayerRingColor;
		}

		//////////////////////////////////////////////////////////////////////////
		// Set up multiple ring sets using altVerts and mCursorLinesForOtherPlayer

		if( hoverLogic )
		{	
			tVec3f textOffset = cursorMatrix.fZAxis( ) * hoverLogic->fSelectionRadius( );

			const tVec3f hoverPos = hover->fObjectToWorld( ).fGetTranslation( );
			tMat3f cursorMatrixCopy = cursorMatrix;
			cursorMatrixCopy.fSetTranslation( hoverPos );

			// pulsing arrows
			mPulseTime += dt;
			while( mPulseTime >= 1.0f )
				mPulseTime -= 1.0f;

			if( mPlayer.fGameController( )->fMode( ) == tGameController::GamePad )
			{
				fPushIndicatorQuads( bounds, cursorMatrixCopy, hoverLogic->fSelectionRadius( ) );
			}
			mHoverText->fMoveTo( hoverPos - textOffset );


			// selection lines
			if( mPlayer.fGameController( )->fMode( ) == tGameController::GamePad )
			{
				fAddRing( verts, cursorMatrixCopy, hoverLogic->fSelectionRadius( ), currentPlayerRingColor );
			}

			if( tGameApp::fInstance( ).fGameMode( ).fIsMultiPlayer( ) ) 
				fAddRing( altVerts, cursorMatrixCopy, hoverLogic->fSelectionRadius( ), otherPlayerRingColor );
			
			// Range rings
			/*mRangePulseTime += dt;
			if( mRangePulseTime > 1.0f )
				mRangePulseTime = -3.0f;*/

			tTurretLogic* turretLogic = hoverLogic->fDynamicCast<tTurretLogic>( );
			f32 maxRange = hoverLogic->fWeaponMaxRange( );
			if( turretLogic && !turretLogic->fDisableYawConstraintAdjust( ) && !fEqual( maxRange, 0.0f ) ) 
			{
				// draw range rings
				targetAlpha = 1.0f;

				s32 currentQuadrant = hoverLogic->fYawConstraintQuadrant( );
				if( hoverLogic->fConstrainYaw( ) && currentQuadrant != mLastRangeRingQuadrant )
					mRangeRingsDirty = true;

				if( mRangeRingsDirty )
				{
					mRangeRingsDirty = false;
					f32 minRange = hoverLogic->fWeaponMinRange( );

					mLastRangeRingBounds.fInvalidate( );
					mLastRangeRingQuadrant = currentQuadrant;
					fPushRangeQuads( mLastRangeRingBounds, cursorMatrixCopy, minRange, maxRange, *hover, isGhost );

					if( hoverLogic->fHasWeaponStation( 0 ) )
						hoverLogic->fWeaponStation( 0 )->fSetGroundSamples( mGroundHeightStartAngle, mGroundHeightAngleRange, mGroundHeightsNear, mGroundHeightsFar );
				}
			}
			else
			{
				mCursorRangeQuads->fSetQuadCount( 0 );
				mLastRangeRingBounds.fInvalidate( );
			}

			if( hoverLogic->fHasWeaponStation( 0 ) )
			{
				const tWeaponStationPtr& station = hoverLogic->fWeaponStation( 0 );
				sigassert( station );
				if( station->fShellCaming( ) )
					targetAlpha = 0.f;
			}

		}
		else
		{
			// clear everything
			if( mPlayer.fGameController( )->fMode( ) == tGameController::GamePad )
			{
				fAddRing( verts, cursorMatrix, 0.5f, centerRingColor );
				fAddRing( verts, cursorMatrix, defaultRadius, currentPlayerRingColor );
			}

			if( tGameApp::fInstance( ).fGameMode( ).fIsMultiPlayer( ) )
			{
				fAddRing( altVerts, cursorMatrix, 0.5f, centerRingColor );
				fAddRing( altVerts, cursorMatrix, defaultRadius, otherPlayerRingColor );
			}

			mCursorQuads->fSetQuadCount( 0 );
			mCursorRangeQuads->fSetQuadCount( 0 );
			mLastRangeRingBounds.fInvalidate( );
			mPulseTime = 0.0f;
		}

		bounds |= mLastRangeRingBounds;

		mCursorLines->fSetGeometry( verts, false );
		mCursorLines->fSetViewportMask( 0 );
		mCursorLines->fEnableViewport( mPlayer.fUser( )->fViewport( )->fViewportIndex( ), true );
		
		if( mCursorLinesForOtherPlayer ) 
		{
			mCursorLinesForOtherPlayer->fSetGeometry( altVerts, false );
			mCursorLinesForOtherPlayer->fSetViewportMask( 0 );
			tDynamicArray< tPlayerPtr > otherPlayers = tGameApp::fInstance( ).fPlayers( );
			for( tDynamicArray< tPlayerPtr >::tIterator player = otherPlayers.fBegin( ); player != otherPlayers.fEnd( ); ++player )
			{
				if( (*player).fGetRawPtr( ) == &mPlayer ) continue;
				mCursorLinesForOtherPlayer->fEnableViewport( (*player)->fUser( )->fViewport( )->fViewportIndex( ), true );
			}
		}

		mCursorQuads->fSetObjectSpaceBox( bounds );
		mCursorQuads->fCreateGeometry( *tGameApp::fInstance( ).fScreen( )->fGetDevice( ) );	

		mCursorRangeQuads->fSetObjectSpaceBox( bounds );
		mCursorRangeQuads->fCreateGeometry( *tGameApp::fInstance( ).fScreen( )->fGetDevice( ) );

		mRingsAlpha = fLerp( mRingsAlpha, targetAlpha, fRangeRingFadeLerp( ) );
		Gfx::tRenderableEntity::fSetRgbaTint( *mCursorRangeQuads, Math::tVec4f( Math::tVec3f( 1 ), mRingsAlpha ) );
	}
	namespace
	{
		f32 fMapAngleToAlpha( f32 angle )
		{
			if( fInBounds( angle, cPiOver2, c3PiOver2 ) )
				return 1.f;
			if( fInBounds( angle, 0.f, cPiOver2 ) )
				return fMax( 0.f, 1.f - ( cPiOver2 - angle ) / cPiOver4 );
			if( fInBounds( angle, c3PiOver2, c2Pi ) )
				return fMax( 0.f, 1.f - ( angle - c3PiOver2 ) / cPiOver4 );
			return 0.f;
		}
	}
	void tRtsCursorDisplay::fAddRing( tGrowableArray< Gfx::tSolidColorRenderVertex >& verts, const tMat3f& xform, f32 radius, const Gfx::tVertexColor& color )
	{
		const tVec3f pos = xform.fGetTranslation( );
		const tVec3f xAxis = radius * xform.fXAxis( );
		const tVec3f zAxis = radius * xform.fZAxis( );
		const u32 numSegments = 60;

		for( u32 i = 0; i < numSegments; ++i )
		{
			Gfx::tSolidColorRenderVertex back;
			if( i == 0 )
				back = Gfx::tSolidColorRenderVertex( pos + zAxis, Gfx::tVertexColor( color.mR/255.f, color.mG/255.f, color.mB/255.f, fMapAngleToAlpha( 0.f ) * color.mA/255.f ).fForGpu( ) );
			else
				back = verts.fBack( );
			verts.fPushBack( back );

			const f32 angle = c2Pi * ( ( i + 1.f ) / ( numSegments - 0.f ) );
			const tVec3f p0 = pos + sinf( angle ) * xAxis + cosf( angle ) * zAxis;
			verts.fPushBack( Gfx::tSolidColorRenderVertex( p0, Gfx::tVertexColor( color.mR/255.f, color.mG/255.f, color.mB/255.f, fMapAngleToAlpha( angle ) * color.mA/255.f ).fForGpu( ) ) );
		}
	}
	void tRtsCursorDisplay::fPushIndicatorQuads( tAabbf& bounds, const tMat3f& xform, f32 radius )
	{
		tVec3f origin = xform.fGetTranslation( );
		tVec3f arrowAxis = xform.fXAxis( );

		f32 pulseDist = (sin( c2Pi * mPulseTime * Gameplay_RtsCursor_ArrowPulseFreq ) + 1.0f) * Gameplay_RtsCursor_ArrowPulseMag;
		radius += Gameplay_RtsCursor_ArrowOffset + pulseDist;
		tVec3f p1 = origin + arrowAxis * radius;
		tVec3f p2 = origin + arrowAxis * -radius;

		u32 qIndex = 0;
		mCursorQuads->fSetQuadCount( 2 );
		mCursorQuads->fSetViewportMask( 0 );
		mCursorQuads->fEnableViewport( mPlayer.fUser( )->fViewport( )->fViewportIndex( ), true );
		fPushIndicatorQuad( mCursorQuads->fQuad( qIndex++ ), bounds, p1, -arrowAxis, xform.fZAxis( ) );
		fPushIndicatorQuad( mCursorQuads->fQuad( qIndex++ ), bounds, p2, arrowAxis, -xform.fZAxis( ) );
	}
	void tRtsCursorDisplay::fPushIndicatorQuad( Gfx::tFullBrightRenderVertex *verts, tAabbf& bounds, const tVec3f& center, const tVec3f& zAxis, const tVec3f& xAxis )
	{
		tVec3f z = zAxis * Gameplay_RtsCursor_ArrowLength;
		tVec3f x = xAxis * Gameplay_RtsCursor_ArrowWidth / 2.0f;

		const u32 color = 0xffffffff;
		verts[0].mP = center - z - x;
		verts[0].mColor = color;
		verts[0].mUv = tVec2f( 1.0f, 0.0f );
		bounds |= verts[0].mP;

		verts[1].mP = center - z + x;
		verts[1].mColor = color;
		verts[1].mUv = tVec2f( 0.0f, 0.0f );
		bounds |= verts[1].mP;

		verts[2].mP = center + x;
		verts[2].mColor = color;
		verts[2].mUv = tVec2f( 0.0f, 1.0f );
		bounds |= verts[2].mP;

		verts[3].mP = center - x;;
		verts[3].mColor = color;
		verts[3].mUv = tVec2f( 1.0f, 1.0f );
		bounds |= verts[3].mP;
	}
	void fCorrectForGroundHeight( tEntity& owner, tVec3f& pt )
	{
		const tVec3f rayDelta( 0, -30, 0 );
		Math::tRayf ray( pt - rayDelta, rayDelta * 5.0f ); // two times down
		tRtsCursorDisplayRayCastCallback rayCastCallback( owner, GameFlags::cFLAG_SHOW_RANGE_RINGS );

		if( Gameplay_RtsCursor_UseTerrainIndex )
			tGameApp::fInstance( ).fSceneGraph( )->fRayCast( ray, rayCastCallback, Gfx::tRenderableEntity::cHeightFieldSpatialSetIndex );
		else
			tGameApp::fInstance( ).fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastCallback );

		if( rayCastCallback.mHit.fHit( ) )
			pt.y = ray.fEvaluate( rayCastCallback.mHit.mT ).y;
	}
	void tRtsCursorDisplay::fPushRangeQuads( tAabbf& bounds, const tMat3f& xform, f32 minRange, f32 maxRange, tEntity& hover, b32 isGhost )
	{
		u32 quadIndexStart = 0;

		tTurretLogic* turret = hover.fLogicDerived<tTurretLogic>( );
		if( turret )
		{
			//const b32 pulsing = mRangePulseTime > 0 && mRangePulseTime < 1 && isGhost;

			if( !turret->fConstrainYaw( ) )
			{
				u32 count = Gameplay_RtsCursor_RangeRingSegments;
				//if( pulsing )
				//	count += Gameplay_RtsCursor_RangeRingSegments;
				mCursorRangeQuads->fSetQuadCount( count );

				//if( pulsing )
				//	fPushRangeArc( bounds, xform, Math::fLerp( minRange, maxRange, mRangePulseTime ), hover, 0, c2Pi, quadIndexStart, NULL );
				fPushRangeArc( bounds, xform, maxRange, hover, 0, c2Pi, quadIndexStart, &mGroundHeightsFar );
				
				if( mGroundHeightsNear.fCount( ) != mGroundHeightsFar.fCount( ) )
					mGroundHeightsNear.fSetCount( mGroundHeightsFar.fCount( ) );
				mGroundHeightsNear.fFill( hover.fObjectToWorld( ).fGetTranslation( ).y );
			}
			else
			{
				u32 count = Gameplay_RtsCursor_RangeRingSegments + Gameplay_RtsCursor_RangeLegSegments * 2;
				if( minRange != cInfinity )
					count += Gameplay_RtsCursor_RangeRingSegments;
				//if( pulsing )
				//	count += Gameplay_RtsCursor_RangeRingSegments;

				mCursorRangeQuads->fSetQuadCount( count );
				f32 startA = turret->fConstraintStartAngle( );
				f32 rangeA = turret->fConstraintRange( );
				
				if( minRange != cInfinity )
					fPushRangeArc( bounds, xform, minRange, hover, startA, rangeA, quadIndexStart, &mGroundHeightsNear );
				//if( pulsing )
				//	fPushRangeArc( bounds, xform, Math::fLerp( minRange, maxRange, mRangePulseTime ), hover, startA, rangeA, quadIndexStart, NULL );

				fPushRangeArc( bounds, xform, maxRange, hover, startA, rangeA, quadIndexStart, &mGroundHeightsFar );
				fPushRangeLeg( bounds, xform, maxRange, startA, hover, quadIndexStart );
				fPushRangeLeg( bounds, xform, maxRange, startA + rangeA, hover, quadIndexStart );				
			}
		}
		else
		{
			mCursorRangeQuads->fSetQuadCount( Gameplay_RtsCursor_RangeRingSegments );
			fPushRangeArc( bounds, xform, maxRange, hover, 0, c2Pi, quadIndexStart, NULL );
		}

		// Only show on current player's viewport
		mCursorRangeQuads->fSetViewportMask( 0 );
		mCursorRangeQuads->fEnableViewport( mPlayer.fUser( )->fViewport( )->fViewportIndex( ), true );
	}
	void tRtsCursorDisplay::fPushRangeArc( tAabbf& bounds, const tMat3f& xform, f32 radius, tEntity& hover, f32 startAngle, f32 angRange, u32& startIndex, tGrowableArray<f32>* samplesOut )
	{
		tVec3f origin = xform.fGetTranslation( );

		f32 rate = angRange / Gameplay_RtsCursor_RangeRingSegments;

		f32 angle0 = startAngle;
		tVec3f axis0;
		axis0.fSetXZHeading( angle0 );
		tVec3f p0 = origin + axis0 * radius;
		axis0 *= Gameplay_RtsCursor_RangeRingWidth;
		f32 tv1 = angle0 / angRange;

		fCorrectForGroundHeight( hover, p0 );

		if( samplesOut )
		{
			mGroundHeightStartAngle = startAngle;
			mGroundHeightAngleRange = angRange;
			if( samplesOut->fCount( ) != Gameplay_RtsCursor_RangeRingSegments + 1 )
				samplesOut->fSetCount( Gameplay_RtsCursor_RangeRingSegments + 1 );
			(*samplesOut)[ 0 ] = p0.y;
		}

		for( u32 i = 0; i < Gameplay_RtsCursor_RangeRingSegments; ++i )
		{
			f32 angle1 = startAngle + (i+1) * rate;
			tVec3f axis1;
			axis1.fSetXZHeading( angle1 );
			tVec3f p1 = origin + axis1 * radius;
			axis1 *= Gameplay_RtsCursor_RangeRingWidth;
			f32 tv2 = angle1 / angRange;

			fCorrectForGroundHeight( hover, p1 );
			if( samplesOut )
				(*samplesOut)[ i + 1 ] = p1.y;

			fPushRangeQuad( mCursorRangeQuads->fQuad( startIndex++ ), bounds
				, p0, axis0, tv1, 1.f
				, p1, axis1, tv2, 1.f );

			axis0 = axis1;
			p0 = p1;
			tv1 = tv2;
		}
	}
	void tRtsCursorDisplay::fPushRangeLeg( tAabbf& bounds, const tMat3f& xform, f32 radius, f32 angle, tEntity& hover, u32& startIndex )
	{
		tVec3f origin = xform.fGetTranslation( );
		f32 rate = (radius - Gameplay_RtsCursor_RangeFadeStart) / Gameplay_RtsCursor_RangeLegSegments;

		tVec3f axis;
		axis.fSetXZHeading( angle );

		tVec3f sideAxis;
		sideAxis.fSetXZHeading( angle + cPiOver2 );
		sideAxis *= Gameplay_RtsCursor_RangeRingWidth;
		tVec3f p0 = origin + axis * Gameplay_RtsCursor_RangeFadeStart;
		f32 tv1 = 0;
		f32 alpha1 = 0;

		fCorrectForGroundHeight( hover, p0 );

		for( u32 i = 0; i < Gameplay_RtsCursor_RangeLegSegments; ++i )
		{
			f32 distance = (i+1) * rate;
			tVec3f p1 = origin + axis * (distance + Gameplay_RtsCursor_RangeFadeStart);
			f32 tv2 = distance / radius;
			f32 alpha2 = fMin( distance / Gameplay_RtsCursor_RangeFadeDist, 1.f );

			fCorrectForGroundHeight( hover, p1 );

			fPushRangeQuad( mCursorRangeQuads->fQuad( startIndex++ ), bounds
				, p0, sideAxis, tv1, alpha1
				, p1, sideAxis, tv2, alpha2 );

			p0 = p1;
			tv1 = tv2;
			alpha1 = alpha2;
		}
	}
	void tRtsCursorDisplay::fPushRangeQuad( Gfx::tFullBrightRenderVertex *verts, tAabbf& bounds
		, const tVec3f& p1, const tVec3f& axis1, f32 tv1, f32 alpha1
		, const tVec3f& p2, const tVec3f& axis2, f32 tv2, f32 alpha2 )
	{
		Gfx::tVertexColor color(255, 255, 255);
		color.mA = u8(alpha1 * 255.0f);
		u32 gpuColor1 = color.fForGpu( );

		verts[0].mP = p1 - axis1;
		verts[0].mColor = gpuColor1;
		verts[0].mUv = tVec2f( 1.0f, tv1 );
		bounds |= verts[0].mP;

		verts[1].mP = p1 + axis1;
		verts[1].mColor = gpuColor1;
		verts[1].mUv = tVec2f( 0.0f, tv1 );
		bounds |= verts[1].mP;

		color.mA = u8(alpha2 * 255.0f);
		u32 gpuColor2 = color.fForGpu( );
		verts[2].mP = p2 + axis2;
		verts[2].mColor = gpuColor2;
		verts[2].mUv = tVec2f( 0.0f, tv2 );
		bounds |= verts[2].mP;

		verts[3].mP = p2 - axis2;
		verts[3].mColor = gpuColor2;
		verts[3].mUv = tVec2f( 1.0f, tv2 );
		bounds |= verts[3].mP;
	}
}
