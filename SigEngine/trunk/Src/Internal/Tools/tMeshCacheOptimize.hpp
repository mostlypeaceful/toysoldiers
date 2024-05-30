#ifndef __tMeshCacheOptimize__
#define __tMeshCacheOptimize__

namespace Sig
{
	namespace Gfx { class tGeometryBufferSysRam; }

	class tools_export tMeshCacheOptimize
	{
	public:
		tMeshCacheOptimize( Gfx::tGeometryBufferSysRam& verts, tDynamicArray< u32 >& triangleIndices, b32 reorderVerts, tGrowableArray< tDynamicArray< u32 > >& additionalIndexBuffersToReorder );

	private:
		struct tVertexData
		{
			u32 mCachePosition;
			f32 mCurrentScore;
			u32 mNewIndex;			// used in final stage of vertex reordering
			u32 mNumToUse;			// number of triangles that haven't used this vertex yet
			tGrowableArray< u32 > mTriangleList; // Triangles yet to use this vertex are in front. [0, mNumToUse).

			tVertexData( );
			void fIncTriCount( );
			void fAddTriangle( u32 index );
			void fComputeScore( );
			b32 fUsed( ) const { return mNewIndex != ~0; }
		};

		struct tTriangleData
		{
			Math::tVec3u mIndices;
			b32 mUsed;
			f32 mScore;

			tTriangleData( )
				: mUsed( false )
				, mScore( 0 )
			{ }

			void fComputeScore( const tGrowableArray< tVertexData >& data );
		};

		tGrowableArray< tVertexData > mVertData;
		tGrowableArray< tTriangleData > mTriData;
		tGrowableArray< u32 > mCacheSim;

		tGrowableArray< u32 > mOrderedTriangleOutput;
		u32 mNextBestTriangle;

		void fBuildData( const Gfx::tGeometryBufferSysRam& verts, const tDynamicArray< u32 >& triangleIndices );
		void fDumpData( Gfx::tGeometryBufferSysRam& verts, tDynamicArray< u32 >& triangleIndices, b32 reorderVerts, tGrowableArray< tDynamicArray< u32 > >& additionalIndexBuffersToReorder );
		void fRemapIndexBuffer( tDynamicArray< u32 >& indices ) const;

		void fUseVertex( u32 index, u32 forTri );
		void fUseTriangle( u32 index );
		void fCleanupCache( );
		void fIterateData( );
	};
}

#endif //__tMeshCacheOptimize__
