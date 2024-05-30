#ifndef __tHeightFieldMesh__
#define __tHeightFieldMesh__
#include "tEntityDef.hpp"
#include "tHeightFieldQuadTree.hpp"

namespace Sig { namespace Gfx
{
	class tMaterial;
}}

namespace Sig
{
	class tHeightFieldMeshEntity;

	///
	/// \brief
	struct base_export tHeightFieldRenderVertex
	{
		Math::tVec3f	P;
		Math::tVec3f	N;
		//u32			C;

		tHeightFieldRenderVertex( ) : N( Math::tVec3f::cYAxis )/*, C( 0xffffffff )*/ { }
		tHeightFieldRenderVertex( const Math::tVec3f& p, const Math::tVec3f& n ) : P(p), N( n )/*, C( 0xffffffff )*/ { }
	};
	
	struct base_export tCompressedHeightFieldRenderVertex
	{
		Math::tVector4<u16>	P;
		Math::tVector2<u16> N;

		tCompressedHeightFieldRenderVertex( ) : P(0), N(0) { }
		tCompressedHeightFieldRenderVertex( const Math::tVector4<u16>& p, const Math::tVector2<u16>& n ) : P(p), N(n) { }
	};

	///
	/// \brief
	class base_export tHeightFieldRez
	{
		declare_reflector( );
	private:

		f32 mWorldSpaceLengthX;
		f32 mWorldSpaceLengthZ;
		u32 mVertexResX;
		u32 mVertexResZ;
		u32 mLogicalVertResX;
		u32 mLogicalVertResZ;
		u32 mChunkQuadResX;
		u32 mChunkQuadResZ;
		u32 mNumChunksX;
		u32 mNumChunksZ;

	public:

		tHeightFieldRez( f32 wslx, f32 wslz, u32 vrx, u32 vrz, u32 cvrx, u32 cvrz );
		tHeightFieldRez( tNoOpTag );

		inline u32 fTotalVertCount( ) const { return mVertexResX * mVertexResZ; }
		inline u32 fTotalQuadCount( ) const { return ( mLogicalVertResX - 1 ) * ( mLogicalVertResZ - 1 ); }
		inline u32 fTotalTriangleCount( ) const { return 2 * fTotalQuadCount( ); }

		inline u32 fLogicalVertCount( ) const { return mLogicalVertResX * mLogicalVertResZ; }
		inline u32 fNumLogicalVertsX( ) const { return mLogicalVertResX; }
		inline u32 fNumLogicalVertsZ( ) const { return mLogicalVertResZ; }
		inline void fLogicalVertIndexInverse( u32 realIdx, u32& x, u32& z ) const { z = realIdx / mLogicalVertResX; x = realIdx % mLogicalVertResX; }
		inline u32 fLogicalVertIndex( u32 x, u32 z ) const { return mLogicalVertResX * z + x; }
		inline u32 fLogicalVertIndex( u32 chunkx, u32 chunkz, u32 x, u32 z ) const {
			return fLogicalVertIndex( chunkx * fChunkQuadResX( ) + x, chunkz * fChunkQuadResZ( ) + z ); }
		inline f32 fLogicalVertexWorldX( u32 logicalVertexX ) const {
			return Math::fLerp( fMinX( ), fMaxX( ), ( logicalVertexX / ( mLogicalVertResX - 1.f ) ) ); }
		inline f32 fLogicalVertexWorldZ( u32 logicalVertexZ ) const {
			return Math::fLerp( fMinZ( ), fMaxZ( ), ( logicalVertexZ / ( mLogicalVertResZ - 1.f ) ) ); }
		inline f32 fWorldX( f32 worldX ) const {
			return fClamp( worldX, fMinX( ), fMaxX( ) ); }
		inline f32 fWorldZ( f32 worldZ ) const {
			return fClamp( worldZ, fMinZ( ), fMaxZ( ) ); }
		inline f32 fWorldXUV( f32 worldX ) const {
			return ( fWorldX( worldX ) - fMinX ( ) ) / mWorldSpaceLengthX; }
		inline f32 fWorldZUV( f32 worldZ ) const {
			return ( fWorldZ( worldZ ) - fMinZ( ) ) / mWorldSpaceLengthZ; }

		template<class tRoundFunc>
		inline u32 fWorldXLogicalQuad( f32 worldX, tRoundFunc roundFunc ) const {
			return roundFunc( fWorldXUV( worldX ) * ( mLogicalVertResX - 1 ) ); }
		template<class tRoundFunc>
		inline u32 fWorldZLogicalQuad( f32 worldZ, tRoundFunc roundFunc ) const {
			return roundFunc( fWorldZUV( worldZ ) * ( mLogicalVertResZ - 1 ) ); }
		inline u32 fWorldXLogicalVertex( f32 worldX ) const {
			return fRound<u32>( fWorldXUV( worldX ) * ( mLogicalVertResX ) ); }
		inline u32 fWorldZLogicalVertex( f32 worldZ ) const {
			return fRound<u32>( fWorldZUV( worldZ ) * ( mLogicalVertResZ ) ); }

		inline f32 fWorldLengthX( ) const { return mWorldSpaceLengthX; }
		inline f32 fWorldLengthZ( ) const { return mWorldSpaceLengthZ; }
		inline f32 fVertexDeltaX( ) const { return mWorldSpaceLengthX / ( mLogicalVertResX - 1.f ); }
		inline f32 fVertexDeltaZ( ) const { return mWorldSpaceLengthZ / ( mLogicalVertResZ - 1.f ); }

		inline u32 fNumChunks( ) const { return mNumChunksX * mNumChunksZ; }
		inline u32 fNumChunksX( ) const { return mNumChunksX; }
		inline u32 fNumChunksZ( ) const { return mNumChunksZ; }
		inline u32 fChunkIndex( u32 x, u32 z ) const { return mNumChunksX * z + x; }

		inline u32 fChunkVertCount( ) const { return ( mChunkQuadResX + 1 ) * ( mChunkQuadResZ + 1 ); }
		inline u32 fChunkVertexResX( ) const { return mChunkQuadResX + 1; }
		inline u32 fChunkVertexResZ( ) const { return mChunkQuadResZ + 1; }

		inline u32 fChunkQuadResX( ) const { return mChunkQuadResX; }
		inline u32 fChunkQuadResZ( ) const { return mChunkQuadResZ; }
		inline u32 fNumQuadsPerChunk( ) const { return mChunkQuadResX * mChunkQuadResZ; }

		inline f32 fChunkWorldCornerX( u32 chunkIndexX ) const {
			return Math::fLerp( fMinX( ), fMaxX( ), ( chunkIndexX / ( f32 )mNumChunksX ) ); }
		inline f32 fChunkWorldCornerZ( u32 chunkIndexZ ) const {
			return Math::fLerp( fMinZ( ), fMaxZ( ), ( chunkIndexZ / ( f32 )mNumChunksZ ) ); }

		inline f32 fChunkLocalX( u32 chunkVertexIndexX ) const {
			return ( chunkVertexIndexX / ( f32 )mChunkQuadResX ) * ( mWorldSpaceLengthX / mNumChunksX ); }
		inline f32 fChunkLocalZ( u32 chunkVertexIndexZ ) const {
			return ( chunkVertexIndexZ / ( f32 )mChunkQuadResZ ) * ( mWorldSpaceLengthZ / mNumChunksZ ); }

		inline f32 fMinX( ) const { return -0.5f * mWorldSpaceLengthX; }
		inline f32 fMaxX( ) const { return +0.5f * mWorldSpaceLengthX; }
		inline f32 fMinZ( ) const { return -0.5f * mWorldSpaceLengthZ; }
		inline f32 fMaxZ( ) const { return +0.5f * mWorldSpaceLengthZ; }

		inline Math::tAabbf fTotalBounds( f32 minHeight = 0.f, f32 maxHeight = 0.f ) const {
			return Math::tAabbf( Math::tVec3f( fMinX( ), minHeight, fMinZ( ) ), Math::tVec3f( fMaxX( ), maxHeight, fMaxZ( ) ) ); }

		inline Math::tAabbf fChunkBounds( u32 chunkIndexX, u32 chunkIndexZ ) const {
			const Math::tVec3f corner = Math::tVec3f( fChunkWorldCornerX( chunkIndexX ), 0.f, fChunkWorldCornerZ( chunkIndexZ ) );
			return Math::tAabbf( corner, corner + Math::tVec3f( fChunkLocalX( mChunkQuadResX ), 0.f, fChunkLocalZ( mChunkQuadResZ ) ) ); }
	};



	class base_export tHeightFieldMesh : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tHeightFieldMesh, 0xD86C030F );

	public:

		static const f32 cHeightHole;

		enum tQuadState
		{
			cQuadNormal = 0,
			cQuadFlipped,
			cQuadRemoved,
		};

		///
		/// \brief A "logical" vertex doubles as both vertex and quad data, as they are
		/// logically very similar and have nearly a 1-1 correspondence.
		struct base_export tLogicalVertex
		{
			declare_reflector( );
		private:
			u32						mBaseRenderVertexId;
			u32						mBaseRenderIndexId;
			tFixedArray<u16,4>		mRenderVertexIds;
			f32						mHeight;
			tEnum<tQuadState,u8>	mQuadState;
			u8						mNumRenderVertexIds;
			u8						pad0, pad1;

		public:

			inline tLogicalVertex( ) { fZeroOut( this ); }

			inline f32			fGetHeight( ) const { return mHeight; }
			inline void			fSetHeight( f32 h ) { mHeight = h; }

			inline tQuadState	fGetQuadState( ) const { return mQuadState; }
			inline void			fSetQuadState( tQuadState qs ) { mQuadState = qs; }

			inline u32			fGetNumRenderVertexIds( ) const { return mNumRenderVertexIds; }
			inline u32			fGetRenderVertexId( u32 ithId ) const { return mBaseRenderVertexId + mRenderVertexIds[ ithId ]; }

			inline void			fSetBaseRenderIndexId( u32 baseIdx ) { mBaseRenderIndexId = baseIdx; }
			inline u32			fGetMinRenderIndexId( ) const { return mBaseRenderIndexId; }
			inline u32			fGetMaxRenderIndexId( ) const { return mBaseRenderIndexId + 6; }

			void				fClearRenderVertexIds( );
			void				fAddRenderVertexId( u32 id );
		};

		typedef tDynamicArray< tLogicalVertex > tLogicalVertexArray;

	protected:
		tHeightFieldRez			mRez;
		tLogicalVertexArray		mLogicalVerts;
		tHeightFieldQuadTree	mQuadTree;

	public:
		tHeightFieldMesh( const Math::tVec2f& worldSpaceLengths, const Math::tVec2i& vertexRes );
		tHeightFieldMesh( tNoOpTag );
		~tHeightFieldMesh( );

		void fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const;
		b32  fTestRay( const Math::tRayf& rayInObject, Math::tRayCastHit& bestHit ) const;
		b32  fTestFrustum( const Math::tFrustumf& frustumInObject ) const;

		inline const tHeightFieldRez&	fRez( ) const { return mRez; }
		void							fComputeLogicalQuadAabb( u32 logicalX, u32 logicalZ, Math::tAabbf& aabb ) const;
		b32								fComputeLogicalTris( u32 logicalX, u32 logicalZ, Math::tTrianglef& tri0, Math::tTrianglef& tri1 ) const;
		Math::tVec3f					fComputeLogicalVertexPosition( u32 logicalX, u32 logicalZ ) const;
		Math::tVec3f					fComputeLogicalVertexNormal( u32 logicalX, u32 logicalZ ) const;
		void							fGetTerrainQuadIndices( f32 localX, f32 localZ, u32& ileft, u32& iright, u32& itop, u32& ibottom, f32& dx, f32& dz ) const;
		f32								fSampleHeight( f32 localX, f32 localZ, const tQuadState* overrideQuadState = 0 ) const;
		tQuadState						fComputeQuadTriangulation( u32 logicalX, u32 logicalZ ) const;
		void							fComputeQuadRenderIndices( tQuadState quadState, u32 ix, u32 iz, u16 idsOut[6] );
		void							fComputeQuadRenderIndicesGeneralized( u32 numVertsPerRow, tQuadState quadState, u32 ix, u32 iz, u32 idsOut[6] );
		inline f32						fLogicalHeight( u32 logicalX, u32 logicalZ ) const 
				{ return mLogicalVerts[ mRez.fLogicalVertIndex( logicalX, logicalZ ) ].fGetHeight( ); }
		inline Math::tAabbf				fComputeBounds( ) const 
				{ return mRez.fTotalBounds( mQuadTree.fMinMaxHeight( ).x, mQuadTree.fMinMaxHeight( ).y ); }
		Math::tAabbf					fComputeChunkBounds( u32 ichunkx, u32 ichunkz ) const;

		struct tHeightField
		{
			tGrowableArray<f32> mHeights;
			tGrowableArray<b32> mQuadsDisabled;

			b32 fHasQuadData( ) const { return mHeights.fCount( ) > 0 && mQuadsDisabled.fCount( ) == mHeights.fCount( ); }
		};

		void 							fSaveHeightField( tHeightField& heightField ) const;
		void 							fRestoreHeightField( const tHeightField& heightField, b32 refreshGfx = true );
		void 							fRestoreHeightField( const tHeightField& heightField, u32 minx, u32 minz, u32 maxx, u32 maxz, b32 refreshGfx = true );

		const tLogicalVertexArray&		fGetLogicalVerts( ) const { return mLogicalVerts; }
		void							fSetLogicalVerts( const tLogicalVertexArray& other ) { mLogicalVerts = other; }

		const tHeightFieldQuadTree&		fGetQuadTree( ) const { return mQuadTree; }
		void							fSetQuadTree( const tHeightFieldQuadTree& other ) { mQuadTree.fCopy( other ); }

	protected:
		void fInitializeRenderVerts( tHeightFieldRenderVertex* renderVerts, u32 vertCount );
		void fInitializeRenderIndices( u16* renderIds, u32 idsCount );
		void fRefreshLogicalBlock( u32 xmin, u32 zmin, u32 xmax, u32 zmax );
		void fRefreshGraphicsBlock( u32 xmin, u32 zmin, u32 xmax, u32 zmax, u32 minRenderVtxIdx, u32 maxRenderVtxIdx );
		virtual tHeightFieldRenderVertex* fLockRenderVerts( u32 startVertex = 0, u32 numVerts = ~0 ) = 0;
		virtual void fUnlockRenderVerts( tHeightFieldRenderVertex* lockedVerts ) = 0;
		virtual u16* fLockRenderIds( u32 startIndex = 0, u32 numIds = ~0 ) = 0;
		virtual void fUnlockRenderIds( u16* lockedIds ) = 0;
	};

}

#endif//__tHeightFieldMesh__
