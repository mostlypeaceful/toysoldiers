#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tVector__
#define __tVector__

namespace Sig { namespace Math
{
	static const f32 cVectorZeroEpsilon = 0.000001f;

	///
	/// \brief Base vector class providing all vector math ops for derived classes;
	/// parameterized both by dimension and underlying number type (i.e., int, float, double, etc).
	template<class t, int N, class derived>
	class tBaseVector
	{
		declare_null_reflector( );

	public:

		enum { cDimension = N };

		inline derived*			fConvertPtr( )						{ return static_cast<derived*>( this ); }
		inline const derived*	fConvertPtr( ) const				{ return static_cast<const derived*>( this ); }
		inline derived&			fConvertRef( )						{ return *static_cast<derived*>( this ); }
		inline const derived&	fConvertRef( ) const				{ return *static_cast<const derived*>( this ); }

		inline t&				fAxis( u32 which )					{ sigassert(which<N); return *((t*)this + which); }
		inline const t&			fAxis( u32 which ) const			{ sigassert(which<N); return *((t*)this + which); }

		inline t&				operator[]( const u32 i )			{ return fAxis( i ); }
		inline const t&			operator[]( const u32 i ) const 	{ return fAxis( i ); }

		inline derived&			operator+=( const derived& rhs )
		{
			for( u32 i = 0; i < N; ++i )
				fAxis( i ) += rhs.fAxis( i );
			sig_assertvecvalid( *this );
			return fConvertRef( );
		}
		inline derived			operator+( const derived& rhs ) const
		{
			derived o;
			for( u32 i = 0; i < N; ++i )
				o.fAxis( i ) = fAxis( i ) + rhs.fAxis( i );
			sig_assertvecvalid( o );
			return o;
		}
		inline derived&			operator-=( const derived& rhs )
		{
			for( u32 i = 0; i < N; ++i )
				fAxis( i ) -= rhs.fAxis( i );
			sig_assertvecvalid( *this );
			return fConvertRef( );
		}
		inline derived			operator-( const derived& rhs ) const
		{
			derived o;
			for( u32 i = 0; i < N; ++i )
				o.fAxis( i ) = fAxis( i ) - rhs.fAxis( i );
			sig_assertvecvalid( o );
			return o;
		}
		inline derived&			operator*=( const derived& rhs )
		{
			for( u32 i = 0; i < N; ++i )
				fAxis( i ) *= rhs.fAxis( i );
			sig_assertvecvalid( *this );
			return fConvertRef( );
		}
		inline derived			operator*( const derived& rhs ) const
		{
			derived o;
			for( u32 i = 0; i < N; ++i )
				o.fAxis( i ) = fAxis( i ) * rhs.fAxis( i );
			sig_assertvecvalid( o );
			return o;
		}
		inline derived&			operator/=( const derived& rhs )
		{
			for( u32 i = 0; i < N; ++i )
				fAxis( i ) /= rhs.fAxis( i );
			sig_assertvecvalid( *this );
			return fConvertRef( );
		}
		inline derived			operator/( const derived& rhs ) const
		{
			derived o;
			for( u32 i = 0; i < N; ++i )
				o.fAxis( i ) = fAxis( i ) / rhs.fAxis( i );
			sig_assertvecvalid( o );
			return o;
		}

		inline derived			operator*( const t scalar ) const
		{
			derived o = fConvertRef( );
			for( u32 i = 0; i < N; ++i )
				o.fAxis( i ) *= scalar;
			sig_assertvecvalid( o );
			return o;
		}
		inline derived&			operator*=( const t scalar )
		{
			for( u32 i = 0; i < N; ++i )
				fAxis( i ) *= scalar;
			sig_assertvecvalid( *this );
			return fConvertRef( );
		}
		inline friend derived	operator*( const t scalar, const derived& rhs )
		{
			derived o = rhs;
			for( u32 i = 0; i < N; ++i )
				o.fAxis( i ) *= scalar;
			sig_assertvecvalid( o );
			return o;
		}

		inline derived			operator/( const t scalar ) const
		{
			derived o = fConvertRef( );
			for( u32 i = 0; i < N; ++i )
				o.fAxis( i ) /= scalar;
			sig_assertvecvalid( o );
			return o;
		}
		inline derived&			operator/=( const t scalar )
		{
			for( u32 i = 0; i < N; ++i )
				fAxis( i ) /= scalar;
			sig_assertvecvalid( *this );
			return fConvertRef( );
		}
		inline friend derived	operator/( const t scalar, const derived& rhs )
		{
			derived o;
			for( u32 i = 0; i < N; ++i )
				o.fAxis( i ) = scalar / rhs.fAxis( i );
			sig_assertvecvalid( o );
			return o;
		}

		inline derived			operator-( ) const
		{
			derived o;
			for( u32 i = 0; i < N; ++i )
				o.fAxis( i ) = -fAxis( i );
			sig_assertvecvalid( o );
			return o;
		}

		inline b32				operator==( const derived& rhs ) const
		{
			return fEqual( rhs );
		}

		inline b32				operator!=( const derived& rhs ) const
		{
			return !fEqual( rhs );
		}

		inline t				fDot( const derived& rhs ) const
		{
			sig_assertvecvalid( *this );
			sig_assertvecvalid( rhs );
			t o = 0;
			for( u32 i = 0; i < N; ++i )
				o += fAxis( i ) * rhs.fAxis( i );
			return o;
		}

		inline t				fLength( ) const
		{
			sig_assertvecvalid( *this );
			return ( t )fSqrt( ( f32 )fLengthSquared( ) );
		}

		inline t				fLengthSquared( ) const
		{
			return fDot( fConvertRef( ) );
		}

		inline t				fDistance( const derived& rhs ) const
		{
			sig_assertvecvalid( *this );
			sig_assertvecvalid( rhs );
			return ( *this - rhs ).fLength( );
		}

		inline derived&			fNormalize( )
		{
			sig_assertvecvalid( *this );
			const t l = fLength( );
			sigassert( !Sig::fEqual( l, t(0), cVectorZeroEpsilon ) );
			return( (*this) /= l );
		}
		inline derived&			fNormalize( t& l )
		{
			sig_assertvecvalid( *this );
			l = fLength( );
			sigassert( !Sig::fEqual( l, t(0), cVectorZeroEpsilon ) );
			return( (*this) /= l );
		}

		inline derived&			fNormalizeSafe( )
		{
			sig_assertvecvalid( *this );
			const t l = fLength( );
			if( !Sig::fEqual( l, t(0), cVectorZeroEpsilon ) )
				return( (*this) /= l );
			return fConvertRef( );
		}
		inline derived&			fNormalizeSafe( t& l )
		{
			sig_assertvecvalid( *this );
			l = fLength( );
			if( !Sig::fEqual( l, t(0), cVectorZeroEpsilon ) )
				return( (*this) /= l );
			return fConvertRef( );
		}

		inline derived&			fNormalizeSafe( const derived& setToIfZero )
		{
			sig_assertvecvalid( *this );
			const t l = fLength( );
			if( Sig::fEqual( l, t(0), cVectorZeroEpsilon ) )
				return( fConvertRef( ) = setToIfZero );
			else
				return( (*this) /= l );
		}
		inline derived&			fNormalizeSafe( const derived& setToIfZero, t& l )
		{
			sig_assertvecvalid( *this );
			l = fLength( );
			if( Sig::fEqual( l, t(0), cVectorZeroEpsilon ) )
				return( fConvertRef( ) = setToIfZero );
			else
				return( (*this) /= l );
		}

		inline derived&			fSetLength( const t& l )
		{
			sig_assertvecvalid( *this );
			fNormalizeSafe( );
			operator*=( l );
			return fConvertRef( );
		}

		inline derived&			fClampLength( const t& l )
		{
			sig_assertvecvalid( *this );	
			sigassert( l >= 0.0f );
			f32 len = fLength( );
			if( len > l )
				operator*=( l / len );
			return fConvertRef( );
		}

		inline derived			fInverse( ) const
		{
			derived o;
			for( u32 i = 0; i < N; ++i )
			{
				const t v = fAxis( i );
				o.fAxis( i ) = v ? ( 1.f / v ) : Math::cInfinity;
			}
			sig_assertvecvalid( o );
			return o;
		}

		inline b32				fIsZero( const f32 epsilon=0.00001f ) const
		{
			sig_assertvecvalid( *this );
			for( u32 i = 0; i < N; ++i )
				if( !Sig::fEqual<t,t>( fAxis( i ), t(0), epsilon ) )
					return false;
			return true;
		}
		inline b32				fIsNan( ) const
		{
			for( u32 i = 0; i < N; ++i )
				if( fAxis( i ) != fAxis( i ) )
					return true;
			return false;
		}

		inline b32				fEqual( const derived& rhs, const f32 epsilon=0.00001f ) const
		{
			sig_assertvecvalid( *this );
			sig_assertvecvalid( rhs );
			for( u32 i = 0; i < N; ++i )
				if( !Sig::fEqual<t,t>( fAxis( i ), rhs.fAxis( i ), epsilon ) )
					return false;
			return true;
		}

		inline t				fMax( ) const
		{
			sig_assertvecvalid( *this );
			t o = fAxis( 0 );
			for( u32 i = 1; i < N; ++i )
				o = Sig::fMax( fAxis( i ), o );
			return o;
		}
		inline u32				fMaxAxisIndex( ) const
		{
			sig_assertvecvalid( *this );
			t   v = fAxis( 0 );
			u32 o = 0;
			for( u32 i = 1; i < N; ++i )
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
			for( u32 i = 1; i < N; ++i )
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
			for( u32 i = 1; i < N; ++i )
				o = Sig::fMax( Sig::fAbs( fAxis( i ) ), o );
			return o;
		}

		inline t				fMin( ) const
		{
			sig_assertvecvalid( *this );
			t o = fAxis( 0 );
			for( u32 i = 1; i < N; ++i )
				o = Sig::fMin( fAxis( i ), o );
			return o;
		}
		inline u32				fMinAxisIndex( ) const
		{
			sig_assertvecvalid( *this );
			t   v = fAxis( 0 );
			u32 o = 0;
			for( u32 i = 1; i < N; ++i )
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
			for( u32 i = 1; i < N; ++i )
				o = Sig::fMin( Sig::fAbs( fAxis( i ) ), o );
			return o;
		}
		inline u32				fMinAxisMagnitudeIndex( t& magOut ) const
		{
			sig_assertvecvalid( *this );
			magOut = Sig::fAbs( fAxis( 0 ) );
			u32 o = 0;
			for( u32 i = 1; i < N; ++i )
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
			for( u32 i = 0; i < N; ++i )
				archive.fSaveLoad( fAxis( i ) );
		}

		inline derived fClone( ) const
		{
			derived o;
			for( u32 i = 0; i < N; ++i )
				o.fAxis( i ) = fAxis( i );
			sig_assertvecvalid( o );
			return o;
		}
	};

	///
	/// \brief 1-D Vector type, parameterized by number type (i.e., int, float, etc.).
	template<class t>
	class tVector1 : public tBaseVector<t, 1, tVector1<t> >
	{
		declare_reflector( );
	public:
		typedef tBaseVector<t, 1, tVector1<t> > tBase;
	public:
		t x;
	public:
		inline tVector1( ) { sig_setvecinvalid( t, x, tBase::cDimension ); }
		inline tVector1( tNoOpTag ) { }
		inline tVector1( const t& _x ) : x(_x) { sig_assertvecvalid( *this ); }
	};

	///
	/// \brief 2-D Vector type, parameterized by number type (i.e., int, float, etc.).
	template<class t>
	class tVector2 : public tBaseVector<t, 2, tVector2<t> >
	{
		declare_reflector( );
		sig_make_loggable( tVector2, "(" << x << ", " << y << ")" );
	public:
		typedef tBaseVector<t, 2, tVector2<t> > tBase;
	public:
		static const tVector2 cZeroVector;
		static const tVector2 cOnesVector;
		static const tVector2 cXAxis;
		static const tVector2 cYAxis;
		static inline tVector2 fConstruct( t x, t y ) { return tVector2( x, y ); }
	public:
		t x,y;
	public:
		inline tVector2( ) { sig_setvecinvalid( t, x, tBase::cDimension ); }
		inline tVector2( tNoOpTag ) { }
		inline tVector2( const t& _all ) : x(_all), y(_all) { sig_assertvecvalid( *this ); }
		inline tVector2( const t& _x, const t& _y ) : x(_x), y(_y) { sig_assertvecvalid( *this ); }
				
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

	};
	template<class t>
	const tVector2<t> tVector2<t>::cZeroVector( t(0.) );
	template<class t>
	const tVector2<t> tVector2<t>::cOnesVector( t(1.) );
	template<class t>
	const tVector2<t> tVector2<t>::cXAxis( t(1.), t(0.) );
	template<class t>
	const tVector2<t> tVector2<t>::cYAxis( t(0.), t(1.) );

	///
	/// \brief 3-D Vector type, parameterized by number type (i.e., int, float, etc.).
	template<class t>
	class tVector3 : public tBaseVector<t, 3, tVector3<t> >
	{
		declare_reflector( );
		define_class_pool_new_delete( tVector3, 256 );
		sig_make_loggable( tVector3, "(" << x << ", " << y << ", " << z << ")" );
	public:
		typedef tBaseVector<t, 3, tVector3<t> > tBase;
	public:
		static const tVector3 cZeroVector;
		static const tVector3 cOnesVector;
		static const tVector3 cXAxis;
		static const tVector3 cYAxis;
		static const tVector3 cZAxis;
		static inline tVector3 fConstruct( t x, t y, t z ) { return tVector3( x, y, z ); }
	public:
		t x,y,z;
	public:
		inline tVector3( ) { sig_setvecinvalid( t, x, tBase::cDimension ); }
		inline tVector3( tNoOpTag ) { }
		inline tVector3( const t& _all ) : x(_all), y(_all), z(_all) { sig_assertvecvalid( *this ); }
		inline tVector3( const t& _x, const t& _y, const t& _z ) : x(_x), y(_y), z(_z) { sig_assertvecvalid( *this ); }
		inline tVector3( const tVector2<t>& _v, const t& _z ) : x(_v.x), y(_v.y), z(_z) { sig_assertvecvalid( *this ); }
		inline tVector3 fCross( const tVector3& rhs ) const { return tVector3( y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x ); }
		inline tVector2<t> fXY( ) const { return tVector2<t>( x, y ); }
		inline tVector2<t> fXZ( ) const { return tVector2<t>( x, z ); }
		inline tVector2<t> fYZ( ) const { return tVector2<t>( y, z ); }
		inline tVector3& fProjectToXZ( ) { sig_assertvecvalid( *this ); y = t(0); return *this; }
		inline tVector3& fProjectToXZAndNormalize( ) { sig_assertvecvalid( *this ); return fProjectToXZ( ).fNormalizeSafe( tVector3::cZAxis ); }

		inline t fXZLength( ) const { return tVector2<t>( x, z ).fLength( ); }
		inline t fXZLengthSquared( ) const { return tVector2<t>( x, z ).fLengthSquared( ); }
		inline t fXZDistance( const tVector3& rhs ) const { return ( *this - rhs ).fXZLength( ); }

		inline t fXZHeading( ) const { return (t)fAtan2( (f32)x, (f32)z ); } //using f32 to remove ambiguity with int types
		inline void fSetXZHeading( t heading ) { x = (t)fSin( (f32)heading ); y = 0; z = (t)fCos( (f32)heading ); }
	};
	template<class t>
	const tVector3<t> tVector3<t>::cZeroVector( t(0.) );
	template<class t>
	const tVector3<t> tVector3<t>::cOnesVector( t(1.) );
	template<class t>
	const tVector3<t> tVector3<t>::cXAxis( t(1.), t(0.), t(0.) );
	template<class t>
	const tVector3<t> tVector3<t>::cYAxis( t(0.), t(1.), t(0.) );
	template<class t>
	const tVector3<t> tVector3<t>::cZAxis( t(0.), t(0.), t(1.) );

	template<class t>
	inline tVector3<t> fToRadians( const tVector3<t>& degrees )
	{
		return tVector3<t>( fToRadians( degrees.x ), fToRadians( degrees.y ), fToRadians( degrees.z ) );
	}

	template<class t>
	inline tVector3<t> fToDegrees( const tVector3<t>& radians )
	{
		return tVector3<t>( fToDegrees( radians.x ), fToDegrees( radians.y ), fToDegrees( radians.z ) );
	}

	///
	/// \brief 4-D Vector type, parameterized by number type (i.e., int, float, etc.).
	template<class t>
	class tVector4 : public tBaseVector<t, 4, tVector4<t> >
	{
		declare_reflector( );
		sig_make_loggable( tVector4, "(" << x << ", " << y << ", " << z << ", " << w << ")" );
	public:
		typedef tBaseVector<t, 4, tVector4<t> > tBase;
	public:
		static const tVector4 cZeroVector;
		static const tVector4 cOnesVector;
		static inline tVector4 fConstruct( t x, t y, t z, t w ) { return tVector4( x, y, z, w ); }
	public:
		t x,y,z,w;
	public:
		inline tVector4( ) { sig_setvecinvalid( t, x, tBase::cDimension ); }
		inline tVector4( tNoOpTag ) { }
		inline tVector4( const t& _all ) : x(_all), y(_all), z(_all), w(_all) { sig_assertvecvalid( *this ); }
		inline tVector4( const t& _x, const t& _y, const t& _z, const t& _w ) : x(_x), y(_y), z(_z), w(_w) { sig_assertvecvalid( *this ); }
		inline tVector4( const tVector3<t>& _v, const t& _w ) : x(_v.x), y(_v.y), z(_v.z), w(_w) { sig_assertvecvalid( *this ); }
		inline tVector2<t> fXY( ) const { return tVector2<t>( x, y ); }
		inline tVector3<t> fXYZ( ) const { return tVector3<t>( x, y, z ); }
		inline tVector4<t> fDivideByW( ) const { return tVector4<t>( x / w, y / w, z / w, ( t )1 ); }
	};
	template<class t>
	const tVector4<t> tVector4<t>::cZeroVector( t(0.) );
	template<class t>
	const tVector4<t> tVector4<t>::cOnesVector( t(1.) );

	typedef tVector1<f32> tVec1f;
	typedef tVector2<f32> tVec2f;
	typedef tVector3<f32> tVec3f;
	typedef tVector4<f32> tVec4f;

	class tNoOpVec2f : public tVec2f
	{
		declare_reflector( );
	public:
		tNoOpVec2f( ) : tVec2f( cNoOpTag ) { }
		tNoOpVec2f( const tVec2f& v ) : tVec2f( v ) { }
	};
	class tNoOpVec3f : public tVec3f
	{
		declare_reflector( );
	public:
		tNoOpVec3f( ) : tVec3f( cNoOpTag ) { }
		tNoOpVec3f( const tVec3f& v ) : tVec3f( v ) { }
	};
	class tNoOpVec4f : public tVec4f
	{
		declare_reflector( );
	public:
		tNoOpVec4f( ) : tVec4f( cNoOpTag ) { }
		tNoOpVec4f( const tVec4f& v ) : tVec4f( v ) { }
	};

	typedef tVector1<f64> tVec1d;
	typedef tVector2<f64> tVec2d;
	typedef tVector3<f64> tVec3d;
	typedef tVector4<f64> tVec4d;

	typedef tVector1<s32> tVec1i;
	typedef tVector2<s32> tVec2i;
	typedef tVector3<s32> tVec3i;
	typedef tVector4<s32> tVec4i;

	typedef tVector1<u32> tVec1u;
	typedef tVector2<u32> tVec2u;
	typedef tVector3<u32> tVec3u;
	typedef tVector4<u32> tVec4u;

}}

namespace Sig
{
	template<class t>
	inline Math::tVector2<t> fMin( const Math::tVector2<t>& toClamp, const Math::tVector2<t>& max )
	{
		return Math::tVector2<t>(
			Sig::fMin( toClamp.x, max.x ),
			Sig::fMin( toClamp.y, max.y ) );
	}

	template<class t>
	inline Math::tVector3<t> fMin( const Math::tVector3<t>& toClamp, const Math::tVector3<t>& max )
	{
		return Math::tVector3<t>(
			Sig::fMin( toClamp.x, max.x ),
			Sig::fMin( toClamp.y, max.y ),
			Sig::fMin( toClamp.z, max.z ) );
	}

	template<class t>
	inline Math::tVector4<t> fMin( const Math::tVector4<t>& toClamp, const Math::tVector4<t>& max )
	{
		return Math::tVector4<t>(
			Sig::fMin( toClamp.x, max.x ),
			Sig::fMin( toClamp.y, max.y ),
			Sig::fMin( toClamp.z, max.z ),
			Sig::fMin( toClamp.w, max.w ) );
	}



	template<class t>
	inline Math::tVector2<t> fMax( const Math::tVector2<t>& toClamp, const Math::tVector2<t>& min )
	{
		return Math::tVector2<t>(
			Sig::fMax( toClamp.x, min.x ),
			Sig::fMax( toClamp.y, min.y ) );
	}

	template<class t>
	inline Math::tVector3<t> fMax( const Math::tVector3<t>& toClamp, const Math::tVector3<t>& min )
	{
		return Math::tVector3<t>(
			Sig::fMax( toClamp.x, min.x ),
			Sig::fMax( toClamp.y, min.y ),
			Sig::fMax( toClamp.z, min.z ) );
	}

	template<class t>
	inline Math::tVector4<t> fMax( const Math::tVector4<t>& toClamp, const Math::tVector4<t>& min )
	{
		return Math::tVector4<t>(
			Sig::fMax( toClamp.x, min.x ),
			Sig::fMax( toClamp.y, min.y ),
			Sig::fMax( toClamp.z, min.z ),
			Sig::fMax( toClamp.w, min.w ) );
	}



	template<class t>
	inline Math::tVector2<t> fClamp( const Math::tVector2<t>& toClamp, const Math::tVector2<t>& min, const Math::tVector2<t>& max )
	{
		return Math::tVector2<t>(
			Sig::fClamp( toClamp.x, min.x, max.x ),
			Sig::fClamp( toClamp.y, min.y, max.y ) );
	}

	template<class t>
	inline Math::tVector3<t> fClamp( const Math::tVector3<t>& toClamp, const Math::tVector3<t>& min, const Math::tVector3<t>& max )
	{
		return Math::tVector3<t>(
			Sig::fClamp( toClamp.x, min.x, max.x ),
			Sig::fClamp( toClamp.y, min.y, max.y ),
			Sig::fClamp( toClamp.z, min.z, max.z ) );
	}

	template<class t>
	inline Math::tVector4<t> fClamp( const Math::tVector4<t>& toClamp, const Math::tVector4<t>& min, const Math::tVector4<t>& max )
	{
		return Math::tVector4<t>(
			Sig::fClamp( toClamp.x, min.x, max.x ),
			Sig::fClamp( toClamp.y, min.y, max.y ),
			Sig::fClamp( toClamp.z, min.z, max.z ),
			Sig::fClamp( toClamp.w, min.w, max.w ) );
	}

}


#endif//__tVector__
