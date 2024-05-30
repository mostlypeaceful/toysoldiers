#ifndef __tUseUnitCamera__
#define __tUseUnitCamera__
#include "Gfx/tCameraController.hpp"

namespace Sig
{
	class tPlayer;
	class tUnitLogic;

	///
	/// \brief Default debug/free-look camera.
	class base_export tUseUnitCamera : public Gfx::tCameraController
	{
		define_dynamic_cast( tUseUnitCamera, Gfx::tCameraController );
	public:
		enum tState
		{
			cStateBlendIn,
			cStateUserControl,
			cStateInactive,
			cStateCount
		};
	
	public:
		explicit tUseUnitCamera( tPlayer& player, tUnitLogic* unitLogic, b32 alignUnitToCamera );
		virtual void fOnTick( f32 dt );
		virtual void fOnActivate( b32 active );

		void fStartBlendIn( );
		b32 fHasBlendedIn( ) const { return mHasBlendedIn; }
		void fSkipBlendIn( ) { mBlendInDist = 0.f; mState = cStateUserControl; mHasBlendedIn = true; }

		tPlayer& fPlayer( ) { return mPlayer; }
		tUnitLogic& fUnitLogic( ) { return *mUnitLogic; }
		const tEntityPtr& fUnitEntity( ) { return mUnitEntity; }

		b32 fCheckForAndHandleExit( b32 forceExit = false );

	protected:
		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );
		virtual b32 fCleanup( ) { return false; } //return true if camera was destroyed

		enum tBlendType
		{
			cBlendPreferEye,
			cBlendPreferLookAt,
			cBlendPureLerp
		};

		void fBlendTripods( f32 lerp, const Gfx::tTripod& newTripod, Gfx::tTripod& oldTripod );
		void fBlendTripodsPreferLookAt( f32 lerp, const Gfx::tTripod& newTripod, Gfx::tTripod& oldTripod );
		void fBlendTripodsPureLerp( f32 lerp, const Gfx::tTripod& newTripod, Gfx::tTripod& oldTripod );

		void fKeepCameraAlignedToUnit( Gfx::tTripod& tripod, const Math::tMat3f& baseXform );
		void fRayCastCorrectNewTripod( Gfx::tTripod& tripod, f32 dt );

		f32 mZoomDist; //additional distance along view dir to place camera, + is forward
		Math::tMat3f mBlendInUnitMatrix;
		b8 mAlignUnitToCamera;
		b8 mHasBlendedIn;
		u8 mBlendType;
		b8 mStepWhenPaused;

		b8 mRaycastTerrainPenetration;
		b8 pad0;
		b8 pad1;
		b8 pad2;

		f32 mBlendLerpStart;
		f32 mBlendLerpEnd;
		f32 mHasBlendedInPercentage;
		f32 mRaycastTerrainPen;
		
	private:
		void fOnTickBlendInControl( f32 dt, Gfx::tCamera& camera );
		void fOnTickUserControl( f32 dt, Gfx::tCamera& camera );

		tPlayer& mPlayer;
		tUnitLogic* mUnitLogic;
		tEntityPtr mUnitEntity;
		tState mState;
		f32 mBlendInDist;
		f32 mBlendInStartDist;

	protected:
		Gfx::tCamera mOriginalCamera;
		Math::tVec4f mOriginalDOF;
		Math::tVec4f mTargetDOF;
		f32 mTargetZoom;
		f32 mOverrideBlendDist;
		
	};
}


#endif//__tUseUnitCamera__

