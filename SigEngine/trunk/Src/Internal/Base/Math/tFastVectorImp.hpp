// Dont use include guards, this file is expected to get included multiple times.
//#ifndef __tFastVectorImp__
//#define __tFastVectorImp__

//  Also dont use any namespaces, because you would be multiple namespaces nested then.
//namespace Sig { namespace Math
//{

	/*
	
		This defines the vector for 4d.
			It works off #defines to disable behavior for lesser dimensions.

		Expects these defines to be set:

		FAST_VEC_TYPE2 = tFastVec2
		FAST_VEC_TYPE3 = tFastVec3

		FAST_VEC_SIZE = 3
		FAST_VEC_TYPE = [ tFastVec2, tFastVec3, tFastVec4 ]
		FAST_VEC_XYZW( x, y, z, w ) x y z w
		FAST_VEC_XYZW_C( x, y, z, w ) x, y, z, w

	*/

#if (FAST_VEC_SIZE > 1) 
#define IF_Y( c ) c
#else
#define IF_Y( c )
#endif

#if (FAST_VEC_SIZE > 2) 
#define IF_Z( c ) c
#else
#define IF_Z( c )
#endif

#if (FAST_VEC_SIZE > 3) 
#define IF_W( c ) c
#else
#define IF_W( c )
#endif

#pragma warning( disable : 4146 )  // "unary minus operator applied to unsigned type, result still unsigned"

#define FAST_VEC_INV( v ) t( v ? (1.f / v) : cInfinity )

	template< typename t >
	class FAST_VEC_TYPE 
	{
		declare_explicit_reflector( );
		sig_make_stringstreamable( FAST_VEC_TYPE, "(" FAST_VEC_XYZW( << x, << ", " << y, << ", " << z, << ", " << w ) << ")" );
#if (FAST_VEC_SIZE == 3) 
		define_class_pool_new_delete( FAST_VEC_TYPE, 256 );
#endif

	public:
		static const u32 cDimension = FAST_VEC_SIZE;

		// The actual data
		t FAST_VEC_XYZW_C( x, y, z, w );

		// Constants
		static const FAST_VEC_TYPE cZeroVector;
		static const FAST_VEC_TYPE cOnesVector;

		static const FAST_VEC_TYPE cXAxis;
		IF_Y( static const FAST_VEC_TYPE cYAxis; )
		IF_Z( static const FAST_VEC_TYPE cZAxis; )
		IF_W( static const FAST_VEC_TYPE cWAxis; )

		// Constructors
		inline FAST_VEC_TYPE( ) { sig_setvecinvalid( t, x, cDimension ); }
		inline FAST_VEC_TYPE( tNoOpTag ) { }
		inline FAST_VEC_TYPE( t all ) : FAST_VEC_XYZW_C( x( all ), y( all ), z( all ), w( all ) ) { }
		IF_Y( inline FAST_VEC_TYPE( FAST_VEC_XYZW_C( t x, t y, t z, t w ) ) : FAST_VEC_XYZW_C( x( x ), y( y ), z( z ), w( w ) ) { } )
				
		static inline FAST_VEC_TYPE fConstruct( FAST_VEC_XYZW_C( t x, t y, t z, t w ) ) { return FAST_VEC_TYPE( FAST_VEC_XYZW_C( x, y, z, w ) ); }

		// Component accessors.
		t& fAxis ( u32 index )
		{
			switch( index )
			{
			case 0: return x;
			IF_Y( case 1: return y; )
			IF_Z( case 2: return z; )
			IF_W( case 3: return w; )
			}
			sigassert( !"axis out of bounds" );
			return x;
		}

		const t& fAxis ( u32 index ) const
		{
			switch( index )
			{
			case 0: return x;
			IF_Y( case 1: return y; )
			IF_Z( case 2: return z; )
			IF_W( case 3: return w; )
			}
			sigassert( !"axis out of bounds" );
			return x;
		}

		inline t&			operator[]( const u32 i )			{ return fAxis( i ); }
		inline const t&		operator[]( const u32 i ) const 	{ return fAxis( i ); }

		inline b32			operator==( const FAST_VEC_TYPE& rhs ) const { return fEqual( rhs ); }
		inline b32			operator!=( const FAST_VEC_TYPE& rhs ) const { return !fEqual( rhs ); }

		inline FAST_VEC_TYPE operator-( ) const
		{
			sig_assertvecvalid( *this );
			return FAST_VEC_TYPE( FAST_VEC_XYZW_C( -x, -y, -z, -w ) );
		}

		inline FAST_VEC_TYPE fCopy( ) { return *this; }

		inline t fLengthSquared( ) const
		{
			return FAST_VEC_XYZW( x*x, + y*y, + z*z, + w*w );
		}

		inline t fLength( ) const
		{
			return (t)fSqrt( (f32)fLengthSquared( ) );
		}

		inline t fDot( const FAST_VEC_TYPE& rhs ) const
		{
			sig_assertvecvalid( *this );
			sig_assertvecvalid( rhs );
			return FAST_VEC_XYZW( x*rhs.x, + y*rhs.y, + z*rhs.z, + w*rhs.w );
		}

		inline t				fDistance( const FAST_VEC_TYPE& rhs ) const
		{
			sig_assertvecvalid( *this );
			sig_assertvecvalid( rhs );
			return ( *this - rhs ).fLength( );
		}

		inline t				fDistanceSquared( const FAST_VEC_TYPE& rhs ) const
		{
			sig_assertvecvalid( *this );
			sig_assertvecvalid( rhs );
			return ( *this - rhs ).fLengthSquared( );
		}

		inline FAST_VEC_TYPE&	fNormalize( )
		{
			sig_assertvecvalid( *this );
			const t l = fLength( );
			sigassert( !Sig::fEqual( l, t(0), cVectorEqualLengthEpsilon ) );
			return( (*this) /= l );
		}
		inline FAST_VEC_TYPE&	fNormalize( t& l )
		{
			sig_assertvecvalid( *this );
			l = fLength( );
			sigassert( !Sig::fEqual( l, t(0), cVectorEqualLengthEpsilon ) );
			return( (*this) /= l );
		}

		inline FAST_VEC_TYPE&	fNormalizeSafe( )
		{
			sig_assertvecvalid( *this );
			const t l = fLength( );
			if( !Sig::fEqual( l, t(0), cVectorEqualLengthEpsilon ) )
				return( (*this) /= l );
			return (*this);
		}
		inline FAST_VEC_TYPE&	fNormalizeSafe( t& l )
		{
			sig_assertvecvalid( *this );
			l = fLength( );
			if( !Sig::fEqual( l, t(0), cVectorEqualLengthEpsilon ) )
				return( (*this) /= l );
			return (*this);
		}

		inline FAST_VEC_TYPE&	fNormalizeSafe( const FAST_VEC_TYPE& setToIfZero )
		{
			sig_assertvecvalid( *this );
			const t l = fLength( );
			if( Sig::fEqual( l, t(0), cVectorEqualLengthEpsilon ) )
				return( (*this) = setToIfZero );
			else
				return( (*this) /= l );
		}
		inline FAST_VEC_TYPE&	fNormalizeSafe( const FAST_VEC_TYPE& setToIfZero, t& l )
		{
			sig_assertvecvalid( *this );
			l = fLength( );
			if( Sig::fEqual( l, t(0), cVectorEqualLengthEpsilon ) )
				return( (*this) = setToIfZero );
			else
				return( (*this) /= l );
		}

		inline FAST_VEC_TYPE&	fSetLength( const t& l )
		{
			sig_assertvecvalid( *this );
			fNormalizeSafe( );
			return operator*=( *this, l );
		}

		inline FAST_VEC_TYPE&	fClampLength( const t& l )
		{
			sig_assertvecvalid( *this );	
			log_assert( l >= 0.0f, "fClampLength(" << l << "): Negative/NaN values invalid" );
			t len = fLength( );
			if( !Sig::fEqual( l, t(0), cVectorEqualLengthEpsilon ) && len > l )
				operator*=( *this, l / len );
			return (*this);
		}

		inline FAST_VEC_TYPE	fInverse( ) const
		{
			FAST_VEC_TYPE o( FAST_VEC_XYZW_C( FAST_VEC_INV( x ), FAST_VEC_INV( y ), FAST_VEC_INV( z ), FAST_VEC_INV( w ) ) );
			sig_assertvecvalid( o );
			return o;
		}

		inline b32				fIsZero( const f32 epsilon=cVectorEqualElementEpsilon ) const
		{
			sig_assertvecvalid( *this );

			if( !Sig::fEqual<t,t>( x, t(0), epsilon ) ) return false;
			IF_Y( if( !Sig::fEqual<t,t>( y, t(0), epsilon ) ) return false; )
			IF_Z( if( !Sig::fEqual<t,t>( z, t(0), epsilon ) ) return false; )
			IF_W( if( !Sig::fEqual<t,t>( w, t(0), epsilon ) ) return false; )

			return true;
		}

		inline b32				fIsAnyZero( const f32 epsilon=cVectorEqualElementEpsilon ) const
		{
			sig_assertvecvalid( *this );

			if( Sig::fEqual<t,t>( x, t(0), epsilon ) ) return true;
			IF_Y( if( Sig::fEqual<t,t>( y, t(0), epsilon ) ) return true; )
			IF_Z( if( Sig::fEqual<t,t>( z, t(0), epsilon ) ) return true; )
			IF_W( if( Sig::fEqual<t,t>( w, t(0), epsilon ) ) return true; )

			return false;
		}

		inline b32				fIsNan( ) const
		{
			if( x != x ) return true;
			IF_Y( if( y != y ) return true; )
			IF_Z( if( z != z ) return true; )
			IF_W( if( w != w ) return true; )

			return false;
		}

		inline b32				fEqual( const FAST_VEC_TYPE& rhs, const f32 epsilon=cVectorEqualElementEpsilon ) const
		{
			sig_assertvecvalid( *this );
			sig_assertvecvalid( rhs );

			if( !Sig::fEqual<t,t>( x, rhs.x, epsilon ) ) return false;
			IF_Y( if( !Sig::fEqual<t,t>( y, rhs.y, epsilon ) ) return false; )
			IF_Z( if( !Sig::fEqual<t,t>( z, rhs.z, epsilon ) ) return false; )
			IF_W( if( !Sig::fEqual<t,t>( w, rhs.w, epsilon ) ) return false; )

			return true;
		}

		inline t				fMax( ) const
		{
			sig_assertvecvalid( *this );
			t o = fAxis( 0 );
			for( u32 i = 1; i < cDimension; ++i )
				o = Sig::fMax( fAxis( i ), o );
			return o;
		}
		inline u32				fMaxAxisIndex( ) const
		{
			sig_assertvecvalid( *this );
			t   v = fAxis( 0 );
			u32 o = 0;
			for( u32 i = 1; i < cDimension; ++i )
			{
				if( fAxis( i ) > v )
				{
					v = fAxis( i );
					o = i;
				}
			}
			return o;
		}
		inline u32				fMaxAxisMagnitudeIndex( t& magOut ) const
		{
			sig_assertvecvalid( *this );
			magOut = Sig::fAbs( fAxis( 0 ) );
			u32 o = 0;
			for( u32 i = 1; i < cDimension; ++i )
			{
				t newV = Sig::fAbs( fAxis( i ) );
				if( newV > magOut )
				{
					magOut = newV;
					o = i;
				}
			}
			return o;
		}
		inline u32				fMaxAxisMagnitudeIndex( ) const
		{
			t mag;
			return fMaxAxisMagnitudeIndex( mag );
		}
		inline t				fMaxMagnitude( ) const
		{
			sig_assertvecvalid( *this );
			t o = Sig::fAbs( fAxis( 0 ) );
			for( u32 i = 1; i < cDimension; ++i )
				o = Sig::fMax( Sig::fAbs( fAxis( i ) ), o );
			return o;
		}

		inline t				fMin( ) const
		{
			sig_assertvecvalid( *this );
			t o = fAxis( 0 );
			for( u32 i = 1; i < cDimension; ++i )
				o = Sig::fMin( fAxis( i ), o );
			return o;
		}
		inline u32				fMinAxisIndex( ) const
		{
			sig_assertvecvalid( *this );
			t   v = fAxis( 0 );
			u32 o = 0;
			for( u32 i = 1; i < cDimension; ++i )
			{
				if( fAxis( i ) < v )
				{
					v = fAxis( i );
					o = i;
				}
			}
			return o;
		}
		inline t				fMinMagnitude( ) const
		{
			sig_assertvecvalid( *this );
			t o = Sig::fAbs( fAxis( 0 ) );
			for( u32 i = 1; i < cDimension; ++i )
				o = Sig::fMin( Sig::fAbs( fAxis( i ) ), o );
			return o;
		}
		inline u32				fMinAxisMagnitudeIndex( t& magOut ) const
		{
			sig_assertvecvalid( *this );
			magOut = Sig::fAbs( fAxis( 0 ) );
			u32 o = 0;
			for( u32 i = 1; i < cDimension; ++i )
			{
				t newV = Sig::fAbs( fAxis( i ) );
				if( newV < magOut )
				{
					magOut = newV;
					o = i;
				}
			}
			return o;
		}
		inline u32				fMinAxisMagnitudeIndex( ) const
		{
			t mag;
			return fMinAxisMagnitudeIndex( mag );
		}

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			for( u32 i = 0; i < cDimension; ++i )
				archive.fSaveLoad( fAxis( i ) );
		}

		inline FAST_VEC_TYPE fClone( ) const
		{
			sig_assertvecvalid( *this );
			return FAST_VEC_TYPE( FAST_VEC_XYZW_C( x, y, z, w ) );
		}

		// Dimension specific operations.
#if (FAST_VEC_SIZE == 2) 
		///
		/// \brief Compute the angle of the vector around the z-axis in radians. 
		/// \note The angle is returned in the range [0,2pi); example vectors and their angle:
		///		(1,0) => 0, 
		///		(0,1) => pi/2
		///		(-1,0) => pi
		///		(0,-1) => 3pi/2
		inline f32 fAngle( ) const
		{
			const f32 theta = fAtan2( (f32)y, (f32)x );
			return theta < 0.f ? Math::c2Pi + theta : theta;
		}
#elif (FAST_VEC_SIZE == 3) 
		// Constructor
		inline FAST_VEC_TYPE( const FAST_VEC_TYPE2<t>& _v, const t& _z ) : x(_v.x), y(_v.y), z(_z) { sig_assertvecvalid( *this ); }

		inline FAST_VEC_TYPE fCross( const FAST_VEC_TYPE& rhs ) const { return FAST_VEC_TYPE( y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x ); }
		inline FAST_VEC_TYPE2<t> fXY( ) const { return FAST_VEC_TYPE2<t>( x, y ); }
		inline FAST_VEC_TYPE2<t> fXZ( ) const { return FAST_VEC_TYPE2<t>( x, z ); }
		inline FAST_VEC_TYPE2<t> fYZ( ) const { return FAST_VEC_TYPE2<t>( y, z ); }
		inline FAST_VEC_TYPE& fProjectToXZ( ) { sig_assertvecvalid( *this ); y = t(0); return *this; }
		inline FAST_VEC_TYPE& fProjectToXZAndNormalize( ) { sig_assertvecvalid( *this ); return fProjectToXZ( ).fNormalizeSafe( FAST_VEC_TYPE3::cZAxis ); }

		inline t fXZLength( ) const { return FAST_VEC_TYPE2<t>( x, z ).fLength( ); }
		inline t fXZLengthSquared( ) const { return FAST_VEC_TYPE2<t>( x, z ).fLengthSquared( ); }
		inline t fXZDistance( const FAST_VEC_TYPE& rhs ) const { return ( *this - rhs ).fXZLength( ); }
		inline t fXZDistanceSquared( const FAST_VEC_TYPE& rhs ) const { return ( *this - rhs ).fXZLengthSquared( ); }

		inline t fXZHeading( ) const { return (t)fAtan2( (f32)x, (f32)z ); } //using f32 to remove ambiguity with int types
		inline void fSetXZHeading( t heading ) { x = (t)fSin( (f32)heading ); y = 0; z = (t)fCos( (f32)heading ); }

		inline FAST_VEC_TYPE fBuildPerp( ) const
		{
			sigassert( !fIsZero( ) );

			// Build a vector that points in a direction that we mostly don't
			const u32 minMagAxisIndex = fMinAxisMagnitudeIndex( );
			FAST_VEC_TYPE other = cZeroVector;
			other[ minMagAxisIndex ] = t( 1 );

			// To guarantee perp we then do a cross with this vector
			return fCross( other ).fNormalize( );
		}

#elif (FAST_VEC_SIZE == 4)
		// Constructor
		inline FAST_VEC_TYPE( const FAST_VEC_TYPE3<t>& _v, const t& _w ) : x(_v.x), y(_v.y), z(_v.z), w(_w) { sig_assertvecvalid( *this ); }

		inline FAST_VEC_TYPE2<t> fXY( ) const { return FAST_VEC_TYPE2<t>( x, y ); }
		inline FAST_VEC_TYPE3<t> fXYZ( ) const { return FAST_VEC_TYPE3<t>( x, y, z ); }
		inline FAST_VEC_TYPE fDivideByW( ) const { return FAST_VEC_TYPE( x / w, y / w, z / w, ( t )1 ); }
#endif

		// Unary function forms of operators. For script.
		inline FAST_VEC_TYPE fAdd( const FAST_VEC_TYPE& rhs ) const { return *this + rhs; }
		inline FAST_VEC_TYPE fSub( const FAST_VEC_TYPE& rhs ) const { return *this - rhs; }
		inline FAST_VEC_TYPE fMul( t rhs ) const { return *this * rhs; }
		inline FAST_VEC_TYPE fDiv( t rhs ) const { return *this / rhs; }
		inline FAST_VEC_TYPE fNeg( ) const { return - *this; }

	};

	template<class t> const FAST_VEC_TYPE<t> FAST_VEC_TYPE<t>::cZeroVector( t(0.) );
	template<class t> const FAST_VEC_TYPE<t> FAST_VEC_TYPE<t>::cOnesVector( t(1.) );

	template<class t> const FAST_VEC_TYPE<t> FAST_VEC_TYPE<t>::cXAxis( FAST_VEC_XYZW_C( t(1.), t(0.), t(0.), t(0.) ) );
	IF_Y( template<class t> const FAST_VEC_TYPE<t> FAST_VEC_TYPE<t>::cYAxis( FAST_VEC_XYZW_C( t(0.), t(1.), t(0.), t(0.) ) ); )
	IF_Z( template<class t> const FAST_VEC_TYPE<t> FAST_VEC_TYPE<t>::cZAxis( FAST_VEC_XYZW_C( t(0.), t(0.), t(1.), t(0.) ) ); )
	IF_W( template<class t> const FAST_VEC_TYPE<t> FAST_VEC_TYPE<t>::cWAxis( FAST_VEC_XYZW_C( t(0.), t(0.), t(0.), t(1.) ) ); )

#define binary_op( type, op )	template< typename t > inline type<t> operator op ( const type<t>& a, const type<t>& b ) { return type<t>( FAST_VEC_XYZW_C(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w) ); } \
	template< typename t > inline type<t>& operator op= ( type<t>& a, const type<t>& b ) { FAST_VEC_XYZW( a.x op= b.x;, a.y op= b.y;, a.z op= b.z;, a.w op= b.w;) return a; }
	binary_op( FAST_VEC_TYPE, * );
	binary_op( FAST_VEC_TYPE, / );
	binary_op( FAST_VEC_TYPE, + );
	binary_op( FAST_VEC_TYPE, - );

#define scalar_op( type, op )	template< typename t, typename s > inline type<t> operator op ( const type<t>& a, const s& b ) { return type<t>( FAST_VEC_XYZW_C(a.x op b, a.y op b, a.z op b, a.w op b) ); } \
	template< typename t, typename s > inline type<t> operator op ( const s& b, const type<t>& a ) { return type<t>( FAST_VEC_XYZW_C(b op a.x, b op a.y, b op a.z, b op a.w) ); } \
	template< typename t, typename s > inline type<t>& operator op= ( type<t>& a, const s& b ) { FAST_VEC_XYZW(a.x op= b;, a.y op= b;, a.z op= b;, a.w op= b;) return a; }
	scalar_op( FAST_VEC_TYPE, * );
	scalar_op( FAST_VEC_TYPE, / );
	scalar_op( FAST_VEC_TYPE, + );
	scalar_op( FAST_VEC_TYPE, - );

#undef binary_op
#undef scalar_op
#undef IF_Y
#undef IF_Z
#undef IF_W
#pragma warning( default : 4146 )

//} }


//#endif//__tFastVectorImp__
