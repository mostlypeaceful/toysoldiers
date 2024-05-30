#ifndef __tHoverLogic__
#define __tHoverLogic__

#include "tVehicleLogic.hpp"
#include "Physics/tHoverPhysics.hpp"
#include "tTurretCameraMovement.hpp"


namespace Sig
{

	class tHoverLogic : public tVehicleLogic
	{
		define_dynamic_cast( tHoverLogic, tVehicleLogic );
	public:
		tHoverLogic( );

		virtual Logic::tPhysical*	fQueryPhysical( ) { return &mPhysics; }
		Physics::tHoverPhysics& fPhysics( ) { return mPhysics; }
		const Physics::tHoverPhysics& fPhysics( ) const { return mPhysics; }

		virtual b32 fHandleLogicEvent( const Logic::tEvent& e );

		virtual void fAnimateMT( f32 dt );
		virtual void fPhysicsMT( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fThinkST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		virtual void fOnDelete( );

		virtual void fResetPhysics( const Math::tMat3f& xform ) { mPhysics.fReset( xform ); }
		virtual const Math::tMat3f& fCurrentTransformMT( ) const { return mPhysics.fTransform( ); }
		virtual const Math::tVec3f& fCurrentVelocityMT( ) const { return mPhysics.fVelocity( ); }
		virtual Math::tVec3f fPointVelocityMT( const Math::tVec3f& worldPoint ) const { return mPhysics.fPointVelocity( worldPoint ); }
		virtual f32 fMaxSpeed( ) const { return mPhysics.fProperties( ).mMaxSpeed; }

		virtual void fRespawn( const Math::tMat3f& tm );
		virtual void fSetupVehicle( );
		virtual void fComputeUserInput( f32 dt );
		virtual void fComputeAIInput( f32 dt );
		virtual void fReactToDamage( const tDamageContext& dc, const tDamageResult& dr );
		virtual void fReactToWeaponFire( const tFireEvent& event );
		virtual void fPushCamera( tPlayer* player, u32 seat );
		virtual void fPopCamera( tPlayer* player );
		virtual void fReapplyTable( );
		virtual void fClearContacts( ) { mPhysics.fClearContacts( ); }
		virtual void fAddContactPt( f32 dt, const Physics::tContactPoint& cp ) { mPhysics.fAddContactPt( dt, cp ); }
		virtual Math::tVec3f fComputeContactResponse( tUnitLogic* theirUnit, const Physics::tContactPoint& cp, f32 mass, b32& ignore );
		virtual void fAddCollisionResponse( const Math::tVec3f& response, f32 mass ) { mPhysics.fAddCollisionResponse( response, mass ); }

		virtual tTurretCameraMovement* fRequestCameraMovement( ) { return &mCameraMovement; }
		// hover specific
		const tTurretCameraMovement& fCameraMovement( ) const { return mCameraMovement; }
		f32 fCameraLerp( ) const { return mCameraLerp; }

		virtual f32 fGroundHeight( ) const { return mPhysics.fCurrentHoverHeight( ); }

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
		void fApplyTableProperties( Physics::tHoverProperties &props );
		void fResetCameraBasis( tPlayer* player );

		tTurretCameraMovement mCameraMovement;

		f32 mCameraLerp;

		Physics::tHoverPhysics mPhysics;
		tEntityPtr mCollisionProbe;
	};

}

#endif//__tHoverLogic__
