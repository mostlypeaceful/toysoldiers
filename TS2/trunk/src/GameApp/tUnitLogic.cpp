#include "GameAppPch.hpp"
#include "tUnitLogic.hpp"
#include "tGoalBoxLogic.hpp"
#include "tLevelLogic.hpp"
#include "tMeshEntity.hpp"
#include "tDebrisLogic.hpp"
#include "tUnitPath.hpp"
#include "Gui/tWorldSpaceScriptedControl.hpp"
#include "tMiniMap.hpp"
#include "tWeapon.hpp"
#include "tHealthBar.hpp"
#include "tSaveGame.hpp"
#include "tVehiclePassengerLogic.hpp" // this is really just passenger, not vehicle specific, should be renamed
#include "tRtsCursorLogic.hpp"
#include "tRtsCamera.hpp"
#include "tReferenceFrameEntity.hpp"
#include "Gui/tWorldSpaceScriptedControl.hpp"
#include "Wwise_IDs.h"
#include "tUser.hpp"
#include "Gui/tCanvas.hpp"
#include "tGameEffects.hpp"
#include "tExplosionLogic.hpp"
#include "Audio/tAudioLogic.hpp"
#include "tSync.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( u32, Gameplay_Character_NumInnerParts, 6 );

	devrgba_clamp( Gameplay_UnitTint_Selection, Math::tVec4f( 3.f, 3.f, 3.f, 1.f ), 0, 4.f, 2 );
	devvar( f32, Gameplay_UnitTint_SelectionBlend, 0.2f );
	devvar( f32, Gameplay_UnitTint_SelectionFlashRate, 1.0f );
	devvar( f32, Gameplay_UnitTint_SelectionMaxDist, 35.f );

	devrgba_clamp( Gameplay_UnitTint_Damage, Math::tVec4f( 0.87f, 0.f, 0.f, 1.f ), 0, 4.f, 2 );
	devvar( f32, Gameplay_UnitTint_DamageDuration, 0.084f );
	devvar( f32, Gameplay_UnitTint_DamageBlend, 0.822f );
	devvar( f32, Gameplay_Weapon_TargetCoolDownMax, 4.0f );
	devvar( bool, AAACheats_EightTimesDamage, false );
	devvar( bool, Gameplay_Weapon_DrawExplosionCollision, false ); 
	devvar( bool, Perf_CheckTintChanged, true );
	devvar( bool, Perf_Audio_DisableDamaged, false );
	devvar( bool, Perf_LogScriptEvents, false );


	devvar( f32, Gameplay_Character_Audio_EmitterHeight, 1.0f );
	devvar( f32, Gameplay_RandomPickupFreq, 0.005f );
	

	namespace 
	{ 
		class tRTSCursorTintEntry : public Gfx::tFlashingTint
		{
		public:
			tRTSCursorTintEntry( tUnitLogic& unit, const tVec4f& tint, f32 blend, f32 rate, f32 maxDistance )
				: Gfx::tFlashingTint( tint, blend, rate )
				, mUnit( unit )
				, mMaxDistance( maxDistance )
				, mMaxDistanceSqrd( maxDistance * maxDistance )
				, mZeroLastFrame( true )
			{ }

			virtual b32 fStep( f32 dt ) 
			{
				b32 changed = false;

				if( mBlendStrength > cActiveThreshold )
				{					
					f32 minDist = cInfinity;
					
					// Don't flash if under user control and another player's cursor is nearby
					if( mUnit.fUnderUserControl( ) )
					{
						mUserBlend = 0;
						Gfx::tFlashingTint::fStep( dt );
						changed = !mZeroLastFrame;
						mZeroLastFrame = true;
					}
					else
					{
						const tVec3f unitPos = mUnit.fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ); 
						const tDynamicArray<tPlayerPtr>& players = tGameApp::fInstance( ).fPlayers( );
						for( u32 i = 0; i < players.fCount( ); ++i )
						{
							if( players[ i ]->fTeam( ) == mUnit.fTeam( ) )
							{
								const tRtsCursorLogicPtr& cursor = players[ i ]->fCursorLogic( );
								if( cursor && cursor->fCamera( ).fIsActive( ) )
								{
									tVec3f delta = cursor->fCurrentPosition( ) - unitPos;
									f32 dist = delta.fLengthSquared( );
									minDist = fMin( dist, minDist );
								}
							}
						}

						if( minDist < mMaxDistanceSqrd )
						{
							f32 dist = fSqrt( minDist );
							mUserBlend = 1.0f - fSquare( dist/mMaxDistance );
							changed = true;
							mZeroLastFrame = false;
						}
						else
						{
							mUserBlend = 0;
							changed = !mZeroLastFrame;
							mZeroLastFrame = true;
						}
					}
				}

				if( Gfx::tFlashingTint::fStep( dt ) )
					changed = true;

				return changed;
			}

		private:
			tUnitLogic&	mUnit;
			f32			mMaxDistance;
			f32			mMaxDistanceSqrd;
			b32			mZeroLastFrame;
		};


		static const tStringPtr cTargetOffset( "targetOffset" ); 
		static const tStringPtr cCollision( "collision" ); 
		static const tStringPtr cHealthBar( "healthBar" );

		static const tStringPtr cUseCamNear("useCamNear");
		static const tStringPtr cUseCamFar("useCamFar"); 
		static const tStringPtr cUseCamScope("useCamScope"); 
		static const tStringPtr cUseCamTarget("useCamTarget");
		static const tStringPtr cSelectionName( "selection" );
		static const tStringPtr cFocalPromptName( "focalPoint" );
		static const tStringPtr cSpecialCamTint( "SpecialCamTint" );
		static const tStringPtr cSoldierExplodedEffect( "SoldierExploded" );
		static const tStringPtr cDebrisFireTrail( "DebrisFireTrail" );
		static const tStringPtr cAirWreckageFireTrail( "AirWreckageFireTrail" );
		static const tFilePathPtr cBreakablePuffPath( "Effects/Entities/misc/breakablepuff.sigml" );
		
	}

	b32 tUnitLogic::gExtraMode = false;

	tUnitLogic::tUnitLogic( )
		: mTeamPlayer( 0 )
		, mTeam( GameFlags::cTEAM_NONE )
		, mCountry( GameFlags::cCOUNTRY_DEFAULT )
		, mUnitType( GameFlags::cUNIT_TYPE_NONE )
		, mLogicType( GameFlags::cLOGIC_TYPE_COUNT )
		, mUnitID( GameFlags::cUNIT_ID_NONE )
		, mUnitIDAlias( GameFlags::cUNIT_ID_NONE )
		, mCreationType( cCreationTypeFromLevel ) // default to from level - other creation types are set dynamically
		, mTimeScale( 1.f )
		, mLevelTimeScale( 1.f )
		, mHitPoints( 10.f ) // so that we don't default to destroyed
		, mHitPointsModifier( 1.f )
		, mDamageModifier( 1.f )
		, mDamageTransferModifier( 1.f )
		, mPickup( ~0 )
		, mUnderUserControl( false )
		, mUseDefaultEndTransition( false )
		, mDeleteAfterStates( false )
		, mConstrainYaw( -1 ) //disabled
		, mTakesDamage( true )
		, mUnderCursor( false )
		, mEnumsQueried( false )
		, mAddedToAliveList( false )
		, mFirstWaveLaunch( false )
		, mWillNotEndInGoal( false )
		, mInAlarmZone( false )
		, mHasDestroyedAnEnemy( false )
		, mSelectionEnable( true )
		, mSelectionOverride( false )
		, mDisableControl( false )
		, mDontLightUpChildren( false )
		, mDontJib( false )
		, mHasScreenSpaceHealthBar( false )
		, mDisableTimeScale( false )
		, mConstraintAxis( tVec3f::cZAxis )
		, mConstraintQuadrantCnt( 8 )
		, mConstraintAngle( cPiOver4 )
		, mSelectionRadius( 0.f )
		, mUseCamBlendValue( 0.f )
		, mPersistentEffect( ~0 )
		, mPersistentTimer( -1.f )
		, mTargetCoolOff( 0.f )
		, mDestroyedByPlayer( NULL )
		, mTargetOffset( tVec3f::cZeroVector )
		, mLastState( 0 )
		, mDestroyState( 0 )
		, mCurrentState( 0 )
		, mFirstState( 0 )
		, mStateOverride( cInvalidStateOverride )
		, mHitpointLinkedUnitLogic( NULL )
		, mChildDamageTintBlend( 1.f )
		, mOnFire( false )
		, mExtraMode( false )
	{
		fSetupTintStack( );
	}
	tUnitLogic::~tUnitLogic( )
	{
	}
	void tUnitLogic::fDontStopBullets( )
	{
		sigassert( fOwnerEntity( ) );
		fOwnerEntity( )->fAddGameTagsRecursive( GameFlags::cFLAG_DONT_STOP_BULLETS );
	}
	tFilePathPtr tUnitLogic::fResourcePath( ) const
	{
		tSceneRefEntity* sceneEntity = fOwnerEntity( )->fDynamicCast<tSceneRefEntity>( );
		if( sceneEntity ) return sceneEntity->fSgResource( )->fGetResourceId( ).fGetPath( );
		else return tFilePathPtr( "No path" );
	}
	void tUnitLogic::fQueryEnums( )
	{
		if( !mEnumsQueried )
		{
			//validate info in sigml
			if( fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_TEAM ) != ~0 ) 
				log_warning( 0, "Remove team enum from: " << fResourcePath( ) );
			if( fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_UNIT_TYPE ) != ~0 ) 
				log_warning( 0, "Remove unit type enum from: " << fResourcePath( ) );
			
			//query fire effect
			u32 fireEffectType = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_FIREEFFECTOVERRIDE );
			if( fireEffectType != ~0 )
				mFireEffectOverride = GameFlags::fFIREEFFECTOVERRIDEEnumToValueString( fireEffectType );

			mCountry = ( GameFlags::tCOUNTRY )fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_COUNTRY, GameFlags::cCOUNTRY_DEFAULT );
			fSetTeam( tGameApp::fInstance( ).fDefaultTeamFromCountry( mCountry ) );
			fSetUnitID( ( GameFlags::tUNIT_ID )fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_UNIT_ID, GameFlags::cUNIT_ID_NONE ) );
			u32 alias = tGameApp::fInstance( ).fUnitIDAlias( mUnitID, mCountry );
			if( alias != ~0 ) fSetUnitIDAlias( alias );			

			u32 personality = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_PERSONALITY_TYPE );
			if( personality != ~0 )
				mPersonalityTypeValue = GameFlags::fPERSONALITY_TYPEEnumToValueString( personality );

			mConstraintAngle = fUnitAttributeQuadrantConstraintAngle( );

			u32 dtm = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_DAMAGE_TRANSFER_MULTIPLIER );
			if( dtm != ~0 )
			{
				sigassert( GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_FIFTY - GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_ONE_TENTH == 9 );
				switch( dtm )
				{
				case GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_ONE_TENTH:	mDamageTransferModifier = 0.1f; break;
				case GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_HALF:		mDamageTransferModifier = 0.5f; break;
				case GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_ONE:		mDamageTransferModifier = 1.0f; break;
				case GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_TWO:		mDamageTransferModifier = 2.0f; break;
				case GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_THREE:		mDamageTransferModifier = 3.0f; break;
				case GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_FIVE:		mDamageTransferModifier = 5.0f; break;
				case GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_TEN:		mDamageTransferModifier = 10.0f; break;
				case GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_TWENTY:		mDamageTransferModifier = 20.0f; break;
				case GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_THIRTY:		mDamageTransferModifier = 30.0f; break;
				case GameFlags::cDAMAGE_TRANSFER_MULTIPLIER_FIFTY:		mDamageTransferModifier = 40.0f; break;
				}
			}

			mEnumsQueried = true;
		}
	}
	void tUnitLogic::fSetTeam( u32 team ) 
	{ 
		mTeam = fClamp( team, (u32)GameFlags::cTEAM_NONE, (u32)GameFlags::cTEAM_COUNT );
		fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_TEAM, mTeam ) ); 
	}
	void tUnitLogic::fSetCountry( u32 country ) 
	{ 
		mCountry = fClamp( country, (u32)GameFlags::cCOUNTRY_DEFAULT, (u32)GameFlags::cCOUNTRY_COUNT ); 
	}
	void tUnitLogic::fSetUnitID( u32 unitID ) 
	{ 
		mUnitID = fClamp( unitID, (u32)GameFlags::cUNIT_ID_NONE, (u32)GameFlags::cUNIT_ID_COUNT ); 
		mUnitIDString = GameFlags::fUNIT_IDEnumToValueString( mUnitID ); 

		if( mUnitIDAlias == GameFlags::cUNIT_ID_NONE )
			mUnitIDAlias = unitID;

		u32 rowIndex = fUnitSharedTable( ).fRowIndex( mUnitIDString );
		if( rowIndex != ~0 )
			mUnitType = ( GameFlags::tUNIT_TYPE )GameFlags::fUNIT_TYPEValueStringToEnum( fUnitSharedTable( ).fIndexByRowCol<tStringPtr>( rowIndex, cUnitSharedUnitType ) );
		else
			mUnitType = GameFlags::cUNIT_TYPE_NONE;

		fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_UNIT_TYPE, mUnitType ) );
	}
	void tUnitLogic::fSetPersonalityType( u32 type )
	{
		mPersonalityTypeValue = GameFlags::fPERSONALITY_TYPEEnumToValueString( type );
		fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_PERSONALITY_TYPE, type ) );
		if( mAudio )
			mAudio->fSetSwitch( tGameApp::cPersonalityTypeSwitchGroup, mPersonalityTypeValue );
	}
	void tUnitLogic::fSetLogicType( u32 logicType )
	{ 
		mLogicType = logicType;
		if( fOwnerEntity( ) )
			fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_LOGIC_TYPE, mLogicType ) );
	}
	void tUnitLogic::fOnSpawn( )
	{
		fOnPause( false );

		tLogic::fOnSpawn( );
		
		fQueryEnums( );

		fRegisterUnit( );

		fResetHitPoints( );

		if( fHasSelectionFlag( ) || fCreationType( ) == cCreationTypeGhost )
		{
			fCollectSelectionShapes( );
		}

		// TODO: If we have multiple offsets, store all of them

		// If there is no health bar offset, use the default
		mHealthBarOffset = 2.5f * tVec3f::cYAxis;
		
		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			const tEntityPtr& e = fOwnerEntity( )->fChild( i );
			const tStringPtr& name = e->fName( );
			if( name == cTargetOffset )
			{
				// Get the obj coordinates of the offset
				const tMat3f& worldToObj = fOwnerEntity( )->fWorldToObject( );
				mTargetOffset = worldToObj.fXformPoint( e->fObjectToWorld( ).fGetTranslation( ) );
			}
			else if( name == cCollision )
				mCollisionShape.fReset( e->fDynamicCast<tShapeEntity>( ) );
			else if( name == cHealthBar )
			{
				// TODO Deal with rotation or bone attachment
				mHealthBarOffset = e->fParentRelative( ).fGetTranslation( );
				//mHealthBarOffset = 6.f * tVec3f::cYAxis;
			}
			else if( name == cUseCamNear )
				mUseCamNear = e;
			else if( name == cUseCamFar )
				mUseCamFar = e;
			else if( name == cUseCamScope )
				mUseCamScope = e;
			else if( name == cUseCamTarget )
				mUseCamTarget = e;
		}

		if( mAlternateDebrisMeshSpawnParent.fLength( )  ) 
			mAlternateDebrisMeshSpawnParentEnt.fReset( fOwnerEntity( )->fFirstDescendentWithName( mAlternateDebrisMeshSpawnParent ) );

		b32 wantsUI = fHasSelectionFlag( ) || (fWave( ) && fWave( )->fList( ).fMakeSelectableUnits( ));
		for( u32 i = 0; i < mWeaponStations.fCount( ); ++i )
		{
			mWeaponStations[ i ]->fOnSpawn( );
			if( wantsUI )
				mWeaponStations[ i ]->fCreateUI( );
		}

		tGoalDriven::fOnSpawn( this );

		fConfigureBreakStates( );
		fRegisterForSaveGame( true );
		fConfigureAudio( );

		// Setup initial yaw constraint
		if( fUnitAttributeConstrainToQuadrant( ) )
		{
			if( mConstrainYaw == -1 ) mConstrainYaw = 0; //dont set to this to zero if its already been set from the rts cursor
			fIncrementYawConstraint( 0, true, NULL );
		}
		else
			mConstraintAxis = fDefaultFacingDirection( );

		if( fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_DOESNT_TAKE_DAMAGE ) )
			mDamageModifier = 0;

		fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_LOGIC_TYPE, mLogicType ) );
	}
	void tUnitLogic::fOnDelete( )
	{
		if( mAudio && mLogicType != GameFlags::cLOGIC_TYPE_CHARACTER )
			mAudio->fHandleEvent( AK::EVENTS::STOP_OBJECT_AMB );

		mHitPoints = 0.f;
		Logic::tGoalDriven::fOnDelete( this );
		tLogic::fOnDelete( );	

		// keep weapons around while goals cleanup
		for( u32 i = 0; i < mWeaponStations.fCount( ); ++i )
			mWeaponStations[ i ]->fOnDelete( );
		mWeaponStations.fSetCapacity( 0 );	

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level ) level->fUnregisterUnit( *this );
		fRemoveFromAliveList( );

		if( fTeamPlayer( ) && !mCreationType == cCreationTypeGhost ) //rts cursor will handle the ghost 
		{
			fTeamPlayer( )->fSetCurrentUnitLogic( NULL );
			fTeamPlayer( )->fSetSelectedUnitLogic( NULL );
		}

		mAlternateDebrisMeshSpawnParentEnt.fRelease( );
		mWave.fRelease( );
		mSelectionShapes.fSetCount( 0 );

		mUseCamNear.fRelease( );
		mUseCamFar.fRelease( );
		mUseCamScope.fRelease( );
		mUseCamTarget.fRelease( );

		mPersistentDamageContext.fRelease( );
		mCollisionShape.fRelease( );
		fReleasePickupIcon( );

		mLevelEvents.fRelease( );
		mAudio.fRelease( );

		for( u32 i = 0; i < mAllHitPointLinkedChildren.fCount( ); ++i )
		{
			tUnitLogic* logic = mAllHitPointLinkedChildren[ i ]->fLogicDerivedStaticCast< tUnitLogic >( );
			sigassert( logic );
			logic->fSetHitpointLinkedUnitLogic( NULL );
		}

		mHitPointLinkedChildren.fSetCount( 0 );
		mAllHitPointLinkedChildren.fSetCount( 0 );
		if( mHitpointLinkedUnitLogic ) 
			mHitpointLinkedUnitLogic->fRemoveHitPointLinkedChild( fOwnerEntity( ) );
		mHitpointLinkedUnitLogic = NULL;

		for( u32 i = 0; i < mCanvasObjs.fCount( ); ++i )
			mCanvasObjs[ i ]->fCanvas( ).fDeleteSelf( );
		mCanvasObjs.fSetCount( 0 );

		mHealthBar.fRelease( );
	}
	void tUnitLogic::fRegisterUnit( )
	{
		tGameApp::fInstance( ).fCurrentLevel( )->fRegisterUnit( *this );
	}
	void tUnitLogic::fAddToAliveList( )
	{
		fQueryEnums( );
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level ) 
		{
			sigassert( level->fWaveManager( ) );
			b32 fromLooping = mWave ? mWave->fList( ).fIsLooping( ) : false;
			level->fWaveManager( )->fAddEnemyAliveUnit( fUnitIDAlias( ), fCountry( ), fromLooping );
			mAddedToAliveList = true;
		}
	}
	void tUnitLogic::fRemoveFromAliveList( )
	{
		if( mAddedToAliveList )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			if( level ) 
			{
				sigassert( level->fWaveManager( ) );
				b32 fromLooping = mWave ? mWave->fList( ).fIsLooping( ) : false;
				level->fWaveManager( )->fRemoveEnemyAliveUnit( fUnitIDAlias( ), fCountry( ), fromLooping );
			}
		}
		mAddedToAliveList = false;
	}
	void tUnitLogic::fConfigureAudio( )
	{
		mAudio.fReset( NEW Audio::tSource( mUnitIDString.fCStr( ) ) );
		mAudio->fSpawn( *fOwnerEntity( ) );
	
		mAudio->fSetSwitch( tGameApp::cUnitIDSwitchGroup, fAudioID( ) );

		if( mLogicType != GameFlags::cLOGIC_TYPE_CHARACTER )
			mAudio->fHandleEvent( AK::EVENTS::PLAY_OBJECT_AMB );
	}
	void tUnitLogic::fConfigureBasicCharacterAudio( )
	{
		mAudio->fSetSwitch( tGameApp::cPersonalityTypeSwitchGroup, mPersonalityTypeValue );
		mAudio->fSetGameParam( AK::GAME_PARAMETERS::SPEED, 1.f ); //shh dont tell jeff :)

		tMat3f xform = tMat3f::cIdentity;
		xform.fSetTranslation( tVec3f( 0, Gameplay_Character_Audio_EmitterHeight, 0 ) );
		mAudio->fSetParentRelativeXform( xform );
	}
	const tStringPtr& tUnitLogic::fAudioID( ) const
	{
		// if we have an audio id, that will override the unit id for audio
		u32 audioID = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_AUDIO_TYPE_ID, ~0 );
		if( audioID != ~0 )
			return GameFlags::fAUDIO_TYPE_IDEnumToValueString( audioID );
		else
			return mUnitIDString;
	}
	void tUnitLogic::fOnPause( b32 paused )
	{
		sigassert( !"tUnitLogic::fOnPause should not be getting called or exist!" );
		//if( paused )
		//{
		//	fRunListRemove( cRunListActST );
		//}
		//else
		//{
		//	fRunListInsert( cRunListActST );
		//}
	}
	f32 tUnitLogic::fAcquireTarget( )
	{
		f32 current = mTargetCoolOff; 
		mTargetCoolOff = fMin( mTargetCoolOff + 1.f, (f32)Gameplay_Weapon_TargetCoolDownMax );
		return current; 
	}
	void tUnitLogic::fComputeCollisionShapeIfItDoesntExist( )
	{
		if( !mCollisionShape )
		{
			//build one
			const tAabbf bounds = tMeshEntity::fCombinedObjectSpaceBox( *fOwnerEntity( ) );
			tShapeEntity *shapeEnt = NEW tShapeEntity( bounds, tShapeEntityDef::cShapeTypeBox );
			shapeEnt->fSpawnImmediate( *fOwnerEntity( ) );

			mCollisionShape.fReset( shapeEnt );
		}
	}
	b32 tUnitLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		sync_event_v_c( e.fEventId( ), tSync::cSCUnit );

		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_GAME_EFFECT:
			{
				// dont let this get into script
				return true;
			}
			break;
		case GameFlags::cEVENT_ANIMATION:
			{
				const tKeyFrameEventContext* context = e.fContext<tKeyFrameEventContext>( );
				if( context )
				{
					if( context->mEventTypeCppValue == GameFlags::cKEYFRAME_EVENT_EFFECT )
					{
						tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), context->mTag );
						return true;
					}
					else if( context->mEventTypeCppValue == GameFlags::cKEYFRAME_EVENT_CHANGE_STATE )
					{
						if( mStateOverride < cInvalidStateOverride )
							fSetCurrentBreakStateAndSpawnDebris( mStateOverride, tVec3f::cZeroVector );
						else if( mCurrentState == mFirstState )
						{
							//if we're still on the first state, advance the first state and apply the rescaled damage state based on the current health percentage
							mFirstState = fMin<u16>( mLastState, mFirstState + 1 );
							tDamageResult dr;
							dr.mHealthPercentEnd = fHealthPercent( );
							fChangeStatesAndSpawnDebris( tDamageContext( ), dr );
							return true;
						}
					}
					else if( context->mEventTypeCppValue == GameFlags::cKEYFRAME_EVENT_REAL_DEATH )
					{
						sigassert( fOwnerEntity( ) && !fOwnerEntity( )->fScriptLogicObject( ).IsNull( ) );
						Sqrat::Function f( fOwnerEntity( )->fScriptLogicObject( ), "RealDeath" );
						if( !f.IsNull( ) )
						{
							f.Execute( );
							return true;
						}
					}
				}
			}
		}

		if( Perf_LogScriptEvents )
		{
			log_line( 0, "Script handled event: " << GameFlags::fGameEventToString( e.fEventId( ) ) );
			if( e.fEventId( ) == GameFlags::cEVENT_ANIMATION )
			{
				const tKeyFrameEventContext* context = e.fContext<tKeyFrameEventContext>( );
				if( context )
				{
					log_line( 0, "  , type=" << context->mEventTypeCppValue << " tag=" << context->mTag );
				}
				else
					log_line( 0, " No context" );
			}
		}
		return Logic::tGoalDriven::fHandleLogicEvent( this, e );
	}

	Math::tVec4f tUnitLogic::fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const
	{
		if( varName == cSpecialCamTint )
		{
			return tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamUnitTint( viewportIndex );
		}

		return Math::tVec4f::cZeroVector;
	}

	f32 tUnitLogic::fUnitAttributeMaxHitPoints( ) const 
	{ 
		if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
		{
			return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedHitPointsVersus ) * mHitPointsModifier;
		}
		else if( fHasSelectionFlag( ) )
		{
			return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedHitPointsNormal ) * mHitPointsModifier; 
		}
		else
		{
			u32 difficulty = tGameApp::fInstance( ).fDiffcultyOverride( );
			if( difficulty == ~0 )
				difficulty = tGameApp::fInstance( ).fDifficulty( );

			sigassert( cUnitSharedHitPointsCasual + difficulty < cUnitSharedHitPointsVersus );
			return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedHitPointsCasual + difficulty ) * mHitPointsModifier; 
		}
	}

	f32 tUnitLogic::fUnitAttributeUserModeDamageMultiplier( ) const
	{ 
		if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
			return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedUserModeDamageMultiplierVersus );
		else
			return fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedUserModeDamageMultiplier );
	}

	u32 tUnitLogic::fUnitAttributeDestroyValue( ) const 
	{ 
		if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
			return ( u32 )fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedDestroyValueVersus );
		else
		{
			u32 difficulty = tGameApp::fInstance( ).fDiffcultyOverride( );
			if( difficulty == ~0 )
				difficulty = tGameApp::fInstance( ).fDifficulty( );

			return ( u32 )fUnitSharedTable( ).fIndexByRowCol<f32>( mUnitIDString, cUnitSharedDestroyValueCasual + difficulty ); 
		}
	}

	void tUnitLogic::fActST( f32 dt )
	{
		dt *= fTimeScale( );

		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( levelLogic );
		mLevelTimeScale = levelLogic->fQueryCurrentUnitTimeScale( *this );

		if( mPersistentEffect != ~0 )
		{
			mPersistentTimer -= dt;
			if( mPersistentDamageContext )
				fDealDamage( *mPersistentDamageContext );

			if( mPersistentTimer <= 0.f )
			{
				mPersistentEffect = ~0;
				mPersistentDamageContext.fRelease( );
			}
		}
		mTargetCoolOff = fMax( 0.f, mTargetCoolOff - dt );

		Logic::tGoalDriven::fActST( this, dt );
	}
	b32 tUnitLogic::fSelectionEnabled( ) const
	{
		return fHasSelectionFlag( ) && mSelectionEnable;
	}
	void tUnitLogic::fEnableSelection( b32 enable )
	{
		mSelectionEnable = enable;
	}
	void tUnitLogic::fSetUnderUserControl( b32 userControl )
	{
		mUnderUserControl = userControl; 
		fUpdateSelectionTint( );
	}
	void tUnitLogic::fAddCanvasObject( const Gui::tScriptedControlPtr& obj )
	{
		mCanvasObjs.fPushBack( obj );
	}
	const tStringHashDataTable& tUnitLogic::fUnitSharedTable( ) const
	{
		sigassert( mCountry < GameFlags::cCOUNTRY_COUNT );
		return tGameApp::fInstance( ).fUnitsSharedTable( mCountry );
	}
	void tUnitLogic::fOnRtsCursorHoverBeginEnd( b32 startStopFlag )
	{
		mUnderCursor = startStopFlag;
		fUpdateSelectionTint( );
	}
	Gui::tRadialMenuPtr tUnitLogic::fCreateSelectionRadialMenu( tPlayer& player )
	{
		return Gui::tRadialMenuPtr( );
	}
	tLocalizedString tUnitLogic::fHoverText( ) const
	{
		tLocalizedString text = fClassText( );
		//tLocalizedString text = fUnitName( );
		return text;
	}
	tLocalizedString tUnitLogic::fPurchaseText( ) const
	{
		//tLocalizedString text = fUnitName( );
		tLocalizedString text = fClassText( );
		text.fJoinWithCString( "\n" );
		text.fJoinWithLocString( tGameApp::fInstance( ).fLocString( tStringPtr( "Cost_NoFormat" ) ) );
		text.fJoinWithCString( " " );
		text.fJoinWithLocString( tLocalizedString::fConstructMoneyString( StringUtil::fToString( fUnitAttributePurchaseCost( ) ).c_str( ) ) );

		text.fJoinWithCString( "\n\n" );
		text.fJoinWithLocString( fUnitName( ) );

		text.fJoinWithCString( "\n" );
		text.fJoinWithLocString( fPurchaseDescription( ) );

		return text;
	}
	std::string tUnitLogic::fLocKey( u32 country, u32 unitID )
	{
		sigassert( country < GameFlags::cCOUNTRY_COUNT );
		std::string name( GameFlags::fCOUNTRYEnumToValueString( country ).fCStr( ) );
		name += "_" + std::string( GameFlags::fUNIT_IDEnumToValueString( unitID ).fCStr( ) );
		return name;
	}
	tLocalizedString tUnitLogic::fUnitName( ) const
	{
		return tGameApp::fInstance( ).fLocString( tStringPtr( fLocKey( fCountry( ), fUnitID( ) ) ) );
	}
	tLocalizedString tUnitLogic::fUnitLocNameScript( u32 country, u32 unitID )
	{
		return tGameApp::fInstance( ).fLocString( tStringPtr( fLocKey( country, unitID ) ) );
	}
	tLocalizedString tUnitLogic::fUnitLocClassScript( u32 country, u32 unitID )
	{
		return tGameApp::fInstance( ).fLocString( tStringPtr( fLocKey( country, unitID ) + "_Class" ) );
	}
	tLocalizedString tUnitLogic::fClassText( ) const
	{
		return tGameApp::fInstance( ).fLocString( tStringPtr( fLocKey( fCountry( ), fUnitID( ) ) + "_Class" ) );
	}
	tLocalizedString tUnitLogic::fPurchaseDescription( ) const
	{
		return tGameApp::fInstance( ).fLocString( tStringPtr( fLocKey( fCountry( ), fUnitID( ) ) + "_PurchaseDescription" ) );
	}
	void tUnitLogic::fConfigureBreakStates( )
	{
		if( mLogicType == GameFlags::cLOGIC_TYPE_CHARACTER )
		{
			mLastState = 0;
			mDeleteAfterStates = false;
			mDestroyState = 0;
		}
		else
		{
			tMeshEntity::fEnableStateChanges( *fOwnerEntity( ), true );

			s32 transitionStatesCnt = -1;
			s32 lastState = 0;
			tMeshEntity::fStateCount( *fOwnerEntity( ), lastState, transitionStatesCnt );
			mLastState = lastState;

			//if we have transition pieces on the last state, we'll delete the entity after it.
			mDeleteAfterStates = ( mLastState == transitionStatesCnt );
			if( mDeleteAfterStates ) 
			{
				//allow us to actually get one past the last state
				// so it's t pieces are spawned, then it's deleted
				++mLastState;
				mDestroyState = mLastState;
			}
		}
	}
	void tUnitLogic::fSetTeamPlayer( tPlayer* player, b32 setCurrent )
	{
		if( setCurrent && mTeamPlayer ) mTeamPlayer->fSetCurrentUnitLogic( NULL );

		mTeamPlayer = player;
		
		if( setCurrent && mTeamPlayer ) mTeamPlayer->fSetCurrentUnitLogic( this );
	}
	void tUnitLogic::fDealDamage( const tDamageContext& damageContext, b32 force )
	{
		if( !force && !fIsValidToReactToDamage( ) )
			return;

		if( !damageContext.fValid( ) )
		{
			log_warning( 0, "Invalid damage context!" );
			return;
		}

		sync_event_v_c( damageContext.fDamageType( ), tSync::cSCUnit );

		// subtract points
		if( mHitpointLinkedUnitLogic && fOwnerEntity( ) )
		{
			const GameFlags::tLINKED_HITPOINTS linkedHP = (GameFlags::tLINKED_HITPOINTS)fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_LINKED_HITPOINTS, GameFlags::cLINKED_HITPOINTS_COUNT );
			switch( linkedHP )
			{
			case GameFlags::cLINKED_HITPOINTS_DIRECT:
			case GameFlags::cLINKED_HITPOINTS_DIRECT_AND_ON_DESTROYED:
				mHitpointLinkedUnitLogic->fComputeAndHandleDamage( mDamageTransferModifier, damageContext, true, force );
				break;

			case GameFlags::cLINKED_HITPOINTS_TRANSFER:
				mHitpointLinkedUnitLogic->fComputeAndHandleDamage( mDamageTransferModifier, damageContext, true, force );
				return;  //Were done here.  Don't take any damage ourselves

			case GameFlags::cLINKED_HITPOINTS_TRANSFER_ONLY_DIRECT:
				if( damageContext.fDamageType( ) == GameFlags::cDAMAGE_TYPE_BULLET )
					mHitpointLinkedUnitLogic->fComputeAndHandleDamage( mDamageTransferModifier, damageContext, true, force );
				return;  //Were done here.  Don't take any damage ourselves
			}
		}

		fComputeAndHandleDamage( 1.f, damageContext, false, force );
	}

	void tUnitLogic::fComputeAndHandleDamage( f32 transferModifier, const tDamageContext& damageContext, b32 fromChild, b32 force )
	{
		tDamageResult result;
		result.mSpawnInfluence = damageContext.mWorldEffectorVector;
		result.mEffect = 1.0f;

		f32 pointsScale = AAACheats_EightTimesDamage ? 8.f : 1.f;
		pointsScale *= transferModifier;

		tVec3f myPos = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		result.mAttackerDirection = damageContext.mWorldPosition - myPos;

		if( damageContext.fDamageType( ) == GameFlags::cDAMAGE_TYPE_EXPLOSION )
		{
			// compute damage fall off
			result.mAttackerDirection.fNormalizeSafe( tVec3f::cYAxis );

			if( mCollisionShape )
			{
				// we have a volume shape, use closest point to compute fall off
				myPos = mCollisionShape->fClosestPoint( damageContext.mWorldPosition );
				if( Gameplay_Weapon_DrawExplosionCollision ) fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mCollisionShape->fBox( ), tVec4f(1,0,0,0.25f) );
			}
			
			tVec3f offset = damageContext.mWorldPosition - myPos;
			f32 distance = offset.fLength( );

			distance /= damageContext.mMaxSize;
			distance = fClamp( distance, 0.0f, 1.0f );
			distance *= distance; // square root fall off

			result.mEffect = fRemapMinimum( 1.0f - distance, damageContext.mFalloff );
			result.mSpawnInfluence = -result.mAttackerDirection * result.mEffect * 20.0f; //just some high number
			result.mSpawnInfluence.y = fAbs( result.mSpawnInfluence.y ); //never blast things down
		}
		else if( damageContext.fDamageType( ) == GameFlags::cDAMAGE_TYPE_AREA )
		{
			if( damageContext.fWeaponDesc( ) 
				&& damageContext.fWeaponDesc( )->mPersistentDamageType != ~0
				&& mPersistentDamageContext.fGetRawPtr( ) != &damageContext )
			{
				// first time receiving persistent damage, damage will be removed later
				pointsScale = 0.f;
			}
			else
				pointsScale = fSceneGraph( )->fFrameDeltaTime( );
		}

		b32 shouldDamage = damageContext.fShouldDamage( fTeam( ) );
		if( shouldDamage )
		{
			f32 pointsToRemove = damageContext.fPointsToRemove( mUnitType ) * pointsScale;

			if( fUnderUserControl( ) ) 
				pointsToRemove *= fUnitAttributeUserModeDamageMultiplier( );

			fApplyDamage( pointsToRemove * result.mEffect, damageContext, result, false, force );
		}

		fReactToDamage( damageContext, result );	
	}

	void tUnitLogic::fApplyDamage( f32 damageAmount, const tDamageContext& damageContext, tDamageResult& result, b32 fromChild, b32 force )
	{
		if( !force && !fIsValidTarget( ) )
			return;

		if( !force )
			damageAmount *= mDamageModifier;

		// Show a hit indicator in the weapon UIs
		if( damageContext.fValid( ) )
		{
			for( u32 w = 0; w < mWeaponStations.fCount( ); ++w )
			{
				const tWeaponStationPtr& weap = mWeaponStations[ w ];
				if( weap->fUnderUserControl( ) && !weap->fUI( ).fNull( ) )
				{
					if( damageContext.fAttacker( ) )
						weap->fUI( )->fGetHit( weap->fUser( ), damageContext.fAttacker( )->fOwnerEntity( ) );
					else
					{
						log_warning( 0, "No attacker set! No hit indicator will be shown." );
						if( damageContext.fWeaponDesc( ) )
							log_warning( 0, "  For weapon: " << damageContext.fWeaponDesc( )->mWeaponDescName );
					}
				}
			}
		}

		sync_event_v_c( damageAmount, tSync::cSCUnit );
		result.mHealthPercentStart = fHealthPercent( );
		mHitPoints -= damageAmount;

		if( !fEqual( damageAmount, 0.f )  )
			if( damageContext.fAttackerPlayer( ) )
				log_line( Log::cFlagDamage, "Player delt damage: " << damageAmount << ", " << mHitPoints << " remaining. Damage Mult: " << damageContext.fDamageMulitplier( mUnitType ) << " Unit Type: " << GameFlags::fUNIT_TYPEEnumToString( fUnitType( ) ) << " Damage Type: " << GameFlags::fDAMAGE_TYPEEnumToString( damageContext.fDamageType( ) ) );

		// dont spam this from persistent effect
		if( &damageContext != mPersistentDamageContext.fGetRawPtr( ) )
		{
			if( mUnitType == GameFlags::cUNIT_TYPE_BOSS )
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_UNIT_DAMAGED ) );

			// trial specific event for hitting apcs
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			if( level && level->fIsTrial( ) && (tPlayer::fIsAPC( fUnitID( ) ) || tPlayer::fIsTransportCopter( fUnitID( ) )) )
				level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_HIT_APC_WITH_LOAD ) );
		}

		// Allow the logic to respond to hitting zero hit points and handle the unit not being destroyed yet
		if( mHitPoints <= 0.f )
		{
			if( mLevelEvents )
				mLevelEvents->fFire( GameFlags::cLEVEL_EVENT_ZERO_HITPOINTS, this );

			fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_UNIT_ZERO_HIT_POINTS ) );
		}

		const b32 destroyed = mHitPoints <= 0.f;

		if( destroyed )
		{
			if( damageContext.fAttackerPlayer( ) )
				mDestroyedByPlayer = damageContext.fAttackerPlayer( );

			if( mLevelEvents )
				mLevelEvents->fFire( GameFlags::cLEVEL_EVENT_UNIT_DESTROYED, this );

			if( mWave )
				mWave->fEnemyKilled( this, damageContext.fAttackerPlayer( ) );

			// If there's a specified player, just use that
			if( damageContext.fAttackerPlayer( ) )
				damageContext.fAttackerPlayer( )->fEnemyKilled( damageContext, damageAmount, *this, 0, 1 );
			else // otherwise apply the kill to all players of the attacking team
			{
				u32 shareCount = tGameApp::fInstance( ).fGameMode( ).fIsCoOp( ) ? 2 : 1;
				u32 shareId = 0;
				const u32 playerCount = tGameApp::fInstance( ).fPlayerCount( );
				for( u32 p = 0; p < playerCount; ++p )
				{
					tPlayer * player = tGameApp::fInstance( ).fGetPlayer( p );
					if( player->fTeam( ) == damageContext.fAttackingTeam( ) )
						player->fEnemyKilled( damageContext, damageAmount, *this, shareId++, shareCount );
				}
			}

			if( mHitpointLinkedUnitLogic && fOwnerEntity( ) && mHitpointLinkedUnitLogic->fIsValidTarget( ) )
			{
				u32 linkedFlag = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_LINKED_HITPOINTS );
				if( linkedFlag == GameFlags::cLINKED_HITPOINTS_ON_DESTROYED
					|| linkedFlag == GameFlags::cLINKED_HITPOINTS_DIRECT_AND_ON_DESTROYED )
				{
					mHitpointLinkedUnitLogic->fApplyDamage( fUnitAttributeMaxHitPoints( ) * mDamageTransferModifier, damageContext, result, true, force );
				}
			}
		}

		result.mDestroysYou = destroyed;
		result.mHealthPercentEnd = fHealthPercent( );
		if( mHealthBar )
			mHealthBar->fSetHealthBarPercent( result.mHealthPercentEnd );
		if( mHasScreenSpaceHealthBar )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			if( level )
				level->fScreenSpaceHealthBarList( )->fSetHealthPercent( *this, result.mHealthPercentEnd );
		}

		if( destroyed )
		{
			if( damageContext.fDontJib( ) )
				mDontJib = true;

			if( damageContext.fAttacker( ) && fTeam( ) == tPlayer::fDefaultEnemyTeam( damageContext.fAttacker( )->fTeam( ) ) )
				damageContext.fAttacker( )->fSetDestroyedEnemyUnit( true );
		}

		fChangeStatesAndSpawnDebris( damageContext, result );
		fActivateDamageTint( fromChild ? mChildDamageTintBlend : 1.f );

		if( destroyed && fSceneGraph( ) )
		{
			if( mAudio && mLogicType != GameFlags::cLOGIC_TYPE_CHARACTER  )
				mAudio->fHandleEvent( AK::EVENTS::PLAY_OBJECT_DESTROY );
			fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_UNIT_DESTROYED ) );
			
			if( !mDontJib )
				fSpawnDefaultDestroyedExplosion( );

			if( mDestroyedEffect.fExists( ) )
			{
				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				if( level )
				{
					tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), mDestroyedEffect );
				}
			}

			fReleasePickupIcon( );
		}
	}
	void tUnitLogic::fActivateDamageTint( f32 strength )
	{
		if( mTintStack.fHasIndex( cTintTypeDamage ) )
		{
			mTintStack.fStack( )[ cTintTypeDamage ]->fSetBlendStrengthUser( strength );
			mTintStack.fStack( )[ cTintTypeDamage ]->fSetActivationTime( Gameplay_UnitTint_DamageDuration );
		}
	}

	void tUnitLogic::fReactToDamage( const tDamageContext& dc, const tDamageResult& dr )
	{
		if( dc.fShouldDamage( fTeam( ) ) )
		{
			u32 newPersistentEffect = dc.fPersistentDamage( );
			b32 newEffect = false;

			if( newPersistentEffect != ~0 )
			{
				if( mPersistentDamageContext.fGetRawPtr( ) != &dc   //skip if it's already from our current persistent damage
					&& dc.fPointsToRemove( fUnitType( ) ) > 0.f )	//skip if this will have no effect
				{
					if( mPersistentEffect != newPersistentEffect )
					{
						newEffect = true;

						//  check stats
						if( dc.fAttackerPlayer( ) && mLogicType == GameFlags::cLOGIC_TYPE_CHARACTER )
						{
							switch( newPersistentEffect )
							{
							case GameFlags::cPERSISTENT_EFFECT_BEHAVIOR_GAS: dc.fAttackerPlayer( )->fStats( ).fIncStat( GameFlags::cSESSION_STATS_INFANTRY_GASSED, 1.f ); break;
							case GameFlags::cPERSISTENT_EFFECT_BEHAVIOR_FIRE: dc.fAttackerPlayer( )->fStats( ).fIncStat( GameFlags::cSESSION_STATS_INFANTRY_SET_ON_FIRE, 1.f ); break;
							}
						}
					}

					mPersistentEffect = newPersistentEffect;
					mPersistentDamageContext.fReset( NEW tDamageContext( dc ) );
					mPersistentTimer = 5.f;
					if( mPersistentEffect == GameFlags::cPERSISTENT_EFFECT_BEHAVIOR_FIRE )
					{
						b32 firstTime = fSetOnFire( );
						if( firstTime && dc.fAttacker( ) )
						{
							tTutorialEvent igniteEvent( GameFlags::cTUTORIAL_EVENT_UNIT_IGNITED, fUnitID( ), fUnitID( ), false, fUnitType( ), fOwnerEntity( ), dc.fAttackerPlayer( ) );
							if( dc.fAttackerPlayer( ) )
								igniteEvent.mPlayerKillerLogic = dc.fAttacker( );
							tGameApp::fInstance( ).fCurrentLevel( )->fHandleTutorialEvent( igniteEvent );
						}
					}
				}
			}
			else
				newEffect = true;

			// special audio treats for direct user damage
			if( newEffect && dc.fAttackerPlayer( ) )
			{
				tStringPtr sound;
				if( fIsDestroyed( ) )
					sound = fUnitAttributeUserKillAudio( );

				if( !sound.fExists( ) )
					sound = fUnitAttributeUserHitAudio( );

				if( sound.fExists( ) )
					dc.fAttackerPlayer( )->fSoundSource( )->fHandleEvent( sound.fCStr( ) );
			}
		}
	}
	void tUnitLogic::fSpawnDefaultDestroyedExplosion( )
	{
		f32 size = fUnitAttributeDestroyedExplosionSize( );

		if( size > 0 )
		{
			tEntity* explosionEnt = NEW tEntity( );

			tExplosionLogic* explosion = NEW tExplosionLogic( );
			explosion->fSetFullSize( size );
			explosion->fSetGrowRate( fUnitAttributeDestroyedExplosionRate( ) );
			explosion->fSetHitPoints( fUnitAttributeDestroyedExplosionPoints( ) );
			if( mDestroyedByPlayer )
				explosion->fSetFiredBy( tDamageID( NULL, mDestroyedByPlayer ) );
			else
				explosion->fSetFiredBy( tDamageID( this, NULL, fTeam( ) ) );

			explosionEnt->fAcquireLogic( NEW tLogicPtr( explosion ) );
			explosionEnt->fSpawn( *fOwnerEntity( ) );
		}
	}
	void tUnitLogic::fSetInAlarmZone( b32 inAZ ) 
	{ 
		b32 fireEvent = !mInAlarmZone;
		mInAlarmZone = inAZ;
		if( fireEvent && mInAlarmZone )
			fHandleLogicEvent( Logic::tEvent::fConstructDefault( GameFlags::cEVENT_REAPPLY_MOTION_STATE ) );
	}
	b32 tUnitLogic::fSetOnFire( )
	{
		if( mOnFire )
			return false;

		mOnFire = (1<<0);
		
		if( mFireEffectOverride.fExists( ) )
		{
			tEntity* effect = tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), mFireEffectOverride );
			if( effect )
			{
				Math::tVec3f objectScale = fOwnerEntity( )->fWorldToObject( ).fGetScale( );
				Math::tMat3f xform = effect->fObjectToWorld( );
				xform.fScaleLocal( objectScale );
				mOnFire |= (1<<1);
			}
		}

		return true;
	}
	void tUnitLogic::fSetCurrentBreakState( u32 stateIndex )
	{
		if( stateIndex != mCurrentState )
		{
			tMeshEntity::tChangeMeshState change( mCurrentState, stateIndex, true, false, GameFlags::cFLAG_DONT_INHERIT_STATE_CHANGE, false );
			change.fChangeState( *fOwnerEntity( ) );
			mCurrentState = stateIndex;
			fOnStateChanged( );
		}
	}
	void tUnitLogic::fSetCurrentBreakStateAndSpawnDebris( u32 stateIndex, const Math::tVec3f& spawnInfluence )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( stateIndex != mCurrentState && level )
		{			
			// change the state
			tMeshEntity::tChangeMeshState change( mCurrentState, stateIndex, mDontJib, (mUseDefaultEndTransition && !mDontJib), GameFlags::cFLAG_DONT_INHERIT_STATE_CHANGE,  false );
			change.mCollectAllTransition = (!mDontJib && fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_DEBRIS_BEHAVIOR ) == GameFlags::cDEBRIS_BEHAVIOR_SPAWN_ALL_TRANSITION);
			change.fChangeState( *fOwnerEntity( ) );

			if( !mDontJib )
			{
				GameFlags::tDEBRIS_TYPE debrisT = ( GameFlags::tDEBRIS_TYPE )fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_DEBRIS_TYPE, GameFlags::cDEBRIS_TYPE_NONE );
				const tDebrisLogicDef& debrisDef = tGameApp::fInstance( ).fDebrisLogicDef( debrisT );

				tVec3f initialVel = tVec3f::cZeroVector;
				Physics::tStandardPhysics* physics = fQueryPhysicalDerived<Physics::tStandardPhysics>( );
				if( physics ) initialVel = physics->fVelocity( );

				sync_line_c( tSync::cSCUnit );
				fSpawnDebris( change.mTransitionPieces, initialVel, spawnInfluence, fSceneGraph( )->fFrameDeltaTime( ), *level->fOwnerEntity( ), debrisDef );

				// Spawn fire-fx on the pieces of debris...what a horrible way to be doing this!!!
				if( debrisT == GameFlags::cDEBRIS_TYPE_AIRPLANE_WRECKAGE )
				{
					f32 biggestSize = 5.f;
					for(u32 i = 0; i < change.mTransitionPieces.fCount( ); ++i)
					{
						tDebrisLogic* debrisLogic = change.mTransitionPieces[ i ]->fLogicDerivedStaticCast<tDebrisLogic>( );
						f32 debrisSize = debrisLogic->fDebrisBounds( ).fComputeDiagonal( ).fLength( );
						if( debrisSize > biggestSize && sync_rand( fIntInRange( 0, 100 ) ) > 50 )
						{
							biggestSize = debrisSize;
							tEntity* effect = tGameEffects::fInstance( ).fPlayEffect( debrisLogic->fOwnerEntity( ), cAirWreckageFireTrail );
							effect->fSetParentRelativeXform( debrisLogic->fOffset( ).fInverse( ) );
							Math::tMat3f xform = effect->fObjectToWorld( );
							xform.fScaleLocal( debrisSize / 25.f );
							effect->fMoveTo( xform );
						}
					}
				}
				else if( fTestBits( mOnFire, (1<<1)) || debrisT == GameFlags::cDEBRIS_TYPE_EXPLODING_BARREL )
				{
					s32 piecesOfFlamingDebris = 0;
					for(u32 i = 0; i < change.mTransitionPieces.fCount( ) && piecesOfFlamingDebris < 5; ++i)
					{
						if( sync_rand( fIntInRange( 0, 100 ) ) < ( 90 - ( piecesOfFlamingDebris * 17 ) ) )	// chances of spawning deteriorate as we have more and more..
						{
							tDebrisLogic* debrisLogic = change.mTransitionPieces[ i ]->fLogicDerivedStaticCast<tDebrisLogic>( );
							f32 debrisSize = debrisLogic->fDebrisBounds( ).fComputeDiagonal( ).fLength( );
							//if( debrisSize > 0.25f )		//aww who cares about this check??? there aren't any pieces too small, either way it looks cool!
							{
								tEntity* effect = tGameEffects::fInstance( ).fPlayEffect( debrisLogic->fOwnerEntity( ), cDebrisFireTrail );
								effect->fSetParentRelativeXform( debrisLogic->fOffset( ).fInverse( ) );
								Math::tMat3f xform = effect->fObjectToWorld( );
								xform.fScaleLocal( debrisSize );
								effect->fMoveTo( xform );
								++piecesOfFlamingDebris;
							}
						}
					}
				}
			}

			mCurrentState = stateIndex;

			fOnStateChanged( );
		}
	}
	void tUnitLogic::fRegisterForLevelEvent( u32 type, Sqrat::Function func )
	{
		if( mLevelEvents.fNull( ) )
			mLevelEvents.fReset( NEW tLevelEventHandler( ) );

		mLevelEvents->fAddObserver( (GameFlags::tLEVEL_EVENT)type, func );
	}
	void tUnitLogic::fFireLevelEvent( u32 type )
	{
		sigassert( type < GameFlags::cLEVEL_EVENT_COUNT );

		if( mLevelEvents )
			mLevelEvents->fFire( ( GameFlags::tLEVEL_EVENT ) type, this );
	}
	void tUnitLogic::fSetStateOverride( u32 state, b32 defer )
	{
		sigassert( state < cInvalidStateOverride );
		mStateOverride = state;
		if( !defer )
			fSetCurrentBreakStateAndSpawnDebris( mStateOverride, tVec3f::cZeroVector );
	}
	void tUnitLogic::fReapplyChangeState( )
	{
		tDamageContext dc;
		tDamageResult dr;
		dr.mHealthPercentEnd = fHealthPercent( );
		fChangeStatesAndSpawnDebris( dc, dr );
	}
	s32 tUnitLogic::fComputeChangeState( const tDamageContext& dc, const tDamageResult& dr )
	{
		if( mStateOverride == cInvalidStateOverride  )
			return s32( mFirstState + s32( (mLastState-mFirstState) * (1.0f - dr.mHealthPercentEnd) ) );
		else
			return mCurrentState; //if we're overriden dont change automatically
	}
	
	void tUnitLogic::fChangeStatesAndSpawnDebris( const tDamageContext& dc, const tDamageResult& dr )
	{
		s32 newState = fComputeChangeState( dc, dr );

		if( newState != mCurrentState || ( mUseDefaultEndTransition && fIsDestroyed( ) ) )
		{
			fSetCurrentBreakStateAndSpawnDebris( newState, dr.mSpawnInfluence );

			if( (mDeleteAfterStates || mUseDefaultEndTransition) && newState >= mDestroyState )
			{
				// goodbye !
				sigassert( fOwnerEntity( ) );
				fOwnerEntity( )->fSpawnChild( cBreakablePuffPath );
				fOwnerEntity( )->fDelete( );
			}
		}
	}
	
	void tUnitLogic::fOnStateChanged( ) 
	{ 
		if( !Perf_Audio_DisableDamaged )
		{
			if( mAudio )
			{
				// Not used: mAudio->fSetGameParam( AK::GAME_PARAMETERS::OBJECT_HEALTH_PERCENTAGE, fHealthPercent( ) * 100.f );
				mAudio->fHandleEvent( AK::EVENTS::PLAY_OBJECT_DAMAGE );
			}
		}

		fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_UNIT_STATE_CHANGED, NEW Logic::tIntEventContext( mCurrentState ) ) );
	}

	void tUnitLogic::fIncrementYawConstraint( s32 dir, b32 onSpawn, tPlayer* player )
	{
		sigassert( fConstrainYaw( ) );

		if( onSpawn && mCreationType == cCreationTypeFromLevel )
		{
			//acquire constraint axis from current direction
			tAxisAnglef aa( tQuatf( fDefaultFacingDirection( ), fOwnerEntity( )->fObjectToWorld( ).fZAxis( ).fNormalizeSafe( tVec3f::cZAxis ) ) );
			const f32 angleStep = c2Pi / mConstraintQuadrantCnt;
			mConstrainYaw = -fRound<u8>( aa.mAngle / angleStep );
		}

		mConstrainYaw += dir;
		while( mConstrainYaw >= mConstraintQuadrantCnt ) mConstrainYaw -= mConstraintQuadrantCnt;
		while( mConstrainYaw < 0 ) mConstrainYaw += mConstraintQuadrantCnt;

		mConstraintAxis = tQuatf( tAxisAnglef( tVec3f::cYAxis, fConstraintAxisAngle( ) ) ).fRotate( fDefaultFacingDirection( ) );

		if( fCreationType( ) == cCreationTypeGhost )
		{
			tMat3f xform = fOwnerEntity( )->fObjectToWorld( );
			xform.fOrientZAxis( mConstraintAxis );
			fOwnerEntity( )->fMoveTo( xform );
		}
	}
	void tUnitLogic::fSetupTintStack( )
	{
		mTintStack.fStack( ).fSetCount( cTintTypeCount );

		if( mCreationType != cCreationTypeFromGenerator ) 
			mTintStack.fStack( )[ cTintTypeSelected ].fReset( NEW Gfx::tSolidTint( Gameplay_UnitTint_Selection, Gameplay_UnitTint_SelectionBlend ) );
		
		mTintStack.fStack( )[ cTintTypeDamage ].fReset( NEW Gfx::tSolidTint( Gameplay_UnitTint_Damage, Gameplay_UnitTint_DamageBlend ) );
	}
	void tUnitLogic::fStepTintStack( f32 dt )
	{
		profile( cProfilePerfTintStack );

		if( mCreationType != cCreationTypeGhost )
		{
			mTintStack.fStep( dt );
			if( !Perf_CheckTintChanged || mTintStack.fChanged( ) || mUnderCursor )
			{
				Gfx::tRenderableEntity::fSetRgbaTintRespectNoIndirectTinting( *fOwnerEntity( ), mTintStack.fCurrentTint( ), mDontLightUpChildren );
			}
		}
	}
	void tUnitLogic::fUpdateSelectionTint( )
	{
		if( mTintStack.fHasIndex( cTintTypeSelected ) )
			mTintStack.fStack( )[ cTintTypeSelected ]->fSetTargetBlendStrength( mSelectionOverride || (mUnderCursor && !mUnderUserControl) ? 1.f : 0.f );
	}
	void tUnitLogic::fEnableRTSCursorPulse( )
	{
		mTintStack.fStack( )[ cTintTypeRTSCursorFlash ].fReset( NEW tRTSCursorTintEntry( *this, Gameplay_UnitTint_Selection, Gameplay_UnitTint_SelectionBlend, Gameplay_UnitTint_SelectionFlashRate, Gameplay_UnitTint_SelectionMaxDist ) );
		mTintStack.fStack( )[ cTintTypeRTSCursorFlash ]->fSetActivationTime( -1.f );
	}
	f32 tUnitLogic::fSelectionRadius( ) const
	{
		return mSelectionRadius;
	}
	b32 tUnitLogic::fIsSelectionShape( tEntity& shape ) const
	{
		return shape.fName( ) == cSelectionName;
	}
	b32 tUnitLogic::fGoalBoxCheck( )
	{
		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( levelLogic && "No level logic found!" );

		for( u32 i = 0; i < levelLogic->fGoalBoxCount( ); ++i )
		{
			tGoalBoxLogic* goalBox = levelLogic->fGoalBox( i )->fLogicDerived< tGoalBoxLogic >( );
			if( goalBox && goalBox->fTeam( ) != mTeam )
			{
				if( goalBox->fCheckInBounds( this ) )
				{
					goalBox->fSomeoneEntered( );
					return true;
				}
			}
		}

		if( !mWillNotEndInGoal )
			log_warning( 0, "Unit reached end of path that was supposed to end in a goal box or park point (but did not)!" );

		return false;
	}

	b32 tUnitLogic::fInCameraBox( ) const
	{
		// destroy vehicle if it's outside camera box
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level )
		{
			tShapeEntity* box = level->fCameraBox( fTeam( ) );
			if( box )
			{
				tVec3f pos = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
				pos.y = box->fBox( ).fCenter( ).y; //always satisfy y
				return box->fContains( pos );
			}
		}

		return true;
	}

	void tUnitLogic::fShowInUseIndicator( Gui::tInUseIndicator* indicator )
	{
		if( indicator )
			indicator->fShow( );
	}

	void tUnitLogic::fReachedEnemyGoal( )
	{
		tGrowableArray< tPlayerPtr > playersOnTeam; 
		tGameApp::fInstance( ).fFindAllEnemyPlayers( mTeam, playersOnTeam );
		for( tGrowableArray< tPlayerPtr >::tIterator i = playersOnTeam.fBegin( ); i != playersOnTeam.fEnd( ); ++i )
		{
			(*i)->fEnemyReachedGoal( this );
		}

		if( mLevelEvents )
			mLevelEvents->fFire( GameFlags::cLEVEL_EVENT_REACHED_GOAL, this );

		tTutorialEvent reachedGoalEvent = tTutorialEvent( GameFlags::cTUTORIAL_EVENT_UNIT_REACHED_GOAL );
		reachedGoalEvent.mEntity = fOwnerEntity( );
		tGameApp::fInstance( ).fCurrentLevel( )->fHandleTutorialEvent( reachedGoalEvent );
	}

	b32 tUnitLogic::fHasPath( )
	{
		tUnitPath *up = fUnitPath( );
		
		b32 result = up && up->fHasWaypoints( );
		return result;
	}
	void tUnitLogic::fExplodeIntoParts( )
	{
		sync_event_v_c( mDontJib, tSync::cSCUnit );
		if( !mDontJib )
		{
			tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), cSoldierExplodedEffect );
			tEntity* parent = mAlternateDebrisMeshSpawnParentEnt ? mAlternateDebrisMeshSpawnParentEnt.fGetRawPtr( ) : fOwnerEntity( );
			fExplodeIntoPartsImp( parent, mAlternateDebrisMesh, Gameplay_Character_NumInnerParts );
		}
	}
	void tUnitLogic::fExplodeIntoAllParts( )
	{
		sync_event_v_c( mDontJib, tSync::cSCUnit );
		if( !mDontJib )
		{
			tEntity* parent = mAlternateDebrisMeshSpawnParentEnt ? mAlternateDebrisMeshSpawnParentEnt.fGetRawPtr( ) : fOwnerEntity( );
			fExplodeIntoPartsImp( parent, mAlternateDebrisMesh, -1 );
		}
	}
	void tUnitLogic::fExplodeIntoPartsImp( tEntity* entity, const tFilePathPtr& debrisPath, s32 needed )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( !level )
			return;

		const tDebrisLogicDef& def = tGameApp::fInstance( ).fDebrisLogicDef( (GameFlags::tDEBRIS_TYPE)entity->fQueryEnumValue( GameFlags::cENUM_DEBRIS_TYPE, GameFlags::cDEBRIS_TYPE_CHARACTER ) );

		//collect prop items to spawn
		tGrowableArray<tEntity*> props;
		entity->fAllDescendentsWithAllTags( GameFlags::cFLAG_SPAWN_AS_DEBRIS, props );

		sync_line_c( tSync::cSCUnit );
		fSpawnDebris( props, tVec3f::cZeroVector, tVec3f::cZeroVector, 0.f, *level->fOwnerEntity( ), def );

		//spawn piece and get state count
		tEntity* part = level->fOwnerEntity( )->fSpawnChild( debrisPath );
		if( part )
		{
			s32 stateCnt, discard;
			tMeshEntity::fStateCount( *part, stateCnt, discard );
			++stateCnt;

			if( needed < 0 ) needed = stateCnt;

			for( s32 state = 0; state <= stateCnt; ++state )
			{
				s32 remaining = stateCnt - state;
				if( needed > sync_rand( fIntInRange( 0, remaining-1 ) ) )
				{
					sync_event_v_c( state, tSync::cSCUnit );

					//choose randomly distributed states
					tMeshEntity::tChangeMeshState change( 0, state, false, false );
					change.fChangeState( *part );

					tDebrisLogic *dl = NEW tDebrisLogic( def );
					tLogicPtr *dlp = NEW tLogicPtr( dl );

					part->fMoveTo( entity->fObjectToWorld( ) );
					part->fAcquireLogic( dlp );
					dl->fPhysicsSpawn( tVec3f::cZeroVector, tVec3f::cZeroVector, 0.f );

					--needed;

					if( needed > 0 )
						part =level->fOwnerEntity( )->fSpawnChild( debrisPath );
				}
			}
		}
		else
			log_warning( 0, "Could not spawn debris for fExplodeIntoPartsImp: " << debrisPath );
	}
	void tUnitLogic::fSetDamageTintColor( const Math::tVec4f& tint )
	{
		if( mTintStack.fHasIndex( cTintTypeDamage ) ) mTintStack.fStack( )[ cTintTypeDamage ]->fSetCurrentTint( tint );
	}
	void tUnitLogic::fResetAmmo( )
	{
		for( u32 s = 0; s < mWeaponStations.fCount( ); ++s )
		{
			tWeaponStationPtr& station = mWeaponStations[ s ];
			for( u32 b = 0; b < station->fBankCount( ); ++b )
			{
				tWeaponBankPtr& bank = station->fBank( b );
				for( u32 w = 0; w < bank->fWeaponCount( ); ++w )
					bank->fWeapon( w )->fResetAmmoCount( );
			}
		}
	}
	void tUnitLogic::fEnableWeapons( b32 enable )
	{
		for( u32 i = 0; i < mWeaponStations.fCount( ); ++i )
			mWeaponStations[ i ]->fEnable( enable );
	}
	b32	tUnitLogic::fHasWeapon( u32 station, u32 bank ) const
	{
		return fHasWeaponStation( station ) && fWeaponStation( station )->fHasBank( bank );
	}	
	b32	tUnitLogic::fHasWeapon( u32 station, u32 bank, u32 weapon ) const
	{
		if( fHasWeaponStation( station ) && fWeaponStation( station )->fHasBank( bank ) )
			return fWeaponStation( station )->fBank( bank )->fHasWeapon( weapon );

		return false;
	}
	tWeaponStationPtr& tUnitLogic::fWeaponStation( u32 index )
	{
		log_assert( index < mWeaponStations.fCount( ), "mWeapons[" << index << "] outside range.  Current count: " << mWeaponStations.fCount( ) );
		return mWeaponStations[ index ];
	}
	const tWeaponStationPtr& tUnitLogic::fWeaponStation( u32 index ) const
	{
		log_assert( index < mWeaponStations.fCount( ), "mWeapons[" << index << "] outside range.  Current count: " << mWeaponStations.fCount( ) );
		return mWeaponStations[ index ];
	}
	tWeaponStation* tUnitLogic::fWeaponStationRawPtr( u32 index )
	{
		fCheckWeaponStationCount( index );
		return mWeaponStations[ index ].fGetRawPtr( );
	}
	tWeapon* tUnitLogic::fWeaponRawPtr( u32 station, u32 bank, u32 weapon )
	{
		log_assert( fHasWeapon( station, bank, weapon ), "Weapon does not exist: " << station << ", " << bank << ", " << weapon );
		return fWeaponStation( station )->fBank( bank )->fWeapon( weapon ).fGetRawPtr( );
	}
	void tUnitLogic::fCheckWeaponStationCount( u32 index )
	{
		if( mWeaponStations.fCount( ) < index + 1 )
		{
			const u32 currentCount = mWeaponStations.fCount( );
			mWeaponStations.fSetCount( index + 1 );

			for( u32 i = currentCount; i < mWeaponStations.fCount( ); ++i )
				mWeaponStations[ i ] = tWeaponStationPtr( NEW tWeaponStation( this ) );
		}
	}
	f32 tUnitLogic::fWeaponMaxRange( ) const
	{
		f32 max = 0;
		for( u32 i = 0; i < mWeaponStations.fCount( ); ++i )
			max = fMax( max, mWeaponStations[ i ]->fMaxRange( ) );
		return max;
	}
	f32 tUnitLogic::fWeaponMinRange( ) const
	{
		f32 min = cInfinity;
		for( u32 i = 0; i < mWeaponStations.fCount( ); ++i )
			min = fMin( min, mWeaponStations[ i ]->fMinRange( ) );
		return min;
	}
	const tVec3f& tUnitLogic::fTargetOffset( ) const
	{
		// TODO: If we have multiple offsets, come up with a convention for picking one or just pick one at random
		return mTargetOffset;
	}
	void tUnitLogic::fCollectSelectionShapes( )
	{
		tSpheref combinedSphere( 0.f );
		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			tEntity* child = fOwnerEntity( )->fChild( i ).fGetRawPtr( );
			if( !fIsSelectionShape( *child ) )
				continue;
			tShapeEntity* shape = child->fDynamicCast< tShapeEntity >( );
			if( !shape )
				continue;
			mSelectionShapes.fPushBack( tShapeEntityPtr( shape ) );
			combinedSphere |= mSelectionShapes.fBack( )->fParentRelativeSphere( );
		}

		mSelectionRadius = combinedSphere.mCenter.fLength( ) + combinedSphere.mRadius;
	}

	namespace
	{
		class tPassengerFinder
		{
		public:
			mutable tGrowableArray<tEntityPtr>& mPassengersOut;
			mutable tGrowableArray<u32> mIDs;

			static const u32 cUsableRandomCount = GameFlags::cPERSONALITY_TYPE_COMMANDO;

			tPassengerFinder( tGrowableArray<tEntityPtr>& passOut )
				: mPassengersOut( passOut )
			{ 
				// only assign one personal each personality type
				mIDs.fSetCount( cUsableRandomCount );
				for( u32 i = 0; i < cUsableRandomCount; ++i ) 
					mIDs[ i ] = i;
			}

			b32 operator( ) ( tEntity& e ) const
			{
				tVehiclePassengerLogic* soldierLogic = e.fLogicDerived<tVehiclePassengerLogic>( );
				if( soldierLogic )
				{
					mPassengersOut.fPushBack( tEntityPtr( &e ) );

					// Assign random personality id
					u32 id;
					if( mIDs.fCount( ) )
					{
						u32 index = sync_rand( fIntInRange( 0, (s32)mIDs.fCount( ) - 1 ) );
						id = mIDs[ index ];
						mIDs.fErase( index );
					}
					else //there are none left just give them a random one.
						id = sync_rand( fIntInRange( 0, cUsableRandomCount - 1 ) );

					soldierLogic->fSetPersonalityType( id );
				}

				return false;
			}
		};
	}

	void tUnitLogic::fCollectSoldiers( tGrowableArray<tEntityPtr>& entitiesOut )
	{
		fOwnerEntity( )->fForEachDescendent( tPassengerFinder( entitiesOut ) );
	}

	tVec3f tUnitLogic::fUseCamOffset( ) const
	{
		if( mUseCamNear.fNull( ) ) return tVec3f( 0.f, 10.f, -25.f );
		
		tVec3f useCamOffset = mUseCamNear->fObjectToWorld( ).fGetTranslation( );

		if( mUseCamFar )
			useCamOffset = fLerp( useCamOffset, mUseCamFar->fObjectToWorld( ).fGetTranslation( ), mUseCamBlendValue );

		return fOwnerEntity( )->fWorldToObject( ).fXformPoint( useCamOffset );
	}
	tVec3f tUnitLogic::fUseCamLookDir( ) const 
	{
		if( mUseCamNear.fNull( ) ) return tVec3f( 0.f, -0.25f, 1.f ).fNormalize( );

		tVec3f useCamLookDir = mUseCamNear->fObjectToWorld( ).fZAxis( );
		if( mUseCamFar )
			useCamLookDir = fLerp( useCamLookDir, mUseCamFar->fObjectToWorld( ).fZAxis( ), mUseCamBlendValue );

		useCamLookDir = fOwnerEntity( )->fWorldToObject( ).fXformVector( useCamLookDir );
		useCamLookDir.fNormalizeSafe( tVec3f::cZAxis );
		return useCamLookDir;
	}
	Math::tVec3f tUnitLogic::fUseCamLookTarget( ) const 
	{
		if( mUseCamTarget )
			return mUseCamTarget->fParentRelative( ).fGetTranslation( );
		else
			return Math::tVec3f::cZAxis;
	}
	f32 tUnitLogic::fDistToUnit( const tVec3f& offset, const tVec3f& lookDir ) const 
	{ 
		return fMax( 0.1f, fAbs( tPlanef( -lookDir, tVec3f::cZeroVector ).fSignedDistance( offset ) ) );
	}
	void tUnitLogic::fUseCamZoomFromGamepad( f32 dt, f32 stickYAxis )
	{
		if( stickYAxis >= 0.25f || stickYAxis <= -0.25f )
			mUseCamBlendValue = fClamp( mUseCamBlendValue + -stickYAxis * dt * 0.75f, 0.f, 1.f );
	}
	void tUnitLogic::fAddHealthBar( )
	{
		if( fTeam( ) == GameFlags::cTEAM_NONE || fUnitAttributeMaxHitPoints( ) <= 0.f || tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
			return;

		const u32 levelObject = fOwnerEntity( )->fQueryEnumValueInherited( GameFlags::cENUM_SPECIAL_LEVEL_OBJECT );
		if( levelObject == GameFlags::cSPECIAL_LEVEL_OBJECT_BOSS )
			return;

		tUserArray users;
		for( u32 i = 0; i < tGameApp::fInstance( ).fPlayers( ).fCount( ); ++i )
		{
			tPlayer& player = *tGameApp::fInstance( ).fPlayers( )[ i ];
			if( !player.fIsActive( ) )
				continue;
			if( !player.fUser( )->fIsViewportVirtual( ) && ( !fTeamPlayer( ) || fTeam( ) != player.fTeam( ) ) )
				users.fPushBack( player.fUser( ) );
		}

		tVec4f healthBarSize = fUnitSharedTable( ).fIndexByRowCol<tVec4f>( mUnitIDString, cUnitSharedHealthBarSize );
		mHealthBar.fReset( NEW Gui::tHealthBar( users ) );
		mHealthBar->fSetSize( healthBarSize.fXY() );
		mHealthBar->fSetObjectSpaceOffset( mHealthBarOffset );
		mHealthBar->fSpawn( *fOwnerEntity( ) );
	}
	void tUnitLogic::fSetHealthBarColor( const Math::tVec4f& color ) 
	{
		if( mHealthBar ) 
			mHealthBar->fSetColor( color ); 

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level )
			level->fScreenSpaceHealthBarList( )->fSetColor( *this, color );
	}
	void tUnitLogic::fSetHealthBarFlashAndFill( f32 flash, f32 fill )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level )
			level->fScreenSpaceHealthBarList( )->fSetFlashAndFill( *this, flash, fill ); 
	}
	void tUnitLogic::fPickedUp( )
	{
		Sqrat::Function f( fOwnerEntity( )->fScriptLogicObject( ), "OnPickedUp" );
		if( !f.IsNull( ) )
			f.Execute( );
	}

	void tUnitLogic::fSetPickup( u32 pickUp )
	{
		mPickup = pickUp;

		// set enum because this is _an_ pickup. ie logic type == pickup
		fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_PICKUPS, pickUp ) );
	}
	void tUnitLogic::fAddPickup( u32 pickup )
	{
		if( mPickup == ~0 )
		{
			mPickup = pickup;
			// DOES NOT SET ENTITY ENUM, this is not A pickup, a picup has been added
			fCreatePickupIcon( pickup );
		}
	}
	void tUnitLogic::fAddRandomPickup( )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( tGameApp::fInstance( ).fDifficulty( ) == GameFlags::cDIFFICULTY_GENERAL 
			|| (level && (level->fDisableRandomPickups( ) || (level->fMapType( ) != GameFlags::cMAP_TYPE_CAMPAIGN && level->fMapType( ) != GameFlags::cMAP_TYPE_SURVIVAL))) )
			return;

		// Must be campaign, sp or co-op, so either player is fine.
		if( tGameApp::fInstance( ).fFrontEndPlayer( )->fTeam( ) == fTeam( ) )
			return; // no pickups over allied dudes

		f32 roll = sync_rand( fFloatInRange( 0, 1.f ) );
		if( roll <= Gameplay_RandomPickupFreq )
		{
			// this prevents bosses and component units from getting pickups
			if( fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_SPECIAL_LEVEL_OBJECT ) != ~0
				|| fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_LINKED_HITPOINTS ) != ~0 )
				return;

			fAddPickup( GameFlags::cPICKUPS_BARRAGE_ROLL );
		}
	}
	void tUnitLogic::fCreatePickupIcon( u32 pickup )
	{
		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
		{
			tUserArray users;
			tGameApp& gameApp = tGameApp::fInstance( );

			for( u32 i = 0; i < gameApp.fPlayers( ).fCount( ); ++i )
			{
				tPlayer* player = gameApp.fPlayers( )[ i ].fGetRawPtr( );
				if( player && player->fIsActive( ) && !player->fUser( )->fIsViewportVirtual( ) )
					users.fPushBack( player->fUser( ) );
			}

			mPickupIcon.fReset( NEW Gui::tTurretRadialIndicator( gameApp.fGlobalScriptResource( tGameApp::cGlobalScriptUnitPickupIndicator ), users ) );
			const Math::tVec3f indicatorOffset( 0, 3, 0 );
			mPickupIcon->fSetObjectSpaceOffset( indicatorOffset );
			mPickupIcon->fSpawn( *fOwnerEntity( ) );
			mPickupIcon->fSetParentRelativeXform( Math::tMat3f::cIdentity );
			mPickupIcon->fSetPercent( (f32)pickup ); // ShowPercent for this sort of thing just sets a new image for the pickup
			mPickupIcon->fShow( );
		}
	}
	void tUnitLogic::fReleasePickupIcon( )
	{
		if( mPickupIcon )
		{
			mPickupIcon->fHide( );
			mPickupIcon.fRelease( );
		}
	}
	void tUnitLogic::fAddToMiniMap( )
	{
		tDynamicArray< tPlayerPtr >& players = tGameApp::fInstance( ).fPlayers( );
		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			const Gui::tMiniMapPtr& miniMap = players[ i ]->fMiniMap( );
			if( !players[ i ]->fIsActive( ) || !miniMap || players[ i ]->fCountry( ) == fCountry( ) )
				continue;

			Gui::tMiniMapUnitIcon* icon = NEW Gui::tMiniMapUnitIcon( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptMiniMap ), players[ i ]->fUser( ) );
			icon->fSetUnit( this );
			fAddCanvasObject( Gui::tScriptedControlPtr( icon ) );
			miniMap->fAddUnit( icon->fCanvas( ).fScriptObject( ) );
		}
	}
	tRefCounterPtr<tEntitySaveData> tUnitLogic::fStoreSaveGameData( b32 entityIsPartOfLevelFile, tSaveGameRewindPreview& preview )
	{
		return tRefCounterPtr<tEntitySaveData>( NEW tEntitySaveData( ) );
	}
	void tUnitLogic::fRegisterForSaveGame( b32 addTo )
	{
		if( addTo )
		{
			if( fCreationType( ) == cCreationTypeGhost )
				return;
			if( !fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_SAVEABLE ) )
				return;
			if( fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_MINIGAME_UNIT ) )
				return;

			if( tGameApp::fInstance( ).fCurrentLevel( ) )
				tGameApp::fInstance( ).fCurrentLevel( )->fRegisterSaveableEntity( fOwnerEntity( ) );
		}
		else
		{
			fOwnerEntity( )->fRemoveGameTags( GameFlags::cFLAG_SAVEABLE );
			if( tGameApp::fInstance( ).fCurrentLevel( ) )
				tGameApp::fInstance( ).fCurrentLevel( )->fUnRegisterSaveableEntity( fOwnerEntity( ) );
		}
	}
	f32	tUnitLogic::fHealthPercent( ) const
	{
		f32 maxHp = fUnitAttributeMaxHitPoints( );
		if( maxHp == 0 )
			return 0.f;
		else
			return fClamp( mHitPoints / maxHp, 0.f, 1.f );
	}
	void tUnitLogic::fResetHitPoints( )
	{
		mHitPoints = fUnitAttributeMaxHitPoints( );
	}
	void tUnitLogic::fFindHitpointLinkedChildren( )
	{
		tEntity* ownerEnt = fOwnerEntity( );
		sigassert( ownerEnt );

		for( u32 i = 0; i < ownerEnt->fChildCount( ); ++i )
		{
			tEntity* child = tReferenceFrameEntity::fSkipOverRefFrameEnts( fOwnerEntity( )->fChild( i ).fGetRawPtr( ) );
			if( child->fQueryEnumValue( GameFlags::cENUM_LINKED_HITPOINTS ) != ~0 )
			{
				tUnitLogic* unitLogic = child->fLogicDerived< tUnitLogic >( );
				if( unitLogic )
				{
					mHitPointLinkedChildren.fSetCount( GameFlags::cWEAPON_INDEX_COUNT + 1 );

					unitLogic->fSetHitpointLinkedUnitLogic( this );
					u32 weaponIndex = child->fQueryEnumValue( GameFlags::cENUM_WEAPON_INDEX, ~0 );
					if( weaponIndex == ~0 ) weaponIndex = GameFlags::cWEAPON_INDEX_COUNT;
					mHitPointLinkedChildren[ weaponIndex ].fPushBack( tEntityPtr( child ) );
					mAllHitPointLinkedChildren.fPushBack( tEntityPtr( child ) );
				}
			}			
		}
	}
	void tUnitLogic::fSetHitpointLinkedUnitLogic( tUnitLogic* unitLogic ) 
	{
		mHitpointLinkedUnitLogic = unitLogic; 

		tUnitLogic *ignoreParent = unitLogic ? unitLogic : this;
		fSetWeaponIgnoreParent( ignoreParent );
	}

	tEntity* tUnitLogic::fSpawnReplace( tEntity* parent, const tFilePathPtr& path )
	{
		sigassert( parent );
		tReferenceFrameEntity* owner = parent->fParent( )->fDynamicCast<tReferenceFrameEntity>( );

		tEntity* result = NULL;

		if( owner ) 
		{
			result = owner->fSpawnChild( path );
			if( result )
			{
				result->fSetName( parent->fName( ) );
				parent->fDelete( );
			}
		}
		else
			result = parent->fSpawnChild( path );

		return result;
	}

	void tUnitLogic::fCancelAllWeaponFire( ) 
	{ 
		// Tell everyone to stop firing!
		// internal weapons
		for( u32 s = 0; s < fWeaponStationCount( ); ++s )
			for( u32 b = 0; b < fWeaponStation( s )->fBankCount( ); ++b )
				for( u32 w = 0; w < fWeaponStation( s )->fBank( b )->fWeaponCount( ); ++w )
				{
					tWeapon* weap = fWeaponRawPtr( s, b, w );
					weap->fSetAIFireOverride( false );
					weap->fSetAITargetOverride( NULL );
					if( weap->fFiring( ) )
						weap->fEndFire( );
				}

		// call to children weapons
		Logic::tEvent newE( GameFlags::cEVENT_WEAPON_ACTION, NEW Logic::tIntEventContext( GameFlags::cWEAPON_ACTION_END_FIRE ) );
		for( u32 i = 0; i < mHitPointLinkedChildren.fCount( ); ++i )
		{
			for( u32 c = 0; c < mHitPointLinkedChildren[ i ].fCount( ); ++c )
			{
				mHitPointLinkedChildren[ i ][ c ]->fLogic( )->fHandleLogicEvent( newE );
			}
		}
	}

	void tUnitLogic::fDisableAIWeaponFire( b32 disable )
	{
		// internal weapons
		for( u32 s = 0; s < fWeaponStationCount( ); ++s )
			fWeaponStation( s )->fEnable( !disable );

		// call to children weapons
		Logic::tEvent newE( GameFlags::cEVENT_WEAPON_ACTION, NEW Logic::tIntEventContext( disable ? GameFlags::cWEAPON_ACTION_DISABLE : GameFlags::cWEAPON_ACTION_ENABLE ) );
		for( u32 i = 0; i < mHitPointLinkedChildren.fCount( ); ++i )
		{
			for( u32 c = 0; c < mHitPointLinkedChildren[ i ].fCount( ); ++c )
			{
				mHitPointLinkedChildren[ i ][ c ]->fLogic( )->fHandleLogicEvent( newE );
			}
		}
	}

	void tUnitLogic::fSetWeaponIgnoreParent( tUnitLogic* p )
	{
		for( u32 s = 0; s < fWeaponStationCount( ); ++s )
			for( u32 b = 0; b < fWeaponStation( s )->fBankCount( ); ++b )
				for( u32 w = 0; w < fWeaponStation( s )->fBank( b )->fWeaponCount( ); ++w )
					fWeaponRawPtr( s,b,w )->fInst( ).mIgnoreParent = p;
	}

	void tUnitLogic::fSetDisableControl( b32 disable ) 
	{ 
		mDisableControl = disable; 
		//if( disable )
		//	fCancelAllWeaponFire( );
	}

	void tUnitLogic::fShowFocalPrompt( const tStringPtr& prompt )
	{
		if( prompt.fExists( ) )
		{
			tEntity* promptPt = fOwnerEntity( )->fFirstDescendentWithName( cFocalPromptName );
			if( !promptPt )
				promptPt = fOwnerEntity( );

			tGameApp::fInstance( ).fCurrentLevel( )->fFocusTarget( promptPt, prompt );
		}
	}

	void tUnitLogic::fLinkChildAudioLogicsRecursive( tEntity* e, b32 linkRTPC, b32 linkEvents )
	{
		tAudioLogic* al = e->fLogicDerived<tAudioLogic>( );
		if( al )
		{
			if( linkRTPC ) mAudio->fAddSwitchAndParamsLinkedChild( al->fSource( ).fGetRawPtr( ) );
			if( linkEvents ) mAudio->fAddEventLinkedChild( al->fSource( ).fGetRawPtr( ) );
		}

		for( u32 i = 0; i < e->fChildCount( ); ++i )
			fLinkChildAudioLogicsRecursive( e->fChild( i ).fGetRawPtr( ), linkRTPC, linkEvents );
	}

	void tUnitLogic::fLinkChildAudioLogics( b32 linkRTPC, b32 linkEvents )
	{
		if( mAudio )
		{
			fLinkChildAudioLogicsRecursive( fOwnerEntity( ), linkRTPC, linkEvents );
			if( linkRTPC )
				mAudio->fSetSwitch( tGameApp::cUnitIDSwitchGroup, fAudioID( ) );
		}
	}

	b32 tUnitLogic::fHasHelmetProp( )
	{
		u32 country = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_COUNTRY );
		if( country == ~0 ) 
			return false;

		u32 props = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_CHARACTER_PROPS );
		if( props == ~0 ) 
			return false;

		const tStringHashDataTable& dt = tGameApp::fInstance( ).fCharacterPropsTable( country );
		u32 row = dt.fRowIndex( GameFlags::fCHARACTER_PROPSEnumToValueString( props ) );

		return dt.fIndexByRowCol<tStringPtr>( row, tGameApp::cCharacterPropsHelmetResource ).fExists( );		
	}

	Math::tVec4f tUnitTintLogic::fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const
	{
		if( varName == cSpecialCamTint )
		{
			return tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamUnitTint( viewportIndex );
		}

		return Math::tVec4f::cZeroVector;
	}

	void tUnitTintLogic::fAddTo( tEntityPtr& e )
	{
		tUnitTintLogic *dl = NEW tUnitTintLogic( );
		tLogicPtr *dlp = NEW tLogicPtr( dl );
		e->fAcquireLogic( dlp );
	}

	namespace 
	{ 
		static const tStringPtr cVocDeathKey( "Voc_Death" ); 
		devvar( u32, Gameplay_Character_DeathVocLimit, 3 );
	}

	void tUnitLogic::fVocDeath( )
	{
		if( tGameApp::fInstance( ).fDeathLimiter( ) < Gameplay_Character_DeathVocLimit )
		{
			++tGameApp::fInstance( ).fDeathLimiter( );
			tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), cVocDeathKey );
		}
	}
}

namespace Sig
{
	void tUnitLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tUnitLogic, tLogic, Sqrat::NoCopy<tUnitLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("CreationType"),		&tUnitLogic::fCreationTypeInt )
			.Prop(_SC("AttributeSize"),		&tUnitLogic::fUnitAttributeSize)
			.Prop(_SC("Team"),				&tUnitLogic::fTeam)
			.Prop(_SC("UnitID"),			&tUnitLogic::fUnitID)
			.Prop(_SC("UnitIDString"),		&tUnitLogic::fUnitIDString)
			.Prop(_SC("Country"),			&tUnitLogic::fCountry)
			.Prop(_SC("UnitType"),			&tUnitLogic::fUnitType)
			.Func(_SC("GoalBoxCheck"),		&tUnitLogic::fGoalBoxCheck )
			.Func(_SC("ReachedEnemyGoal"),	&tUnitLogic::fReachedEnemyGoal )
			.Func(_SC("ExplodeIntoParts"),	&tUnitLogic::fExplodeIntoParts )
			.Func(_SC("ExplodeIntoAllParts"),	&tUnitLogic::fExplodeIntoAllParts )
			.Func(_SC("SetAlternateDebrisMesh"), &tUnitLogic::fSetAlternateDebrisMesh )
			.Func(_SC("SetAlternateDebrisMeshParent"), &tUnitLogic::fSetAlternateDebrisMeshParent )
			.Func(_SC("SetDestroyedEffect"), &tUnitLogic::fSetDestroyedEffect )
			.Prop(_SC("HasPath"),			&tUnitLogic::fHasPath )
			.Prop(_SC("UnitPath"),			&tUnitLogic::fUnitPath)
			.Prop(_SC("EnableSelection"),	&tUnitLogic::fSelectionEnabled, &tUnitLogic::fEnableSelection )
			.Prop(_SC("UnderUserControl"),	&tUnitLogic::fUnderUserControlScript, &tUnitLogic::fSetUnderUserControl )
			.Prop(_SC("HitPoints"),			&tUnitLogic::fCurrentHitPoints, &tUnitLogic::fSetHitPoints )
			.Prop(_SC("MaxHitPoints"),		&tUnitLogic::fUnitAttributeMaxHitPoints )
			.Prop(_SC("HealthPercent"),		&tUnitLogic::fHealthPercent)
			.Func(_SC("ResetHitPoints"),	&tUnitLogic::fResetHitPoints)
			.Var(_SC("DamageModifier"),		&tUnitLogic::mDamageModifier)
			.Var(_SC("HitPointsModifier"),	&tUnitLogic::mHitPointsModifier)
			.Prop(_SC("UseDefaultEndTransition"),		&tUnitLogic::fUseDefaultEndTransition, &tUnitLogic::fSetUseDefaultEndTransition )
			.Func(_SC("WeaponStation"),					&tUnitLogic::fWeaponStationRawPtr )
			.Func(_SC("RegisterForLevelEvent"),			&tUnitLogic::fRegisterForLevelEvent )
			.Func(_SC("Weapon"),						&tUnitLogic::fWeaponRawPtr )
			.Func(_SC("FindHitpointLinkedChildren"),	&tUnitLogic::fFindHitpointLinkedChildren)
			.Func(_SC("FireLevelEvent"),				&tUnitLogic::fFireLevelEvent)
			.Prop(_SC("TakesDamage"),					&tUnitLogic::fTakesDamage, &tUnitLogic::fSetTakesDamage)
			.Func(_SC("AddHealthBar"),					&tUnitLogic::fAddHealthBar)
			.Func(_SC("SetHealthBarColor"),				&tUnitLogic::fSetHealthBarColor)
			.Func(_SC("SetHealthBarFlashAndFill"),		&tUnitLogic::fSetHealthBarFlashAndFill)			
			.Func(_SC("HitpointLinkedChildrenCount"),	&tUnitLogic::fHitPointLinkedChildrenCount)
			.Func(_SC("HitpointLinkedChild"),			&tUnitLogic::fHitPointLinkedChild)
			.Func(_SC("SetDamageTintColor"),			&tUnitLogic::fSetDamageTintColor)
			.Func(_SC("EnableRTSCursorPulse"),			&tUnitLogic::fEnableRTSCursorPulse)
			.Prop(_SC("FirstWaveLaunch"),				&tUnitLogic::fFirstWaveLaunch, &tUnitLogic::fSetFirstWaveLaunch)
			.StaticFunc(_SC("SpawnReplace"),			&tUnitLogic::fSpawnReplace)
			.Prop(_SC("ExtraMode"),						&tUnitLogic::fExtraMode, &tUnitLogic::fSetExtraMode)
			.Prop(_SC("CurrentPersistentEffect"),		&tUnitLogic::fCurrentPersistentEffect)	
			.Prop(_SC("UnitAttributeDestroyValue"),		&tUnitLogic::fUnitAttributeDestroyValue)
			.Func(_SC("DontStopBullets"),				&tUnitLogic::fDontStopBullets)
			.Overload<void (tUnitLogic::*)(void)>(_SC("Destroy"),		 &tUnitLogic::fDestroy)
			.Func(_SC("ForceDestroy"),					&tUnitLogic::fForceDestroyForScript)
			//.Overload<void (tUnitLogic::*)(tUnitLogic*, b32)>(_SC("Destroy"), &tUnitLogic::fDestroy)
			.Prop(_SC("RepairCost"),					&tUnitLogic::fUnitAttributeRepairCost)
			.Prop(_SC("SellValue"),						&tUnitLogic::fUnitAttributeSellValue)
			.Var(_SC("ChildDamageTintBlend"),			&tUnitLogic::mChildDamageTintBlend)
			.Var(_SC("FireEffectOverride"),				&tUnitLogic::mFireEffectOverride)
			.Var(_SC("DamageTransferModifier"),			&tUnitLogic::mDamageTransferModifier)
			.Func(_SC("RemoveFromAliveList"),			&tUnitLogic::fRemoveFromAliveList)
			.Func(_SC("SetStateOverride"),				&tUnitLogic::fSetStateOverride)
			.Func(_SC("ReapplyChangeState"),					&tUnitLogic::fReapplyChangeState )
			.Func(_SC("DisableAIWeaponFire"),			&tUnitLogic::fDisableAIWeaponFire)
			.Func(_SC("CancelAllWeaponFire"),			&tUnitLogic::fCancelAllWeaponFire)
			.Prop(_SC("Pickup"),						&tUnitLogic::fPickup, &tUnitLogic::fSetPickup)
			.Prop(_SC("DisableTimeScale"),				&tUnitLogic::fDisableTimeScale, &tUnitLogic::fSetDisableTimeScale)
			.Func(_SC("SetPersonalityType"),			&tUnitLogic::fSetPersonalityType)
			.Func(_SC("LinkChildAudioLogics"),			&tUnitLogic::fLinkChildAudioLogics)
			.Prop(_SC("WaveDisabledAIFire"),			&tUnitLogic::fWaveDisabledAIFire)
			.StaticFunc(_SC("UnitLocName"),				&tUnitLogic::fUnitLocNameScript)
			.StaticFunc(_SC("UnitLocClass"),			&tUnitLogic::fUnitLocClassScript)
			.Prop(_SC("HasHelmetProp"),					&tUnitLogic::fHasHelmetProp)
			.Prop(_SC("InAlarmZone"),					&tUnitLogic::fInAlarmZone, &tUnitLogic::fSetInAlarmZone)
			.Func(_SC("VocDeath"),						&tUnitLogic::fVocDeath)
			;
		vm.fRootTable( ).Bind(_SC("UnitLogic"), classDesc);

		vm.fConstTable( ).Const(_SC("CREATIONTYPE_FROMLEVEL"), ( int ) cCreationTypeFromLevel );
		vm.fConstTable( ).Const(_SC("CREATIONTYPE_GHOST"), ( int ) cCreationTypeGhost );
		vm.fConstTable( ).Const(_SC("CREATIONTYPE_FROMBUILDSITE"), ( int ) cCreationTypeFromBuildSite );
		vm.fConstTable( ).Const(_SC("CREATIONTYPE_FROMGENERATOR"), ( int ) cCreationTypeFromGenerator );
	}

}
