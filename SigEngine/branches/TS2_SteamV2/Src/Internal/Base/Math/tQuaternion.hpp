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
		cEulerOrderZYZ  // 0x0B [1011]
	};

	template<class t>
	class tEulerAngles : public tBaseVector<t, 3, tEulerAngles<t> >
	{
	public:
		static const tEulerAngles cZeroAngles;
		static inline tEulerAngles fConstruct( t x, t y, t z ) { return tEulerAngles( x, y, z ); }
	public:

		t x, y, z;

		inline tEulerAngles( ) { }
		inline tEulerAngles( t val ) : x( val ), y( val ), z( val ) { }
		inline tEulerAngles( t _x, t _y, t _z ) : x( _x ), y( _y ), z( _z ) { }
		inline explicit tEulerAngles( const tQuaternion< t >& quat );
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

		inline tAxisAngle( const tVector3<t>& axis, t angle ) : mAxis( axis ), mAngle( angle ) { }
		inline explicit tAxisAngle( const tQuaternion< t >& quat );
		inline explicit tAxisAngle( const tVector3<t>& a, const tVector3<t>& b )
		{ 
			sigassert( (Sig::fEqual<t,t>( a.fLength( ), 1.f, cVectorZeroEpsilon ) && Sig::fEqual<t,t>( b.fLength( ), 1.f, cVectorZeroEpsilon )) );

			mAxis = a.fCross( b );
			t len = mAxis.fLength( );
			if( Sig::fEqual<t,t>( len, 0 ) )
			{
				mAxis = tVector3<t>::cXAxis;
				mAngle = 0;
			}
			else
			{
				mAxis.fNormalize( );
				mAngle = fAcos( a.fDot( b ) );
			}
		}
	};

	typedef tAxisAngle<f32> tAxisAnglef;
	typedef tAxisAngle<f64> tAxisAngled;



	template<class t>
	class tQuaternion : public tBaseVector<t, 4, tQuaternion<t> >
	{
		declare_reflector( );
		sig_make_loggable( tQuaternion, "(" << x << ", " << y << ", " << z << ", " << w << ")" );
	public:
		typedef tBaseVector<t, 4, tQuaternion<t> > tBase;
	public:
		static const tQuaternion cIdentity;
		static const tQuaternion cZeroQuat;

	public:

		t x,y,z,w;

	public:
 
		inline tQuaternion( ) { sig_setvecinvalid( t, x, tBase::cDimension ); }
		inline explicit tQuaternion( tNoOpTag ) { }
		inline tQuaternion( const tVector3<t>& _im, const t& _re=0 ) : x(_im.x), y(_im.y), z(_im.z), w(_re) { }
		inline tQuaternion( const t& _x, const t& _y, const t& _z, const t& _w ) : x(_x), y(_y), z(_z), w(_w) { }
		inline tQuaternion( f32 f ) { *this = cIdentity; }
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

		inline b32				fAntiPodal( const tQuaternion& other ) const { return fDot( other ) < cAntiPodalQuatThreshold; }

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
		inline tQuaternion		fInverse( ) const							{ return tQuaternion( fConjugate( ) / tBase::fLengthSquared( ) ); }

		///
		/// \brief Vector scalar multiplication
		inline tQuaternion		operator*( const t& scale ) const			{ return tBaseVector<t, 4, tQuaternion<t> >::operator *( scale ); }
		inline tQuaternion&		operator*=( const t& scale )				{ return tBaseVector<t, 4, tQuaternion<t> >::operator *=( scale ); }

		///
		/// \brief Quaternion algebra multiplication
		inline tQuaternion		operator*( const tQuaternion& q ) const
		{
			const t					r0=fRe( ), r1=q.fRe( );
			const tVector3<t>		i0=fIm( ), i1=q.fIm( );
			return tQuaternion(r0*i1 + r1*i0 + i0.fCross(i1), r0*r1 - i0.fDot(i1));
		}
		inline tQuaternion&		operator*=( const tQuaternion& q )			{ return (*this = (*this * q)); }

		///
		/// \brief Rotation operator (rotates the vector 'axis' as though the quat were a 3x3 rotation matrix)
		inline tVector3<t>		fRotate( const tVector3<t>& axis ) const
		{
			const t				r=fRe( );
			const tVector3<t>	i=fIm( );
			return ( r*r - i.fLengthSquared( ) ) * axis + 2 * ( i.fDot( axis ) * i + r * i.fCross( axis ) );
		}

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
	};

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
		: x( fAtan2( 2.f * ( q.x * q.w - q.y * q.z ), 1.f - 2.f * ( q.x * q.x + q.z * q.z ) ) )
		, y( fAtan2( 2.f * ( q.y * q.w - q.x * q.z ), 1.f - 2.f * ( q.y * q.y + q.z * q.z ) ) )
		, z( fAsin( 2.f * ( q.x * q.y + q.z * q.w ) ) )
	{
	}

}}


#endif//__tQuaternion__
