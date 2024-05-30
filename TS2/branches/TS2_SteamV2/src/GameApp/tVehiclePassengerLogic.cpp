#include "GameAppPch.hpp"
#include "tVehiclePassengerLogic.hpp"
#include "tVehicleLogic.hpp"
#include "tTurretLogic.hpp"
#include "AI/tMotionGoal.hpp"
#include "tCharacterLogic.hpp"
#include "tReferenceFrameEntity.hpp"
#include "tGameEffects.hpp"
#include "tGamePostEffectMgr.hpp"
#include "tVehiclePassengerAnimTrack.hpp"

using namespace Sig::Math;
using namespace Sig::Physics;

namespace Sig
{

	devvar( bool, Debug_Vehicle_SprungMassesDraw, false );
	devvar( f32, Gameplay_Vehicle_SprungMassesD, 12.0f );   
	devvar( f32, Gameplay_Vehicle_SprungMassesP, 50.0f ); 

	devvar( f32, Gameplay_Vehicle_IK_PosX, 0.35f );  
	devvar( f32, Gameplay_Vehicle_IK_PosY, 1.185f );
	devvar( f32, Gameplay_Vehicle_IK_PosZ, 0.5f ); 

	devvar( f32, Gameplay_Vehicle_RandomAnimMin, 5.0f );
	devvar( f32, Gameplay_Vehicle_RandomAnimMax, 10.0f );

	namespace
	{
		const tStringPtr cDefaultSingleShotWeapon( "USA_INFANTRY_RIFLE" );
		const tStringPtr cCharcterLProp( "l_prop" );
		const tStringPtr cCharcterRProp( "r_prop" );
		const tStringPtr cCharcterHelmProp( "attach_helm0" );
		static const tStringPtr cSpecialCamTint = tStringPtr( "SpecialCamTint" );

		tUnitLogic* fFindParent( tEntity* me )
		{
			tUnitLogic* parent = me->fParent( )->fFirstAncestorWithLogicOfType<tUnitLogic>( );
			while( parent )
			{
				if( parent->fLogicType( ) < GameFlags::cLOGIC_TYPE_COUNT )
					break;

				parent = parent->fOwnerEntity( )->fParent( )->fFirstAncestorWithLogicOfType<tUnitLogic>( );
			}

			return parent;
		}

		f32 fRandomPassengerAnimTime( )
		{
			return sync_rand( fFloatInRange( Gameplay_Vehicle_RandomAnimMin, Gameplay_Vehicle_RandomAnimMax ) );
		}
	}

	tVehiclePassengerLogic::tVehiclePassengerLogic( )
		: mOrphaned( false )
		, mSprung( false )
		, mGenRandomAnims( false )
		, mRandomAnimTime( 0.f )
		, mSprungMassToTransform( tMat3f::cIdentity )
		, mSingleShotWeaponID( cDefaultSingleShotWeapon )
		, mParent( NULL )
	{	
		mAnimatable.fSetLogic( this );	
	}

	void tVehiclePassengerLogic::fOnSpawn( )
	{
		mParent = fFindParent( fOwnerEntity( ) );
		sigassert( mParent && "Vehicle passengers must be under vehicle or turret logics." );
		mParentEnt.fReset( mParent->fOwnerEntity( ) );

		mParent->fQueryEnums( );
		u32 parentCountry = mParent->fCountry( );
		if( parentCountry != GameFlags::cCOUNTRY_USA && parentCountry != GameFlags::cCOUNTRY_USSR  && parentCountry != GameFlags::cCOUNTRY_BRITISH  && parentCountry != GameFlags::cCOUNTRY_GERMAN  && parentCountry != GameFlags::cCOUNTRY_FRENCH)
		{
			tSceneRefEntity* refEnt = mParent->fOwnerEntity( )->fDynamicCast<tSceneRefEntity>( );
			if( refEnt )
			{
				log_warning( 0, "No parent country in : " << refEnt->fSgResource( )->fGetPath( ) );
			}
			else
				log_warning( 0, "No parent country in : " << GameFlags::fUNIT_IDEnumToValueString( mParent->fUnitID( ) ) );
			sigassert( 0 );
		}

		fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_COUNTRY, mParent->fCountry( ) ) );
		fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_UNIT_ID, GameFlags::cUNIT_ID_PASSENGER_01 ) );
		mEnumsQueried = false;

		u32 passengerType = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_ARTILLERY_SOLDIER );
		if( passengerType == GameFlags::cARTILLERY_SOLDIER_INCONSEQUENTIAL )
		{
			fAddHealthBar( );
		}
		else
		{
			u32 hitLink = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_LINKED_HITPOINTS );
			if( hitLink == ~0 )
				fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_LINKED_HITPOINTS, GameFlags::cLINKED_HITPOINTS_TRANSFER_ONLY_DIRECT ) );
			else if( hitLink != GameFlags::cLINKED_HITPOINTS_TRANSFER_ONLY_DIRECT )
				fAddHealthBar( );
		}

		// Base spawn
		tUnitLogic::fOnSpawn( );
		fOnPause( false );

		fOutfitCharacterProps( *fOwnerEntity( ) );
		fOwnerEntity( )->fAddGameTags( GameFlags::cFLAG_DONT_INHERIT_STATE_CHANGE ); //dont inherent any state changes for characters

		fConfigureAudio( );
	}

	void tVehiclePassengerLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListAnimateMT );
			fRunListRemove( cRunListMoveST );
		}
		else
		{
			// add self to run lists
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListAnimateMT );
			fRunListInsert( cRunListMoveST );
		}
	}

	void tVehiclePassengerLogic::fPostGoalSet( )
	{
		tGoalDriven::fOnSpawn( this );

		//we need to OnActivate the current goal so it's mostate gets applied.
		// it's up to the goal to make sure the desired motion goal is on top when the master goal is set.
		tGoalDriven::fActST( this, 0.01f );

		mAnimatable.fOnSpawn( );
	}

	void tVehiclePassengerLogic::fConfigureAudio( )
	{
		// Audio IDs
		u32 crewman = fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_CREWMAN );
		if( crewman != ~0 )
			mCrewman = GameFlags::fCREWMANEnumToValueString( crewman );

		mAudio->fSetSwitch( tGameApp::cCrewmanSwitchGroup, mCrewman );
		
		fConfigureBasicCharacterAudio( );
	}

	void tVehiclePassengerLogic::fGenerateRandomAnimEvents( b32 enable )
	{
		mGenRandomAnims = enable;
		if( enable )
			mRandomAnimTime = fRandomPassengerAnimTime( );
	}

	void tVehiclePassengerLogic::fOnSkeletonPropagated( )
	{
		tUnitLogic::fOnSkeletonPropagated( );
		mAnimatable.fListenForAnimEvents( *this );
	}

	void tVehiclePassengerLogic::fOnDelete( )
	{
		for( u32 i = 0; i < mHandTargets.fCount( ); ++i )
			mHandTargets[ i ].fRelease( );

		mPhysics.fOnDelete( );
		mAnimatable.fOnDelete( );
		mParentEnt.fRelease( );
		mParent = NULL;
		tUnitLogic::fOnDelete( );
	}

	void tVehiclePassengerLogic::fSetup( const Math::tVec3f& offset, const Math::tMat3f& myTransform, const Math::tMat3f &parent )
	{
		mPhysics.fBasicSetup( GameFlags::cFLAG_GROUND, GameFlags::cFLAG_COLLISION, 2.f );
		mPhysics.fDisable( true );

		tMat3f sprungBasis( tQuatf::cIdentity, offset );
		mSprungMass.fSetBasis( sprungBasis );
		mSprungMass.fResetParent( parent );

		f32 verticalMaxAcc = 20.0f; //2 g
		f32 forwardMaxAcc = 15.0f;
		f32 horizontalMaxAcc = 25.0f;
		mSprungMass.fSetMaxAcc( tVec3f( horizontalMaxAcc, verticalMaxAcc, forwardMaxAcc ) );

		mSprungMassToTransform = myTransform * sprungBasis.fInverse( );
		mSprung = true;
	}

	void tVehiclePassengerLogic::fSetTargets( tEntity* leftHand, tEntity* rightHand )
	{
		mHandTargets[ 0 ].fReset( leftHand );
		mHandTargets[ 1 ].fReset( rightHand );
	}

	void tVehiclePassengerLogic::fEject( const Math::tVec3f& launchVel )
	{
		mPhysics.fDisable( false );
		mPhysics.fJump( launchVel );
		mTakesDamage = 0;

		mOrphaned = true;
		fOnPause( false );
		Gfx::tRenderableEntity::fSetRgbaTint( *fOwnerEntity( ), Math::tVec4f::cOnesVector );
	}

	f32 tVehiclePassengerLogic::fPassengerTimeScale( ) const
	{
		if( mParent )
			return mParent->fTimeScale( );
		else
			return fTimeScale( );
	}

	void tVehiclePassengerLogic::fAnimateMT( f32 dt )
	{
		profile( cProfilePerfPassengerAnimateMT );

		dt *= fPassengerTimeScale( );
		mAnimatable.fAnimateMT( dt );

		if( mOrphaned )
		{
			mPhysics.fSetTransform( mPhysics.fApplyRefFrameDelta( fOwnerEntity( )->fObjectToWorld( ), mAnimatable.fAnimatedSkeleton( )->fRefFrameDelta( ) ) );
			mPhysics.fPhysicsMT( this, dt, false );
		}
	}

	void tVehiclePassengerLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfPassengerMoveST );
		dt *= fPassengerTimeScale( );

		mAnimatable.fMoveST( dt );

		if( mOrphaned )
		{
			fOwnerEntity( )->fMoveTo( mPhysics.fTransform( ) );

			if( mPhysics.fStartedFalling( ) ) 
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_FALL ) );
			if( mPhysics.fJustLanded( ) ) 
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_LAND ) );
		}
		else if( mSprung )
		{
			tMat3f sprungBasis( tQuatf::cIdentity, mSprungMass.fGetPosition( ) );
			tMat3f newTransform = mSprungMassToTransform * sprungBasis;
			fOwnerEntity( )->fSetParentRelativeXform( newTransform );
		}

		if( mGenRandomAnims )
		{
			mRandomAnimTime -= dt;
			sync_event_v_c( mRandomAnimTime, tSync::cSCVehicle );
			if( mRandomAnimTime < 0.f )
			{
				mRandomAnimTime = fRandomPassengerAnimTime( );
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_RANDOM_CHARACTER_ANIM, NEW Logic::tIntEventContext( 0 ) ) );
			}
		}
	}
	
	b32 tVehiclePassengerLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_GAME_EFFECT:
			{
				const tEffectLogicEventContext* context = e.fContext<tEffectLogicEventContext>( );
				if( context && context->mAudio )
				{
					mAudio->fHandleEvent( context->mAudioEvent );
					context->mAudio.fRelease( );
				}
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
					else if( mSingleShotWeaponID.fExists( ) && context->mEventTypeCppValue == GameFlags::cKEYFRAME_EVENT_FIRE_WEAPON )
					{
						tEntity* weap = fOwnerEntity( )->fFirstDescendentWithName( tWeapon::cWeaponAttachName );
						if( weap )
						{
							tVec3f target = weap->fObjectToWorld( ).fXformPoint( tVec3f( 0,0,1000.f ) );
							tWeapon::fSingleShot( weap->fObjectToWorld( ), target, mSingleShotWeaponID, fTeam( ), fOwnerEntity( ) );
						}

						return true;
					}
				}
			}
			break;
		}
		return tUnitLogic::fHandleLogicEvent( e );
	}

	void tVehiclePassengerLogic::fCallbackMT( IK::tIKCallbackArgs& args )
	{
		if( args.mChannel == 2 )
		{
			// head, -Y is forward
			tMat3f world = fOwnerEntity( )->fObjectToWorld( ) * args.mObjectSpaceEffectorTarget;
			tVec3f target = tVec3f::cZeroVector;
			tVec3f direction = target - world.fGetTranslation( );
			tVec3f curDirection = -world.fYAxis( );
			direction.fNormalizeSafe( curDirection );

			if( curDirection.fDot( direction ) > 0.f )
			{
				world.fOrientYAxis( -direction, tVec3f::cYAxis );
				args.mObjectSpaceEffectorTarget = fOwnerEntity( )->fWorldToObject( ) * world;
				args.mChanged = true;
			}
		}
		else
		{
			// hands
			const tEntityPtr &e = mHandTargets[ args.mChannel ];
			if( e )
			{
				args.mObjectSpaceEffectorTarget.fSetTranslation( (fOwnerEntity( )->fWorldToObject( ) * e->fObjectToWorld( )).fGetTranslation( ) );
				args.mChanged = true;
			}
		}
	}

	void tVehiclePassengerLogic::fReset( const Math::tMat3f& parent )
	{
		mSprungMass.fResetParent( parent );
	}

	void tVehiclePassengerLogic::fDependentPhysicsMT( f32 dt, const Math::tMat3f &parent, tLogic& logic )
	{
		mSprungMass.fSetDamping( Gameplay_Vehicle_SprungMassesP, Gameplay_Vehicle_SprungMassesD );
		mSprungMass.fStep( parent, dt );

		if( Debug_Vehicle_SprungMassesDraw )
		{
			tVec3f p = parent.fXformPoint( mSprungMass.fGetPosition( ) + tVec3f( mSprungMass.fGetHorizontalAcc( ).x, 0, mSprungMass.fGetHorizontalAcc( ).y ) );
			tVec3f cp = parent.fXformPoint(mSprungMass.fGetCenterPosition( ) );

			logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( p,  cp, tVec4f(1,0,0,1) );
			logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tSpheref( p, 0.125f ), tVec4f(1,0,0,1) );
		}
	}

	/*static*/ void tVehiclePassengerLogic::fOutfitCharacterProps( tEntity& root )
	{
		u32 props = root.fQueryEnumValue( GameFlags::cENUM_CHARACTER_PROPS );
		if( props == ~0 ) 
			return;

		u32 country = root.fQueryEnumValue( GameFlags::cENUM_COUNTRY );
		if( country == ~0 )
		{
			log_warning( 0, root.fName( ) << " needs a country!" );
			country = GameFlags::cCOUNTRY_DEFAULT;
		}

		const tStringHashDataTable& dt = tGameApp::fInstance( ).fCharacterPropsTable( country );
		u32 row = dt.fRowIndex( GameFlags::fCHARACTER_PROPSEnumToValueString( props ) );

		tEntity* leftAttach = NULL;
		tEntity* rightAttach = NULL;
		tEntity* helmAttach = NULL;

		for( u32 i = 0; i < root.fChildCount( ); ++i )
		{
			tEntity* child = tReferenceFrameEntity::fSkipOverRefFrameEnts( root.fChild( i ).fGetRawPtr( ) );
			if( child->fName( ) == cCharcterLProp )
				leftAttach = child;
			else if( child->fName( ) == cCharcterRProp )
				rightAttach = child;
			else if( child->fName( ) == cCharcterHelmProp )
				helmAttach = child;			
		}

		if( row != ~0 )
		{
			// These spawns will replace the attachment object with the prop for performance
			tFilePathPtr leftProp( dt.fIndexByRowCol<tStringPtr>( row, tGameApp::cCharacterPropsLeftHandResource ) );
			tFilePathPtr rightProp( dt.fIndexByRowCol<tStringPtr>( row, tGameApp::cCharacterPropsRightHandResource ) );
			tFilePathPtr helmetProp( dt.fIndexByRowCol<tStringPtr>( row, tGameApp::cCharacterPropsHelmetResource ) );

			if( leftProp.fExists( ) )
			{
				if( leftAttach )
				{
					// the parent will be a tReferenceFrameEntity
					tEntity* ent = tUnitLogic::fSpawnReplace( leftAttach, leftProp );
					if( ent ) 
					{
						leftAttach = NULL;
						ent->fAddGameTags( GameFlags::cFLAG_SPAWN_AS_DEBRIS );
					}
				}
				else
					log_warning( 0, "No '" << cCharcterLProp << "' attachment point for character prop." );
			}

			if( rightProp.fExists( ) )
			{
				if( rightAttach ) 
				{
					// the parent will be a tReferenceFrameEntity
					tEntity* ent = tUnitLogic::fSpawnReplace( rightAttach, rightProp );
					if( ent ) 
					{
						rightAttach = NULL;
						ent->fAddGameTags( GameFlags::cFLAG_SPAWN_AS_DEBRIS );
					}
				}
				else
					log_warning( 0, "No '" << cCharcterRProp << "' attachment point for character prop." );
			}

			if( helmetProp.fExists( ) )
			{
				if( helmAttach ) 
				{
					// the parent will be a tReferenceFrameEntity
					tEntity* ent = tUnitLogic::fSpawnReplace( helmAttach, helmetProp );
					if( ent ) 
					{
						helmAttach = NULL;
						ent->fAddGameTags( GameFlags::cFLAG_SPAWN_AS_DEBRIS );
					}
				}
				else
					log_warning( 0, "No '" << cCharcterHelmProp << "' attachment point for character prop." );
			}
		}

		// clean up attachment points, for perf
		if( leftAttach ) leftAttach->fDelete( );
		if( rightAttach ) rightAttach->fDelete( );
		if( helmAttach ) helmAttach->fDelete( );
	}

	namespace
	{


		tSprungMassRef fBuildSprungMassRef( tVehiclePassengerLogic* ptr ) 
		{ 
			return tSprungMassRef( &(ptr->fSprungMass( ).fGetHorizontalAcc( )) ); 
		}
	}

	void tVehiclePassengerLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tVehiclePassengerLogic, tUnitLogic, Sqrat::NoCopy<tVehiclePassengerLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("PostGoalSet"),			&tVehiclePassengerLogic::fPostGoalSet)
			.GlobalFunc( _SC("SprungMass"),		&fBuildSprungMassRef)
			;
		vm.fRootTable( ).Bind(_SC("VehiclePassengerLogic"), classDesc);
	}

}

