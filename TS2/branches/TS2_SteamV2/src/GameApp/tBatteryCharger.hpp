#ifndef __tBatteryCharger__
#define __tBatteryCharger__

#include "tShapeEntity.hpp"
#include "tBreakableLogic.hpp"
#include "tProximity.hpp"
#include "tWorldSpaceFloatingText.hpp"

namespace Sig
{
	class tVehicleLogic;

	class tBatteryCharger : public tAnimatedBreakableLogic
	{
		define_dynamic_cast( tBatteryCharger, tAnimatedBreakableLogic );
	public:
		tBatteryCharger( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fMoveST( f32 dt );
		virtual void fCoRenderMT( f32 dt );
		virtual Math::tVec4f fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const;
		virtual Gui::tRadialMenuPtr fCreateSelectionRadialMenu( tPlayer& player );

		virtual b32 fShouldSelect( ) const { return !mHasPurchased && fSelectionEnabled( ); }

		virtual b32 fHandleLogicEvent( const Logic::tEvent& e );

		b32 fIsPurchasePlatform( ) const { return fHasSelectionFlag( ); }

	public:

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tShapeEntity*	mShape;
		tProximity		mProximity;

		tVehicleLogic*  mVehicle;
		tEntityPtr		mVehicleEnt;

		f32				mTargetBarProgress;
		f32				mBarProgress;
		tEntityPtr		mHasPurchasedEnt;
		u32				mHasPurchased; //unit id of purchased unit, zero for none
		tPlayerPtr		mPurchaser;

		tEntityPtr		mSpawnPt;

		Gui::tWorldSpaceFloatingTextPtr mText;

		void fSetVehicle( tVehicleLogic* veh );
		void fApplyText( );

		b32 fPurchaseVehicle( tPlayer* player, u32 unitID, u32 price );
		b32 fShouldShowStatus( tVehicleLogic* vehicle ) const;
		void fActuallySpawn( );

	};

}

#endif//__tBatteryCharger__

