#ifndef __tLightningWeapon__
#define __tLightningWeapon__
#include "tGunWeapon.hpp"

namespace Sig
{
	class tLightningEntity;
	class tAreaDamageLogic;
	class tPathEntity;

	class tLightningWeapon : public tGunWeapon
	{
	public:
		explicit tLightningWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fProcessST( f32 dt );
		virtual void fProcessMT( f32 dt );
		virtual void fSpawnFireEffect( tProjectileLogic* projectile, u32 anchorIndex );

		virtual void fAudioEnd( );

		tRefCounterPtr<tLightningEntity> mLightning;

		tEntityPtr mExplosion;
		FX::tFxSystemsArray mExplosionArray;
	};


	class tOrbitalLaserWeapon : public tGunWeapon
	{
	public:
		tOrbitalLaserWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fProcessST( f32 dt );

		virtual void fBeginAreaEffect( u32 anchorIndex );
		virtual void fEndAreaEffect( u32 anchorIndex );


		static const u32 cLaserStartSound;
		static const u32 cLaserStopSound;

		static const u32 cLaserBurnStartSound;
		static const u32 cLaserBurnStopSound;

		static u32 fNumFracs( );

	private:
		b32 mIsFiring;

		tRefCounterPtr<tLightningEntity> mLightning, mLightning2Space;

		// ground burning
		tEntityPtr				mBurn;
		FX::tFxSystemsArray		mFx;
		tAreaDamageLogic*		mDamageLogic;
		Audio::tSourcePtr		mBurnSound;

		f32 mPathDist;
		tPathEntity*			mPath;
	};

}

#endif//__tLightningWeapon__
