#ifndef __tEditableTerrainEntity__
#define __tEditableTerrainEntity__
#include "tEditableObject.hpp"
#include "tTerrainGeometry.hpp"

namespace Sig
{
	namespace Sigml { class tHeightFieldMeshObject; }

	class tools_export tEditableTerrainGeometry : public tEditableObject
	{
		define_dynamic_cast( tEditableTerrainGeometry, tEditableObject );
	public:
		typedef tTerrainGeometry::tEditableVertex tEditableVertex;
		typedef tTerrainGeometry::tEditableVertices tEditableVertices;
		typedef tTerrainGeometry::tEditableGroundCoverMask tEditableGroundCoverMask;
		typedef tTerrainGeometry::tEditableGroundCoverHeights tEditableGroundCoverHeights;
		typedef tTerrainGeometry::tGroundCover tGroundCover;
		typedef tTerrainGeometry::tHeightField tHeightField;

		tEditableTerrainGeometry( tEditableObjectContainer& container );
		virtual const tHeightFieldRez& fRez( ) const = 0;
		virtual void fBeginEditingVerts( tEditableVertices& editableVerts, const Math::tVec3f& centerInLocal, f32 xlen, f32 zlen ) = 0;
		virtual void fEndEditingVerts( const tEditableVertices& editableVerts ) = 0;
		virtual f32	 fSampleHeight( f32 localX, f32 localZ ) const = 0;
		virtual void fSaveHeightField( tHeightField& heightField ) const = 0;
		virtual void fRestoreHeightField( const tHeightField& heightField ) = 0;
		virtual void fRestoreHeightField( const tHeightField& heightField, u32 minx, u32 minz, u32 maxx, u32 maxz ) = 0;
		virtual void fResetGeometry( const tStrongPtr<tTerrainGeometry>& oldGeometry ) = 0;
		virtual void fSaveHeightFieldToBitmap( const tFilePathPtr& path ) const = 0;
		virtual tTextureSysRam::tSurface& fGetMaskTexture( ) = 0;
		virtual tTextureSysRam::tSurface& fGetMtlIdsTexture( ) = 0;
		virtual void fUpdateDynamicTextureReferences( ) = 0;
		virtual void fUpdateMaterialTilingFactors( const tDynamicArray<f32>& tilingFactors ) = 0;

		virtual b32 fInitializeGroundCover( ) = 0;
		virtual void fSetGroundCoverInitialized( ) = 0;
		virtual void fRestoreGroundCover( const Sigml::tGroundCoverLayer & layer ) = 0;
		virtual void fAddGroundCover( const Sigml::tGroundCoverLayer & layer ) = 0;
		virtual void fRemoveGroundCover( u32 id ) = 0;
		virtual void fSaveGroundCover( tGroundCover & cover ) = 0;
		virtual void fRestoreGroundCover( const tGroundCover & cover ) = 0;
		virtual void fBeginEditingGroundCoverMask( 
			const Sigml::tGroundCoverLayer & layer, tEditableGroundCoverMask & editableMask,
			const Math::tVec3f& centerInLocal, f32 xlen, f32 zlen ) = 0;
		virtual void fEndEditingGroundCoverMask( const tEditableGroundCoverMask & editableMask ) = 0;
		virtual void fUpdateGroundCover( const Sigml::tGroundCoverLayer & layer ) = 0;
		virtual void fUpdateGroundCoverVisiblity( const Sigml::tGroundCoverLayer & layer ) = 0;
		virtual void fUpdateGroundCoverShadows( const Sigml::tGroundCoverLayer & layer ) = 0;
		virtual void fUpdateGroundCoverFrequency( const Sigml::tGroundCoverLayer & layer ) = 0;
		virtual void fUpdateGroundCoverSpawnCount( const Sigml::tGroundCoverLayer & layer ) = 0;
		virtual void fRefreshGroundCoverHeights( const Sigml::tGroundCoverLayer & layer ) = 0;
	};

	class tools_export tEditableTerrainEntity : public tEditableTerrainGeometry
	{
		define_dynamic_cast( tEditableTerrainEntity, tEditableTerrainGeometry );
	private:

		tStrongPtr<tTerrainGeometry> mTerrainGeometry;
		tEntityPtr mHeightFieldRoot;

		tHashTable<u32, tEntityPtr> mGroundCoverEntities;
		b32 mGroundCoverInitialized;

	public:
		tEditableTerrainEntity( tEditableObjectContainer& container );
		tEditableTerrainEntity( tEditableObjectContainer& container, const Sigml::tHeightFieldMeshObject& sigmlObject );
		~tEditableTerrainEntity( );
		std::string fGetToolTip( ) const;
		void fCleanTextureMaterials( u32 firstMat, u32 secondMat, u32 thirdMat );
	private:
		void fCommonCtor( );
		void fNewTerrainGeometry( const Math::tVec2f& dims, const Math::tVec2i& tess, const Math::tVec2i& mtlRes );
		void fRefreshRenderables( );
		void fUpdateChunkBounds( u32 iChunkXMin, u32 iChunkXMax, u32 iChunkZMin, u32 iChunkZMax );
		void fUpdateBounds( );
		void fUpdateGroundCoverHeights( u32 id );
		void fUpdateGroundCoverHeights( tEditableGroundCoverHeights & editor );

	protected:
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual b32 fSupportsScale( ) { return false; }
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
		virtual const tHeightFieldRez& fRez( ) const;
		virtual void fBeginEditingVerts( tEditableVertices& editableVerts, const Math::tVec3f& centerInLocal, f32 xlen, f32 zlen );
		virtual void fEndEditingVerts( const tEditableVertices& editableVerts );
		virtual f32	 fSampleHeight( f32 localX, f32 localZ ) const;
		virtual void fSaveHeightField( tHeightField& heightField ) const;
		virtual void fRestoreHeightField( const tHeightField& heightField );
		virtual void fRestoreHeightField( const tHeightField& heightField, u32 minx, u32 minz, u32 maxx, u32 maxz );
		virtual void fResetGeometry( const tStrongPtr<tTerrainGeometry>& oldGeometry );
		virtual void fSaveHeightFieldToBitmap( const tFilePathPtr& path ) const;
		virtual tTextureSysRam::tSurface& fGetMaskTexture( );
		virtual tTextureSysRam::tSurface& fGetMtlIdsTexture( );
		virtual void fUpdateDynamicTextureReferences( );
		virtual void fUpdateMaterialTilingFactors( const tDynamicArray<f32>& tilingFactors );

		virtual b32 fInitializeGroundCover( );
		virtual void fSetGroundCoverInitialized( );
		virtual void fRestoreGroundCover( const Sigml::tGroundCoverLayer & layer );
		virtual void fAddGroundCover( const Sigml::tGroundCoverLayer & layer );
		virtual void fRemoveGroundCover( u32 id );
		virtual void fSaveGroundCover( tGroundCover & cover );
		virtual void fRestoreGroundCover( const tGroundCover & cover );

		virtual void fBeginEditingGroundCoverMask( 
			const Sigml::tGroundCoverLayer & layer, tEditableGroundCoverMask & editableMask,
			const Math::tVec3f& centerInLocal, f32 xlen, f32 zlen );
		virtual void fEndEditingGroundCoverMask( const tEditableGroundCoverMask & editableMask );
		virtual void fUpdateGroundCover( const Sigml::tGroundCoverLayer & layer );
		virtual void fUpdateGroundCoverVisiblity( const Sigml::tGroundCoverLayer & layer );
		virtual void fUpdateGroundCoverShadows( const Sigml::tGroundCoverLayer & layer );
		virtual void fUpdateGroundCoverFrequency( const Sigml::tGroundCoverLayer & layer );
		virtual void fUpdateGroundCoverSpawnCount( const Sigml::tGroundCoverLayer & layer );
		virtual void fRefreshGroundCoverHeights( const Sigml::tGroundCoverLayer & layer );
		virtual void fRestoreGroundCover( const tDynamicArray<tTerrainGeometry::tGroundCover> & gcs );

	};

}

#endif//__tEditableTerrainEntity__
