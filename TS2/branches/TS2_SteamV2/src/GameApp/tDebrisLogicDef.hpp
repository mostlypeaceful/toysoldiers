#ifndef __tDebrisLogicDef__
#define __tDebrisLogicDef__

namespace Sig
{

	class tDebrisLogicDef : public tRefCounter
	{
	public:
		f32 mGravity;
		f32 mUpVelMax;
		f32 mUpVelMin;
		f32 mHorzVelMax;
		f32 mTorqueArm;
		f32 mBounceRestitution;
		f32 mBounceFriction;
		f32 mLifeMax;
		f32 mBounceHeight; //once bounces dont bounce this high, debris will fall through
		f32 mInheritedVelocityFactor;
		f32 mVelocityMax;
		f32 mCollisionDelay;
		f32 mEffectMod;

		tDebrisLogicDef( )
		{
			fMemSet( *this, 0 );
		}
	};

	define_smart_ptr( base_export, tRefCounterPtr, tDebrisLogicDef );
}

#endif //__tDebrisLogicDef__