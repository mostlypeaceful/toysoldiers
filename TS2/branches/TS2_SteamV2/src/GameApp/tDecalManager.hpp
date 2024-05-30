#ifndef __tDecalManager__
#define __tDecalManager__
#include "Gfx/tWorldSpaceDecal.hpp"

namespace Sig
{
	class tDecalManager
	{
		declare_singleton( tDecalManager );
	public:
		Gfx::tWorldSpaceDecalPtr fPlace( const tResourcePtr& colorMap, const Math::tObbf& projectionBox );
		void fUpdate( f32 dt );
	private:
		tGrowableArray<Gfx::tWorldSpaceDecalPtr> mDecals;
	};
}

#endif//__tDecalManager__
