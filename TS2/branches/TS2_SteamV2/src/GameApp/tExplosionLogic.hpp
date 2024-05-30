#ifndef __tExplosionLogic__
#define __tExplosionLogic__
#include "tProximity.hpp"
#include "tPlayer.hpp"
#include "Audio/tSource.hpp"
#include "tDamageContext.hpp"

namespace Sig
{
	class tWeapon;
	class tLightEffectLogic;
	struct tWeaponDesc;

	class tExplosionLogic : public tLogic
	{
		define_dynamic_cast( tExplosionLogic, tLogic );
	public:
		tExplosionLogic( );
		virtual ~tExplosionLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fActST( f32 dt );
		virtual void fThinkST( f32 dt );
		virtual void fCoRenderMT( f32 dt );
		virtual b32  fReadyForDeletion( );

		void fReuseLight( tLightEffectLogic* light ) { mReuseLight = light; }
		void fSpawnLight( const Math::tMat3f& worldXForm );
	public: // accessors
		f32			fFullSize( ) { return mFullSize; }
		void		fSetFullSize( f32 size ) { mFullSize = size; }
		f32			fGrowRate( ) { return mGrowRate; }
		void		fSetGrowRate( f32 growRate ) { mGrowRate = growRate; }
		f32			fFalloff( ) { return mFalloff; } //percentage of damage to do at edge of explosion
		void		fSetFalloff( f32 falloff ) { mFalloff = falloff; }
		f32			fHitPoints( ) const { return mExplicitHitPoints; }
		void		fSetHitPoints( f32 hitpoints ) { mExplicitHitPoints = hitpoints; }
		void		fSetCountry( u32 country ) { mCountry = country; }
		void		fSetFiredBy( const tDamageID& id ) { mDamageID = id; }
		//void		fSetEffectiveCone( const Math::tVec3f& worldAxis, f32 angle ) { mConeWorldAxis = worldAxis; mConeAngle = angle; }
		void		fSetDamageMod( f32 mod ) { mDamageMod = mod; }
		void		fSetLightValues( f32 size, f32 time, const Math::tVec4f& color ) { mLightSize = size; mLightExpandTime = 0.f; mLightCollapseTime = time; mLightColor = color; }

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		tProximity mProximity;

	private:
		tDamageID mDamageID;
		f32	mSize;
		f32	mFullSize;
		f32	mGrowRate;
		f32 mFalloff;
		//Math::tVec3f mConeWorldAxis;
		//f32 mConeAngle;
		f32 mExplicitHitPoints;
		f32 mDamageMod;
		u32 mCountry; //will translate to a team, so as not to damage your own team

		b32 mLightSpawned;

		f32 mLightSize;
		f32 mLightExpandTime;
		f32 mLightCollapseTime;
		f32 mLightHeight;
		Math::tVec4f mLightColor;

		tLightEffectLogic* mReuseLight;

		void fConfigureAudio( );
	};

}

#endif//__tExplosionLogic__