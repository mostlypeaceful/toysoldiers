#ifndef __tQuadtree__
#define __tQuadtree__
#include "tSpatialTree.hpp"

namespace Sig
{

	class base_export tQuadtree : public tSpatialTree
	{
		declare_reflector( );

	public:

		enum tCellLocation
		{
			cCell_pXpZ,
			cCell_nXpZ,
			cCell_pXnZ,
			cCell_nXnZ,
			cCellCount
		};

		Math::tAabbf fComputeChildAabb( const Math::tAabbf& aabb, u32 cell ) const;
	};

}

#endif//__tQuadtree__

