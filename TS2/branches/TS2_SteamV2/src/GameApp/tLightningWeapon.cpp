#include "GameAppPch.hpp"
#include "tLightningWeapon.hpp"
#include "tLightningLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "tAreaDamageLogic.hpp"
#include "tLevelLogic.hpp"

#include "Wwise_IDs.h"

using namespace Sig::Math;

namespace Sig
{

	namespace
	{
		const tFilePathPtr cImpactEffect( "Effects/Entities/Boss/electroplosion.sigml" );
	}


	const u32 tOrbitalLaserWeapon::cLaserStartSound = AK::EVENTS::PLAY_WEAPON_BARRAGE_LASER_FIRE;
	const u32 tOrbitalLaserWeapon::cLaserStopSound = AK::EVENTS::STOP_WEAPON_BARRAGE_LASER_FIRE;

	const u32 tOrbitalLaserWeapon::cLaserBurnStartSound = AK::EVENTS::PLAY_WEAPON_BARRAGE_LASER_IMPACT;
	const u32 tOrbitalLaserWeapon::cLaserBurnStopSound = AK::EVENTS::STOP_WEAPON_BARRAGE_LASER_IMPACT;


	devvar( u32, Gameplay_Weapon_Orbital_Fracs, 0 );
	devvar( f32, Gameplay_Weapon_Orbital_ApexHeight, 300 );
	devvar( u32, Gameplay_Weapon_Lightning_Fracs, 3 );

	tLightningWeapon::tLightningWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst )
		: tGunWeapon( desc, inst )
	{
	}

	void tLightningWeapon::fOnSpawn( )
	{
		tGunWeapon::fOnSpawn( );

		mLightning.fReset( NEW tLightningEntity( mDesc.mBulletTracerType ) );


		sigassert( mInst.mMuzzles.fCount( ) );

		tEntity* parent = mInst.mMuzzles.fFront( ).mProjectileSpawn.fGetRawPtr( );
		mLightning->fSpawn( *parent );
		mLightning->fSetFracs( Gameplay_Weapon_Lightning_Fracs );

		//mExplosion.fReset( parent->fSceneGraph( )->fRootEntity( ).fSpawnChild( cImpactEffect ) );
		//mExplosion->fForEachDescendent( FX::tAddPausedFxSystem( mExplosionArray ) );
	}

	void tLightningWeapon::fOnDelete( )
	{
		tGunWeapon::fOnDelete( );
		mLightning.fRelease( );
		mExplosionArray.fSafeCleanup( );
		mExplosion.fRelease( );
	}

	void tLightningWeapon::fProcessST( f32 dt )
	{
		if( fUnderUserControl( ) && mDesc.mRaycastAdjustTargets )
			mLightning->fSetTarget( mReticalTargetPosition );
		else
			mLightning->fSetTarget( mPredictedTargetPosition );
		mLightning->fSetAlphaTarget( mLastProjectiles.fCount( ) ? 1.f : 0.f );

		tGunWeapon::fProcessST( dt );

		// happens after base process to allow a proj to be spawned
		if( mLoopingAudioPushed && !mLastProjectiles.fCount( ) )
			tGunWeapon::fAudioEnd( ); //calls base, not overrided dead one
	}

	void tLightningWeapon::fProcessMT( f32 dt )
	{
		tGunWeapon::fProcessMT( dt );
	}

	void tLightningWeapon::fSpawnFireEffect( tProjectileLogic* projectile, u32 anchorIndex )
	{
		mExplosionArray.fReset( );
		fPlayRecoilMotion( anchorIndex );
	}

	void tLightningWeapon::fAudioEnd( )
	{
		//override, no behavior
	}


	namespace
	{
		static const tStringPtr cLaserTracer( "LASER_BARRAGE" );
		static const tFilePathPtr cLaserBurn( "gameplay/barrage/laser_burn.sigml" );
	}

	u32 tOrbitalLaserWeapon::fNumFracs( ) 
	{
		return Gameplay_Weapon_Orbital_Fracs;
	}

	tOrbitalLaserWeapon::tOrbitalLaserWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst )
		: tGunWeapon( desc, inst )
		, mPathDist( 0.f )
		, mPath( NULL )
		, mIsFiring( false )
	{ }

	void tOrbitalLaserWeapon::fOnSpawn( )
	{
		tGunWeapon::fOnSpawn( );

		u32 tracer = tGameApp::fInstance( ).fTracersTable( ).fTable( ).fRowIndex( cLaserTracer );
		log_assert( tracer != ~0, "Tracer not found: " << cLaserTracer);

		mLightning.fReset( new tLightningEntity( tracer ) );
		mLightning->fSpawn( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
		mLightning->fSetFracs( Gameplay_Weapon_Orbital_Fracs );

		mLightning2Space.fReset( new tLightningEntity( tracer ) );
		mLightning2Space->fSpawn( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
		mLightning2Space->fSetFracs( Gameplay_Weapon_Orbital_Fracs );

		mBurn.fReset( tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( )->fSpawnChild( cLaserBurn ) );
		mBurn->fForEachDescendent( FX::tAddPausedFxSystem( mFx ) );
		mFx.fPause( false );
		mFx.fSetEmissionPercent( 0.f );

		// do some damage
		mDamageLogic = NEW tAreaDamageLogic( );
		tLogicPtr *dlp = NEW tLogicPtr( mDamageLogic );
		mBurn->fAcquireLogic( dlp );

		mDamageLogic->fSetDamageID( fBuildID( ) );
		mDamageLogic->fEnable( false );

		mBurnSound.fReset( new Audio::tSource( "Laser Burn" ) );
		mBurnSound->fSpawn( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
	}

	void tOrbitalLaserWeapon::fOnDelete( )
	{
		tGunWeapon::fOnDelete( );
		mLightning.fRelease( );
		mLightning2Space.fRelease( );
		mFx.fSafeCleanup( );
		mBurn.fRelease( );
		mDamageLogic = NULL;

		if( mIsFiring && mBurnSound )
			mBurnSound->fHandleEvent( cLaserBurnStopSound );
		mBurnSound.fRelease( );
	}

	void tOrbitalLaserWeapon::fBeginAreaEffect( u32 anchorIndex )
	{
		mIsFiring = true;
		mDamageLogic->fEnable( true );

		if( mAITargetOverride )
			mPath = mAITargetOverride->fDynamicCast<tPathEntity>( );
		else if( mAITarget )
			mPath = mAITarget->fDynamicCast<tPathEntity>( );
		else
			mPath = NULL;

		mPathDist = 0.f;

		tGunWeapon::fBeginAreaEffect( anchorIndex );

		fHandleAudioEvent( cLaserStartSound );
		mBurnSound->fHandleEvent( cLaserBurnStartSound );
	}

	void tOrbitalLaserWeapon::fEndAreaEffect( u32 anchorIndex )
	{
		mIsFiring = false;
		mDamageLogic->fEnable( false );

		tGunWeapon::fEndAreaEffect( anchorIndex );

		fHandleAudioEvent( cLaserStopSound );
		mBurnSound->fHandleEvent( cLaserBurnStopSound );
	}

	void tOrbitalLaserWeapon::fProcessST( f32 dt )
	{
		tGunWeapon::fProcessST( dt );

		if( mIsFiring )
		{
			tVec3f target;

			if( mPath )
			{
				f32 rate = 10.f;
				mPathDist += dt * rate;

				b32 onPathStill = mPath->fTraversePath( mPathDist, target );
				if( !onPathStill )
				{
					if( mAIFireOverride )
						fSetAIFireOverride( false );
					fEndFire( );
				}
			}
			else if( fUnderUserControl( ) && mDesc.mRaycastAdjustTargets )
				target = mReticalTargetPosition;
			else
				target = mPredictedTargetPosition;


			tVec3f anchorPt = fFirstAnchorPoint( )->fObjectToWorld( ).fGetTranslation( );
			tVec3f midPoint = (target + anchorPt) * 0.5f + tVec3f( 0, Gameplay_Weapon_Orbital_ApexHeight, 0 );

			mLightning->fSetTarget( target );
			mLightning->fMoveTo( midPoint );
			mLightning2Space->fSetTarget( midPoint );
			mLightning2Space->fMoveTo( anchorPt );

			mBurnSound->fMoveTo( target );
			mBurn->fMoveTo( target );
			mFx.fSetEmissionPercent( 1.0f );

			mLightning->fSetAlphaTarget( 1.f );
			mLightning2Space->fSetAlphaTarget( 1.f );
		}
		else
		{
			mLightning->fSetAlphaTarget( 0.f );
			mLightning2Space->fSetAlphaTarget( 0.f );
			mFx.fSetEmissionPercent( 0.f );
		}
	}
}

