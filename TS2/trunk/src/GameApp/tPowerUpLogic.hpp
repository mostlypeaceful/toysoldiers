#ifndef __tPowerUpLogic__
#define __tPowerUpLogic__

#include "tUnitLogic.hpp"

namespace Sig
{

	// TODO: Maybe doesn't need to inherit from tUnitLogic, but power ups might be destructible or selectable, so...
	class tPowerUpLogic : public tUnitLogic
	{
		define_dynamic_cast( tPowerUpLogic, tUnitLogic );
	public:
		tPowerUpLogic( );
		virtual ~tPowerUpLogic( );
		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fActST( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		virtual void fReactToDamage( const tDamageContext& dc, const tDamageResult& dr );

		const Math::tVec3f& fLaunchVector( ) const { return mLaunchVector; }
		void fSetLaunchVector( const Math::tVec3f& lv ) { mLaunchVector = lv; }

		f32 fPowerUpValue( ) const { return mPowerUpValue; }
		void fSetPowerUpValue( f32 value ) { mPowerUpValue = value; }
		void fSetTeam( GameFlags::tTEAM team );

		virtual Gui::tRadialMenuPtr fCreateSelectionRadialMenu( tPlayer& player );
		b32 fTryToUse( );

		virtual tLocalizedString fHoverText( ) const;

	private:
		void fComputeNewPosition( f32 dt );
		void fRayCast( );
		b32 fCheckLevelBounds( ) const;

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		Math::tVec3f mNextPos;
		Math::tVec3f mLaunchVector;
		b16 mFalling;
		b16 mOutsideLevel;
		f32 mSpin;
		f32 mBounce;
		f32 mRestHeight;

		f32 mPowerUpValue;
	};

}

#endif//__tPowerUpLogic__
