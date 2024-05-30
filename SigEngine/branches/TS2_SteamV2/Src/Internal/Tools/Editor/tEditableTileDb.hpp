//------------------------------------------------------------------------------
// \file tEditableTileDb.hpp - 28 Oct 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tEditableTileDb__
#define __tEditableTileDb__

#include "tEditableTileEntity.hpp"
#include "tSgFileRefEntity.hpp"
#include "Tiledb.hpp"
#include "tResourceDepot.hpp"
#include "tEditableTileSetPigment.hpp"
#include "tEditableScriptNodeDef.hpp"
#include "tEditableScriptNodeEntity.hpp"

namespace Sig
{
	class tSigTileMainWindow;
	class tTileDbTree;
	class tTileDbPropPanel;
	class tTilePaintPanel;
	class tEditableTileDb;
	class tToolsGuiMainWindow;

	///
	/// \class tEditableTileData
	/// \brief 
	struct tools_export tEditableTileDef
	{
		tEditableTileDef( ) : mRandomWeight( 0.f ), mDims( 1, 1 ) { }
		~tEditableTileDef( );

		std::string mShortName;
		tFilePathPtr mTileAssetPath; //!< The 2D tile texture to show in 2D mode
		tFilePathPtr mModelAssetPath; //!< stored relative to the tile set's dir
		tResourcePtr mTileResource;
		tResourcePtr mModelResource;
		tSgFileRefEntityPtr mModelEntity;

		f32 mRandomWeight;

		Math::tVec2u mDims;

		b32 fIsRandomable( ) const { return mRandomWeight > 0.f; }

		void fLoadAndSetTexture( const tFilePathPtr& filePath, const tResourceDepotPtr& resourceDepot );
		void fLoadAndSetModel( const tFilePathPtr& modelPath, const tFilePathPtr& filePath, const tResourceDepotPtr& resourceDepot );

		void fUnloadResources( );

	private:
		void fUnloadResource( tResourcePtr& resource );
	};

	///
	/// \class tEditableTileTypeList
	/// \brief 
	class tools_export tEditableTileTypeList
	{
		tTileTypes mTypeEnum;
		std::string mTypeName;
		tFilePathPtr mDefaultTileAssetPath;
		tResourcePtr mDefaultTileResource;
		tGrowableArray< tEditableTileDef* > mTileDefs;

	public:
		tEditableTileTypeList( tTileTypes type, std::string name );
		~tEditableTileTypeList( );

		void fLoadDefaultTile( const tResourceDepotPtr& resourceDepot, const tFilePathPtr& defaultTilePath );

		void			fSetTypeEnum( tTileTypes newEnum ) { mTypeEnum = newEnum; }
		tTileTypes		fTypeEnum( ) const { return mTypeEnum; }
		void			fSetTypeName( const std::string& newName ) { mTypeName = newName; }
		std::string		fTypeName( ) const { return mTypeName; }
		tResourcePtr	fDefaultTileResource( ) const { return mDefaultTileResource; }

		u32							fNumTileDefs( ) const { return mTileDefs.fCount( ); }
		const tEditableTileDef*		fTileDef( u32 idx ) const { return mTileDefs[ idx ]; }
		tEditableTileDef*			fTileDef( u32 idx ) { return mTileDefs[ idx ]; }
		void						fAddTileDef( tEditableTileDef* newTileData ) { mTileDefs.fPushBack( newTileData ); }
		void						fEraseTileDef( u32 idx );

		b32							fHasRandomTiles( ) const;
		const tEditableTileDef*		fGetRandomTileDef( ) const;

		b32 fHasModel( const tFilePathPtr& modelPath ) const;

		TileDb::tTileTypeList fSerialize( ) const;
	};

	///
	/// \class tEditableTileSet
	/// \brief Contains run-time editable tile set data.
	class tools_export tEditableTileSet
	{
		friend class tEditableTileDb;

		u32				mGuid;
		std::string		mName;
		std::string		mFamily;
		tFilePathPtr	mResDir;
		tFilePathPtr	mDiscoveredDir; //!< Stupid but necessary
		tGrowableArray< tEditableTileTypeList* > mTileTypesData;

	public:
		tEditableTileSet( );
		~tEditableTileSet( );

		void fLoadDefaultTileTextures( const tResourceDepotPtr& resourceDepot );

		void fSetName( std::string newName ) { mName = newName; }
		void fSetFamily( std::string family ) { mFamily = family; }
		void fSetFamily( const tFilePathPtr& path ); // Deduces family by going up one directory
		void fSetResDir( const tFilePathPtr& resDir ) { mResDir = resDir; }
		void fSetDiscoveredDir( const tFilePathPtr& discDir ) { mDiscoveredDir = discDir; }
		void fSetGuid( u32 newGuid ) { mGuid = newGuid; }

		u32 fGetVariationId( tTileTypes requestedType, const tFilePathPtr& modelPath ) const;

		std::string				fName( ) const { return mName; }
		std::string				fFamily( ) const { return mFamily; }
		tFilePathPtr			fResDir( ) const { return mResDir; }
		tFilePathPtr			fDiscoveredDir( ) const { return mDiscoveredDir; }
		tResourcePtr			fGetTileTexture( tTileTypes type, u32 variationId ) const;
		tResourcePtr			fGetDefaultTileTexture( tTileTypes type ) const;
		tSgFileRefEntityPtr		fGetTileModel( tTileTypes type, u32 variationId ) const;
		tFilePathPtr			fGetTileModelPath( tTileTypes type, u32 variationId ) const;
		Math::tVec2u			fGetTileDims( tTileTypes type, u32 variationId ) const;
		u32						fGuid( ) const { return mGuid; }

		b32 fHasRandomTiles( tTileTypes requestedType ) const;
		tFilePathPtr fConstructTileModelPath( const Tieml::tTile& tile ) const;
		const tEditableTileDef* fGetRandomTileDef( tTileTypes requestedType ) const;

		u32 fNumTileTypeLists( ) const { return mTileTypesData.fCount( ); }
		tEditableTileTypeList* fTileTypeList( tTileTypes type ) const { return mTileTypesData[ type ]; }

		void fSerialize( ) const;
		void fDeserialize( const TileDb::tFile& set, const tResourceDepotPtr& resourceDepot );

		void fAddTileDef( const tResourceDepotPtr& resourceDepot, tTileTypes type, std::string modelName, const tFilePathPtr& tileSetDir, const tFilePathPtr& modelPath, const tFilePathPtr& tilePath );
		b32 fHasTileDef(  tTileTypes type, const tFilePathPtr& modelPath );
		void fRemoveDeletedTileDefs( const tFilePathPtrList& discoveredFiles );
	};

	///
	/// \class tEditableTileDb
	/// \brief 
	class tools_export tEditableTileDb
	{
		tGrowableArray< tEditableTileSet* > mTileSets;
		tEditableTileSetPigmentSet mPigments; // Using the container class to ease of movement later. (if necessary)
		tEditableScriptNodeDefSet mNodeDefs; // ditto

		tDynamicArray<tResourcePtr> mRandomTileResources;

	public:
		tEditableTileDb( );
		~tEditableTileDb( );

		// This will serialize out the individual tiledb files.
		void fSerializeTileSets( ) const;

		/// 
		/// \brief Deserializes a tile set. If useRepairDialog is off, it will log warnings without using the repair dialog. If
		/// resourceDepot is left null, it will not load the textures and models.
		b32 fDeserializeTileSets( const TileDb::tFile& file, b32 useRepairDialog, const tResourceDepotPtr& resourceDepot );

		void fSerializeTileSetPigments( Tieml::tFile& file ) const;
		void fDeserializeTileSetPigments( Tieml::tFile& file );
		void fClearTileSetPigments( );

		void fSerializeScriptNodeDefs( Tieml::tFile& file ) const;
		void fDeserializeScriptNodeDefs( Tieml::tFile& file, const tResourceDepotPtr& resourceDepot );
		void fClearScriptNodeDefs( );

		void fLoadRandomTileAssets( const tResourceDepotPtr& resourceDepot );
		tResourcePtr fRandomTileMarker( tTileTypes forThisType );

		void fRebuild( const tResourceDepotPtr& resourceDepot );

		// Tile set pigments
		tEditableTileSetPigment*		fAddEmptyPigment( );
		void							fDeletePigment( tEditableTileSetPigment* pigment );
		u32								fNumPigments( ) const;
		const tEditableTileSetPigment*	fPigmentByIdx( u32 idx ) const;
		tEditableTileSetPigment*		fPigmentByIdx( u32 idx );
		const tEditableTileSetPigment*	fPigmentByGuid( u32 guid )const ;

		tEditableScriptNodeDef*			fAddEmptyScriptNode( const tResourceDepotPtr& resourceDepot );
		void							fDeleteNode( tEditableScriptNodeDef* node );
		u32								fNumNodes( ) const;
		const tEditableScriptNodeDef*	fNodeByIdx( u32 idx ) const;
		tEditableScriptNodeDef*			fNodeByIdx( u32 idx );
		const tEditableScriptNodeDef*	fNodeByGuid( u32 guid )const ;

		void							fAddTileSet( const tResourceDepotPtr& resourceDepot, const tFilePathPtr& dirPath, const char* family );
		u32								fNumTileSets( ) const { return mTileSets.fCount( ); }
		tEditableTileSet*				fTileSet( u32 idx ) { return mTileSets[idx]; }
		const tEditableTileSet*			fTileSet( u32 idx ) const { return mTileSets[idx]; }
		tEditableTileSet*				fTileSetByGuid( u32 guid ) const;

		tFilePathPtr fConstructTileModelPath( const Tieml::tTile& tile );
		void fSeedRandomTiles( );
		void fBakeRandomTileForEditor( tEditableTileEntityPtr& tile ) const;

		tFilePathPtr fGetScriptPath( u32 scriptGuid );

		tEditableTileEntityPtr fDeserializeTile( const Tieml::tTile& tile ) const;

	private:
		void fAddMissingTileDefs( const tResourceDepotPtr& resourceDepot, u32 i, tFilePathPtrList& files );
		void fRemoveDeletedTileDefs( u32 i, const tFilePathPtrList& files );

		u32 fGetFreeGuid( ) const;

		const tEditableTileSet* fTileSetFromSerializedTile( const Tieml::tTile& tile ) const;
		const tEditableTileSet* fTileSetFromEditableTile( const tEditableTileEntityPtr& tile ) const;
	};
}

#endif // __tEditableTileDb__
