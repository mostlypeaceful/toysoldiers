#ifndef __tPolySoupKDTree__
#define __tPolySoupKDTree__
#include "tLoadInPlaceObjects.hpp"
#include "tPolySoupRaycastHit.hpp"

namespace Sig
{
	class tSortedKDData;

	class base_export tKDNode
	{
		declare_reflector( );
	public:
		//user accessable stuff
		enum tAxis { cXAxis, cYAxis, cZAxis, cAxisCount, cAxisInvalid };

		inline b32	fIsLeaf( ) const { return !mChildren[ 0 ] && !mChildren[ 1 ]; }
		void		fDraw( const Math::tMat3f& xform, Gfx::tDebugGeometryContainer& dgc, const Math::tAabbf& bounds, const Math::tVec4f& color, s32 level ) const;
		
		void		fTestRay(
						tPolySoupRayCastHit& bestHit,
						const Math::tRayf& ray,
						const Math::tAabbf& cellBounds,
						const tPolySoupVertexList& verts, 
						const tPolySoupTriangleList& tris ) const;


		b32			fTestFrustum(
						const Math::tFrustumf& frustum,
						const Math::tAabbf& cellBounds,
						const tPolySoupVertexList& verts, 
						const tPolySoupTriangleList& tris ) const;

		void		fCollectTris(
						tGrowableArray<Math::tTrianglef>& trisOut,
						const Math::tAabbf& aabb,
						const Math::tAabbf& cellBounds,
						const tPolySoupVertexList& verts, 
						const tPolySoupTriangleList& tris ) const;

	public:
		// building stuff
		tKDNode( );
		~tKDNode( );

		void fBuild( const tSortedKDData& potentialTris, const Math::tAabbf& parentVoxel, tAxis dontUseAxis );
		void fDestroy( b32 wasLoadedInPlace = false );
		u32 fMemorySizeRecursive( ) const;
	
	private:
		f32 fComputeSplitCost( const tSortedKDData& data, tAxis axis, f32 pos, const Math::tAabbf& parentVoxel );
		f32 fComputeLeafCost( const tSortedKDData& data );


		tFixedArray< tLoadInPlacePtrWrapper< tKDNode >, 2 > mChildren;
		tEnum<tAxis, u32>	mAxis;
		f32					mSplitPosition; // along axis

		tDynamicArray< u32 > mItems; //only populated for leafs
	};

	class base_export tPolySoupKDTree
	{
		declare_reflector( );

	public:
		void fConstruct( const tDynamicArray< Math::tVec3u >& triangles, const tDynamicArray< Math::tVec3f >& verts, b32 wasLoadedInPlace = false );
		u32 fMemorySizeRecursive( ) const;
		void fDraw( const Math::tMat3f& xform, Gfx::tDebugGeometryContainer& dgc, s32 level ) const;

		void fTestRay(
			tPolySoupRayCastHit& bestHit,
			const Math::tRayf& ray,
			const tPolySoupVertexList& verts, 
			const tPolySoupTriangleList& tris ) const;

		b32	fTestFrustum(
			const Math::tFrustumf& frustum,
			const tPolySoupVertexList& verts, 
			const tPolySoupTriangleList& tris ) const;

		void fCollectTris(
			tGrowableArray<Math::tTrianglef>& trisOut,
			const Math::tAabbf& aabb,
			const tPolySoupVertexList& verts, 
			const tPolySoupTriangleList& tris ) const;

	private:
		Math::tAabbf mBounds;
		tKDNode mFirstSplit;
	};

}

#endif//__tPolySoupKDTree__

