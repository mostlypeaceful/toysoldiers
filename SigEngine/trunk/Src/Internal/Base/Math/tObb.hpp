#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tObb__
#define __tObb__

namespace Sig { namespace Math
{
	template<class t>
	class tObb
	{
		declare_reflector( );
	public:
		tVector3<t>					mCenter;
		tFixedArray<tVector3<t>,3>	mAxes;
		tVector3<t>					mExtents;

		static const tObb			cZeroSized;

	public:
		static inline tObb fConstruct( const tAabb<t>& aabb, const tMatrix3<t>& xform ) { return tObb( aabb, xform ); }

	public:
		inline tObb( ) { }
		inline tObb( tNoOpTag ) { }
		inline explicit tObb( const tAabb<t>& aabb )
			: mCenter( aabb.fComputeCenter( ) )
		{
			mAxes[ 0 ] = tVector3<t>::cXAxis;
			mAxes[ 1 ] = tVector3<t>::cYAxis;
			mAxes[ 2 ] = tVector3<t>::cZAxis;
			mExtents = 0.5f * aabb.fComputeDiagonal( );
		}
		inline tObb( const tAabb<t>& aabb, const tMatrix3<t>& xform )
			: mCenter( xform.fXformPoint( aabb.fComputeCenter( ) ) )
		{
			tVector3<t> xformScale;
			mAxes[ 0 ] = xform.fXAxis( ).fNormalizeSafe( xformScale.x );
			mAxes[ 1 ] = xform.fYAxis( ).fNormalizeSafe( xformScale.y );
			mAxes[ 2 ] = xform.fZAxis( ).fNormalizeSafe( xformScale.z );
			mExtents = 0.5f * aabb.fComputeDiagonal( ) * xformScale;
		}
		inline explicit tObb( const tSphere<t>& sphere )
			: mCenter( sphere.mCenter )
		{
			mAxes[ 0 ] = tVector3<t>::cXAxis;
			mAxes[ 1 ] = tVector3<t>::cYAxis;
			mAxes[ 2 ] = tVector3<t>::cZAxis;
			mExtents = tVector3<t>( sphere.mRadius );
		}
		inline tObb( const tVector3<t>& center, const tVector3<t>& xExtent, const tVector3<t>& yExtent, const tVector3<t>& zExtent )
			: mCenter( center )
		{
			mAxes[ 0 ] = xExtent; mAxes[ 0 ].fNormalizeSafe( mExtents.x );
			mAxes[ 1 ] = yExtent; mAxes[ 1 ].fNormalizeSafe( mExtents.y );
			mAxes[ 2 ] = zExtent; mAxes[ 2 ].fNormalizeSafe( mExtents.z );
		}
		inline tObb( const tVector3<t>& center, const tVector3<t>& x, const tVector3<t>& y, const tVector3<t>& z, const tVector3<t>& extents )
			: mCenter( center )
		{
			mAxes[ 0 ] = x;
			mAxes[ 1 ] = y;
			mAxes[ 2 ] = z;
			sigassert( ::Sig::fEqual( x.fLengthSquared( ), 1.f ) );
			sigassert( ::Sig::fEqual( y.fLengthSquared( ), 1.f ) );
			sigassert( ::Sig::fEqual( z.fLengthSquared( ), 1.f ) );
			mExtents = extents;
		}

		inline tObb fTransform( const tMatrix3<t>& xform ) const
		{
			tObb o;

			o.mCenter = xform.fXformPoint( mCenter );

			for( u32 i = 0; i < 3; ++i )
				o.mAxes[ i ] = xform.fXformVector( mAxes[ i ] * mExtents.fAxis( i ) );

			o.mAxes[ 0 ].fNormalizeSafe( o.mExtents.x );
			o.mAxes[ 1 ].fNormalizeSafe( o.mExtents.y );
			o.mAxes[ 2 ].fNormalizeSafe( o.mExtents.z );

			return o;
		}

		inline const tVector3<t>& fCenter( ) const { return mCenter; }
		inline const tVector3<t>& fAxis( u32 i ) const { return mAxes[ i ]; }
		inline const tVector3<t>& fExtents( ) const { return mExtents; }

		inline tMatrix3<t> fGetTransform( ) const { return tMatrix3<t>( mAxes[0], mAxes[1], mAxes[2], mCenter ); }

		tVector3<t> fMin( ) const { return mCenter - mAxes[0] * mExtents.x - mAxes[1] * mExtents.y - mAxes[2] * mExtents.z; }
		tVector3<t> fMax( ) const { return mCenter + mAxes[0] * mExtents.x + mAxes[1] * mExtents.y + mAxes[2] * mExtents.z; }

		tAabb<t> fToAabb( ) const { return tAabb<t>( *this ); }

		inline tObb fInflate( t additionalExtent ) const
		{
			return tObb( mCenter, mAxes[ 0 ], mAxes[ 1 ], mAxes[ 2 ], mExtents + tVector3<t>( additionalExtent ) );
		}

		t fProjectedRadius( const tVector3<t>& alongThisNormal ) const
		{
			return 
				mExtents.x * fAbs( alongThisNormal.fDot( mAxes[ 0 ] ) ) + 
				mExtents.y * fAbs( alongThisNormal.fDot( mAxes[ 1 ] ) ) + 
				mExtents.z * fAbs( alongThisNormal.fDot( mAxes[ 2 ] ) );
		}

		inline tVector3<t> fProjection( const tVector3<t>& alongThisNormal ) const
		{
			return fProjectedRadius( alongThisNormal ) * alongThisNormal;
		}

		inline void fMinMaxProject( t& min, t& max, const tVector3<t>& alongThisNormal) const
		{
			t s = fAbs( mAxes[0].fDot(alongThisNormal) ) * mExtents.x;
			t u = fAbs( mAxes[1].fDot(alongThisNormal) ) * mExtents.y;
			t d = fAbs( mAxes[2].fDot(alongThisNormal) ) * mExtents.z;
			t r = s + u + d;
			t p = fCenter().fDot( alongThisNormal );
			min = p-r;
			max = p+r;
		}

		inline b32 fSeparatedOnAxis( const tObb& other, const tVector3<t>& d ) const
		{
			return fSeparationOnAxis( other, d ) > 0.0f;
		}

		inline t fSeparationOnAxis( const tObb& other, const tVector3<t>& d ) const
		{
			// Get projected distance between OBB centers
			const t centerDistance = fAbs( d.fDot( mCenter - other.mCenter ) );
			const t combinedRaidii = fProjectedRadius( d ) + other.fProjectedRadius( d );
			return centerDistance - combinedRaidii;
		}

		tMatrix3<t> fAdjustMatrix( tMatrix3<t> xform, t expandPct = (t)0.f ) const
		{
			t localSpaceExtentLength = (t)0.f;
			const tVector3<t> localSpaceExtentUnit = tVector3<t>( mExtents ).fNormalizeSafe( localSpaceExtentLength );
			const tVector3<t> halfLocalSpaceExtentExpanded = ( (t)localSpaceExtentLength + expandPct ) * localSpaceExtentUnit;

			tMatrix3<t> newXform;
			newXform.fSetTranslation( mCenter );
			newXform.fXAxis( mAxes[ 0 ] * halfLocalSpaceExtentExpanded.x );
			newXform.fYAxis( mAxes[ 1 ] * halfLocalSpaceExtentExpanded.y );
			newXform.fZAxis( mAxes[ 2 ] * halfLocalSpaceExtentExpanded.z );

			return xform * newXform;
		}

		inline void fPointToLocalVector( tRay<t>& ray, tAabb<t>& aabb ) const
		{
			ray.mOrigin = fPointToLocalVector( ray.mOrigin );
			ray.mExtent = tVector3<t>( mAxes[ 0 ].fDot( ray.mExtent ), mAxes[ 1 ].fDot( ray.mExtent ), mAxes[ 2 ].fDot( ray.mExtent ) );
			aabb.mMin = -mExtents;
			aabb.mMax =  mExtents;
		}

		inline tVector3<t> fPointToLocalVector( tVector3<t> point ) const
		{
			point -= mCenter;
			return tVector3<t>( mAxes[ 0 ].fDot( point ), mAxes[ 1 ].fDot( point ), mAxes[ 2 ].fDot( point ) );
		}

		inline tVector3<t> fWorldVectorToLocalVector( const tVector3<t>& worldVec ) const
		{
			return tVector3<t>( mAxes[ 0 ].fDot( worldVec ), mAxes[ 1 ].fDot( worldVec ), mAxes[ 2 ].fDot( worldVec ) );
		}

		inline tVector3<t> fToWorldPoint( const tVector3<t>& localVector ) const
		{
			return mCenter + localVector.x * mAxes[0] + localVector.y * mAxes[1] + localVector.z * mAxes[2];
		}

		inline tVector3<t> fToWorldVector( const tVector3<t>& localVector ) const
		{
			return localVector.x * mAxes[0] + localVector.y * mAxes[1] + localVector.z * mAxes[2];
		}
		
		inline tVector3<t> fCorner( u32 ithCorner ) const
		{
			sigassert( ithCorner < 8 );
			return fToWorldPoint( fCornerLocalVector( ithCorner ) );
		}

		inline tVector3<t> fCornerWorldVector( u32 ithCorner ) const
		{
			sigassert( ithCorner < 8 );
			tVector3<t> c = fCornerLocalVector( ithCorner );
			return c.x * mAxes[0] + c.y * mAxes[1] + c.z * mAxes[2];
		}

		inline tVector3<t> fCornerLocalVector( u32 ithCorner ) const
		{
			sigassert( ithCorner < 8 );
			return tVector3<t>(
				( ithCorner & 1 ) ? -mExtents.x : mExtents.x,
				( ithCorner & 2 ) ? -mExtents.y : mExtents.y,
				( ithCorner & 4 ) ? -mExtents.z : mExtents.z );
		}

		inline b32 fContains( const tVector3<t>& point ) const
		{
			return tAabb<t>( -mExtents, mExtents ).fContains( fPointToLocalVector( point ) );
		}

		inline b32 fContains2D( const tVector3<t>& point ) const
		{
			return tAabb<t>( -mExtents, mExtents ).fContains2D( fPointToLocalVector( point ) );
		}

		inline u32 fSupportingCornerIndex(  const tVector3<t>& worldD ) const
		{
			u32 result = 0;
			t maxD = fCornerWorldVector( 0 ).fDot( worldD );
			for( u32 i = 1; i < 8; ++i )
			{
				t d = fCornerWorldVector( i ).fDot( worldD );
				if( d > maxD )
				{
					maxD = d;
					result = i;
				}
			}

			return result;
		}

		// Result will always be a corner
		inline tVector3<t> fSupportingCorner( const tVector3<t>& worldD ) const
		{
			return fCorner( fSupportingCornerIndex( worldD ) );
		}

		// This result could be the center of a face, edge, or point
		inline tVector3<t> fSupportingPoint( const tVector3<t>& axis ) const
		{
			t as = axis.fDot( fAxis( 0 ) );
			t au = axis.fDot( fAxis( 1 ) );
			t ad = axis.fDot( fAxis( 2 ) );

			const t threshold = cEpsilon;

			tVector3<t> result = fCenter( );

			if (as < -threshold)
				result += fAxis( 0 ) * fExtents().x;
			else if (as >= threshold)
				result -= fAxis( 0 ) * fExtents().x;

			if (au < -threshold)
				result += fAxis( 1 ) * fExtents().y;
			else if (au > threshold)
				result -= fAxis( 1 ) * fExtents().y;

			if (ad < -threshold)
				result += fAxis( 2 ) * fExtents().z;
			else if (ad > threshold)
				result -= fAxis( 2 ) * fExtents().z;

			return result;
		}

		inline tVector3<t> fClosestPoint( const tVector3<t>& point ) const
		{
			tVector3<t> local = fPointToLocalVector( point );

			for( u32 i = 0; i < 3; ++i )
				local[ i ] = fClamp( local[ i ], -mExtents[ i ], mExtents[ i ] );

			return fToWorldPoint( local );
		}

		tObb fInterpolate( const tObb& other, t amountOfOther ) const
		{
			tObb o;

			o.mCenter = fLerp( mCenter, other.mCenter, amountOfOther );
			o.mExtents = fLerp( mExtents, other.mExtents, amountOfOther );
			
			Math::tQuaternion<t> meRot( Math::tMatrix3<t>( mAxes[ 0 ], mAxes[ 1 ], mAxes[ 2 ], Math::tVector3<t>::cZeroVector ) );
			Math::tQuaternion<t> otherRot( Math::tMatrix3<t>( other.mAxes[ 0 ], other.mAxes[ 1 ], other.mAxes[ 2 ], Math::tVector3<t>::cZeroVector ) );
			Math::tMatrix3<t> oMat( fSlerp( meRot, otherRot, amountOfOther ), Math::tVector3<t>::cZeroVector );

			for( u32 i = 0; i < 3; ++i )
				o.mAxes[ i ] = oMat.fGetCol( i );

			return o;
		}
	};

	template<class t>
	const tObb<t> tObb<t>::cZeroSized( tSphere<t>( t(0.) ) );

	typedef tObb<f32> tObbf;
	typedef tObb<f64> tObbd;

	template<class t>
	inline tSphere<t>::tSphere( const tObb<t>& obb )
		: mCenter( obb.fCenter( ) )
		, mRadius( obb.fExtents( ).fLength( ) )
	{
	}

	template<class t>
	inline tAabb<t>::tAabb( const tObb<t>& obb )
	{
		this->fInvalidate( );

		for( u32 i = 0; i < 8; ++i )
			this->operator |=( obb.fCorner( i ) );
	}

}}

#endif//__tObb__
