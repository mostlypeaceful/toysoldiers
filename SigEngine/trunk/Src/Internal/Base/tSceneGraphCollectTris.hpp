#ifndef __tSceneGraphCollectTris__
#define __tSceneGraphCollectTris__
#include "Gfx/tRenderableEntity.hpp"
#include "tSceneGraph.hpp"
#include "tEntityCloud.hpp"

namespace Sig
{
	///
	/// \brief Obtain all triangles in world space intersecting the specified world space volume.
	template< class t >
	void tSceneGraph::fCollectTris( const Math::tAabbf& v, t& collector )
	{
		mSpatialSets[ Gfx::tRenderableEntity::cSpatialSetIndex ]->fIntersect( v, collector );
		mSpatialSets[ Gfx::tRenderableEntity::cHeightFieldSpatialSetIndex ]->fIntersect( v, collector );
	}

	///
	/// \brief Obtain all triangles in world space intersecting the specified world space volume.
	template< class t >
	void tSceneGraph::fCollectTris( const Math::tObbf& v, t& collector )
	{
		mSpatialSets[ Gfx::tRenderableEntity::cSpatialSetIndex ]->fIntersect( v, collector );
		mSpatialSets[ Gfx::tRenderableEntity::cHeightFieldSpatialSetIndex ]->fIntersect( v, collector );
	}

	///
	/// \brief Visit all the tEntityClouds and gather entities whose bounds interesect the specified volume.
	template<class tVolume, class tIntersectionOperator>
	void tSceneGraph::fIntersectCloudRenderables( 
		const tVolume & v, 
		const tIntersectionOperator & intersectCb,
		b32 forShadows ) const
	{

		tVolumeCallback<tVolume, tIntersectionOperator> cbObj( v, intersectCb );

		const u32 cloudCount = mCloudList.fCount( );
		for( u32 c = 0; c < cloudCount; ++c )
			mCloudList[ c ]->fGatherRenderables( &cbObj.mDelegate, forShadows );
	}
}

#endif//__tSceneGraph__
