#include "ToolsPch.hpp"
#include "tEditableTerrainEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tTextureSysRam.hpp"
#include "tHeightFieldMeshObject.hpp"
#include "tHeightFieldMeshEntity.hpp"
#include "Gfx/tGroundCoverCloud.hpp"
#include "tEditableGroundCoverCloud.hpp"

namespace Sig
{
	namespace
	{
		static inline const char* fStrTesselation( ) { return Sigml::tHeightFieldMeshObject::fStrTesselation( ); }
		static inline const char* fStrDimensions( ) { return Sigml::tHeightFieldMeshObject::fStrDimensions( ); }
		static inline const char* fStrMaterialRes( ) { return Sigml::tHeightFieldMeshObject::fStrMaterialRes( ); }
		static inline const char* fStrOptimizationTarget( ) { return Sigml::tHeightFieldMeshObject::fStrOptimizationTarget( ); }

		static const Math::tVec2f cDefaultTesselation = Sigml::tHeightFieldMeshObject::fDefaultTesselation( );
		static const Math::tVec2f cDefaultDimensions = Sigml::tHeightFieldMeshObject::fDefaultDimensions( );
		static const Math::tVec2i cDefaultMaterialRes = Sigml::tHeightFieldMeshObject::fDefaultMaterialRes( );
	}

	class tChangeTerrainResolutionAction : public tEditorAction
	{
		tEntityPtr mEntity;
		tStrongPtr<tTerrainGeometry> mOldGeometry, mNewGeometry;
	public:
		tChangeTerrainResolutionAction( 
			const tEntityPtr& entity, 
			const tStrongPtr<tTerrainGeometry>& oldGeometry,
			const tStrongPtr<tTerrainGeometry>& newGeometry )
			: mEntity( entity )
			, mOldGeometry( oldGeometry )
			, mNewGeometry( newGeometry )
		{
			fChangeDevice( mOldGeometry, tStrongPtr<tTerrainGeometry>( ) );
		}
		virtual void fUndo( )
		{
			fChangeDevice( mNewGeometry, mOldGeometry );
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fResetGeometry( mOldGeometry );
		}
		virtual void fRedo( )
		{
			fChangeDevice( mOldGeometry, mNewGeometry );
			mEntity->fDynamicCast< tEditableTerrainGeometry >( )->fResetGeometry( mNewGeometry );
		}
	private:
		void fChangeDevice( const tStrongPtr<tTerrainGeometry>& refDevice, const tStrongPtr<tTerrainGeometry>& realDevice )
		{
			if( !refDevice.fNull( ) )
			{
				refDevice->fGetMaskTexture( ).fChangeDevice( refDevice->fGetMaskTexture( ).fGetReferenceDevice( ) );
				refDevice->fGetMtlIdsTexture( ).fChangeDevice( refDevice->fGetMtlIdsTexture( ).fGetReferenceDevice( ) );
			}
			if( !realDevice.fNull( ) )
			{
				realDevice->fGetMaskTexture( ).fChangeDevice( Gfx::tDevice::fGetDefaultDevice( ) );
				realDevice->fGetMtlIdsTexture( ).fChangeDevice( Gfx::tDevice::fGetDefaultDevice( ) );
			}
		}
	};

	tEditableTerrainGeometry::tEditableTerrainGeometry( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
	}

	tEditableTerrainEntity::tEditableTerrainEntity( tEditableObjectContainer& container )
		: tEditableTerrainGeometry( container )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropInvisible( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropCastShadow( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropReceiveShadow( ), true ) ) );

		fCommonCtor( );
		fUpdateBounds( );
	}

	tEditableTerrainEntity::tEditableTerrainEntity( tEditableObjectContainer& container, const Sigml::tHeightFieldMeshObject& sigmlObject )
		: tEditableTerrainGeometry( container )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropInvisible( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropCastShadow( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropReceiveShadow( ), true ) ) );

		fDeserializeBaseObject( &sigmlObject );
		fCommonCtor( );

		if( sigmlObject.mMaterialMaskTexture.fAllocated( ) )
			mTerrainGeometry->fGetMaskTexture( ).fCopyCpu( Gfx::tDevice::fGetDefaultDevice( ), sigmlObject.mMaterialMaskTexture );
		if( sigmlObject.mMaterialIdsTexture.fAllocated( ) )
			mTerrainGeometry->fGetMtlIdsTexture( ).fCopyCpu( Gfx::tDevice::fGetDefaultDevice( ), sigmlObject.mMaterialIdsTexture );
		fUpdateDynamicTextureReferences( );
		fRestoreHeightField( sigmlObject.mHeightField );
		fRestoreGroundCover( sigmlObject.mGroundCover );
	}

	tEditableTerrainEntity::~tEditableTerrainEntity( )
	{
		mHeightFieldRoot->fClearChildren( );
	}

	std::string tEditableTerrainEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return "HeightField - " + name;
		return "HeightField";
	}

	void tEditableTerrainEntity::fCleanTextureMaterials( u32 firstMat, u32 secondMat, u32 thirdMat )
	{
		tTextureSysRam::tSurface& maskTex = fGetMaskTexture( );
		tTextureSysRam::tSurface& idsTex = fGetMtlIdsTexture( );

		// Get the top mips and lock them.
		Gfx::tTextureFile::tLockedMip idsMip = idsTex.fLock( );
		Gfx::tTextureFile::tLockedMip maskMip = maskTex.fLock( );

		// Clear out all the materials in the ids texture.
		for( u32 v = 0; v < idsTex.fHeight( ); ++v )
		{
			for( u32 u = 0; u < idsTex.fWidth( ); ++u )
			{
				u16* idsTexel = idsMip.fGetTexel<u16>( u, v );
				*idsTexel = Gfx::tTextureVRam::fPackColorR5G6B5( firstMat, secondMat, thirdMat );
			}
		}

		// Set the first channel to be dominant.
		for( u32 v = 0; v < maskTex.fHeight( ); ++v )
		{
			for( u32 u = 0; u < maskTex.fWidth( ); ++u )
			{
				u32* maskTexel = maskMip.fGetTexel<u32>( u, v );
				*maskTexel = Gfx::tTextureVRam::fPackColorR8G8B8A8( 255, 0, 0, 0 );
			}
		}

		idsTex.fUnlock( );
		maskTex.fUnlock( );
	}

	void tEditableTerrainEntity::fCommonCtor( )
	{
		mGroundCoverInitialized = false;

		mHeightFieldRoot.fReset( new tEntity( ) );
		mHeightFieldRoot->fSpawnImmediate( *this );

		// add editable properties; note that the values we use here are defaults; if the properties
		// are already present, these values will be ignored
		mEditableProperties.fInsert( 
			tEditablePropertyPtr( new tEditablePropertyVec2f( fStrTesselation( ), cDefaultTesselation, 32, 512, 32, 0 ) ) );
		mEditableProperties.fInsert( 
			tEditablePropertyPtr( new tEditablePropertyVec2f( fStrDimensions( ), cDefaultDimensions, 1.f, 999999.f, 0.5f, 1 ) ) );
		mEditableProperties.fInsert(
			tEditablePropertyPtr( new tEditablePropertyVec2i( fStrMaterialRes( ), cDefaultMaterialRes, 32, 4096, 32 ) ) );
		mEditableProperties.fInsert(
			tEditablePropertyPtr( new tEditablePropertyFloat( fStrOptimizationTarget( ), 0.0f, 0.f, 1.f, 0.01f, 2 ) ) );

		// construct terrain geometry; we retrieve them explicitly in case they were already present
		// and saved with different values than what we just tried to insert them with
		const Math::tVec2f tess = mEditableProperties.fGetValue( fStrTesselation( ),cDefaultTesselation );
		const Math::tVec2f dims = mEditableProperties.fGetValue( fStrDimensions( ), cDefaultDimensions );
		const Math::tVec2i mtlRes = mEditableProperties.fGetValue( fStrMaterialRes( ), cDefaultMaterialRes );

		fNewTerrainGeometry( dims, Math::tVec2i( ( u32 )tess.x, ( u32 )tess.y ), mtlRes );
	}

	void tEditableTerrainEntity::fNewTerrainGeometry( const Math::tVec2f& dims, const Math::tVec2i& tess, const Math::tVec2i& mtlRes )
	{
		// create new terrain geometry object
		mTerrainGeometry.fReset( new tTerrainGeometry( 
			mContainer.fGetResourceDepot( ), 
			mContainer.fGetDevice( ),
			dims,
			tess,
			mtlRes,
			mContainer.fSharedHeightFieldDiffuseMap( ),
			mContainer.fSharedHeightFieldNormalMap( ) ) );

		fRefreshRenderables( );
	}

	void tEditableTerrainEntity::fRefreshRenderables( )
	{
		mHeightFieldRoot->fClearChildren( );
		mTerrainGeometry->fCollectEntities( *mHeightFieldRoot, tEntityCreationFlags( ) );
		fUpdateChunkBounds( 0, mTerrainGeometry->fRez( ).fNumChunksX( ) - 1, 0, mTerrainGeometry->fRez( ).fNumChunksZ( ) - 1 );
		fUpdateBounds( );
		fUpdateStateTint( );
		fUpdateShadowCasting( );
	}

	void tEditableTerrainEntity::fUpdateChunkBounds( u32 iChunkXMin, u32 iChunkXMax, u32 iChunkZMin, u32 iChunkZMax )
	{
		// update chunks' bounds and move in scene graph if necessary
		for( u32 iChunkZ = iChunkZMin; iChunkZ <= iChunkZMax; ++iChunkZ )
		{
			for( u32 iChunkX = iChunkXMin; iChunkX <= iChunkXMax; ++iChunkX )
			{
				const u32 ichunk = mTerrainGeometry->fRez( ).fChunkIndex( iChunkX, iChunkZ );
				mHeightFieldRoot->fChild( ichunk )->fDynamicCast< tSpatialEntity >( )->fSetObjectSpaceBox( mTerrainGeometry->fComputeChunkBounds( iChunkX, iChunkZ ) );
			}
		}

	}

	void tEditableTerrainEntity::fUpdateBounds( )
	{
		const Math::tAabbf bounds = mTerrainGeometry->fComputeBounds( );
		fSetLocalSpaceMinMax( bounds.mMin, bounds.mMax );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fUpdateGroundCoverHeights( u32 id )
	{
		tTerrainGeometry::tEditableGroundCoverHeights editor;
		mTerrainGeometry->fBeginEditingGroundCoverHeights( id, editor );

		fUpdateGroundCoverHeights( editor );

		mTerrainGeometry->fEndEditingGroundCoverHeights( editor );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fUpdateGroundCoverHeights( tEditableGroundCoverHeights & editor )
	{
		const tEntityPtr & ptr = mGroundCoverEntities[ editor.fId( ) ];
		tEditableGroundCoverCloud * entity = ptr->fDynamicCast< tEditableGroundCoverCloud >( );
		sigassert( entity );

		// No objects means no spawns
		if( !entity->fDef( ).fElementCount( ) )
		{
			entity->fSetNeedsHeights( true );
			return;
		}

		const Math::tMat3f & world = ptr->fObjectToWorld( );
		const Math::tMat3f & worldInv = ptr->fWorldToObject( );
		
		Math::tRayf ray( Math::tVec3f::cZeroVector, -2000.f * Math::tVec3f::cYAxis );

		const u32 dimx = editor.fDimX( );
		const u32 dimz = editor.fDimZ( );
		const f32 minx = entity->fXPos( editor.fStartX( ) );
		const f32 minz = entity->fZPos( editor.fStartZ( ) );
		const f32 unitSize = entity->fUnitSize( );

		tHeightFieldMeshEntity * heightField = 
			mHeightFieldRoot->fFirstDescendentOfType<tHeightFieldMeshEntity>( );
		sigassert( heightField );

		tGrowableArray< Gfx::tGroundCoverCloud::tSpawnInfo > spawns;

		f32 zP = minz;
		for( u32 z = 0; z < dimz; ++z, zP += unitSize )
		{
			f32 xP = minx;
			for( u32 x = 0; x < dimx; ++x, xP += unitSize )
			{
				spawns.fSetCount( 0 );
				Gfx::tGroundCoverCloud::fSpawnCell( 
					&entity->fDef( ), 
					x + editor.fStartX( ), 
					z + editor.fStartZ( ), 
					xP, zP, 
					spawns, 
					true );

				const u32 spawnCount = spawns.fCount( );
				sigassert( spawnCount == editor.fPerItem( ) );
				for( u32 s = 0; s < spawnCount; ++s )
				{
					const Gfx::tGroundCoverCloud::tSpawnInfo & info = spawns[ s ];

					ray.mOrigin = world.fXformPoint( 
						Math::tVec3f( 
							info.mLocalXform.mRow0.w, 
							1000.f, 
							info.mLocalXform.mRow2.w ) 
					);

					Math::tRayCastHit hit;
					heightField->fRayCast( ray, hit );

					if( hit.fHit( ) )
						editor.fIndex( x, z, s ) = worldInv.fXformPoint( ray.fEvaluate( hit.mT ) ).y;
					else
						editor.fIndex( x, z, s ) = 0;
				}
			}
		}
	}

	Sigml::tObjectPtr tEditableTerrainEntity::fSerialize( b32 clone ) const
	{
		Sigml::tHeightFieldMeshObject* o = new Sigml::tHeightFieldMeshObject( );
		fSerializeBaseObject( o, clone );
		mTerrainGeometry->fSaveHeightField( o->mHeightField );
		mTerrainGeometry->fSaveGroundCover( o->mGroundCover );

		o->mMaterialMaskTexture.fCopyCpu( o->mMaterialMaskTexture.fGetReferenceDevice( ), mTerrainGeometry->fGetMaskTexture( ) );
		o->mMaterialIdsTexture.fCopyCpu( o->mMaterialIdsTexture.fGetReferenceDevice( ), mTerrainGeometry->fGetMtlIdsTexture( ) );

		return Sigml::tObjectPtr( o );
	}

	void tEditableTerrainEntity::fNotifyPropertyChanged( tEditableProperty& property )
	{
		tEditableObject::fNotifyPropertyChanged( property );

		const Math::tVec2f tess = mEditableProperties.fGetValue( fStrTesselation( ), cDefaultTesselation );
		const Math::tVec2f dims = mEditableProperties.fGetValue( fStrDimensions( ), cDefaultDimensions );
		const Math::tVec2i mtlRes = mEditableProperties.fGetValue( fStrMaterialRes( ), cDefaultMaterialRes );

		if( property.fGetName( ) == fStrTesselation( ) ||
			property.fGetName( ) == fStrDimensions( ) )
		{
			// either the terrain dimensions or tesselation has changed

			tStrongPtr<tTerrainGeometry> oldGeometry = mTerrainGeometry;
			const f32 oldMinX = oldGeometry->fRez( ).fMinX( );
			const f32 oldMinZ = oldGeometry->fRez( ).fMinZ( );
			const f32 oldMaxX = oldGeometry->fRez( ).fMaxX( );
			const f32 oldMaxZ = oldGeometry->fRez( ).fMaxZ( );

			fNewTerrainGeometry( dims, Math::tVec2i( ( u32 )tess.x, ( u32 )tess.y ), mtlRes );

			// Save the ground cover
			tDynamicArray<tGroundCover> groundCover;
			oldGeometry->fSaveGroundCover( groundCover );
			mTerrainGeometry->fRestoreGroundCover( groundCover );

			// Now refresh for the new heights
			const u32 gcCount = groundCover.fCount( );
			for( u32 gc = 0; gc < gcCount; ++gc )
			{
				tGroundCover & cover = groundCover[ gc ];
				b32 diff = mTerrainGeometry->fUpdateGroundCoverDims(
					cover.mCoverId, cover.mUnitSize, cover.mMaxUnitSpawns );

				tEditableGroundCoverCloud * entity = 
					mGroundCoverEntities[ cover.mCoverId ]->fDynamicCast<tEditableGroundCoverCloud>( );

				entity->fSetDimensions( dims.x, dims.y );
				
				// Something changed so we need the new ground cover
				if( diff )
				{
					mTerrainGeometry->fSaveGroundCover( cover );
					entity->fUpdateDef( cover );
				}
			}
			
			tTerrainGeometry::tEditableVertices editableVerts;
			mTerrainGeometry->fBeginEditingVerts( 
				editableVerts, 
				mTerrainGeometry->fRez( ).fMinX( ),
				mTerrainGeometry->fRez( ).fMinZ( ),
				mTerrainGeometry->fRez( ).fMaxX( ),
				mTerrainGeometry->fRez( ).fMaxZ( ) );

			for( u32 z = 0; z < editableVerts.fDimZ( ); ++z )
			{
				const f32 normz = z / ( editableVerts.fDimZ( ) - 1.f );
				for( u32 x = 0; x < editableVerts.fDimX( ); ++x )
				{
					const f32 normx = x / ( editableVerts.fDimX( ) - 1.f );
					const f32 h = oldGeometry->fSampleHeight( Math::fLerp( oldMinX, oldMaxX, normx ), Math::fLerp( oldMinZ, oldMaxZ, normz ) );
					editableVerts.fIndex( x, z ).mLocalHeight = h;
				}
			}

			fEndEditingVerts( editableVerts );

			mTerrainGeometry->fGetMaskTexture( ).fCopyCpu( oldGeometry->fGetMaskTexture( ) );
			mTerrainGeometry->fGetMtlIdsTexture( ).fCopyCpu( oldGeometry->fGetMtlIdsTexture( ) );

			mContainer.fGetActionStack( ).fAddAction( 
				tEditorActionPtr( new tChangeTerrainResolutionAction( fToSmartPtr( ), oldGeometry, mTerrainGeometry ) ) );
		}
		else if( property.fGetName( ) == fStrMaterialRes( ) )
		{
			tStrongPtr<tTerrainGeometry> oldGeometry = mTerrainGeometry;
			tTerrainGeometry::tHeightField heightField;
			oldGeometry->fSaveHeightField( heightField );

			fNewTerrainGeometry( dims, Math::tVec2i( ( u32 )tess.x, ( u32 )tess.y ), mtlRes );

			mTerrainGeometry->fGetMaskTexture( ).fFilteredCopyA8R8G8B8( oldGeometry->fGetMaskTexture( ) );
			mTerrainGeometry->fGetMtlIdsTexture( ).fUnfilteredCopyR5G6B5( oldGeometry->fGetMtlIdsTexture( ) );
			mTerrainGeometry->fRestoreHeightField( heightField );

			mContainer.fGetActionStack( ).fAddAction( 
				tEditorActionPtr( new tChangeTerrainResolutionAction( fToSmartPtr( ), oldGeometry, mTerrainGeometry ) ) );
		}
	}

	const tHeightFieldRez& tEditableTerrainEntity::fRez( ) const
	{
		return mTerrainGeometry->fRez( );
	}

	void tEditableTerrainEntity::fBeginEditingVerts( tEditableVertices& editableVerts, const Math::tVec3f& centerInLocal, f32 xlen, f32 zlen )
	{
		const f32 minLocalX = centerInLocal.x - 0.5f * xlen;
		const f32 minLocalZ = centerInLocal.z - 0.5f * zlen;
		const f32 maxLocalX = centerInLocal.x + 0.5f * xlen;
		const f32 maxLocalZ = centerInLocal.z + 0.5f * zlen;
		mTerrainGeometry->fBeginEditingVerts( editableVerts, minLocalX, minLocalZ, maxLocalX, maxLocalZ );
	}

	void tEditableTerrainEntity::fEndEditingVerts( const tEditableVertices& editableVerts )
	{
		mTerrainGeometry->fEndEditingVerts( editableVerts );

		const tEditableVertex& tl = editableVerts.fIndex( 0, 0 );
		const tEditableVertex& tr = editableVerts.fIndex( editableVerts.fDimX( ) - 1, 0 );
		const tEditableVertex& bl = editableVerts.fIndex( 0, editableVerts.fDimZ( ) - 1 );

		// only update bounding boxes of chunks containing the edited verts
		const u32 iChunkXMin = tl.fLogicalX( ) / mTerrainGeometry->fRez( ).fChunkQuadResX( );
		const u32 iChunkXMax = fMin( mTerrainGeometry->fRez( ).fNumLogicalVertsX( ) - 2, tr.fLogicalX( ) ) / mTerrainGeometry->fRez( ).fChunkQuadResX( );
		const u32 iChunkZMin = tl.fLogicalZ( ) / mTerrainGeometry->fRez( ).fChunkQuadResZ( );
		const u32 iChunkZMax = fMin( mTerrainGeometry->fRez( ).fNumLogicalVertsZ( ) - 2, bl.fLogicalZ( ) ) / mTerrainGeometry->fRez( ).fChunkQuadResZ( );

		fUpdateChunkBounds( iChunkXMin, iChunkXMax, iChunkZMin, iChunkZMax );
		fUpdateBounds( );

		// Update ground cover
		{
			tHashTable<u32, tEntityPtr>::tIterator itr = mGroundCoverEntities.fBegin( );
			tHashTable<u32, tEntityPtr>::tIterator itrEnd = mGroundCoverEntities.fEnd( );
			for( ; itr != itrEnd; ++itr )
			{
				if( itr->fNullOrRemoved( ) )
					continue;

				tEditableGroundCoverHeights editor;
				mTerrainGeometry->fBeginEditingGroundCoverHeights( 
					itr->mKey,
					editor, 
					editableVerts.fMinX( ), 
					editableVerts.fMinZ( ), 
					editableVerts.fMaxX( ), 
					editableVerts.fMaxZ( ) );

				if( !editor.fDimX( ) || !editor.fDimZ( ) )
				{
					mTerrainGeometry->fEndEditingGroundCoverHeights( editor );
					continue;
				}

				fUpdateGroundCoverHeights( editor );
				mTerrainGeometry->fEndEditingGroundCoverHeights( editor );


				tEditableGroundCoverCloud * entity = itr->mValue->fDynamicCast< tEditableGroundCoverCloud >( );
				sigassert( entity );

				entity->fUpdateHeights( 
					editor.fStartX( ), 
					editor.fStartZ( ), 
					editor.fStartX( ) + editor.fDimX( ), 
					editor.fStartZ( ) + editor.fDimZ( ), 
					&editor.fIndex( 0, 0 ) );
			}

		}
	}

	f32	tEditableTerrainEntity::fSampleHeight( f32 localX, f32 localZ ) const
	{
		return mTerrainGeometry->fSampleHeight( localX, localZ );
	}

	void tEditableTerrainEntity::fSaveHeightField( tHeightField& heightField ) const
	{
		mTerrainGeometry->fSaveHeightField( heightField );
	}

	void tEditableTerrainEntity::fRestoreHeightField( const tHeightField& heightField )
	{
		mTerrainGeometry->fRestoreHeightField( heightField );
		fUpdateChunkBounds( 0, mTerrainGeometry->fRez( ).fNumChunksX( ) - 1, 0, mTerrainGeometry->fRez( ).fNumChunksZ( ) - 1 );
		fUpdateBounds( );
	}

	void tEditableTerrainEntity::fRestoreHeightField( const tHeightField& heightField, u32 minx, u32 minz, u32 maxx, u32 maxz )
	{
		mTerrainGeometry->fRestoreHeightField( heightField, minx, minz, maxx, maxz );
		// TODO optimize sub-rect of chunks to update using minx, minz, maxx, maxz
		fUpdateChunkBounds( 0, mTerrainGeometry->fRez( ).fNumChunksX( ) - 1, 0, mTerrainGeometry->fRez( ).fNumChunksZ( ) - 1 );
		fUpdateBounds( );
	}

	void tEditableTerrainEntity::fResetGeometry( const tStrongPtr<tTerrainGeometry>& oldGeometry )
	{
		const Math::tVec2f tess = Math::tVec2f( 
			oldGeometry->fRez( ).fNumLogicalVertsX( ) - 1,			
			oldGeometry->fRez( ).fNumLogicalVertsZ( ) - 1 );
		const Math::tVec2f dims = Math::tVec2f( 
			oldGeometry->fRez( ).fWorldLengthX( ),
			oldGeometry->fRez( ).fWorldLengthZ( ) );
		const Math::tVec2i mtlRes = Math::tVec2i(
			oldGeometry->fGetMaskTexture( ).fWidth( ),
			oldGeometry->fGetMaskTexture( ).fHeight( ) );

		mEditableProperties.fSetDataNoNotify( fStrTesselation( ), tess );
		mEditableProperties.fSetDataNoNotify( fStrDimensions( ), dims );
		mEditableProperties.fSetDataNoNotify( fStrMaterialRes( ), mtlRes );
		mEditableProperties.fSetDataNoNotify( fStrOptimizationTarget( ), 0.f );

		mTerrainGeometry = oldGeometry;

		// Update ground cover
		tDynamicArray<tGroundCover> groundCover;
		mTerrainGeometry->fSaveGroundCover( groundCover );

		const u32 gcCount = groundCover.fCount( );
		for( u32 gc = 0; gc < gcCount; ++gc )
		{
			const tGroundCover & cover = groundCover[ gc ];
			tEditableGroundCoverCloud * cloud = 
				mGroundCoverEntities[ cover.mCoverId ]->fDynamicCast<tEditableGroundCoverCloud>( );

			cloud->fSetDimensions( fRez( ).fWorldLengthX( ), fRez( ).fWorldLengthZ( ) );
			cloud->fUpdateDef( cover );
		}

		fUpdateDynamicTextureReferences( );
	}

	void tEditableTerrainEntity::fSaveHeightFieldToBitmap( const tFilePathPtr& path ) const
	{
		// quick n dirty baby, thanks to d3dx

		tHeightField heightField;
		mTerrainGeometry->fSaveHeightField( heightField );

		Math::tVec2f tess = mEditableProperties.fGetValue( fStrTesselation( ), cDefaultTesselation );
		tess += Math::tVec2f( 1.f, 1.f );

		const tFilePathPtr heightMapPath = path;
		const Gfx::tDevicePtr device = Gfx::tDevice::fGetDefaultDevice( );

		IDirect3DTexture9* heightMapTexture = 0;
		device->fGetDevice( )->CreateTexture( ( u32 )tess.x, ( u32 )tess.y, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SCRATCH, &heightMapTexture, 0 );
		sigassert( heightMapTexture );

		D3DLOCKED_RECT rect;
		heightMapTexture->LockRect( 0, &rect, 0, 0 );

		for( u32 y = 0; y < ( u32 )tess.y; ++y )
		{
			for( u32 x = 0; x < ( u32 )tess.x; ++x )
			{
				Sig::byte* texel = ( Sig::byte* )rect.pBits + y * rect.Pitch + x * 4;

				const u32 index = y * ( u32 )tess.x + x;
				if( heightField.fHasQuadData( ) && heightField.mQuadsDisabled[ index ] )
				{
					texel[0] = 0;
					texel[1] = 0;
					texel[2] = 0;
					texel[3] = 0xff;
				}
				else
				{
					const f32 h = heightField.mHeights[ index ];
					const f32 hn = ( h - fObjectSpaceBox( ).mMin.y ) / ( fObjectSpaceBox( ).mMax.y - fObjectSpaceBox( ).mMin.y );
					const u8  hi = ( u8 )( 255.f * fClamp( hn, 0.f, 1.f ) );
					texel[0] = hi;
					texel[1] = hi;
					texel[2] = hi;
					texel[3] = 0xff;
				}
			}
		}

		heightMapTexture->UnlockRect( 0 );
		D3DXSaveTextureToFile( heightMapPath.fCStr( ), D3DXIFF_BMP, heightMapTexture, 0 );
		heightMapTexture->Release( );
	}

	tTextureSysRam::tSurface& tEditableTerrainEntity::fGetMaskTexture( )
	{
		return mTerrainGeometry->fGetMaskTexture( );
	}

	tTextureSysRam::tSurface& tEditableTerrainEntity::fGetMtlIdsTexture( )
	{
		return mTerrainGeometry->fGetMtlIdsTexture( );
	}

	void tEditableTerrainEntity::fUpdateDynamicTextureReferences( )
	{
		mTerrainGeometry->fUpdateDynamicTextureReferences(
			mContainer.fSharedHeightFieldDiffuseMap( ),
			mContainer.fSharedHeightFieldNormalMap( ) );
		fRefreshRenderables( );
	}

	void tEditableTerrainEntity::fUpdateMaterialTilingFactors( const tDynamicArray<f32>& tilingFactors )
	{
		mTerrainGeometry->fUpdateMaterialTilingFactors( tilingFactors );
	}

	//------------------------------------------------------------------------------
	b32 tEditableTerrainEntity::fInitializeGroundCover( )
	{
		return !mGroundCoverInitialized;
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fSetGroundCoverInitialized( )
	{
		mGroundCoverInitialized = true;
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fRestoreGroundCover( const Sigml::tGroundCoverLayer & layer )
	{
		b32 gcChanged = mTerrainGeometry->fRestoreGroundCover( 
			layer.fUniqueId( ), layer.fUnitSize( ), layer.fComputeMaxUnitSpawns( ) );

		tEntityPtr * ptr = mGroundCoverEntities.fFind( layer.fUniqueId( ) );
		if( !ptr )
		{
			tEditableGroundCoverCloud * entity = new tEditableGroundCoverCloud( 
				mContainer, fRez( ).fWorldLengthX( ), fRez( ).fWorldLengthZ( ) );
			entity->fSpawnImmediate( *this );

			ptr = mGroundCoverEntities.fInsert( layer.fUniqueId( ), tEntityPtr( entity ) );
		}

		tEditableGroundCoverCloud * entity = (*ptr)->fDynamicCast< tEditableGroundCoverCloud >( );
		sigassert( entity );

		entity->fUpdateDef( layer, !gcChanged );
		entity->fSetNeedsHeights( false );

		// Handle old terrain with miscalculated dims
		if( gcChanged )
		{
			tGroundCover cover( layer.fUniqueId( ) );

			// Give it the zero heights and right dims
			mTerrainGeometry->fSaveGroundCover( cover );
			entity->fUpdateDef( cover, false );

			// Update the heights
			fUpdateGroundCoverHeights( layer.fUniqueId( ) );

			// Give it the real heights
			mTerrainGeometry->fSaveGroundCover( cover );
			entity->fUpdateDef( cover );
		}
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fAddGroundCover( const Sigml::tGroundCoverLayer & layer )
	{
		if( mGroundCoverEntities.fFind( layer.fUniqueId( ) ) )
		{
			fRestoreGroundCover( layer );
			return;
		}

		mTerrainGeometry->fAddGroundCover( 
			layer.fUniqueId( ), layer.fUnitSize( ), layer.fComputeMaxUnitSpawns( ) );

		tEditableGroundCoverCloud * entity = new tEditableGroundCoverCloud( 
			mContainer, fRez( ).fWorldLengthX( ), fRez( ).fWorldLengthZ( ) );
		entity->fSpawnImmediate( *this );

		mGroundCoverEntities.fInsert( layer.fUniqueId( ), tEntityPtr( entity ) );

		tTerrainGeometry::tGroundCover cover( layer.fUniqueId( ) );
		mTerrainGeometry->fSaveGroundCover( cover );
		sigassert( cover.mCoverId == layer.fUniqueId( ) );

		entity->fUpdateDef( layer, false );
		entity->fUpdateDef( cover );

		if( layer.fElementCount( ) )
			fUpdateGroundCoverHeights( layer.fUniqueId( ) );
		else
			entity->fSetNeedsHeights( true );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fRemoveGroundCover( u32 id )
	{
		mTerrainGeometry->fRemoveGroundCover( id );
		
		tEntityPtr * ptr = mGroundCoverEntities.fFind( id );
		if( !ptr )
			return;

		( *ptr )->fDeleteImmediate( );
		mGroundCoverEntities.fRemove( ptr );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fSaveGroundCover( tGroundCover & cover )
	{
		mTerrainGeometry->fSaveGroundCover( cover );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fRestoreGroundCover( const tGroundCover & cover )
	{
		mTerrainGeometry->fRestoreGroundCover( cover );

		tEntityPtr * ptr = mGroundCoverEntities.fFind( cover.mCoverId );
		if( !ptr )
			return;

		tEditableGroundCoverCloud * entity = ( *ptr )->fDynamicCast< tEditableGroundCoverCloud >( );
		sigassert( entity );

		entity->fUpdateDef( cover );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fBeginEditingGroundCoverMask( 
		const Sigml::tGroundCoverLayer & layer, tEditableGroundCoverMask & editableMask,
		const Math::tVec3f& centerInLocal, f32 xlen, f32 zlen )
	{
		const f32 minLocalX = centerInLocal.x - 0.5f * xlen;
		const f32 minLocalZ = centerInLocal.z - 0.5f * zlen;
		const f32 maxLocalX = centerInLocal.x + 0.5f * xlen;
		const f32 maxLocalZ = centerInLocal.z + 0.5f * zlen;

		mTerrainGeometry->fBeginEditingGroundCoverMask(
			layer.fUniqueId( ), editableMask, minLocalX, minLocalZ, maxLocalX, maxLocalZ );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fEndEditingGroundCoverMask( 
		const tEditableGroundCoverMask & editableMask )
	{
		mTerrainGeometry->fEndEditingGroundCoverMask( editableMask );

		tEntityPtr * ptr = mGroundCoverEntities.fFind( editableMask.fId( ) );
		if( !ptr )
			return;

		tEditableGroundCoverCloud * entity = ( *ptr )->fDynamicCast< tEditableGroundCoverCloud >( );
		sigassert( entity );

		const u32 minX = editableMask.fStartX( );
		const u32 minZ = editableMask.fStartZ( );
		const u32 maxX = minX + editableMask.fDimX( );
		const u32 maxZ = minZ + editableMask.fDimZ( );

		tDynamicArray<f32> mask;
		mTerrainGeometry->fQueryGroundCoverMask( editableMask.fId( ), minX, minZ, maxX, maxZ, mask ); 

		entity->fUpdateMask( minX, minZ, maxX, maxZ, mask.fBegin( ) );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fUpdateGroundCover( const Sigml::tGroundCoverLayer & layer )
	{
		b32 gcChanged = mTerrainGeometry->fUpdateGroundCoverDims( 
			layer.fUniqueId( ), layer.fUnitSize( ), layer.fComputeMaxUnitSpawns( ) );

		tEntityPtr * ptr = mGroundCoverEntities.fFind( layer.fUniqueId( ) );
		if( !ptr )
			return;

		tEditableGroundCoverCloud * entity = ( *ptr )->fDynamicCast< tEditableGroundCoverCloud >( );
		sigassert( entity );

		if( gcChanged )
		{
			tTerrainGeometry::tGroundCover cover( layer.fUniqueId( ) );
			mTerrainGeometry->fSaveGroundCover( cover );
			sigassert( cover.mCoverId != ~0 );

			entity->fUpdateDef( cover, false );
		}

		entity->fUpdateDef( layer, false );

		if( gcChanged || entity->fNeedsHeights( ) )
		{
			entity->fSetNeedsHeights( false );

			fUpdateGroundCoverHeights( layer.fUniqueId( ) );

			tDynamicArray<f32> heights;
			mTerrainGeometry->fQueryGroundCoverHeights( layer.fUniqueId( ), heights );
			entity->fUpdateHeights( heights.fCount( ), heights.fBegin( ), entity->fDirty( ) );
		}

		if( entity->fDirty( ) )
			entity->fUpdateCover( );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fUpdateGroundCoverVisiblity( const Sigml::tGroundCoverLayer & layer )
	{
		tEntityPtr * ptr = mGroundCoverEntities.fFind( layer.fUniqueId( ) );
		if( !ptr )
			return;

		tEditableGroundCoverCloud * entity = ( *ptr )->fDynamicCast< tEditableGroundCoverCloud >( );
		sigassert( entity );

		entity->fSetVisible( layer.fVisible( ) );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fUpdateGroundCoverShadows( const Sigml::tGroundCoverLayer & layer )
	{
		tEntityPtr * ptr = mGroundCoverEntities.fFind( layer.fUniqueId( ) );
		if( !ptr )
			return;

		tEditableGroundCoverCloud * entity = ( *ptr )->fDynamicCast< tEditableGroundCoverCloud >( );
		sigassert( entity );

		entity->fUpdateShadows( layer );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fUpdateGroundCoverFrequency( const Sigml::tGroundCoverLayer & layer )
	{
		tEntityPtr * ptr = mGroundCoverEntities.fFind( layer.fUniqueId( ) );
		if( !ptr )
			return;

		tEditableGroundCoverCloud * entity = ( *ptr )->fDynamicCast< tEditableGroundCoverCloud >( );
		sigassert( entity );

		entity->fUpdateFrequency( layer );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fUpdateGroundCoverSpawnCount( const Sigml::tGroundCoverLayer & layer )
	{
		tEntityPtr * ptr = mGroundCoverEntities.fFind( layer.fUniqueId( ) );
		if( !ptr )
			return;

		tEditableGroundCoverCloud * entity = ( *ptr )->fDynamicCast< tEditableGroundCoverCloud >( );
		sigassert( entity );

		//Update the element spawn counts
		entity->fUpdateSpawnCount( layer );

		// Update the cover
		fUpdateGroundCover( layer );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fRefreshGroundCoverHeights( const Sigml::tGroundCoverLayer & layer )
	{
		fUpdateGroundCoverHeights( layer.fUniqueId( ) );

		tEntityPtr * ptr = mGroundCoverEntities.fFind( layer.fUniqueId( ) );
		if( !ptr )
			return;

		tEditableGroundCoverCloud * entity = ( *ptr )->fDynamicCast< tEditableGroundCoverCloud >( );
		sigassert( entity );

		tDynamicArray< f32 > heights;
		mTerrainGeometry->fQueryGroundCoverHeights( layer.fUniqueId( ), heights );

		sigassert( heights.fCount( ) );

		entity->fUpdateHeights(heights.fCount( ), heights.fBegin( ) );
	}

	//------------------------------------------------------------------------------
	void tEditableTerrainEntity::fRestoreGroundCover( 
		const tDynamicArray<tTerrainGeometry::tGroundCover> & gcs )
	{
		mTerrainGeometry->fRestoreGroundCover( gcs );

		const u32 gcCount = gcs.fCount( );
		for( u32 gc = 0; gc < gcCount; ++gc )
		{
			const tTerrainGeometry::tGroundCover & cover = gcs[ gc ];
			
			tEntityPtr * ptr = mGroundCoverEntities.fFind( cover.mCoverId );
			if( !ptr )
			{
				tEditableGroundCoverCloud * entity = new tEditableGroundCoverCloud( 
					mContainer, fRez( ).fWorldLengthX( ), fRez( ).fWorldLengthZ( ) );
				entity->fSpawnImmediate( *this );

				ptr = mGroundCoverEntities.fInsert( cover.mCoverId, tEntityPtr( entity ) );
			}

			tEditableGroundCoverCloud * entity = (*ptr)->fDynamicCast< tEditableGroundCoverCloud >( );
			sigassert( entity );

			entity->fUpdateDef( cover, false );
		}
	}
}

