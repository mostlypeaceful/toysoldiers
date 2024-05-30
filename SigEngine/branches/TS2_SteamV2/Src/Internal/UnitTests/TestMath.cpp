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


define_unittest(TestMath)
{
	tRandom& rand = tRandom::fSubjectiveRand( );

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

