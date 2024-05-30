#include "GameAppPch.hpp"
#include "tBarrageImp.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tGameEffects.hpp"
#include "tUserControllableCharacterLogic.hpp"
#include "tProximityLogic.hpp"
#include "tRtsCamera.hpp"
#include "tAirborneLogic.hpp"
#include "tProjectileLogic.hpp"
#include "tTurretLogic.hpp"
#include "tFocusCamera.hpp"
#include "tLightningLogic.hpp"
#include "tAreaDamageLogic.hpp"
#include "tLightningWeapon.hpp"
#include "tSync.hpp"

using namespace Sig::Math;

namespace Sig
{
	// the probability that an artillery shot will go to an "active" area of units.
	//  If this probability does not succeed it will randomly choose one of any of the barrage points.
	devvar( f32, Gameplay_Barrage_ArtilleryTargettingPercentage, 1.0 );

	tArtilleryBarrage::tArtilleryBarrage( )
		: mNumberOfExplosions( 1 )
		, mDelayMin( 0.5f )
		, mDelayMax( 1.f )
		, mSpawnStraightOver( false )
		, mTimer( 0.f )
		, mExplosionsLeft( 1 )
		, mStarted( false )
		, mTargetSet( false )
		, mTarget( tVec3f::cZeroVector )
		, mLastProjLogic( NULL )
		, mWasInUnitLogic( NULL )
		, mShellCam( NULL )
	{ }

	void tArtilleryBarrage::fSetSpawnPtName( const tStringPtr& name )
	{
		mSpawnPt.fReset( tGameApp::fInstance( ).fCurrentLevel( )->fArtilleryBarrageSpawnPt( name ) );
		if( !mSpawnPt )
			log_warning( 0, "No artillery barrage spawn point found named: " << name );
	}

	void tArtilleryBarrage::fEnableProximities( b32 enable, tPlayer* player )
	{
		const tGrowableArray<tEntityPtr>& artPts = tGameApp::fInstance( ).fCurrentLevel( )->fArtilleryBarragePtsForTeam( player->fTeam( ) );
		for( u32 i = 0; i < artPts.fCount( ); ++i )
		{
			tProximityLogic* pl = artPts[ i ]->fLogicDerivedStaticCast<tProximityLogic>( );
			sigassert( pl );

			pl->fSetEnabled( enable );
		}
	}

	void tArtilleryBarrage::fBegin( tPlayer* player )
	{
		tBarrage::fBegin( player );
		fTargetBegin( player );
	}

	void tArtilleryBarrage::fTargetBegin( tPlayer* player )
	{
		mExplosionsLeft = mNumberOfExplosions;
		mStarted = true;
		mTimer = 0.f;

		fEnableProximities( true, player );

		fSpawnExplosion( player );

		fAudioState( cRolling, player );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level ) level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_BARRAGE_USED ) );
	}

	void tArtilleryBarrage::fReset( tPlayer* player )
	{
		fEnableProximities( false, player );
		mStarted = false;
		mWasInUnitLogic = NULL;
		mWasInUnit.fRelease( );
		mShellCam = false;
		mShellCamPtr.fRelease( );
		mTargetSet = false;
		fPushPopCamera( player, false );
		tBarrage::fReset( player );
	}

	//void tArtilleryBarrage::fPushPopCamera( tPlayer* player, b32 push )
	//{
	//	if( push && !mShellCam )
	//	{
	//		tGrowableArray<tProjectileLogic*> logs;
	//		logs.fPushBack( mLastProjLogic );
	//		mShellCam = NEW tShellCamera( *player, logs, NULL );
	//		mShellCamPtr = Gfx::tCameraControllerPtr( mShellCam );
	//		player->fPushCamera( mShellCamPtr );
	//		if( player->fCurrentUnit( ) )
	//			player->fCurrentUnit( )->fSetDisableControl( true );
	//	}
	//	else
	//	{
	//		player->fCameraStack( ).fPopCamerasOfType<tShellCamera>( );
	//		mShellCamPtr.fRelease( );
	//		mShellCam = NULL;
	//		if( player->fCurrentUnit( ) )
	//			player->fCurrentUnit( )->fSetDisableControl( false );
	//	}
	//}

	void tArtilleryBarrage::fPushPopCamera( tPlayer* player, b32 push )
	{
		if( push && !mShellCam )
		{
			mWasInUnitLogic = player->fCurrentUnit( );
			if( mWasInUnitLogic )
			{
				mWasInUnit.fReset( mWasInUnitLogic->fOwnerEntity( ) );
				player->fUnlockFromUnit( true );
				tTurretLogic* turret = mWasInUnitLogic->fDynamicCast<tTurretLogic>( );
				if( turret )
					turret->fSetDontAlign( true );
			}
			else
				mWasInUnit.fRelease( );

			tGrowableArray<tProjectileLogic*> logs;
			logs.fPushBack( mLastProjLogic );
			mShellCam = NEW tShellCamera( *player, logs, NULL );
			mShellCamPtr = Gfx::tCameraControllerPtr( mShellCam );
			player->fPushCamera( mShellCamPtr );

			fAudioEvent( mAudioEventUse, player );
		}
		else if( mShellCam )
		{
			for( u32 i = 0; i < player->fCameraStack( ).fCount( ); ++i )
			{
				if( &player->fCameraStack( )[ i ] == mShellCam )
				{
					player->fCameraStack( ).fEraseOrdered( i );
					if( mWasInUnitLogic && !mWasInUnitLogic->fIsDestroyed( ) )
					{
						player->fLockInUnitDirect( mWasInUnitLogic );
						player->fUnlockFromUnit( false );
					}

					mWasInUnitLogic = NULL;
					mWasInUnit.fRelease( );
					break;
				}
			}

			mShellCam = false;
			mShellCamPtr.fRelease( );
		}
	}

	b32 tArtilleryBarrage::fBarrageUsable( ) const
	{
		return (mLastProjLogic && !mShellCam);
	}

	f32 tArtilleryBarrage::fProcessST( tPlayer* player, f32 dt )
	{
		mTimer -= dt;
		if( mExplosionsLeft > 0 && mTimer <= 0.f )
			fSpawnExplosion( player );

		sigassert( mNumberOfExplosions != 0 );
		mProgress = 1.f - mExplosionsLeft/(f32)mNumberOfExplosions;
		if( mLastProjLogic )
			mProgress = fMin( 0.95f, mProgress );

		if( mLastProjLogic )
		{
			if( mLastProjLogic->fDeleteMe( ) )
			{
				mProgress = 1.1f;
				mLastProjectile.fRelease( );
				mLastProjLogic = NULL;
				fPushPopCamera( player, false );
			}
			else if( !mShellCam )
			{
				if( player->fGamepad( ).fButtonDown( Input::tGamepad::cButtonY ) )
					fPushPopCamera( player, true );
			}
			else
			{
				if( !mShellCam->fPushed( ) )
					fPushPopCamera( player, false );
			}
		}
		
		return mProgress;
	}

	void tArtilleryBarrage::fSpawnExplosion( tPlayer* player/*, tEntity* point*/ )
	{
		sigassert( mSpawnPt );

		if( mTargetSet )
		{
			Math::tMat3f spawnMat = mSpawnPt->fObjectToWorld( );
			if( mSpawnStraightOver )
				spawnMat.fSetTranslation( mTarget + ( Math::tVec3f::cYAxis * 500.f ) + ( Math::tVec3f::cXAxis * 10.f ) );

			mLastProjLogic = tWeapon::fSingleShot( spawnMat, mTarget, mWeaponID, player->fTeam( ), NULL, player );
			if( mLastProjLogic )
			{
				mLastProjectile.fReset( mLastProjLogic->fOwnerEntity( ) );
			}
		}
		else
		{
			const tGrowableArray<tEntityPtr>* artPts = &tGameApp::fInstance( ).fCurrentLevel( )->fArtilleryBarragePtsForTeam( player->fTeam( ) );
			tGrowableArray<tEntityPtr> tempPoints;
			
			f32 useTargettingRoll = sync_rand( fFloatZeroToOne( ) );
			if( useTargettingRoll < Gameplay_Barrage_ArtilleryTargettingPercentage )
			{
				// Remove points with no one around
				tempPoints = *artPts;
				
				for( u32 i = 0; i < tempPoints.fCount( ); ++i )
				{
					tProximityLogic* prox = tempPoints[ i ]->fLogicDerived<tProximityLogic>( );
					if( prox && prox->fEntityList( ).fCount( ) == 0 )
					{
						tempPoints.fErase( i );
						--i;
					}
				}

				// if we still have targetting points use it
				if( tempPoints.fCount( ) > 0 ) artPts = &tempPoints;
			}

			if( artPts->fCount( ) )
			{
				tEntity* e = (*artPts)[ sync_rand( fIntInRange( 0, artPts->fCount( )-1 ) ) ].fGetRawPtr( );

				if( mSpawnPt )
				{
					tVec3f target = sync_rand( fVec<Math::tVec3f>( ) );
					target.y = 0;
					target = e->fObjectToWorld( ).fXformPoint( target );
					tWeapon::fSingleShot( mSpawnPt->fObjectToWorld( ), target, mWeaponID, player->fTeam( ), NULL, player );
				}
				
				//old direct effect way
				//Math::tMat3f xform = Math::tMat3f::cIdentity;			
				//xform.fSetTranslation( e->fObjectToWorld( ).fGetTranslation( ) );			
				//tGameEffects::fInstance( ).fPlayEffect( tGameApp::fInstance( ).fCurrentLevel( )->fRootEntity( ), mEffectID, &xform );
			}
		}

		--mExplosionsLeft;
		mTimer = sync_rand( fFloatInRange( mDelayMin, mDelayMax ) );
	}


	tRamboBarrage::tRamboBarrage( )
		: mTimer( 0.f )
		, mCharacter( NULL )
		, mPlayer( NULL )
		, mCanUseLastFrame( false )
		, mTimerStarted( false )
		, mEverCouldUse( false )
		, mForceInto( false )
		, mLockIn( false )
		, mAchievementAwarded( false )
	{
	}

	void tRamboBarrage::fSetTarget( tPlayer* player, tEntity* target ) 
	{ 
		mSkipToTarget.fReset( target ); 
	}

	void tRamboBarrage::fSkipInto( tPlayer* player ) 
	{ 
		tBarrage::fSkipInto( player );
	}

	void tRamboBarrage::fForceUse( tPlayer* player )
	{
		mForceInto = true;
		mLockIn = true;
	}

	void tRamboBarrage::fBegin( tPlayer* player )
	{
		tBarrage::fBegin( player );

		mTimer = 0.f;
		mCharacter = NULL;
		mPlayer = player;


		Gfx::tTripod camera = player->fCameraStackTop( )->fViewport( )->fLogicCamera( ).fGetTripod( );

		tVec3f dropPoint = camera.mLookAt;
		tVec3f faceDir = camera.mLookAt - camera.mEye;

		if( mSkipToTarget )
		{
			dropPoint = mSkipToTarget->fObjectToWorld( ).fGetTranslation( );
			faceDir = mSkipToTarget->fObjectToWorld( ).fZAxis( );
		}
		else
		{
			tEntity* dropEntity = tGameApp::fInstance( ).fCurrentLevel( )->fBarrageDropPt( player->fTeam( ), camera.mLookAt, mDropPtName );

			if( dropEntity )
				dropPoint = dropEntity->fObjectToWorld( ).fGetTranslation( );
			else
				log_warning( 0, "No drop point found for barrage: " << mDropPtName );
		}

		dropPoint.y += 5.0f;
		faceDir.y = 0;

		tEntity* character = tGameApp::fInstance( ).fSceneGraph( )->fRootEntity( ).fSpawnChild( mCharacterPath );
		if( character )
		{
			mCharacterEnt.fReset( character );

			tMat3f xform;
			xform.fSetTranslation( dropPoint );
			xform.fOrientYWithZAxis( tVec3f::cYAxis, faceDir );
			character->fMoveTo( xform );

			mCharacter = character->fLogicDerived<tUserControllableCharacterLogic>( );
			sigassert( mCharacter );
			mCharacter->fSetCreationType( tUnitLogic::cCreationTypeFromGenerator );
			mCharacter->fSetForBarrage( player );
			mCharacter->fEnableSelection( false );
			mCharacter->fSetDontInstaDestroy( mForTutorial );

			mForceInto = true;
		}
		else
			log_warning( 0, "Could not spawn rambo barrage! " << mCharacterPath );
	}

	void tRamboBarrage::fReset( tPlayer* player )
	{
		mCharacterEnt.fRelease( );
		mCharacter = NULL;
		mPlayer = NULL;
		mSkipToTarget.fRelease( );
		mCanUseLastFrame = false;
		mTimerStarted = false;
		mEverCouldUse = false;
		mForceInto = false;
		mLockIn = false;
		mAchievementAwarded = false;
		tBarrage::fReset( player );
	}

	b32 tRamboBarrage::fBarrageUsable( ) const
	{
		return (mCharacter && mPlayer && mPlayer->fCurrentUnit( ) != mCharacter && mCharacter->fCanBeUsed( ));
	}

	f32 tRamboBarrage::fProcessST( tPlayer* player, f32 dt )
	{
		b32 canUse = mCharacter->fCanBeUsed( );

		if( mTimerStarted )
			mTimer += dt;

		if( canUse )
		{
			mCharacter->fEnableSelection( true );

			if( !mEverCouldUse )
				player->fSoundSource( )->fHandleEvent( mAudioEventReady );
			mEverCouldUse = true;

			fFirstUse( player );
		}

		sigassert( mDuration > 0 );
		mProgress = mTimer/mDuration;

		if( mCharacter && mCharacter->fUnderUserControl( ) && mCharacter->fTeamPlayer( ) )
		{
			sigassert( mCharacter->fTeamPlayer( ) == player );

			//incremented in commando logic now
			//// using
			//if( mUsingStatToIncrement != ~0 )
			//	player->fStats( ).fIncQuick( mUsingStatToIncrement, dt );

			if( mCanUseLastFrame ) // just entered
			{
				fFirstUse( player );

				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				if( level ) 
					level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_BARRAGE_USED ) );			
			}
		}
		
		mCanUseLastFrame = canUse;

		if( mCharacter && player->fCurrentUnit( ) != mCharacter && ((canUse && mForceInto) || (!mForceInto && player->fGamepad( ).fButtonDown( Input::tGamepad::cButtonY ))) )
		{
			player->fLockInUnitDirect( mCharacter );
			if( !mLockIn && !mForTutorial )
				player->fUnlockFromUnit( false );

			mLockIn = false;
			mForceInto = false;
		}

		if( mProgress >= 1.f && mCharacter )
		{
			mCharacter->fOwnerEntity( )->fRemoveGameTags( GameFlags::cFLAG_SELECTABLE );
			mCharacter->fEnableSelection( false );
			mCharacter->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_BARRAGE_ENDED ) );
		}

		if( !mAchievementAwarded && mCharacter && mProgress > 0.5f )
		{
			mAchievementAwarded = true;
			player->fAwardAchievement( GameFlags::cACHIEVEMENTS_DEMOLITION_MAN );
		}


		return mProgress;
	}

	void tRamboBarrage::fFirstUse( tPlayer* player )
	{
		if( !mTimerStarted )
		{
			mTimerStarted = true;
			player->fSoundSource( )->fHandleEvent( mAudioEventFirstUse );
			fAudioState( cRolling, player );
		}
	}

	b32 tRamboBarrage::fEnteredBarrageUnit( tUnitLogic* logic, b32 enter )
	{
		sigassert( logic );
		if( logic == mCharacter )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );

			if( level )
				level->fSuspendTutorial( enter );

			return true;
		}
		
		return false;
	}



	tWaveLaunchBarrage::tWaveLaunchBarrage( )
		: mDuration( 10.f )
		, mSelectableUnits( false )
		, mTimer( 0.f )
	{

	}

	void tWaveLaunchBarrage::fBegin( tPlayer* player )
	{
		sigassert( mDuration > 0.f );
		mWaveList.fReset( tGameApp::fInstance( ).fCurrentLevel( )->fWaveManager( )->fAddCommonWaveList( mWaveListName ) );

		if( mWaveList )
		{
			mWaveList->fSetSaveable( false );
			if( mSelectableUnits )
				mWaveList->fSetMakeSelectableUnits( true );
			mWaveList->fActivate( );
			mTimer = 0.f;
			fAudioState( cRolling, player );
			player->fSoundSource( )->fHandleEvent( mAudioEventLaunched );
		}
		else
			log_warning( 0, "Could not find barrage wave in common table: " << mWaveListName );
	}

	f32 tWaveLaunchBarrage::fProcessST( tPlayer* player, f32 dt )
	{
		mTimer += dt;
		sigassert( mDuration > 0 );
		mProgress = mTimer/mDuration;

		if( mTimer > mDuration )
		{
			if( mWaveList )
			{
				tGameApp::fInstance( ).fCurrentLevel( )->fWaveManager( )->fRemoveWaveList( mWaveList.fGetRawPtr( ) );
				mWaveList.fRelease( );
			}

			mProgress = 1.f;
		}

		return mProgress;
	}

	void tWaveLaunchBarrage::fSetTargetPt( const Math::tMat3f& xform ) 
	{ 
		if( mPathSigml )
			mPathSigml->fMoveTo( xform );
	}

	void tWaveLaunchBarrage::fSetPathSigml( tEntity* ent ) 
	{
		if( !ent )
		{
			log_warning( 0, "No path sigml set for Targetted Barrage" );
		}

		mPathSigml.fReset( ent ); 
	}



	tUsableWaveLaunchBarrage::tUsableWaveLaunchBarrage( )
		: mPlayer( NULL )
		, mCurrentUsingLogic( false )
		, mUsable( false )
		, mFoundUsable( false )
		, mForceInto( false )
		, mLockIn( false )
	{
	}

	void tUsableWaveLaunchBarrage::fForceUse( tPlayer* player )
	{
		mForceInto = true;
		mLockIn = true;
	}

	void tUsableWaveLaunchBarrage::fBegin( tPlayer* player )
	{
		tBarrage::fBegin( player );
		fTargetBegin( player );
	}

	void tUsableWaveLaunchBarrage::fTargetBegin( tPlayer* player )
	{
		mPlayer = player;
		mUnits.fSetCount( mNames.fCount( ) );
		mLogics.fSetCount( mNames.fCount( ) );
		mLogics.fFill( NULL );

		tWaveLaunchBarrage::fBegin( player );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );

		//overwrite anyones name who's in our use list. SORRY!
		mFoundUsable = false;
		for( u32 i = 0; i < mNames.fCount( ); ++i )
		{
			for( u32 u = 0; u < level->fUseableUnitCount( ); ++u )
			{
				if( level->fUseableUnit( u )->fName( ) == mNames[ i ] )
					level->fUseableUnit( u )->fSetName( tStringPtr::cNullPtr );
			}
		}

		// always force into
		mForceInto = true;
	}

	void tUsableWaveLaunchBarrage::fReset( tPlayer* player )
	{
		for( u32 i = 0; i < mLogics.fCount( ); ++i )
		{
			if( !mLogics[ i ] || !mLogics[ i ]->fOwnerEntity( ) )
				continue;

			//if we're riding on a vehicle delete that too
			tEntity* parent = mLogics[ i ]->fOwnerEntity( )->fParent( );
			tUnitLogic* parentVeh = NULL;
			if( parent ) parentVeh = parent->fFirstAncestorWithLogicOfType<tUnitLogic>( );
			if( parentVeh ) parentVeh->fOwnerEntity( )->fDelete( );

			mLogics[ i ]->fOwnerEntity( )->fDelete( );
		}

		mUnits.fSetCount( 0 );
		mLogics.fSetCount( 0 );

		mPlayer = NULL;
		mCurrentUsingLogic = NULL;
		mLastUsedName = tStringPtr::cNullPtr;
		mUsable = false;
		mFoundUsable = false;
		mForceInto = false;
		mLockIn = false;

		tBarrage::fReset( player );
	}

	b32 tUsableWaveLaunchBarrage::fBarrageUsable( ) const
	{
		mUsable = mFoundUsable && !fPlayerInNamedUnit( );
		return mUsable;
	}

	b32 tUsableWaveLaunchBarrage::fPlayerInNamedUnit( ) const
	{
		sigassert( mPlayer );

		b32 inNamedUnit (mPlayer->fCurrentUnit( ) && mNames.fIndexOf( mPlayer->fCurrentUnit( )->fOwnerEntity( )->fName( ) ) != ~0);

		if( inNamedUnit )
		{
			mCurrentUsingLogic = mPlayer->fCurrentUnit( );
			mLastUsedName = mCurrentUsingLogic->fOwnerEntity( )->fName( );
		}
		else
			mCurrentUsingLogic = NULL;

		return inNamedUnit;
	}

	f32 tUsableWaveLaunchBarrage::fProcessST( tPlayer* player, f32 dt )
	{
		fLookForLogics( );

		if( mUsable && (mForceInto || mPlayer->fGamepad( ).fButtonDown( Input::tGamepad::cButtonY )) )
		{
			mUsable = false;
			mForceInto = false;

			tStringPtr name;
			if( mLastUsedName.fExists( ) )
			{
				name = mLastUsedName;
			}
			else
			{
				for( u32 i = 0; i < mNames.fCount( ); ++i )
				{
					if( mLogics[ i ] )
					{
						name = mNames[ i ];
						break;
					}
				}
			}

			if( name.fExists( ) )
			{
				player->fLockInUnit( name );
				if( !mLockIn )
					player->fUnlockFromUnit( false );
				tRtsCamera* camera = player->fCameraStack( ).fFindCameraOfType<tRtsCamera>( );
				sigassert( camera );
				camera->fSetPreventPositionAcquisition( true );
			}

			mLockIn = false;
		}

		if( mCurrentUsingLogic && mUsingStatToIncrement != ~0 )
			player->fStats( ).fIncQuick( mUsingStatToIncrement, dt );

		tWaveLaunchBarrage::fProcessST( player, dt );

		if( mProgress >= 1.f )
		{
			if( fPlayerInNamedUnit( ) )
				player->fUnlockFromUnit( true );
		}

		return mProgress;
	}

	void tUsableWaveLaunchBarrage::fLookForLogics( )
	{
		// haveing a player ptr means fBegin was called
		if( mPlayer && !mFoundUsable && mNames.fCount( ) )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			mFoundUsable = true;

			for( u32 i = 0; i < mNames.fCount( ); ++i )
			{
				if( !mUnits[ i ] )
				{
					mUnits[ i ].fReset( level->fUseableUnit( mNames[ i ] ) );
					if( mUnits[ i ] )
						mLogics[ i ] = mUnits[ i ]->fLogicDerived<tUnitLogic>( );
				}

				if( !mUnits[ i ] )
					mFoundUsable = false; // must find all usable.
			}
		}
	}
	
	b32 tUsableWaveLaunchBarrage::fEnteredBarrageUnit( tUnitLogic* logic, b32 enter, tPlayer* player )
	{
		sigassert( logic );
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );

		u32 index = mLogics.fIndexOf( logic );
		if( index != ~0 )
		{
			if( enter )
			{
				player->fSoundSource( )->fHandleEvent( mAudioEventUse );
				logic->fCancelAllWeaponFire( );
			}

			if( level )
				level->fSuspendTutorial( enter );

			// disable other units
			for( u32 i = 0; i < mLogics.fCount( ); ++i )
				mLogics[ i ]->fDisableAIWeaponFire( (enter && i != index) );

			tAirborneLogic* al = logic->fDynamicCast<tAirborneLogic>( );
			if( al && enter )
			{
				al->fLockInBombCam( true );
				al->fDontDitchOnExit( true );
			}

			return true;
		}

		return false;
	}

	b32 tUsableWaveLaunchBarrage::fUsedBarrageUnit( tUnitLogic* logic, tEntity* ent, tPlayer* player )
	{
		u32 index = mLogics.fIndexOf( logic );
		if( index != ~0 )
		{
			player->fSoundSource( )->fHandleEvent( mAudioEventFire );
		}

		tFocusCamera* cam = player->fCameraStackTop( )->fDynamicCast<tFocusCamera>( );
		if( cam && !cam->fHasChanged( ) )
		{
			if( ++cam->fWaitCount( ) >= 3 )
			{
				cam->fChangeTarget( ent );
			}
		}

		return false;
	}


	template< typename base >
	class tTargetedBarrage : public base
	{
	public:
		tTargetedBarrage( )
			: mTargetingDuration( 30.f )
			, mTargetSet( false )
			, mCameraPushed( false )
			, mCamera( NULL )
			, mPlayer( NULL )
			, mTimer( 0.f )
			, mWasInUnitLogic( NULL )
		{
		}

		virtual void fBegin( tPlayer* player )
		{
			mPlayer = player;
			fPushPopCamera( true );
			mTimer = 0;
			tBarrage::fBegin( player ); //base waits for fTargetBegin
		}

		virtual void fReset( tPlayer* player )
		{
			fPushPopCamera( false );
			mTargetSet = false;
			mWasInUnitLogic = NULL;
			mWasInUnit.fRelease( );
			base::fReset( player );
		}

		virtual b32 fBarrageUsable( ) const
		{
			if( !mTargetSet )
			{
				return !mCameraPushed;
			}
			else
				return base::fBarrageUsable( );
		}

		virtual f32 fProcessST( tPlayer* player, f32 dt )
		{
			mTimer += dt;
			sigassert( mTargetingDuration > 0 );
			mProgress = mTimer/mTargetingDuration;

			if( mProgress >= 1.f )
			{
				//too late!
				fPushPopCamera( false );
			}
			else
			{
				if( !mTargetSet )
				{
					if( player->fGamepad( ).fButtonDown( Input::tGamepad::cButtonB ) )
						fPushPopCamera( false );
					else if( player->fGamepad( ).fButtonDown( Input::tGamepad::cButtonY ) )
						fPushPopCamera( true );
				}

				if( mCameraPushed && mCamera->fHasBlendedIn( ) && player->fGamepad( ).fButtonDown( Input::tGamepad::cButtonA ) )
				{
					sigassert( mCamera );

					player->fSoundSource( )->fHandleEvent( mAudioEventTargetSet );

					tMat3f xform;
					xform.fSetTranslation( mCamera->fCursorPosition( ) );
					xform.fOrientZAxis( mCamera->fCursorDir( ) );
					fSetTargetPt( xform );

					mTargetSet = true;
					mTimer = 0.f;

					base::fTargetBegin( player );
					fPushPopCamera( false );
				}
			}

			if( mTargetSet )
				return base::fProcessST( player, dt );
			else
				return mProgress;
		}

		f32 mTargetingDuration; //time to choose target
		tStringPtr mAudioEventTargetSet;

	protected:
		b8 mTargetSet;
		b8 mCameraPushed;
		b8 pad1;
		b8 pad2;
		
		f32 mTimer;

		tPlayer*   mPlayer;
		tRtsCamera* mCamera;
		Gfx::tCameraControllerPtr mCameraPtr;

		tEntityPtr mWasInUnit;
		tUnitLogic* mWasInUnitLogic;

		virtual void fPushPopCamera( b32 push )
		{
			sigassert( mPlayer );
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );

			if( push && !mCameraPushed && !mTargetSet )
			{
				mCameraPushed = true;
				if( level )
					level->fSuspendTutorial( true );

				mWasInUnitLogic = mPlayer->fCurrentUnit( );
				if( mWasInUnitLogic )
				{
					mWasInUnit.fReset( mWasInUnitLogic->fOwnerEntity( ) );
					mPlayer->fUnlockFromUnit( true );
					tTurretLogic* turret = mWasInUnitLogic->fDynamicCast<tTurretLogic>( );
					if( turret )
						turret->fSetDontAlign( true );
				}
				else
					mWasInUnit.fRelease( );

				//push rts cam with bomb drop mode
				mCamera = NEW tRtsCamera( *mPlayer, true );
				mCameraPtr.fReset( mCamera );
				mPlayer->fPushCamera( mCameraPtr );
			}
			else if( mCameraPushed )
			{
				mCameraPushed = false;
				if( level )
					level->fSuspendTutorial( false );

				for( u32 i = 0; i < mPlayer->fCameraStack( ).fCount( ); ++i )
				{
					if( &mPlayer->fCameraStack( )[ i ] == mCamera )
					{
						mPlayer->fCameraStack( ).fEraseOrdered( i );
						if( mWasInUnitLogic && !mWasInUnitLogic->fIsDestroyed( ) )
						{
							mPlayer->fLockInUnitDirect( mWasInUnitLogic );
							mPlayer->fUnlockFromUnit( false );
						}

						mWasInUnitLogic = NULL;
						mWasInUnit.fRelease( );
						break;
					}
				}
			}
		}
	};

	typedef tTargetedBarrage<tUsableWaveLaunchBarrage> tTargetedWaveLaunchBarrage;
	typedef tTargetedBarrage<tArtilleryBarrage>  tTargetedArtilleryBarrage;

	namespace
	{
		static const tStringPtr cLaserTracer( "LASER_BARRAGE" );
		static const tFilePathPtr cLaserBurn( "gameplay/barrage/laser_burn.sigml" );
		static const tStringPtr cLaserWeapon( "ARTILLERY_BARRAGE_LAZER" );
	}

	class tTargetedLaserBarrage : public tTargetedArtilleryBarrage
	{
	public:
		tTargetedLaserBarrage( )
			: tTargetedArtilleryBarrage( )
			, mFiring( false )
			, mDamageLogic( NULL )
		{
			u32 tracer = tGameApp::fInstance( ).fTracersTable( ).fTable( ).fRowIndex( cLaserTracer );
			log_assert( tracer != ~0, "Tracer not found: " << cLaserTracer);

			mLightning.fReset( new tLightningEntity( tracer ) );
			mLightning->fSpawn( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
			mLightning->fSetFracs( tOrbitalLaserWeapon::fNumFracs( ) );

			mBurn.fReset( tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( )->fSpawnChild( cLaserBurn ) );
			mBurn->fForEachDescendent( FX::tAddPausedFxSystem( mFx ) );
			mFx.fPause( false );
			mFx.fSetEmissionPercent( 0.f );

			mBurnSound.fReset( new Audio::tSource( "Laser Burn" ) );
			mBurnSound->fSpawn( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
		}


		~tTargetedLaserBarrage( )
		{
			mFx.fSafeCleanup( );

			if( mFiring && mBurnSound )
				mBurnSound->fHandleEvent( tOrbitalLaserWeapon::cLaserBurnStopSound );
			mBurnSound.fRelease( );
		}

		void fSetPlayer( tPlayer* player )
		{
			// do some damage
			mDamageLogic = NEW tAreaDamageLogic( );
			tLogicPtr *dlp = NEW tLogicPtr( mDamageLogic );
			mBurn->fAcquireLogic( dlp );

			tDamageID id( NULL, player, player->fTeam( ) );
			id.mDesc = &tWeaponDescs::fInstance( ).fDesc( cLaserWeapon );

			mDamageLogic->fSetDamageID( id );
			mDamageLogic->fEnable( false );
		}

		virtual f32 fProcessST( tPlayer* player, f32 dt )
		{
			mTimer += dt;
			sigassert( mTargetingDuration > 0 );
			mProgress = mTimer/mTargetingDuration;

			if( mProgress >= 1.f )
			{
				//too late!
				fPushPopCamera( false );
			}
			else
			{
				//if( !mTargetSet )
				{
					if( player->fGamepad( ).fButtonDown( Input::tGamepad::cButtonB ) )
						fPushPopCamera( false );
					else if( player->fGamepad( ).fButtonDown( Input::tGamepad::cButtonY ) )
						fPushPopCamera( true );
				}

				if( mCameraPushed && mCamera->fHasBlendedIn( ) )
				{
					if( player->fGamepad( ).fButtonDown( Input::tGamepad::cButtonA ) )
						fFire( true );
					else if( player->fGamepad( ).fButtonUp( Input::tGamepad::cButtonA ) )
						fFire( false );
				}
			}

			if( mFiring )
			{
				mLightning->fSetAlpha( 1.0f );
				mLightning->fSetTarget( mCamera->fCursorPosition( ) );

				tMat3f xform = mCamera->fViewport( )->fRenderCamera( ).fLocalToWorld( );
				xform.fTranslateLocal( tVec3f( -5, 5, 0 ) );
				mLightning->fMoveTo( xform );

				mBurnSound->fMoveTo( mCamera->fCursorPosition( ) );
				mBurn->fMoveTo( mCamera->fCursorPosition( ) );
				mFx.fSetEmissionPercent( 1.0f );

			}
			else
			{
				mLightning->fSetAlpha( 0.f );
				mFx.fSetEmissionPercent( 0.f );
			}

			return mProgress;
		}

		void fFire( b32 fire )
		{
			mFiring = fire;
			mDamageLogic->fEnable( fire );
			
			sigassert( mPlayer );
			sigassert( mPlayer->fSoundSource( ) );
			mPlayer->fSoundSource( )->fHandleEvent( fire ? tOrbitalLaserWeapon::cLaserStartSound : tOrbitalLaserWeapon::cLaserStopSound );
			mBurnSound->fHandleEvent( fire ? tOrbitalLaserWeapon::cLaserBurnStartSound : tOrbitalLaserWeapon::cLaserBurnStopSound );
		}

		virtual void fPushPopCamera( b32 push )
		{
			if( !push )
				fFire( false );

			tTargetedArtilleryBarrage::fPushPopCamera( push );
		}

	private:
		b32 mFiring;
		tRefCounterPtr<tLightningEntity> mLightning;

		// ground burning
		tEntityPtr				mBurn;
		FX::tFxSystemsArray		mFx;
		tAreaDamageLogic*		mDamageLogic;
		Audio::tSourcePtr		mBurnSound;
	};

}


namespace Sig
{
	namespace tBarrageImp
	{
		void fExportScriptInterface( tScriptVm& vm )
		{
			{
				Sqrat::DerivedClass<tArtilleryBarrage, tBarrage, Sqrat::DefaultAllocator<tArtilleryBarrage>> classDesc( vm.fSq( ) );

				classDesc
					.Var( _SC("EffectID"),				&tArtilleryBarrage::mEffectID )
					.Var( _SC("WeaponID"),				&tArtilleryBarrage::mWeaponID )
					.Var( _SC("NumberOfExplosions"),	&tArtilleryBarrage::mNumberOfExplosions )
					.Var( _SC("DelayMin"),				&tArtilleryBarrage::mDelayMin )
					.Var( _SC("DelayMax"),				&tArtilleryBarrage::mDelayMax )
					.Var( _SC("SpawnStraightOver"),		&tArtilleryBarrage::mSpawnStraightOver )
					.Var( _SC("AudioEventUse"),			&tArtilleryBarrage::mAudioEventUse )	
					.Func(_SC("SetSpawnPtName"),		&tArtilleryBarrage::fSetSpawnPtName)
					;

				vm.fRootTable( ).Bind(_SC("ArtilleryBarrage"), classDesc );
			}
			{
				Sqrat::DerivedClass<tRamboBarrage, tBarrage, Sqrat::DefaultAllocator<tRamboBarrage>> classDesc( vm.fSq( ) );

				classDesc
					.Var( _SC("CharacterPath"), &tRamboBarrage::mCharacterPath )
					.Var( _SC("Duration"),		&tRamboBarrage::mDuration )
					.Var( _SC("DropPtName"),	&tRamboBarrage::mDropPtName )	
					.Var( _SC("AudioEventReady"), &tRamboBarrage::mAudioEventReady )
					.Var( _SC("AudioEventFirstUse"), &tRamboBarrage::mAudioEventFirstUse )				
					
					;

				vm.fRootTable( ).Bind(_SC("RamboBarrage"), classDesc );
			}
			{
				Sqrat::DerivedClass<tWaveLaunchBarrage, tBarrage, Sqrat::DefaultAllocator<tWaveLaunchBarrage>> classDesc( vm.fSq( ) );

				classDesc
					.Var( _SC("WaveListName"),		&tWaveLaunchBarrage::mWaveListName )
					.Var( _SC("Duration"),			&tWaveLaunchBarrage::mDuration )	
					.Var( _SC("SelectableUnits"),	&tWaveLaunchBarrage::mSelectableUnits )	
					.Var( _SC("AudioEventLaunched"), &tWaveLaunchBarrage::mAudioEventLaunched )
					.Func(_SC("SetPathSigml"),		&tWaveLaunchBarrage::fSetPathSigml)	
					;

				vm.fRootTable( ).Bind(_SC("WaveLaunchBarrage"), classDesc );
			}
			{
				Sqrat::DerivedClass<tUsableWaveLaunchBarrage, tWaveLaunchBarrage, Sqrat::DefaultAllocator<tUsableWaveLaunchBarrage>> classDesc( vm.fSq( ) );

				classDesc
					.Func( _SC("AddUsableName"),	&tUsableWaveLaunchBarrage::fAddUsableName )
					.Var( _SC("AudioEventUse"),		&tUsableWaveLaunchBarrage::mAudioEventUse )	
					.Var( _SC("AudioEventFire"),	&tUsableWaveLaunchBarrage::mAudioEventFire )					
					;

				vm.fRootTable( ).Bind(_SC("UsableWaveLaunchBarrage"), classDesc );
			}
			{
				Sqrat::DerivedClass<tTargetedWaveLaunchBarrage, tUsableWaveLaunchBarrage, Sqrat::DefaultAllocator<tTargetedWaveLaunchBarrage>> classDesc( vm.fSq( ) );

				classDesc		
					.Var( _SC("TargetingDuration"),		&tTargetedWaveLaunchBarrage::mTargetingDuration)
					.Var( _SC("AudioEventTargetSet"),	&tTargetedWaveLaunchBarrage::mAudioEventTargetSet )
					;

				vm.fRootTable( ).Bind(_SC("TargetedWaveLaunchBarrage"), classDesc );
			}
			{
				Sqrat::DerivedClass<tTargetedArtilleryBarrage, tArtilleryBarrage, Sqrat::DefaultAllocator<tTargetedArtilleryBarrage>> classDesc( vm.fSq( ) );

				classDesc		
					.Var( _SC("TargetingDuration"),		&tTargetedArtilleryBarrage::mTargetingDuration)
					.Var( _SC("AudioEventTargetSet"),	&tTargetedArtilleryBarrage::mAudioEventTargetSet )
					;

				vm.fRootTable( ).Bind(_SC("TargetedArtilleryBarrage"), classDesc );
			}
			{
				Sqrat::DerivedClass<tTargetedLaserBarrage, tTargetedArtilleryBarrage, Sqrat::DefaultAllocator<tTargetedLaserBarrage>> classDesc( vm.fSq( ) );

				classDesc	
					.Func( _SC("SetPlayer"), &tTargetedLaserBarrage::fSetPlayer )
					;

				vm.fRootTable( ).Bind(_SC("TargetedLaserBarrage"), classDesc );
			}
		}
}	}

