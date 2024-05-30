#ifndef __tWeaponUI__
#define __tWeaponUI__
#include "Gui/tScriptedControl.hpp"
#include "tGamePostEffectMgr.hpp"
#include "tUser.hpp"
namespace Sig {

	struct tWeaponDesc;
	class tPlayer;

namespace Gui
{

	struct tWeaponEffectData : public tRefCounter
	{
		tWeaponEffectData( )
			: mNightVision( false )
		{ }

		tGamePostEffectsData mData;
		b32 mNightVision;

		tGamePostEffectsData* fData( ) { return &mData; }
	};

	typedef tRefCounterPtr< tWeaponEffectData > tWeaponEffectDataPtr;

	class tWeaponUI : public tScriptedControl
	{
	public:
		explicit tWeaponUI( const tWeaponDesc* desc, const tResourcePtr& scriptResource );
		~tWeaponUI( );

		void fSetViewPortIndex( u32 index );
		void fUserControl( b32 userControl, tPlayer* player );
		void fSetCenterPos( const Math::tVec3f& pos, const Math::tRect& safeRect );
		void fShowControls( b32 showAlts = false );
		void fHideControls( b32 hideAlts = false );
		void fFadeOutControls( b32 alts = false );

		// Hits
		void fGetHit( const tUserPtr& drivingUser, tEntity* attackerEntity );
		f32  fGetAngleToAttacker( );
		
		// Ammo Tracking
		void fSetAmmoValues( u32 weaponIndex, f32 percent, s32 count, b32 reloading, b32 forceRefresh );
		void fAddAmmoCounter( tFilePathPtr iconPath, tFilePathPtr tickMarkPath, u32 maxAmmo );

		// Default Reticle
		void fSetReticleOverTarget( b32 over );
		void fSetReticlePos( const Math::tVec3f& pos );
		void fShowHideReticle( b32 show );
		
		// Gun Weapons (Zoom and scope)
		void fSetScopeBlend( f32 blend );

		// Shell cam
		void fShowShellCam( b32 show, tEntity* projectileEntity, tPlayer* player );
		void fOnSpecialScreenEffect( b32 show );

		// Expanding Reticle
		void fSetReticleSpread( f32 spread );

		// Advanced Targeting
		void fAddTarget( u32 uID, const Math::tVec2f& pos );
		void fRemoveTarget( u32 uID );
		void fSetTargetPosition( u32 uID, const Math::tVec2f& pos );
		void fAddLock( u32 uID, const Math::tVec2f& pos );
		void fRemoveLock( u32 uID );
		void fSetLockPosition( u32 uID, const Math::tVec2f& pos );
		void fClearTargets( );

		//DEBUG TESTING
		void fSetVehicleStats( f32 speed, f32 rpm, f32 load );
		Sqrat::Function fSetWheelStats( ) const;

		Math::tVec2f fTargetingBoxSize( ) const;
		Math::tVec2f fAutoAimBoxSize( ) const;
		Math::tVec2f fReticleSize( ) const;
		Math::tVec2f fTargetLockSize( ) const;

		u32  fCurrentScreenEffect( ) const { return mScreenEffectActive; }
		void fActivateScreenEffect( u32 index );

		b32 fUsingNightVision( ) const;

		enum tScreenEffects
		{
			cScreenEffectNormal,
			cScreenEffectScope,
			cScreenEffectSpecial,
			cScreenEffectShellCam,
			cScreenEffectCount
		};

		enum tControls
		{
			cMainControls = false,
			cAltControls = true
		};

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:

		tFixedArray<tWeaponEffectDataPtr, cScreenEffectCount> mScreenEffects;
		tWeaponEffectData* fScreenEffectsForScript( u32 type ) { if( !mScreenEffects[ type ] ) mScreenEffects[ type ].fReset( NEW tWeaponEffectData( ) ); return mScreenEffects[ type ].fGetRawPtr( ); }

		u32 mViewPortIndex;
		u32 mScreenEffectActive;

		void fShowShellCamOverlay( b32 show, tEntity* projectileEntity, tPlayer* player );
		void fPlaySound( const tStringPtr& event );

		tUserPtr mDrivingUser;
		tPlayer* mDrivingPlayer;
		tEntity* mAttackerEntity;
		const tWeaponDesc* mDesc;
	};

	typedef tRefCounterPtr< tWeaponUI > tWeaponUIPtr;

}}

#endif//__tWeaponUI__
