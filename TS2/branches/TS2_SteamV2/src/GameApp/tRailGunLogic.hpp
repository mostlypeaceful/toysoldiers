#ifndef __tRailGunLogic__
#define __tRailGunLogic__
#include "tWheeledVehicleLogic.hpp"

namespace Sig
{
	namespace RailGunState
	{
		enum tState
		{
			cDormant,
			cComingOut,
			cFiringRandomly,
			cGoingIn,
			cChargingGoal,
			cSpawnTransports,
			cDead
		};
	}

	class tRailGunLogic : public tWheeledVehicleLogic
	{
		define_dynamic_cast( tRailGunLogic, tWheeledVehicleLogic );
	public:
		tRailGunLogic( );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	public:
		virtual void fActST( f32 dt );

	private:

		// States

		void fEnterStateDormant( );
		void fUpdateStateDormant( f32 dt );

		void fEnterStateComingOut( );
		void fUpdateStateComingOut( f32 dt );

		void fEnterStateFiringRandomly( );
		void fUpdateStateFiringRandomly( f32 dt );

		void fEnterStateGoingIn( );
		void fUpdateStateGoingIn( f32 dt );

		void fEnterStateChargingGoal( );
		void fUpdateStateChargingGoal( f32 dt );

		void fEnterStateSpawnTransports( );
		void fUpdateStateSpawnTransports( f32 dt );

		void fEnterStateDead( );
		void fUpdateStateDead( f32 dt );
	private:

		b32 fHandleLogicEvent( const Logic::tEvent& e );
	private:

		// Current state of the railgun.
		RailGunState::tState mCurrentState;

		// Timer to stay in the current state.
		f32 mStateTimer;

		// Speed to travel at.
		f32 mSpeed;

		// Count the number of times we've come out.
		u32 mComingOutCount;

		//  Wave list for the transports the boss will spawn
		tWaveList * mTransportWaveList;

		// Current angles for the gun.
		f32 mGunPitch;
		f32 mGunYaw;
		// Target angles for gun aiming.
		f32 mGunTargetPitch;
		f32 mGunTargetYaw;
		// Gun momentum while rotating toward target.
		f32 mGunMomentum;

		// Reload time.
		f32 mGunReloadTime;

		// Number of times to fire.
		u32 mFireCount;

		//  How often to blow the whistle while charging
		f32 mWhistleAudioTimer;

		// Flag indicating we've called the reached goal.
		b32 mReachedGoal;

	};
}

#endif//__tRailGunLogic__
