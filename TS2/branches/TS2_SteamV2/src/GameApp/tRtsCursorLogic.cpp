#include "GameAppPch.hpp"
#include "tRtsCursorLogic.hpp"
#include "tGameApp.hpp"
#include "tRtsCamera.hpp"
#include "tUnitLogic.hpp"
#include "tProximity.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tBuildSiteLogic.hpp"
#include "tLevelLogic.hpp"
#include "tTurretLogic.hpp"
#include "tVehicleLogic.hpp"

#include "Wwise_IDs.h"

namespace Sig
{
	devvar( bool, Gameplay_RtsCursor_SelectAllTeams, false );

	namespace
	{
		struct tGroundRayCastCallback
		{
			mutable Math::tRayCastHit	mHit;
			tEntity* mIgnore;

			explicit tGroundRayCastCallback( tEntity* ignore ) 
				: mIgnore( ignore )
			{
			}
			inline void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i );
				if( !spatial->fHasGameTagsAny( GameFlags::cFLAG_GROUND ) )
					return;
				if( i->fQuickRejectByBox( ray ) )
					return;

				if( mIgnore && (spatial == mIgnore || spatial->fIsAncestorOfMine( *mIgnore )) )
					return;

				tEntity* logicEnt = spatial->fFirstAncestorWithLogic( );
				if( logicEnt )
				{
					if( logicEnt->fLogicDerived<tTurretLogic>( ) || logicEnt->fLogicDerived<tVehicleLogic>( ) )
						return;
				}

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );

				//tGrowableArray< Math::tTrianglef > tris;
				//spatial->fCollectTris( Math::tAabbf( ray ).fInflate( 0.25f ), tris );
				//for( u32 i = 0; i < tris.fCount( ) && i < 100; ++i )
				//	spatial->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tris[ i ], Math::tVec4f(0.f,1.f,0.f,1.f) );

				if( !hit.fHit( ) || hit.mT >= mHit.mT )
				{
					//spatial->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( Math::tAabbf( ray ).fInflate( 0.25f ), Math::tVec4f(0.f,0.f,1.f,0.25f) );
					return;
				}

				mHit			= hit;
			}
		};

		struct tFilterWallPlacement : public tEntityBVH::tIntersectVolumeCallback<Math::tObbf>
		{
			mutable b32 mInZone;
			mutable b32 mTouchingOthers;
			u32 mTeam;

			tFilterWallPlacement( u32 team ) 
				: mInZone( false )
				, mTouchingOthers( false )
				, mTeam( team )
			{ }

			b32 fOkToPlace( ) const { return (mInZone && !mTouchingOthers); }

			void operator()( const Math::tObbf& v, tEntityBVH::tObjectPtr octreeObject, b32 aabbWhollyContained ) const
			{
				tSpatialEntity* test = static_cast< tSpatialEntity* >( octreeObject );
				if( !test->fHasGameTagsAny( GameFlags::cFLAG_WALL_PLACEMENT_ZONE | GameFlags::cFLAG_WALL_VOLUME ) )
					return;

				if( !fQuickAabbTest( v, octreeObject, aabbWhollyContained ) )
					return;

				if( !test->fIntersects( v ) )
					return;

				if( test->fHasGameTagsAny( GameFlags::cFLAG_WALL_PLACEMENT_ZONE ) )
					mInZone = true;
				if( test->fHasGameTagsAny( GameFlags::cFLAG_WALL_VOLUME ) )
					mTouchingOthers = true;
			}
		};

		static inline tEntity* fFilterHoverUnit( tEntity* entity, u32 team )
		{
			tShapeEntity* shape = entity->fDynamicCast< tShapeEntity >( );
			if( !shape )
				return 0;
			tEntity* unitEnt = shape->fFirstAncestorWithLogic( );
			if( !unitEnt )
				return 0;
			tUnitLogic* unitLogic = unitEnt->fLogicDerived<tUnitLogic>( );
			if( !unitLogic )
				return 0;
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			if( unitLogic->fTeam( ) != team && (level && !level->fIsDisplayCase( )) && !Gameplay_RtsCursor_SelectAllTeams )
				return 0;
			if( !unitLogic->fShouldSelect( ) )
				return 0;
			if( !unitLogic->fIsSelectionShape( *shape ) )
				return 0;
			return unitEnt;
		}
		static inline tShapeEntity* fFilterBuildSite( tEntity* entity, u32 team, b32& notCaptured )
		{
			tShapeEntity* shape = entity->fDynamicCast< tShapeEntity >( );
			if( !shape )
				return 0;
			tEntity* logicEnt = shape->fFirstAncestorWithLogic( );
			if( !logicEnt )
				return 0;
			tBuildSiteLogic* buildSiteLogic = logicEnt->fLogicDerived<tBuildSiteLogic>( );
			if( !buildSiteLogic )
				return 0;
			if( buildSiteLogic->fShape( ) != shape )
				return 0;

			if( !buildSiteLogic->fIsUsableBy( team ) )
				notCaptured = true;

			if( !buildSiteLogic->fIsValid( team ) )
				return 0;
			return shape;
		}

		static const f32 cRayCastOffsetFromGround = 10.f;
		static const f32 cRayCastToGroundRayLength = 50.f;
		static const f32 cCursorRadius = 4.f;
		static const u32 cHidePlatformGlowsGamepad = ~0;
		static const u32 cHidePlatformGlowsKeyboard = ~0 - 1;
	}

	tRtsCursorLogic::tRtsCursorLogic( tPlayer& player, tRtsCamera& camera )
		: mPlayer( player )
		, mCamera( camera )
		, mState( cStateRoaming )
		, mHoverUnitLogic( NULL )
		, mGhostTurretLogic( NULL )
		, mActive( false )
		, mBuildOnBuildSite( false )
		, mGhostBarbedWire( false )
		, mGhostBarbedWireValidPlacement( false )
		, mPlatformNeedsCaptured( false )
		, mPlatformNeedsCapturedMT( false )
		, mDisplay( player )
		, mPlacedLogic( NULL )
		, mLastSelectedAngle( Math::cPiOver2 )
		, mLastSelectedMag( 1.f )
	{
	}
	void tRtsCursorLogic::fOnSpawn( )
	{
		fOnPause( false );
		mCursorPosition = fTargetCursorPosition( );
	}
	void tRtsCursorLogic::fOnDelete( )
	{
		mNextHoverUnitMT.fRelease( );
		mState = cStateRoaming;
		mPlacedEntity.fRelease( );
		mPlacedLogic = NULL;
		//fUpdateHoverUnitST( );

		mHoverUnit.fRelease( );
		tLogic::fOnDelete( );
	}
	void tRtsCursorLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			mDisplay.fSetVisibility( tRtsCursorDisplay::cShowNothing );
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListThinkST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			mDisplay.fSetVisibility( fVisibility( ) );
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListThinkST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}

	void tRtsCursorLogic::fActST( f32 dt )
	{
		b32 forceUpdate = false;

		if( mCamera.fIsActive( ) )
		{
			switch( mState )
			{
			case cStateRoaming:
				fUpdateHoverUnitST( );
				break;
			case cStateUnitSelected:
				fUpdateSelection( );
				break;
			case cStateSelectingTurretToPlace:
				fUpdateTurretToPlace( );
				break;
			case cStatePlacingTurret:
				fUpdateTurretPlacement( );
				break;
			}
		}
		else 
		{
			if( mPlayer.fCurrentUnit( ) != mHoverUnitLogic )
			{
				// this is to handle quick dpad switching
				//  ie someone else controling which turret we're in other than the cursor
				mNextHoverUnitMT.fReset(  mPlayer.fCurrentUnit( ) ? mPlayer.fCurrentUnit( )->fOwnerEntity( ) : NULL );
				fUpdateHoverUnitImp( );
				mDisplay.fSetVisibility( fVisibility( ) );
				mDisplay.fForceRangeRingUpdate( );
				forceUpdate = true;
			}

			if( !tGameApp::fInstance( ).fCurrentLevelDemand( )->fLockPlaceMenuUpUntilBuild( ) )
				fResetNavigation( );
		}

		if( forceUpdate || fVisibility( ) != tRtsCursorDisplay::cShowNothing )
		{
			if( mActive && mGhostTurret && !mHoverUnit && !mBuildSiteMT )
				mDisplay.fForceRangeRingUpdate( ); //we're ghost turreting arround, keep the range rings always updated :(

			Math::tMat3f cursorMatrix = Math::tMat3f::cIdentity;
			cursorMatrix.fSetTranslation( mCursorPosition );
			cursorMatrix.fOrientZAxis( mPlayer.fUser( )->fViewport( )->fRenderCamera( ).fZAxis( ).fProjectToXZAndNormalize( ) );

			b32 otherMenuIsOpen = mTurretPlacementMenu || ( mUnitSelectionMenu && mUnitSelectionMenu->fIsActive( ) ) || mPlayer.fOffensiveMenuOpen( );
			tEntity* unit = mHoverUnit ? mHoverUnit.fGetRawPtr( ) : mGhostTurret.fGetRawPtr( );
			b32 showStats = ( mHoverUnit && !mPlayer.fCurrentUnit( ) ) && !otherMenuIsOpen && !tGameApp::fInstance( ).fIsDisplayCase( );
			b32 isGhost = !mHoverUnit.fGetRawPtr( );

			sync_event_v_c( cursorMatrix, tSync::cSCLogic );
			sync_event_v_c( showStats, tSync::cSCLogic );

			mDisplay.fUpdate( 
				dt, 
				cursorMatrix, 
				cCursorRadius, 
				unit, 
				showStats,
				isGhost );

			if( mDisplay.fUI( ) )
			{
				b32 showText = ( mGhostTurret && mTurretPlacementMenu && !mTurretPlacementMenu->fCanvas( ).fCanvas( )->fInvisible( ) );
				mDisplay.fUI( )->fSetGhostUnit( showText ? mGhostTurretLogic : NULL );
			}
		}
	}
	void tRtsCursorLogic::fResetNavigation( )
	{
		if( mTurretPlacementMenu )
			fKillPlacementMenu( );

		fDestroyGhostTurret( );

		if( mUnitSelectionMenu )
			mUnitSelectionMenu.fRelease( ); 

		mState = cStateRoaming;
	}
	void tRtsCursorLogic::fThinkST( f32 dt )
	{
		if( !mCamera.fIsActive( ) )
			return;
	}
	void tRtsCursorLogic::fCoRenderMT( f32 dt )
	{
		if( !mCamera.fIsActive( ) )
			return;
		fEvaluateCursorPositionFromTargetMT( );
		fEvaluateHoverUnitMT( );
	}
	b32 tRtsCursorLogic::fLockout( ) const
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( !level )
			return false;
		return (level->fVictoryOrDefeat( ) 
			|| mCamera.fTiltShift( ) 
			|| mCamera.fIsBombDropper( ) 
			|| (level->fIsDisplayCase( ) && !level->fOverDisplayCaseTurret( ))
			|| (level->fMapType( ) == GameFlags::cMAP_TYPE_MINIGAME && tGameApp::fInstance( ).fSingleScreenControlPlayer( ) != &mPlayer)
			)
			;
	}
	void tRtsCursorLogic::fHoverUnitSelectionChanged( )
	{
		tUnitLogic *hoveringUnit = mHoverUnitLogic ? mHoverUnitLogic : mGhostTurretLogic;
		mPlayer.fSetSelectedUnitLogic( hoveringUnit );
		mDisplay.fForceRangeRingUpdate( );
	}
	void tRtsCursorLogic::fOnCameraActivate( b32 active )
	{
		mActive = active;
		mDisplay.fSetVisibility( fVisibility( ) );
	}
	tRtsCursorDisplay::tVisibility tRtsCursorLogic::fVisibility( ) const
	{
		tGameApp& app = tGameApp::fInstance( );
		tLevelLogic* level = app.fCurrentLevel( );


		if( mCamera.fTiltShift( ) || mCamera.fIsBombDropper( ) || (level && level->fDisableRTSRings( )) )
			return tRtsCursorDisplay::cShowNothing;
		else if( mActive ) 
			return tRtsCursorDisplay::cShowEverything;
		else if( mHoverUnitLogic && mHoverUnitLogic->fConstrainYaw( ) ) 
			return tRtsCursorDisplay::cShowJustRings;
		else 
			return tRtsCursorDisplay::cShowNothing;
	}
	Math::tVec3f tRtsCursorLogic::fTargetCursorPosition( ) const
	{
		b32 placeMenuLocked = tGameApp::fInstance( ).fCurrentLevelDemand( )->fLockPlaceMenuUpUntilBuild( );

		const tGameControllerPtr gc = ( placeMenuLocked && mTurretPlacementMenu ) ? mTurretPlacementMenu->fGameController( ) : mPlayer.fGameController( );
		if( ( gc->fMode( ) == tGameController::KeyboardMouse ) && !tGameApp::fInstance( ).fIsDisplayCase( ) )
		{
			u32 inputFilter = ( placeMenuLocked && mTurretPlacementMenu ) ? mTurretPlacementMenu->fInputFilterLevel( ) : 0;

			Math::tVec2f mousePos = gc->fUIMousePos( inputFilter );

			Math::tRayf ray = mPlayer.fUser( )->fComputePickingRay( mousePos );

			tGroundRayCastCallback rayCastCb( mGhostTurret.fGetRawPtr( ) );
			fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastCb );
			if( rayCastCb.mHit.fHit( ) )
			{
				Math::tVec3f rayIntersectPoint = ray.fEvaluate( rayCastCb.mHit.mT );
				rayIntersectPoint += mCamera.fConstrainToBlocker( rayIntersectPoint );
				return rayIntersectPoint;
			}

		}

		return mCamera.fViewport( )->fLogicCamera( ).fGetTripod( ).mLookAt;
	}
	void tRtsCursorLogic::fEvaluateCursorPositionFromTargetMT( )
	{
		b32 placeMenuLocked = tGameApp::fInstance( ).fCurrentLevelDemand( )->fLockPlaceMenuUpUntilBuild( );
		u32 inputFilter = ( placeMenuLocked && mTurretPlacementMenu ) ? mTurretPlacementMenu->fInputFilterLevel( ) : 0;
		if( inputFilter != mPlayer.fUser( )->fInputFilterLevel( ) )
			return; // don't update the cursor position if the current input is being filtered out

		const Math::tVec3f targetCursorPos = fTargetCursorPosition( );

		Math::tRayf ray = Math::tRayf( targetCursorPos + cRayCastOffsetFromGround * Math::tVec3f::cYAxis, -cRayCastToGroundRayLength * Math::tVec3f::cYAxis );
		tGroundRayCastCallback rayCastCb( mGhostTurret.fGetRawPtr( ) );
		fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastCb );
		if( rayCastCb.mHit.fHit( ) )
			mCursorPosition = ray.fEvaluate( rayCastCb.mHit.mT );
		else
			mCursorPosition = targetCursorPos;
	}
	void tRtsCursorLogic::fEvaluateHoverUnitMT( )
	{
		const b32 hadBuildSite = mBuildSiteMT;
		const tGameControllerPtr gc = mPlayer.fGameController( );

		if( mPlacedEntity )
		{
			if( mPlacedLogic->fIsDestroyed( ) || !mPlacedLogic->fShouldSelect( ) )
				fClearPlacementLock( );
			else
			{
				mNextHoverUnitMT = mPlacedEntity;
				return;
			}
		}

		mNextHoverUnitMT.fRelease( );
		mBuildSiteMT.fRelease( );
		mGhostBarbedWireValidPlacement = false;
		mBuildOnBuildSite = false;

		// turn off highlight of current build site or reset
		if( !mMouseCursorHighlightedBuildSite.fNull( ) )
		{
			tBuildSiteLogic* buildSiteLogic = mMouseCursorHighlightedBuildSite->fFirstAncestorWithLogicOfType< tBuildSiteLogic >( );
			if( buildSiteLogic )
			{
				buildSiteLogic->fSetSelectionOverride( false );
			}
			mMouseCursorHighlightedBuildSite.fRelease( );

			// reset to normal gamepad highlighting status
			fUpdateGhostTurretDisable( );
		}

		if( fLockout( ) )
			return;

		tProximity proximity;

		// only look for shape entities
		tDynamicArray<u32> spatialSetIndices( 1 );
		spatialSetIndices[ 0 ] = tShapeEntity::cSpatialSetIndex;
		proximity.fSetSpatialSetIndices( spatialSetIndices );

		// configure proximity with a few additional options if we have a ghost turret
		if( mGhostTurret )
		{
			sigassert( mGhostTurretLogic );

			const s32 unitSize = mGhostTurretLogic->fUnitAttributeSize( );
			if( unitSize >= 0 )
			{
				// we're looking for build sites, so add filter
				mBuildOnBuildSite = true;

				const Math::tVec3f cursorShift = ( mCursorPosition - mGhostTurret->fObjectToWorld( ).fGetTranslation( ) );

				// add query shapes from ghost turret instead of default cursor
				for( u32 i = 0; i < mGhostTurretLogic->fSelectionShapes( ).fCount( ); ++i )
				{
					const tShapeEntityPtr& shape = mGhostTurretLogic->fSelectionShapes( )[ i ];
					switch( shape->fShapeType( ) )
					{
					case tShapeEntityDef::cShapeTypeBox:
						proximity.fAddObb( Math::tAabbf( shape->fBox( ) ).fTranslate( cursorShift ) );
						break;
					case tShapeEntityDef::cShapeTypeSphere:
						proximity.fAddSphere( shape->fSphere( ).fTranslate( cursorShift ) );
						break;
					}
				}

				proximity.fFilter( ).fAddProperty( tEntityEnumProperty( GameFlags::cENUM_BUILD_SITE, GameFlags::cBUILD_SITE_LARGE ) );
				if( unitSize == 0 )
					proximity.fFilter( ).fAddProperty( tEntityEnumProperty( GameFlags::cENUM_BUILD_SITE, GameFlags::cBUILD_SITE_SMALL ) );
			}
			else
			{

				mBuildOnBuildSite = false; //we're free roaming, such as barbed wire
				if( mGhostBarbedWire )
				{
					tFilterWallPlacement cb( mPlayer.fTeam( ) );
					Math::tObbf v( mGhostBarbedWireBounds, mGhostTurret->fObjectToWorld( ) );
					fSceneGraph( )->fIntersect( v, cb, tShapeEntity::cSpatialSetIndex );
					mGhostBarbedWireValidPlacement = cb.fOkToPlace( );
				}
			}
		}
		else
		{
			if( gc->fMode( ) == tGameController::KeyboardMouse )
			{
				proximity.fFilter( ).fAddProperty( tEntityEnumProperty( GameFlags::cENUM_BUILD_SITE, GameFlags::cBUILD_SITE_LARGE ) );
				proximity.fFilter( ).fAddProperty( tEntityEnumProperty( GameFlags::cENUM_BUILD_SITE, GameFlags::cBUILD_SITE_SMALL ) );
			}
		}


		// if no shapes were added, add default cursor radius sphere
		if( proximity.fShapes( ).fCount( ) == 0 )
			proximity.fAddSphere( Math::tSpheref( mCursorPosition, cCursorRadius ) );

		// broad-based culling
		proximity.fFilter( ).fAddTag( GameFlags::cFLAG_SELECTABLE );
		proximity.fSetQueryInheritedProperties( true );

		// run proximity query
		proximity.fRefreshMT( 0.f, fSceneGraph( )->fRootEntity( ) );

		// get closest hover unit
		f32 bestHoverUnitDist = Math::cInfinity;
		for( u32 i = 0; i < proximity.fEntityCount( ); ++i )
		{
			tEntity* unitEnt = fFilterHoverUnit( proximity.fGetEntity( i ), mPlayer.fTeam( ) );
			if( !unitEnt )
				continue;
			const f32 dist = ( unitEnt->fObjectToWorld( ).fGetTranslation( ) - mCursorPosition ).fLengthSquared( );
			if( dist < bestHoverUnitDist )
			{
				bestHoverUnitDist = dist;
				mNextHoverUnitMT.fReset( unitEnt );
			}
		}

		if( mGhostTurret || gc->fMode( ) == tGameController::KeyboardMouse )
		{
			// get closest build site
			f32 bestBuildSiteDist = Math::cInfinity;
			mPlatformNeedsCapturedMT = false;
			for( u32 i = 0; i < proximity.fEntityCount( ); ++i )
			{
				tShapeEntity* shape = fFilterBuildSite( proximity.fGetEntity( i ), mPlayer.fTeam( ), mPlatformNeedsCapturedMT );
				if( !shape )
					continue;
				const f32 dist = ( shape->fObjectToWorld( ).fGetTranslation( ) - mCursorPosition ).fLengthSquared( );
				if( dist < bestBuildSiteDist )
				{
					bestBuildSiteDist = dist;
					mBuildSiteMT.fReset( shape );
					if( !mBuildSiteMT.fNull( ) && ( gc->fMode( ) == tGameController::KeyboardMouse ) )
					{
						mMouseCursorHighlightedBuildSite = mBuildSiteMT;
						tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
						level->fUpdatePlatformGlow( cHidePlatformGlowsKeyboard );
						
						tBuildSiteLogic* buildSiteLogic = mBuildSiteMT->fFirstAncestorWithLogicOfType<tBuildSiteLogic>( );
						if( buildSiteLogic )
						{
							b32 show = !level->fLightUpOnlyTutorialPlatforms( ) || ( mBuildSiteMT->fQueryEnumValue( GameFlags::cENUM_PLATFORM_TYPE ) == GameFlags::cPLATFORM_TYPE_TUTORIAL );
							buildSiteLogic->fSetSelectionOverride( show && !buildSiteLogic->fIsOccupied( ) );
						}
					}
				}
			}

			if( !hadBuildSite && mBuildSiteMT )
			{
				mDisplay.fForceRangeRingUpdate( );
				mCamera.fHitSpeedBump( );
			}
		}
	}
	void tRtsCursorLogic::fUpdateHoverUnitImp( )
	{
		if( mNextHoverUnitMT != mHoverUnit )
		{
			mUnitSelectionMenu.fRelease( );

			mHoverUnitLogic = NULL;

			if( mNextHoverUnitMT ) // notify next hover unit that it is about to become the current hover unit
			{
				mHoverUnitLogic = mNextHoverUnitMT->fLogicDerived< tUnitLogic >( );
			}

			if( mNextHoverUnitMT && !mHoverUnit )
				mCamera.fHitSpeedBump( );

			mHoverUnit = mNextHoverUnitMT;

			fHoverUnitSelectionChanged( );
		}

		mNextHoverUnitMT.fRelease( );
	}
	void tRtsCursorLogic::fUpdateHoverUnitST( )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		b32 placeMenuLocked = level && level->fLockPlaceMenuUpUntilBuild( );

		sigassert( mState == cStateRoaming || mState == cStatePlacingTurret );
		
		if( mTurretPlacementMenu && !placeMenuLocked )
			fKillPlacementMenu( );

		if( mPlayer.fOffensiveMenuOpen( ) || mCamera.fIsBombDropper( ) )
			return;

		const tGameControllerPtr gc = mPlayer.fGameController( );
		if( gc->fMode( ) == tGameController::GamePad && gc->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_PLACEMENT_MENU ) )
			fClearPlacementLock( );

		fUpdateHoverUnitImp( );

		b32 lockout = fLockout( );
		b32 placementDisabled = level && level->fDisablePlaceMenu( );

		if( !lockout && mHoverUnit && !mHoverUnitLogic->fIsDestroyed( ) && !mUnitSelectionMenu )
			fCreateTurretSelectionMenu( );
		else if( !lockout && mUnitSelectionMenu && mUnitSelectionMenu->fTryHotKeys( gc ) )
		{
			// a hot key was triggered, don't do anything
		}
		else
		{
			if( !lockout )
			{
				b32 menuButtonPressed = gc->fMode() == tGameController::GamePad && gc->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_PLACEMENT_MENU );

				//left click, make build menu in keyboard mouse, when we have don't have a ghost turret, and hovering on a build site
				if( gc->fMode() == tGameController::KeyboardMouse && ( !mGhostTurret && mBuildSiteMT ) )
				{
					menuButtonPressed = gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_PLACEMENT_MENU );

					// set the (perceived) cursor position to be exactly on the build site (will not be updated while in the build menu)
					mCursorPosition = mBuildSiteMT->fObjectToWorld( ).fGetTranslation( );

					//center view on cursor
					if( menuButtonPressed )
						mCamera.fPanToCursor( 0.33f );
				}

				if( !placementDisabled && !mPlayer.fLockedInUnit( ) && menuButtonPressed )
					fCreateTurretPlacementMenu( );
				else if( !placementDisabled && gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_BARBED_WIRE ) )
				{
					if( mGhostBarbedWire ) 
					{
						fDestroyGhostTurret( );
						mState = cStateRoaming;
					}
					else 
						fCreateBarbedWireGhost( );
				}
				else if( mHoverUnit && mHoverUnitLogic->fShouldSelect( ) )
				{
					if( gc->fButtonUp( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_SELECT ) )
					{
						fShowTurretSelectionMenu( );

						if(gc->fMode() == tGameController::KeyboardMouse)
						{
							mCamera.fPanToCursor( 0.33f );
						}
					}
					else if( mHoverUnitLogic->fConstrainYaw( ) )
					{
						if( gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_LEFT ) )
							mHoverUnitLogic->fIncrementYawConstraint( 1, false, &mPlayer );
						else if( gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_RIGHT ) )
							mHoverUnitLogic->fIncrementYawConstraint( -1, false, &mPlayer );
					}
				}
				else if( !placementDisabled && mGhostTurret )
				{
					if( mGhostTurretLogic->fConstrainYaw( ) )
					{
						if( gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_LEFT ) )
						{
							mGhostTurretLogic->fIncrementYawConstraint( 1, false, &mPlayer );
							if( mGhostBarbedWire )
								if( mPlayer.fUser( )->fIsLocal( ) )
									mPlayer.fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_HUD_WEAPONMENU_ROTATE_BARBWIRE );
						}
						else if( gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_RIGHT ) )
						{
							mGhostTurretLogic->fIncrementYawConstraint( -1, false, &mPlayer );
							if( mGhostBarbedWire )
								if( mPlayer.fUser( )->fIsLocal( ) )
									mPlayer.fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_HUD_WEAPONMENU_ROTATE_BARBWIRE );
						}
					}
				}
			}
			
			fUpdateGhostTurretDisable( );
		}
	}
	void tRtsCursorLogic::fCreateBarbedWireGhost( )
	{
		fCreateGhostTurret( GameFlags::cUNIT_ID_WALL_WIRE );
		fUpdateGhostTurret( );
		mState = cStatePlacingTurret;
		mGhostBarbedWire = true;
		mGhostBarbedWireBounds.fInvalidate( );
		if( mPlayer.fUser( )->fIsLocal( ) )
			mPlayer.fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_HUD_WEAPONMENU_SELECT_BARBWIRE );
		if( mGhostTurret )
		{
			tGrowableArray<tEntity*> shapes;
			mGhostTurret->fAllDescendentsWithAllTags( GameFlags::cFLAG_WALL_VOLUME, shapes, 1 );
			
			for( u32 i = 0; i < shapes.fCount( ); ++i )
			{
				shapes[ i ]->fRemoveGameTags( GameFlags::cFLAG_WALL_VOLUME );
				tShapeEntity* ent = shapes[ i ]->fDynamicCast<tShapeEntity>( );
				sigassert( ent && "Wall_Volume tag on non tShapeEntity" );
				Math::tObbf localBox = ent->fBox( ).fTransform( mGhostTurret->fWorldToObject( ) );
				mGhostBarbedWireBounds |= localBox;
			}

			mGhostTurret->fRemoveGameTagsRecursive( tEntityTagMask( GameFlags::cFLAG_COLLISION | GameFlags::cFLAG_CONTEXT_ANIMATION | GameFlags::cFLAG_GROUND | GameFlags::cFLAG_INSTA_DESTROY ) );
		}
	}
	void tRtsCursorLogic::fUpdateSelection( )
	{
		sigassert( mState == cStateUnitSelected );
		sigassert( mUnitSelectionMenu && mHoverUnit );

		if( mPlayer.fOffensiveMenuOpen( ) || mCamera.fIsBombDropper( ) ) 
			return;

		tUnitLogic* unitLogic = mHoverUnit->fLogicDerived< tUnitLogic >( );
		sigassert( unitLogic );

		const tGameControllerPtr gc = mUnitSelectionMenu->fGameController( );
		u32 inputFilter = mUnitSelectionMenu->fInputFilterLevel( );

		b32 closeMenu = false;

		if( unitLogic->fIsDestroyed( ) )
			closeMenu = true;
		else if( (gc->fMode() == tGameController::GamePad && gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_CANCEL, inputFilter )) 
			|| (gc->fMode() == tGameController::KeyboardMouse && gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_TRIGGER_SECONDARY, inputFilter )) )
			closeMenu = true;
		else if( unitLogic->fUnderUserControl( ) )
			closeMenu = true;
		else
		{

			if(gc->fMode() == tGameController::KeyboardMouse)
			{
				mUnitSelectionMenu->fHighlightByPosition( gc->fUIMousePos( inputFilter ), 20.0f);

				if( gc->fButtonUp( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY, inputFilter ) && mUnitSelectionMenu->fSelectActiveIcon( ) )
					closeMenu = true;

			}
			else
			{
				Math::tVec2f stick = gc->fMoveStick( tUserProfile::cProfileCamera, inputFilter );
				f32 stickMag = gc->fMoveStickMagnitude( tUserProfile::cProfileCamera, inputFilter );
				mUnitSelectionMenu->fHighlightByAngle( stick.fAngle( ),  stickMag );

				if( gc->fButtonUp( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_SELECT, inputFilter ) && mUnitSelectionMenu->fSelectActiveIcon( ) )
					closeMenu = true;

			}

		}

		if( closeMenu )
		{
			if( !unitLogic->fUnderUserControl( ) )
				unitLogic->fEnableSelection( true );
			mUnitSelectionMenu->fFadeOut( );
			mUnitSelectionMenu->fReleaseCanvas( );
			mUnitSelectionMenu.fRelease( );
			mState = cStateRoaming;
		}
	}
	void tRtsCursorLogic::fUpdateTurretToPlace( )
	{
		sigassert( mState == cStateSelectingTurretToPlace );
		sigassert( mTurretPlacementMenu );

		if( mPlayer.fOffensiveMenuOpen( ) || mCamera.fIsBombDropper( ) ) 
			return;

		const tGameControllerPtr gc = mTurretPlacementMenu->fGameController( );
		u32 inputFilter = mTurretPlacementMenu->fInputFilterLevel( );


		b32 closeMenu = false;

		b32 placeMenuLocked = tGameApp::fInstance( ).fCurrentLevelDemand( )->fLockPlaceMenuUpUntilBuild( );
		if( mPlayer.fLockedInUnit( ) || ( gc->fMode() == tGameController::GamePad && !gc->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_PLACEMENT_MENU, inputFilter ) && !placeMenuLocked ) )
			closeMenu = true;
		else
		if(gc->fMode() == tGameController::KeyboardMouse && gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_TRIGGER_SECONDARY, inputFilter )) //right click close build menu
		{
			closeMenu = true;
		}
		else
		{

			b32 iconHighlighted = false;

			if(gc->fMode() == tGameController::KeyboardMouse)
			{
				iconHighlighted = mTurretPlacementMenu->fHighlightByPosition( gc->fUIMousePos( inputFilter ), 20.0f );
			}
			else
			{
				iconHighlighted = mTurretPlacementMenu->fHighlightByAngle( gc->fMoveStick( tUserProfile::cProfileCamera, inputFilter ).fAngle( ), gc->fMoveStickMagnitude( tUserProfile::cProfileCamera, inputFilter ) );
			}


			if( iconHighlighted )
			{
				mTurretPlacementMenu->fSelectActiveIcon( );
				mLastSelectedAngle = gc->fMoveStick( tUserProfile::cProfileCamera, inputFilter ).fAngle( );
				mLastSelectedMag = gc->fMoveStickMagnitude( tUserProfile::cProfileCamera, inputFilter );
			}
		}

		fUpdateGhostTurret( );
		fUpdateGhostTurretDisable( );

		if( closeMenu )
		{
			fKillPlacementMenu( );

			if( mGhostTurret )
				mState = cStatePlacingTurret;
			else
				mState = cStateRoaming;
		}
		else if( mGhostTurret && !mHoverUnit && gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_SELECT, inputFilter ) )
		{
			fKillPlacementMenu( );
			fSpawnGameTurretFromGhostTurret( );
			mState = cStatePlacingTurret;

			if(gc->fMode() == tGameController::KeyboardMouse)
			{
				fClearPlacementLock();
			}

		}
		else if( placeMenuLocked )
			mState = cStatePlacingTurret;

	}
	void tRtsCursorLogic::fKillPlacementMenu( )
	{
		mTurretPlacementMenu->fFadeOut( );
		mTurretPlacementMenu->fReleaseCanvas( );
		mTurretPlacementMenu.fRelease( );
	}
	void tRtsCursorLogic::fUpdateTurretPlacement( )
	{
		fUpdateHoverUnitST( );
		if( mState != cStatePlacingTurret )
			return; // changed state during fUpdateHoverUnitST, to radial menu of some sort

		b32 placeMenuLocked = tGameApp::fInstance( ).fCurrentLevelDemand( )->fLockPlaceMenuUpUntilBuild( );

		sigassert( mState == cStatePlacingTurret );
		sigassert( mHoverUnit || !mUnitSelectionMenu );
		sigassert( mGhostTurret );

		const tGameControllerPtr gc = ( placeMenuLocked && mTurretPlacementMenu ) ? mTurretPlacementMenu->fGameController( ) : mPlayer.fGameController( );
		u32 inputFilter = ( placeMenuLocked && mTurretPlacementMenu ) ? mTurretPlacementMenu->fInputFilterLevel( ) : 0;

		if( !mPlayer.fLockedInUnit( ) && ( gc->fMode( ) == tGameController::GamePad && gc->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_PLACEMENT_MENU, inputFilter ) ) && !mTurretPlacementMenu )
			fCreateTurretPlacementMenu( );
		else
		{
			fUpdateGhostTurret( );

			if( !placeMenuLocked && ((gc->fMode() == tGameController::GamePad && gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_CANCEL, inputFilter )) 
			|| (gc->fMode() == tGameController::KeyboardMouse && gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_TRIGGER_SECONDARY, inputFilter ))) )
			{
				fDestroyGhostTurret( );
				mState = cStateRoaming;
			}
			else if( gc->fButtonUp( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_SELECT, inputFilter ) )
			{
				fSpawnGameTurretFromGhostTurret( );

				if(gc->fMode() == tGameController::KeyboardMouse)
				{
					fClearPlacementLock();
				}

			}
			
		}
	}
	void tRtsCursorLogic::fUpdateGhostTurret( )
	{
		if( !mGhostTurret )
			return;

		if( mPlatformNeedsCapturedMT != mPlatformNeedsCaptured )
		{
			mPlatformNeedsCaptured = mPlatformNeedsCapturedMT;
			if( mDisplay.fUI( ) )
				mDisplay.fUI( )->fShowCapturePlatform( mPlatformNeedsCaptured );
		}

		if( mBuildSiteMT )
			mGhostTurret->fMoveTo( mBuildSiteMT->fObjectToWorld( ).fGetTranslation( ) );
		else
			mGhostTurret->fMoveTo( mCursorPosition );

		b32 canAffordGhost = mPlayer.fCouldPurchase( mGhostTurretLogic->fUnitAttributePurchaseCost( ) );

		if( mDisplay.fUI( ) )
			mDisplay.fUI( )->fSetNoMoney( !mHoverUnit && !canAffordGhost, false, mGhostTurretLogic->fUnitAttributePurchaseCost( ) );

		if( mGhostBarbedWireValidPlacement )
		{
			for( s32 i = mBarbedWires.fCount( ) - 1; i >= 0; --i )
				if( !mBarbedWires[ i ]->fSceneGraph( ) )
					mBarbedWires.fErase( i );

			const u32 cMaxBarbedWires = 8;
			if( mBarbedWires.fCount( ) >= cMaxBarbedWires )
				mGhostBarbedWireValidPlacement = false;
		}

		if( (!mBuildOnBuildSite || mBuildSiteMT) && canAffordGhost && (!mGhostBarbedWire || mGhostBarbedWireValidPlacement) )
		{
			// OK to build here
			Gfx::tRenderableEntity::fSetRgbaTint( *mGhostTurret, Math::tVec4f( 0.5f, 1.f, 0.5f, 1.f ) );
		}
		else
		{
			// Not ok to build here
			Gfx::tRenderableEntity::fSetRgbaTint( *mGhostTurret, Math::tVec4f( 1.f, 0.5f, 0.5f, 1.f ) );
		}
	}
	void tRtsCursorLogic::fUpdateGhostTurretDisable( )
	{
		tLevelLogic* currentLevel = tGameApp::fInstance( ).fCurrentLevel( );

		if( mGhostTurret )
		{
			Gfx::tRenderableEntity::fSetDisabled( *mGhostTurret, !mHoverUnit.fNull( ) );
			if( currentLevel && ( mPlayer.fGameController( )->fMode( ) == tGameController::GamePad ) )
				currentLevel->fUpdatePlatformGlow( mHoverUnit.fNull( ) ? mGhostTurretLogic->fUnitAttributeSize( ) : cHidePlatformGlowsGamepad );
		}
		else
		{
			if( currentLevel && ( mPlayer.fGameController( )->fMode( ) == tGameController::GamePad ) )
				currentLevel->fUpdatePlatformGlow( cHidePlatformGlowsGamepad );
		}

	}
	void tRtsCursorLogic::fCreateTurretPlacementMenu( )
	{
		sigassert( !mTurretPlacementMenu );


		mTurretPlacementMenu = Gui::tRadialMenuPtr( NEW Gui::tRadialMenu( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptPlaceTurret ), mPlayer.fUser( ), mPlayer.fGameController( ) ) );
		Sqrat::Function( mTurretPlacementMenu->fCanvas( ).fScriptObject( ), "DefaultSetup" ).Execute( this );

		tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ).fAddChild( mTurretPlacementMenu->fCanvas( ) );
		const Math::tRect vpRect = mPlayer.fUser( )->fComputeViewportRect( );
		mTurretPlacementMenu->fCanvas( ).fCodeObject( )->fSetPosition( Math::tVec3f( vpRect.fCenter( ), 0.5f ) );
		mTurretPlacementMenu->fFadeIn( );

		mTurretPlacementMenu->fCanvas( ).fCanvas( )->fSetInvisible( mPlayer.fUser( )->fViewport( )->fIsVirtual( ) );
		mTurretPlacementMenu->fHighlightByAngle( mLastSelectedAngle, mLastSelectedMag );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level ) level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_PLACE_MENU ) );

		mState = cStateSelectingTurretToPlace;

		if( mGhostTurret && mGhostBarbedWire )
			fDestroyGhostTurret( );

		if( !mGhostTurret )
			mTurretPlacementMenu->fSelectActiveIcon( );
	}
	void tRtsCursorLogic::fCreateTurretSelectionMenu( )
	{
		sigassert( !mUnitSelectionMenu );
		sigassert( mHoverUnit );
		tUnitLogic* unitLogic = mHoverUnit->fLogicDerived< tUnitLogic >( );
		sigassert( unitLogic );
		mUnitSelectionMenu = unitLogic->fCreateSelectionRadialMenu( mPlayer );

		if( mUnitSelectionMenu )
			mUnitSelectionMenu->fCanvas( ).fCanvas( )->fSetInvisible( mPlayer.fUser( )->fViewport( )->fIsVirtual( ) );
	}
	void tRtsCursorLogic::fShowTurretSelectionMenu( )
	{
		fDestroyGhostTurret( );

		sigassert( mHoverUnit );
		tUnitLogic* unitLogic = mHoverUnit->fLogicDerived< tUnitLogic >( );
		sigassert( unitLogic );

		if( mUnitSelectionMenu )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			sigassert( level );
			if( level && level->fIsDisplayCase( ) )
			{
				tTurretLogic* turret = unitLogic->fDynamicCast<tTurretLogic>( );
				sigassert( turret );
				turret->fTryToUse( &mPlayer );
			}
			else
			{
				unitLogic->fEnableSelection( false );

				tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ).fAddChild( mUnitSelectionMenu->fCanvas( ) );
				const Math::tRect vpRect = mPlayer.fUser( )->fComputeViewportRect( );
				mUnitSelectionMenu->fCanvas( ).fCodeObject( )->fSetPosition( Math::tVec3f( vpRect.fCenter( ), 0.5f ) );
				mUnitSelectionMenu->fFadeIn( );


				//set cursor position to first entry when opening turret selection menu
				const tGameControllerPtr gc = mPlayer.fGameController( );
				if( gc->fMode( ) == tGameController::KeyboardMouse )
				{
					const Math::tVec2f entryPos = mUnitSelectionMenu->fGetEntryPosition( 0 ); // UI coords
					gc->fUISetMouseCursorPos( entryPos );
				}
				

				mState = cStateUnitSelected;

				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				if( level ) level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_USE_MENU ) );
			}
		}
	}

	namespace
	{
		void fDisableSubUnits( tEntity* ent )
		{
			tUnitLogic* unit = ent->fLogicDerived<tUnitLogic>( );
			if( unit )
				unit->fSetTakesDamage( false );

			for( u32 i = 0; i < ent->fChildCount( ); ++i )
				fDisableSubUnits( ent->fChild( i ).fGetRawPtr( ) );
		}
	}

	b32 tRtsCursorLogic::fCreateGhostTurret( u32 turretID )
	{
		fDestroyGhostTurret( );

		const tFilePathPtr path = tGameApp::fInstance( ).fUnitResourcePath( (GameFlags::tUNIT_ID)turretID, mPlayer.fCountry( ) );
		if( path.fLength( ) == 0 )
		{
			log_warning( 0, "Attempting to create Ghost Turret - turret path was not specified for turret type (" << GameFlags::fUNIT_IDEnumToValueString( turretID ) << ")" );
			return false;
		}

		Math::tMat3f ghostTurretMatrix = Math::tMat3f::cIdentity;
		ghostTurretMatrix.fOrientZAxis( tGameApp::fInstance( ).fDefaultTurretDirection( mPlayer.fTeam( ) ) );

		if( mBuildSiteMT )
			ghostTurretMatrix.fSetTranslation( mBuildSiteMT->fObjectToWorld( ).fGetTranslation( ) );
		else
			ghostTurretMatrix.fSetTranslation( mCursorPosition );
		
		tResourcePtr ghostTurretResource = tGameApp::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake< tSceneGraphFile >( path ) );
		if( !ghostTurretResource->fLoaded( ) )
		{ 
			log_warning( 0, "Attempting to spawn a turret without a loaded scene graph file [" << path << "]" );
			return false;
		}

		tSgFileRefEntity* ghostTurret = NEW tSgFileRefEntity( ghostTurretResource );
		mGhostTurret.fReset( ghostTurret );
		mGhostTurret->fSpawn( fSceneGraph( )->fRootEntity( ) );
		mGhostTurret->fMoveTo( ghostTurretMatrix );

		mGhostTurretLogic = mGhostTurret->fLogicDerived< tUnitLogic >( );
		sigassert( mGhostTurretLogic );

 		mGhostTurretLogic->fSetCreationType( tUnitLogic::cCreationTypeGhost );
		mGhostTurretLogic->fQueryEnums( );
		mGhostTurretLogic->fSetTeamPlayer( &mPlayer, false );
		ghostTurret->fRemoveGameTagsRecursive( tEntityTagMask( GameFlags::cFLAG_COLLISION | GameFlags::cFLAG_CONTEXT_ANIMATION | GameFlags::cFLAG_GROUND | GameFlags::cFLAG_INSTA_DESTROY ) );
		ghostTurret->fAddGameTagsRecursive( tEntityTagMask( GameFlags::cFLAG_DUMMY ) );

		fDisableSubUnits( ghostTurret );

		fHoverUnitSelectionChanged( );
		
		// hide ghost turret so it doesn't flash
		Gfx::tRenderableEntity::fSetDisabled( *mGhostTurret, true );

		return true;
	}
	void tRtsCursorLogic::fDestroyGhostTurret( )
	{
		mLastSelectedAngle = Math::cPiOver2;
		mLastSelectedMag = 1.f;

		if( mGhostTurret )
		{
			if( mGhostBarbedWire )
				if( mPlayer.fUser( )->fIsLocal( ) )
					mPlayer.fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_HUD_WEAPONMENU_SELECT_BARBWIRE );
			mGhostTurret->fDelete( );
			mGhostTurret.fRelease( );
			mGhostTurretLogic = NULL;
			mGhostBarbedWire = false;

			fHoverUnitSelectionChanged( );

			if( mDisplay.fUI( ) )
				mDisplay.fUI( )->fSetNoMoney( false, false, 0 );
		}

		if( mPlatformNeedsCaptured )
		{
			mPlatformNeedsCaptured = false;
			if( mDisplay.fUI( ) )
				mDisplay.fUI( )->fShowCapturePlatform( mPlatformNeedsCaptured );
		}

		tLevelLogic *level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level && ( mPlayer.fGameController( )->fMode( ) == tGameController::GamePad ) )
			level->fUpdatePlatformGlow( cHidePlatformGlowsGamepad );
	}
	void tRtsCursorLogic::fSpawnGameTurretFromGhostTurret( )
	{
		u32 cost = mGhostTurretLogic->fUnitAttributePurchaseCost( );

		if( mBuildSiteMT && mBuildSiteMT->fChildCount( ) )
			return; //already occupied somehow, most likely by our coop buddy

		if( mBuildSiteMT )
		{
			u32 buildSiteSize = mBuildSiteMT->fQueryEnumValue( GameFlags::cENUM_BUILD_SITE );
			const s32 unitSize = mGhostTurretLogic->fUnitAttributeSize( );

			if( unitSize > (s32)buildSiteSize )
				return;
		}

		if( !fLockout( )
			&& (!mBuildOnBuildSite || mBuildSiteMT) 
			&& (!mGhostBarbedWire || mGhostBarbedWireValidPlacement)
			&& mPlayer.fAttemptPurchase( cost ) )
		{
			tEntity* parent = mGhostTurret->fParent( );
			if( mBuildSiteMT ) 
			{
				// We use build site parenting to indicate if the build site is occupied
				sigassert( mBuildSiteMT->fChildCount( ) == 0 );
				tBuildSiteLogic* buildSiteLogic = mBuildSiteMT->fFirstAncestorWithLogicOfType<tBuildSiteLogic>( );

				if( buildSiteLogic )
				{
					// If it's reserved then the other local player in a coop game probably placed a unit but it hasn't spawned yet
					if( buildSiteLogic->fIsReserved( ) )
						return;

					buildSiteLogic->fSetReserved( true );
				}

				parent = mBuildSiteMT.fGetRawPtr( );
			}

			// ** could be a barbed wire! * *
			tSgFileRefEntity* gameTurret = NEW tSgFileRefEntity( mGhostTurret->fSgResource( ) );
			gameTurret->fSetLockedToParent( false );
			gameTurret->fSpawn( *parent );
			gameTurret->fMoveTo( mGhostTurret->fObjectToWorld( ) );
			if( mGhostBarbedWire )
			{
				mBarbedWires.fPushBack( tEntityPtr( gameTurret ) );
				mPlacedEntity.fRelease( );
				mGhostTurretLogic->fAudioSource( )->fHandleEvent( AK::EVENTS::PLAY_HUD_WEAPONMENU_PLACE_BARBWIRE );
			}
			else
				mPlacedEntity.fReset( gameTurret );
			
			tUnitLogic* unitLogic = gameTurret->fLogicDerived< tUnitLogic >( );
			sigassert( unitLogic );
			mPlacedLogic = unitLogic;
			gameTurret->fAddGameTags( GameFlags::cFLAG_SELECTABLE );
			unitLogic->fSetCreationType( tUnitLogic::cCreationTypeFromBuildSite );
			if( mGhostTurretLogic->fConstrainYaw( ) )
				unitLogic->fSetYawConstraintQuadrant( mGhostTurretLogic->fYawConstraintQuadrant( ) );

			tTurretLogic* turret = gameTurret->fLogicDerived< tTurretLogic >( );
			if( turret )
				turret->fSetPurchasedBy( &mPlayer );

			// hide ghost turret so it doesn't flash
			Gfx::tRenderableEntity::fSetDisabled( *mGhostTurret, true );

			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevelDemand( );
			{
				unitLogic->fQueryEnums( );				
				tTutorialEvent event( GameFlags::cTUTORIAL_EVENT_UNIT_BUILT, unitLogic->fUnitID( ), unitLogic->fUnitID( ), false, unitLogic->fUnitType( ), gameTurret, &mPlayer );
				event.mPlatformName = parent->fName( );
				level->fHandleTutorialEvent( event );
			}

			if( mTurretPlacementMenu )
				fKillPlacementMenu( );
			level->fResetLockPlaceMenuUpUntilBuild( );

			if( !mGhostBarbedWire )
				mPlayer.fStats( ).fIncStat( GameFlags::cSESSION_STATS_UNITS_PURCHASED, 1 );
		}
		else
		{
			// TODO audio/icon notification that you can't build
		}
	}
}


namespace Sig
{
	void tRtsCursorLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tRtsCursorLogic, tLogic, Sqrat::NoConstructor > classDesc( vm.fSq( ) );

		classDesc
			.Func(_SC("CreateGhostTurret"),	&tRtsCursorLogic::fCreateGhostTurret)
			.Func(_SC("DestroyGhostTurret"), &tRtsCursorLogic::fDestroyGhostTurret)
			.Prop(_SC("Player"),			&tRtsCursorLogic::fPlayer)
			;

		vm.fRootTable( ).Bind(_SC("RtsCursorLogic"), classDesc);
	}
}

