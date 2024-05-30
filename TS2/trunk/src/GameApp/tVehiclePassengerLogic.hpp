#ifndef __tVehiclePassengerLogic__
#define __tVehiclePassengerLogic__
#include "tUnitLogic.hpp"
#include "Logic/tAnimatable.hpp"
#include "IK/tIK.hpp"
#include "Physics/tSprungMass.hpp"
#include "Physics/tCharacterPhysics.hpp"

namespace Sig
{
	class tVehiclePassengerLogic : public tUnitLogic, public IK::tIKCallback
	{
		define_dynamic_cast( tVehiclePassengerLogic, tUnitLogic );
	public:
		tVehiclePassengerLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fAnimateMT( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual b32 fHandleLogicEvent( const Logic::tEvent& e );
		static void fExportScriptInterface( tScriptVm& vm );
		virtual void fOnSkeletonPropagated( );

		virtual Logic::tAnimatable* fQueryAnimatable( ) { return &mAnimatable; }
		virtual Logic::tPhysical*	fQueryPhysical( ) { return &mPhysics; }

		void fPostGoalSet( );

		static void fOutfitCharacterProps( tEntity& root );

		void fEject( const Math::tVec3f& launchVel );

		// Specific to IK
		virtual void fCallbackMT( IK::tIKCallbackArgs& args );

		// Specific to vehicles and 
		void fSetup( const Math::tVec3f& offset, const Math::tMat3f& myTransform, const Math::tMat3f &parent );
		void fSetTargets( tEntity* leftHand, tEntity* rightHand );
		void fReset( const Math::tMat3f& parent );
		
		void fDependentPhysicsMT( f32 dt, const Math::tMat3f &parent, tLogic& logic );
		Physics::tSprungMass& fSprungMass( ) { return mSprungMass; }		

		void fGenerateRandomAnimEvents( b32 enable );

	private:
		b8				mOrphaned; //true if no longer dependent on vehicle.
		b8				mSprung;
		b8				mGenRandomAnims;
		b8 pad1;

		f32				mRandomAnimTime;
		tUnitLogic*		mParent;
		tEntityPtr		mParentEnt;

		tStringPtr		mCrewman;
		tStringPtr		mSingleShotWeaponID;

		Physics::tCharacterPhysics mPhysics;
		Physics::tSprungMass	mSprungMass;
		Math::tMat3f			mSprungMassToTransform;

		tFixedArray< tEntityPtr, 2 > mHandTargets;
		Logic::tAnimatable mAnimatable;

		void fConfigureAudio( );
		f32 fPassengerTimeScale( ) const;
	};

}

#endif//__tVehiclePassengerLogic__
