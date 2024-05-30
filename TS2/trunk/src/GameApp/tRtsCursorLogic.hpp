#ifndef __tRtsCursorLogic__
#define __tRtsCursorLogic__
#include "tShapeEntity.hpp"
#include "tSgFileRefEntity.hpp"
#include "tRtsCursorDisplay.hpp"

namespace Sig
{
	class tPlayer;
	class tRtsCamera;

	class tRtsCursorLogic : public tLogic
	{
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		enum tState
		{
			cStateRoaming,
			cStateUnitSelected,
			cStateSelectingTurretToPlace,
			cStatePlacingTurret,
			cStateCount
		};
	public:
		explicit tRtsCursorLogic( tPlayer& player, tRtsCamera& camera );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fActST( f32 dt );
		virtual void fThinkST( f32 dt );
		virtual void fCoRenderMT( f32 dt );
		void fOnCameraActivate( b32 active );

		void fResetNavigation( );

		const Math::tVec3f& fCurrentPosition( ) const { return mCursorPosition; }
		const tRtsCamera& fCamera( ) const { return mCamera; }
		tRtsCursorDisplay& fDisplay( ) { return mDisplay; }
		tPlayer* fPlayer( ) { return &mPlayer; }

		void fClearPlacementLock( ) { mPlacedLogic = NULL; mPlacedEntity.fRelease( ); }

	private:
		Math::tVec3f fTargetCursorPosition( ) const;
		void fEvaluateCursorPositionFromTargetMT( );
		void fEvaluateHoverUnitMT( );
		void fUpdateHoverUnitImp( );
		void fUpdateHoverUnitST( );
		void fUpdateSelection( );
		void fUpdateTurretToPlace( );
		void fUpdateTurretPlacement( );
		void fUpdateGhostTurret( );
		void fUpdateGhostTurretDisable( );
		void fCreateTurretPlacementMenu( );
		void fCreateTurretSelectionMenu( );
		void fShowTurretSelectionMenu( );
		void fHoverUnitSelectionChanged( );

		b32  fLockout( ) const;
		void fCreateBarbedWireGhost( );
	private:
		b32 fCreateGhostTurret( u32 turretID );
		void fDestroyGhostTurret( );
		void fKillPlacementMenu( );
		void fSpawnGameTurretFromGhostTurret( );
		tRtsCursorDisplay::tVisibility fVisibility( ) const;
	private:
		tPlayer& mPlayer;
		tRtsCamera& mCamera;
		tState mState;
		tRtsCursorDisplay mDisplay;
		Math::tVec3f mCursorPosition;

		tEntityPtr mHoverUnit, mNextHoverUnitMT;
		tUnitLogic* mHoverUnitLogic;
		tShapeEntityPtr mBuildSiteMT;

		tEntityPtr mPlacedEntity;
		tUnitLogic* mPlacedLogic;

		tSgFileRefEntityPtr mGhostTurret;
		tUnitLogic* mGhostTurretLogic;
		b8 mActive;
		b8 mBuildOnBuildSite;
		b8 mGhostBarbedWire;
		b8 mGhostBarbedWireValidPlacement;
		
		tGrowableArray<tEntityPtr> mBarbedWires;

		b32 mPlatformNeedsCaptured;
		b32 mPlatformNeedsCapturedMT;

		Math::tAabbf mGhostBarbedWireBounds;

		Gui::tRadialMenuPtr mUnitSelectionMenu;
		Gui::tRadialMenuPtr mTurretPlacementMenu;

		f32 mLastSelectedAngle;
		f32 mLastSelectedMag;
	};

	typedef tRefCounterPtr<tRtsCursorLogic>	tRtsCursorLogicPtr;
}

#endif//__tRtsCursorLogic__
