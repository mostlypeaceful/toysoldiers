#ifndef __tLandMineLogic__
#define __tLandMineLogic__
#include "tProximity.hpp"
#include "tShellLogic.hpp"

namespace Sig
{
	class tLandMineLogic : public tProjectileLogic
	{
		define_dynamic_cast( tLandMineLogic, tProjectileLogic );
	public:
		tLandMineLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fActST( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		virtual void fInitPhysics( );
		virtual void fComputeNewPosition( f32 dt );
		virtual void fHitSomething( const tEntityPtr& ent );

	public: // accessors
		enum tMineType { cProximityMine = 0, cTimedMine = 1, cRemoteDetonateMine = 2 };

		// param can mean [distance, time, or group-id] depending on type
		void fSetMineType( tMineType type, f32 param );
		void fSetMineTypeScript( u32 type, f32 param );
		void fSetPlacedBy( tEntity* placedBy );
		static void fThrow( const Math::tMat3f& origin, const Math::tVec3f& target, const Math::tVec3f& normal, tEntity* owner );


		f32  fFullSize( ) { return mFullSize; }
		void fSetFullSize( f32 size ) { mFullSize = size; }
		f32	 fGrowRate( ) { return mGrowRate; }
		void fSetGrowRate( f32 growRate ) { mGrowRate = growRate; }
		f32  fHitPoints( ) { return mExplicitHitPoints; }
		void fSetHitPoints( f32 hitpoints ) { mExplicitHitPoints = hitpoints; }

		// the owner of the mine can configure this how ever they want
		tProximity& fProximity( ) { return mProximity; }
		void fExplode( );

		void fSetExplosionPath( const tFilePathPtr& fp ) { mExplosionPath = fp; }

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		tShellPhysics mPhysics;

		tProximity mProximity;

		tEntityPtr mPlacedBy;
		u32 mLayedByTeam;

		tMineType mType;
		f32 mParam; //usage depends on mType
		f32 mTime;
		b32 mTriggered;

		tFilePathPtr mExplosionPath;
		f32	mFullSize;
		f32	mGrowRate;
		f32 mExplicitHitPoints;
		
		b32 mFalling, mWasThrown;
		Math::tVec3f mTarget, mNormal;
		Math::tMat3f mRelativeTransform;

	};

}

#endif//__tLandMineLogic__