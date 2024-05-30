#include "GameAppPch.hpp"
#include "tWeaponStation.hpp"
#include "tGameApp.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tDataTableFile.hpp"

#include "tGunWeapon.hpp"
#include "tCannonWeapon.hpp"
#include "tMortarWeapon.hpp"
#include "tLightningWeapon.hpp"
#include "tShellLogic.hpp"

#include "tUnitLogic.hpp"
#include "tLevelLogic.hpp"
#include "Input/tRumbleManager.hpp"
#include "Math/ProjectileUtility.hpp"
#include "Physics/tLongRangeRaycaster.hpp"

#include "Wwise_IDs.h"

using namespace Sig::Math;

namespace Sig
{

	devvar( f32, Gameplay_Weapon_UISpeed, 10.0f );
	devvar( f32, Gameplay_Weapon_UIBorder, 100.0f );
	devvar( f32, Gameplay_Weapon_UIBorderBottom, 10.0f );
	devvar( f32, Gameplay_Weapon_RangeRingYTweak, 6.5f );
	devvar( bool, Gameplay_Weapon_ApplyTable, false );
	devvar( bool, Gameplay_Weapon_EnableRumble, true );
	devvar( f32, Gameplay_Weapon_OverChargeRumbleMult, 1.5f );
	devvar( f32, Gameplay_Weapon_ScreenShakeMult, 1.5f );



	devvar( bool, Perf_LongRangeRC_Raycaster, false );
	devvar( bool, Perf_LongRangeRC_QuickReject, false );
	devvar( f32, Perf_LongRangeRC_Extra, 25.0f );


	devvar( bool, Debug_Weapons_DisableScreenTargetting, false );

	namespace 
	{ 
		static const tStringPtr cShellCasing("shellCasing"); 
		static const tStringPtr cShellCasingDir("shellCasingDir");

		enum tWeaponTableColumns
		{
			cWeaponTableColumnAudioAlias,
			cWeaponTableColumnAudioType,
			cWeaponTableColumnUseBankAudio,
			cWeaponTableColumnProjectilePath,
			cWeaponTableColumnHitEffect,
			cWeaponTableColumnHitEffectOverCharged,
			cWeaponTableColumnFireEffectPath,
			cWeaponTableColumnFireEffectPathOverCharged,
			cWeaponTableColumnAfterFireEffectPath,
			cWeaponTableColumnReloadEffectPath,
			cWeaponTableColumnShellCasingPath,

			cWeaponTableColumnUIScript,
			cWeaponTableColumnUIAmmoIconPath,
			cWeaponTableColumnUIAmmoTickMarkPath,
			cWeaponTableColumnWorldSpaceTrajectory,
			cWeaponTableColumnShootThroughReticle,
			cWeaponTableColumnStickyReticle,

			cWeaponTableColumnArrowWidth,
			cWeaponTableColumnWeaponType,
			cWeaponTableColumnCheckTargetLineOfSight,
			cWeaponTableColumnEffectDamageType,
			cWeaponTableColumnDoesntNeedToPointAtTarget,
			cWeaponTableColumnMinTargetRange,
			cWeaponTableColumnMaxTargetRange,
			cWeaponTableColumnMinPitch,
			cWeaponTableColumnMaxPitch,
			cWeaponTableColumnProjectileVelocity,
			cWeaponTableColumnProjectileAcceleration,
			cWeaponTableColumnProjectileGravity,
			cWeaponTableColumnProjectileSpin,
			cWeaponTableColumnParentVel,
			cWeaponTableColumnExplosionSize,
			cWeaponTableColumnExplosionGrowth,
			cWeaponTableColumnExplosionFallOff,
			cWeaponTableColumnMaxAmmo,
			cWeaponTableColumnMaxAmmoAI,
			cWeaponTableColumnContinuousFire,
			cWeaponTableColumnRapidFire,
			cWeaponTableColumnSpinUp,
			cWeaponTableColumnAreaEffect,
			cWeaponTableColumnPersistentDamage,
			cWeaponTableColumnFireRate,
			cWeaponTableColumnBankFireRate,
			cWeaponTableColumnMaxSpread,
			cWeaponTableColumnSpreadRate,
			cWeaponTableColumnSpreadSettleRate,
			cWeaponTableColumnAiSpread,
			cWeaponTableColumnBulletTracerType,
			cWeaponTableColumnBulletTracerTypeOverCharged,
			cWeaponTableColumnTracerTrailType,
			cWeaponTableColumnTracerTrailInterval,
			cWeaponTableColumnTracerTrailTypeOverCharged,
			cWeaponTableColumnTracerTrailIntervalOverCharged,
			cWeaponTableColumnTurnRate,
			cWeaponTableColumnPreSpawnProjectiles,
			cWeaponTableColumnReloadTime,
			cWeaponTableColumnReloadTimeAI,
			cWeaponTableColumnFireMode,
			cWeaponTableColumnAnimationDriven,
			cWeaponTableColumnShotCount,

			cWeaponTableColumnDamageCasual,
			cWeaponTableColumnDamageNormal,
			cWeaponTableColumnDamageHard,
			cWeaponTableColumnDamageElite,
			cWeaponTableColumnDamageGeneral,
			cWeaponTableColumnDamageVersus,
			cWeaponTableColumnDamageModDirectHit,
			cWeaponTableColumnDamageModTurret,
			cWeaponTableColumnDamageModVehicle,
			cWeaponTableColumnDamageModInfantry,
			cWeaponTableColumnDamageModAir,
			cWeaponTableColumnDamageModBoss,
			cWeaponTableColumnDamageModLightProp,
			cWeaponTableColumnDamageModHeavyProp,
			cWeaponTableColumnUserModeDamageMultiplier,
			cWeaponTableColumnUserModeDamageMultiplierVersus,
			cWeaponTableColumnOverChargeMultiplier,
			cWeaponTableColumnTargetTypes,
			cWeaponTableColumnLeadTarget,
			cWeaponTableColumnReactionImpulse,
			cWeaponTableColumnDamageImpulseScale,
			cWeaponTableColumnMaxLocks,
			cWeaponTableColumnLockTypes,
			cWeaponTableColumnShootIfNoLock,
			cWeaponTableColumnRumbleMag,
			cWeaponTableColumnRumbleMix,
			cWeaponTableColumnRumbleTime,
			cWeaponTableColumnRumbleDecay,
			cWeaponTableColumnCameraShakeMag,
			cWeaponTableColumnCameraShakeTime,

			cWeaponTableColumnLightSize, //muzle flash light
			cWeaponTableColumnLightLife,
			cWeaponTableColumnLightColor,
			cWeaponTableColumnImpLightSize, //impact flash light
			cWeaponTableColumnImpLightLife,
			cWeaponTableColumnImpLightColor,

			cWeaponTableColumnWantsShellCam,
			cWeaponTableColumnShellCamBlend,
			cWeaponTableColumnShellCamInitiateTimer,
			cWeaponTableColumnShellCamBlendSpeeds,

			cWeaponTableColumnNormalStartAudioID,
			cWeaponTableColumnNormalStopAudioID,
			cWeaponTableColumnShellCamStartAudioID,
			cWeaponTableColumnShellCamStopAudioID,
			cWeaponTableColumnScopeStartAudioID,
			cWeaponTableColumnScopeStopAudioID,
			cWeaponTableColumnSpecialStartAudioID,
			cWeaponTableColumnSpecialStopAudioID,

			cWeaponTableColumnBarrageWeapon,
			cWeaponTableColumnStatToInc
		};

		void fParseUnitTypeString( const tStringPtr& str, tGrowableArray<GameFlags::tUNIT_TYPE>& out )
		{
			out.fSetCount( 0 );

			tGrowableArray<std::string> types;
			StringUtil::fSplit( types, StringUtil::fStripQuotes( std::string( str.fCStr( ) ) ).c_str( ), "," );
			for( u32 i = 0; i < types.fCount( ); ++i )
			{
				std::string type = StringUtil::fEatWhiteSpace( types[ i ] );
				u32 enumVal = GameFlags::fUNIT_TYPEValueStringToEnum( tStringPtr( type ) );
				if( enumVal < GameFlags::cUNIT_TYPE_COUNT )
					out.fPushBack( (GameFlags::tUNIT_TYPE)enumVal );
			}
		}

		#define fCheckIndexAddToArray( array, member, ent ) \
		{ \
			sigassert( ent ); \
			u32 index = ent->fQueryEnumValue( GameFlags::cENUM_WEAPON_INDEX, ~0 ); \
			if( index != ~0 ) \
			{ \
				if( index >= array.fCount( ) ) array.fSetCount( index + 1 ); \
				if( array[ index ].member.fGetRawPtr( ) ) \
					log_warning( 0, "Index already occupied by weapon attachment point!" ); \
				array[ index ].member.fReset( ent ); \
			} \
			else \
			{ \
				array.fPushBack( tMuzzle( ) ); \
				array.fBack( ).member.fReset( ent ); \
			} \
		} \

		#define fCheckIndexAddToArrayFirstEmpty( array, member, ent ) \
		{ \
			sigassert( ent ); \
			u32 index = ent->fQueryEnumValue( GameFlags::cENUM_WEAPON_INDEX, ~0 ); \
			if( index != ~0 ) \
			{ \
				if( index >= array.fCount( ) ) array.fSetCount( index + 1 ); \
				if( array[ index ].member ) \
					log_warning( 0, "Index already occupied by weapon attachment point!" ); \
				array[ index ].member.fReset( ent ); \
			} \
			else \
			{ \
				u32 index = ~0; \
				for( u32 x = 0; x < array.fCount( ); ++x ) if( !array[ x ].member ) { index = x; break; } \
				if( index != ~0 ) array[ index ].member.fReset( ent ); \
				else \
					log_warning( 0, "No empty index!" ); \
			} \
		} \

		b32 fCheckArrayValidity( tGrowableArray<tMuzzle>& array )
		{
			b32 errors = false;

			for( s32 i = 0; i < (s32)array.fCount( ); ++i )
				if( !array[ i ].mProjectileSpawn )
				{
					errors = true;
					array.fErase( i );
					--i;
				}

			return !errors;			
		}
	}


	void tWeaponDescs::fSetup( )
	{
		const tDataTable& datatable = tGameApp::fInstance( ).fWeaponsTable( ).fTable( );
		mDescs.fSetCount( datatable.fRowCount( ) );
		for( u32 r = 0; r < mDescs.fCount( ); ++r )
			fLoadDesc( r );
	}

	void tWeaponDescs::fLoadDesc( u32 r )
	{
		const tDataTable& datatable = tGameApp::fInstance( ).fWeaponsTable( ).fTable( );
		tWeaponDesc& desc = mDescs[ r ];

		desc.mWeaponDescIndex = r;
		desc.mWeaponDescName = datatable.fRowName( r );
		desc.mAudioAlias = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnAudioAlias );
		if( desc.mAudioAlias.fLength( ) == 0 )
			desc.mAudioAlias = desc.mWeaponDescName;
		desc.mAudioType = (u32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnAudioType );
		desc.mUseBankAudio = (u32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnUseBankAudio );	

		desc.mProjectilePath = datatable.fIndexByRowCol<tFilePathPtr>( r, cWeaponTableColumnProjectilePath );
		desc.mHitEffect = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnHitEffect );
		desc.mHitEffectOverCharged = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnHitEffectOverCharged );
		desc.mFireEffectPath = datatable.fIndexByRowCol<tFilePathPtr>( r, cWeaponTableColumnFireEffectPath );
		desc.mFireEffectPathOverCharged = datatable.fIndexByRowCol<tFilePathPtr>( r, cWeaponTableColumnFireEffectPathOverCharged );
		desc.mAfterFireEffectPath = datatable.fIndexByRowCol<tFilePathPtr>( r, cWeaponTableColumnAfterFireEffectPath );
		desc.mReloadEffectPath = datatable.fIndexByRowCol<tFilePathPtr>( r, cWeaponTableColumnReloadEffectPath );
		desc.mShellCasingPath = datatable.fIndexByRowCol<tFilePathPtr>( r, cWeaponTableColumnShellCasingPath );
		desc.mUIScriptPath = datatable.fIndexByRowCol<tFilePathPtr>( r, cWeaponTableColumnUIScript );
		desc.mUIAmmoIconPath = datatable.fIndexByRowCol<tFilePathPtr>( r, cWeaponTableColumnUIAmmoIconPath );
		desc.mUIAmmoTickMarkPath = datatable.fIndexByRowCol<tFilePathPtr>( r, cWeaponTableColumnUIAmmoTickMarkPath );

		desc.mMinRange = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnMinTargetRange );
		desc.mMaxRange = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnMaxTargetRange );
		desc.mMinPitch = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnMinPitch );
		desc.mMaxPitch = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnMaxPitch );
		desc.mTurnRate = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnTurnRate );
		desc.mFireRate = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnFireRate );
		desc.mBankFireRate = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnBankFireRate );
		desc.mMaxSpread = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnMaxSpread );
		desc.mSpreadRate = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnSpreadRate );
		desc.mSpreadSettleRate = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnSpreadSettleRate );
		desc.mAISpread = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnAiSpread );
		desc.mProjectileSpeed = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnProjectileVelocity );
		desc.mProjectileAcceleration = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnProjectileAcceleration );
		desc.mProjectileGravity = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnProjectileGravity );
		desc.mProjectileSpin = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnProjectileSpin );		
		desc.mParentVelScale = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnParentVel );
		desc.mExplosionFullSize = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnExplosionSize );
		desc.mExplosionGrowthRate = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnExplosionGrowth );
		desc.mExplosionFallOff = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnExplosionFallOff );
		desc.mMaxAmmo = (u32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnMaxAmmo );
		desc.mMaxAmmoAI = (u32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnMaxAmmoAI );

		desc.mIsContinuousFire = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnContinuousFire );
		desc.mRapidFire = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnRapidFire );
		desc.mSpinUp = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnSpinUp );
		desc.mAreaDamage = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnAreaEffect );
		desc.mPersistentDamageType = GameFlags::fPERSISTENT_EFFECT_BEHAVIORValueStringToEnum( datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnPersistentDamage ) );
		if( desc.mPersistentDamageType == GameFlags::cPERSISTENT_EFFECT_BEHAVIOR_COUNT ) desc.mPersistentDamageType = ~0;
		
		desc.mRaycastAdjustTargets = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnShootThroughReticle );
		desc.mStickyReticle = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnStickyReticle );
		desc.mWantsShellCam = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnWantsShellCam );
		desc.mShootIfNoLock = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnShootIfNoLock );
		desc.mWantsWorldSpaceUI = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnWorldSpaceTrajectory );
		desc.mBulletTracerType = tGameApp::fInstance( ).fTracersTable( ).fRowIndex( datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnBulletTracerType ) );
		desc.mBulletTracerTypeOverCharged = tGameApp::fInstance( ).fTracersTable( ).fRowIndex( datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnBulletTracerTypeOverCharged ) );
		desc.mTracerTrailType = tGameApp::fInstance( ).fTracersTable( ).fRowIndex( datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnTracerTrailType ) );
		desc.mTracerTrailInterval = (u32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnTracerTrailInterval );
		desc.mTracerTrailTypeOverCharged = tGameApp::fInstance( ).fTracersTable( ).fRowIndex( datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnTracerTrailTypeOverCharged ) );
		desc.mTracerTrailIntervalOverCharged = (u32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnTracerTrailIntervalOverCharged );
		desc.mPreSpawnProjectiles = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnPreSpawnProjectiles );
		desc.mReloadTime = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnReloadTime );
		desc.mReloadTimeAI = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnReloadTimeAI );
		desc.mFireMode = (tWeaponFireMode)(u32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnFireMode );
		desc.mAnimationDriven = (b32)datatable.fIndexByRowCol<u32>( r, cWeaponTableColumnAnimationDriven );
		u32 sc = datatable.fIndexByRowCol<u32>( r, cWeaponTableColumnShotCount );
		sigassert( sc < std::numeric_limits<u8>::max( ) );
		desc.mShotCount = sc;
		
		desc.mHitPointsVersus = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnDamageVersus );
		sigassert( cWeaponTableColumnDamageCasual + tGameApp::fInstance( ).fDifficulty( ) < cWeaponTableColumnDamageVersus );
		for( u32 i = 0; i < GameFlags::cDIFFICULTY_COUNT; ++i )
			desc.mHitPoints[ i ] = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnDamageCasual + i );

		desc.mDamageModDirectHit = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnDamageModDirectHit );
		for( u32 i = GameFlags::cUNIT_TYPE_TURRET; i <= GameFlags::cUNIT_TYPE_HEAVY_PROP; ++i )
			desc.mDamageMod[ i ] = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnDamageModTurret + i - 1 ); // mDamageMod[ 0 ] stays default at 1.f;
		desc.mUserModeDamageMultiplierVersus = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnUserModeDamageMultiplierVersus );
		desc.mUserModeDamageMultiplier = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnUserModeDamageMultiplier );
		desc.mOverChargeMultiplier = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnOverChargeMultiplier );

		desc.mArrowWidth = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnArrowWidth );
		desc.mWeaponType = (u32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnWeaponType );
		desc.mCheckTargetLineOfSight = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnCheckTargetLineOfSight );
		desc.mEffectDamageType = (GameFlags::tDAMAGE_TYPE)GameFlags::fDAMAGE_TYPEValueStringToEnum( datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnEffectDamageType ) );
		desc.mReactionImpulse = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnReactionImpulse );
		desc.mDamageImpulseScale = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnDamageImpulseScale  );
		desc.mMaxLocks = (u16)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnMaxLocks );
		desc.mLeadTargets = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnLeadTarget );
		desc.mDoesntNeedtoPointAtTarget = (b32)datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnDoesntNeedToPointAtTarget );		

		desc.mMuzzleFlashLightSize = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnLightSize );		
		desc.mMuzzleFlashLightLife = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnLightLife );			
		desc.mMuzzleFlashLightColor = datatable.fIndexByRowCol<tVec4f>( r, cWeaponTableColumnLightColor );	
		desc.mImpactLightSize = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnImpLightSize );		
		desc.mImpactLightLife = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnImpLightLife );			
		desc.mImpactLightColor = datatable.fIndexByRowCol<tVec4f>( r, cWeaponTableColumnImpLightColor );	

		desc.mShellCamInitiateTimer = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnShellCamInitiateTimer  );
		Math::tVec4f shellCamBlends = datatable.fIndexByRowCol<Math::tVec4f>( r, cWeaponTableColumnShellCamBlendSpeeds  );
		desc.mShellCamBlendInDepart = shellCamBlends.x;
		desc.mShellCamBlendInArrive = shellCamBlends.y;
		desc.mShellCamOverallBlend = datatable.fIndexByRowCol<f32>( r, cWeaponTableColumnShellCamBlend );

		desc.mNormalStartAudioID = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnNormalStartAudioID );
		desc.mNormalStopAudioID = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnNormalStopAudioID );
		desc.mShellCamStartAudioID = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnShellCamStartAudioID );
		desc.mShellCamStopAudioID = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnShellCamStopAudioID );
		desc.mScopeStartAudioID = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnScopeStartAudioID );
		desc.mScopeStopAudioID = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnScopeStopAudioID );
		desc.mSpecialStartAudioID = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnSpecialStartAudioID );
		desc.mSpecialStopAudioID = datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnSpecialStopAudioID );

		desc.mBarrageWeapon = datatable.fIndexByRowCol<b32>( r, cWeaponTableColumnBarrageWeapon );
		desc.mUseStatToInc = GameFlags::fSESSION_STATSStringToEnum( datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnStatToInc ) );
		


		fParseUnitTypeString( datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnTargetTypes ), desc.mTargetTypes );
		fParseUnitTypeString( datatable.fIndexByRowCol<tStringPtr>( r, cWeaponTableColumnLockTypes ), desc.mLockTypes );

		if( desc.mWeaponType == tGameApp::cWeaponDerivedTypeCannon || desc.mWeaponType == tGameApp::cWeaponDerivedTypeMortar )
		{
			const f32 gravity = desc.fShellGravity( );
			if( gravity < 0.f && desc.mProjectileSpeed > 0.f )
			{
				//recompute min and max pitch to use the min and max range. given the projectile velocity
				f32 minAngle = 0;
				f32 maxAngle = 0;
				b32 mortar = (desc.mWeaponType == tGameApp::cWeaponDerivedTypeMortar);

				// TODO, FIGURE OUT WHERE TO GET THE ACTUAL Y DELTA, INSTEAD OF A HARDCODED ONE.
				f32 heightAboveTarget = Gameplay_Weapon_RangeRingYTweak; //recurssively sample this to get perfect range capabilities. 
				b32 minValid = ProjectileUtility::fComputeLaunchAngle( minAngle, desc.mProjectileSpeed, tVec2f( desc.mMinRange, -heightAboveTarget ), gravity, mortar );
				b32 maxValid = ProjectileUtility::fComputeLaunchAngle( maxAngle, desc.mProjectileSpeed, tVec2f( desc.mMaxRange, -heightAboveTarget ), gravity, mortar );

				minAngle = fToDegrees( minAngle );
				maxAngle = fToDegrees( maxAngle );

				b32 error = false;

				if( !minValid ) { error = true; log_warning( 0, "Weapon cannot hit min range with current projectile speed and gravity. ID: " << desc.mWeaponDescName ); }
				if( !maxValid ) { error = true; log_warning( 0, "Weapon cannot hit max range with current projectile speed and gravity. ID: " << desc.mWeaponDescName ); }
				if( minAngle < desc.mMinPitch ) { error = true; log_warning( 0, "Weapon cannot pitch low enough for min range. ID: " << desc.mWeaponDescName ); }
				if( maxAngle > desc.mMaxPitch ) { error = true; log_warning( 0, "Weapon cannot pitch high enough for max range. ID: " << desc.mWeaponDescName ); }

				if( minAngle > maxAngle ) 
				{
					if( !mortar ) 
						log_line( 0, "Had to swap angles for weapon which is not a mortar. Verify data for weapon ID: " << desc.mWeaponDescName );
					fSwap( minAngle, maxAngle ); //may want to assert that this only happens for mortars.
				}

				if( !error )
				{
					desc.mMinPitch = minAngle;
					desc.mMaxPitch = maxAngle;
				}
				else
					log_warning( 0, "Weapon pitches could not be adjusted." );
			}
		}
	}

	const tWeaponDesc& tWeaponDescs::fDesc( const tStringPtr& weaponID )
	{
		const tStringHashDataTable& datatable = tGameApp::fInstance( ).fWeaponsTable( );
		u32 rowIndex = datatable.fRowIndex( weaponID );
		log_assert( rowIndex != ~0, "Could not find weapon in table: " << weaponID );

		if( Gameplay_Weapon_ApplyTable ) 
			fLoadDesc( rowIndex );

		return mDescs[ rowIndex ];
	}

	tWeaponBank::tWeaponBank( tUnitLogic* unitLogic, tWeaponStation* station, u32 id )
		: mUnitLogic( unitLogic )
		, mStation( station )
		, mFireMode( cWeaponFireModeAll )
		, mNextFireIndex( 0 )
		, mTriggerButton( GameFlags::cGAME_CONTROLS_TRIGGER_PRIMARY )
		, mFireRate( -1.f )
		, mFireRateTimer( -1.f )
		, mAcquiringLocks( false )
		, mShootIfNoLock( true )
		, mEnabled( true )
		, mBankID( id )
		, mLastAudioEvent( ~0 )
		, mMaxLocks( 0 )
	{
	}

	tWeapon* tWeaponBank::fAddWeapon( const tStringPtr& weaponName, const tStringPtr& parentEntity )
	{
		mWeapons.fPushBack( fCreateWeapon( weaponName, parentEntity ) );

		tWeapon* weap = mWeapons.fBack( ).fGetRawPtr( );
		weap->fInst( ).mBankID = mBankID;

		if( mWeapons.fCount( ) > 1 )
		{
			//tell second and latter weapons to child their targetting off of the first weapon in the bank
			weap->fInst( ).mTargettingParent = mWeapons[ 0 ];
		}

		return mWeapons.fBack( ).fGetRawPtr( );
	}

	const tWeaponPtr& tWeaponBank::fWeapon( u32 index ) const
	{
		log_assert( index < mWeapons.fCount( ), "mWeapons[" << index << "] outside range.  Current count: " << mWeapons.fCount( ) );
		return mWeapons[ index ];
	}

	tWeaponPtr& tWeaponBank::fWeapon( u32 index )
	{
		log_assert( index < mWeapons.fCount( ), "mWeapons[" << index << "] outside range.  Current count: " << mWeapons.fCount( ) );
		return mWeapons[ index ];
	}

	tWeapon* tWeaponBank::fWeaponRawPtr( u32 index )
	{
		log_assert( index < mWeapons.fCount( ), "mWeapons[" << index << "] outside range.  Current count: " << mWeapons.fCount( ) );
		return mWeapons[ index ].fGetRawPtr( );
	}

	void tWeaponBank::fOnSpawn( )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fOnSpawn( );
	}

	void tWeaponBank::fOnDelete( )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fOnDelete( );
	}

	tWeaponPtr tWeaponBank::fCreateWeapon( const tStringPtr& weaponName, const tStringPtr& parentEntity )
	{
		sigassert( mUnitLogic );
		sigassert( mStation );
		sigassert( mUnitLogic->fOwnerEntity( ) );

		const tWeaponDesc& desc = tWeaponDescs::fInstance( ).fDesc( weaponName );
		const tGameMode& gm = tGameApp::fInstance( ).fGameMode( );

		tWeaponInstData inst;
		inst.mOwnerUnit = mUnitLogic;
		inst.mIgnoreParent = mUnitLogic->fHitpointLinkedUnitLogic( ) ? mUnitLogic->fHitpointLinkedUnitLogic( ) : mUnitLogic;
		inst.mStation = mStation;
		inst.mTriggerButton = mTriggerButton;
		inst.mUserDamageMultiplier = gm.fIsVersus( ) ? desc.mUserModeDamageMultiplierVersus : desc.mUserModeDamageMultiplier;

		tEntity* srcEntity = mUnitLogic->fOwnerEntity( );
		if( parentEntity.fExists( ) )
		{
			tEntity* find = srcEntity->fFirstDescendentWithName( parentEntity );
			if( find ) 
				srcEntity = find;
		}

		// TODO THESE POINTS AND CASINGS AND DIRS NEED TO BE COORDINATED WITH AN INDEX TAG
		tGrowableArray<tEntity*> anchorPoints;
		srcEntity->fAllDescendentsWithName( tWeapon::cWeaponAttachName, anchorPoints, false );
		if( anchorPoints.fCount( ) == 0 )
		{
			log_warning( 0, "Could not find attachment point named: " << tWeapon::cWeaponAttachName << " under sigml: " << parentEntity << " for weapon type: " << weaponName << " searching recursively as fallback." );
			srcEntity->fAllDescendentsWithName( tWeapon::cWeaponAttachName, anchorPoints, true );
			log_assert( anchorPoints.fCount( ), "Could not find attachment point named: " << tWeapon::cWeaponAttachName << " under sigml: " << parentEntity << " for weapon type: " << weaponName << " cannot continue." );
		}

		for( u32 i = 0; i < anchorPoints.fCount( ); ++i )
			fCheckIndexAddToArray( inst.mMuzzles, mProjectileSpawn, anchorPoints[ i ] );
		if( !fCheckArrayValidity( inst.mMuzzles ) )
			log_warning( 0, "Weapon attachment points missing for weapon: " << desc.mWeaponDescName );	

		tGrowableArray<tEntity*> shellCasing;
		srcEntity->fAllDescendentsWithName( cShellCasing, shellCasing, false );
		for( u32 i = 0; i < shellCasing.fCount( ); ++i )
			fCheckIndexAddToArrayFirstEmpty( inst.mMuzzles, mShellCasingSpawn, shellCasing[ i ] );

		tGrowableArray<tEntity*> shellCasingDir;
		srcEntity->fAllDescendentsWithName( cShellCasingDir, shellCasingDir, false );
		for( u32 i = 0; i < shellCasingDir.fCount( ); ++i )
			fCheckIndexAddToArrayFirstEmpty( inst.mMuzzles, mShellCasingDir, shellCasingDir[ i ] );

		tWeaponPtr weaponPtr;
		switch( desc.mWeaponType )
		{
		case tGameApp::cWeaponDerivedTypeGun:
			weaponPtr = tWeaponPtr( NEW tGunWeapon( desc, inst ) );
			break;
		case tGameApp::cWeaponDerivedTypeCannon:
			weaponPtr = tWeaponPtr( NEW tCannonWeapon( desc, inst ) );
			break;
		case tGameApp::cWeaponDerivedTypeMortar:
			weaponPtr = tWeaponPtr( NEW tMortarWeapon( desc, inst ) );
			break;
		case tGameApp::cWeaponDerivedTypeLightning:
			weaponPtr = tWeaponPtr( NEW tLightningWeapon( desc, inst ) );
			break;
		case tGameApp::cWeaponDerivedTypeLaser:
			weaponPtr = tWeaponPtr( NEW tOrbitalLaserWeapon( desc, inst ) );
			break;
		}

		if( weaponPtr ) 
			weaponPtr->fInitParticles( );

		return weaponPtr;
	}

	void tWeaponBank::fProcessUserInput( )
	{
		sigassert( mStation->fPlayer( ) );

		b32 firing = fFiring( );
		if( !firing && mStation->fPlayer( )->fGameController( )->fButtonHeld( tUserProfile::cProfileVehicles, mTriggerButton ) )		
			fFire( );
		else if( firing && !mStation->fPlayer( )->fGameController( )->fButtonHeld( tUserProfile::cProfileVehicles, mTriggerButton ) )	
			fEndFire( );

		if( fNeedsReload( ) ) 
		{
			if( firing )
				fEndFire( );
			fReloadAfterTimer( );
		}
	}

	void tWeaponBank::fBeginUserControl( tPlayer* player )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fBeginUserControl( player );
	}

	void tWeaponBank::fEndUserControl( )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fEndUserControl( );

		fClearLocks( );
		mAcquiringLocks = false;
	}

	void tWeaponBank::fProcessST( f32 dt )
	{
		mFireRateTimer -= dt;
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fProcessST( dt );
	}

	void tWeaponBank::fProcessMT( f32 dt )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fProcessMT( dt );
	}

	void tWeaponBank::fSetAcquireTargets( b32 enable )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fSetAcquireTargets( enable );
	}

	b32 tWeaponBank::fFire( )
	{
		b32 result = false;

		if( mEnabled || fAIFireOverride( ) )
		{
			if( mWeapons.fCount( ) == 0 ) 
				return false;

			mFireRateTimer = mFireRate;
			
			if( mMaxLocks > 0 )
				mAcquiringLocks = true;
			else
				result = fReallyFire( );
		}

		return result;
	}
	b32 tWeaponBank::fReallyFire( const tWeaponTarget* target )
	{
		b32 result = false;
		u32 wCnt = mWeapons.fCount( );

		switch( mFireMode )
		{
		case cWeaponFireModeAll:
			{
				for( u32 i = 0; i < wCnt; ++i )
					if( mWeapons[ i ]->fShouldFire( ) && mWeapons[ i ]->fFire( target ) )
						result = true;
			}
			break;
		case cWeaponFireModeAlternate:
			{
				//validate current next fire index
				for( u32 i = 0; i < wCnt; ++i )
				{
					u32 index = fModulus( mNextFireIndex + i, wCnt );
					if( mWeapons[ index ]->fShouldFire( ) )
					{
						mNextFireIndex = index;
						break;
					}
				}

				if( mNextFireIndex < wCnt )
				{
					if( mWeapons[ mNextFireIndex ]->fShouldFire( ) && mWeapons[ mNextFireIndex++ ]->fFire( target ) )
						result = true;
				}
			}
			break;
		}

		return result;
	}
	void tWeaponBank::fEndFire( )
	{
		if( mAcquiringLocks )
		{
			if( mLocks.fCount( ) )
			{
				u32 myIndex = ~0;
				for( u32 i = 0; i < mLocks.fCount( ); ++i )
				{
					myIndex = mLocks[ i ]->mLockedForBank;
					fReallyFire( mLocks[ i ].fGetRawPtr( ) );
				}
				mLocks.fSetCount( 0 );
				if( myIndex != ~0 ) mStation->fClearBankLocks( myIndex );
			}
			else if( mShootIfNoLock )
				fReallyFire( );
		}

		mAcquiringLocks = false;
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fEndFire( );
	}
	void tWeaponBank::fReload( )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fReload( );
	}
	void tWeaponBank::fReloadAfterTimer( )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fReloadAfterTimer( );
	}
	void tWeaponBank::fAccumulateFireEvents( tFireEventList& list )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fAccumulateFireEvents( list );
	}
	void tWeaponBank::fPitchTowardsDesiredAngle( f32 dt )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fPitchTowardsIdealAngle( dt );
	}
	void tWeaponBank::fSpinUp( b32 up )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fSpinUp( up );
	}
	b32 tWeaponBank::fFiring( ) const
	{
		b32 result = mAcquiringLocks;
		for( u32 i = 0; !result && i < mWeapons.fCount( ); ++i )
			if( mWeapons[ i ]->fFiring( ) )
				result = true;
		return result;
	}
	b32 tWeaponBank::fNeedsReload( ) const
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			if( !mWeapons[ i ]->fNeedsReload( ) )
				return false;
		return true;
	}
	b32 tWeaponBank::fIsContinuousFire( ) const
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			if( mWeapons[ i ]->fIsContinuousFire( ) )
				return true;
		return false;
	}
	b32 tWeaponBank::fShellCaming( ) const
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			if( mWeapons[ i ]->fShellCaming( ) )
				return true;
		return false;
	}
	b32 tWeaponBank::fShouldAcquire( ) const
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			if( mWeapons[ i ]->fShouldAcquire( ) )
				return true;
		return false;
	}
	b32 tWeaponBank::fHasTarget( ) const
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			if( mWeapons[ i ]->fHasTarget( ) )
				return true;
		return false;
	}
	b32 tWeaponBank::fReloading( ) const
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			if( mWeapons[ i ]->fReloading( ) )
				return true;
		return false;
	}
	void tWeaponBank::fSetAIFireOverride( b32 fire )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fSetAIFireOverride( fire );
	}
	b32 tWeaponBank::fAIFireOverride( ) const
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			if( mWeapons[ i ]->fAIFireOverride( ) )
				return true;
		return false;
	}
	void tWeaponBank::fSetReloadOverride( b32 fire )
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fSetReloadOverride( fire );
	}
	b32 tWeaponBank::fCanFire( ) const
	{
		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			if( mWeapons[ i ]->fCanFire( ) )
				return true;
		return false;
	}
	b32 tWeaponBank::fShouldFire( ) const
	{
		if( mFireRateTimer > 0.f )
			return false;

		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			if( mWeapons[ i ]->fShouldFire( ) )
				return true;
		return false;
	}
	void tWeaponBank::fCacheWeaponData( )
	{
		mLockTypes.fSetCount( 0 );
		mShootIfNoLock = false;
		mMaxLocks = 0;
		mAnimationDriven = false;
		mFireRate = -1.f;

		tVec3f avgMuzzlePos = tVec3f::cZeroVector;
		tVec3f muzzleDir = tVec3f::cZAxis;
		u32 useAvgMP = 0;

		for( u32 w = 0; w < mWeapons.fCount( ); ++w )
		{
			const tWeaponDesc& desc = mWeapons[ w ]->fDesc( );
			mMaxLocks = fMax( mMaxLocks, (u32)desc.mMaxLocks );
			mShootIfNoLock = mShootIfNoLock || desc.mShootIfNoLock;
			mLockTypes.fJoin( desc.mLockTypes );
			mAnimationDriven = mAnimationDriven || desc.mAnimationDriven;
			mFireRate = desc.mBankFireRate;

			if( desc.mUseBankAudio )
			{
				++useAvgMP;

				const tMat3f& xform = mWeapons[ w ]->fInst( ).mMuzzles[ 0 ].mProjectileSpawn->fParentRelative( );
				avgMuzzlePos += xform.fGetTranslation( );
				muzzleDir = xform.fZAxis( ); //not averaged

				if( !mBankAudio )
				{
					mBankAudio.fReset( NEW Audio::tSource( "Bank audio" ) );
					mBankAudio->fSpawn( *mWeapons[ w ]->fInst( ).mOwnerUnit->fOwnerEntity( ) );
					mBankAudio->fSetSwitch( tGameApp::cWeaponTypeSwitchGroup, desc.mAudioAlias );
					mBankAudio->fSetGameParam( tGameApp::cPlayerControlRTPC, 0.f );
				}
			}
		}

		if( useAvgMP )
		{
			avgMuzzlePos /= (f32)useAvgMP;

			tMat3f xf;
			xf.fSetTranslation( avgMuzzlePos );
			xf.fOrientZAxis( muzzleDir );

			mBankAudio->fSetParentRelativeXform( xf );
		}
	}
	b32 tWeaponBank::fInterestedInLock( GameFlags::tUNIT_TYPE type ) const
	{ 
		return mLockTypes.fFind( type ) != NULL;
	}

	void tWeaponBank::fEnable( b32 enable )
	{
		mEnabled = enable;
		if( !mEnabled ) 
			fEndFire( );

		for( u32 i = 0; i < mWeapons.fCount( ); ++i )
			mWeapons[ i ]->fEnable( enable );
	}
	
	void tWeaponBank::fHandleBankAudioEvent( u32 event )
	{
		sigassert( mBankAudio );
		if( event != mLastAudioEvent )
		{
			mLastAudioEvent = event;
			mBankAudio->fHandleEvent( event );
		}
	}





	namespace 
	{
		struct tWeaponReticalCastCallback
		{
			mutable tRayCastHit		mHit;
			mutable tEntity*		mHitEntity;
			tEntity*				mIgnoreEntity;

			explicit tWeaponReticalCastCallback( tEntity* ignore ) 
				: mHitEntity( 0 ), mIgnoreEntity( ignore )
			{
			}

			inline void fRayCastAgainstSpatial( const tRayf& ray, tSpatialEntity* spatial ) const
			{
				if( spatial == mIgnoreEntity || spatial->fIsAncestorOfMine( *mIgnoreEntity ) )
					return;

				if( Perf_LongRangeRC_QuickReject )
				{
					// since the meshes are in spatial trees this is kind of a waste of time.
					if( spatial->fToSpatialSetObject( )->fQuickRejectByBox( ray ) )
						return;
				}

				tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( !hit.fHit( ) )
					return;

				if( hit.mT < mHit.mT )
				{
					mHit = hit;
					mHitEntity = spatial;
				}					
			}

			inline void operator( )( const tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;

				tSpatialEntity* spatial = static_cast<tSpatialEntity*>( i );

				if( spatial->fHasGameTagsAny( GameFlags::cFLAG_DUMMY | GameFlags::cFLAG_DONT_STOP_BULLETS ) )
					return;

				fRayCastAgainstSpatial( ray, spatial );
			}
		};
	}





	tWeaponStation::tWeaponStation( tUnitLogic* unitLogic ) 
		: mPlayer( NULL )
		, mUnitLogic( unitLogic )
		, mUIPosition( tVec2f::cZeroVector )
		, mMaxRange( 0.f )
		, mMinRange( 0.f )
		, mWantsLocks( false )
		, mStickyReticle( false )
		, mFirstUpdate( false )
		, mToggleCameraButton( ~0 )
		, mLastRaycastDistance( 1.f )
		, mGroundHeightStartAngle( 0 )
		, mGroundHeightAngleRange( 1 )
	{
	}

	void tWeaponStation::fOnSpawn( )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fOnSpawn( );
		fCacheBankData( );
	}

	void tWeaponStation::fOnDelete( )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fOnDelete( );
	}

	const tWeaponBankPtr& tWeaponStation::fBank( u32 index ) const
	{
		sigassert( index < mBanks.fCount( ) );
		return mBanks[ index ];
	}

	tWeaponBankPtr& tWeaponStation::fBank( u32 index )
	{
		sigassert( index < mBanks.fCount( ) );
		return mBanks[ index ];
	}

	tWeaponBank* tWeaponStation::fBankRawPtr( u32 index )
	{
		fCheckBankSize( index ); //this auto resizes to reduce calls from script
		return mBanks[ index ].fGetRawPtr( );
	}

	void tWeaponStation::fCheckBankSize( u32 index )
	{
		u32 cnt = mBanks.fCount( );
		for( u32 i = cnt; i <= index; ++i )
			mBanks.fPushBack( tWeaponBankPtr( NEW tWeaponBank( mUnitLogic, this, mBanks.fCount( ) ) ) );
	}

	void tWeaponStation::fCacheBankData( )
	{
		mMaxRange = 0.f;
		mMinRange = cInfinity;
		mWantsLocks = false;
		mStickyReticle = false;
		mTargetTypes.fSetCount( 0 );

		for( u32 i = 0; i < mBanks.fCount( ); ++i )
		{
			mBanks[ i ]->fCacheWeaponData( );
			mWantsLocks = mWantsLocks || mBanks[ i ]->fWantsLocks( );

			for( u32 w = 0; w < mBanks[ i ]->fWeaponCount( ); ++w )
			{
				const tWeaponDesc& desc = mBanks[ i ]->fWeapon( w )->fDesc( );
				mMaxRange = fMax( mMaxRange, desc.mMaxRange );
				mMinRange = fMin( mMinRange, desc.mMinRange );
				mStickyReticle = mStickyReticle || desc.mStickyReticle;
				
				for( u32 u = 0; u < desc.mTargetTypes.fCount( ); ++u )
					mTargetTypes.fFindOrAdd( desc.mTargetTypes[ u ] );
			}
		}
	}

	const tUserPtr& tWeaponStation::fUser( ) const 
	{ 
		sigassert( mPlayer ); 
		return mPlayer->fUser( ); 
	}

	tPlayer* tWeaponStation::fPlayer( ) const
	{
		sigassert( mPlayer ); 
		return mPlayer;
	}

	void tWeaponStation::fBeginUserControl( tPlayer* player )
	{
		sigassert( player );
		mPlayer = player;
		mFirstUpdate = true;

		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fBeginUserControl( player );

		if( mWeaponUI )
		{
			mWeaponUI->fSetViewPortIndex( mPlayer->fUser( )->fViewportIndex( ) );
			mWeaponUI->fUserControl( true, player );
			fAdjustReticle( NULL ); //reset position
		}

		fPositionUI( );
		fBeginRendering( );
	}

	void tWeaponStation::fEndUserControl( )
	{
		sigassert( mPlayer );

		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fEndUserControl( );

		if( mWeaponUI ) 
		{
			mWeaponUI->fUserControl( false, mPlayer );
			mWeaponUI->fClearTargets( );
		}

		mTargets.fSetCount( 0 );
		mCurrentTarget.fRelease( );

		fEndRendering( );

		mPlayer = NULL;
	}

	void tWeaponStation::fBeginRendering( )
	{
		if( mWeaponUI && mPlayer && !mPlayer->fUser( )->fIsViewportVirtual( ) )
			tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ).fAddChild( mWeaponUI->fCanvas( ) );	
	}

	void tWeaponStation::fEndRendering( )
	{
		if( mWeaponUI && mPlayer && !mPlayer->fUser( )->fIsViewportVirtual( ) )
			tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ).fRemoveChild( mWeaponUI->fCanvas( ) );
	}

	void tWeaponStation::fCreateUI( )
	{
		tWeaponPtr src;

		// will use hte first weapon it finds unless there is one that has mWantsUI true

		for( u32 b = 0; b < mBanks.fCount( ); ++b )
		{
			tWeaponBankPtr& bank = mBanks[b];
			for( u32 w = 0; w < bank->fWeaponCount( ); ++w )
			{
				tWeaponPtr& weap = bank->fWeapon( w );
				if( !src ) src = weap;
				if( weap->fInst( ).mWantsUI )
					goto found;
			}
		}

found:
		if( src && src->fDesc( ).mUIScriptPath.fExists( ) )
		{
			tResourceId id = tResourceId::fMake< tScriptFile >( src->fDesc( ).mUIScriptPath );
			tResourcePtr res = tGameApp::fInstance( ).fResourceDepot( )->fQuery( id );
			log_assert( res->fLoaded( ), "Could not load weapon ui script: " << src->fDesc( ).mUIScriptPath );

			mWeaponUI.fReset( NEW Gui::tWeaponUI( &src->fDesc( ), res ) );

			// Set up the ammo counters
			for( u32 b = 0; b < mBanks.fCount( ); ++b )
			{
				if( mBanks[ b ]->fWeaponCount( ) <= 0 ) continue;

				u32 maxAmmo = 0;
				for( u32 w = 0; w < mBanks[ b ]->fWeaponCount( ); ++w )
				{
					tWeaponPtr& weap = mBanks[ b ]->fWeapon( w );
					maxAmmo += weap->fReloadAmmoCount( true );
				}
				mWeaponUI->fAddAmmoCounter( mBanks[ b ]->fWeapon( 0 )->fDesc( ).mUIAmmoIconPath, mBanks[ b ]->fWeapon( 0 )->fDesc( ).mUIAmmoTickMarkPath, maxAmmo );
			}
		}
	}

	void tWeaponStation::fPositionUI( )
	{
		if( !mWeaponUI ) return;

		const tRect safeRect = fUser( )->fComputeViewportRect( );
		const tVec2f uiPosition = safeRect.fCenter( );

		mWeaponUI->fCanvas( ).fCodeObject( )->fSetPosition( tVec3f( 0.f, 0.f, 0.5f ) );
		mWeaponUI->fSetCenterPos( tVec3f( uiPosition, 0.f ), fUser( )->fComputeViewportSafeRect( ) );
	}

	void tWeaponStation::fShowHideReticle( b32 show )
	{
		if( mWeaponUI )
			return mWeaponUI->fShowHideReticle( show );
	}

	void tWeaponStation::fAdjustReticle( const Input::tGamepad *control )
	{
		if( !mWeaponUI ) return;

		const tRect safeRect = fUser( )->fComputeViewportRect( );

		if( !control )
		{
			mUIPosition = safeRect.fCenter( );	
		}
		else
		{
			sigassert( !"reticle control depricated" );
			//tVec2f stick = control->fRightStick( );
			//stick.y *= -1.0f;

			//mUIPosition += stick * Gameplay_Weapon_UISpeed;

			//tVec2f tl = safeRect.fTopLeft( ) + tVec2f(Gameplay_Weapon_UIBorder);
			//tVec2f br = safeRect.fBottomRight( );

			//br.y = safeRect.fHeight( )/2.0f - Gameplay_Weapon_UIBorderBottom;
			//br.x -= Gameplay_Weapon_UIBorder;

			//mUIPosition.x = fClamp( mUIPosition.x, tl.x, br.x );
			//mUIPosition.y = fClamp( mUIPosition.y, tl.y, br.y );
		}

		mWeaponUI->fSetCenterPos( tVec3f( mUIPosition.x, mUIPosition.y, 0.f ), fUser( )->fComputeViewportSafeRect( ) );		
	}

	//0.f no scope, 1.f fully blended into scope
	void tWeaponStation::fSetScopeBlend( f32 blend )
	{
		if( !mWeaponUI ) return;
		mWeaponUI->fSetScopeBlend( blend );
	}

	tRayf tWeaponStation::fRayThroughRetical( const tVec3f& muzzlePt ) const 
	{
		sigassert( fUser( ) );
		tRayf result = fUser( )->fComputePickingRay( mUIPosition );

		//truncate ray so that we dont pick up any objects behind the muzzle.

		tVec3f dir = result.mExtent;
		dir.fNormalize( );

		tVec3f newOrigin = muzzlePt - result.mOrigin;
		f32 t = newOrigin.fDot( dir );
		result.mOrigin += dir * t;

		return result;				
	}

	b32 tWeaponStation::fComputeRealTargetThroughRetical( const tVec3f& muzzlePt, tVec3f& worldPosOut, tEntityPtr* entOut )
	{
		if( fUser( ) )
		{
			if( mCurrentTarget )
			{
				// if we have a target already, point at it
				worldPosOut = mCurrentTarget->mEntity->fObjectToWorld( ).fXformPoint( mCurrentTarget->mLogic->fTargetOffset( ) );
				if( entOut ) *entOut = mCurrentTarget->mEntity;
				return true;
			}
			else
			{
				tWeaponReticalCastCallback rayCastcb( mUnitLogic->fOwnerEntity( ) );
				tRayf ray = fRayThroughRetical( muzzlePt );	
				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				f32 maxDim = level ? level->fMaxLevelDimension( ) : Math::cInfinity;

				if( Perf_LongRangeRC_Raycaster )
				{
					Physics::tLongRangeRaycaster caster( ray, maxDim, mLastRaycastDistance );
					mLastRaycastDistance = caster.fRayCast( *mUnitLogic->fSceneGraph( ), rayCastcb ) + Perf_LongRangeRC_Extra;
				}
				else
					mUnitLogic->fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastcb );

				if( rayCastcb.mHit.fHit( ) )
				{
					worldPosOut = ray.fEvaluate( rayCastcb.mHit.mT );
					if( entOut ) 
						entOut->fReset( rayCastcb.mHitEntity );
					return true;
				}
				else
				{
					//It doesnt make sense to be pointing at nothing.. the back drop must be missing.
					// Choose something super far away.
					worldPosOut = ray.fEvaluate( 0.75f );
					if( entOut ) entOut->fRelease( );
					return true;
				}
			}
		}

		return false;
	}

	//namespace
	//{
	//	struct base_export tFrustumEntityCallback
	//	{
	//		typedef tGrowableArray<tEntity*> tListType;
	//		mutable tListType mList;
	//		const tEntity* mIgnore;
	//		GameFlags::tTEAM mIgnoreTeam;

	//		tFrustumEntityCallback( tEntity* ignore, GameFlags::tTEAM ignoreTeam ) : mIgnore( ignore ), mIgnoreTeam( ignoreTeam ) { }
	//		
	//		inline void operator()( const Math::tFrustumf& v, tSpatialEntity* entity ) const
	//		{
	//			if( entity == mIgnore )
	//				return;

	//			if( mIgnoreTeam != GameFlags::cTEAM_NONE && entity->fQueryEnumValue( GameFlags::cENUM_TEAM, GameFlags::cTEAM_NONE ) == mIgnoreTeam )
	//				return;

	//			if( v.fContains( entity->fObjectToWorld( ).fGetTranslation( ) ) )
	//				mList.fFindOrAdd( entity );
	//		}
	//	};
	//}

	b32 tWeaponStation::fAnyBankWantsLock( u32 unitType ) const
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fInterestedInLock( (GameFlags::tUNIT_TYPE)unitType ) )
				return true;
		return false;
	}

	b32 tWeaponStation::fProcessAdvancedTargetting( )
	{
		profile( cProfilePerfWeaponUI );

		if( fPlayer( ) && mWeaponUI )
		{
			const Gfx::tCamera& worldCamera = fUser( )->fViewport( )->fLogicCamera( );
			tEntity* ignoreEntity = mUnitLogic->fOwnerEntity( );
			u32 ignoreTeam = mUnitLogic->fTeam( );

			const tVec2f& targetingSize = mWeaponUI->fTargetingBoxSize( );
			const tVec2f& autoAimSize = mWeaponUI->fAutoAimBoxSize( );
			const tVec2f& reticleSize = mWeaponUI->fReticleSize( );
			const tRect targetRect( mUIPosition - targetingSize * 0.5f, targetingSize );
			const tRect autoAimRect( mUIPosition - autoAimSize * 0.5f, autoAimSize );
			const tRect reticleRect( mUIPosition - reticleSize * 0.5f, reticleSize );
			
			//prune old targets
			for( s32 i = 0; i < (s32)mTargets.fCount( ); ++i  )
			{
				tWeaponTargetPtr& targ = mTargets[ i ];
				tVec3f screenPt3;
				b32 onScreen = fUser( )->fProjectToScreenClamped( worldCamera, targ->mEntity->fObjectToWorld( ).fGetTranslation( ), 0.f, screenPt3 );
				tVec2f screenPt = screenPt3.fXY( );
				
				if( !onScreen || !targetRect.fContains( screenPt ) )
				{
					fTargetLost( targ );
					mWeaponUI->fRemoveTarget( targ->mUID );
					//targ->mAddedToUI = false;
					mTargets.fErase( i );	
					--i;			
				}	
				else
				{
					targ->mScreenPos = screenPt;
				}
			}

			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );

			for( u32 u = 0; u < mTargetTypes.fCount( ); ++u )
			{
				const tGrowableArray<tEntityPtr>& units = level->fUnitList( mTargetTypes[ u ] );
				GameFlags::tUNIT_TYPE type = (GameFlags::tUNIT_TYPE)mTargetTypes[ u ];

				for( u32 i = 0; i < units.fCount( ); ++i )
				{
					const tEntityPtr& e = units[ i ];

					u32 team = e->fQueryEnumValue( GameFlags::cENUM_TEAM, GameFlags::cTEAM_NONE );
					if( e == ignoreEntity || team == ignoreTeam || team == GameFlags::cTEAM_NONE )
						continue;

					tVec3f screenPt3;
					b32 onScreen = fUser( )->fProjectToScreenClamped( worldCamera, e->fObjectToWorld( ).fGetTranslation( ), 0.f, screenPt3 );
					tVec2f screenPt = screenPt3.fXY( );

					if( onScreen && targetRect.fContains( screenPt ) && !fHasTarget( e.fGetRawPtr( ) ) )
					{
						u32 uID = (u32)e.fGetRawPtr( );
						mTargets.fPushBack( tWeaponTargetPtr( NEW tWeaponTarget( uID, e, e->fLogicDerived<tUnitLogic>( ), type, screenPt ) ) );

						if( mWantsLocks && mWeaponUI && fAnyBankWantsLock( type ) )
						{
							mWeaponUI->fAddTarget( uID, screenPt );  //Only target lockable types
							mTargets.fBack( )->mAddedToUI = true;
						}
					}

					//mUnitLogic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tSpheref( cb.mList[ i ]->fObjectToWorld( ).fGetTranslation( ), 10.f ), tVec4f( 1,0,0,0.5f ) );
				}
			}

			// process targets and choose a current target.
			b32 currentInReticle = false;
			if( mCurrentTarget )
			{
				if( !autoAimRect.fContains( mCurrentTarget->mScreenPos ) )
					fAimLost( mCurrentTarget );
				else
					currentInReticle = reticleRect.fContains( mCurrentTarget->mScreenPos );
			}

			//find new current target
			if( mCurrentTarget || !currentInReticle )
			{
				for( s32 i = 0; i < (s32)mTargets.fCount( ); ++i  )
				{
					tWeaponTargetPtr& targ = mTargets[ i ];

					if( reticleRect.fContains( targ->mScreenPos ) )
					{
						mCurrentTarget = targ;
						break;
					}
				}
			}

			return true;
		}

		return false;
	}

	void tWeaponStation::fProcessST( f32 dt )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fProcessST( dt );

		if( mPlayer )
		{
			b32 reloading = fReloading( );
			mPlayer->fPauseComboTimers( reloading );

			if( mWeaponUI && !Debug_Weapons_DisableScreenTargetting )
			{
				profile( cProfilePerfWeaponUI );
				const tVec2f& targetLockSize = mWeaponUI->fTargetLockSize( );
				const tRect targetLockRect( mUIPosition - targetLockSize * 0.5f, targetLockSize );

				for( s32 i = 0; i < (s32)mTargets.fCount( ); ++i )
				{
					tWeaponTargetPtr& targ = mTargets[ i ];
					if( targ->mLogic->fIsDestroyed( ) )
					{
						fTargetLost( targ );
						mWeaponUI->fRemoveTarget( targ->mUID );
						//targ->mAddedToUI = false;
						mTargets.fErase( i );
						--i;
					}
					else if( targ->mAddedToUI )
					{
						tVec2f screenPos = fUser( )->fProjectToScreen( targ->mEntity->fObjectToWorld( ).fGetTranslation( ), 0.f ).fXY( );
						mWeaponUI->fSetTargetPosition( targ->mUID, screenPos );

						//check if we should add a lock
						if( mWantsLocks && !targ->fLocked( ) && targetLockRect.fContains( screenPos ) )
						{						
							for( u32 b = 0; b < mBanks.fCount( ); ++b )
							{
								if( mBanks[ b ]->fAcquireLocks( ) && mBanks[ b ]->fInterestedInLock( targ->mType ) )
								{
									fLockMade( targ, b );
									break;
								}
							}
						}

						if( targ->fLocked( ) ) 
							mWeaponUI->fSetLockPosition( targ->mUID, screenPos );
					}
				}

				b32 pointingAtTArget = false;

				for( u32 i = 0; i < mBanks.fCount( ); ++i )
					for( u32 w = 0; w < mBanks[ i ]->fWeaponCount( ); ++w )
						if( mBanks[ i ]->fWeapon( w )->fReticalOverTarget( ) )
							pointingAtTArget = true;

				mWeaponUI->fSetReticleOverTarget( pointingAtTArget );

				tVec2f reticlePos = mUIPosition;
				if( mCurrentTarget && mStickyReticle )
					reticlePos = mCurrentTarget->mScreenPos;

				mWeaponUI->fSetReticlePos( tVec3f( reticlePos, 0 ) );
				f32 spread = 0.f;
				for( u32 i = 0; i < mBanks.fCount( ); ++i )
					for( u32 w = 0; w < mBanks[ i ]->fWeaponCount( ); ++w )
						spread = fMax( spread, mBanks[ i ]->fWeapon( w )->fCurrentSpreadPercentage( ) );
				mWeaponUI->fSetReticleSpread( spread );

				// Set information about ammo
				for( u32 i = 0; i < mBanks.fCount( ); ++i )
				{
					f32 percent = 0.0f;
					s32 count = 0;
					u32 maxAmmo = 0;
					u32 altMaxAmmo = 0;
					b32 reloading = false;
					b32 infiniteAmmo = false;

					// HACK: This assumes all the weapons in the bank are the same type
					for( u32 w = 0; w < mBanks[ i ]->fWeaponCount( ); ++w )
					{
						tWeaponPtr& weap = mBanks[ i ]->fWeapon( w );

						reloading = reloading || weap->fReloading( );

						if( reloading ) percent = weap->fReloadProgress( );

						count += weap->fCurrentAmmo( );
						maxAmmo += weap->fReloadAmmoCount( );
						infiniteAmmo = infiniteAmmo || weap->fInst( ).fUnlimitedAmmo( );
					}

					if( !reloading )
					{
						percent = f32( count ) / f32( maxAmmo );
					}
					if( infiniteAmmo )
					{
						count = -1;
					}
					mWeaponUI->fSetAmmoValues( i, percent, count, reloading, mFirstUpdate );
				}
			}

			mFirstUpdate = false;
		}
	}

	void tWeaponStation::fAimLost( const tWeaponTargetPtr& target )
	{
		if( target == mCurrentTarget )
			mCurrentTarget.fRelease( );
	}

	void tWeaponStation::fTargetLost( const tWeaponTargetPtr& target )
	{
		fAimLost( target );
		fRemoveLock( target );
	}
	
	void tWeaponStation::fLockMade( const tWeaponTargetPtr& target, u32 bankIndex )
	{
		target->mLockedForBank = bankIndex;
		if( mWeaponUI )
			mWeaponUI->fAddLock( target->mUID, target->mScreenPos );
		mBanks[ bankIndex ]->fAddLock( target );
		

		Audio::tSourcePtr src = tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( );
		if( mPlayer )
			src = mPlayer->fSoundSource( );
		sigassert( src );

		src->fSetSwitch( tGameApp::cWeaponTypeSwitchGroup, mBanks[ bankIndex ]->fWeapon( 0 )->fDesc( ).mAudioAlias );
		src->fHandleEvent( AK::EVENTS::PLAY_WEAPON_TARGETLOCK );
	}

	void tWeaponStation::fRemoveLock( const tWeaponTargetPtr& target )
	{
		if( target->fLocked( ) )
		{
			mBanks[ target->mLockedForBank ]->fRemoveLock( target );
			target->mLockedForBank = ~0;
			if( mWeaponUI )
				mWeaponUI->fRemoveLock( target->mUID );
		}
	}

	void tWeaponStation::fClearBankLocks( u32 bankIndex )
	{
		for( u32 i = 0; i < mTargets.fCount( ); ++i )
		{
			if( mTargets[ i ]->mLockedForBank == bankIndex )
				fRemoveLock( mTargets[ i ] );
		}
	}

	b32 tWeaponStation::fHasTarget( const tEntity* e ) const
	{
		for( u32 i = 0; i < mTargets.fCount( ); ++i )
			if( mTargets[ i ]->mEntity == e )
				return true;
		return false;
	}

	void tWeaponStation::fProcessMT( f32 dt )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fProcessMT( dt );
	}

	void tWeaponStation::fSetAcquireTargets( b32 enable )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fSetAcquireTargets( enable );
	}

	b32 tWeaponStation::fFire( )
	{
		b32 result = false;
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fFire( ) )
				result = true;
		return result;
	}
	void tWeaponStation::fEndFire( )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fEndFire( );
	}
	void tWeaponStation::fReload( )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fReload( );
	}
	void tWeaponStation::fReloadAfterTimer( )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fReloadAfterTimer( );
	}
	void tWeaponStation::fAccumulateFireEvents( tFireEventList& list )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fAccumulateFireEvents( list );
	}
	void tWeaponStation::fPitchTowardsDesiredAngle( f32 dt )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fPitchTowardsDesiredAngle( dt );
	}
	void tWeaponStation::fSpinUp( b32 up )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fSpinUp( up );
	}
	void tWeaponStation::fEnable( b32 enable )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fEnable( enable );
	}
	b32 tWeaponStation::fFiring( ) const
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fFiring( ) )
				return true;
		return false;
	}
	b32 tWeaponStation::fCanFire( ) const
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fCanFire( ) )
				return true;
		return false;
	}
	b32 tWeaponStation::fShouldFire( ) const
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fShouldFire( ) )
				return true;
		return false;
	}
	b32 tWeaponStation::fNeedsReload( ) const
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fNeedsReload( ) )
				return true;
		return false;
	}
	b32 tWeaponStation::fIsContinuousFire( ) const
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fIsContinuousFire( ) )
				return true;
		return false;
	}
	b32 tWeaponStation::fShellCaming( ) const
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fShellCaming( ) )
				return true;
		return false;
	}
	b32 tWeaponStation::fShouldAcquire( ) const
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fShouldAcquire( ) )
				return true;
		return false;
	}
	b32 tWeaponStation::fHasTarget( ) const
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fHasTarget( ) )
				return true;
		return false;
	}
	b32 tWeaponStation::fReloading( ) const
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			if( mBanks[ i ]->fReloading( ) )
				return true;
		return false;
	}
	void tWeaponStation::fSetAIFireOverride( b32 fire )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fSetAIFireOverride( fire );
	}
	void tWeaponStation::fProcessUserInput( )
	{
		for( u32 i = 0; i < mBanks.fCount( ); ++i )
			mBanks[ i ]->fProcessUserInput( );

		if( mWeaponUI && mToggleCameraButton != ~0 )
		{
			u32 cameraButton = mToggleCameraButton;
			//if( mPlayer->fProfile( )->fSouthPaw( tUserProfile::cProfileVehicles ) )
			//	cameraButton = ( cameraButton == GameFlags::cGAME_CONTROLS_GAME_RIGHT_THUMB ) ? GameFlags::cGAME_CONTROLS_GAME_LEFT_THUMB : GameFlags::cGAME_CONTROLS_GAME_RIGHT_THUMB;
			
			if( mPlayer->fGameController( )->fButtonDown( tUserProfile::cProfileVehicles, cameraButton ) )
			{
				if( mWeaponUI->fCurrentScreenEffect( ) == Gui::tWeaponUI::cScreenEffectNormal )
					mWeaponUI->fActivateScreenEffect( Gui::tWeaponUI::cScreenEffectSpecial );
				else
					mWeaponUI->fActivateScreenEffect( Gui::tWeaponUI::cScreenEffectNormal );
			}
		}
	}
	void tWeaponStation::fApplyRumbleEvent( const tFireEvent& fireEvent, const tPlayer& player )
	{
		u32 weapon = fireEvent.mWeapon->fDesc( ).mWeaponDescIndex;
		const tDataTable& datatable = tGameApp::fInstance( ).fWeaponsTable( ).fTable( );
		Input::tRumbleEvent event;
		event.mIntensity	= datatable.fIndexByRowCol<f32>( weapon, cWeaponTableColumnRumbleMag );
		event.mVibeMix		= datatable.fIndexByRowCol<f32>( weapon, cWeaponTableColumnRumbleMix );
		event.mDuration		= datatable.fIndexByRowCol<f32>( weapon, cWeaponTableColumnRumbleTime );
		event.mDecay		= datatable.fIndexByRowCol<f32>( weapon, cWeaponTableColumnRumbleDecay );

		f32 camShake			= datatable.fIndexByRowCol<f32>( weapon, cWeaponTableColumnCameraShakeMag );
		f32 camShakeTime		= datatable.fIndexByRowCol<f32>( weapon, cWeaponTableColumnCameraShakeTime );

		if( fireEvent.mWeapon->fInst( ).mOverCharged )
		{
			event.mIntensity *= Gameplay_Weapon_OverChargeRumbleMult;
			camShake *= Gameplay_Weapon_ScreenShakeMult;
		}

		if( Gameplay_Weapon_EnableRumble ) 
			player.fUser( )->fRawGamepad( ).fRumble( ).fAddEvent( event );
		player.fCameraStackTop( )->fBeginCameraShake( tVec2f( camShake ), camShakeTime );
	}

	f32 tWeaponStation::fGetGroundHeight( f32 headingAngle, f32 distZeroTo1 ) const
	{
		if( mGroundHeightsFar.fCount( ) == 0 )
			return 0.f;
		else
		{
			sigassert( mGroundHeightsFar.fCount( ) == mGroundHeightsNear.fCount( ) && "tWeaponStation ground sample arrays not in sync!" );
			u32 segments = mGroundHeightsFar.fCount( ) - 1;

			// rectify angle and compute zero to one through height samples
			headingAngle = mGroundHeightStartAngle + fShortestWayAround( mGroundHeightStartAngle, headingAngle );
			f32 delta = headingAngle - mGroundHeightStartAngle;
			f32 zTo1 = delta / mGroundHeightAngleRange;
			zTo1 = fClamp( zTo1, 0.f, 1.f );
			//log_line( 0, "Heading: " << headingAngle << " start: " << mGroundHeightStartAngle << " Angle: " << zTo1 << " dist: " << distZeroTo1 );

			// interpolate between two height values, so it doesnt stair step
			f32 step = mGroundHeightAngleRange / segments;
			u32 index = u32(segments * zTo1);
			index = fMin( index, segments - 1 );
			f32 graphLerp = (delta - index * step) / step;

			//log_line( 0, "Index: " << index << " gLerp: " << graphLerp );
			f32 nearH = fLerp( mGroundHeightsNear[ index ], mGroundHeightsNear[ index + 1 ], graphLerp );
			f32 farH = fLerp( mGroundHeightsFar[ index ], mGroundHeightsFar[ index + 1 ], graphLerp );

			f32 distance = fLerp( nearH, farH, distZeroTo1 );
			return distance;
		}
	}

	void tWeaponStation::fSetGroundSamples( f32 start, f32 range, tGrowableArray<f32>& samplesNear, tGrowableArray<f32>& samplesFar )
	{
		mGroundHeightStartAngle = start;
		mGroundHeightAngleRange = range;
		samplesNear.fSwap( mGroundHeightsNear );
		samplesFar.fSwap( mGroundHeightsFar );

		while( mGroundHeightStartAngle <= 0.f )
			mGroundHeightStartAngle += c2Pi;
	}
}

namespace Sig
{
	void tWeaponStation::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tWeaponBank, Sqrat::DefaultAllocator<tWeaponBank>> classDesc( vm.fSq( ) );
			classDesc
				.Func(_SC("AddWeapon"),			&tWeaponBank::fAddWeapon)
				.Func(_SC("Weapon"),			&tWeaponBank::fWeaponRawPtr)
				.Func(_SC("Reload"),			&tWeaponBank::fReload)
				.Func(_SC("ReloadAfterTimer"),	&tWeaponBank::fReloadAfterTimer)
				.Prop(_SC("Reloading"),			&tWeaponBank::fReloading)
				.Prop(_SC("IsContinuousFire"),	&tWeaponBank::fIsContinuousFire)
				.Prop(_SC("FireMode"),			&tWeaponBank::fFireMode, &tWeaponBank::fSetFireMode)
				.Prop(_SC("TriggerButton"),		&tWeaponBank::fTriggerButton, &tWeaponBank::fSetTriggerButton)
				.Prop(_SC("Enabled"),			&tWeaponBank::fEnabled, &tWeaponBank::fEnable)
				.Prop(_SC("AIFireOverride"),	&tWeaponBank::fAIFireOverride, &tWeaponBank::fSetAIFireOverride)
				;
			vm.fRootTable( ).Bind( _SC("WeaponBank"), classDesc );
		}
		{
			Sqrat::Class<tWeaponStation, Sqrat::DefaultAllocator<tWeaponStation>> classDesc( vm.fSq( ) );
			classDesc
				.Func(_SC("Bank"),				&tWeaponStation::fBankRawPtr)
				.Func(_SC("Reload"),			&tWeaponStation::fReload)
				.Func(_SC("SpinUp"),			&tWeaponStation::fSpinUp)
				.Prop(_SC("IsContinuousFire"),	&tWeaponStation::fIsContinuousFire)
				.Prop(_SC("IsContinuousFire"),	&tWeaponStation::fIsContinuousFire)
				.Var(_SC("ToggleCameraButton"),	&tWeaponStation::mToggleCameraButton)
				.Func(_SC("ShowHideReticle"),	&tWeaponStation::fShowHideReticle)
				.Prop(_SC("WeaponUI"),			&tWeaponStation::fGetUIScript)
				;
			vm.fRootTable( ).Bind( _SC("WeaponStation"), classDesc );
		}

		if( Gameplay_Weapon_ApplyTable ) log_warning( 0, "Performance warning: weapons applying table." );
	}
}

