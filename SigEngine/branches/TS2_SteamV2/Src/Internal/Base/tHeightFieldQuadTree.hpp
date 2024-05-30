#ifndef __tTerrainCellQuadTree__
#define __tTerrainCellQuadTree__
#include "tLoadInPlaceObjects.hpp"

namespace Sig
{
	class tHeightFieldMesh;

	///
	/// \brief Output/result structure for a raycast query
	class base_export tHeightFieldRayCastHit : public Math::tRayCastHit
	{
	public:
		s16	mX, mZ;

		inline tHeightFieldRayCastHit( ) : mX( -1 ), mZ( -1 ) { }
		inline tHeightFieldRayCastHit( f32 t, const Math::tVec3f& n, s16 x, s16 z ) : tRayCastHit( t, n ), mX( x ), mZ( z ) { }
		inline b32 fHit( ) const { return fInBounds( mT, 0.f, 1.f ) && mX >= 0 && mZ >= 0; }
	};

	///
	/// \brief Optimization structure for performing ray-casts
	/// or other spatial queries on the quad tree triangles.
	class base_export tHeightFieldQuadTree
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

	private:
		typedef tFixedArray< tLoadInPlacePtrWrapper<tHeightFieldQuadTree>, cCellCount > tCellArray;
		Math::tVec2f mMinMaxHeight;
		Math::tVec2u mMinMaxLogicalX;
		Math::tVec2u mMinMaxLogicalZ;
		tCellArray   mChildren;

	public:
		tHeightFieldQuadTree( );
		tHeightFieldQuadTree( tNoOpTag );
		~tHeightFieldQuadTree( );

		inline const Math::tVec2f& fMinMaxHeight( ) const { return mMinMaxHeight; }

		void fConstruct( 
			const Math::tAabbf& myCellAabb,
			const tHeightFieldMesh& hfMesh, 
			u32 maxDepth, u32 splitThreshold );

	private:
		void fConstruct( 
			const Math::tAabbf& myCellAabb,
			const tHeightFieldMesh& hfMesh, 
			u32 currentDepth, u32 maxDepth, u32 splitThreshold );

	public:

		void fCopy( const tHeightFieldQuadTree& other );
		void fDestroy( );

		void fCollectTris(
			tGrowableArray<Math::tTrianglef>& trisOut,
			const Math::tAabbf& aabb,
			const Math::tAabbf& myCellAabb,
			const tHeightFieldMesh& hfMesh ) const;

		void fTestRay(
			tHeightFieldRayCastHit& bestHit,
			const Math::tRayf& ray,
			const Math::tAabbf& myCellAabb,
			const tHeightFieldMesh& hfMesh ) const;

		b32 fTestFrustum(
			const Math::tFrustumf& frustum,
			const Math::tAabbf& myCellAabb,
			const tHeightFieldMesh& hfMesh ) const;

		///
		/// \brief Updating bounding info for the entire quadtree for the specified
		/// logical vertex index; this is pretty quick.
		void fUpdateBounds( u32 logicalX, u32 logicalZ, const tHeightFieldMesh& hfMesh );

		///
		/// \brief Compute the specified child's aabb from the parent's aabb.
		Math::tAabbf fComputeChildAabb( const Math::tAabbf& myCellAabb, u32 cell ) const;

	private:
		b32 fComputeMinMaxHeightEstimateRecursive( 
			Math::tVec2f& out,
			const Math::tVec2u& minMaxLogicalX,
			const Math::tVec2u& minMaxLogicalZ ) const;

	public:
		///
		/// \brief Finds the smallest cell that contains the specified rectangle,
		/// and returns its min/max height. I.e., it does not visit every vertex
		/// in the rectangle to generate a true min/max height.
		Math::tVec2f fComputeMinMaxHeightEstimate( 
			const Math::tVec2u& minMaxLogicalX,
			const Math::tVec2u& minMaxLogicalZ ) const;

		b32 fIsLeaf( ) const;
		u32 fQuadCount( u32 depth ) const;
		//b32 fEqual( const tHeightFieldQuadTree& other ) const;
	};
}


#endif//__tTerrainCellQuadTree__
