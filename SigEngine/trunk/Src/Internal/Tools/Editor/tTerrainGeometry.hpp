//------------------------------------------------------------------------------
// \file tTerrainGeometry.hpp - 02 Sep 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTerrainGeometry__
#define __tTerrainGeometry__
#include "Sigml.hpp"
#include "tHeightFieldMesh.hpp"
#include "tResourceDepot.hpp"
#include "tTextureSysRam.hpp"
#include "Gfx/tHeightFieldMaterial.hpp"
#include "Gfx/tViewport.hpp"
#include "Gfx/tDynamicTextureVRam.hpp"
#include "Gfx/tGeometryBufferSysRam.hpp"
#include "Gfx/tGeometryBufferVRam.hpp"
#include "Gfx/tIndexBufferSysRam.hpp"
#include "Gfx/tIndexBufferVRam.hpp"
#include "Gfx/tRenderBatch.hpp"
#include "Gfx/tDeviceResource.hpp"
#include "tXmlBase64.hpp"

namespace Sig
{

	///
	/// \brief
	class tools_export tTerrainGeometry : public tHeightFieldMesh, public Gfx::tDeviceResource
	{
	public:

		enum tMaterialType
		{
			cMaterialTypeDefault,
			cMaterialTypeTransition,

			cMaterialTypeCount
		};
		static const u32 cTransitionMaterialSpan = 4;

		template<class EditType>
		class tEditableRegion
		{
		public:

			tEditableRegion( )
				: mDimX( 0 )
				, mDimZ( 0 )
				, mMinX( 0.f )
				, mMinZ( 0.f )
				, mMaxX( 0.f )
				, mMaxZ( 0.f ) { }

			inline void fSetDimensions( u32 dimX, u32 dimZ, f32 minX, f32 minZ, f32 maxX, f32 maxZ, u32 perItem = 1  )
			{ 
				mItems.fNewArray( ( dimX * dimZ ) * perItem ); 
				mPerItem = perItem; 
				mDimX = dimX; mDimZ = dimZ; 
				mMinX = minX; mMinZ = minZ; 
				mMaxX = maxX; mMaxZ = maxZ; 
			}
			
			inline EditType&		fIndex( u32 x, u32 z, u32 i = 0 )		{ return mItems[ ( z * mDimX + x ) * mPerItem + i ]; }
			inline const EditType&	fIndex( u32 x, u32 z, u32 i = 0 ) const	{ return mItems[ ( z * mDimX + x ) * mPerItem + i ]; }

			inline u32	fDimX( ) const		{ return mDimX; }
			inline u32	fDimZ( ) const		{ return mDimZ; }
			inline f32	fMinX( ) const		{ return mMinX; }
			inline f32	fMinZ( ) const		{ return mMinZ; }
			inline f32	fMaxX( ) const		{ return mMaxX; }
			inline f32	fMaxZ( ) const		{ return mMaxZ; }
			inline u32	fPerItem( ) const	{ return mPerItem; }

		protected:

			tDynamicArray< EditType > mItems;
			u32 mDimX, mDimZ;
			f32 mMinX, mMinZ;
			f32 mMaxX, mMaxZ;

			u32 mPerItem;
		};

		///
		/// \brief
		struct tools_export tEditableVertex
		{
			friend class tTerrainGeometry;
		public:
			f32								mLocalHeight;
			tHeightFieldMesh::tQuadState	mQuadState;
		private:
			f32 mLocalX, mLocalZ;
			u32 mLogicalX, mLogicalZ;
		public:
			inline tEditableVertex( ) { fZeroOut( this ); }
			inline f32 fLocalX( ) const { return mLocalX; }
			inline f32 fLocalZ( ) const { return mLocalZ; }
			inline u32 fLogicalX( ) const { return mLogicalX; }
			inline u32 fLogicalZ( ) const { return mLogicalZ; }
		};

		///
		/// \brief
		class tools_export tEditableVertices : public tEditableRegion< tEditableVertex >
		{
			friend class tTerrainGeometry;
		private:
			u32 mMinRenderVtxIdx, mMaxRenderVtxIdx;
			f32 mMinHeight, mMaxHeight, mAvgHeight;
		public:
			tEditableVertices( ) 
				: mMinRenderVtxIdx( 0 )
				, mMaxRenderVtxIdx( 0 )
				, mMinHeight( 0.f )
				, mMaxHeight( 0.f )
				, mAvgHeight( 0.f )
			{ }

			inline f32						fMinHeight( ) const					{ return mMinHeight; }
			inline f32						fMaxHeight( ) const					{ return mMaxHeight; }
			inline f32						fAvgHeight( ) const					{ return mAvgHeight; }
		};

		///
		/// \class tEditableGroundCoverMask
		/// \brief 
		class tools_export tEditableGroundCoverMask : public tEditableRegion< f32 >
		{
		public:
			
			tEditableGroundCoverMask( ) { }

			u32 fId( ) const { return mGroundCoverId; }
			u32 fStartX( ) const { return mStartX; }
			u32 fStartZ( ) const { return mStartZ; }

		private:
			
			friend class tTerrainGeometry;

			u32 mGroundCoverId;
			u32 mStartX, mStartZ;
		};

		///
		/// \class tEditableGroundCoverHeights
		/// \brief 
		class tools_export tEditableGroundCoverHeights : public tEditableRegion< f32 >
		{
		public:

			tEditableGroundCoverHeights( ) { }

			u32 fId( ) const { return mGroundCoverId; }
			u32 fStartX( ) const { return mStartX; }
			u32 fStartZ( ) const { return mStartZ; }
			
		private:

			friend class tTerrainGeometry;

			u32 mGroundCoverId;
			u32 mStartX, mStartZ;
		};

		///
		/// \class tGroundCover
		/// \brief 
		struct tGroundCover
		{
			tGroundCover( ) : mCoverId( ~0 ), mDimX( 0 ), mDimZ( 0 ), mMaxUnitSpawns( 0 ), mUnitSize( 0 ) { }
			tGroundCover( u32 id ) : mCoverId( id ), mDimX( 0 ), mDimZ( 0 ), mMaxUnitSpawns( 0 ), mUnitSize( 0 ) { }

			u32 mCoverId;
			u32 mDimX, mDimZ;
			u32 mMaxUnitSpawns;
			f32 mUnitSize;

			tDynamicArray<f32> mMask; // mDimX * mDimZ
			tDynamicArray<f32> mHeights; // ( mDimX * mDimZ ) * mMaxUnitSpawns

			template<class tSerializer>
			void fSerializeXml( tSerializer& s )
			{
				s( "CoverId", mCoverId );
				s( "DimX", mDimX );
				s( "DimZ", mDimZ );
				s( "MaxUnitSpawns", mMaxUnitSpawns );
				s( "UnitSize", mUnitSize );
				s( "Mask", tXmlBase64DynamicArraySerializer<f32>( mMask ) );
				s( "Heights", tXmlBase64DynamicArraySerializer<f32>( mHeights ) );
			}
		};

	private:

		struct tGroundCoverInternal : public tGroundCover
		{
			tGroundCoverInternal( ) : mEditors( 0 ) { }

			tGroundCoverInternal & operator= ( const tGroundCover & cover )
			{
				*(tGroundCover *)this = cover;
				return *this;
			}

			u32 mEditors;
		};

		Gfx::tGeometryBufferVRam					mVRamVerts;
		Gfx::tIndexBufferVRam						mVRamIndices;
		tResourcePtr								mTerrainMaterialFile;
		Gfx::tHeightFieldMaterialBasePtr			mTerrainMaterial;
		tMaterialType								mHFMatType;

		tTextureSysRam::tSurface					mMaskTexture;
		tTextureSysRam::tSurface					mMtlIdsTexture;

		tGrowableArray<tGroundCoverInternal *>		mGroundCover;

		Gfx::tGeometryBufferVRam&		fGetGpuVerts( ) { return mVRamVerts; }
		Gfx::tIndexBufferVRam&			fGetGpuIndices( ) { return mVRamIndices; }
		const Gfx::tGeometryBufferVRam&	fGetGpuVerts( ) const { return mVRamVerts; }
		const Gfx::tIndexBufferVRam&	fGetGpuIndices( ) const { return mVRamIndices; }

	public:

	
		tTerrainGeometry( 
			const tResourceDepotPtr& resourceDepot,
			const Gfx::tDevicePtr& device, 
			const Math::tVec2f& worldSpaceLengths, 
			const Math::tVec2i& vertexRes,
			const Math::tVec2i& materialRes,
			const tResourcePtr& diffuseMap,
			const tResourcePtr& normalMap );
		~tTerrainGeometry( );
		virtual void fOnDeviceLost( Gfx::tDevice* device ) { }
		virtual void fOnDeviceReset( Gfx::tDevice* device );

		tTextureSysRam::tSurface&		fGetMaskTexture( ) { return mMaskTexture; }
		tTextureSysRam::tSurface&		fGetMtlIdsTexture( ) { return mMtlIdsTexture; }
		void fUpdateDynamicTextureReferences( const tResourcePtr& diffuseMap, const tResourcePtr& normalMap );
		void fUpdateMaterialTilingFactors( const tDynamicArray<f32>& tilingFactors );
		void fCopyMaterialTilingFactors( const tTerrainGeometry& other );
		void fBeginEditingVerts( tEditableVertices& editableVerts, f32 minLocalX, f32 minLocalZ, f32 maxLocalX, f32 maxLocalZ ) const;
		void fEndEditingVerts( const tEditableVertices& editableVerts );

		b32 fRestoreGroundCover( u32 id, f32 unitSize, u32 maxUnitSpawns );
		void fAddGroundCover( u32 id, f32 unitSize, u32 maxUnitSpawns );
		void fRemoveGroundCover( u32 id );
		void fBeginEditingGroundCoverMask( 
			u32 id, tEditableGroundCoverMask & editableMask,
			f32 minLocalX, f32 minLocalZ, f32 maxLocalX, f32 maxLocalZ );
		void fEndEditingGroundCoverMask( const tEditableGroundCoverMask & editableMask );
		b32 fUpdateGroundCoverDims( 
			u32 id, f32 unitSize, u32 maxUnitSpawns, b32 preserveSpacing, b32 preserveLeft, b32 preserveTop );
		void fBeginEditingGroundCoverHeights( u32 id, tEditableGroundCoverHeights & editableHeights );
		void fBeginEditingGroundCoverHeights( 
			u32 id, tEditableGroundCoverHeights & editableHeights, 
			f32 minLocalX, f32 minLocalZ, f32 maxLocalX, f32 maxLocalZ );
		void fEndEditingGroundCoverHeights( const tEditableGroundCoverHeights & editableHeights );
		void fQueryGroundCoverMask( u32 id, u32 minX, u32 minZ, u32 maxX, u32 maxZ, tDynamicArray< f32 > & mask );
		void fQueryGroundCoverHeights( u32 id, tDynamicArray< f32 > & heights );
		void fQueryGroundCoverHeights( u32 id, u32 minX, u32 minZ, u32 maxX, u32 maxZ, tDynamicArray< f32 > & heights );

		void fSaveGroundCover( tGroundCover & gc );
		void fSaveGroundCover( tDynamicArray<tGroundCover> & groundCover );
		void fRestoreGroundCover( const tGroundCover & gc );
		void fRestoreGroundCover( const tDynamicArray<tGroundCover> & groundCover );

		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;
		virtual tHeightFieldRenderVertex* fLockRenderVerts( u32 startVertex = 0, u32 numVerts = ~0 );
		virtual void fUnlockRenderVerts( tHeightFieldRenderVertex* lockedVerts );
		virtual u16* fLockRenderIds( u32 startIndex = 0, u32 numIds = ~0 );
		virtual void fUnlockRenderIds( u16* lockedIds );

		tMaterialType fHeightFieldMaterialType( ) const { return mHFMatType; }

	private:

		tGroundCoverInternal * fFindGroundCover( u32 id );
		b32 fUpdateGroundCoverDims( 
			tGroundCoverInternal & cover, f32 unitSize, u32 maxUnitSpawns, b32 preserveSpacing, b32 preserveLeft, b32 preserveTop );

	public:

		void fAcquireMaterial( 
			const Gfx::tDevicePtr& device, 
			const tResourceDepotPtr& resourceDepot,
			tMaterialType hfType,
			const Math::tVec2i& materialRes,
			const tResourcePtr& diffuseMap,
			const tResourcePtr& normalMap );
	};

	class tHeightFieldMeshEntityDef;

	///
	/// \brief Used to translate heightfield geometry from editor format to binary, game-ready format.
	class tools_export tHeightFieldGeometryConverter : public tHeightFieldMesh
	{
	public:

		tHeightFieldGeometryConverter( 
			const Math::tVec2f& worldSpaceLengths, 
			const Math::tVec2i& vertexRes,
			const tHeightField& heightField );

		void fConvertToEntityDef( tHeightFieldMeshEntityDef* entityDefOut, tSigmlConverter& sigmlConverter, const f32 optimizeTarget );
		
		void fDumpRawTriangles( 
			tGrowableArray< Math::tVec3f >& rawVerts, 
			tGrowableArray< Math::tVec3u >& rawTris );

		void fDumpRawTriangles(
			tGrowableArray< Math::tVec3f >& rawVerts,
			tGrowableArray< tGrowableArray< Math::tVec3u > >& splitTris );

	private:

		void fConstructVerts( 
			u32 vertCount, const Math::tVec3f verts[],
			u32 triCount, const Math::tVec3u tris[],
			tDynamicArray<tCompressedHeightFieldRenderVertex> & out );

		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const { } // TODO it'd be nice if we didn't need to override this
		virtual tHeightFieldRenderVertex* fLockRenderVerts( u32 startVertex = 0, u32 numVerts = ~0 );
		virtual void fUnlockRenderVerts( tHeightFieldRenderVertex* lockedVerts );
		virtual u16* fLockRenderIds( u32 startIndex = 0, u32 numIds = ~0 );
		virtual void fUnlockRenderIds( u16* lockedIds );
	};

}

#endif//__tTerrainGeometry__
