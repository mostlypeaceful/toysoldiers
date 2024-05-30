#include "ToolsPch.hpp"
#include "MeshPrimitives.hpp"

namespace Sig { namespace MeshSimplify
{
	Math::tMat3d tQuadric::fGetTensor( ) const
	{
		return Math::tMat3d( 
			a2, ab, ac, 0.0,
			ab, b2, bc, 0.0,
			ac, bc, c2, 0.0 );
	}

	Math::tMat4d tQuadric::fGetMatrix( ) const
	{
		return Math::tMat4d(
			a2, ab, ac, ad,
			ab, b2, bc, bd,
			ac, bc, c2, cd,
			ad, bd, cd, d2 );
	}

	tQuadric& tQuadric::operator+=( const tQuadric& Q )
	{
		a2 += Q.a2;	ab += Q.ab;	ac += Q.ac;	ad += Q.ad;
					b2 += Q.b2;	bc += Q.bc;	bd += Q.bd;
								c2 += Q.c2;	cd += Q.cd;
											d2 += Q.d2;

		mAreaWeight = Q.mAreaWeight;

		return *this;
	}

	tQuadric& tQuadric::operator*=( f64 scalar )
	{
		a2 *= scalar;	ab *= scalar;	ac *= scalar;	ad *= scalar;
						b2 *= scalar;	bc *= scalar;	bd *= scalar;
										c2 *= scalar;	cd *= scalar;
														d2 *= scalar;

		return *this;
	}

	tQuadric& tQuadric::operator/=( f64 scalar )
	{
		a2 /= scalar;	ab /= scalar;	ac /= scalar;	ad /= scalar;
						b2 /= scalar;	bc /= scalar;	bd /= scalar;
										c2 /= scalar;	cd /= scalar;
														d2 /= scalar;

		return *this;
	}

	void tQuadric::fSetPlaneConstraint( f64 a, f64 b, f64 c, f64 d )
	{
		a2 = a*a;	ab = a*b;	ac = a*c;	ad = a*d;
		b2 = b*b;	bc = b*c;	bd = b*d;
		c2 = c*c;	cd = c*d;
		d2 = d*d;
	}

	void tQuadric::fSetVertConstraint( const Math::tVec3d& v )
	{
		// A = identity matrix
		a2 = b2 = c2 = 1.0;  
		ab = ac = bc = 0.0;

		// b = -v
		ad = -v[0]; 
		bd = -v[1]; 
		cd = -v[2];	

		// c = p*p
		d2 = v.fDot( v );	
	}

	b32 tQuadric::fGetOptimalPos( Math::tVec3d& v ) const
	{
		Math::tMat3d Ainv = fGetTensor( ).fInverse( );
		f64 det = Ainv.fDeterminant( );

		if( fEqual( det, 0.0 ) )
			return false;

		v = -( Ainv.fXformVector( fGetVector( ) ) );
		return true;
	}

	f64 tQuadric::fEvaluate( const Math::tVec3d& v ) const
	{
		const f64 x = v.x;
		const f64 y = v.y;
		const f64 z = v.z;

		return	x*x*a2	+ 2*x*y*ab	+ 2*x*z*ac	+ 2*x*ad
						+ y*y*b2	+ 2*y*z*bc	+ 2*y*bd
									+ z*z*c2	+ 2*z*cd
												+ d2;
	}

	void tContractPair::fSetVerts( u32 v0, u32 v1 )
	{
		mV[0] = v0;
		mV[1] = v1;
	}

	u32 tContractPair::fGetOtherVert( u32 v ) const
	{
		if( mV[0] == v )
		{
			return mV[1];
		}
		else if( mV[1] == v )
		{
			return mV[0];
		}

		sigassert( 0 );
		return 0;
	}

	void tContractPair::fComputeCandidateAndCost( const tQuadric& Q, const Math::tVec3d& v0, const Math::tVec3d& v1 )
	{
		if( Q.fGetOptimalPos( mCandidate ) )
		{
			mCost = Q.fEvaluate( mCandidate );
		}
		else
		{
			// No optimal position based on the quadric matrix. Check the lowest cost given just the two end point vecs.
			const f64 v0Cost = Q.fEvaluate( v0 );
			const f64 v1Cost = Q.fEvaluate( v1 );

			if( v0Cost <= v1Cost )
			{
				mCandidate = v0;
				mCost = v0Cost;
			}
			else
			{
				mCandidate = v1;
				mCost = v1Cost;
			}
		}
	}

	void tVert::fRemapNeighbor( u32 from, u32 to )
	{
		for( u32 i = 0; i < mNeighborFaces.fCount( ); ++i )
		{
			if( mNeighborFaces[i] == from )
				mNeighborFaces[i] = to;
		}
	}

	void tFace::fRemapVert( u32 from, u32 to )
	{
		for( int i = 0; i < 3; ++i )
		{
			if( mVerts[i] == from )
			{
				mVerts[i] = to;
				return;
			}
		}

		sigassert( 0 );
	}
} }
