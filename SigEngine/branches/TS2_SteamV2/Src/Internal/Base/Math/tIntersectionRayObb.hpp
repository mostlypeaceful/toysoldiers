#ifndef __tIntersectionRayObb__
#define __tIntersectionRayObb__
#include "tIntersectionRayAabb.hpp"

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionRayObb : public tIntersectionRayAabb<t>
	{
	public:

		inline tIntersectionRayObb( ) { }

		inline tIntersectionRayObb( const tRay<t>& ray, const tObb<t>& obb )
		{
			fIntersect( ray, obb );
		}

		inline tIntersectionRayObb( const tAabb<t>& obb, const tRay<t>& ray )
		{
			fIntersect( ray, obb );
		}

		inline void fIntersect( const tRay<t>& ray, const tObb<t>& obb )
		{
			tRay<t> localRay = ray;
			tAabb<t> localAabb;
			obb.fPointToLocalVector( localRay, localAabb );
			tIntersectionRayAabb<t>::fIntersect( localRay, localAabb );
		}
	};

}}

#endif//__tIntersectionRayObb__
