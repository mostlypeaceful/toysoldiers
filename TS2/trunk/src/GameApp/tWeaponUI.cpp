#include "GameAppPch.hpp"
#include "tWeaponUI.hpp"
#include "tGameApp.hpp"
#include "tWeapon.hpp"
#include "Audio\tSource.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateWeaponUI( "CanvasCreateWeaponUI" ); }

	tWeaponUI::tWeaponUI( const tWeaponDesc* desc, const tResourcePtr& scriptResource )
		: tScriptedControl( scriptResource )
		, mScreenEffectActive( ~0 )
		, mViewPortIndex( ~0 )
		, mDesc( desc )
		, mDrivingPlayer( NULL )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateWeaponUI, this );
		log_assert( !mCanvas.fIsNull( ), "Canvas couldn't be created from script: " << scriptResource->fGetPath( ) );
	}

	tWeaponUI::~tWeaponUI( )
	{
	}

	Math::tVec2f tWeaponUI::fTargetingBoxSize() const
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "TargetingBoxSize" ).Evaluate<Math::tVec2f>( );
	}

	Math::tVec2f tWeaponUI::fAutoAimBoxSize() const
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "AutoAimBoxSize" ).Evaluate<Math::tVec2f>( );
	}

	Math::tVec2f tWeaponUI::fReticleSize() const
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "ReticleSize" ).Evaluate<Math::tVec2f>( );
	}

	Math::tVec2f tWeaponUI::fTargetLockSize() const
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "TargetLockSize" ).Evaluate<Math::tVec2f>( );
	}

	void tWeaponUI::fUserControl( b32 control, tPlayer* player )
	{
		if( control ) 
		{
			mDrivingPlayer = player;
			fActivateScreenEffect( cScreenEffectNormal );
		}
		else 
		{
			fActivateScreenEffect( ~0 );
			mDrivingPlayer = NULL;
		}

		Sqrat::Function( mCanvas.fScriptObject( ), "UserControl" ).Execute( control, player );
	}
	void tWeaponUI::fSetReticlePos( const Math::tVec3f& pos )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetReticlePos" ).Execute( pos );
	}
	void tWeaponUI::fShowHideReticle( b32 show )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "ShowHideReticle" ).Execute( show );
	}
	void tWeaponUI::fSetReticleOverTarget( b32 over )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetReticleOverTarget" ).Execute( over ? true : false );
	}
	void tWeaponUI::fSetReticleSpread( f32 spread )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetReticleSpread" ).Execute( spread );
	}
	void tWeaponUI::fSetCenterPos( const Math::tVec3f& pos, const Math::tRect& safeRect )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetCenterPos" ).Execute( pos, safeRect );
	}
	void tWeaponUI::fAddTarget( u32 uID, const Math::tVec2f& pos )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "AddTarget" ).Execute( uID, Math::tVec3f( pos, 0 ) );
	}
	void tWeaponUI::fRemoveTarget( u32 uID )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "RemoveTarget" ).Execute( uID );
	}
	void tWeaponUI::fSetTargetPosition( u32 uID, const Math::tVec2f& pos )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetTargetPosition" ).Execute( uID, Math::tVec3f( pos, 0 ) );
	}
	void tWeaponUI::fAddLock( u32 uID, const Math::tVec2f& pos )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "AddLock" ).Execute( uID, Math::tVec3f( pos, 0 ) );
	}
	void tWeaponUI::fRemoveLock( u32 uID )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "RemoveLock" ).Execute( uID );
	}
	void tWeaponUI::fSetLockPosition( u32 uID, const Math::tVec2f& pos )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetLockPosition" ).Execute( uID, Math::tVec3f( pos, 0 ) );
	}
	void tWeaponUI::fClearTargets( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "ClearTargets" ).Execute( );
	}
	void tWeaponUI::fSetVehicleStats( f32 speed, f32 rpm, f32 load )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetVehicleStats" ).Execute( speed, rpm, load );
	}
	Sqrat::Function tWeaponUI::fSetWheelStats(  ) const
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "SetWheelStats" );
	}
	void tWeaponUI::fSetAmmoValues( u32 weaponIndex, f32 percent, s32 count, b32 reloading, b32 forceRefresh )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetAmmoValues" ).Execute( weaponIndex, percent, count, reloading, forceRefresh );
	}

	void tWeaponUI::fAddAmmoCounter( tFilePathPtr iconPath, tFilePathPtr tickMarkPath, u32 maxAmmo )
	{
		Sqrat::Function f( mCanvas.fScriptObject( ), "AddAmmoCounter" );
		
		b32 iconPathValid = ( iconPath.fExists( ) );
		b32 tickMarkPathValid = ( tickMarkPath.fExists( ) );
		if( iconPathValid && tickMarkPathValid )
		{
			f.Execute( iconPath.fCStr( ), tickMarkPath.fCStr( ), maxAmmo );
		}
		else if( !iconPathValid && !tickMarkPathValid )
		{
			f.Execute( Sqrat::Object( ), Sqrat::Object( ), maxAmmo );
		}
		else if( !iconPathValid && tickMarkPathValid )
		{
			f.Execute( Sqrat::Object( ), tickMarkPath.fCStr( ), maxAmmo );
		}
		else if( iconPathValid && !tickMarkPathValid )
		{
			f.Execute( iconPath.fCStr( ), Sqrat::Object( ), maxAmmo );
		}
	}

	void tWeaponUI::fSetScopeBlend( f32 blend )
	{
		if( blend > 0.5f ) 
			fActivateScreenEffect( cScreenEffectScope );
		else 
			fActivateScreenEffect( cScreenEffectNormal );

		Sqrat::Function( mCanvas.fScriptObject( ), "SetScopeBlend" ).Execute( blend );
	}

	void tWeaponUI::fSetViewPortIndex( u32 index )
	{
		mViewPortIndex = index;
		Sqrat::Function( mCanvas.fScriptObject( ), "SetViewportIndex" ).Execute( mViewPortIndex );
	}

	void tWeaponUI::fActivateScreenEffect( u32 index )
	{
		if( mViewPortIndex != ~0 )
		{
			if( index != mScreenEffectActive )
			{
				if( mScreenEffectActive != ~0 )
				{
					switch( mScreenEffectActive )
					{
					case cScreenEffectShellCam: fPlaySound( mDesc->mShellCamStopAudioID ); break;
					case cScreenEffectNormal: fPlaySound( mDesc->mNormalStopAudioID ); break;
					case cScreenEffectScope: fPlaySound( mDesc->mScopeStopAudioID ); break;
					case cScreenEffectSpecial: fPlaySound( mDesc->mSpecialStopAudioID ); break;
					}

					if( mScreenEffectActive < mScreenEffects.fCount( ) && mScreenEffects[ mScreenEffectActive ] )
						tGameApp::fInstance( ).fPostEffectsManager( )->fPopEffectsData( mViewPortIndex );
					mScreenEffectActive = ~0;
				}

				mScreenEffectActive = index;

				if( mScreenEffectActive < mScreenEffects.fCount( ) && mScreenEffects[ mScreenEffectActive ] )
				{
					tGameApp::fInstance( ).fPostEffectsManager( )->fPushEffectsData( mScreenEffects[ mScreenEffectActive ]->mData, mViewPortIndex );
				}

				switch( mScreenEffectActive )
				{
				case cScreenEffectShellCam: fPlaySound( mDesc->mShellCamStartAudioID ); break;
				case cScreenEffectNormal: fPlaySound( mDesc->mNormalStartAudioID ); break;
				case cScreenEffectScope: fPlaySound( mDesc->mScopeStartAudioID ); break;
				case cScreenEffectSpecial: fPlaySound( mDesc->mSpecialStartAudioID ); break;
				}
			}
		}
		else
			log_warning( 0, "Called fActivateScreenEffect with no viewport. Did you call fEndUserControl before fBeginUserControl?" );
	}

	void tWeaponUI::fPlaySound( const tStringPtr& event )
	{
		if( event.fExists( ) )
		{
			Audio::tSourcePtr audioSource;

			if( mDrivingPlayer )
				audioSource = mDrivingPlayer->fSoundSource( );
			else
				audioSource = tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( );

			audioSource->fSetSwitch( tGameApp::cWeaponTypeSwitchGroup, mDesc->mAudioAlias );
			audioSource->fHandleEvent( event );
		}
	}

	void tWeaponUI::fShowControls( b32 showAlts /*= false */ )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "ShowControls" ).Execute( showAlts );
	}

	void tWeaponUI::fHideControls( b32 hideAlts /*= false */ )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "HideControls" ).Execute( hideAlts );
	}

	void tWeaponUI::fFadeOutControls( b32 alts /*= false */ )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "FadeOutControls" ).Execute( alts );
	}

	void tWeaponUI::fShowShellCam( b32 show, tEntity* projectileEntity, tPlayer* player )
	{
		if( show )
		{
			fShowControls( cAltControls );
			fHideControls( cMainControls );
			fActivateScreenEffect( cScreenEffectShellCam );
		}
		else
		{
			fFadeOutControls( cAltControls );
			fActivateScreenEffect( cScreenEffectNormal );
		}

		fShowShellCamOverlay( show, projectileEntity, player );
	}

	void tWeaponUI::fShowShellCamOverlay( b32 show, tEntity* projectileEntity, tPlayer* player )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "ShowShellCamOverlay" ).Execute( show, projectileEntity, player );
	}

	void tWeaponUI::fOnSpecialScreenEffect( b32 show )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "OnSpecialScreenEffect" ).Execute( show );
	}

	void tWeaponUI::fGetHit( const tUserPtr& drivingUser, tEntity* attackerEntity )
	{
		mDrivingUser = drivingUser;
		mDrivingPlayer = tGameApp::fInstance( ).fGetPlayerByUser( mDrivingUser.fGetRawPtr( ) );
		mAttackerEntity = attackerEntity;
		Sqrat::Function( mCanvas.fScriptObject( ), "GetHit" ).Execute( );
	}

	Sig::f32 tWeaponUI::fGetAngleToAttacker( )
	{
		if( mDrivingUser.fNull( ) && !mAttackerEntity )
			return 0.0f;

		const Gfx::tViewportPtr& vp = mDrivingUser->fViewport( );
		const Gfx::tCamera& camera = vp->fRenderCamera( );
		Math::tVec3f objDir = mAttackerEntity->fObjectToWorld( ).fGetTranslation( ) - camera.fGetTripod( ).mEye;
		objDir.fProjectToXZAndNormalize( );
		Math::tVec3f camDir = camera.fGetTripod( ).mLookAt - camera.fGetTripod( ).mEye;
		camDir.fProjectToXZAndNormalize( );
		const f32 objAngle = Math::fAtan2( objDir.x, objDir.z );
		const f32 camAngle = Math::fAtan2( camDir.x, camDir.z );
		return camAngle - objAngle - Math::cPiOver2;
	}

	b32 tWeaponUI::fUsingNightVision( ) const
	{
		if( mScreenEffectActive != ~0 && mScreenEffects[ mScreenEffectActive ] && mScreenEffects[ mScreenEffectActive ]->mNightVision )
			return true;
		else
			return false;
	}

}}


namespace Sig { namespace Gui
{
	void tWeaponUI::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tWeaponEffectData, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				.Prop(_SC("Data"),			&tWeaponEffectData::fData)
				.Var(_SC("NightVision"),	&tWeaponEffectData::mNightVision)
				;
			vm.fNamespace(_SC("Gui")).Bind( _SC("WeaponEffectData"), classDesc );
		}
		{
			Sqrat::Class<tWeaponUI, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				.Func(_SC("ScreenEffects"), &tWeaponUI::fScreenEffectsForScript)
				.Func(_SC("GetAngleToAttacker"), &tWeaponUI::fGetAngleToAttacker)
				;
			vm.fNamespace(_SC("Gui")).Bind( _SC("WeaponUI"), classDesc );
		}

		vm.fConstTable( ).Const(_SC("WEAPON_SCREEN_EFFECT_NORMAL"), cScreenEffectNormal);
		vm.fConstTable( ).Const(_SC("WEAPON_SCREEN_EFFECT_SCOPE"), cScreenEffectScope);
		vm.fConstTable( ).Const(_SC("WEAPON_SCREEN_EFFECT_SPECIAL"), cScreenEffectSpecial);
		vm.fConstTable( ).Const(_SC("WEAPON_SCREEN_EFFECT_SHELLCAM"), cScreenEffectShellCam);
	}
}}

