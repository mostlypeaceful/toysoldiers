#include "BasePch.hpp"
#include "tSprungMass.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	tSprungMass::tSprungMass( )
		: mPrevParent( tMat3f::cIdentity )
		, mBasis( tMat3f::cIdentity )
		, mPrevVelocity( tVec3f::cZeroVector )
		, mCurrentPosition( tVec3f::cZeroVector )
		, mMaxAcc( 10.0f ) //gravity

		, mMaxCompression( 0.5f )
		, mMaxExtension( 0.75f )
		, mCompressionSpring( 2.0f )
		, mExtensionSpring( 0.75f )
	{
		
	}
	
	void tSprungMass::fResetParent( const Math::tMat3f& basis ) 
	{ 
		mPrevParent = basis; 
		mPrevVelocity = tVec3f::cZeroVector;
		mCurrentPosition.fSetValue( tVec3f::cZeroVector );
	}
	
	void tSprungMass::fStep( const Math::tMat3f& newParent, f32 dt )
	{
		// compute acceleration of point
		tVec3f p = fGetPosition( );
		tVec3f p0 = mPrevParent.fXformPoint( p );
		tVec3f p1 = newParent.fXformPoint( p );
		
		mPrevParent = newParent;

		tVec3f vel = (p1 - p0) / dt;
		tVec3f acc = (vel - mPrevVelocity) / dt;

		mPrevVelocity = vel;

		// quantinize
		f32 verticalAcc = newParent.fYAxis( ).fDot( acc ) / mMaxAcc.y;
		verticalAcc = fClamp( verticalAcc, -1.0f, 1.0f );

		tVec2f horizontalAcc( newParent.fXAxis( ).fDot( acc ) / mMaxAcc.x, newParent.fZAxis( ).fDot( acc ) / mMaxAcc.z );

		if( horizontalAcc.fLengthSquared( ) > 1.0f )
			horizontalAcc.fNormalize( );

		// convert to position
		f32 verticalPos = 0.0f;
		if( verticalAcc > 0.0f )
			verticalPos = verticalAcc * -mMaxCompression;
		else 
			verticalPos = verticalAcc * -mMaxExtension;

		tVec3f targetPos( -horizontalAcc.x, verticalPos, -horizontalAcc.y );

		// smooth integrate
		mCurrentPosition.fStep( targetPos, dt );
	}
	
}}
