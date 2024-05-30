#include "BasePch.hpp"
#include "tDamped.hpp"

namespace Sig { namespace Math
{

	void tVelocityDamped::fStep( f32 target, f32 dt )
	{
		f32 delta = target - mCurrentValue;

		f32 absVel = fAbs( mVelocity );
		f32 absDelta = fAbs( delta );
		b32 stop = false;

		if( fSign( mVelocity ) != fSign( delta ) && fEqual( delta, 0.f ) )
			stop = true;
		else if( !fEqual( absVel, 0.f ) )
		{
			f32 stopTime = absVel / mAcceleration;
			f32 stopDistance = absVel * stopTime;
			if( stopDistance >= absDelta )
				stop = true;
		}

		f32 dv = 0.f;
		if( stop )
		{
			dv = -fSign( mVelocity ) * mAcceleration * dt;
			dv = fClamp( dv, -absVel, absVel );
		}
		else if( !fEqual( delta, 0.f ) )
		{
			dv = fSign( delta ) * mAcceleration * dt;
			dv = fClamp( dv, -absDelta * 0.5f, absDelta * 0.5f );
		}

		mVelocity += dv;
		mCurrentValue += mVelocity * dt;
	}

} }
