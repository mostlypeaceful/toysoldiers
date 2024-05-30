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
			sigassert( !norm.fIsZero( ) );
			return fComputeNormal( ).fNormalize( );
		}

		inline tPlane<t> fComputePlane( ) const
		{
			return tPlane<t>( fComputeUnitNormal( ), mA );
		}

		inline tTriangle<t> fTransform( const tMatrix3<t>& xform ) const
		{
			return tTriangle( xform.fXformPoint( mA ), xform.fXformPoint( mB ), xform.fXformPoint( mC ) );
		}
	};

	typedef tTriangle<f32> tTrianglef;
	typedef tTriangle<f64> tTriangled;

}}

#endif//__tTriangle__

