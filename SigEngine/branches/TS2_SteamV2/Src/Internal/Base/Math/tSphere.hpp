#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tSphere__
#define __tSphere__

namespace Sig { namespace Math
{
	template<class t>
	class tAabb;

	template<class t>
	class tObb;

	template<class t>
	class tSphere
	{
		declare_reflector( );
	public:
		tVector3<t>		mCenter;
		t				mRadius;

	public:
		static inline tSphere fConstruct( const tVector3<t>& c, t r ) { return tSphere( c, r ); }
	public:
		inline tSphere( ) { }
		inline explicit tSphere( t r ) : mCenter( Math::tVector3<t>::cZeroVector ), mRadius( r ) { }
		inline tSphere( const tVector3<t>& c, t r ) : mCenter( c ), mRadius( r ) { sig_assertvecvalid( mCenter ); }
		inline explicit tSphere( const tAabb<t>& aabb );
		inline explicit tSphere( const tObb<t>& obb );

		inline const tVector3<t>& fCenter( ) const { return mCenter; }
		inline const t fRadius( ) const { return mRadius; }

		inline tVector3<t> fMin( ) const { return mCenter - mRadius; }
		inline tVector3<t> fMax( ) const { return mCenter + mRadius; }

		inline tMatrix3<t> fAdjustMatrix( tMatrix3<t> xform, t expandPct = (t)0.f ) const
		{
			const tVector3<t>& localSpaceCenter = mCenter;
			const tVector3<t>  localSpaceExtent = 2.f * mRadius;

			t localSpaceExtentLength = (t)0.f;
			const tVector3<t> localSpaceExtentUnit = tVector3<t>( localSpaceExtent ).fNormalizeSafe( localSpaceExtentLength );
			const tVector3<t> halfLocalSpaceExtentExpanded = ( (t)0.5f * localSpaceExtentLength + expandPct ) * localSpaceExtentUnit;

			xform.fTranslateLocal( localSpaceCenter );
			xform.fScaleLocal( halfLocalSpaceExtentExpanded );

			return xform;
		}

		inline tSphere fTranslate( const tVector3<t>& dt ) const
		{
			return tSphere( mCenter + dt, mRadius );
		}

		inline b32 fIntersects( const tSphere& other ) const
		{
			return ( mCenter - other.mCenter ).fLengthSquared( ) <= fSquare( mRadius + other.mRadius );
		}

		inline b32 fContains( const tVector3<t>& point ) const
		{
			return ( point - mCenter ).fLengthSquared( ) <= fSquare( mRadius );
		}

		inline tSphere operator|( const tSphere& other ) const
		{
			if( fContains( other.mCenter ) )
				return tSphere( mCenter, ::Sig::fMax( mRadius, other.mRadius + ( other.mCenter - mCenter ).fLength( ) ) );
			else
				return tSphere( 0.5f * ( mCenter + other.mCenter ), mRadius + other.mRadius + 0.5f * ( other.mCenter - mCenter ).fLength( ) );
		}

		inline tSphere& operator|=( const tSphere& other )
		{
			*this = *this | other;
			return *this;
		}

		tSphere fInterpolate( const tSphere& other, t amountOfOther ) const
		{
			tSphere o;

			o.mCenter = fLerp( mCenter, other.mCenter, amountOfOther );
			o.mRadius = fLerp( mRadius, other.mRadius, amountOfOther );

			return o;
		}
	};

	typedef tSphere<f32> tSpheref;
	typedef tSphere<f64> tSphered;

}}

#endif//__tSphere__
