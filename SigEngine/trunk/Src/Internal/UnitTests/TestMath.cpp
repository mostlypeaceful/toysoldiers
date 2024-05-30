#include "UnitTestsPch.hpp"
#include "Base.rtti.hpp"

using namespace Sig;
using namespace Sig::Math;

template tVector1<float>;
template tVector2<float>;
template tVector3<float>;
template tVector4<float>;

template tQuaternion<float>;

template tMatrix4<float>;

void fTestAssertCase1( )
{
	// Origin: \\Shares\Shared\QA\Rise\Crashdumps\evanbox\08-02-13\17.12

	Math::tMat3f xform
		( +5.81824137e-011f,	+3.51140020e-011f,	-1.09511476e-010f, +753.3170780f
		, -1.59469556e-011f,	-1.19078622e-010f,	-4.66541215e-011f, + 30.3839836f
		, -1.13892271e-010f,	+3.46116434e-011f,	-4.94119294e-011f, +234.1471710f );
	const Math::tVec3f toTarget( 0, 0, -1 );
	const f32 dt = 0.0547934212f;
	const f32 turnRate = 0.200000003;
	xform.fTurnToFaceZ( toTarget, turnRate, dt );
}

void fTestArbitraryCornerCases1( )
{
	Math::tMat3f m = Math::tMat3f::cIdentity;
	m.fScaleGlobal( Math::tVec3f::cOnesVector * 9001 );
	m.fTurnToFaceZ(  Math::tVec3f::cZAxis, 1, 90001 );
	m.fTurnToFaceZ( -Math::tVec3f::cZAxis, 1, 90001 ); // 180
	m.fTurnToFaceZ(  Math::tVec3f::cXAxis, 1, 90001 );
	m.fTurnToFaceZ( -Math::tVec3f::cZAxis, 1, 90001 );
}

void fTestMatrixCornerCases( )
{
	fTestAssertCase1( );

	fTestArbitraryCornerCases1( );
}

define_unittest(TestMath)
{
	fTestAssertCase1( );

	fTestMatrixCornerCases( );

	tRandom& rand = tRandom::fSubjectiveRand( );

	{
		// test inverse of matrix3 and matrix4
		for( u32 i = 0; i < 10000; ++i )
		{
			{
				Math::tQuatf q( Math::tAxisAnglef( rand.fVecNorm<Math::tVec3f>( ), rand.fFloatInRange( 0.f, Math::cPi ) ) );
				Math::tMat4f m = Math::tMat4f::cIdentity;
				q.fToMatrix( m );
				m.fScaleLocal( rand.fFloatInRange( 0.5f, 10.f ) );
				m.fSetTranslation( Math::tVec3f( rand.fFloatInRange( -1000.f, +1000.f ), rand.fFloatInRange( -1000.f, +1000.f ), rand.fFloatInRange( -1000.f, +1000.f ) ) );
				Math::tMat4f inv = m.fInverse( );
				Math::tMat4f identity = inv * m;
				fAssert( identity.fEqual( Math::tMat4f::cIdentity, 0.01f ) );
			}
			{
				Math::tQuatf q( Math::tAxisAnglef( rand.fVecNorm<Math::tVec3f>( ), rand.fFloatInRange( 0.f, Math::cPi ) ) );
				Math::tMat3f m = Math::tMat3f::cIdentity;
				q.fToMatrix( m );
				m.fScaleLocal( rand.fFloatInRange( 0.5f, 10.f ) );
				m.fSetTranslation( Math::tVec3f( rand.fFloatInRange( -1000.f, +1000.f ), rand.fFloatInRange( -1000.f, +1000.f ), rand.fFloatInRange( -1000.f, +1000.f ) ) );
				Math::tMat3f inv = m.fInverse( );
				Math::tMat3f identity = inv * m;
				fAssert( identity.fEqual( Math::tMat3f::cIdentity, 0.01f ) );
			}
		}

		tVec3f a,b;

		a.x = 0.f;
		a.y = 1.f;
		a.z = 0.f;

		a.fNormalize( );
		b = a;
		b.fNormalizeSafe( );

		f32 l;
		b.fNormalizeSafe( l );

		tVec3f c = a + b;
		a = c.fInverse( );

		c /= b;
		c *= 1.f;

		b = 1.f * a;

		f32 m = b.fMin( );
		m = a.fMaxMagnitude( );

		tMat4f xform( MatrixUtil::cIdentityTag );

		xform.fXAxis( a );
		xform = xform.fInverse( );

		if( xform.fEqual( xform ) )
		{
		}

		tQuatf q0( Math::tAxisAnglef( rand.fVecNorm<Math::tVec3f>( ), rand.fFloatInRange( 0.f, Math::cPi ) ) ),
			q1( Math::tAxisAnglef( rand.fVecNorm<Math::tVec3f>( ), rand.fFloatInRange( 0.f, Math::cPi ) ) );
		q0 *= q1;

		q0 = fNLerp( q0, q1, 0.75f );

		a = fLerp( a, b, tVec3f(0.75f) );
	}




	// + -
	tVec4f a1( 1,2,3,4 );
	tVec4f b1( 5,6,7,8 );

	sigassert( (a1 + b1 - a1).fEqual( b1 ) );

	// scalar and component mul
	tVec4f d2d = 2.f / a1;
	tVec4f d2d2 = d2d * a1;
	sigassert( d2d2.fEqual( tVec4f( 2.f ) ) );
	tVec4f d3d = a1 / 2.f;
	tVec4f d3d2 = 2.f * d3d;
	sigassert( d3d2.fEqual( a1 ) );

	// normalize
	a1.fNormalize( );
	sigassert( fEqual( a1.fLength( ), 1.f ) );

	// Cross products
	tVec3f a2( 5,6,7 );
	tVec3f b2( 1,1,1 );

	a2.fNormalize( );
	b2.fNormalize( );
	tVec3f c2 = a2.fCross( b2 );
	sigassert( (-c2).fEqual( b2.fCross( a2 ) ) );

	sigassert( tVec3f::cYAxis.fEqual( tVec3f::cZAxis.fCross( tVec3f::cXAxis ) ) );
	sigassert( tVec3f::cXAxis.fEqual( tVec3f::cYAxis.fCross( tVec3f::cZAxis ) ) );
	sigassert( tVec3f::cZAxis.fEqual( tVec3f::cXAxis.fCross( tVec3f::cYAxis ) ) );

	// dot
	sigassert( fEqual( 2.f, tVec3f::cZAxis.fDot( tVec3f::cZAxis * 2 ) ) );
	sigassert( fEqual( 0.f, tVec3f::cZeroVector.fDot( tVec3f::cZAxis * -2 ) ) );
	sigassert( fEqual( -2.f, tVec3f::cZAxis.fDot( tVec3f::cZAxis * -2 ) ) );
	sigassert( fEqual( 4.f, tVec4f::cOnesVector.fDot( tVec4f::cOnesVector ) ) );

	// quaternions
	tQuatf q1( tAxisAnglef( tVec3f::cYAxis, cPiOver2 ) );
	tVec3f c3 = q1.fRotate( tVec3f::cZAxis );
	sigassert( c3.fEqual( tVec3f::cXAxis ) );

	tQuatf q2 = q1*q1;
	tVec3f c33 = q2.fRotate( tVec3f::cZAxis );
	sigassert( c33.fEqual( -tVec3f::cZAxis ) );

	// matrix
	{
		tMat3f scaleTest = tMat3f::cIdentity;
		sigassert( fEqual( scaleTest.fGetScale( ), tVec3f::cOnesVector ) );
		scaleTest.fScaleLocal( tVec3f( 2 ) );
		sigassert( fEqual( scaleTest.fGetScale( ), 2 * tVec3f::cOnesVector ) );
		scaleTest.fScaleLocal( tVec3f( 0.25f ) );
		sigassert( fEqual( scaleTest.fGetScale( ), 0.5f * tVec3f::cOnesVector ) );
	}
}

