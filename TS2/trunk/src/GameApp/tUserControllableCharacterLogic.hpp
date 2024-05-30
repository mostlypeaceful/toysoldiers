#ifndef __tUserControllableCharacterLogic__
#define __tUserControllableCharacterLogic__
#include "tCharacterLogic.hpp"
#include "tTurretCameraMovement.hpp"
#include "tHoverTimer.hpp"

namespace Sig
{
	namespace IK { class tCharacterLegTargets; }

	class tVehicleLogic;

	class tUserControllableCharacterLogic : public tCharacterLogic
	{
		define_dynamic_cast( tUserControllableCharacterLogic, tCharacterLogic );
	public:
		tUserControllableCharacterLogic( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fActST( f32 dt );
		virtual void fPhysicsMT( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fThinkST( f32 dt );
		virtual void fCoRenderMT( f32 dt );
		virtual b32  fHandleLogicEvent( const Logic::tEvent& e );

		virtual Gui::tRadialMenuPtr fCreateSelectionRadialMenu( tPlayer& player );

		// user control stuff
		virtual b32 fCanBeUsed( ) const;
		virtual b32 fTryToUse( tPlayer* player ); // returns true if can use
		virtual b32 fEndUse( tPlayer& player ); //return true if totally vacated
		b32 fEndUseScript( );

		void fActivateWeapons( b32 activate );
		void fSetDoAdvancedMovement( b32 enabled ) { mDoAdvancedMovement = enabled; }
		void fComputeUserInput( f32 dt, const Input::tGamepad& gamepad );
		void fComputeAIInput( f32 dt );
		f32 fGetSpeed( ) { return mSpeed; }

		b32  fSprinting( ) const { return mSprinting; }
		void fSetSprinting( b32 value );

		const Math::tVec3f& fGetMoveVec( ) { return mMoveVec; }

		f32 fGetAimBlendDownUp( ) const { return mAimBlendDownUp; }
		f32 fGetAimBlendMain( ) const { return mAimBlendMain.fValue( ); }
		f32 fGetAimPitchMax( ) const;

		void fRaiseGun( );
		void fSpawnGrenade( );
		void fRespawn( );

		void fSetForBarrage( tPlayer* player ) { mForBarrage = player; }

		// Extra Stuff
		Math::tVec3f mLookVec;
		Math::tVec2f mRotateVel;
		Math::tDampedFloat mDampedSpeed;

		void fFirstPersonMovement( const Math::tVec2f& moveStick, const Math::tVec2f& aimStick, f32 dt );
		void f3rdPersonMovement( const Math::tVec2f& moveStick, Math::tVec2f aimStick, f32 dt );
		void fComputeUserInputExtra( const Math::tVec2f& moveStick, const Math::tVec2f& aimStick, f32 dt, const Input::tGamepad& gamepad );
		const Math::tVec3f& fGetLookDirection( ) const { return mLookVec; }

		void fFindNearestVehicleMT( );
		u32 mNearestVehicleSeatIndex;
		tEntityPtr mNearestVehicle;
		tEntityPtr mRidingVehicle;

		enum tEquip { cLeftHandEquip, cRightHandEquip, cEquipCount, cAllEquip };
		tFixedArray<tEntityPtr, cEquipCount> mEquip;
		b32		fInVehicle( ) const { return mRidingVehicle; }
		u32		fSeatIndex( ) const { return mNearestVehicleSeatIndex; }
		void	fHandleEnterExitVehicle( const Input::tGamepad& gamepad );
		void	fSwitchSeats( u32 newSeat );
		void				fSetRidingVehicle( tEntity* vehicle ) { mRidingVehicle.fReset( vehicle ); }
		const tEntityPtr&	fRidingVehicle( ) const { return mRidingVehicle; }
		const tEntityPtr&	fNearestVehicle( ) const { return mNearestVehicle; }
		tVehicleLogic*		fRidingVehicleLogic( ) const;
		Math::tMat3f&		fSeatRelativeXform( ) { return mParentRelativeXformMT; }
		void				fAcquireExitVehicleXform( Math::tMat3f xform, tVehicleLogic* vehicle );
		void				fShowEquip( u32 equip, b32 show );
		// End Extra Stuff

		IK::tCharacterLegTargets* fGetIKLegs( );

		Math::tMat3f mSpawnLocation;

		//user control stuff
		Math::tVec3f mMoveVec;
		f32 mSpeed;

		tPlayer* mForBarrage;

		tTurretCameraMovement	mCameraMovement;
		f32						mAimBlendDownUp;
		Math::tDampedFloat		mAimBlendMain;
		f32						mAimTimer;
		tRefCounterPtr< IK::tCharacterLegTargets > mIKLegs;

		b8 mDoAdvancedMovement; //if this is false, try to save some performance
		b8 mWantsJump;
		b8 mFirstUserInput;
		b8 mLookToggle;

		b8 mSprinting;
		b8 mVehicleBaseStarted;
		b8 pad1;
		b8 pad2;

		Gui::tHoverTimerPtr mExpireTimer;
		Gui::tHoverTimerPtr mWaitTimer;
		void fShowWaitTimer( );

		// these support binding the character to a vehicle, so they become a vehicle. lol jetpack.
		tFilePathPtr mVehicleBasePath;
		tVehicleLogic* mVehicleBaseLogic;
		tEntityPtr mVehicleBase;
		tEntityPtr mVehicleBaseAttachPt;
		Math::tVec3f mVehicleBaseAcc;
		Math::tMat3f mOriginalXform;

		Math::tVec3f* fGetBaseVehAccVec( ) { return &mVehicleBaseAcc; }
		void fBindToVehicle( b32 bind );

		enum tCommandoAudio
		{
			cCommandoAudioSelect,
			cCommandoAudioFire,
			cCommandoAudioDeath,
		};
		const Audio::tSourcePtr& fPlayerOrLocalSource( ) const;
		void fCommandoAudio( u32 commandoEvent );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

}

#endif//__tUserControllableCharacterLogic__
