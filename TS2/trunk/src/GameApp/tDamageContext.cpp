#include "GameAppPch.hpp"
#include "tDamageContext.hpp"
#include "tUnitLogic.hpp"
#include "tWeaponStation.hpp"

using namespace Sig::Math;

namespace Sig
{

	tDamageID::tDamageID( tUnitLogic* unit, tPlayer* player, u32 team )
		: mPlayer( player )
		, mDesc( NULL )
		, mUnitLogic( unit )
		, mUnit( unit ? unit->fOwnerEntity( ) : NULL )
		, mTeam( player ? player->fTeam( ) : team )
		, mOverCharged( false )
		, mUserControlled( false )
		, mNightVision( false )
		, mSpeedBonus( 0.f )
		, mUserDamageMultiplier( 1.f )
	{ }

	void tDamageContext::fDestroyDamage( tUnitLogic* toKill )
	{
		sigassert( toKill );
		fSetExplicit( fMax( toKill->fCurrentHitPoints( ), 0.f ) * 100.f );
		fSetAttacker( tDamageID( GameFlags::cTEAM_NONE ), GameFlags::cDAMAGE_TYPE_NONE );
	}

	u32 tDamageContext::fPersistentDamage( ) const 
	{ 
		return fWeaponDesc( ) ? fWeaponDesc( )->mPersistentDamageType : ~0; 
	}

	GameFlags::tDAMAGE_TYPE tDamageContext::fEffectDamageType( ) const 
	{ 
		return fWeaponDesc( ) ? fWeaponDesc( )->mEffectDamageType : GameFlags::cDAMAGE_TYPE_NONE; 
	}


	f32 tDamageContext::fDamageMulitplier( u32 unitType ) const
	{
		f32 mult = mDamageMultiplier;

		const tWeaponDesc* desc = fWeaponDesc( );
		if( desc )
		{
			if( mDamageID.mUserControlled || desc->mBarrageWeapon )
				mult *= mDamageID.mUserDamageMultiplier;

			if( mDamageID.mOverCharged ) 
				mult *= desc->mOverChargeMultiplier;

			if( unitType < desc->mDamageMod.fCount( ) )
				mult *= desc->mDamageMod[ unitType ];

			if( mDamageType == GameFlags::cDAMAGE_TYPE_BULLET )
				mult *= desc->mDamageModDirectHit;
		}

		return mult;
	}

	f32 tDamageContext::fPointsToRemove( u32 attackedUnitType ) const
	{
		f32 points = mExplicitPoints;

		if( mExplicitPoints == 0.f )
		{
			if( points == 0.0f && fWeaponDesc( ) )
			{
				if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
					points = fWeaponDesc( )->mHitPointsVersus;
				else if( mDamageID.mUserControlled )
					points = fWeaponDesc( )->mHitPoints[ GameFlags::cDIFFICULTY_NORMAL ];
				else
					points = fWeaponDesc( )->mHitPoints[ tGameApp::fInstance( ).fDifficulty( ) ];

				points *= fDamageMulitplier( attackedUnitType );
			}
		}

		return points;
	}


}

