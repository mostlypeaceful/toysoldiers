#ifndef __tIntersectionRayAabb__
#define __tIntersectionRayAabb__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionRayAabb
	{
	protected:
		t					mT;
		b32					mIntersects;

	public:

		inline tIntersectionRayAabb( ) { }

		inline tIntersectionRayAabb( const tRay<t>& ray, const tAabb<t>& aabb )
		{
			fIntersect( ray, aabb );
		}

		inline tIntersectionRayAabb( const tAabb<t>& aabb, const tRay<t>& ray )
		{
			fIntersect( ray, aabb );
		}

		inline t fT( ) const { return mT; }
		inline b32 fIntersects( ) const { return mIntersects; }

		inline void fIntersect( const tRay<t>& ray, const tAabb<t>& aabb )
		{
			mIntersects = aabb.fIntersectsWalls( ray, mT );
		}
	};

}}

#endif//__tIntersectionRayAabb__
