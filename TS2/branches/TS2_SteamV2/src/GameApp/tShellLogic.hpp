#ifndef __tShellLogic__
#define __tShellLogic__

#include "tProjectileLogic.hpp"
#include "Audio/tSource.hpp"

namespace Sig
{

	class tShellPhysics
	{
	public:
		Math::tVec3f mPosition;
		Math::tVec3f mVelocity;
		Math::tVec3f mGravity;

		tShellPhysics( const Math::tVec3f& position = Math::tVec3f()
					, const Math::tVec3f& velocity = Math::tVec3f()
					, f32 gravity = -9.8f)
			: mPosition( position )
			, mVelocity( velocity )
			, mGravity( 0.f, gravity, 0.f )
		{ }

		Math::tVec3f& fStep( f32 dt, f32 mVelocityMultiplier = 1.f )
		{
			mVelocity += mGravity * dt;
			mPosition += ( mVelocity * mVelocityMultiplier ) * dt;
			return mPosition;
		}
	};

	class tShellLogic : public tProjectileLogic
	{
		define_dynamic_cast( tShellLogic, tProjectileLogic );
	public:
		tShellLogic( );

		virtual void fMoveST( f32 dt );

		virtual f32 fUpdateShellCam( tPlayer& player, f32 dt, u32 inputFilter );

		virtual void fInherit( tProjectileLogic& from, f32 dt );

		static void fExportScriptInterface( tScriptVm& vm );

		virtual void fBurst( f32 dt );
		void fProcessBursting( f32 dt );

		u32					fBurstCount( ) const { return mBurstCount; }
		const tFilePathPtr& fBurstPath( ) const { return mBurstPath; }
		const tStringPtr&	fBurstEffect( ) const { return mBurstEffect; }
		f32					fBurstOffset( ) const { return mBurstOffset; }
		f32					fBurstSpread( ) const { return mBurstSpread; }

		void fSetRotation( f32 rotation ) { mRotation = rotation; }
		f32 fRotation( ) const { return mRotation; }
		f32 fRotationRate( ) const { return mRotationRate; }

		tShellPhysics* fPhysics( ) { return &mPhysics; }
		Math::tVec3f fOriginalX( ) const { return mOriginalX; }

	protected:
		virtual void fInitPhysics( );
		virtual void fComputeNewPosition( f32 dt );

		virtual Math::tVec3f& fCurrentVelocity( ) { return mPhysics.mVelocity; }

	private:
		tShellPhysics mPhysics;
		Math::tVec3f mOriginalX;
		f32 mShellCamSteerRate;
		f32 mRotation;
		f32 mRotationRate;
		
		u32 mBurstCount; //how many shells to burst into when left trigger is hit.
		f32 mBurstDamageMod; //how much mDamageMod each bursted shell will have
		f32 mBurstCountdown;	//if this reaches zero, it's burst time
		
		f32 mBurstOffset;
		f32 mBurstSpread;

		tFilePathPtr mBurstPath; //new shell to spawn during burst. if not sell will spawn self
		tStringPtr mBurstEffect;
	};

}

#endif//__tShellLogic__
