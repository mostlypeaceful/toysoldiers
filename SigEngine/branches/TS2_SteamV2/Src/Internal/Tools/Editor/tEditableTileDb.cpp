//------------------------------------------------------------------------------
// \file tEditableTileDb.cpp - 28 Oct 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tEditableTileDb.hpp"
#include "Gfx\tTextureFile.hpp"
#include "FileSystem.hpp"
#include "Guid.hpp"

namespace Sig
{
	const char* gFloorID	= "floor_";
	const char* gCornerID	= "corner_";
	const char* gWallID		= "wall_";
	const char* gNicheID	= "niche_";
	const char* gUniquesID	= "unique_";
	const char* gAutoID		= "_auto_";

	//------------------------------------------------------------------------------
	// tEditableTileData
	//------------------------------------------------------------------------------
	tEditableTileDef::~tEditableTileDef( )
	{
		fUnloadResources( );
	}

	void tEditableTileDef::fLoadAndSetTexture( const tFilePathPtr& filePath, const tResourceDepotPtr& resourceDepot )
	{
		mTileAssetPath = filePath;

		if( !resourceDepot.fNull( ) )
			mTileResource = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( mTileAssetPath ), this );
	}

	void tEditableTileDef::fLoadAndSetModel( const tFilePathPtr& modelPath, const tFilePathPtr& filePath, const tResourceDepotPtr& resourceDepot )
	{
		mModelAssetPath = modelPath;

		if( !resourceDepot.fNull( ) )
		{
			mModelResource = resourceDepot->fQueryLoadBlock( tResourceId::fMake< tSceneGraphFile >( filePath ), this );
			mModelEntity = tSgFileRefEntityPtr( new tSgFileRefEntity( mModelResource ) );
		}
	}

	void tEditableTileDef::fUnloadResources( )
	{
		fUnloadResource( mTileResource );
		fUnloadResource( mModelResource );
	}

	void tEditableTileDef::fUnloadResource( tResourcePtr& resource )
	{
		if( !resource.fNull( ) )
			resource->fUnload( this );
	}

	//------------------------------------------------------------------------------
	// tEditableTileTypeList
	//------------------------------------------------------------------------------
	tEditableTileTypeList::tEditableTileTypeList( tTileTypes type, std::string name )
		: mTypeEnum( type )
		, mTypeName( name )
	{
	}

	tEditableTileTypeList::~tEditableTileTypeList( )
	{
		if( !mDefaultTileResource.fNull( ) )
			mDefaultTileResource->fUnload( this );

		for( u32 i = 0; i < mTileDefs.fCount( ); ++i )
			delete mTileDefs[ i ];
	}

	void tEditableTileTypeList::fLoadDefaultTile( const tResourceDepotPtr& resourceDepot, const tFilePathPtr& defaultTilePath )
	{
		mDefaultTileAssetPath = defaultTilePath;

		if( !resourceDepot.fNull( ) )
			mDefaultTileResource = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( mDefaultTileAssetPath ), this );
	}

	void tEditableTileTypeList::fEraseTileDef( u32 idx )
	{
		delete mTileDefs[idx];
		mTileDefs.fEraseOrdered( idx );
	}

	b32 tEditableTileTypeList::fHasRandomTiles( ) const
	{
		for( u32 i = 0; i < mTileDefs.fCount( ); ++i )
		{
			if( mTileDefs[i]->fIsRandomable( ) )
				return true;
		}

		return false;
	}

	const tEditableTileDef* tEditableTileTypeList::fGetRandomTileDef( ) const
	{
		if( mTileDefs.fCount( ) == 0 )
			return NULL;

		f32 totalWeight = 0.f;
		for( u32 i = 0; i < mTileDefs.fCount( ); ++i )
			totalWeight += mTileDefs[i]->mRandomWeight;

		if( totalWeight == 0.f )
		{
			log_warning( 0, "Tried to randomize a tile but there were no random tiles available." );
			return mTileDefs[0];
		}

		f32 targetWeight =  tRandom::fSubjectiveRand( ).fFloatZeroToValue( totalWeight );
		const tEditableTileDef* returnTileDef = mTileDefs[0];
		for( u32 i = 0; i < mTileDefs.fCount( ); ++i )
		{
			if( mTileDefs[i]->fIsRandomable( ) && targetWeight <= mTileDefs[i]->mRandomWeight )
			{
				returnTileDef = mTileDefs[i];
				break;
			}

			targetWeight -= mTileDefs[i]->mRandomWeight;
		}

		return returnTileDef;
	}

	b32 tEditableTileTypeList::fHasModel( const tFilePathPtr& modelPath ) const
	{
		for( u32 i = 0; i < mTileDefs.fCount( ); ++i )
		{
			if( mTileDefs[i]->mModelAssetPath == modelPath )
				return true;
		}

		return false;
	}

	TileDb::tTileTypeList tEditableTileTypeList::fSerialize( ) const
	{
		TileDb::tTileTypeList ret;

		ret.mTypeName = mTypeName;
		ret.mTypeEnum = mTypeEnum;

		ret.mTileData.fResize( mTileDefs.fCount( ) );
		for( u32 i = 0; i < mTileDefs.fCount( ); ++i )
		{
			tEditableTileDef* thisData = mTileDefs[ i ];
			TileDb::tTileData saveData;

			saveData.mTileFilePath = thisData->mTileAssetPath;
			saveData.mModelFilePath = thisData->mModelAssetPath;

			ret.mTileData[i] = saveData;
		}

		return ret;
	}

	//------------------------------------------------------------------------------
	// tEditableTileSet
	//------------------------------------------------------------------------------
	tEditableTileSet::tEditableTileSet( )
		: mGuid( 0 )
	{
		mTileTypesData.fSetCount( cNumTileTypes );

		mTileTypesData[ cFloor ] = new tEditableTileTypeList( cFloor, "Floor" );
		mTileTypesData[ cWall ] = new tEditableTileTypeList( cWall, "Wall" );
		mTileTypesData[ cNiche ] = new tEditableTileTypeList( cNiche, "Niche" );
		mTileTypesData[ cCorner ] = new tEditableTileTypeList( cCorner, "Corner" );
		mTileTypesData[ cUniques ] = new tEditableTileTypeList( cUniques, "Unique" );
	}

	tEditableTileSet::~tEditableTileSet( )
	{
		for( u32 i = 0; i < mTileTypesData.fCount( ); ++i )
			delete mTileTypesData[ i ];
	}

	void tEditableTileSet::fLoadDefaultTileTextures( const tResourceDepotPtr& resourceDepot )
	{
		mTileTypesData[ cFloor ]->fLoadDefaultTile( resourceDepot, tFilePathPtr( "_tools/SigTile/TileFloor_d.tga" ) );
		mTileTypesData[ cWall ]->fLoadDefaultTile( resourceDepot, tFilePathPtr( "_tools/SigTile/TileWall_d.tga" ) );
		mTileTypesData[ cNiche ]->fLoadDefaultTile( resourceDepot, tFilePathPtr( "_tools/SigTile/TileNiche_d.tga" ) );
		mTileTypesData[ cCorner ]->fLoadDefaultTile( resourceDepot, tFilePathPtr( "_tools/SigTile/TileCorner_d.tga" ) );
		mTileTypesData[ cUniques ]->fLoadDefaultTile( resourceDepot, tFilePathPtr( "_tools/SigTile/TileUnique_d.tga" ) );
	}

	void tEditableTileSet::fSetFamily( const tFilePathPtr& path )
	{
		mFamily = StringUtil::fNameFromPath( StringUtil::fUpNDirectories( path.fCStr( ), 1 ).c_str( ) );
	}

	tResourcePtr tEditableTileSet::fGetTileTexture( tTileTypes type, u32 variationId ) const
	{
		tEditableTileDef* thisVariation = mTileTypesData[ type ]->fTileDef( variationId );
		if( thisVariation->mTileResource.fNull( ) )
			return mTileTypesData[ type ]->fDefaultTileResource( );

		return thisVariation->mTileResource;
	}

	tResourcePtr tEditableTileSet::fGetDefaultTileTexture( tTileTypes type ) const
	{
		return mTileTypesData[ type ]->fDefaultTileResource( );
	}

	tSgFileRefEntityPtr tEditableTileSet::fGetTileModel( tTileTypes type, u32 variationId ) const
	{
		return mTileTypesData[ type ]->fTileDef( variationId )->mModelEntity;
	}

	tFilePathPtr tEditableTileSet::fGetTileModelPath( tTileTypes type, u32 variationId ) const
	{
		return mTileTypesData[ type ]->fTileDef( variationId )->mModelAssetPath;
	}

	Math::tVec2u tEditableTileSet::fGetTileDims( tTileTypes type, u32 variationId ) const
	{
		return mTileTypesData[ type ]->fTileDef( variationId )->mDims;
	}

	

	b32 tEditableTileSet::fHasRandomTiles( tTileTypes requestedType ) const
	{
		return mTileTypesData[ requestedType ]->fHasRandomTiles( );
	}

	tFilePathPtr tEditableTileSet::fConstructTileModelPath( const Tieml::tTile& tile ) const
	{
		tFilePathPtr modelAssetPath;

		if( tile.mRandType == cNotRandom )
		{
			modelAssetPath = tile.mModelPath;
		}
		else
		{
			const tEditableTileDef* randomTile = fGetRandomTileDef( (tTileTypes)tile.mTileType );
			if( !randomTile )
				return tFilePathPtr( );

			modelAssetPath = randomTile->mModelAssetPath;
		}

		return tFilePathPtr::fConstructPath( mResDir, modelAssetPath );
	}

	const tEditableTileDef* tEditableTileSet::fGetRandomTileDef( tTileTypes requestedType ) const
	{
		return mTileTypesData[ requestedType ]->fGetRandomTileDef( );
	}

	void tEditableTileSet::fSerialize( ) const
	{
		TileDb::tFile out;

		out.mName = mName;
		out.mFamily = mFamily;
		out.mGuid = mGuid;
		out.mResDir = mResDir;

		out.mTileTypesList.fResize( mTileTypesData.fCount( ) );
		for( u32 i = 0; i < mTileTypesData.fCount( ); ++i )
			out.mTileTypesList[ i ] = mTileTypesData[ i ]->fSerialize( );

		tFilePathPtr outputPath = tFilePathPtr::fConstructPath( mResDir, tFilePathPtr( "\\database.tiledb" ) );
		out.fSaveXml( ToolsPaths::fMakeResAbsolute( outputPath ), true );
	}

	void tEditableTileSet::fDeserialize( const TileDb::tFile& set, const tResourceDepotPtr& resourceDepot )
	{
		mName = set.mName;
		mFamily = set.mFamily;
		mGuid = set.mGuid;
		mResDir = set.mResDir;
		mDiscoveredDir = set.mDiscoveredDir;

		fLoadDefaultTileTextures( resourceDepot );

		mTileTypesData.fSetCount( set.mTileTypesList.fCount( ) );
		for( u32 i = 0; i < set.mTileTypesList.fCount( ); ++i )
		{
			const TileDb::tTileTypeList& typeList = set.mTileTypesList[i];
			mTileTypesData[i]->fSetTypeEnum( (tTileTypes)set.mTileTypesList[i].mTypeEnum );
			mTileTypesData[i]->fSetTypeName( set.mTileTypesList[i].mTypeName );

			for( u32 j = 0; j < typeList.mTileData.fCount( ); ++j )
				fAddTileDef( resourceDepot, (tTileTypes)typeList.mTypeEnum, StringUtil::fNameFromPath( typeList.mTileData[j].mModelFilePath.fCStr( ), true ), mResDir, typeList.mTileData[j].mModelFilePath, typeList.mTileData[j].mTileFilePath );
		}
	}

	void tEditableTileSet::fAddTileDef( const tResourceDepotPtr& resourceDepot, tTileTypes type, std::string modelName, const tFilePathPtr& tileSetDir, const tFilePathPtr& modelPath, const tFilePathPtr& tilePath )
	{
		tEditableTileDef* newTile = new tEditableTileDef( );

		std::string o = modelName;

		size_t lastUnderscore = o.find_last_of( "_" );
		size_t firstDot = o.find( ".", lastUnderscore );
		size_t separationX = o.find( "x", lastUnderscore );
		if( lastUnderscore != std::string::npos && firstDot != std::string::npos && lastUnderscore < separationX && separationX < firstDot )
		{
			o.resize( firstDot );
			std::string lastSlice = &o[ lastUnderscore ];
			lastSlice = StringUtil::fFirstCharAfter( lastSlice.c_str( ), "_" );

			tGrowableArray< std::string > halves;
			StringUtil::fSplit( halves, lastSlice.c_str( ), "x" );

			sigassert( halves.fCount( ) == 2 );

			u32 xDim = (u32)atof( halves[0].c_str( ) );
			u32 yDim = (u32)atof( halves[1].c_str( ) );

			newTile->mDims = Math::tVec2u( xDim, yDim );
		}

		newTile->mShortName = modelName;
		newTile->fLoadAndSetTexture( tilePath, resourceDepot );

		if( StringUtil::fStrStrI( modelName.c_str( ), gAutoID ) )
			newTile->mRandomWeight = 1.f;

		tFilePathPtr path = tFilePathPtr::fConstructPath( tileSetDir, modelPath);
		newTile->fLoadAndSetModel( modelPath, path, resourceDepot );

		mTileTypesData[ type ]->fAddTileDef( newTile );
	}

	b32 tEditableTileSet::fHasTileDef( tTileTypes type, const tFilePathPtr& modelPath )
	{
		for( u32 i = 0; i < mTileTypesData[ type ]->fNumTileDefs( ); ++i )
		{
			if( mTileTypesData[ type ]->fTileDef( i )->mModelAssetPath == modelPath )
				return true;
		}

		return false;
	}

	void tEditableTileSet::fRemoveDeletedTileDefs( const tFilePathPtrList& discoveredFiles )
	{
		for( u32 type = 0; type < cNumTileTypes; ++type )
		{
			tEditableTileTypeList* typeList = mTileTypesData[ type ];
			for( u32 i = 0; i < typeList->fNumTileDefs( ); ++i )
			{
				tEditableTileDef* tileDef = typeList->fTileDef( i );
				if( !discoveredFiles.fFind( tileDef->mModelAssetPath ) )
					typeList->fEraseTileDef( i-- );
			}
		}
	}

	u32 tEditableTileSet::fGetVariationId( tTileTypes requestedType, const tFilePathPtr& modelPath ) const
	{
		for( u32 i = 0; i < mTileTypesData[ requestedType ]->fNumTileDefs( ); ++i )
		{
			if( mTileTypesData[ requestedType ]->fTileDef( i )->mModelAssetPath == modelPath )
				return i;
		}

		return ~0;
	}

	//------------------------------------------------------------------------------
	// tEditableTileDb
	//------------------------------------------------------------------------------
	tEditableTileDb::tEditableTileDb( )
		: mPigments( this )
		, mNodeDefs( this )
	{
	}

	tEditableTileDb::~tEditableTileDb( )
	{
		for( u32 i = 0; i < mTileSets.fCount( ); ++i )
			delete mTileSets[i];

		for( u32 i = 0; i < mRandomTileResources.fCount( ); ++i )
		{
			if( !mRandomTileResources[i].fNull( ) )
				mRandomTileResources[i]->fUnload( this );
		}
	}

	tEditableTileSet* tEditableTileDb::fTileSetByGuid( u32 guid ) const
	{
		for( u32 i = 0; i < mTileSets.fCount( ); ++i )
		{
			tEditableTileSet* thisSet = mTileSets[ i ];
			if( thisSet->fGuid( ) == guid )
				return thisSet;
		}

		return NULL;
	}

	tFilePathPtr tEditableTileDb::fConstructTileModelPath( const Tieml::tTile& tile )
	{
		const tEditableTileSet* tileSet = fTileSetFromSerializedTile( tile );
		if( ! tileSet )
			return tFilePathPtr( );

		return tileSet->fConstructTileModelPath( tile );
	}

	void tEditableTileDb::fSeedRandomTiles( )
	{
		for( u32 i = 0; i < mPigments.fNumPigments( ); ++i )
			mPigments.fPigmentByIdx( i )->fSeedRandom( );
	}

	void tEditableTileDb::fBakeRandomTileForEditor( tEditableTileEntityPtr& tile ) const
	{
		const tEditableTileSet* tileSet = fTileSetFromEditableTile( tile );
		if( ! tileSet )
			return;

		const tEditableTileDef* randomTile = tileSet->fGetRandomTileDef( tile->fTileType( ) );
		if( !randomTile )
		{
			log_warning( 0, "fBakeRandomTileForEditor: failed to get random tile." );
			return;
		}

		tile->fBakeRandomizedTile( randomTile->mModelAssetPath, randomTile->mModelEntity );
	}

	tFilePathPtr tEditableTileDb::fGetScriptPath( u32 scriptGuid )
	{
		const tEditableScriptNodeDef* node = fNodeByGuid( scriptGuid );
		if( !node )
			return tFilePathPtr( );

		return node->fScriptPath( );
	}

	tEditableTileEntityPtr tEditableTileDb::fDeserializeTile( const Tieml::tTile& tile ) const
	{
		const tEditableTileSetPigment* brush = fPigmentByGuid( tile.mPigmentGuid );
		if( !brush )
			return tEditableTileEntityPtr( );

		tEditableTileEntityPtr ret = brush->fDeserializeTile( tile );
		ret->fDeserializeScripts( tile.mAttachedScriptGuids, this );

		return ret;
	}

	void tEditableTileDb::fAddMissingTileDefs( const tResourceDepotPtr& resourceDepot, u32 tileIdx, tFilePathPtrList& files )
	{
		tEditableTileSet* tileSet = mTileSets[tileIdx];
		const char* dirString = tileSet->fResDir( ).fCStr( );
		const char slashString[] = { fPlatformFilePathSlash( cCurrentPlatform ), '\0' };

		for( u32 i = 0; i < files.fCount( ); ++i )
		{
			// Snip the relative file path.
			files[i] = tFilePathPtr( StringUtil::fFirstCharAfter( files[ i ].fCStr( ), dirString) );

			tGrowableArray< std::string > subStrings;
			StringUtil::fSplit( subStrings, files[i].fCStr( ), slashString );

			b32 foundTileSet = false;
			for( u32 isub = 0; isub < subStrings.fCount( ); ++isub )
			{
				std::string thisSubString = subStrings[ isub ];

				if( !StringUtil::fCheckExtension( thisSubString, ".sigml" ) )
					continue;

				const char* thisCStr = thisSubString.c_str( );

				// Add item to category.
				if( StringUtil::fStrStrI( thisCStr, gFloorID ) == thisCStr && !tileSet->fHasTileDef( cFloor, files[i] ) )
				{
					tileSet->fAddTileDef( resourceDepot, cFloor, thisSubString, tileSet->fResDir( ), files[i], tFilePathPtr( "_tools/SigTile/TileFloor_d.tga" ) );
					break;
				}
				else if( StringUtil::fStrStrI( thisCStr, gWallID ) == thisCStr && !tileSet->fHasTileDef( cWall, files[i] ) )
				{
					tileSet->fAddTileDef( resourceDepot, cWall, thisSubString, tileSet->fResDir( ), files[i], tFilePathPtr( "_tools/SigTile/TileWall_d.tga" ) );
					break;
				}
				else if( StringUtil::fStrStrI( thisCStr, gNicheID ) == thisCStr && !tileSet->fHasTileDef( cNiche, files[i] ) )
				{
					tileSet->fAddTileDef( resourceDepot, cNiche, thisSubString, tileSet->fResDir( ), files[i], tFilePathPtr( "_tools/SigTile/TileNiche_d.tga" ) );
					break;
				}
				else if( StringUtil::fStrStrI( thisCStr, gCornerID ) == thisCStr && !tileSet->fHasTileDef( cCorner, files[i] ) )
				{
					tileSet->fAddTileDef( resourceDepot, cCorner, thisSubString, tileSet->fResDir( ), files[i], tFilePathPtr( "_tools/SigTile/TileCorner_d.tga" ) );
					break;
				}
				else if( StringUtil::fStrStrI( thisCStr, gUniquesID ) == thisCStr && !tileSet->fHasTileDef( cUniques, files[i] ) )
				{
					tileSet->fAddTileDef( resourceDepot, cUniques, thisSubString, tileSet->fResDir( ), files[i], tFilePathPtr( "_tools/SigTile/TileUnique_d.tga" ) );
					break;
				}
			}
		}
	}

	void tEditableTileDb::fRemoveDeletedTileDefs( u32 tileIdx, const tFilePathPtrList& files )
	{
		tEditableTileSet* tileSet = mTileSets[tileIdx];
		tileSet->fRemoveDeletedTileDefs( files );
	}

	u32 tEditableTileDb::fGetFreeGuid( ) const
	{
		b32 everythingIsOk = false;
		u32 potentialGuid = fGenerateGuid( );
		while( !everythingIsOk )
		{
			everythingIsOk = true;
			potentialGuid = fGenerateGuid( );

			for( u32 i = 0; i < mTileSets.fCount( ); ++i )
			{
				if( mTileSets[i]->fGuid( ) == potentialGuid )
				{
					everythingIsOk = false; // everything is not ok!
					break;
				}
			}
		}

		return potentialGuid;
	}

	const tEditableTileSet* tEditableTileDb::fTileSetFromSerializedTile( const Tieml::tTile& tile ) const
	{
		tEditableTileSetPigment* brush = mPigments.fTileSetPigmentByGuid( tile.mPigmentGuid );
		if( !brush )
		{
			// Kyle, I had to disable this to get a build through -Max
			//log_warning( 0, "fTileSetFromSerializedTile: brush GUID didn't find a brush." );
			return NULL;
		}

		tEditableTileSet* tileSet = fTileSetByGuid( brush->fGetTileSetGuidAuto( ) );
		if( !tileSet )
		{
			log_warning( 0, "fTileSetFromSerializedTile: tile set GUID didn't find a tile set." );
			return NULL;
		}

		return tileSet;
	}

	const tEditableTileSet* tEditableTileDb::fTileSetFromEditableTile( const tEditableTileEntityPtr& tile ) const
	{
		tEditableTileSetPigment* pigment = mPigments.fTileSetPigmentByGuid( tile->fPigmentGuid( ) );
		if( !pigment )
		{
			log_warning( 0, "fTileSetFromEditableTile: pigment GUID didn't find a pigment." );
			return NULL;
		}

		tEditableTileSet* tileSet = fTileSetByGuid( pigment->fGetTileSetGuidAuto( ) );
		if( !tileSet )
		{
			log_warning( 0, "fTileSetFromEditableTile: tile set GUID didn't find a tile set." );
			return NULL;
		}

		return tileSet;
	}

	void tEditableTileDb::fSerializeTileSets( ) const
	{
		for( u32 i = 0; i < mTileSets.fCount( ); ++i )
			mTileSets[ i ]->fSerialize( );
	}

	b32 tEditableTileDb::fDeserializeTileSets( const TileDb::tFile& file, b32 useRepairDialog, const tResourceDepotPtr& resourceDepot )
	{
		tEditableTileSet* newTileSet = new tEditableTileSet( );
		newTileSet->fDeserialize( file, resourceDepot );

		// Check for copied tile sets.
		for( u32 i = 0; i < mTileSets.fCount( ); ++i )
		{
			if( newTileSet->fGuid( ) != mTileSets[i]->fGuid( ) && file.mResDir == file.mDiscoveredDir )
				continue;

			// Discover which set is suspect.
			tEditableTileSet* conflictedSet = NULL;
			if( newTileSet->fResDir( ) != newTileSet->fDiscoveredDir( ) )
			{
				conflictedSet = newTileSet;
			}
			else if( mTileSets[i]->fResDir( ) != mTileSets[i]->fDiscoveredDir( ) )
			{
				conflictedSet = mTileSets[i];
			}
			else
			{
				// TODO: how to handle this error case.
				return true;
			}

			// If it's not using dialogs, it must be AssetGen so just log a warning and exit.
			if( !useRepairDialog )
			{
				log_warning( 0, "Conflicted tile set found, please correct using SigTile. (" << conflictedSet->mDiscoveredDir.fCStr( ) << ")" );
				return false;
			}

			std::stringstream ss;
			ss << "The following tile set appears to have been copied from a different location ";
			ss << "and conflicts with currently loaded tile sets. Clean the tile set?\n";
			ss << "\nInvalid tile set:	" << conflictedSet->fDiscoveredDir( ).fCStr( ) << "\n";
			ss << "\nNOTE: Choosing No will close SigTile without changing anything. ";
			ss << "You will need to manually clean up the conflict by deleting the bad tile set.";

			tStrongPtr<wxMessageDialog> conflictDialog( new wxMessageDialog( 
				NULL,
				ss.str( ),
				"Conflicting Tile Set GUIDs Detected",
				wxYES_NO | wxNO_DEFAULT | wxICON_ERROR ) );

			if( conflictDialog->ShowModal( ) == wxID_YES )
			{
				conflictedSet->fSetGuid( fGenerateGuid( ) );
				conflictedSet->fSetResDir( conflictedSet->fDiscoveredDir( ) );
				conflictedSet->fSetFamily( conflictedSet->fDiscoveredDir( ) );
				conflictedSet->fSetName( StringUtil::fNameFromPath( conflictedSet->fDiscoveredDir( ).fCStr( ) ) );
			}
			else
			{
				// Close immediately.
				return false;
			}
		}

		mTileSets.fPushBack( newTileSet );

		return true;
	}

	void tEditableTileDb::fSerializeTileSetPigments( Tieml::tFile& file ) const
	{
		mPigments.fSerialize( file );
	}

	void tEditableTileDb::fDeserializeTileSetPigments( Tieml::tFile& file )
	{
		mPigments.fDeserialize( file );
	}

	void tEditableTileDb::fClearTileSetPigments( )
	{
		mPigments.fClear( );
	}

	void tEditableTileDb::fSerializeScriptNodeDefs( Tieml::tFile& file ) const
	{
		mNodeDefs.fSerialize( file );
	}

	void tEditableTileDb::fDeserializeScriptNodeDefs( Tieml::tFile& file, const tResourceDepotPtr& resourceDepot )
	{
		mNodeDefs.fDeserialize( file, resourceDepot );
	}

	void tEditableTileDb::fClearScriptNodeDefs( )
	{
		mNodeDefs.fClear( );
	}

	void tEditableTileDb::fLoadRandomTileAssets( const tResourceDepotPtr& resourceDepot )
	{
		mRandomTileResources.fResize( cNumTileTypes );
		mRandomTileResources[ cFloor ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr( "_tools/SigTile/TileFloor_d.tga" ) ), this );
		mRandomTileResources[ cWall ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr( "_tools/SigTile/TileWall_d.tga" ) ), this );
		mRandomTileResources[ cNiche ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr( "_tools/SigTile/TileNiche_d.tga" ) ), this );
		mRandomTileResources[ cCorner ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr( "_tools/SigTile/TileCorner_d.tga" ) ), this );
		mRandomTileResources[ cUniques ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr( "_tools/SigTile/TileUnique_d.tga" ) ), this );
	}

	tResourcePtr tEditableTileDb::fRandomTileMarker( tTileTypes forThisType )
	{
		return mRandomTileResources[ forThisType ];
	}

	void tEditableTileDb::fRebuild( const tResourceDepotPtr& resourceDepot )
	{
		for( u32 i = 0; i < mTileSets.fCount( ); ++i )
		{
			// Add any missing tile defs.
			tFilePathPtrList files;
			FileSystem::fGetFileNamesInFolder( files, ToolsPaths::fMakeResAbsolute( mTileSets[i]->fResDir( ) ), true, true );

			fAddMissingTileDefs( resourceDepot, i, files );

			// Subtract any gone tile defs.
			fRemoveDeletedTileDefs( i, files );
		}

		mPigments.fOnTileDbChanged( );
	}

	void tEditableTileDb::fAddTileSet( const tResourceDepotPtr& resourceDepot, const tFilePathPtr& dirPath, const char* family )
	{
		// Get name.
		const char slashString[] = { fPlatformFilePathSlash( cCurrentPlatform ), '\0' };
		std::string tileSetName = StringUtil::fNameFromPath( dirPath.fCStr( ) );

		// Configure and add the tile set here.
		tEditableTileSet* newSet = new tEditableTileSet( );
		newSet->fSetName( tileSetName );
		newSet->fSetResDir( ToolsPaths::fMakeResRelative( dirPath ) );
		if( !family )
			newSet->fSetFamily( dirPath );
		else
			newSet->fSetFamily( family );
		newSet->fSetGuid( fGenerateGuid( ) );
		mTileSets.fPushBack( newSet );

		newSet->fLoadDefaultTileTextures( resourceDepot );

		// Grab all names.
		tFilePathPtrList files;
		FileSystem::fGetFileNamesInFolder( files, dirPath, true, true );

		// add
		for( u32 i = 0; i < files.fCount( ); ++i )
		{
			const tFilePathPtr file( StringUtil::fFirstCharAfter( files[ i ].fCStr( ), dirPath.fCStr( ) ) );

			tGrowableArray< std::string > subStrings;
			StringUtil::fSplit( subStrings, file.fCStr( ), slashString );

			b32 foundTileSet = false;
			for( u32 isub = 0; isub < subStrings.fCount( ); ++isub )
			{
				std::string thisSubString = subStrings[ isub ];

				if( !StringUtil::fCheckExtension( thisSubString, ".sigml" ) )
					continue;

				const char* thisCStr = thisSubString.c_str( );

				// Add item to category.
				if( StringUtil::fStrStrI( thisSubString.c_str( ), gFloorID ) == thisCStr )
				{
					newSet->fAddTileDef( resourceDepot, cFloor, thisSubString, newSet->fResDir( ), file, tFilePathPtr( "_tools/SigTile/TileFloor_d.tga" ) );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str( ), gWallID ) == thisCStr )
				{
					newSet->fAddTileDef( resourceDepot, cWall, thisSubString, newSet->fResDir( ), file, tFilePathPtr( "_tools/SigTile/TileWall_d.tga" ) );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str( ), gNicheID ) == thisCStr )
				{
					newSet->fAddTileDef( resourceDepot, cNiche, thisSubString, newSet->fResDir( ), file, tFilePathPtr( "_tools/SigTile/TileNiche_d.tga" ) );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str( ), gCornerID ) == thisCStr )
				{
					newSet->fAddTileDef( resourceDepot, cCorner, thisSubString, newSet->fResDir( ), file, tFilePathPtr( "_tools/SigTile/TileCorner_d.tga" ) );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str( ), gUniquesID ) == thisCStr )
				{
					newSet->fAddTileDef( resourceDepot, cUniques, thisSubString, newSet->fResDir( ), file, tFilePathPtr( "_tools/SigTile/TileUnique_d.tga" ) );
					break;
				}
			}
		}
	}

	tEditableTileSetPigment* tEditableTileDb::fAddEmptyPigment( )
	{
		return mPigments.fAddEmptyPigment( );
	}

	void tEditableTileDb::fDeletePigment( tEditableTileSetPigment* pigment )
	{
		mPigments.fDeletePigment( pigment );
	}

	u32 tEditableTileDb::fNumPigments( ) const
	{
		return mPigments.fNumPigments( );
	}

	const tEditableTileSetPigment* tEditableTileDb::fPigmentByIdx( u32 idx ) const
	{
		return mPigments.fPigmentByIdx( idx );
	}

	tEditableTileSetPigment* tEditableTileDb::fPigmentByIdx( u32 idx )
	{
		return mPigments.fPigmentByIdx( idx );
	}

	const tEditableTileSetPigment* tEditableTileDb::fPigmentByGuid( u32 idx ) const
	{
		return mPigments.fTileSetPigmentByGuid( idx );
	}

	tEditableScriptNodeDef* tEditableTileDb::fAddEmptyScriptNode( const tResourceDepotPtr& resourceDepot )
	{
		return mNodeDefs.fAddEmptyNode( resourceDepot );
	}

	void tEditableTileDb::fDeleteNode( tEditableScriptNodeDef* node )
	{
		mNodeDefs.fDeleteNode( node );
	}

	u32 tEditableTileDb::fNumNodes( ) const
	{
		return mNodeDefs.fNumNodes( );
	}

	const tEditableScriptNodeDef* tEditableTileDb::fNodeByIdx( u32 idx ) const
	{
		return mNodeDefs.fNodeByIdx( idx );
	}

	tEditableScriptNodeDef* tEditableTileDb::fNodeByIdx( u32 idx )
	{
		return mNodeDefs.fNodeByIdx( idx );
	}

	const tEditableScriptNodeDef* tEditableTileDb::fNodeByGuid( u32 guid )const 
	{
		return mNodeDefs.fNodeByGuid( guid );
	}
}
