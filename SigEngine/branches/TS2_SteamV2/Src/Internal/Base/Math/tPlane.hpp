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
	class tPlane : public tBaseVector<t, 4, tPlane<t> >
	{
		declare_reflector( );
	public:
		typedef tBaseVector<t, 4, tPlane<t> > tBase;

	public:
		t a,b,c,d;

	public:
		inline tPlane( )			{ sig_setvecinvalid( t, a, tBase::cDimension ); }
		inline tPlane( tNoOpTag )	{ }
		inline tPlane( t _a, t _b, t _c, t _d ) 
			: a(_a), b(_b), c(_c), d(_d) { sig_assertvecvalid( *this ); }
		inline tPlane( const tVector3<t>& n, t d )
			: a(n.x), b(n.y), c(n.z), d(d) { sig_assertvecvalid( *this ); }
		inline tPlane( const tVector3<t>& n, const tVector3<t>& p )
			: a(n.x), b(n.y), c(n.z), d( -n.fDot( p ) ) { sig_assertvecvalid( *this ); }
		inline tPlane( const tVector4<t>& pv )
			: a(pv.x), b(pv.y), c(pv.z), d(pv.w) { sig_assertvecvalid( *this ); }

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

		///
		/// \brief Normalize the plane (NOT standard vector normalization, but rather 
		/// dividing the whole plane equation thru by the length of the normal); this
		/// method will ensure that the plane's normal is unit-length.
		inline tPlane&		fPlaneNormalize( ) { *this /= fGetNormal( ).fLength( ); sig_assertvecvalid( *this ); return *this; }

		///
		/// \brief Tests the signed distance of a point to the plane.
		/// N.B.! for a "true" distance, the plane must first be normalized
		/// using the planenorm() method.
		/// This method acts as a half-space test:
		/// if the return value is greater than zero, the point
		/// lies in the pos. half-space, if 0, point is on the plane,
		/// and if less than zero, point is in neg. half-space.
		inline t			fSignedDistance( const tVector3<t>& p ) const { return p.fDot( fGetNormal( ) ) + d; }
	};

	typedef tPlane<f32> tPlanef;
	typedef tPlane<f64> tPlaned;

}}

#endif//__tPlane__
