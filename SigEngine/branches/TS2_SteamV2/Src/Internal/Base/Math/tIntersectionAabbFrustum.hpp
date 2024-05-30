#ifndef __tIntersectionAabbFrustum__
#define __tIntersectionAabbFrustum__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionAabbFrustum
	{
	protected:

		b32				mContained; ///< This will be true if the aabb is fully contained in the frustum
		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

	public:

		inline tIntersectionAabbFrustum( ) { }

		inline tIntersectionAabbFrustum( const tAabb<t>& aabb, const tFrustum<t>& frustum )
		{
			fIntersect( aabb, frustum );
		}

		inline tIntersectionAabbFrustum( const tFrustum<t>& frustum, const tAabb<t>& aabb )
		{
			fIntersect( aabb, frustum );
		}

		inline b32 fIntersects( ) const { return mIntersects; }
		inline b32 fContained( ) const { return mContained; }

		inline void fIntersect( const tAabb<t>& aabb, const tFrustum<t>& frustum )
		{
			mIntersects = frustum.fIntersectsOrContains( aabb, mContained );
		}

	};

}}

#endif//__tIntersectionAabbFrustum__
