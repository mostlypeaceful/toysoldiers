#include "BasePch.hpp"
#include "tHinge.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{
	
	tOneWayHinge::tOneWayHinge( const Math::tMat3f& hingeXform, const Math::tMat3f& bXform )
		: mAOffset( hingeXform )
		, mBOffset( hingeXform.fInverse( ) * bXform )
		, mLastHingeVel( tVec3f::cZeroVector )
		, mAngle( 0.f )
		, mAngleVel( 0.f )
		, mLowerLimit( -cPi )
		, mUpperLimit( cPi )
		, mDamping( 0.1f )
		, mBtoHingeRadius( mBOffset.fGetTranslation( ).fLength( ) )
		, mLatched( false )
		, mAutoLatch( false )
		, mAutoLatchDelay( 0.f )
	{
		fRecomputeBRelToA( );
	}

	void tOneWayHinge::fSetAngle( f32 angle )
	{
		mAngle = angle;  
	}

	void tOneWayHinge::fSetAngleVel( f32 angVel )
	{
		mAngleVel = angVel; 
	}

	void tOneWayHinge::fLatch( f32 angle ) 
	{ 
		mLatched = true; 
		mAngle = angle; 
		mAngleVel = 0.f; 
	}

	void tOneWayHinge::fUnLatch( ) 
	{ 
		mLatched = false; 
		mAutoLatch = false;
	}

	void tOneWayHinge::fAutoLatch( f32 delay )
	{
		mAutoLatch = true;
		mAutoLatchDelay = delay;
	}

	void tOneWayHinge::fStepMT( const Math::tVec3f& worldVel, const Math::tVec3f& worldGravity, const Math::tMat3f& aXform, f32 dt )
	{
		if( !mLatched )
		{
			tVec3f hingeAcc = (worldVel - mLastHingeVel) / dt - worldGravity;

			hingeAcc = aXform.fInverseXformVector( hingeAcc );

			tVec3f bToHinge = mBRelToA.fGetTranslation( ) - mAOffset.fGetTranslation( );

			tVec3f angAccV = bToHinge.fCross( hingeAcc );
			f32 angAcc = -angAccV.fDot( mAOffset.fYAxis( ) );

			mAngleVel += angAcc * dt;
			f32 newAngle = mAngle + mAngleVel * dt;
			newAngle = fClamp( newAngle, mLowerLimit, mUpperLimit );
			mAngleVel = (newAngle - mAngle) / dt;
			mAngle = newAngle;

			f32 absVel = fAbs( mAngleVel );
			f32 damp = fClamp( fSign( mAngleVel ) * mDamping * dt, -absVel, absVel );
			mAngleVel -= damp;

			if( mAutoLatch )
			{
				const f32 cLatchThresh = 0.01f;
				if( mAutoLatchDelay <= 0.f && fAbs(mAngle) < cLatchThresh )
					fLatch( 0.f );
				else
					mAutoLatchDelay -= dt;
			}
		}

		mLastHingeVel = worldVel;
		fRecomputeBRelToA( );
	}

	void tOneWayHinge::fRecomputeBRelToA( )
	{
		tMat3f rotate( tQuatf( tAxisAnglef( mBOffset.fYAxis( ), mAngle ) ) );
		mBRelToA = mAOffset * rotate * mBOffset;
	}

	void tOneWayHinge::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tOneWayHinge, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("Angle"), &tOneWayHinge::mAngle)
			.Var(_SC("LowerLimit"), &tOneWayHinge::mLowerLimit)
			.Var(_SC("UpperLimit"), &tOneWayHinge::mUpperLimit)
			.Var(_SC("Damping"), &tOneWayHinge::mDamping)
			;

		vm.fNamespace(_SC("Physics")).Bind(_SC("OneWayHinge"), classDesc);
	}
}}