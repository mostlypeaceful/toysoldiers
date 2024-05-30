#include "BasePch.hpp"
#include "tOctree.hpp"

namespace Sig
{

	Math::tAabbf tOctree::fComputeChildAabb( const Math::tAabbf& aabb, u32 cell ) const
	{
		const Math::tVec3f c = aabb.fComputeCenter( ); // center
		const Math::tVec3f d = 0.5f * aabb.fComputeDiagonal( ); // diagonal

		switch( cell )
		{
		case cCell_pXpYpZ:
			return Math::tAabbf( c, c + d );
		case cCell_nXpYpZ:
			return Math::tAabbf( Math::tVec3f( c.x - d.x, c.y, c.z ), Math::tVec3f( c.x, c.y + d.y, c.z + d.z ) );
		case cCell_pXpYnZ:
			return Math::tAabbf( Math::tVec3f( c.x, c.y, c.z - d.z ), Math::tVec3f( c.x + d.x, c.y + d.y, c.z ) );
		case cCell_nXpYnZ:
			return Math::tAabbf( Math::tVec3f( c.x - d.x, c.y, c.z - d.z ), Math::tVec3f( c.x, c.y + d.y, c.z ) );

		case cCell_pXnYpZ:
			return Math::tAabbf( Math::tVec3f( c.x, c.y - d.y, c.z ), Math::tVec3f( c.x + d.x, c.y, c.z + d.z ) );
		case cCell_nXnYpZ:
			return Math::tAabbf( Math::tVec3f( c.x - d.x, c.y - d.y, c.z ), Math::tVec3f( c.x, c.y, c.z + d.z ) );
		case cCell_pXnYnZ:
			return Math::tAabbf( Math::tVec3f( c.x, c.y - d.y, c.z - d.z ), Math::tVec3f( c.x + d.x, c.y, c.z ) );
		case cCell_nXnYnZ:
			return Math::tAabbf( Math::tVec3f( c.x - d.x, c.y - d.y, c.z - d.z ), Math::tVec3f( c.x, c.y, c.z ) );

		default: sigassert( !"invalid cell index" ); break;
		}

		return aabb;
	}
}
