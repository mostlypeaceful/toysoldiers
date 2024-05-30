#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tAabb__
#define __tAabb__

namespace Sig { namespace Math
{
	template<class t>
	class tAabb
	{
		declare_reflector( );
		sig_make_loggable( tAabb, "(" << mMin << ", " << mMax << ")" );
	public:

		tVector3<t>			mMin, mMax;

		static const tAabb	cZeroSized;

	public:
		static inline tAabb fConstruct( const tVector3<t>& min, const tVector3<t>& max ) { return tAabb( min, max ); }

	public:
		inline tAabb( ) { }
		inline tAabb( tNoOpTag ) : mMin( cNoOpTag ), mMax( cNoOpTag ) { }
		inline tAabb( const tVector3<t>& min, const tVector3<t>& max )
			: mMin( min ), mMax( max ) { sig_assertvecvalid( mMin ); sig_assertvecvalid( mMax ); }
		inline explicit tAabb( const tSphere<t>& s )
			: mMin( s.fMin( ) ), mMax( s.fMax( ) ) { sig_assertvecvalid( mMin ); sig_assertvecvalid( mMax ); }
		inline explicit tAabb( const tObb<t>& b );
		inline explicit tAabb( const tRay<t>& ray );

		inline void fInvalidate( )
		{
			mMin = tVector3<t>( +cInfinity );
			mMax = tVector3<t>( -cInfinity );
		}

		inline b32 fIsValid( ) const
		{
			sig_assertvecvalid( mMin ); sig_assertvecvalid( mMax );
			return 
				mMax.x >= mMin.x && 
				mMax.y >= mMin.y && 
				mMax.z >= mMin.z;
		}

		inline b32 fIsNan( ) const
		{
			return mMin.fIsNan( ) || mMax.fIsNan( );
		}

		inline void operator|=( const tVector3<t>& point )
		{
			for( u32 iaxis = 0; iaxis < point.cDimension; ++iaxis )
			{
				mMin.fAxis( iaxis ) = fMin( point.fAxis( iaxis ), mMin.fAxis( iaxis ) );
				mMax.fAxis( iaxis ) = fMax( point.fAxis( iaxis ), mMax.fAxis( iaxis ) );
			}
			sig_assertvecvalid( mMin ); sig_assertvecvalid( mMax );
		}

		inline void operator|=( const tAabb& aabb )
		{
			operator|=( aabb.mMin );
			operator|=( aabb.mMax );
		}

		inline void operator|=( const tObb<t>& obb )
		{
			operator|=( obb.fToAabb( ) );
		}

		inline tVector3<t> fComputeCenter( ) const
		{
			return 0.5f * ( mMin + mMax );
		}

		inline tVector3<t> fComputeDiagonal( ) const
		{
			return ( mMax - mMin );
		}

		inline t fHeight( ) const
		{
			return mMax.y - mMin.y;
		}

		inline tVector3<t> fMaxVertex( const tVector3<t>& alongThisNormal ) const
		{
			tVector3<t> v = mMin;
			if( alongThisNormal.x >= 0.f ) v.x = mMax.x;
			if( alongThisNormal.y >= 0.f ) v.y = mMax.y;
			if( alongThisNormal.z >= 0.f ) v.z = mMax.z;
			return v;
		}

		inline tVector3<t> fMinVertex( const tVector3<t>& alongThisNormal ) const
		{
			tVector3<t> v = mMax;
			if( alongThisNormal.x >= 0.f ) v.x = mMin.x;
			if( alongThisNormal.y >= 0.f ) v.y = mMin.y;
			if( alongThisNormal.z >= 0.f ) v.z = mMin.z;
			return v;
		}

		inline void fMinMaxAlongVector( const tVector3<t>& alongThisNormal, t& minOut, t& maxOut ) const
		{
			minOut = mMin.fDot( alongThisNormal );
			maxOut = mMax.fDot( alongThisNormal );

			if( maxOut < minOut ) fSwap( maxOut, minOut );
		}

		inline t fDistanceToPoint( const tVector3<t>& point ) const
		{
			t sqDist = (t)0.f;

			for( u32 i = 0; i < 3; ++i )
			{
				const t v = point.fAxis( i );
				if( v < mMin.fAxis( i ) ) sqDist += fSquare( mMin.fAxis( i ) - v );
				if( v > mMax.fAxis( i ) ) sqDist += fSquare( v - mMax.fAxis( i ) );
			}

			return ( t )fSqrt( ( f32 )sqDist );
		}

		inline tVector3<t> fCorner( u32 ithCorner ) const
		{
			sigassert( ithCorner < 8 );
			return tVector3<t>(
				( ithCorner & 1 ) ? mMin.x : mMax.x,
				( ithCorner & 2 ) ? mMin.y : mMax.y,
				( ithCorner & 4 ) ? mMin.z : mMax.z );
		}

		inline tAabb fTransform( const tMatrix3<t>& xform ) const
		{
			Math::tVector3<t> center = xform.fGetTranslation( );
			tAabb o( center, center );

			for( u32 j = 0; j < 3; ++j )
			{
				for( u32 i = 0; i < 3; ++i )
				{
					t m = xform( j, i );
					t a = m * mMin[ i ];
					t b = m * mMax[ i ];

#ifdef platform_xbox360
					t diff = a - b;
					o.mMin[ j ] += (f32)__fsel( diff, b, a );
					o.mMax[ j ] += (f32)__fsel( diff, a, b );
#else
					if( a < b )
					{
						o.mMin[ j ] += a;
						o.mMax[ j ] += b;
					}
					else
					{
						o.mMin[ j ] += b;
						o.mMax[ j ] += a;
					}
#endif
				}
			}

			return o;
		}

		inline tAabb fTranslate( const tVector3<t>& dt ) const
		{
			return tAabb( mMin + dt, mMax + dt );
		}

		inline tAabb fInflate( const t amount ) const
		{
			return tAabb( tVector3<t>( mMin.x - amount, mMin.y - amount, mMin.z - amount ), tVector3<t>( mMax.x + amount, mMax.y + amount, mMax.z + amount ) );
		}

		tAabb fSubAabb( const tVector3<t>& barycentricMin, const tVector3<t>& barycentricMax ) const
		{
			const tVector3<t> diag = ( mMax - mMin );
			return tAabb( mMin + barycentricMin * diag, mMin + barycentricMax * diag );
		}

		inline tMatrix3<t> fAdjustMatrix( tMatrix3<t> xform, t expandPct = (t)0.f ) const
		{
			const tVector3<t> localSpaceCenter = fComputeCenter( );
			const tVector3<t> localSpaceExtent = fComputeDiagonal( );

			t localSpaceExtentLength = (t)0.f;
			const tVector3<t> localSpaceExtentUnit = tVector3<t>( localSpaceExtent ).fNormalizeSafe( localSpaceExtentLength );
			const tVector3<t> halfLocalSpaceExtentExpanded = ( (t)0.5f * localSpaceExtentLength + expandPct ) * localSpaceExtentUnit;

			xform.fTranslateLocal( localSpaceCenter );
			xform.fScaleLocal( halfLocalSpaceExtentExpanded );

			return xform;
		}

		inline b32 fContains( const tVector3<t>& p ) const
		{
			return 
				fInBounds( p.x, mMin.x, mMax.x ) &&
				fInBounds( p.y, mMin.y, mMax.y ) &&
				fInBounds( p.z, mMin.z, mMax.z );
		}

		inline b32 fContainsXZ( const tVector3<t>& p ) const // i.e., a 2D containment, assuming Y is up
		{
			return 
				fInBounds( p.x, mMin.x, mMax.x ) &&
				fInBounds( p.z, mMin.z, mMax.z );
		}

		inline b32 fContains( const tAabb& other ) const
		{
			return fContains( other.mMin ) && fContains( other.mMax );
		}

		inline b32 fContainsEitherEndPoint( const tRay<t>& r ) const
		{
			return fContains( r.mOrigin ) || fContains( r.mOrigin + r.mExtent );
		}

		t fSurfaceArea( ) const 
		{
			tVector3<t> diag = fComputeDiagonal( );

			t total = diag.x * diag.y * 2.0f;
			total += diag.y * diag.z * 2.0f;
			total += diag.z * diag.x * 2.0f;

			return total;
		}

		inline b32 fIntersects( const tAabb& other ) const
		{
			return !(	mMin.x > other.mMax.x || mMin.y > other.mMax.y || mMin.z > other.mMax.z ||
						mMax.x < other.mMin.x || mMax.y < other.mMin.y || mMax.z < other.mMin.z );
		}

		inline b32 fIntersectsWalls( const tRay<t>& ray, t& tOut ) const
		{
			const t t0 = 0.f;
			const t t1 = 1.f;

			const tVector3<t> rayExtentInv = ray.mExtent.fInverse( );

			const u32 sign[] =
			{
				( rayExtentInv.x < 0 ),
				( rayExtentInv.y < 0 ),
				( rayExtentInv.z < 0 ),
			};

			const tVector3<t>* boxCorners = &mMin;

			t tmin	= ( boxCorners[     sign[0] ].x - ray.mOrigin.x ) * rayExtentInv.x;
			t tmax	= ( boxCorners[ 1 - sign[0] ].x - ray.mOrigin.x ) * rayExtentInv.x;
			t tymin = ( boxCorners[     sign[1] ].y - ray.mOrigin.y ) * rayExtentInv.y;
			t tymax = ( boxCorners[ 1 - sign[1] ].y - ray.mOrigin.y ) * rayExtentInv.y;

			if ( (tmin > tymax) || (tymin > tmax) )
				return false; // no intersection
			if( tymin > tmin )
				tmin = tymin;
			if( tymax < tmax )
				tmax = tymax;

			t tzmin = ( boxCorners[     sign[2] ].z - ray.mOrigin.z ) * rayExtentInv.z;
			t tzmax = ( boxCorners[ 1 - sign[2] ].z - ray.mOrigin.z ) * rayExtentInv.z;

			if( (tmin > tzmax) || (tzmin > tmax) )
				return false; // no intersection
			if( tzmin > tmin )
				tmin = tzmin;
			if( tzmax < tmax )
				tmax = tzmax;
	
			if( fInBounds( tmin, t0, t1 ) )
			{
				tOut = tmin;
				return true;
			}
			else if( fInBounds( tmax, t0, t1 ) )
			{
				tOut = tmax;
				return true;
			}

			return false;
		}

		inline b32 fIntersectsOrContains( const tRay<t>& ray ) const
		{
			if( fContainsEitherEndPoint( ray ) )
				return true;
			t dummyT;
			return fIntersectsWalls( ray, dummyT );
		}

		inline b32 fIntersectsOrContains( const tRay<t>& ray, t& tOut ) const
		{
			if( fIntersectsWalls( ray, tOut ) )
				return true;
			if( fContainsEitherEndPoint( ray ) )
			{
				tOut = 1.f; // the ray is contained but doesn't intersect the walls, call the time of intersection 1
				return true;
			}
			return false;
		}

		inline tVector3<t> fClosestPoint( const tVector3<t>& point ) const
		{
			tVector3<t> pt = point;

			for( u32 i = 0; i < 3; ++i )
				pt[ i ] = fClamp( pt[ i ], mMin[ i ], mMax[ i ] );

			return pt;
		}
	};

	template<class t>
	const tAabb<t> tAabb<t>::cZeroSized( tSphere<t>( t(0.) ) );

	typedef tAabb<f32> tAabbf;
	typedef tAabb<f64> tAabbd;


	template<class t>
	inline tSphere<t>::tSphere( const tAabb<t>& aabb )
		: mCenter( aabb.fComputeCenter( ) )
		, mRadius( 0.5f * aabb.fComputeDiagonal( ).fLength( ) )
	{
	}

	template<class t>
	inline tAabb<t>::tAabb( const tRay<t>& ray )
	{
		this->fInvalidate( );
		this->operator |=( ray.fEvaluate( 0.f ) );
		this->operator |=( ray.fEvaluate( 1.f ) );
	}
}}

#endif//__tAabb__
