#include "ToolsPch.hpp"
#include "tHeightFieldMeshObject.hpp"
#include "tHeightFieldMeshEntity.hpp"
#include "tHeightFieldMaterialGen.hpp"
#include "tHeightFieldTransitionMaterialGen.hpp"
#include "tSigmlConverter.hpp"
#include "Tatml.hpp"
#include "Editor/tEditableTerrainEntity.hpp"

namespace Sig { namespace Sigml
{
	register_rtti_factory( tHeightFieldMeshObject, false );

	tHeightFieldMeshObject::tHeightFieldMeshObject( )
		: mHFMatType( tTerrainGeometry::cMaterialTypeDefault )
	{
	}

	void tHeightFieldMeshObject::fDumpRawTriangles( tGrowableArray< Math::tVec3f >& rawVerts, tGrowableArray< Math::tVec3u >& rawTris ) const
	{
		const Math::tVec2f tess = mEditableProperties.fGetValue( fStrTesselation( ), fDefaultTesselation( ) );
		const Math::tVec2f dims = mEditableProperties.fGetValue( fStrDimensions( ), fDefaultDimensions( ) );
		tHeightFieldGeometryConverter hfConverter( dims, Math::tVec2i( ( u32 )tess.x, ( u32 )tess.y ), mHeightField );
		tGrowableArray< tGrowableArray< Math::tVec3u > > dummy;
		hfConverter.fDumpRawTriangles( rawVerts, rawTris );
	}

	void tHeightFieldMeshObject::fSerialize( tXmlSerializer& s )	{ fSerializeXmlObject( s, *this ); }
	void tHeightFieldMeshObject::fSerialize( tXmlDeserializer& s )	{ fSerializeXmlObject( s, *this ); }

	namespace
	{
		u32 fCalculateOptimalCellsToConsume( const Gfx::tGroundCoverCloudDef& def )
		{
			//we probably want at least 100 instances before instanced rendering 
			// is worth it (just an educated guess, no formal testing done)

			f32 largestCell = 1.0f;
			for( u32 e = 0; e < def.fElementCount( ); ++e )
			{
				f32 cellSize = def.fElements( )[ e ].mSpawnCount * def.fElements( )[ e ].mFrequency;
				if( cellSize > largestCell )
					largestCell = cellSize;
			}

			while( Math::fSquare( largestCell ) < 100 )
				largestCell += 1.0f;

			return fRoundUp<u32>( largestCell );
		}
		void fBakeCellMats( Gfx::tGroundCoverCloudDef& def )
		{
			//determine optimal instance info
			def.mInstancedCellDiff = fCalculateOptimalCellsToConsume( def );
			def.mInstancedUnitSize = def.mUnitSize * def.mInstancedCellDiff;

			//allocate our cell array
			const u32 gridW = Gfx::tInstanceGrid::fDimX( &def );
			const u32 gridH = Gfx::tInstanceGrid::fDimZ( &def );
			def.mCells.fNewArray( def.fElementCount( ) );
			for( u32 e = 0; e < def.fElementCount( ); ++e )
				def.mCells[ e ].fNewArray( gridW * gridH );

			//Gather all spawns in the entire world
			const f32 unitSize = def.fUnitSize( );
			const u32 minX = 0;
			const u32 maxX = def.fDimX( );
			const u32 minZ = 0;
			const u32 maxZ = def.fDimZ( );
			f32 pZ = def.fZPos( minZ );
			for( u32 z = minZ; z < maxZ; ++z, pZ += unitSize  )
			{
				f32 pX = def.fXPos( minX );
				for( u32 x = minX; x < maxX; ++x, pX += unitSize )
				{
					tGrowableArray<Gfx::tGroundCoverCloud::tSpawnInfo> tempSpawnArray; //required because fGenSpawnInfo uses the size of this to calculate shit. this is really dumb but required to match how we've always done ground cover stuff
					Gfx::tGroundCoverCloud::fGenSpawnInfo( &def, x, z, pX, pZ, tempSpawnArray );

					for( u32 ti = 0; ti < tempSpawnArray.fCount( ); ++ti )
					{
						const Gfx::tGroundCoverCloud::tSpawnInfo& info = tempSpawnArray[ ti ];
						if( !info.mDoSpawn )
							continue;

						//find element idx
						u32 iElement = 0;
						for( ; iElement < def.fElementCount( ); ++iElement )
						{
							if( info.mElement == &def.fElements( )[ iElement ] )
								break;
						}

						//copy matrix
						const u32 cellIdx = Gfx::tInstanceGrid::fConvert( &def, x, z );
						def.mCells[ iElement ][ cellIdx ].fPushBack( info.mLocalXform );
					}
				}
			}
		}
	}//unnamed

	tEntityDef* tHeightFieldMeshObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		const Math::tVec2f tess = mEditableProperties.fGetValue( fStrTesselation( ), fDefaultTesselation( ) );
		const Math::tVec2f dims = mEditableProperties.fGetValue( fStrDimensions( ), fDefaultDimensions( ) );
		const f32 optimizeTarget = mEditableProperties.fGetValue( fStrOptimizationTarget( ), 0.f );
		const Math::tVec2i vertexRes( ( u32 )tess.x, ( u32 )tess.y );

		tHeightFieldMeshEntityDef* entityDef = new tHeightFieldMeshEntityDef( dims, vertexRes );
		fConvertEntityDefBase( entityDef, sigmlConverter );

		// compute mesh data and add to sigml converter
		tHeightFieldGeometryConverter hfConverter( dims, vertexRes, mHeightField );
		hfConverter.fConvertToEntityDef( entityDef, sigmlConverter, optimizeTarget );

		// handle material stuffs
		{
			const u32 uniqueTexId = sigmlConverter.fGetTextureObjectCount( );

			tStrongPtr< tSigmlConverter::tTextureData > maskTexData( new tSigmlConverter::tTextureData );
			tStrongPtr< tSigmlConverter::tTextureData > mtlIdsTexData( new tSigmlConverter::tTextureData );
			{
				std::stringstream ss;
				ss << "." << uniqueTexId + 0 << ".texb";
				maskTexData->mResourcePath = tFilePathPtr::fSwapExtension( sigmlConverter.fGetResourcePath( ), ss.str( ).c_str( ) );
				if( !mMaterialMaskTexture.fAllocated( ) )
					mMaterialMaskTexture.fAllocate( mMaterialMaskTexture.fGetReferenceDevice( ), 32, 32, Gfx::tTextureFile::cFormatA8R8G8B8 );
				maskTexData->mTextureObject.fFromSurface( mMaterialMaskTexture, Gfx::tTextureFile::cSemanticDiffuse, mMaterialMaskTexture.fFormat( ) );
			}
			{
				std::stringstream ss;
				ss << "." << uniqueTexId + 1 << ".texb";
				mtlIdsTexData->mResourcePath = tFilePathPtr::fSwapExtension( sigmlConverter.fGetResourcePath( ), ss.str( ).c_str( ) );
				if( !mMaterialIdsTexture.fAllocated( ) )
					mMaterialIdsTexture.fAllocate( mMaterialIdsTexture.fGetReferenceDevice( ), 32, 32, Gfx::tTextureFile::cFormatR5G6B5 );
				mtlIdsTexData->mTextureObject.fFromSurface( mMaterialIdsTexture, Gfx::tTextureFile::cSemanticLookUpTable, mMaterialIdsTexture.fFormat( ) );
			}

			sigmlConverter.fAddTextureObject( maskTexData );
			sigmlConverter.fAddTextureObject( mtlIdsTexData );


			
			tHeightFieldMaterialGenBase* mtlGen = NULL;
			switch( mHFMatType )
			{
			case tTerrainGeometry::cMaterialTypeDefault: mtlGen = NEW_TYPED( tHeightFieldMaterialGen )( ); break;
			case tTerrainGeometry::cMaterialTypeTransition: mtlGen = NEW_TYPED( tHeightFieldTransitionMaterialGen )( ); break;
			default: sig_nodefault( );
			};

			mtlGen->mWorldSpaceDims = dims;
			mtlGen->mMaskTextureName = maskTexData->mResourcePath;
			mtlGen->mMtlIdsTextureName = mtlIdsTexData->mResourcePath;

			const tFilePathPtr tatmldPath = sigmlConverter.fSigmlFile( ).mDiffuseMapAtlas;
			const tFilePathPtr tatmlnPath = sigmlConverter.fSigmlFile( ).mNormalMapAtlas;
			mtlGen->mDiffuseTextureName = Tatml::fTatmlPathToTatb( tatmldPath );
			if( tatmlnPath.fLength( ) > 0 )
				mtlGen->mNormalMapTextureName = Tatml::fTatmlPathToTatb( tatmlnPath );

			// set defaults, in case we can't load tatml
			mtlGen->mSubDiffuseRectDims = Math::tVec2f( 512.f, 512.f );
			mtlGen->mSubNormalRectDims = Math::tVec2f( 512.f, 512.f );
			mtlGen->mDiffuseCount_NormalCount = Math::tVec4f( 1.f );

			// set material tiling factors
			const tDynamicArray<f32>& sigmlTileFactors = sigmlConverter.fSigmlFile( ).mHeightFieldMaterialTileFactors;
			for( u32 i = 0; i < sigmlTileFactors.fCount( ); i += 4 )
			{
				for( u32 j = 0; j < 4; ++j )
				{
					const u32 absoluteIndex = i + j;
					if( absoluteIndex < sigmlTileFactors.fCount( ) )
						mtlGen->mTileFactors[ i / 4 ].fAxis( j ) = sigmlTileFactors[ absoluteIndex ];
				}
			}

			Tatml::tFile tatmld;
			if( tatmldPath.fLength( ) > 0 && tatmld.fLoadXml( ToolsPaths::fMakeResAbsolute( tatmldPath ) ) )
			{
				mtlGen->mSubDiffuseRectDims = Math::tVec2f( tatmld.mSubWidth, tatmld.mSubHeight );

				u32 xCount = 1, yCount = 1;
				tatmld.fDetermineNumTexturesXandY( xCount, yCount );
				mtlGen->mDiffuseCount_NormalCount.x = (f32)xCount;
				mtlGen->mDiffuseCount_NormalCount.y = (f32)yCount;
			}

			Tatml::tFile tatmln;
			if( tatmlnPath.fLength( ) > 0 && tatmln.fLoadXml( ToolsPaths::fMakeResAbsolute( tatmlnPath ) ) )
			{
				mtlGen->mSubNormalRectDims = Math::tVec2f( tatmln.mSubWidth, tatmln.mSubHeight);

				u32 xCount = 1, yCount = 1;
				tatmln.fDetermineNumTexturesXandY( xCount, yCount );
				mtlGen->mDiffuseCount_NormalCount.z = (f32)xCount;
				mtlGen->mDiffuseCount_NormalCount.w = (f32)yCount;
			}

			entityDef->mMaterial = mtlGen->fCreateHeightFieldGfxMaterial( sigmlConverter, false, true );
		}

		// handle ground cover stuffs
		{
			const u32 gcCount = mGroundCover.fCount( );
			entityDef->mGroundCoverDefs.fNewArray( gcCount );

			for( u32 gc = 0; gc < gcCount; ++gc )
			{
				Gfx::tGroundCoverCloudDef & def = entityDef->mGroundCoverDefs[ gc ];
				const tTerrainGeometry::tGroundCover & cover = mGroundCover[ gc ];
				const Sigml::tGroundCoverLayer * layer = 
					sigmlConverter.fSigmlFile( ).fFindGroundCover( cover.mCoverId );

				sigassert( layer && "Sigml file doesn't contain specified ground cover layer" );

				def.mUnitSize = layer->fUnitSize( );
				def.mPaintUnits = layer->fPaintUnits( );
				def.mDimX = cover.mDimX;
				def.mDimZ = cover.mDimZ;
				def.mWorldLengthX = dims.x;
				def.mWorldLengthZ = dims.y;
				def.mRotation = ( Gfx::tGroundCoverCloudDef::tRotation )layer->fRotation( );
				def.mTranslation = ( Gfx::tGroundCoverCloudDef::tTranslation )layer->fTranslation( );
				def.mYRotationScale = layer->fYRotationScale( );
				def.mXZRotationScale = layer->fXZRotationScale( );
				def.mXZTranslationScale = layer->fXZTranslationScale( );
				def.mYTranslationScale = layer->fYTranslationScale( );
				def.mScaleRangeAdjustor = layer->fScaleRangeAdjustor( );
				def.mFarVisibility = ( Gfx::tGroundCoverCloudDef::tVisibility )layer->fFarVisibility( );
				def.mNearVisibility = ( Gfx::tGroundCoverCloudDef::tVisibility )layer->fNearVisibility( );
				def.mMaxUnitSpawns = layer->fComputeMaxUnitSpawns( );

				Gfx::tGroundCoverCloudDef::fConvertMask( 
					cover.mDimX, cover.mDimZ, cover.mMask.fBegin( ), 
					def.mPaintUnits, 
					def.mMaskDimX, def.mMaskDimZ, def.mMask );

				sigassert( cover.mHeights.fCount( ) == ( cover.mDimX * cover.mDimZ ) * def.mMaxUnitSpawns );
				def.mHeights.fInitialize( cover.mHeights.fBegin( ), cover.mHeights.fCount( ) );

				const u32 elementCount = layer->fElementCount( );
				const Sigml::tGroundCoverLayer::tElement * layerElements = layer->fElements( );
				def.mElements.fNewArray( elementCount );
				for( u32 e = 0; e < elementCount; ++e )
				{
					const Sigml::tGroundCoverLayer::tElement & from = layerElements[ e ];
					Gfx::tGroundCoverCloudDef::tElement & to = def.mElements[ e ];
	
					to.mCastsShadow = from.mCastsShadow;
					to.mFrequency = from.mFrequency;
					to.mSpawnCount = from.mSpawnCount;
					to.mSgFile = sigmlConverter.fAddLoadInPlaceResourcePtr(
						tResourceId::fMake<tSceneGraphFile>( from.mSgPath ) );
				}

				fBakeCellMats( def );
			}
		}

		return entityDef;
	}

	tEditableObject* tHeightFieldMeshObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableTerrainEntity( container, *this );
	}
}}
