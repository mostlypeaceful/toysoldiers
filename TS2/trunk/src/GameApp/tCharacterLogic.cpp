#include "GameAppPch.hpp"
#include "tCharacterLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tUseInfantryCamera.hpp"
#include "AI\tSigAIGoal.hpp"
#include "AI\tSigAIGoal.hpp"
#include "tSync.hpp"

#include "tGunWeapon.hpp"
#include "tVehicleLogic.hpp"
#include "IK/tIK.hpp"
#include "ContextAnim.hpp"
#include "tTeleporterLogic.hpp"
#include "tReferenceFrameEntity.hpp"
#include "Math/ProjectileUtility.hpp"
#include "tGameEffects.hpp"

#include "Wwise_IDs.h"

// extra stuff
#include "tRPGCamera.hpp"
#include "tKeyFrameAnimation.hpp"
#include "Physics/tGroundRayCastCallback.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( bool, Gameplay_Character_InheritVelocity, true ); 
	devvar( f32, Gameplay_Character_NormalSpeed, 3.0f ); //what ever the run animation is, m/s

	devvar( f32, Gameplay_Character_Parachute_FallSpeed, 6.f );
	devvar( f32, Gameplay_Character_Parachute_DecelerateLerp, 0.1f );
	devvar( f32, Gameplay_Character_Parachute_InitialSpread, 3.f );
	devvar( f32, Gameplay_Character_Parachute_FallSpread, 1.f );

	devvar( f32, Gameplay_Character_Explosion_HorzVelMax, 14.0f ); 
	devvar( f32, Gameplay_Character_Explosion_HorzVelMin, 12.0f );
	devvar( f32, Gameplay_Character_Explosion_VertVelMin, 20.0f ); 
	devvar( f32, Gameplay_Character_Explosion_VertVelMax, 35.0f ); 
	devvar( f32, Gameplay_Character_Explosion_Gravity, 10.0f ); 
	devvar( f32, Gameplay_Character_Explosion_Probability, 0.25f );
	devvar( f32, Gameplay_Character_Explosion_ShellShockTimeMin, 3.0f ); 
	devvar( f32, Gameplay_Character_Explosion_ShellShockTimeMax, 5.0f );


	devvar( bool, Debug_Character_RenderWeaponTargets, false );
	devvar( bool, Debug_Character_RenderSkeleton, false );
	devvar( bool, Debug_ContextAnims_Render, false );

	devvar( f32, Gameplay_Character_RandomAnimMin, 5.0f );
	devvar( f32, Gameplay_Character_RandomAnimMax, 10.0f );

	devvar( bool, Perf_DoCharacterContext, true );
	devvar( f32, Perf_CharacterContextRadius, 5.0f );



	namespace
	{
		
		static const tStringPtr cParachute( "parachute" );
		static const tStringPtr cPackage( "package" );
		static const tStringPtr cDropPackage( "dropPackage" );

		f32 fRandomCharacterAnimTime( )
		{
			return sync_rand( fFloatInRange( Gameplay_Character_RandomAnimMin, Gameplay_Character_RandomAnimMax ) );
		}
	}

	tCharacterLogic::tCharacterLogic( )
		: mParachuteLogic( NULL )
		, mLastPosition( Math::tVec3f::cZeroVector )
		, mHasPackage( false )
		, mParachuting( false )
		, mChuteOpen( false )
		, mDieImmediately( false )
		, mParentRelativeUntilLand( false )
		, mParentRelative( false )
		, mContextAnimActive( false )
		, mUseSingleShotTarget( false )
		, mFellOutofLevel( false )
		, mExtraModeRespawn( false )
		, mAITargeting( false )
		, mDontInstaDestroy( false )
		, mFlyingStartHeight( cInfinity )
		, mTimeTillNextRandomAnim( fRandomCharacterAnimTime( ) )
		, mCurrentContextAnimType( GameFlags::cCONTEXT_ANIM_TYPE_NONE )
		, mNextContextAnimTypeMT( GameFlags::cCONTEXT_ANIM_TYPE_NONE )
		, mNextContextAnimHeightMT( 0.f )
		, mCurrentContextAnimEntity( NULL )
		, mNextContextAnimEntityMT( NULL )
		, mContextAnimInAir( false )
		, mWasUsable( false )
		, mStandingOnSurfaceType( GameFlags::cSURFACE_TYPE_DIRT )
		, mFireTarget( tVec3f::cZeroVector )
	{
		mAnimatable.fSetLogic( this );
		fSetLogicType( GameFlags::cLOGIC_TYPE_CHARACTER );
		mTargetOffset = tVec3f( 0, 0.5f, 0 ); //more suitable default for characters

		mPhysics.fBasicSetup( GameFlags::cFLAG_GROUND, GameFlags::cFLAG_COLLISION, 2.f );

		mUnitPath.fReset( NEW tUnitPath( ) );

		// removed to save memory. renable with caution
		//mContextUnitPath.fReset( NEW tUnitPath( ) );
		//mContextUnitPath->fSetDesiredWaypointCnt( 2 );
	}

	tCharacterLogic::~tCharacterLogic( )
	{
	}

	void tCharacterLogic::fOnSpawn( )
	{
		fOnPause( false );

		tUnitLogic::fOnSpawn( );

		mUnitPath->fSetOwnerEntity( fOwnerEntity( ) );
		//mContextUnitPath->fSetOwnerEntity( fOwnerEntity( ) );
		mPhysics.fSetTransform( fOwnerEntity( )->fObjectToWorld( ) );

		if( fHasSelectionFlag( ) )
		{
			tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
			levelLogic->fRegisterUseableUnit( fOwnerEntity( ) );
			mWasUsable = true;
		}
		else
			fAddRandomPickup( );

		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			tEntity* child = tReferenceFrameEntity::fSkipOverRefFrameEnts( fOwnerEntity( )->fChild( i ).fGetRawPtr( ) );
			if( mParachuting && child->fName( ) == cParachute )
			{
				mParachute.fReset( child );
				mParachuteLogic = mParachute->fLogic( );
			}
			else if( child->fName( ) == cPackage && child->fSceneGraph( ) && !child->fInDeletionList( ) )
				mPackage.fReset( child );

			// These were for Extra
			//else if( child->fName( ) == cLProp )
			//	mEquip[ cLeftHandEquip ] = child;
			//else if( child->fName( ) == cRProp )
			//	mEquip[ cRightHandEquip ] = child;
		}

		fAddHealthBar( );
		
		fAddToMiniMap( );
		fConfigureAudio( );

		tVehiclePassengerLogic::fOutfitCharacterProps( *fOwnerEntity( ) );

		tDynamicArray<u32> spatialSetIndices( 1 );
		spatialSetIndices[ 0 ] = tShapeEntity::cSpatialSetIndex;
		mContextAnimAndCollisionProximty.fSetSpatialSetIndices( spatialSetIndices );

		tSpheref contextShape( tVec3f( 0 ), Perf_CharacterContextRadius );
		mContextAnimAndCollisionProximty.fAddSphere( contextShape );
		mContextAnimAndCollisionProximty.fSetRefreshFrequency( Perf_CharacterContextRadius / Gameplay_Character_NormalSpeed, 0.1f );

		if( mAITargeting )
		{
			for( u32 i = 0; i < mWeaponStations.fCount( ); ++i )
				mWeaponStations[ i ]->fSetAcquireTargets( true );
		}

		// See through debugging
		//Gfx::tRenderableEntity::fSetRgbaTint( *fOwnerEntity( ), tVec4f( 1,1,1,0.25f ) );

		if( mExtraMode && fTeamPlayer( ) )
			fTryToUse( fTeamPlayer( ) );

		fOwnerEntity( )->fAddGameTags( GameFlags::cFLAG_DONT_INHERIT_STATE_CHANGE ); //dont inherent any state changes for characters
	}

	void tCharacterLogic::fPostGoalSet( )
	{
		tGoalDriven::fOnSpawn( this );

		//we need to OnActivate the current goal so it's mostate gets applied.
		// it's up to the goal to make sure the desired motion goal is on top when the master goal is set.
		tGoalDriven::fActST( this, 0.01f );

		mAnimatable.fOnSpawn( );
	}

	void tCharacterLogic::fOnDelete( )
	{
		mUnitPath.fRelease( );
		//mContextUnitPath.fRelease( );

		if( mWasUsable )
		{
			mWasUsable = false;
			tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
			if( levelLogic )
				levelLogic->fUnregisterUseableUnit( fOwnerEntity( ) );
		}
		mPhysics.fOnDelete( );
		mAnimatable.fOnDelete( );
		mParachute.fRelease( );
		mPackage.fRelease( );

		mContextAnimAndCollisionProximty.fReleaseAllEntityRefsST( );

		tUnitLogic::fOnDelete( );
	}

	void tCharacterLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListAnimateMT );
			fRunListRemove( cRunListPhysicsMT );
			fRunListRemove( cRunListMoveST );
			fRunListRemove( cRunListThinkST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			// add self to run lists
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListAnimateMT );
			fRunListInsert( cRunListPhysicsMT );
			fRunListInsert( cRunListMoveST );
			fRunListInsert( cRunListThinkST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}

	void tCharacterLogic::fOnSkeletonPropagated( )
	{
		tUnitLogic::fOnSkeletonPropagated( );
		mAnimatable.fListenForAnimEvents( *this );
	}

	void tCharacterLogic::fConfigureAudio( )
	{
		mAudio->fSetSwitch( tGameApp::cSurfaceTypeSwitchGroup, GameFlags::fSURFACE_TYPEEnumToValueString( mStandingOnSurfaceType ) );
		if( fIsCommando( ) )
			mAudio->fSetSwitch( tGameApp::cBarrageFactionSwitchGroup, GameFlags::fCOUNTRYEnumToValueString( fCountry( ) ) );

		fConfigureBasicCharacterAudio( );
	}

	b32 tCharacterLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_GAME_EFFECT:
			{
				const tEffectLogicEventContext* context = e.fContext<tEffectLogicEventContext>( );
				if( context )
				{
					mAudio->fHandleEvent( context->mAudioEvent );
					context->mAudio.fRelease( );
				}
				return true;
			}
			break;
		case GameFlags::cEVENT_ANIMATION:
			{
				if( mPackage )
				{
					const tKeyFrameEventContext* context = e.fContext<tKeyFrameEventContext>( );
					if( context->mTag == cDropPackage )
					{
						mPackage->fReparent( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
						mPackage.fRelease( );
						mHasPackage = false;
						return true;
					}
				}
			}
			break;
		case GameFlags::cEVENT_UNIT_DESTROYED:
			{
				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				if( fHasSelectionFlag( ) && (!level || !level->fDisableVehicleRespawn( ))  )
				{
					fOwnerEntity( )->fRemoveGameTags( GameFlags::cFLAG_SELECTABLE );

					tFilePathPtr unitPath = tGameApp::fInstance( ).fUnitResourcePath( fUnitID( ), fCountry( ) );
					tEntity* unit = tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( )->fSpawnChild( unitPath );
					if( unit )
					{
						unit->fAddGameTagsRecursive( GameFlags::cFLAG_SELECTABLE );
						
						if( fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_MINIGAME_UNIT ) ) 
							unit->fAddGameTagsRecursive( GameFlags::cFLAG_MINIGAME_UNIT );

						tUnitLogic* ul = unit->fLogicDerived<tUnitLogic>( );
						ul->fCopyLevelEvents( this );

						unit->fSetName( fOwnerEntity( )->fName( ) );

						const tMat3f* intialXform = tGameApp::fInstance( ).fCurrentLevel( )->fUsableUnitInitialTransform( fOwnerEntity( ) );
						if( intialXform )
							unit->fMoveTo( *intialXform );
						else
							unit->fMoveTo( fOwnerEntity( )->fObjectToWorld( ) );
					}
					else
						log_warning( 0, "Could not spawn unit: " << GameFlags::fUNIT_IDEnumToValueString( fUnitID( ) ) << " path: " << unitPath );
				}
			}
			break;
		}

		return tUnitLogic::fHandleLogicEvent( e );
	}

	void tCharacterLogic::fActST( f32 dt )
	{
		profile( cProfilePerfCharacterLogicActST );
		tUnitLogic::fActST( dt );	

		if( mFellOutofLevel )
		{
			mFellOutofLevel = false;
			fOwnerEntity( )->fDelete( );
		}
	}

	void tCharacterLogic::fAnimateMT( f32 dt )
	{
		sigassert( dt == dt );

		dt *= fTimeScale( );

		fStepTintStack( dt );

		profile( cProfilePerfCharacterLogicAnimateMT );

		mAnimatable.fAnimateMT( dt );

		if( Debug_Character_RenderSkeleton )
			mAnimatable.fAnimatedSkeleton( )->fRenderDebug( fSceneGraph( )->fDebugGeometry( ), fOwnerEntity( )->fObjectToWorld( ), tVec4f(1) );
	}

	void tCharacterLogic::fPhysicsMT( f32 dt )
	{
		profile( cProfilePerfCharacterLogicPhysicsMT );
		dt *= fTimeScale( );

		f32 collisionRadius = 1.0;
		const tSpheref myPrevShape( mPhysics.fTransform( ).fGetTranslation( ) + tVec3f( 0, collisionRadius, 0 ), collisionRadius );

		if( mParentRelative )
			mParentRelativeXformMT = mPhysics.fApplyRefFrameDelta( mParentRelativeXformMT, mAnimatable.fAnimatedSkeleton( )->fRefFrameDelta( ) );

		// --- This causes a desync.
		mPhysics.fSetTransform( mPhysics.fApplyRefFrameDelta( fOwnerEntity( )->fObjectToWorld( ), mAnimatable.fAnimatedSkeleton( )->fRefFrameDelta( ) ) );
	
		mPhysics.fPhysicsMT( this, dt, fUnderUserControl( ) );
		if( mParachuting )
		{
			if( mChuteOpen )
			{
				tVec3f vel = mPhysics.fVelocity( );
				tVec2f horzVel = vel.fXZ( );
				horzVel.fSetLength( Gameplay_Character_Parachute_FallSpread );

				f32 idealySpeed = fMax( vel.y, -(f32)Gameplay_Character_Parachute_FallSpeed );
				vel.y = fLerp( vel.y, idealySpeed, (f32)Gameplay_Character_Parachute_DecelerateLerp );
				vel.x = fLerp( vel.x, horzVel.x, (f32)Gameplay_Character_Parachute_DecelerateLerp ); 
				vel.z = fLerp( vel.z, horzVel.y, (f32)Gameplay_Character_Parachute_DecelerateLerp ); 
				mPhysics.fSetVelocity( vel );
			}
		}

		// Run a query for collision detection and context anim purposes
		tGrowableArray< tEntity* > contextAnims;
		tGrowableArray< tEntity* > collisionShapes;
		if( Perf_DoCharacterContext )
			fQueryCollisionAndContextAnimOffenders( dt, contextAnims, collisionShapes, fUnderUserControl( ) );

		if( fUnderUserControl( ) )
		{
			const tSpheref myShape( mPhysics.fTransform( ).fGetTranslation( ) + tVec3f( 0, collisionRadius, 0 ), collisionRadius );
			mPhysics.fCollideAndResolve( this, dt, myShape, myPrevShape, collisionShapes );
		}

		fUpdateContextAnimsMT( dt, contextAnims );

		// compute actual velocity
		Math::tVec3f newPos = mPhysics.fTransform( ).fGetTranslation( );
		sigassert( !newPos.fIsNan( ) );
		if( !mPhysics.fFalling( ) && !mPhysics.fWantsJump( ) && Gameplay_Character_InheritVelocity ) 
		{
			sigassert( !mLastPosition.fIsNan( ) );
			sigassert( dt > 0.00001f );
			Math::tVec3f velocity = (newPos - mLastPosition) / dt;
			sigassert( !velocity.fIsNan( ) );
			velocity.y = fMin( velocity.y, 1.5f ); //dont inherit more than 3 mph of upward vel, to prevent uber jump
			mPhysics.fSetVelocity( velocity );
		}

		mLastPosition = newPos;
	}

	void tCharacterLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfCharacterLogicMoveST );
		dt *= fTimeScale( );

		mAnimatable.fMoveST( dt );

		if( mDestroyedByPlayer && mPhysics.fFalling( ) )
		{
			f32 height = mLastPosition.y - mFlyingStartHeight;
			mDestroyedByPlayer->fStats( ).fMaxStat( GameFlags::cSESSION_STATS_HIGHEST_FLYING_SOLDIER, height );
		}

		if( !mParentRelative )
		{
			sigassert( !mPhysics.fTransform( ).fIsNan( ) );
			fOwnerEntity( )->fMoveTo( mPhysics.fTransform( ) );
		}

		//fSceneGraph( )->fDebugGeometry( ).fRenderOnce( fOwnerEntity( )->fObjectToWorld( ), 3.f, 0.9f );
	}

	void tCharacterLogic::fThinkST( f32 dt )
	{
		profile( cProfilePerfCharacterLogicThinkST );
		dt *= fTimeScale( );

		//delayed move for parent relative peeps
		if( mParentRelative )
		{
			sigassert( !mParentRelativeXformMT.fIsNan( ) );
			fOwnerEntity( )->fSetParentRelativeXform( mParentRelativeXformMT );
		}

		mPhysics.fThinkST( this, dt );

		sigassert( !( mPhysics.fStartedFalling( ) && mPhysics.fJustLanded( ) ) );
		if( mPhysics.fStartedFalling( ) )	
		{
			fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_FALL ) );

			if( mParachuting )
			{
				//random spread vel
				tVec3f v = sync_rand( fVecNorm<tVec3f>( ) );
				v.y = 0;
				v *= Gameplay_Character_Parachute_InitialSpread;
				mPhysics.fSetVelocity( mPhysics.fVelocity( ) + v );
				if( mParachuteLogic ) 
					mParachuteLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_FALL ) );
			}
		}
		if( mPhysics.fJustLanded( ) )
		{
			if( mParentRelativeUntilLand )
				fClearParentRelativeUntilLand( );

			fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_LAND ) );
			if( mParachuteLogic )
			{
				mParachuteLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_LAND ) );
				mParachuteLogic = NULL;
				mParachute.fRelease( );
				mParachuting = false;
			}
			if( mPackage && mPackage->fLogic( ) && mTakesDamage ) //takes damage check allows the package to stay closed until we're ready to actualy fight!
			{
				//trigger land and open anim for package
				mPackage->fLogic( )->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_REAPPLY_MOTION_STATE ) );
			}
		}

		mUnitPath->fUpdate( dt );
		//mContextUnitPath->fUpdate( dt );

		for( u32 i = 0; i < fWeaponStationCount( ); ++i )
			fWeaponStation( i )->fProcessST( dt );

		fUpdateContextAnimsST( dt );

		mTimeTillNextRandomAnim -= dt;
		sync_event_v_c( mTimeTillNextRandomAnim, tSync::cSCLogic );
		if( mTimeTillNextRandomAnim < 0.f )
		{
			mTimeTillNextRandomAnim = fRandomCharacterAnimTime( );
			fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_RANDOM_CHARACTER_ANIM ) );
		}
	}

	void tCharacterLogic::fCoRenderMT( f32 dt )
	{
		profile( cProfilePerfCharacterLogicCoRenderMT );
		dt *= fTimeScale( );

		mPhysics.fCoRenderMT( this, dt );

		if( mPhysics.fFalling( ) && !fIsWithinLevelBounds( ) )
		{
			if( mExtraMode )
			{
				mExtraModeRespawn = true;
			}
			else
			{
				log_warning( 0, "Character fell out of level." );
				mFellOutofLevel = true;
			}
		}
	}

	void tCharacterLogic::fSetParentRelativeUntilLand( b32 disablePhys ) 
	{ 
		mParentRelative = true;
		mParentRelativeUntilLand = true; 
		mParentRelativeXformMT = Math::tMat3f::cIdentity; 
		if( disablePhys )
			fDisablePhysics( true );
	}

	void tCharacterLogic::fClearParentRelativeUntilLand( )
	{
		fDisablePhysics( false );
		mParentRelative = false;
		mParentRelativeUntilLand = false;
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level )
			fOwnerEntity( )->fReparent( *level->fRootEntity( ) );
	}

	void tCharacterLogic::fQueryCollisionAndContextAnimOffenders( f32 dt, tGrowableArray< tEntity* >& contextAnims, tGrowableArray< tEntity* >& collisionShapes, b32 collectCollision )
	{		
		u32 tags = collectCollision ? (GameFlags::cFLAG_CONTEXT_ANIMATION | GameFlags::cFLAG_COLLISION) : GameFlags::cFLAG_CONTEXT_ANIMATION;
		mContextAnimAndCollisionProximty.fFilter( ).fSetTags( tags );

		if( collectCollision )
			mContextAnimAndCollisionProximty.fClearRefreshDelay( );
		mContextAnimAndCollisionProximty.fRefreshMT( dt, *fOwnerEntity( ) );

		contextAnims.fSetCount( 0 );
		contextAnims.fSetCapacity( mContextAnimAndCollisionProximty.fEntityCount( ) );
		if( collectCollision )
		{
			collisionShapes.fSetCount( 0 );
			collisionShapes.fSetCapacity( mContextAnimAndCollisionProximty.fEntityCount( ) );
		}

		for( u32 i = 0; i < mContextAnimAndCollisionProximty.fEntityCount( ); ++i )
		{
			tEntity * const e = mContextAnimAndCollisionProximty.fGetEntity( i );

			if( e->fHasGameTagsAny( GameFlags::cFLAG_CONTEXT_ANIMATION ) )
				contextAnims.fPushBack( e );

			if( collectCollision && e->fHasGameTagsAny( GameFlags::cFLAG_COLLISION ) )
			{
				if( e->fQueryEnumValue( GameFlags::cENUM_UNIT_TYPE ) == GameFlags::cUNIT_TYPE_INFANTRY )
					continue;

				// dont collect collision from vehicles or characters
				tEntity * const logicE = e->fFirstAncestorWithLogic( );
				if( logicE )
				{
					u32 type = logicE->fQueryEnumValue( GameFlags::cENUM_UNIT_TYPE );
					if( type == GameFlags::cUNIT_TYPE_INFANTRY
						|| (!mExtraMode && type == GameFlags::cUNIT_TYPE_VEHICLE) )
						continue;
				}

				collisionShapes.fPushBack( e );
			}
		}
	}

	namespace
	{
		static enum tContextAnimDataTableColumns 
		{
			cContextAnimDataTableColumnAnimBegin,
			cContextAnimDataTableColumnAnimEnd,
			cContextAnimDataTableColumnAnimLoop,
			cContextAnimDataTableColumnAnimMoveTo,
			cContextAnimDataTableColumnAnimAlignTo,
			cContextAnimDataTableColumnFilter,
			cContextAnimDataTableColumnMatchDirection,
			cContextAnimDataTableColumnDisablePhysics,
			cContextAnimDataTableColumnJumping,
			cContextAnimDataTableColumnJumpVel,
			cContextAnimDataTableColumnPathing,
			cContextAnimDataTableColumnTargetWidth,
			cContextAnimDataTableColumnTeleport,
			cContextAnimDataTableColumnTeleportSpeed
		};
	}

	void tCharacterLogic::fUpdateContextAnimsMT( f32 dt, const tGrowableArray< tEntity* >& offenders )
	{
		mNextContextAnimTypeMT = GameFlags::cCONTEXT_ANIM_TYPE_NONE;
		mNextContextAnimEntityMT = NULL;

		const tVec3f myPosition = mPhysics.fTransform( ).fGetTranslation( );
		const tVec3f myFacing = mPhysics.fTransform( ).fZAxis( );

		for( u32 i = 0; i < offenders.fCount( ); ++i )
		{
			tEntity* e = offenders[ i ];

			sigassert( e->fHasGameTagsAny( GameFlags::cFLAG_CONTEXT_ANIMATION ) );
			tShapeEntity *sE = e->fDynamicCast<tShapeEntity>( );
			if( sE && sE->fShapeType( ) == tShapeEntityDef::cShapeTypeBox )
			{	
				//do narrow phase to see if we collided

				const tObbf& theirObb = sE->fBox( );

				if( theirObb.fContains( myPosition ) )
				{
					u32 contextType = e->fQueryEnumValue( GameFlags::cENUM_CONTEXT_ANIM_TYPE );

					const tDataTable &table = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableContextAnims ).fIndexTable( 0 );
					f32 matchDirection = Math::fToRadians( table.fIndexByRowCol<f32>( mNextContextAnimTypeMT, cContextAnimDataTableColumnMatchDirection ) );

					tVec3f theirZ = sE->fObjectToWorld( ).fZAxis( ).fNormalize( );
					if( fAcos( myFacing.fDot( theirZ ) ) < matchDirection )
					{
						mNextContextAnimHeightMT = theirObb.fExtents( )[ 1 ] - theirObb.fPointToLocalVector( myPosition )[ 1 ]; //how far from where we're at to the top of the context box
						mNextContextAnimEntityMT = sE;
						mNextContextAnimTypeMT = contextType;
						break;
					}
				}
			}
		}
	}

	void tCharacterLogic::fUpdateContextAnimsST( f32 dt )
	{
		mContextAnimAndCollisionProximty.fCleanST( );

		// defer any context changes until we've landed an air anim
		//  that way we can land in a context anim and switch to it then
		if( !mContextAnimActive && mNextContextAnimTypeMT != mCurrentContextAnimType && !mContextAnimInAir)
		{	
			if( mNextContextAnimTypeMT == GameFlags::cCONTEXT_ANIM_TYPE_NONE )
			{
				tUnitLogic::fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CONTEXT_ANIM_END ) );
			}
			else
			{
				const tDataTable &table = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableContextAnims ).fIndexTable( 0 );
				tContextAnimEventContext *context = NEW tContextAnimEventContext( );
				context->mAnimBegin = table.fIndexByRowCol<tStringPtr>( mNextContextAnimTypeMT, cContextAnimDataTableColumnAnimBegin );
				context->mAnimLoop = table.fIndexByRowCol<tStringPtr>( mNextContextAnimTypeMT, cContextAnimDataTableColumnAnimLoop );
				context->mAnimEnd = table.fIndexByRowCol<tStringPtr>( mNextContextAnimTypeMT, cContextAnimDataTableColumnAnimEnd );
				context->mAnimMoveTo = table.fIndexByRowCol<tStringPtr>( mNextContextAnimTypeMT, cContextAnimDataTableColumnAnimMoveTo );
				context->mAnimAlignTo = table.fIndexByRowCol<tStringPtr>( mNextContextAnimTypeMT, cContextAnimDataTableColumnAnimAlignTo );
				context->mDisablePhysics = (b32)table.fIndexByRowCol<f32>( mNextContextAnimTypeMT, cContextAnimDataTableColumnDisablePhysics );
				context->mJumping = (b32)table.fIndexByRowCol<f32>( mNextContextAnimTypeMT, cContextAnimDataTableColumnJumping );
				context->mPathing = (b32)table.fIndexByRowCol<f32>( mNextContextAnimTypeMT, cContextAnimDataTableColumnPathing );
				context->mTeleport = (b32)table.fIndexByRowCol<f32>( mNextContextAnimTypeMT, cContextAnimDataTableColumnTeleport );
				context->mTeleportSpeed = table.fIndexByRowCol<f32>( mNextContextAnimTypeMT, cContextAnimDataTableColumnTeleportSpeed );
				context->mHeight = mNextContextAnimHeightMT;

				b32 oneShot = !context->mAnimLoop.fExists( );
				b32 abort = false;

				// setup context unit path
				//tUnitPath* up = fContextUnitPath( );
				//up->fClearPath( );
				//up->fClearPathStarts( );
				//up->fClearTargetOverride( );

				if( context->mPathing )
				{
					sigassert( 0 && "No pathing context anims supported, context path has been removed." );

					//tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
					//const tVec3f startPt = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );

					//s32 pathStart = tUnitPath::fFindClosestStartPoint( startPt, levelLogic->fContextPathStarts( ), mNextContextAnimEntityMT->fName( ) );
					//if( pathStart > -1 ) 
					//{
					//	up->fSetDistanceTolerance( 1.f );
					//	up->fAddPathStartEntry( tUnitPath::tPathStartEntry( levelLogic->fContextPathStarts( )[ pathStart ], -1.0f, false, true, false ) );
					//	up->fStartPathSequence( );
					//}
					//else
					//	abort = true;

					//if( context->mTeleport )
					//{
					//	tEntity* contextParent = mNextContextAnimEntityMT->fParent( );
					//	const tStringPtr& teleName = contextParent->fName( );

					//	tGrowableArray<tEntity*> ents;

					//	const tGrowableArray<tEntityPtr>& teleporters = levelLogic->fTeleporters( );
					//	for( u32 i = 0; i < teleporters.fCount( ); ++i )
					//	{
					//		if( teleporters[ i ].fGetRawPtr( ) != contextParent && teleName == teleporters[ i ]->fName( ) )
					//			ents.fPushBack( teleporters[ i ].fGetRawPtr( ) );
					//	}

					//	if( ents.fCount( ) == 0 )
					//	{
					//		log_warning( 0, "Could not find destination for teleporter named: " << teleName );
					//		abort = true;
					//	}
					//	else
					//	{
					//		tEntity* teleportDest = ents[ sync_rand( fIntInRange( 0, ents.fCount( ) - 1 ) ) ];
					//		mTeleportTarget.fReset( teleportDest );
					//		sigassert( mTeleportTarget );
					//	}
					//}
				}
				
				fUnitPath( )->fContextTarget( ) = mNextContextAnimEntityMT->fObjectToWorld( ).fZAxis( );
				
				if( !abort )
				{
					mContextAnimActive = oneShot;
					mContextAnimInAir = context->mJumping;
					tUnitLogic::fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CONTEXT_ANIM_START, context ) );
				}
			}

			// If we're teleporting we want to apply new Next's to the current so we dont re enter once we pop out on the other side.
			// Once we're out (next == none) then we're done teleporting
			mCurrentContextAnimType = mNextContextAnimTypeMT;
			mCurrentContextAnimEntity = mNextContextAnimEntityMT;
		}

		// now update ground type
		u32 newGroundType = ~0;
		if( mPhysics.fStandingOn( ) ) 
			newGroundType = mPhysics.fStandingOn( )->fQueryEnumValue( GameFlags::cENUM_SURFACE_TYPE, ~0 );

		if( newGroundType != ~0 && newGroundType != mStandingOnSurfaceType )
		{
			mStandingOnSurfaceType = newGroundType;
			mAudio->fSetSwitch( tGameApp::cSurfaceTypeSwitchGroup, GameFlags::fSURFACE_TYPEEnumToValueString( mStandingOnSurfaceType ) );
			tUnitLogic::fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_REAPPLY_MOTION_STATE ) );
		}

		if( Debug_ContextAnims_Render && mCurrentContextAnimEntity )
		{
			const tObbf& theirObb = mCurrentContextAnimEntity->fBox( );
			tVec4f color( 0,0,0, 0.5f );

			if( mContextAnimInAir ) color[ 0 ] = 1.0f;
			else color[ 1 ] = 1.0f;

			fSceneGraph( )->fDebugGeometry( ).fRenderOnce( theirObb, color );
		}
	}
	Math::tVec3f tCharacterLogic::fLinearVelocity( const Math::tVec3f& localOffset )
	{
		return mPhysics.fVelocity( );
	}
	b32 tCharacterLogic::fIsWithinLevelBounds( ) const
	{
		if( !tGameApp::fInstance( ).fCurrentLevel( ) )
			return false;
		const Math::tAabbf& bounds = tGameApp::fInstance( ).fCurrentLevel( )->fLevelBounds( );
		const Math::tVec3f pos = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		return bounds.fContains( pos );
	}
	
	void tCharacterLogic::fContextAnimJump( )
	{
		if( mCurrentContextAnimEntity && mCurrentContextAnimEntity->fShapeType( ) == tShapeEntityDef::cShapeTypeBox )
		{	
			const tVec3f currentPt = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
			const tObbf& theirObb = mCurrentContextAnimEntity->fBox( );

			// allow some tolerance from center line
			const tDataTable &table = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableContextAnims ).fIndexTable( 0 );
			f32 jumpVel = table.fIndexByRowCol<f32>( mCurrentContextAnimType, cContextAnimDataTableColumnJumpVel );
			f32 targetWidth = table.fIndexByRowCol<f32>( mCurrentContextAnimType, cContextAnimDataTableColumnTargetWidth );
			f32 distFromCenter = theirObb.fAxis( 0 ).fDot( currentPt - theirObb.fCenter( ) );
			if( targetWidth >= 0.0f )
				distFromCenter = fClamp( distFromCenter, -targetWidth, targetWidth );
			
			//intersection of +z axis and box plane
			tVec3f targetPt = theirObb.fAxis( 2 ) * theirObb.fExtents( )[ 2 ] + theirObb.fCenter( )
				+ theirObb.fAxis( 0 ) * distFromCenter;

			tVec3f displacement = targetPt - currentPt;
			tVec2f projectedDisplacement( tVec2f(displacement.x, displacement.z).fLength( ), displacement.y );

			const f32 launchVel = jumpVel;
			f32 launchAngle = 0;
			if( ProjectileUtility::fComputeLaunchAngle( launchAngle, launchVel, projectedDisplacement, mPhysics.fGravity( ), false ) )
			{
				//displacement now becomes launch vec
				displacement.y = 0; displacement.fNormalizeSafe( tVec3f::cZeroVector );
				
				displacement *= cos( launchAngle );
				displacement.y = sin( launchAngle );
				displacement *= launchVel;

				mPhysics.fJump( displacement );
			}
		}
	}

	void tCharacterLogic::fReactToDamage( const tDamageContext& dc, const tDamageResult& dr )
	{
		u32 damageType = dc.fEffectDamageType( );

		if( damageType == GameFlags::cDAMAGE_TYPE_EXPLOSION  )
		{
			f32 effect = dr.mEffect;

			tVec3f deltaV = mPhysics.fPosition( ) - dc.mWorldPosition;
			sigassert( !deltaV.fIsNan( ) );
			deltaV.fProjectToXZAndNormalize( );

			if( dr.mDestroysYou && effect > 0.5f )
			{
				// send me flying
				if( sync_rand( fFloatZeroToOne( ) ) <= Gameplay_Character_Explosion_Probability )
				{
					//remap 0-1
					effect *= 2.0f;
					effect -= 1.f;

					deltaV *= fLerp( (f32)Gameplay_Character_Explosion_HorzVelMin, (f32)Gameplay_Character_Explosion_HorzVelMax, (1.f - effect) );
					deltaV.y = fLerp( (f32)Gameplay_Character_Explosion_VertVelMin, (f32)Gameplay_Character_Explosion_VertVelMax, effect );

					mPhysics.fSetGravity( -Gameplay_Character_Explosion_Gravity );
					mPhysics.fJump( deltaV );
					mFlyingStartHeight = mPhysics.fTransform( ).fGetTranslation( ).y;

					fSetOnFire( );
				}
			}

			u32 weaponType = ~0;
			if( dc.fWeaponDesc( ) ) 
				weaponType = dc.fWeaponDesc( )->mWeaponType;

			if( weaponType == tGameApp::cWeaponDerivedTypeMortar )
			{
				// this is not a regular persistent effect, it does not deal damage
				mPersistentEffect = GameFlags::cPERSISTENT_EFFECT_BEHAVIOR_STUN;
				mPersistentTimer = 5.0f;
			}
		}

		tUnitLogic::fReactToDamage( dc, dr );
	}

	void tCharacterLogic::fContextAnimJumpLanded( )
	{
		mContextAnimInAir = false;
	}

	void tCharacterLogic::fAddToMiniMap( )
	{
		f32 rand = sync_rand( fFloatZeroToOne( ) );
		if( rand <= 0.25f )
			tUnitLogic::fAddToMiniMap( );
	}

	void tCharacterLogic::fDisablePhysics( b32 disable )
	{
		mPhysics.fDisable( disable );
	}

	b32 tCharacterLogic::fTestAndSetFireTarget( )
	{
		if( mSingleShotWeaponID.fExists( ) )
		{
			u32 enemy = tPlayer::fDefaultEnemyTeam( fTeam( ) );

			const tWeaponDesc& desc = tWeaponDescs::fInstance( ).fDesc( mSingleShotWeaponID );

			tEntityPtr target;
			tWeapon::fFindTargetMT( NULL, fTeam( ), desc, fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ), target );

			if( target  )
			{
				// actually fire
				tUnitLogic* targetLogic = target->fLogicDerived<tUnitLogic>( );
				if( targetLogic )
				{
					mFireTarget = targetLogic->fOwnerEntity( )->fObjectToWorld( ).fXformPoint( targetLogic->fTargetOffset( ) );
					fUnitPath( )->fContextTarget( ) = mFireTarget - fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
					return true;
				}
			}
		}

		return false;
	}

	namespace { const tStringPtr cFireLoopingTag( "FireLooping" ); }

	void tCharacterLogic::fBlendOutFireLooping( )
	{
		tWeaponBankPtr& gunBank = fWeaponStation( 0 )->fBank( 0 );
		if( gunBank->fFiring( ) )
		{
			gunBank->fEndFire( );

			const tAnimatedSkeletonPtr& asp = mAnimatable.fAnimatedSkeleton( );
			for( u32 i = 0; i < asp->fTrackCount( ); ++i )
			{
				tAnimTrack &atp = asp->fTrack( i );
				if( atp.fTag( ) == cFireLoopingTag )
					atp.fBeginBlendingOut( 0.2f );
			}
		}
	}


	// ---------- GOALS --------------------
	typedef AI::tDerivedLogicGoalHelper<tCharacterLogic> tCharacterGoalHelper;

	
	class tInfantryMotionBase : public AI::tSigAIGoal, public tCharacterGoalHelper
	{
		define_dynamic_cast(tInfantryMotionBase, AI::tSigAIGoal);
	public:
		u32 mLastPersistantEffect;

		tInfantryMotionBase( )
			: mLastPersistantEffect( ~0 )
		{ }

		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			mLastPersistantEffect = fLogic( )->fCurrentPersistentEffect( );
			AI::tSigAIGoal::fOnActivate( logic );
		}

		virtual void fOnSuspend( tLogic* logic )
		{
			AI::tSigAIGoal::fOnSuspend( logic );
		}

		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			u32 persistantEffect = fLogic( )->fCurrentPersistentEffect( );
			if( persistantEffect != mLastPersistantEffect )
				fExecuteMotionState( logic );

			mLastPersistantEffect = persistantEffect;

			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );

			if( fLogic( )->fAITargeting( ) )
			{
				const tWeaponStationPtr& station = fLogic( )->fWeaponStation( 0 );
				sigassert( station );

				if( station->fShouldFire( ) )
					logic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_FIRE_BEGIN ) );
				else
					fLogic( )->fBlendOutFireLooping( );
			}
		}
	};


	class tInfantryMoveForward : public tInfantryMotionBase
	{
	public:
		define_dynamic_cast(tInfantryMoveForward, tInfantryMotionBase);

		virtual void fOnActivate( tLogic* logic )
		{
			tInfantryMotionBase::fOnActivate( logic );
			fLogic( )->fAudioSource( )->fHandleEvent( AK::EVENTS::PLAY_CHARACTER_FOOTSTEP_LP );
		}

		virtual void fOnSuspend( tLogic* logic )
		{
			fLogic( )->fAudioSource( )->fHandleEvent( AK::EVENTS::STOP_CHARACTER_FOOTSTEP_LP );
			tInfantryMotionBase::fOnSuspend( logic );
		}

		virtual b32	fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e )
		{
			switch( e.fEventId( ) )
			{
			case GameFlags::cEVENT_ANIMATION:
				{
					if( fLogic( )->fSingleShotWeaponID( ).fExists( ) )
					{
						const tKeyFrameEventContext* context = e.fContext< tKeyFrameEventContext >( );

						if( context->mEventTypeCppValue == GameFlags::cKEYFRAME_EVENT_FIRE_WEAPON )
						{
							tEntity* weap = fLogic( )->fOwnerEntity( )->fFirstDescendentWithName( tWeapon::cWeaponAttachName );
							if( weap )
							{
								tVec3f target = fLogic( )->fFireTarget( );
								if( !fLogic( )->fUseSingleShotTarget( ) )
									target = weap->fObjectToWorld( ).fXformPoint( tVec3f( 0,0,1000.f ) );
								tWeapon::fSingleShot( weap->fObjectToWorld( ), target, fLogic( )->fSingleShotWeaponID( ), fLogic( )->fTeam( ), fLogic( )->fOwnerEntity( ) );
							}

							return true;
						}
					}
				}
				break;
			default:
				return tInfantryMotionBase::fHandleLogicEvent( logic, e );
			}
			return true;
		}
	};

	


	static void fExportCharacterGoalsScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tInfantryMotionBase, AI::tSigAIGoal, Sqrat::NoCopy<tInfantryMotionBase> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("InfantryMotionBase"), classDesc);
		}
		{
			Sqrat::DerivedClass<tInfantryMoveForward, tInfantryMotionBase, Sqrat::NoCopy<tInfantryMoveForward> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("InfantryMoveForward"), classDesc);
		}
	}
}


namespace Sig
{
	void tCharacterLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tCharacterLogic, tUnitLogic, Sqrat::NoCopy<tCharacterLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("UnitPath"),				&tCharacterLogic::fUnitPath)
			.Prop(_SC("ContextUnitPath"),		&tCharacterLogic::fContextUnitPath)
			.Func(_SC("IsWithinLevelBounds"),	&tCharacterLogic::fIsWithinLevelBounds)
			.Prop(_SC("ContextAnimActive"),		&tCharacterLogic::fContextAnimActive, &tCharacterLogic::fSetContextAnimActive)
			.Func(_SC("ContextAnimJump"),		&tCharacterLogic::fContextAnimJump)
			.Func(_SC("ContextAnimJumpLanded"), &tCharacterLogic::fContextAnimJumpLanded)
			.Func(_SC("SurfaceTypeEnum"),		&tCharacterLogic::fSurfaceTypeEnum)
			.Prop(_SC("ChuteOpen"),				&tCharacterLogic::fChuteOpen, &tCharacterLogic::fSetChuteOpen)
			.Prop(_SC("DieImmediately"),		&tCharacterLogic::fDieImmediately, &tCharacterLogic::fSetDieImmediately)
			.Prop(_SC("Parachuting"),			&tCharacterLogic::fParachuting, &tCharacterLogic::fSetParachuteing)
			.Prop(_SC("HasPackage"),			&tCharacterLogic::fHasPackage, &tCharacterLogic::fSetHasPackage)
			.Func(_SC("DeletePackage"),			&tCharacterLogic::fDeletePackage)
			.Func(_SC("TestAndSetFireTarget"),	&tCharacterLogic::fTestAndSetFireTarget)
			.Var(_SC("SingleShotWeaponID"),		&tCharacterLogic::mSingleShotWeaponID)
			.Func(_SC("DisablePhysics"),		&tCharacterLogic::fDisablePhysics)
			.Prop(_SC("IsCaptain" ),			&tCharacterLogic::fIsCaptain)
			.Prop(_SC("IsCommando" ),			&tCharacterLogic::fIsCommando)
			.Prop(_SC("UseSingleShotTarget"),	&tCharacterLogic::fUseSingleShotTarget, &tCharacterLogic::fSetUseSingleShotTarget)
			.Prop(_SC("ParentRelativeUntilLand"),	&tCharacterLogic::fParentRelativeUntilLand)
			.Func(_SC("ClearParentRelativeUntilLand"), &tCharacterLogic::fClearParentRelativeUntilLand)
			.Func(_SC("PostGoalSet"),			&tCharacterLogic::fPostGoalSet)
			.Prop(_SC("AITargeting"),			&tCharacterLogic::fAITargeting, &tCharacterLogic::fSetAITargeting)
			;

		vm.fRootTable( ).Bind(_SC("CharacterLogic"), classDesc);
		
		fExportCharacterGoalsScriptInterface( vm );
		tContextAnimEventContext::fExportScriptInterface( vm );
	}
}

