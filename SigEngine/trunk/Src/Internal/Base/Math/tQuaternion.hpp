#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tQuaternion__
#define __tQuaternion__

namespace Sig { namespace Math
{
	template<class t>
	class tQuaternion;

	// Replicated from CML
	enum tEulerOrder {
		cEulerOrderXYZ, // 0x00 [0000]
		cEulerOrderXYX, // 0x01 [0001]
		cEulerOrderXZY, // 0x02 [0010]
		cEulerOrderXZX, // 0x03 [0011]
		cEulerOrderYZX, // 0x04 [0100]
		cEulerOrderYZY, // 0x05 [0101]
		cEulerOrderYXZ, // 0x06 [0110]
		cEulerOrderYXY, // 0x07 [0111]
		cEulerOrderZXY, // 0x08 [1000]
		cEulerOrderZXZ, // 0x09 [1001]
		cEulerOrderZYX, // 0x0A [1010]
		cEulerOrderZYZ,  // 0x0B [1011]
		cEulerOrderCount
	};

	template<class t>
	class tEulerAngles : public tVector3<t>
	{
		sig_make_stringstreamable( tEulerAngles, "(" << x << ", " << y << ", " << z << ")" );
	public:
		static const tEulerAngles cZeroAngles;
		static inline tEulerAngles fConstruct( t x, t y, t z ) { return tEulerAngles( x, y, z ); }
	public:
		inline tEulerAngles( ) { }
		inline tEulerAngles( t val ) : tVector3( val ) { }
		inline tEulerAngles( t x, t y, t z ) : tVector3( x, y, z ) { }
		inline explicit tEulerAngles( const tQuaternion< t >& quat );
		inline explicit tEulerAngles( const tVector3< t >& a ) : tVector3( a ) { }
	};

	template<class t>
	inline tEulerAngles<t> fToRadians( const tEulerAngles<t>& degrees )
	{
		return tEulerAngles<t>( fToRadians( degrees.x ), fToRadians( degrees.y ), fToRadians( degrees.z ) );
	}

	template<class t>
	inline tEulerAngles<t> fToDegrees( const tEulerAngles<t>& radians )
	{
		return tEulerAngles<t>( fToDegrees( radians.x ), fToDegrees( radians.y ), fToDegrees( radians.z ) );
	}

	template<class t>
	const tEulerAngles<t> tEulerAngles<t>::cZeroAngles( 0, 0, 0 );

	typedef tEulerAngles<f32> tEulerAnglesf;
	typedef tEulerAngles<f64> tEulerAnglesd;


	template<class t>
	class tAxisAngle
	{
	public:

		tVector3<t> mAxis;
		t			mAngle;

	public:

		inline tAxisAngle( ) { }
		inline tAxisAngle( const tVector3<t>& axis, t angle ) : mAxis( axis ), mAngle( angle ) { }
		inline explicit tAxisAngle( const tQuaternion< t >& quat );
		inline explicit tAxisAngle( const tVector3<t>& a, const tVector3<t>& b )
		{ 
			fFromUnitVectors( a, b, a.fBuildPerp( ) );
		}

		inline explicit tAxisAngle( const tVector3<t>& a, const tVector3<t>& b, const tVector3<t>& fallbackAxis )
		{ 
			fFromUnitVectors( a, b, fallbackAxis );
		}

		void fFromUnitVectors( const tVector3<t>& a, const tVector3<t>& b, const tVector3<t> & fallbackAxis )
		{
			sig_assert_unit_vector( a );
			sig_assert_unit_vector( b );
			sig_assert_unit_vector( fallbackAxis );
			
			const t cAngleEpsilon = (t)0.001f;

			mAngle = fAcos( a.fDot( b ) );
			if( mAngle >= cAngleEpsilon && mAngle <= Math::cPi - cAngleEpsilon )
			{
				mAxis = a.fCross( b ).fNormalize( );
				return;
			}

			// If we're here then a and b are ~colinear
			
			// Ensure the specified fall back axis is ~perpendicular to a
			const f32 cAxisOrthogonalityEpsilon = 0.0001f;
			sigassert( (Sig::fEqual<t,t>( a.fDot( fallbackAxis ), 0, cAxisOrthogonalityEpsilon ) ) );

			if( mAngle < cAngleEpsilon )
				mAngle = 0;
			else // mAngle > Math::cPi - cAngleEpsilon	
				mAngle = Math::cPi;

			mAxis = fallbackAxis;
		}
	};

	typedef tAxisAngle<f32> tAxisAnglef;
	typedef tAxisAngle<f64> tAxisAngled;



	template<class t>
	class tQuaternion
	{
		declare_reflector( );
		sig_make_stringstreamable( tQuaternion, "(" << x << ", " << y << ", " << z << ", " << w << ")" );
	public:
		static const tQuaternion cIdentity;
		static const tQuaternion cZeroQuat;
		static const u32 cDimension = 4;

	public:

		t x,y,z,w;

	public:
 
		inline tQuaternion( ) { sig_setvecinvalid( t, x, cDimension ); }
		inline explicit tQuaternion( tNoOpTag ) { }
		inline tQuaternion( const tVector3<t>& _im, const t& _re=0 ) : x(_im.x), y(_im.y), z(_im.z), w(_re) { }
		inline tQuaternion( const t& _x, const t& _y, const t& _z, const t& _w ) : x(_x), y(_y), z(_z), w(_w) { }

		inline tQuaternion( const tVector3<t>& a, const tVector3<t>& b )
		{ 
			*this = tQuaternion( tAxisAngle<t>( a, b ) );
		}

		inline explicit tQuaternion( const tAxisAngle<t>& angleAxis )
		{
			const t a = fSin( 0.5f * angleAxis.mAngle );
			const t b = fCos( 0.5f * angleAxis.mAngle );
			x = a * angleAxis.mAxis.x;
			y = a * angleAxis.mAxis.y;
			z = a * angleAxis.mAxis.z;
			w = b;
		}

		inline explicit tQuaternion( const tEulerAngles<t>& eulerAngles )
		{
			const t cosx = fCos( 0.5f * eulerAngles.x );
			const t cosy = fCos( 0.5f * eulerAngles.y );
			const t cosz = fCos( 0.5f * eulerAngles.z );
			const t sinx = fSin( 0.5f * eulerAngles.x );
			const t siny = fSin( 0.5f * eulerAngles.y );
			const t sinz = fSin( 0.5f * eulerAngles.z );

			x = cosx*siny*sinz + sinx*cosy*cosz;
			y = cosx*cosz*siny - sinx*cosy*sinz;
			z = cosx*cosy*sinz + sinx*cosz*siny;
			w = cosx*cosy*cosz - sinx*siny*sinz;
		}

		template<class matrix3x3>
		inline explicit tQuaternion( const matrix3x3& m )
		{
			fFromMatrix( m );
		}

		inline t				fDot( const tQuaternion& rhs ) const		{ return x*rhs.x + y*rhs.y + z*rhs.z + w*rhs.w; }
		inline b32				fAntiPodal( const tQuaternion& other ) const { return this->fDot( other ) < cAntiPodalQuatThreshold; }

		inline t				fLengthSquared( ) const						{ return x*x + y*y + z*z + w*w; }
		inline t				fLength( ) const							{ return fSqrt( fLengthSquared( ) ); }


		inline tQuaternion&		fNormalize( )
		{
			sig_assertvecvalid( *this );
			const t l = fLength( );
			sigassert( !Sig::fEqual( l, t(0), cVectorEqualLengthEpsilon ) );
			return( (*this) /= l );
		}
		inline tQuaternion&		fNormalizeSafe( )
		{
			sig_assertvecvalid( *this );
			const t l = fLength( );
			if( !Sig::fEqual( l, t(0), cVectorEqualLengthEpsilon ) )
				return( (*this) /= l );
			return *this;
		}

		inline tQuaternion&		fNormalizeSafe( const tQuaternion& setToIfZero )
		{
			sig_assertvecvalid( *this );
			const t l = fLength( );
			if( Sig::fEqual( l, t(0), cVectorEqualLengthEpsilon ) )
				return( *this = setToIfZero );
			else
				return( *this /= l );
		}

		///
		/// \brief Set/get real (scalar) part
		inline t				fRe( ) const								{ return w; }
		inline void				fRe( const t re )							{ w = re; }

		///
		/// \brief Set/get imaginary (vector) part
		inline tVector3<t>		fIm( ) const								{ return tVector3<t>( x, y, z ); }
		inline void				fIm( const tVector3<t>& im )				{ x = im.x; y = im.y; z = im.z; }

		///
		/// \brief Inverse/conjugate (like matrices/complex numbers)
		inline tQuaternion		fConjugate( ) const							{ return tQuaternion( -x, -y, -z, w ); }
		inline tQuaternion		fInverse( ) const							{ return tQuaternion( fConjugate( ) / fLengthSquared( ) ); }

		///
		/// \brief Rotation operator (rotates the vector 'axis' as though the quat were a 3x3 rotation matrix)
		inline tVector3<t>		fRotate( const tVector3<t>& axis ) const
		{
			const t				r=fRe( );
			const tVector3<t>	i=fIm( );

			// Supposedly faster.
			//tVector3<t> x1 = 2 * i.fCross( axis );
			//tVector3<t> rv = axis + r * x1 + i.fCross( x1 ); 
			//return rv;
			return ( r*r - i.fLengthSquared( ) ) * axis + 2 * ( i.fDot( axis ) * i + r * i.fCross( axis ) );
		}

		inline t& fAxis ( u32 index )
		{
			switch( index )
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			case 3: return w;
			}
			sigassert( !"axis out of bounds" );
			return x;
		}

		inline const t& fAxis ( u32 index ) const
		{
			switch( index )
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			case 3: return w;
			}
			sigassert( !"axis out of bounds" );
			return x;
		}

		inline tQuaternion operator-( ) const
		{
			sig_assertvecvalid( *this );
			return tQuaternion( -x, -y, -z, -w );
		}

		inline b32 fIsZero( const f32 epsilon=cVectorEqualElementEpsilon ) const
		{
			sig_assertvecvalid( *this );
			if( !Sig::fEqual<t,t>( x, t(0), epsilon )
				|| !Sig::fEqual<t,t>( y, t(0), epsilon )
				|| !Sig::fEqual<t,t>( z, t(0), epsilon )
				|| !Sig::fEqual<t,t>( w, t(0), epsilon ) )
				return false;
			return true;
		}

		inline b32 fEqual( const tQuaternion& rhs, const f32 epsilon=cVectorEqualElementEpsilon ) const
		{
			sig_assertvecvalid( *this );
			sig_assertvecvalid( rhs );

			if( !Sig::fEqual<t,t>( x, rhs.x, epsilon )
				|| !Sig::fEqual<t,t>( y, rhs.y, epsilon )
				|| !Sig::fEqual<t,t>( z, rhs.z, epsilon )
				|| !Sig::fEqual<t,t>( w, rhs.w, epsilon ) ) 
				return false;

			return true;
		}

		inline b32 fIsNan( ) const
		{
			return ( x!=x || y!=y || z!=z || w!=w );				
		}

		inline b32 operator == ( const tQuaternion& rhs ) const { return fEqual( rhs ); }
		inline b32 operator != ( const tQuaternion& rhs ) const { return !fEqual( rhs ); }

		///
		/// \brief Convert 3x3 rotation matrix to quaternion
		template<class matrix3x3>
		void fFromMatrix( const matrix3x3& m )
		{
			t trace,s;

			trace = m(0,0) + m(1,1) + m(2,2);

			if( trace >= 0.0f )
			{
				s = fSqrt( trace + 1.f );
				w = 0.5f * s;
				s = 0.5f / s;
				x = ( m(2,1) - m(1,2) ) * s;
				y = ( m(0,2) - m(2,0) ) * s;
				z = ( m(1,0) - m(0,1) ) * s;
			} 
			else 
			{
				u32 i = 0;
				if( m(1,1) > m(0,0) )
					i = 1;
				if( m(2,2) > m(i,i) )
					i = 2;

				switch( i )
				{
				case 0:
					{
						s = fSqrt( m(0,0) - m(1,1) - m(2,2) + 1.f );
						x = 0.5f * s;
						s = 0.5f / s;
						y = ( m(0,1) + m(1,0) ) * s;
						z = ( m(2,0) + m(0,2) ) * s;
						w = ( m(2,1) - m(1,2) ) * s;
					}
					break;
				case 1:
					{
						s = fSqrt( m(1,1) - m(2,2) - m(0,0) + 1.f );
						y = 0.5f * s;
						s = 0.5f / s;
						x = ( m(0,1) + m(1,0) ) * s;
						z = ( m(1,2) + m(2,1) ) * s;
						w = ( m(0,2) - m(2,0) ) * s;
					}
					break;
				case 2:
					{
						s = fSqrt( m(2,2) - m(0,0) - m(1,1) + 1.f );
						z = 0.5f * s;
						s = 0.5f / s;
						x = ( m(2,0) + m(0,2) ) * s;
						y = ( m(1,2) + m(2,1) ) * s;
						w = ( m(1,0) - m(0,1) ) * s;
					}
					break;
				}
			}
		}

		///
		/// convert quaternion to 3x3 rotation matrix
		template<class matrix3x3>
		void fToMatrix( matrix3x3& m ) const
		{
			const t xx = 2*x*x;
			const t yy = 2*y*y;
			const t zz = 2*z*z;
			const t xy = 2*x*y;
			const t xz = 2*x*z;
			const t xw = 2*x*w;
			const t yz = 2*y*z;
			const t yw = 2*y*w;
			const t zw = 2*z*w;

			m(0,0) = 1 - yy - zz;
			m(0,1) = xy - zw;
			m(0,2) = xz + yw;
			
			m(1,0) = xy + zw;
			m(1,1) = 1 - xx - zz;
			m(1,2) = yz - xw;
			
			m(2,0) = xz - yw;
			m(2,1) = yz + xw;
			m(2,2) = 1 - xx - yy;
		}

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( x );
			archive.fSaveLoad( y );
			archive.fSaveLoad( z );
			archive.fSaveLoad( w );
		}
	};


#define quat_binary_op( type, op )	template< typename t >inline type operator op ( const type& a, const type& b ) { return type( a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w ); } \
	template< typename t > inline type& operator op= ( type& a, const type& b ) { a.x op= b.x; a.y op= b.y; a.z op= b.z; a.w op= b.w; return a; }
	quat_binary_op( tQuaternion<t>, + );
	quat_binary_op( tQuaternion<t>, - );

#define quat_scalar_op( type, op )	template< typename t > inline type operator op ( const type& a, const t& b ) { return type( a.x op b, a.y op b, a.z op b, a.w op b); } \
	template< typename t > inline type operator op ( const t& b, const type& a ) { return type( b op a.x, b op a.y, b op a.z, b op a.w); } \
	template< typename t > inline type& operator op= ( type& a, const t& b ) { a.x op= b; a.y op= b; a.z op= b; a.w op= b; return a; }
	quat_scalar_op( tQuaternion<t>, * );
	quat_scalar_op( tQuaternion<t>, / );

	///
	/// \brief Quaternion algebra multiplication
	template< typename t >
	inline tQuaternion<t> operator*( const tQuaternion<t>& lhs, const tQuaternion<t>& rhs )
	{
		const t					r0=lhs.fRe( ), r1=rhs.fRe( );
		const tVector3<t>		i0=lhs.fIm( ), i1=rhs.fIm( );
		return tQuaternion<t>(r0*i1 + r1*i0 + i0.fCross(i1), r0*r1 - i0.fDot(i1));
	}

	template< typename t >
	inline tQuaternion<t>& operator*=( tQuaternion<t>& lhs, const tQuaternion<t>& rhs )
	{ 
		lhs = (lhs * rhs);
		return lhs; 
	}

	template<class t>
	const tQuaternion<t> tQuaternion<t>::cIdentity( 0, 0, 0, 1.f );

	template<class t>
	const tQuaternion<t> tQuaternion<t>::cZeroQuat( 0, 0, 0, 0 );

	typedef tQuaternion<f32> tQuatf;
	typedef tQuaternion<f64> tQuatd;



	template<class t>
	tAxisAngle<t>::tAxisAngle( const tQuaternion< t >& quat )
	{
		t axisLen = 0.f;
		mAxis = quat.fIm( );
		mAxis.fNormalizeSafe( axisLen );
		mAngle = 2.f * fAtan2( axisLen, quat.fRe( ) );
	}

	template<class t>
	tEulerAngles<t>::tEulerAngles( const tQuaternion< t >& q )
		: tVector3( fAtan2( 2.f * ( q.x * q.w - q.y * q.z ), 1.f - 2.f * ( q.x * q.x + q.z * q.z ) )
		, fAtan2( 2.f * ( q.y * q.w - q.x * q.z ), 1.f - 2.f * ( q.y * q.y + q.z * q.z ) )
		, fAsin( 2.f * ( q.x * q.y + q.z * q.w ) ) )
	{
	}

	tEulerAnglesf fCMLDecompose( u32 order, const tQuatf& delta );
	tQuatf fCMLCompose( u32 order, const tEulerAnglesf& ea );

#undef quat_binary_op
#undef quat_scalar_op

}}


#endif//__tQuaternion__
