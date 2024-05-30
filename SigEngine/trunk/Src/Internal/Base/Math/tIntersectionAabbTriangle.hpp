#ifndef __tIntersectionAabbTriangle__
#define __tIntersectionAabbTriangle__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionAabbTriangle
	{
	protected:

		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

	public:

		inline tIntersectionAabbTriangle( ) { }

		inline tIntersectionAabbTriangle( const tAabb<t>& aabb, const tTriangle<t>& triangle )
		{
			fIntersect( aabb, triangle );
		}

		inline tIntersectionAabbTriangle( const tTriangle<t>& triangle, const tAabb<t>& aabb )
		{
			fIntersect( aabb, triangle );
		}

		inline b32 fIntersects( ) const { return mIntersects; }

		inline void fIntersect( const tAabb<t>& aabb, const tTriangle<t>& triangle )
		{
			sigassert( !"todo" );
		}

	};

}}

#endif//__tIntersectionAabbTriangle__
