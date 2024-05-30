#ifndef __tUberBreakableDebris__
#define __tUberBreakableDebris__

namespace Sig
{
	class tMeshEntity;

	class tUberBreakableDebris : public tLogic
	{
		define_dynamic_cast( tUberBreakableDebris, tLogic );
	public:
		typedef void (*tUberBreakableDebrisOnBounce)(tUberBreakableDebris& uberBreakableDebris);
		static tUberBreakableDebrisOnBounce gOnDebrisBounce;
	public:

		static void fSpawnFromMesh( tMeshEntity& mesh, tEntity& newParent, const Math::tVec3f& velocity = Math::tVec3f::cZeroVector );
	public:
		tUberBreakableDebris( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		void fPhysicsSpawn( const Math::tVec3f& velocity );

		// Used to clamp processing time
		static u32 sDebrisCount; 
		static inline void fIncDebrisCount( ) { ++sDebrisCount; }
		static inline void fDecDebrisCount( ) { if( sDebrisCount > 0 ) --sDebrisCount; }
		static b32 fTooMuchDebris( );

	protected:
		virtual void fMoveST( f32 dt );
		virtual void fCoRenderMT( f32 dt );
		void fCollideAndRespond( f32 dt, const Math::tMat3f& prevPos );

	protected:
		Math::tAabbf mBounds;
		Math::tVec3f mCOMOffset;
		Math::tVec3f mScale;
		Math::tMat3f mPhysicsPos;
		Math::tVec3f mVelocity;
		Math::tQuatf mRotation;
		Math::tQuatf mRotationDelta;
		tRandom		 mRandom;

		b8 mActive;
		b8 mBounced;
		b8 mDormantDueToPerf;
		b8 pad0;

		u32 mBounceCount;
		f32 mDeathHeight;
		f32 mTotalTimer;

		u32 mMaxBounces;
		f32 mGravity;
		f32 mMassCoeff;
		f32 mEnergyLossNormCoeff;
		f32 mEnergyLossTangCoeff;
		f32 mRandVelocityCoeff;
		f32 mRandRotationCoeff;
	};
}

#endif//__tUberBreakableDebris__
