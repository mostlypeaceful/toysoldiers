#ifndef __tCharacterLogic__
#define __tCharacterLogic__
#include "tUnitLogic.hpp"
#include "Logic/tAnimatable.hpp"
#include "Audio/tSource.hpp"
#include "Physics/tCharacterPhysics.hpp"
#include "tUnitPath.hpp"
#include "Math/tDamped.hpp"

namespace Sig
{
	class tCharacterLogic : public tUnitLogic
	{
		define_dynamic_cast( tCharacterLogic, tUnitLogic );
	public:
		tCharacterLogic( );
		virtual ~tCharacterLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fOnSkeletonPropagated( );
		virtual b32 fHandleLogicEvent( const Logic::tEvent& e );
		virtual void fReactToDamage( const tDamageContext& dc, const tDamageResult& dr );

		Physics::tCharacterPhysics& fPhysics( ) { return mPhysics; }
		void fDisablePhysics( b32 disable );

		void fPostGoalSet( ); //call after master goal has been set

	public: // query specific components
		virtual Logic::tAnimatable* fQueryAnimatable( ) { return &mAnimatable; }
		virtual Logic::tPhysical*	fQueryPhysical( ) { return &mPhysics; }

	public: // accessors
		virtual tUnitPath*	fUnitPath( )	{ return mUnitPath.fGetRawPtr( ); }
		tUnitPath*			fContextUnitPath( )	{ sigassert( 0 && "Context Unit Path has been removed to save memory. do you realllly need it? " ); return NULL; } //return mContextUnitPath.fGetRawPtr( ); }
	public:
		virtual Math::tVec3f fLinearVelocity( const Math::tVec3f& localOffset );
		b32 fIsWithinLevelBounds( ) const;

		u32  fSurfaceTypeEnum( ) const { return mStandingOnSurfaceType; }

		void fSetParachuteing( b32 parachuting ) { mParachuting = parachuting; }
		b32  fParachuting( ) const { return mParachuting; }
		b32  fChuteOpen( ) const { return mChuteOpen; }
		void fSetChuteOpen( b32 open ) { mChuteOpen = open; }

		void fSetParentRelativeUntilLand( b32 disablePhys );
		void fClearParentRelativeUntilLand( );
		b32  fParentRelativeUntilLand( ) const { return mParentRelativeUntilLand; }


		b32  fDieImmediately( ) const { return mDieImmediately; }
		void fSetDieImmediately( b32 val ) { mDieImmediately = val; }

		b32  fContextAnimActive( ) const { return mContextAnimActive; }
		void fSetContextAnimActive( b32 active ) { mContextAnimActive = active; }
		u32  fCurrentContextAnimType( ) const { return mCurrentContextAnimType; }
		b32  fIsCaptain( ) const { return mUnitIDAlias != mUnitID; }
		b32  fIsCommando( ) const { return fUnitID( ) == GameFlags::cUNIT_ID_INFANTRY_OFFICER_01; }

		b32  fTestAndSetFireTarget( );
		const tStringPtr& fSingleShotWeaponID( ) const { return mSingleShotWeaponID; }

		b32 fUseSingleShotTarget( ) const { return mUseSingleShotTarget; }
		void fSetUseSingleShotTarget( b32 use ) { mUseSingleShotTarget = use; }

		b32 fAITargeting( ) const { return mAITargeting; }
		void fSetAITargeting( b32 target ) { mAITargeting = target; }

		void fSetDontInstaDestroy( b32 dont ) { mDontInstaDestroy = dont; }

		const Math::tVec3f& fFireTarget( ) const { return mFireTarget; }
		void  fBlendOutFireLooping( );

	protected:
		virtual void fAddToMiniMap( );

	protected:
		virtual void fActST( f32 dt );
		virtual void fAnimateMT( f32 dt );
		virtual void fPhysicsMT( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fThinkST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		// Context stuff
		void fQueryCollisionAndContextAnimOffenders( f32 dt, tGrowableArray< tEntity* >& contextAnims, tGrowableArray< tEntity* >& collisionShapes, b32 collectCollision );
		void fUpdateContextAnimsMT( f32 dt, const tGrowableArray< tEntity* >& offenders );
		void fUpdateContextAnimsST( f32 dt );
		void fContextAnimJump( );
		void fContextAnimJumpLanded( );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		Logic::tAnimatable			mAnimatable;
		Physics::tCharacterPhysics	mPhysics;
		tUnitPathPtr				mUnitPath;
		//tUnitPathPtr				mContextUnitPath;

		tEntityPtr mParachute;
		tLogic* mParachuteLogic;

		Math::tVec3f mLastPosition; //used to compute velocity

		b32 fHasPackage( ) const { return mHasPackage; }
		void fSetHasPackage( b32 has ) { mHasPackage = has; }
		void fDeletePackage( ) { if( mPackage ) mPackage->fDelete( ); mPackage.fRelease( ); }

		b8 mParachuting;
		b8 mChuteOpen;
		b8 mDieImmediately;
		b8 mParentRelative;

		b8 mParentRelativeUntilLand;
		b8 mContextAnimActive;
		b8 mUseSingleShotTarget;
		b8 mFellOutofLevel;

		b8 mHasPackage;
		b8 mExtraModeRespawn;
		b8 mAITargeting;
		b8 mDontInstaDestroy;

		b8 mContextAnimInAir;
		b8 mWasUsable;
		b8 pad0;
		b8 pad2;

		f32 mFlyingStartHeight;
		f32 mTimeTillNextRandomAnim;

		void fConfigureAudio( );

		// Context animations
		tShapeEntity*	mCurrentContextAnimEntity;
		tShapeEntity*	mNextContextAnimEntityMT;
		f32				mNextContextAnimHeightMT;

		tProximity mContextAnimAndCollisionProximty;
		u32 mCurrentContextAnimType;
		u32 mNextContextAnimTypeMT;
		u32 mStandingOnSurfaceType;

		tEntityPtr mPackage; // delivery package

		tStringPtr mSingleShotWeaponID;

		Math::tVec3f mFireTarget;
		Math::tMat3f mParentRelativeXformMT;
	};

}

#endif//__tCharacterLogic__
