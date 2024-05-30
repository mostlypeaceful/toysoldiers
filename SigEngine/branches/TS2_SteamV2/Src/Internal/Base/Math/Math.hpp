#ifndef __Math__
#define __Math__

namespace Sig { namespace Math
{
	// script-specific
	void fExportScriptInterface( tScriptVm& vm );

	extern const f32 cInvalidFloat;
	base_export extern const f32 cAntiPodalQuatThreshold;
	static const f32 cInfinity	= 9999999999.f;
	static const f32 cPi		= (f32)3.1415926535897932384626433832795f;
	static const f32 c3PiOver2	= 3.f * cPi / 2.f;
	static const f32 c2Pi		= 2.f * cPi;
	static const f32 cPiOver2	= cPi / 2.f;
	static const f32 cPiOver4	= cPi / 4.f;
	static const f32 cPiOver8	= cPi / 8.f;

	inline b32 fIsValidFloat( f32 f ) { return f == f; }

#ifdef sig_debugfloat
	template<class t>
	class tSetFloatsInvalid
	{
	public: static void fInvalidate( t* fs, u32 numFloats ) { }
	};
	template<>
	class tSetFloatsInvalid<f32>
	{
	public: static void fInvalidate( f32* fs, u32 numFloats ) { for( u32 i = 0; i < numFloats; ++i ) fs[ i ] = cInvalidFloat; }
	};

#	define sig_setvecinvalid( t, x, count ) tSetFloatsInvalid<t>::fInvalidate( &(x), count )
#	define sig_assertvecvalid( vec ) sigassert( !(vec).fIsNan( ) )
	class tDebugF32
	{
	public:
		tDebugF32( ) : mValue( cInvalidFloat ) { }
		tDebugF32( f32 val ) : mValue( val ) { fAssertValid( ); }
		operator f32( ) const { fAssertValid( ); return mValue; }
		void fAssertValid( ) const { sigassert( fIsValidFloat( mValue ) ); }
	private:
		f32 mValue;
	};
#else//sig_debugfloat
#	define sig_setvecinvalid( t, x, count )
#	define sig_assertvecvalid( vec )
	typedef f32 tDebugF32;
#endif//sig_debugfloat

	template<class t>
	inline t fMetersToFeet( const t& meters )
	{
		return ( t )( meters * 3.280839f );
	}
	
	template<class t>
	inline t fFeetToMeters( const t& feet )
	{
		return ( t )( feet / 3.280839f );
	}

	template<class t>
	inline t fToRadians( const t& degrees )
	{
		return ( t )( ( cPi / 180.f ) * degrees );
	}

	template<class t>
	inline t fToDegrees( const t& radians )
	{
		return ( t )( ( 180.f / cPi ) * radians );
	}

	template<class t>
	inline t fSquare( const t& x )
	{
		return x * x;
	}

	template<class t>
	inline t fRemapZeroToOne( const t& a, const t& b, const t& amount )
	{
		return ( amount - a ) / ( b - a );
	}

	// takes a value from [zero, one], and makes it [min, one]
	template<class t>
	inline t fRemapMinimum( const t& a, const t& min )
	{
		t range = 1.f - min;
		return min + a * range;
	}
	
	template<class t, class u>
	inline t fLerp( const t& a, const t& b, const u& amount )
	{
		return (u(1)-amount) * a + amount * b;
	}

	template<class t, class u>
	inline t fNLerp( const t& a, const t& b, const u& amount )
	{
		return ( (u(1)-amount) * a + amount * b ).fNormalize( );
	}

	template<class t, class u>
	inline t fNLerpSafe( const t& a, const t& b, const u& amount, const t& ifZeroUseThis )
	{
		return ( (u(1)-amount) * a + amount * b ).fNormalizeSafe( ifZeroUseThis );
	}

	template<class t, class u>
	inline t fNLerpRotation( const t& a, const t& b, const u& amount )
	{
		if( !a.fAntiPodal( b ) )	return fNLerpSafe( a,  b, amount, t::cIdentity );
		else						return fNLerpSafe( a, -b, amount, t::cIdentity );
	}

	template<class t, class u>
	u fCatmullRom( const u& p0, const u& p1, const u& p2, const u& p3, t x )
	{
		const t x2 = x * x;
		const t x3 = x2 * x;
		const u m1 = t(0.5) * ( p2 - p0 );
		const u m2 = t(0.5) * ( p3 - p1 );
		const u o =  
			(2*x3 - 3*x2 + 1)*p1 +
			(x3 - 2*x2 + x)*m1 +
			(-2*x3 + 3*x2)*p2 +
			(x3 - x2)*m2;
		return o;
	}

	template<class t>
	inline t fProject( const t& projectOnto, const t& v )
	{
		return ( projectOnto.fDot( v ) / projectOnto.fLengthSquared( ) ) * projectOnto;
	}

	template<class t>
	inline t fProjectToUnit( const t& projectOnto, const t& v )
	{
		return ( projectOnto.fDot( v ) ) * projectOnto;
	}

	template<class t>
	inline t fReflect( const t& incident, const t& normal )
	{
		return incident - ( 2.f * incident.fDot( normal ) ) * normal;
	}

	template<class t>
	inline t fSqrt( t val )
	{
		sigassert( val >= -0.0f );
		return std::sqrt( val );
	}

	template<class t>
	inline t fPow( t val, t exp )
	{
		return std::pow( val, exp );
	}

	template<class t>
	inline t fSin( t fval )
	{
		return std::sin( fval );
	}

	template<class t>
	inline t fAsin( t fval )
	{
		return std::asin( fClamp<t>( fval, -1, +1 ) );
	}

	template<class t>
	inline t fCos( t fval )
	{
		return std::cos( fval );
	}

	template<class t>
	inline t fAcos( t fval )
	{
		return std::acos( fClamp<t>( fval, -1, +1 ) );
	}

	template<class t>
	inline t fTan( t fval )
	{
		return std::tan( fval );
	}

	template<class t>
	inline t fAtan( t fval )
	{
		return std::atan( fval );
	}

	template<class t>
	inline t fAtan2( t y, t x )
	{
		return std::atan2( y, x );
	}

	template<class t>
	inline t fModulus( t x, t div )
	{
		t result = x % div;
		if( result < 0 ) result += div;
		return result;
	}

	/// Input is two angles
	template<class t>
	inline t fShortestWayAround( t from, t to )
	{
		f32 delta = to - from;
		while( delta > cPi ) delta -= c2Pi;
		while( delta < -cPi ) delta += c2Pi;
		return delta;
	}
}}


#include "tVector.hpp"
#include "tQuaternion.hpp"
#include "MatrixUtil.hpp"
#include "tMatrix.hpp"
#include "tPRSXform.hpp"
#include "tSphere.hpp"
#include "tRay.hpp"
#include "tAabb.hpp"
#include "tObb.hpp"
#include "tPlane.hpp"
#include "tFrustum.hpp"
#include "tTriangle.hpp"
#include "tRect.hpp"
#include "tDamped.hpp"

#endif//__Math__
