#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tFrustum__
#define __tFrustum__

namespace Sig { namespace Math
{
	template<class t>
	class tFrustum
	{
		declare_reflector( );

	public:

		enum tPlaneName
		{
			cPlaneLeft,
			cPlaneRight,
			cPlaneBottom,
			cPlaneTop,
			cPlaneFront,
			cPlaneBack,
			cPlaneCount
		};

	private:
		typedef tFixedArray< tPlane<t>, cPlaneCount > tPlaneArray;
		tPlaneArray mPlanes;

	public:

		inline tPlane<t>&		operator[]( u32 face )			{ return mPlanes[face]; }
		inline const tPlane<t>&	operator[]( u32 face ) const	{ return mPlanes[face]; }

		inline void fNormalize( )
		{
			for( u32 i = 0; i < cPlaneCount; ++i )
				if( !mPlanes[ i ].fIsZero( ) )
					mPlanes[ i ].fPlaneNormalize( );
		}

		template<class tMat>
		inline void fTransform( tFrustum& xformedOut, const tMat& matrix ) const
		{
			for( u32 i = 0; i < cPlaneCount; ++i )
				xformedOut[ i ] = mPlanes[ i ].fTransform( matrix );
		}

		inline b32 fContains( const tVector3<t>& point ) const
		{
			for( u32 i = 0; i < cPlaneCount; ++i )
			{
				if( mPlanes[ i ].fSignedDistance( point ) > 0.f )
					return false;
			}
			return true;
		}

		inline b32 fIntersects( const tAabb<t>& aabb ) const
		{
			for( u32 i = 0; i < cPlaneCount; ++i )
			{
				if( mPlanes[ i ].fSignedDistance( aabb.fMinVertex( mPlanes[ i ].fGetNormal( ) ) ) > 0.f )
					return false;
			}
			return true;
		}

		inline b32 fIntersectsOrContains( const tAabb<t>& aabb, b32& contains ) const
		{
			contains = true;
			for( u32 i = 0; i < cPlaneCount; ++i )
			{
				if( mPlanes[ i ].fSignedDistance( aabb.fMinVertex( mPlanes[ i ].fGetNormal( ) ) ) > 0.f )
				{
					contains = false;
					return false;
				}
				else if( mPlanes[ i ].fSignedDistance( aabb.fMaxVertex( mPlanes[ i ].fGetNormal( ) ) ) > 0.f )
				{
					contains = false;
				}
			}
			return true;
		}

		inline b32 fIntersects( const tSphere<t>& sphere ) const
		{
			for( u32 i = 0; i < cPlaneCount; ++i )
			{
				if( mPlanes[ i ].fSignedDistance( sphere.mCenter ) > sphere.mRadius )
					return false;
			}
			return true;
		}

		inline b32 fIntersectsOrContains( const tSphere<t> & sphere, b32 & contains ) const
		{
			contains = true;
			for( u32 p = 0; p < cPlaneCount; ++p )
			{
				const t signedDist = mPlanes[ p ].fSignedDistance( sphere.mCenter );
				if( signedDist > sphere.mRadius )
				{
					contains = false;
					return false;
				}
				else if( fAbs( signedDist ) < sphere.mRadius )
				{
					contains = false;
				}
			}
			return true;
		}
	};

	typedef tFrustum<f32> tFrustumf;
	typedef tFrustum<f64> tFrustumd;

}}

#endif//__tFrustum__
