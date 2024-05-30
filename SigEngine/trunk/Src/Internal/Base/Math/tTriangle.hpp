#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tTriangle__
#define __tTriangle__

namespace Sig { namespace Math
{

	template<class t>
	class tTriangle
	{
	public:

		typedef tVector3<t> tVertex;

		tVector3<t> mA, mB, mC;

	public:
				
		inline tTriangle( ) { }

		inline tTriangle( const tVertex& a, const tVertex& b, const tVertex& c ) 
			: mA( a ), mB( b ), mC( c ) { }

		// Will not identify triangles that are isomorphic but with different vertex order.
		b32 operator==( const tTriangle& other ) const
		{
			return fEqual( mA, other.mA ) && fEqual( mB, other.mB ) && fEqual( mC, other.mC );
		}

		inline tAabb<t> fGetAabb( ) const
		{
			tAabb<t> o( mA, mA );
			o |= mB;
			o |= mC;
			return o;
		}

		inline tVector3<t> fComputeNormal( ) const
		{
			return ( mB - mA ).fCross( mC - mB );
		}

		inline tVector3<t> fComputeUnitNormal( ) const
		{
			tVector3<t> norm = fComputeNormal( );
			sigcheckfail( !norm.fIsZero( ), return tVector3<t>::cYAxis );

			return norm.fNormalize( );
		}

		inline tPlane<t> fComputePlane( ) const
		{
			return tPlane<t>( fComputeUnitNormal( ), mA );
		}

		inline tTriangle<t> fTransform( const tMatrix3<t>& xform ) const
		{
			return tTriangle( xform.fXformPoint( mA ), xform.fXformPoint( mB ), xform.fXformPoint( mC ) );
		}

		inline void fMinMaxProject( t& min, t& max, const tVector3<t>& alongThisNormal) const
		{
			t s = mA.fDot( alongThisNormal );
			t u = mB.fDot( alongThisNormal );
			t d = mC.fDot( alongThisNormal );
			min = fMin( s, u, d );
			max = fMax( s, u, d );
		}

		inline const tVector3<t>& fCorner( u32 index ) const
		{
			switch( index )
			{
			case 0: return mA; break;
			case 1: return mB; break;
			case 2: return mC; break;
			}

			sigassert( !"Should not have got here." );
			return tVector3<t>::cZeroVector;
		}

		u32 fSupportingCornerIndex( const tVector3<t>& axis ) const
		{
			const t d0 = fCorner( 0 ).fDot( axis );
			const t d1 = fCorner( 1 ).fDot( axis );
			const t d2 = fCorner( 2 ).fDot( axis );

			return (d0 > d1) ? (d0 > d2 ? 0 : 2) : ((d1 > d2) ? 1 : 2);
		}

		tVector3<t> fSupportingCorner( const tVector3<t>& axis ) const
		{
			return fCorner( fSupportingCornerIndex( axis ) );
		}

		tVector3<t> fComputeCenter( ) const
		{
			return (mA + mB + mC)/3.f;
		}

		//http://www.gamedev.net/topic/552906-closest-point-on-triangle/
		tVector3<t> fClampPtInTriangle( const tVector3<t> &sourcePosition ) const
		{
			tVector3<t> edge0 = mB - mA;
			tVector3<t> edge1 = mC - mA;
			tVector3<t> v0 = mA - sourcePosition;

			t a = edge0.fDot( edge0 );
			t b = edge0.fDot( edge1 );
			t c = edge1.fDot( edge1 );
			t d = edge0.fDot( v0 );
			t e = edge1.fDot( v0 );

			t det = a*c - b*b;
			t s = b*e - c*d;
			t time = b*d - a*e;

			if ( s + time < det )
			{
				if ( s < 0.f )
				{
					if ( time < 0.f )
					{
						if ( d < 0.f )
						{
							s = fClamp<t>( -d/a, 0.f, 1.f );
							time = 0.f;
						}
						else
						{
							s = 0.f;
							time = fClamp<t>( -e/c, 0.f, 1.f );
						}
					}
					else
					{
						s = 0.f;
						time = fClamp<t>( -e/c, 0.f, 1.f );
					}
				}
				else if ( time < 0.f )
				{
					s = fClamp<t>( -d/a, 0.f, 1.f );
					time = 0.f;
				}
				else
				{
					t invDet = 1.f / det;
					s *= invDet;
					time *= invDet;
				}
			}
			else
			{
				if ( s < 0.f )
				{
					t tmp0 = b+d;
					t tmp1 = c+e;
					if ( tmp1 > tmp0 )
					{
						t numer = tmp1 - tmp0;
						t denom = a-2*b+c;
						s = fClamp<t>( numer/denom, 0.f, 1.f );
						time = 1-s;
					}
					else
					{
						time = fClamp<t>( -e/c, 0.f, 1.f );
						s = 0.f;
					}
				}
				else if ( time < 0.f )
				{
					if ( a+d > b+e )
					{
						t numer = c+e-b-d;
						t denom = a-2*b+c;
						s = fClamp<t>( numer/denom, 0.f, 1.f );
						time = 1-s;
					}
					else
					{
						s = fClamp<t>( -e/c, 0.f, 1.f );
						time = 0.f;
					}
				}
				else
				{
					t numer = c+e-b-d;
					t denom = a-2*b+c;
					s = fClamp<t>( numer/denom, 0.f, 1.f );
					time = 1.f - s;
				}
			}

			return mA + s * edge0 + time * edge1;
		}
	};

	typedef tTriangle<f32> tTrianglef;
	typedef tTriangle<f64> tTriangled;

}}

#endif//__tTriangle__

