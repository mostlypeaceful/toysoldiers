#include "BasePch.hpp"
#include "tContactPoint.hpp"
#include "tPhysicsWorld.hpp" //for debug rendering

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	void tContactPoint::fFlip( )
	{
		mPoint = fOtherBodyPt( );
		mNormal *= -1.f;
	}

	Math::tVec3f tContactPoint::fOtherBodyPt( ) const
	{
		// Default operation is to subtract the depth * the normal
		return mPoint - mNormal * mDepth;
	}

	void tContactPoint::fRender( const Math::tVec4f& color ) const
	{
		const Math::tVec4f oppositeColor( Math::tVec3f::cOnesVector - color.fXYZ( ), color.w );
		const Math::tVec3f otherBodyPt = fOtherBodyPt( );

		tPhysicsWorld::fDebugGeometry( ).fRenderOnce( mPoint, otherBodyPt, color );
		tPhysicsWorld::fDebugGeometry( ).fRenderOnce( Math::tSpheref( mPoint, 0.125f ), color );
		tPhysicsWorld::fDebugGeometry( ).fRenderOnce( Math::tSpheref( otherBodyPt, 0.125f ), oppositeColor );
	}

	// returns closest point on A.
	//  o[A,B] are the edge origins.
	//  dir[A,B] are expected to be normal length.
	//  sep normal expected to be normalized dirA cross dirB
	b32 tFullContactWitness::fClosestPointOnEdges( const tVec3f& oA, const tVec3f& dirA, const tVec3f& oB, const tVec3f& dirB, const tVec3f& sepNormal, tVec3f& output )
	{
		sigassert( fEqual( dirA.fLengthSquared( ), 1.f ) );
		sigassert( fEqual( dirB.fLengthSquared( ), 1.f ) );
		sigassert( fEqual( sepNormal.fLengthSquared( ), 1.f ) );

		// construct a plane along the separation normal and edgeB, through pointB
		tVec3f planeNormal = sepNormal.fCross( dirB );
		sigassert( fEqual( planeNormal.fLengthSquared( ), 1.f ) );

		f32 div = dirA.fDot( planeNormal );
		if( fAbs( div ) < Math::cEpsilon )
			return false; //edge are parallel

		f32 planeD = planeNormal.fDot( oB );
		f32 t = (planeD - oA.fDot( planeNormal )) / div;
		output = oA + dirA * t;

		return true;
	}


	void tFullContactWitness::fReCompute( )
	{

	}

} }