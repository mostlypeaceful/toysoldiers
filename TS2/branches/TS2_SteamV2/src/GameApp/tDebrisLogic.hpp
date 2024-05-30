#ifndef __tDebrisLogic__
#define __tDebrisLogic__

#include "Physics/tRigidBody.hpp"
#include "tMeshEntity.hpp"
#include "tDebrisLogicDef.hpp"

namespace Sig
{
	class tDebrisLogic : public tLogic
	{
		define_dynamic_cast( tDebrisLogic, tLogic );

	public:
		tDebrisLogic( const tDebrisLogicDef& def );
		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fOnDelete( );
		virtual Math::tVec4f fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const;

		void fPhysicsSpawn( const Math::tVec3f& initialVel, const Math::tVec3f& effectVel, f32 dt );

		void fSetIgnore( tEntityPtr& ignore ) { mIgnore = ignore; }

		const Math::tAabbf& fDebrisBounds( ) const { return mBounds; }
		const Math::tMat3f& fOffset( ) const { return mOffset; }

		// Used to clamp processing time
		static u32 sDebrisCount; 
		static inline void fIncDebrisCount( ) { ++sDebrisCount; }
		static inline void fDecDebrisCount( ) { if( sDebrisCount > 0 ) --sDebrisCount; }
		static b32 fTooMuchDebris( );

	protected:
		virtual void fCoRenderMT( f32 dt );
		virtual void fMoveST( f32 dt );
		void fCollideAndRespond( f32 dt );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		Math::tAabbf mBounds;
		Math::tMat3f mOffset;
		tRandom mRandom;

		f32 mBounceMeter;
		f32 mBounceY;

		b8 mActive;
		b8 mFirstTick;
		b8 mDormantDueToPerf;
		b8 pad0;

		f32 mDeathHeight;
		f32 mTotalTimer;
		f32 mRandomFactor;
		f32 mCollisionDelay;

		const tDebrisLogicDef& mDef;

		Physics::tRigidBodyLight mPhysics;

		tEntityPtr mIgnore; //set this to be a weapon in the case of shell casings
	};

	void fSpawnDebris( tGrowableArray< tMeshEntityPtr >& entities, const Math::tVec3f& initialVel, const Math::tVec3f& effectVel, f32 dt, tEntity& toParent, const tDebrisLogicDef& debrisDef );
	void fSpawnDebris( tGrowableArray< tEntity* >& entities, const Math::tVec3f& initialVel, const Math::tVec3f& effectVel, f32 dt, tEntity& toParent, const tDebrisLogicDef& debrisDef );
	void fSpawnDebris( tEntity* entity, const Math::tVec3f& initialVel, const Math::tVec3f& effectVel, f32 dt, tEntity& toParent, const tDebrisLogicDef& debrisDef );

}

#endif//__tDebrisLogic__
