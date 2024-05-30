#ifndef __DamageContext__
#define __DamageContext__

namespace Sig
{
	class tPlayer;
	class tUnitLogic;
	struct tWeaponDesc;
	struct tWeaponInstData;
	class tWeapon;

	// This keeps alive all necessary info to give kill credit.
	struct tDamageID
	{
		tPlayer*				mPlayer;
		u32						mTeam;
		const tWeaponDesc*		mDesc;
		tUnitLogic*				mUnitLogic;
		tEntityPtr				mUnit;
		tRefCounterPtr<tWeapon> mWeapon;

		b32 mOverCharged;
		b32 mUserControlled;
		b32 mNightVision;
		f32 mSpeedBonus; //multiple of kill value
		f32 mUserDamageMultiplier;

		tDamageID( )
			: mPlayer( NULL )
			, mDesc( NULL )
			, mUnitLogic( NULL )
			, mTeam( ~0 )
			, mOverCharged( false )
			, mUserControlled( false )
			, mNightVision( false )
			, mSpeedBonus( 0.f )
			, mUserDamageMultiplier( 1.f )
		{ }

		tDamageID( u32 team )
			: mPlayer( NULL )
			, mDesc( NULL )
			, mUnitLogic( NULL )
			, mTeam( team)
			, mOverCharged( false )
			, mUserControlled( false )
			, mNightVision( false )
			, mSpeedBonus( 0.f )
			, mUserDamageMultiplier( 1.f )
		{ }

		explicit tDamageID( tUnitLogic* unit, tPlayer* player = NULL, u32 team = ~0 );
	};


	struct tDamageContext : public tRefCounter
	{
		tDamageContext( )
			: mContext( ~0 )
			, mDamageType( GameFlags::cDAMAGE_TYPE_NONE )
			, mDontJib( false )
			, mWorldEffectorVector( Math::tVec3f::cZeroVector )
			, mWorldPosition( Math::cInvalidFloat )
			, mMaxSize( 0.f )
			, mFalloff( 0.f )
			, mExplicitPoints( 0.f )
			, mAttackerSet( false )
			, mWeaponDataSet( false )
			, mDamageMultiplier( 1.f )
		{
		}

		tDamageContext( const tDamageContext& dc )
		{
			*this = dc;
		}

		void fOnDelete( )
		{
			mDamageID = tDamageID( );
		}

		b32  fValid( ) const { return mAttackerSet && mWeaponDataSet; }
		void fInvalidate( ) { mAttackerSet = false; mWeaponDataSet = false; }

		b32 fDontJib( ) const { return mDontJib; }
		void fSetDontJib( b32 dont ) { mDontJib = dont; }

		u32 fPersistentDamage( ) const;

		const tDamageID& fID( ) const { return mDamageID; }
		tUnitLogic* fAttacker( ) const { return mDamageID.mUnitLogic; }
		GameFlags::tTEAM fAttackingTeam( ) const { return (GameFlags::tTEAM)mDamageID.mTeam; }
		GameFlags::tDAMAGE_TYPE fDamageType( ) const { return mDamageType; }
		const tWeaponDesc* fWeaponDesc( ) const { return mDamageID.mDesc; }
		GameFlags::tDAMAGE_TYPE fEffectDamageType( ) const;
		tPlayer* fAttackerPlayer( ) const { return mDamageID.mPlayer; }

		void fSetDamageMultiplier( f32 mult ) { mDamageMultiplier = mult; }

		// Call this to get enough damage to destroy something, like yourself
		void fDestroyDamage( tUnitLogic* toKill );

		void fSetAttacker( const tDamageID& id ) 
		{ 
			mDamageID = id; 
			mAttackerSet = true; 
		}

		void fSetAttacker( const tDamageID& id, GameFlags::tDAMAGE_TYPE damageType ) 
		{ 
			mDamageID = id; 
			mDamageType = damageType; 
			mWeaponDataSet = true; 
			mAttackerSet = true; 
		}

		void fSetExplicit( f32 hitPoints )
		{
			mExplicitPoints = hitPoints;
		}

		f32 fDamageMulitplier( u32 unitType ) const;

		f32 fPointsToRemove( u32 attackedUnitType ) const;

		b32 fShouldDamage( u32 team ) const
		{
			return fAttackingTeam( ) != team || fAttackingTeam( ) == GameFlags::cTEAM_NONE;
		}

		// These are still willy nilly :(
		Math::tVec3f			mWorldPosition;
		Math::tVec3f			mWorldEffectorVector;
		f32						mMaxSize; //for explosions
		f32						mFalloff; //for explosions

	private:
		u32						mContext;

		GameFlags::tTEAM		mTeam;
		GameFlags::tDAMAGE_TYPE mDamageType;
		f32						mDamageMultiplier;
		
		tDamageID				mDamageID;

		//This value is to hackishly get damage dealt while we consider the damage look up process
		// it should always be removed from use as soon as possible
		f32						mExplicitPoints; 

		b8 mDontJib;
		b8 pad0;
		// These are use for validation
		b8 mAttackerSet;
		b8 mWeaponDataSet;
	};

	typedef tRefCounterPtr<tDamageContext> tDamageContextPtr;

	struct tDamageResult
	{
		b32	mDestroysYou;
		f32 mHealthPercentStart; //before we got hit
		f32 mHealthPercentEnd; //after we got hit
		f32 mEffect;
		Math::tVec3f mAttackerDirection;
		Math::tVec3f mSpawnInfluence;

		tDamageResult( )
			: mDestroysYou( false )
			, mHealthPercentStart( 0.f )
			, mHealthPercentEnd( 0.f )
			, mEffect( 1.f )
			, mAttackerDirection( Math::tVec3f::cZeroVector )
			, mSpawnInfluence( Math::tVec3f::cZeroVector )
		{ }
	};

}

#endif//__tDamageContext__

