#ifndef __tOctree__
#define __tOctree__
#include "tSpatialTree.hpp"

namespace Sig
{

	class base_export tOctree : public tSpatialTree
	{
		declare_reflector( );

	public:

		enum tCellLocation
		{
			cCell_pXpYpZ,
			cCell_nXpYpZ,
			cCell_pXpYnZ,
			cCell_nXpYnZ,
			cCell_pXnYpZ,
			cCell_nXnYpZ,
			cCell_pXnYnZ,
			cCell_nXnYnZ,
			cCellCount
		};

		Math::tAabbf fComputeChildAabb( const Math::tAabbf& aabb, u32 cell ) const;
	};

}

#endif//__tOctree__

