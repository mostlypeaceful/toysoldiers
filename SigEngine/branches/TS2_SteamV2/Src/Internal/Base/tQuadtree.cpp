#include "BasePch.hpp"
#include "tQuadtree.hpp"

namespace Sig
{

	Math::tAabbf tQuadtree::fComputeChildAabb( const Math::tAabbf& aabb, u32 cell ) const
	{
		const Math::tVec3f c = aabb.fComputeCenter( ); // center
		const Math::tVec3f d = 0.5f * aabb.fComputeDiagonal( ); // diagonal

		switch( cell )
		{
		case cCell_pXpZ:
			return Math::tAabbf( Math::tVec3f( c.x, c.y - 2.f*d.y, c.z ), c + d );
		case cCell_nXpZ:
			return Math::tAabbf( Math::tVec3f( c.x - d.x, c.y - 2.f*d.y, c.z ), Math::tVec3f( c.x, c.y + 2.f*d.y, c.z + d.z ) );
		case cCell_pXnZ:
			return Math::tAabbf( Math::tVec3f( c.x, c.y - 2.f*d.y, c.z - d.z ), Math::tVec3f( c.x + d.x, c.y + 2.f*d.y, c.z ) );
		case cCell_nXnZ:
			return Math::tAabbf( Math::tVec3f( c.x - d.x, c.y - 2.f*d.y, c.z - d.z ), Math::tVec3f( c.x, c.y + 2.f*d.y, c.z ) );

		default: sigassert( !"invalid cell index" ); break;
		}

		return aabb;
	}
}
