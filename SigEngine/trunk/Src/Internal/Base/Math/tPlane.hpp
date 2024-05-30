#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tPlane__
#define __tPlane__

namespace Sig { namespace Math
{

	///
	/// \brief Encapsulates a 4-component plane equation, of
	/// the form ax + by + cz + d.
	template<class t>
	class tPlane
	{
		declare_reflector( );
	public:
		static const u32 cDimension = 4;

		t a,b,c,d;

	public:
		inline tPlane( )			{ sig_setvecinvalid( t, a, cDimension ); }
		inline tPlane( tNoOpTag )	{ }
		inline tPlane( t _a, t _b, t _c, t _d ) 
			: a(_a), b(_b), c(_c), d(_d) { sig_assertvecvalid( *this ); }
		inline tPlane( const tVector3<t>& n, t d )
			: a(n.x), b(n.y), c(n.z), d(d) { sig_assertvecvalid( *this ); }
		inline tPlane( const tVector3<t>& n, const tVector3<t>& p )
			: a(n.x), b(n.y), c(n.z), d( -n.fDot( p ) ) { sig_assertvecvalid( *this ); }
		inline tPlane( const tVector4<t>& pv )
			: a(pv.x), b(pv.y), c(pv.z), d(pv.w) { sig_assertvecvalid( *this ); }
		inline tPlane( const tVector3<t>& v0, const tVector3<t>& v1, const tVector3<t>& v2 )
		{
			tVector3<t> n = (v1 - v0).fCross( v2 - v0 ).fNormalize( );
			a = n.x; b = n.y; c = n.z; d = -n.fDot( v0 );
			sig_assertvecvalid( *this ); 
		}

		///
		/// \brief Get the (potentially non-unit length) normal.
		inline tVector3<t> fGetNormal( )		const { return tVector3<t>( a, b, c ); }

		///
		/// \brief Get the unit-length normal; this method will actually normalize the normal before returning it.
		inline tVector3<t> fGetNormalUnit( )	const { return tVector3<t>( a, b, c ).fNormalize( ); }

		///
		/// \brief Transform the plane (returns a new plane, not modifying this).
		template<class tMat>
		inline tPlane fTransform( const tMat& matrix ) const { return matrix.fXform( a, b, c, d ); }

		/// \brief Return a new plane translated along fGetNormal( ) by distance.
		inline tPlane fTranslatedAlongNormal( t distance ) const
		{
			return tPlane( a, b, c, d - distance );
		}

		///
		/// \brief Normalize the plane (NOT standard vector normalization, but rather 
		/// dividing the whole plane equation thru by the length of the normal); this
		/// method will ensure that the plane's normal is unit-length.
		inline tPlane&		fPlaneNormalize( )
		{
			fDivide( fGetNormal( ).fLength( ) );
			sig_assertvecvalid( *this );
			return *this;
		}

		///
		/// \brief Tests the signed distance of a point to the plane.
		/// N.B.! for a "true" distance, the plane must first be normalized using the planenorm() method.
		///   This method acts as a half-space test:
		/// If the return value is greater than zero, the point lies in the positive half-space (e.g. the +normal side)
		//  If the return value is zero, then the point is on the plane,
		/// If the return value is less than zero, the point lies in the negative half-space (e.g. the -normal side)
		inline t fSignedDistance( const tVector3<t>& p ) const
		{
			return p.fDot( fGetNormal( ) ) + d;
		}

		/// \brief Projects the vector p onto the plane (that is, given a vector going through p at a right angle to
		/// the plane, it returns where that vector went through the plane).  N.B. Assumes a normalized plane!
		inline tVector3<t> fProject( const tVector3<t>& p ) const
		{
			sigassert( fEqual( fGetNormal( ).fLength( ), 1.0f ) );
			return p - fGetNormal( ) * fSignedDistance( p );
		}

		inline b32 fIsZero( const f32 epsilon=cVectorEqualElementEpsilon ) const
		{
			sig_assertvecvalid( *this );
			if( !Sig::fEqual<t,t>( a, t(0), epsilon )
				|| !Sig::fEqual<t,t>( b, t(0), epsilon )
				|| !Sig::fEqual<t,t>( c, t(0), epsilon )
				|| !Sig::fEqual<t,t>( d, t(0), epsilon ) )
				return false;
			return true;
		}

		inline b32 fIsNan( ) const
		{
			return ( a!=a || b!=b || c!=c || d!=d );
		}

		inline void fDivide( t div )
		{
			a /= div;
			b /= div;
			c /= div;
			d /= div;
		}
	};

	typedef tPlane<f32> tPlanef;
	typedef tPlane<f64> tPlaned;

}}

#endif//__tPlane__
