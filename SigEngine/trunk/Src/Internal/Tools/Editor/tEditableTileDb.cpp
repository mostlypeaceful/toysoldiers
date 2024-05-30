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
#include "tPlatform.hpp"
#include "Editor/tEditableTileCanvas.hpp"
#include "tWin32Window.hpp"

namespace Sig
{
	namespace
	{
		const char* gFloorID	= "f_";
		const char* gCornerID	= "co_";
		const char* gWallID		= "w_";
		const char* gNicheID	= "ci_";
		const char* gUniquesID	= "u_";
		const char* gDoorID		= "d_";
		const char* gFloorDoorID= "f_";

		const char* gPropCeiling= "pc_";
		const char* gPropFloor	= "pf_";

		static tEditableTiledml* gHackyGlobal = NULL;

		tFilePathPtr gDefaultFloorTilePath( "_tools/SigTile/TileFloor_d.tga" );
		tFilePathPtr gDefaultFloorDoorTilePath( "_tools/SigTile/TileFloorDoor_d.tga" );
		tFilePathPtr gDefaultWallTilePath( "_tools/SigTile/TileWall_d.tga" );
		tFilePathPtr gDefaultNicheTilePath( "_tools/SigTile/TileNiche_d.tga" );
		tFilePathPtr gDefaultCornerTilePath( "_tools/SigTile/TileCorner_d.tga" );
		tFilePathPtr gDefaultUniqueTilePath( "_tools/SigTile/TileUnique_d.tga" );
		tFilePathPtr gDefaultDoorTilePath( "_tools/SigTile/TileDoor_d.tga" );
		tFilePathPtr gDefaultExitTilePath( "_tools/SigTile/TileExit_d.tga" );
		tFilePathPtr gDefaultPropFloorTilePath( "_tools/SigTile/TileUnique_d.tga" );
		tFilePathPtr gDefaultPropCeilingTilePath( "_tools/SigTile/TileUnique_d.tga" );
	}

	//------------------------------------------------------------------------------
	// tEditableTileData
	//------------------------------------------------------------------------------
	tEditableTileDef::tEditableTileDef( const tEditableTileDef& copyFrom )
		: mRandomWeight( copyFrom.mRandomWeight )
		, mDims( copyFrom.mDims )
	{
		mIdString = copyFrom.mIdString;
		mShortName = copyFrom.mShortName;
		mTileAssetPath = copyFrom.mTileAssetPath;
		mModelAssetPath = copyFrom.mModelAssetPath;
		mAltered = copyFrom.mAltered;
	}

	tEditableTileDef::~tEditableTileDef( )
	{
		fUnloadResources( );
	}

	void tEditableTileDef::fLoadAndSetTexture( const tFilePathPtr& filePath, const tResourceDepotPtr& resourceDepot )
	{
		if( mTileAssetPath.fExists() && mTileAssetPath != filePath )
			mAltered = true;

		mTileAssetPath = filePath;
		mTileResource.fRelease( );

		if( !resourceDepot.fNull( ) )
			mTileResource = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( mTileAssetPath ), this );

		mAltered = true;
	}

	void tEditableTileDef::fLoadAndSetModel( const tFilePathPtr& modelPath, const tFilePathPtr& filePath, const tResourceDepotPtr& resourceDepot )
	{
		if( mModelAssetPath.fExists() && mModelAssetPath != modelPath )
			mAltered = true;

		mModelAssetPath = modelPath;
		mModelResource.fRelease( );

		if( !resourceDepot.fNull( ) )
		{
			mModelResource = resourceDepot->fQueryLoadBlock( tResourceId::fMake< tSceneGraphFile >( filePath ), this );
			mModelEntity = tSgFileRefEntityPtr( new tSgFileRefEntity( mModelResource ) );
		}

		
	}

	void tEditableTileDef::fUnloadResources( )
	{
		mTileResource.fRelease( );
		mModelResource.fRelease( );
	}

	//------------------------------------------------------------------------------
	// tEditableTileTypeList
	//------------------------------------------------------------------------------
	tEditableTileTypeList::tEditableTileTypeList( tTileTypes type, std::string name )
		: mTypeEnum( type )
		, mTypeName( name )
	{
	}

	tEditableTileTypeList::tEditableTileTypeList( const tEditableTileTypeList& copyFrom )
		: mTypeEnum( copyFrom.mTypeEnum )
		, mTypeName( copyFrom.mTypeName )
	{
		for( u32 i = 0; i < copyFrom.fNumTileDefs(); ++i )
			mTileDefs.fPushBack( new tEditableTileDef( *copyFrom.mTileDefs[i] ) );
	}

	tEditableTileTypeList::~tEditableTileTypeList( )
	{
		fClear();
	}

	void tEditableTileTypeList::fClear( )
	{
		for( u32 i = 0; i < mTileDefs.fCount(); ++i )
			delete mTileDefs[i];

		mTileDefs.fSetCount( 0 );
	}

	void tEditableTileTypeList::fLoadDefaultTile( const tResourceDepotPtr& resourceDepot, const tFilePathPtr& defaultTilePath )
	{
		mDefaultTileAssetPath = defaultTilePath;

		if( !resourceDepot.fNull( ) )
			mDefaultTileResource = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( mDefaultTileAssetPath ), this );
	}

	const tEditableTileDef* tEditableTileTypeList::fTileDef( std::string id ) const
	{
		for( u32 i = 0; i < mTileDefs.fCount(); ++i )
		{
			if( mTileDefs[i]->mIdString == id )
				return mTileDefs[i];
		}

		return NULL;
	}
	tEditableTileDef* tEditableTileTypeList::fTileDef( std::string id )
	{
		for( u32 i = 0; i < mTileDefs.fCount(); ++i )
		{
			if( mTileDefs[i]->mIdString == id )
				return mTileDefs[i];
		}

		return NULL;
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
			log_warning( "Tried to randomize a tile but there were no random tiles available." );
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

	Tiledml::tTileTypeList tEditableTileTypeList::fSerialize( ) const
	{
		Tiledml::tTileTypeList ret;

		ret.mTypeName = mTypeName;
		ret.mTypeEnum = mTypeEnum;

		ret.mTileData.fResize( mTileDefs.fCount( ) );
		for( u32 i = 0; i < mTileDefs.fCount( ); ++i )
		{
			tEditableTileDef* thisData = mTileDefs[ i ];
			Tiledml::tTileData saveData;

			saveData.mTileFilePath = thisData->mTileAssetPath;
			saveData.mModelFilePath = thisData->mModelAssetPath;
			saveData.mIdString = tStringPtr( thisData->mIdString );

			ret.mTileData[i] = saveData;
		}

		return ret;
	}

	//------------------------------------------------------------------------------
	// tEditableTileSet
	//------------------------------------------------------------------------------
	tEditableTileSet::tEditableTileSet( )
		: mGuid( 0 )
		, mLoaded( false )
		, mAltered( false )
	{
		mTileTypesData.fSetCount( cNumTileTypes );

		mTileTypesData[ cFloor ] = new tEditableTileTypeList( cFloor, "Floor" );
		mTileTypesData[ cWall ] = new tEditableTileTypeList( cWall, "Wall" );
		mTileTypesData[ cNiche ] = new tEditableTileTypeList( cNiche, "Niche" );
		mTileTypesData[ cCorner ] = new tEditableTileTypeList( cCorner, "Corner" );
		mTileTypesData[ cUniques ] = new tEditableTileTypeList( cUniques, "Unique" );
		mTileTypesData[ cWallDoor ] = new tEditableTileTypeList( cWallDoor, "WallDoor" );
		mTileTypesData[ cWallExit ] = new tEditableTileTypeList( cWallExit, "Exit" );
		mTileTypesData[ cFloorDoor ] = new tEditableTileTypeList( cFloorDoor, "FloorDoor" );
		mTileTypesData[ cPropCeiling ] = new tEditableTileTypeList( cPropCeiling, "PropCeiling" );
		mTileTypesData[ cPropFloor ] = new tEditableTileTypeList( cPropFloor, "PropFloor" );
	}

	tEditableTileSet::tEditableTileSet( const tEditableTileSet& copyFrom )
		: mGuid( copyFrom.mGuid )
		, mLoaded( copyFrom.mLoaded )
	{
		mTileTypesData.fSetCount( cNumTileTypes );

		for( u32 i = 0; i < mTileTypesData.fCount(); ++i )
			mTileTypesData[i] = new tEditableTileTypeList( *copyFrom.mTileTypesData[i] );
	}

	tEditableTileSet::~tEditableTileSet( )
	{
		for( u32 i = 0; i < mTileTypesData.fCount(); ++i )
			delete mTileTypesData[i];
	}

	b32 tEditableTileSet::fAltered( ) const
	{
		if( mAltered )
			return true;

		for( u32 i = 0; i < mTileTypesData.fCount(); ++i )
		{
			const tEditableTileTypeList* someList = mTileTypesData[i];
			for( u32 j = 0; j < someList->fNumTileDefs(); ++j )
			{
				if( someList->fTileDef(j)->fAltered() )
					return true;
			}
		}

		return false;
	}

	void tEditableTileSet::fLoad( const tResourceDepotPtr& resourceDepot, wxProgressDialog* optionalDialog, s32 optDialogProgress )
	{
		mLoaded = true;
		fLoadDefaultTileTextures( resourceDepot );

		for( u32 i = 0; i < mTileTypesData.fCount(); ++i )
		{
			const u32 numTileDefs = mTileTypesData[i]->fNumTileDefs();
			for( u32 tileIdx = 0; tileIdx < numTileDefs; ++tileIdx )
			{
				if( optionalDialog )
				{
					optionalDialog->Update( optDialogProgress );
					tWin32Window::fMessagePump( );
				}

				tEditableTileDef* def = mTileTypesData[i]->fTileDef( tileIdx );
				def->fLoadAndSetTexture( def->mTileAssetPath, resourceDepot );

				tFilePathPtr path = tFilePathPtr::fConstructPath( mResDir, def->mModelAssetPath );
				def->fLoadAndSetModel( def->mModelAssetPath, path, resourceDepot );
			}
		}
	}

	void tEditableTileSet::fLoadDefaultTileTextures( const tResourceDepotPtr& resourceDepot )
	{
		mTileTypesData[ cFloor ]->fLoadDefaultTile( resourceDepot, gDefaultFloorTilePath );
		mTileTypesData[ cWall ]->fLoadDefaultTile( resourceDepot, gDefaultWallTilePath );
		mTileTypesData[ cNiche ]->fLoadDefaultTile( resourceDepot, gDefaultNicheTilePath );
		mTileTypesData[ cCorner ]->fLoadDefaultTile( resourceDepot, gDefaultCornerTilePath );
		mTileTypesData[ cUniques ]->fLoadDefaultTile( resourceDepot, gDefaultUniqueTilePath );
		mTileTypesData[ cWallDoor ]->fLoadDefaultTile( resourceDepot, gDefaultDoorTilePath );
		mTileTypesData[ cFloorDoor ]->fLoadDefaultTile( resourceDepot, gDefaultFloorDoorTilePath );
		mTileTypesData[ cPropCeiling ]->fLoadDefaultTile( resourceDepot, gDefaultPropCeilingTilePath );
		mTileTypesData[ cPropFloor ]->fLoadDefaultTile( resourceDepot, gDefaultPropFloorTilePath );
	}

	void tEditableTileSet::fExtractTextures( const tEditableTileSet& copyFrom )
	{
		for( u32 t = 0; t < mTileTypesData.fCount(); ++t )
		{
			// Kept to assign to any tiles that do not have a asset path.
			tFilePathPtr cachedGoodPath;

			tEditableTileTypeList* typeList = ( t < copyFrom.fNumTileTypeLists() ) ? copyFrom.fTileTypeList( (tTileTypes)t ) : NULL;
			tEditableTileTypeList* myList = mTileTypesData[t];
			for( u32 d = 0; d < myList->fNumTileDefs(); ++d )
			{
				// Find the corresponding tile def.
				tEditableTileDef* myDef = myList->fTileDef( d );
				tEditableTileDef* tileDef = ( typeList ) ? typeList->fTileDef( myDef->mIdString ) : NULL;

				if( !tileDef || !tileDef->mTileAssetPath.fExists() || tileDef->mTileAssetPath == typeList->fDefaultTilePath() )
				{
					// Source tile doesn't have a path. Use the cached if possible.
					if( cachedGoodPath.fExists() )
						myDef->mTileAssetPath = cachedGoodPath;
				}
				else
				{
					// Source tile has a path, use it.
					myDef->mTileAssetPath = tileDef->mTileAssetPath;

					// Save this for anything else in this type we might need it for.
					if( !cachedGoodPath.fExists() )
						cachedGoodPath = tileDef->mTileAssetPath;
				}
			}
		}
	}

	void tEditableTileSet::fExtractResources( const tEditableTileSet& copyFrom, const tResourceDepotPtr& resourceDepot )
	{
		for( u32 t = 0; t < mTileTypesData.fCount(); ++t )
		{
			tEditableTileTypeList* typeList = ( t < copyFrom.fNumTileTypeLists() ) ? copyFrom.fTileTypeList( (tTileTypes)t ) : NULL;
			tEditableTileTypeList* myList = mTileTypesData[t];
			for( u32 d = 0; d < myList->fNumTileDefs(); ++d )
			{
				// Find the corresponding tile def.
				tEditableTileDef* myDef = myList->fTileDef( d );
				tEditableTileDef* tileDef = ( typeList ) ? typeList->fTileDef( myDef->mIdString ) : NULL;
				
				if( !tileDef )
				{
					// There's no corresponding tile def so it has to be loaded.
					myDef->fLoadAndSetTexture( myDef->mTileAssetPath, resourceDepot );
					tFilePathPtr path = tFilePathPtr::fConstructPath( mResDir, myDef->mModelAssetPath );
					myDef->fLoadAndSetModel( myDef->mModelAssetPath, path, resourceDepot );
				}
				else
				{
					myDef->mTileResource = tileDef->mTileResource;
					myDef->mModelAssetPath = tileDef->mModelAssetPath;
					myDef->mModelEntity = tileDef->mModelEntity;
					myDef->mModelResource = tileDef->mModelResource;
				}
			}
		}
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

	tFilePathPtr tEditableTileSet::fFilePath( ) const
	{
		return tFilePathPtr::fConstructPath( mResDir, tFilePathPtr(mFileName) );
	}

	b32 tEditableTileSet::fHasRandomTiles( tTileTypes requestedType ) const
	{
		return mTileTypesData[ requestedType ]->fHasRandomTiles( );
	}

	const tEditableTileDef* tEditableTileSet::fGetRandomTileDef( tTileTypes requestedType ) const
	{
		return mTileTypesData[ requestedType ]->fGetRandomTileDef( );
	}

	const tEditableTileDef* tEditableTileSet::fGetSpecificTileDef( tTileTypes requestedType, const std::string& idString ) const
	{
		return mTileTypesData[ requestedType ]->fTileDef( idString );
	}

	void tEditableTileSet::fSerialize( ) const
	{
		Tiledml::tFile out;

		out.mFileName = mFileName;
		out.mGuid = mGuid;
		out.mResDir = mResDir;

		out.mTileTypesList.fResize( mTileTypesData.fCount( ) );
		for( u32 i = 0; i < mTileTypesData.fCount( ); ++i )
			out.mTileTypesList[ i ] = mTileTypesData[ i ]->fSerialize( );

		out.fSaveXml( ToolsPaths::fMakeResAbsolute( fFilePath() ), true );
	}

	void tEditableTileSet::fDeserialize( const Tiledml::tFile& set )
	{
		mFileName		= set.mFileName;
		mGuid			= set.mGuid;
		mResDir			= set.mResDir;
		mDiscoveredDir	= set.mDiscoveredDir;

		// Read in the tile types. There may be more types than exist in this saved set due to
		// added categories.
		for( u32 i = 0; i < set.mTileTypesList.fCount( ); ++i )
		{
			const Tiledml::tTileTypeList& typeList = set.mTileTypesList[i];
			mTileTypesData[i]->fSetTypeEnum( (tTileTypes)set.mTileTypesList[i].mTypeEnum );
			mTileTypesData[i]->fSetTypeName( set.mTileTypesList[i].mTypeName );

			for( u32 j = 0; j < typeList.mTileData.fCount( ); ++j )
				fAddTileDef( (tTileTypes)typeList.mTypeEnum, StringUtil::fNameFromPath( typeList.mTileData[j].mModelFilePath.fCStr( ), true ), mResDir, typeList.mTileData[j].mModelFilePath, typeList.mTileData[j].mTileFilePath );
		}

		mAltered = false;
	}

	void tEditableTileSet::fBuild( const tFilePathPtr& dirPath )
	{
		// Get name.
		const char slashString[] = { fPlatformFilePathSlash( cCurrentPlatform ), '\0' };
		std::string tileSetName = StringUtil::fNameFromPath( dirPath.fCStr( ) );
		tileSetName += ".tiledml";

		// Configure and add the tile set here.
		fSetFileName( tileSetName );
		fSetResDir( ToolsPaths::fMakeResRelative( dirPath ) );
		fSetGuid( fGenerateGuid( ) );

		// Grab all names.
		tFilePathPtrList files;
		FileSystem::fGetFileNamesInFolder( files, dirPath, true, true, tFilePathPtr(".sigml") );

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

				const char* thisCStr = thisSubString.c_str( );

				// Add item to category.
				if( StringUtil::fStrStrI( thisSubString.c_str(), gFloorID ) == thisCStr )
				{
					fAddTileDef( cFloor, thisSubString, fResDir(), file, gDefaultFloorTilePath );
					fAddTileDef( cFloorDoor, thisSubString, fResDir(), file, gDefaultFloorDoorTilePath );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str(), gWallID ) == thisCStr )
				{
					fAddTileDef( cWall, thisSubString, fResDir(), file, gDefaultWallTilePath );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str(), gNicheID ) == thisCStr )
				{
					fAddTileDef( cNiche, thisSubString, fResDir(), file, gDefaultNicheTilePath );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str(), gCornerID ) == thisCStr )
				{
					fAddTileDef( cCorner, thisSubString, fResDir(), file, gDefaultCornerTilePath );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str(), gUniquesID ) == thisCStr )
				{
					fAddTileDef( cUniques, thisSubString, fResDir(), file, gDefaultUniqueTilePath );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str(), gDoorID ) == thisCStr )
				{
					fAddTileDef( cWallDoor, thisSubString, fResDir(), file, gDefaultDoorTilePath );
					fAddTileDef( cWallExit, thisSubString, fResDir(), file, gDefaultExitTilePath );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str(), gPropCeiling ) == thisCStr )
				{
					fAddTileDef( cPropCeiling, thisSubString, fResDir(), file, gDefaultPropCeilingTilePath );
					break;
				}
				else if( StringUtil::fStrStrI( thisSubString.c_str(), gPropFloor ) == thisCStr )
				{
					fAddTileDef( cPropFloor, thisSubString, fResDir(), file, gDefaultPropFloorTilePath );
					break;
				}
			}
		}
	}

	void tEditableTileSet::fRebuild( const tResourceDepotPtr& resourceDepot )
	{
		tEditableTileSet copyFrom( *this );

		for( u32 i = 0; i < mTileTypesData.fCount(); ++i )
			mTileTypesData[i]->fClear();

		u32 oldGuid = mGuid;
		fBuild( ToolsPaths::fMakeResAbsolute( fResDir() ) );
		mGuid = oldGuid;
		
		// Always grab textures.
		fExtractTextures( copyFrom );

		// Grab resources if they're loaded.
		if( copyFrom.fLoaded() )
		{
			mLoaded = true;
			fExtractResources( copyFrom, resourceDepot );
		}
		else
			mLoaded = false;

		mAltered = true;
	}

	void tEditableTileSet::fAddTileDef( tTileTypes type, std::string modelName, const tFilePathPtr& tileSetDir, const tFilePathPtr& modelPath, const tFilePathPtr& tilePath )
	{
		tEditableTileDef* newTile = new tEditableTileDef( );
		newTile->mDims = fExtractDimxDimInfo( modelName.c_str( ) );

		// Extract identifier tag. Ex "f_01" from "f_01_flat.sigml"
		std::string o = modelName;
		size_t firstNumber = o.find_first_of( "0123456789" );
		size_t underscoreAfterNumber = o.find( "_", firstNumber );
		if( firstNumber != std::string::npos && underscoreAfterNumber != std::string::npos )
		{
			o.resize( underscoreAfterNumber );
			newTile->mIdString = o;
		}

		// Save name and texture and other stuff.
		newTile->mShortName = modelName;
		newTile->mTileAssetPath = tilePath;
		newTile->mModelAssetPath = modelPath;

		newTile->mRandomWeight = 1.f;

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

	Math::tVec2u tEditableTileSet::fExtractDimxDimInfo( const char* modelName )
	{
		std::string o = modelName;

		// Extract any dimXdim info
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
			return Math::tVec2u( xDim, yDim );
		}
		return Math::tVec2u( 1, 1 );
	}

	u32 tEditableTileSet::fGetVariationIdx( tTileTypes requestedType, const tFilePathPtr& modelPath ) const
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
	tEditableTiledml::tEditableTiledml( )
	{
		gHackyGlobal = this;
	}

	tEditableTiledml::~tEditableTiledml( )
	{
		gHackyGlobal = NULL;

		for( u32 i = 0; i < mTileSets.fCount( ); ++i )
			delete mTileSets[i];
	}

	tEditableTileSet* tEditableTiledml::fTileSetByGuid( u32 guid ) const
	{
		for( u32 i = 0; i < mTileSets.fCount( ); ++i )
		{
			tEditableTileSet* thisSet = mTileSets[ i ];
			if( thisSet->fGuid( ) == guid )
				return thisSet;
		}

		return NULL;
	}

	tEditableTileSet* tEditableTiledml::fTileSetByFilePath( const tFilePathPtr& filepath ) const
	{
		for( u32 i = 0; i < mTileSets.fCount( ); ++i )
		{
			tEditableTileSet* thisSet = mTileSets[ i ];
			if( thisSet->fFilePath() == filepath )
				return thisSet;
		}

		return NULL;
	}

	tEditableTileSet* tEditableTiledml::fTileSetByFileName( const wxString& fileName ) const
	{
		for( u32 i = 0; i < mTileSets.fCount( ); ++i )
		{
			tEditableTileSet* thisSet = mTileSets[ i ];
			if( thisSet->fFileName() == fileName )
				return thisSet;
		}

		return NULL;
	}

	b32 tEditableTiledml::fAllTileSetsLoaded( ) const
	{
		for( u32 i = 0; i < mTileSets.fCount(); ++i )
		{
			if( !mTileSets[i]->fLoaded() )
				return false;
		}

		return true;
	}

	b32 tEditableTiledml::fAnyTileSetLoaded( ) const
	{
		for( u32 i = 0; i < mTileSets.fCount(); ++i )
		{
			if( mTileSets[i]->fLoaded() )
				return true;
		}

		return false;
	}

	wxArrayString tEditableTiledml::fGetLoadedTileSetsForDisplay( ) const
	{
		wxArrayString ret;

		for( u32 i = 0; i < mTileSets.fCount(); ++i )
		{
			if( mTileSets[i]->fLoaded() )
				ret.Add( mTileSets[i]->fFileName() );
		}

		return ret;
	}

	u32 tEditableTiledml::fGetFreeGuid( ) const
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

	tEditableTiledml* tEditableTiledml::fHackyGlobal()
	{
		return gHackyGlobal;
	}

	void tEditableTiledml::fSerializeTileSets( ) const
	{
		for( u32 i = 0; i < mTileSets.fCount(); ++i )
		{
			if( mTileSets[i]->fAltered() )
			{
				mTileSets[i]->fSerialize( );
				mTileSets[i]->fSaved( );
			}
		}
	}

	b32 tEditableTiledml::fDeserializeTileSet( const Tiledml::tFile& file, b32 useRepairDialog )
	{
		tEditableTileSet* newTileSet = new tEditableTileSet( );
		newTileSet->fDeserialize( file );

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
				log_warning( "Conflicted tile set found, please correct using SigTile. (" << conflictedSet->mDiscoveredDir.fCStr( ) << ")" );
				return false;
			}

			std::stringstream ss;
			ss << "The following tile set appears to have been copied from a different location ";
			ss << "and conflicts with currently loaded tile sets. Clean the tile set?\n";
			ss << "\nInvalid tile set:	" << conflictedSet->fDiscoveredDir( ).fCStr( ) << "\n";
			ss << "\nNOTE: Choosing No will cancel loading any further sets. ";
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
				conflictedSet->fSetFileName( StringUtil::fNameFromPath( conflictedSet->fDiscoveredDir( ).fCStr( ) ) );
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

	void tEditableTiledml::fLoadAllTileSets( const tResourceDepotPtr& resourceDepot )
	{
		if( mTileSets.fCount() == 0 )
		{
			tStrongPtr<wxMessageDialog> conflictDialog( new wxMessageDialog( 
				NULL,
				"No tile sets to load.",
				"No Tile Sets" ) );
			return;
		}

		wxProgressDialog progress( "Loading tile sets...", wxEmptyString, mTileSets.fCount(), NULL, wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxPD_CAN_ABORT );
		progress.SetSize( 300, progress.GetSize( ).y );

		const f32 tilecount = (f32)mTileSets.fCount( );

		for( u32 i = 0; i < mTileSets.fCount() && progress.Update( i,  mTileSets[i]->fFileName().c_str() ); ++i )
			mTileSets[i]->fLoad( resourceDepot, &progress, i );
	}

	void tEditableTiledml::fLoadTileAssets( const tResourceDepotPtr& resourceDepot )
	{
		mRandomTileResources.fResize( cNumTileTypes );
		mRandomTileResources[ cFloor ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr(gDefaultFloorTilePath) ), this );
		mRandomTileResources[ cWall ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr(gDefaultWallTilePath) ), this );
		mRandomTileResources[ cNiche ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr(gDefaultNicheTilePath) ), this );
		mRandomTileResources[ cCorner ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr(gDefaultCornerTilePath) ), this );
		mRandomTileResources[ cUniques ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr(gDefaultUniqueTilePath) ), this );
		mRandomTileResources[ cWallDoor ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr(gDefaultDoorTilePath) ), this );
		mRandomTileResources[ cWallExit ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr(gDefaultExitTilePath) ), this );
		mRandomTileResources[ cFloorDoor ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr(gDefaultFloorDoorTilePath) ), this );
		mRandomTileResources[ cPropCeiling ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr(gDefaultPropCeilingTilePath) ), this );
		mRandomTileResources[ cPropFloor ] = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr(gDefaultPropFloorTilePath) ), this );
	}

	tResourcePtr tEditableTiledml::fTileMarker( tTileTypes forThisType )
	{
		return mRandomTileResources[ forThisType ];
	}

	tEditableTileEntityPtr tEditableTiledml::fGetRandomizedTile( tEditableObjectContainer& container, tTileTypes requestedType )
	{
		tEditableTileEntity* newTile = new tEditableTileEntity( container );
		newTile->fConfigure(
			requestedType,
			fTileMarker( requestedType ), 
			"", 
			0xFFFFFFFF,
			Math::tVec2u( 1, 1 )
			);

		return tEditableTileEntityPtr( newTile );
	}

	tEditableTileEntityPtr tEditableTiledml::fGetSpecificTile( tEditableObjectContainer& container, tTileTypes requestedType, const tPair<u32,u32>& tileDefKey )
	{
		tEditableTileSet* tileSet = fTileSet( tileDefKey.mA );
		tEditableTileDef* tileDef = tileSet->fTileTypeList( requestedType )->fTileDef( tileDefKey.mB );

		tEditableTileEntity* newTile = new tEditableTileEntity( container );
		newTile->fConfigure(
			requestedType,
			fTileMarker( requestedType ), 
			tileDef->mIdString,
			0xFF9999FF, 
			tileDef->mDims
			);

		return tEditableTileEntityPtr( newTile );
	}

	void tEditableTiledml::fAddTileSet( const tResourceDepotPtr& resourceDepot, const tFilePathPtr& dirPath, const tEditableTileSet* texturesSet )
	{
		// Configure and add the tile set here.
		tEditableTileSet* newSet = new tEditableTileSet( );
		newSet->fLoadDefaultTileTextures( resourceDepot );
		newSet->fBuild( dirPath );
		if( texturesSet )
			newSet->fExtractTextures( *texturesSet );
		mTileSets.fPushBack( newSet );
	}
}
