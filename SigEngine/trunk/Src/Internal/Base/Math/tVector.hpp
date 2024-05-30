#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tVector__
#define __tVector__

namespace Sig { namespace Math
{
	static const f32 cVectorEqualLengthEpsilon     = 0.000001f;
	static const f32 cVectorEqualElementEpsilon    = 0.00001f;
	static const f32 cVectorEqualDotProductEpsilon = 0.00001f;
} }

#define sig_assert_unit_vector( v ) log_assert( ::Sig::fEqual( v.fLengthSquared( ), 1.0f ), "Vector [" << #v << "] not unit length: " << v << " (length: " << v.fLength( ) << ")" );

#include "tFastVector.hpp"

// By this point tVector[1-4] will have been defined.

namespace Sig { namespace Math
{

	typedef tVector1<f32> tVec1f;
	typedef tVector2<f32> tVec2f;
	typedef tVector3<f32> tVec3f;
	typedef tVector4<f32> tVec4f;

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



	// Vector math functions: For use on arbitrary sized vectors of data, not related to the tVector[n] types, directly.

	// a + b
	template<class t>
	void fVectorAdd( t * result, const t * a, const t * b, u32 dim )
	{
		for( u32 i = 0; i < dim; ++i )
			result[ i ] = a[ i ] + b[ i ];
	}

	// a - b
	template<class t>
	void fVectorSubtract( t * result, const t * a, const t * b, u32 dim )
	{
		for( u32 i = 0; i < dim; ++i )
			result[ i ] = a[ i ] - b[ i ];
	}

	// a dot b
	template<class t>
	t fVectorDot( const t * a, const t * b, u32 dim )
	{
		t d = 0;
		for( u32 i = 0; i < dim; ++i )
			d += a[ i ] * b[ i ];

		return d;
	}

	// ||a||2
	template<class t>
	t fVectorMagnitudeSq( const t * a, u32 dim )
	{
		return fVectorDot( a, a, dim );
	}

	// ||a||
	template<class t>
	t fVectorMagnitude( const t * a, u32 dim )
	{
		return Math::fSqrt( fVectorMagnitudeSq( a, dim ) );
	}

	// a * b
	template<class t>
	void fVectorScale( t * result, const t * a, const t * b, u32 dim )
	{
		for( u32 i = 0; i < dim; ++i )
			result[ i ] = a[ i ] * b[ i ];
	}

	// a * s
	template<class t>
	void fVectorScale( t * result, const t * a, t s, u32 dim )
	{
		for( u32 i = 0; i < dim; ++i )
			result[ i ] = a[ i ] * s;
	}

	// dest = src
	template<class t>
	void fVectorStore( t * dest, const t * src, u32 dim )
	{
		fMemCpy( dest, src, sizeof( t ) * dim );
	}

	template<class t>
	void fVectorSet( t * dest, const t & val, u32 dim )
	{
		for( u32 i = 0; i < dim; ++i )
			dest[ i ] = val;
	}

	// a /= ||a||
	template<class t>
	void fVectorNormalize( t * a, u32 dim )
	{
		fVectorScale( a, a, 1 / fVectorMagnitude( a, dim ), dim );
	}

	// result = a / ||a||
	template<class t>
	void fVectorNormalize( t * result, const t * a, u32 dim )
	{
		fVectorStore( result, a, dim );
		fVectorNormalize( result, dim );
	}

	// a /= ||a|| iff ||a|| > 0
	template<class t>
	void fVectorNormalizeSafe( t * a, u32 dim )
	{
		t m2 = fVectorMagnitudeSq( a, dim );
		if( fEqual( m2, 0 ) )
			return;

		fVectorScale( a, a, 1 / Math::fSqrt( m2 ), dim );
	}

	// result = a / ||a|| iff ||a|| > 0 otherwise result = a
	template<class t>
	void fVectorNormalizeSafe( t * result, const t * a, u32 dim )
	{
		fVectorStore( result, a, dim );
		fVectorNormalizeSafe( result, dim );
	}

	// a /= ||a||
	template<class t>
	void fVectorNormalize( t * a, u32 dim, t & magOut )
	{
		magOut = fVectorMagnitude( a, dim );
		fVectorScale( a, a, 1 / magOut, dim );
	}

	// result = a / ||a||
	template<class t>
	void fVectorNormalize( t * result, const t * a, u32 dim, t & magOut )
	{
		fVectorStore( result, a, dim );
		fVectorNormalize( result, dim, magOut );
	}

	// a /= ||a|| iff ||a|| > 0
	template<class t>
	void fVectorNormalizeSafe( t * a, u32 dim, t & magOut )
	{
		t m2 = fVectorMagnitudeSq( a, dim );
		if( fEqual( m2, 0 ) )
			return;

		magOut = Math::fSqrt( m2 );
		fVectorScale( a, a, 1 / magOut, dim );
	}

	// result = a / ||a|| iff ||a|| > 0 otherwise result = a
	template<class t>
	void fVectorNormalizeSafe( t * result, const t * a, u32 dim, t & magOut )
	{
		fVectorStore( result, a, dim );
		fVectorNormalizeSafe( result, dim, magOut );
	}

	// result = Lerp( a, b, amount )
	template<class t>
	void fVectorLerp( t * result, const t * a, const t * b, const t & amount, u32 dim )
	{
		for( u32 i = 0; i < dim; ++i )
			result[ i ] = fLerp( a[ i ], b[ i ], amount );
	}

	// result = vT * m where vT is the transpose of v where v is a dim vector and 
	// m is a dim x dim matrix
	template<class t>
	void fVectorMultiplyMatrix( t * result, const t * v, const t * m, u32 dim )
	{
		sigassert( result != v && "Result cannot be src" );
		for( u32 i = 0; i < dim; ++i )
		{
			result[ i ] = 0;
			for( u32 j = 0, jthRow = 0; j < dim; ++j, jthRow += dim )
				result[ i ] += m[ jthRow + i ] * v[ j ];
		}
	}

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

	template<class t, class u>
	inline b32 fEqual( const Math::tVector1<t>& lhs, const Math::tVector1<u>& rhs, const f32 epsilon=Math::cVectorEqualElementEpsilon )
	{
		return lhs.fEqual( rhs, epsilon );
	}

	template<class t, class u>
	inline b32 fEqual( const Math::tVector2<t>& lhs, const Math::tVector2<u>& rhs, const f32 epsilon=Math::cVectorEqualElementEpsilon )
	{
		return lhs.fEqual( rhs, epsilon );
	}

	template<class t, class u>
	inline b32 fEqual( const Math::tVector3<t>& lhs, const Math::tVector3<u>& rhs, const f32 epsilon=Math::cVectorEqualElementEpsilon )
	{
		return lhs.fEqual( rhs, epsilon );
	}

	template<class t, class u>
	inline b32 fEqual( const Math::tVector4<t>& lhs, const Math::tVector4<u>& rhs, const f32 epsilon=Math::cVectorEqualElementEpsilon )
	{
		return lhs.fEqual( rhs, epsilon );
	}
}


#endif//__tVector__
