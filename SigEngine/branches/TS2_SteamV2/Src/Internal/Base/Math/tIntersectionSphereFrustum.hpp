#ifndef __tIntersectionSphereFrustum__
#define __tIntersectionSphereFrustum__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionSphereFrustum
	{
	protected:

		b32				mIntersects;

	public:

		inline tIntersectionSphereFrustum( ) { }

		inline tIntersectionSphereFrustum( const tSphere<t>& sphere, const tFrustum<t>& frustum )
		{
			fIntersect( sphere, frustum );
		}

		inline tIntersectionSphereFrustum( const tFrustum<t>& frustum, const tSphere<t>& sphere )
		{
			fIntersect( sphere, frustum );
		}

		inline b32 fIntersects( ) const { return mIntersects; }

		inline void fIntersect( const tSphere<t>& sphere, const tFrustum<t>& frustum )
		{
			for( u32 i = 0; i < tFrustum<t>::cPlaneCount; ++i )
			{
				if( frustum[ i ].fSignedDistance( sphere.mCenter ) > sphere.mRadius )
				{
					mIntersects = false;
					return;
				}
			}

			mIntersects = true;
		}
	};

}}

#endif//__tIntersectionSphereFrustum__
