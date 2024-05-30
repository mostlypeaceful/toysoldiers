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
#include "tWxProgressDialog.hpp"

class tWxProgressDialog;

namespace Sig
{
	class tSigTileMainWindow;
	class tTiledmlTree;
	class tTiledmlPropPanel;
	class tTilePaintPanel;
	class tEditableTiledml;
	class tToolsGuiMainWindow;

	///
	/// \class tEditableTileData
	/// \brief 
	struct tools_export tEditableTileDef
	{
	private:
		b32 mAltered;

	public:
		tEditableTileDef( ) : mRandomWeight( 0.f ), mDims( 1, 1 ), mAltered( false ) { }
		tEditableTileDef( const tEditableTileDef& copyFrom );
		~tEditableTileDef( );

		// IMPORTANT NOTE: if these are modified directly, it won't be caught by the
		// save checking in the editor panel.
		std::string mShortName;
		std::string mIdString; //!< By which the specific tile is identified when tilesets are swapped
		tFilePathPtr mTileAssetPath; //!< The 2D tile texture to show in 2D mode
		tFilePathPtr mModelAssetPath; //!< stored relative to the tile set's dir
		tResourcePtr mTileResource;
		tResourcePtr mModelResource;
		tSgFileRefEntityPtr mModelEntity;

		f32 mRandomWeight;

		Math::tVec2u mDims;

		b32 fAltered( ) const { return mAltered; }
		b32 fIsRandomable( ) const { return mRandomWeight > 0.f; }

		// IMPORTANT NOTE CONT'D: these are the only modifiers that track with saving.
		void fLoadAndSetTexture( const tFilePathPtr& filePath, const tResourceDepotPtr& resourceDepot );
		void fLoadAndSetModel( const tFilePathPtr& modelPath, const tFilePathPtr& filePath, const tResourceDepotPtr& resourceDepot );

		void fUnloadResources( );
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
		tEditableTileTypeList( const tEditableTileTypeList& copyFrom );
		~tEditableTileTypeList( );

		void fClear( );

		void fLoadDefaultTile( const tResourceDepotPtr& resourceDepot, const tFilePathPtr& defaultTilePath );

		void			fSetTypeEnum( tTileTypes newEnum ) { mTypeEnum = newEnum; }
		tTileTypes		fTypeEnum( ) const { return mTypeEnum; }
		void			fSetTypeName( const std::string& newName ) { mTypeName = newName; }
		std::string		fTypeName( ) const { return mTypeName; }
		tResourcePtr	fDefaultTileResource( ) const { return mDefaultTileResource; }
		tFilePathPtr	fDefaultTilePath( ) const { return mDefaultTileAssetPath; }

		u32							fNumTileDefs( ) const { return mTileDefs.fCount( ); }
		const tEditableTileDef*		fTileDef( u32 idx ) const { return mTileDefs[ idx ]; }
		tEditableTileDef*			fTileDef( u32 idx ) { return mTileDefs[ idx ]; }
		const tEditableTileDef*		fTileDef( std::string id ) const;
		tEditableTileDef*			fTileDef( std::string id );
		void						fAddTileDef( tEditableTileDef* newTileData ) { mTileDefs.fPushBack( newTileData ); }
		void						fEraseTileDef( u32 idx );

		b32							fHasRandomTiles( ) const;
		const tEditableTileDef*		fGetRandomTileDef( ) const;

		b32 fHasModel( const tFilePathPtr& modelPath ) const;

		Tiledml::tTileTypeList fSerialize( ) const;
	};

	///
	/// \class tEditableTileSet
	/// \brief Contains run-time editable tile set data.
	class tools_export tEditableTileSet
	{
		friend class tEditableTiledml;

		u32				mGuid; // TODO: remove?
		std::string		mFileName;
		tFilePathPtr	mResDir;
		tFilePathPtr	mDiscoveredDir; //!< Stupid but necessary (TODO: explain)
		tGrowableArray< tEditableTileTypeList* > mTileTypesData;
		b32				mLoaded;
		b32				mAltered;

	public:
		tEditableTileSet( );
		tEditableTileSet( const tEditableTileSet& copyFrom );
		~tEditableTileSet( );

		b32 fAltered( ) const;
		void fSaved( ) { mAltered = false; }
		b32 fLoaded( ) const { return mLoaded; }
		void fLoad( const tResourceDepotPtr& resourceDepot, wxProgressDialog* optionalDialog = NULL, s32 optDialogProgress = 0 );
		void fLoadDefaultTileTextures( const tResourceDepotPtr& resourceDepot );

		void fExtractTextures( const tEditableTileSet& copyFrom );
		void fExtractResources( const tEditableTileSet& copyFrom, const tResourceDepotPtr& resourceDepot );

		void fSetFileName( const std::string& newName ) { mFileName = newName; mAltered = true; }
		void fSetResDir( const tFilePathPtr& resDir ) { mResDir = resDir; mAltered = true; }
		void fSetDiscoveredDir( const tFilePathPtr& discDir ) { mDiscoveredDir = discDir; mAltered = true; }
		void fSetGuid( u32 newGuid ) { mGuid = newGuid; mAltered = true; }

		u32 fGetVariationIdx( tTileTypes requestedType, const tFilePathPtr& modelPath ) const;

		std::string				fFileName( ) const { return mFileName; }
		tFilePathPtr			fResDir( ) const { return mResDir; }
		tFilePathPtr			fDiscoveredDir( ) const { return mDiscoveredDir; }
		tResourcePtr			fGetTileTexture( tTileTypes type, u32 variationId ) const;
		tResourcePtr			fGetDefaultTileTexture( tTileTypes type ) const;
		tSgFileRefEntityPtr		fGetTileModel( tTileTypes type, u32 variationId ) const;
		tFilePathPtr			fGetTileModelPath( tTileTypes type, u32 variationId ) const;
		Math::tVec2u			fGetTileDims( tTileTypes type, u32 variationId ) const;
		u32						fGuid( ) const { return mGuid; }
		tFilePathPtr			fFilePath( ) const;

		b32 fHasRandomTiles( tTileTypes requestedType ) const;
		const tEditableTileDef* fGetRandomTileDef( tTileTypes requestedType ) const;
		const tEditableTileDef* fGetSpecificTileDef( tTileTypes requestedType, const std::string& idString ) const;

		u32 fNumTileTypeLists( ) const { return mTileTypesData.fCount( ); }
		tEditableTileTypeList* fTileTypeList( tTileTypes type ) const { return mTileTypesData[ type ]; }

		void fSerialize( ) const;
		void fDeserialize( const Tiledml::tFile& set );

		void fBuild( const tFilePathPtr& dirPath );
		void fRebuild( const tResourceDepotPtr& resourceDepot );

		void fAddTileDef( tTileTypes type, std::string modelName, const tFilePathPtr& tileSetDir, const tFilePathPtr& modelPath, const tFilePathPtr& tilePath );
		b32 fHasTileDef(  tTileTypes type, const tFilePathPtr& modelPath );
		void fRemoveDeletedTileDefs( const tFilePathPtrList& discoveredFiles );

		static Math::tVec2u fExtractDimxDimInfo( const char* modelName );
	};

	///
	/// \class tEditableTiledml
	/// \brief An editable database of tiles.
	class tools_export tEditableTiledml
	{
		tGrowableArray< tEditableTileSet* > mTileSets;
		tDynamicArray<tResourcePtr> mRandomTileResources;

	public:
		tEditableTiledml( );
		~tEditableTiledml( );

		// TODO: refactor intelligently
		static tEditableTiledml* fHackyGlobal();

		// This will serialize out the individual tiledml files.
		void fSerializeTileSets( ) const;

		/// 
		/// \brief Deserializes a tile set. If useRepairDialog is off, it will log warnings without using the repair dialog. If
		/// resourceDepot is left null, it will not load the textures and models.
		b32 fDeserializeTileSet( const Tiledml::tFile& file, b32 useRepairDialog );
		void fLoadAllTileSets( const tResourceDepotPtr& resourceDepot );

		void fLoadTileAssets( const tResourceDepotPtr& resourceDepot );
		tResourcePtr fTileMarker( tTileTypes forThisType );

		tEditableTileEntityPtr fGetRandomizedTile( tEditableObjectContainer& container, tTileTypes requestedType );
		tEditableTileEntityPtr fGetSpecificTile( tEditableObjectContainer& container, tTileTypes requestedType, const tPair<u32,u32>& tileDefKey );

		void							fAddTileSet( const tResourceDepotPtr& resourceDepot, const tFilePathPtr& dirPath, const tEditableTileSet* texturesSet = NULL );
		u32								fNumTileSets( ) const { return mTileSets.fCount( ); }
		tEditableTileSet*				fTileSet( u32 idx ) { return mTileSets[idx]; }
		const tEditableTileSet*			fTileSet( u32 idx ) const { return mTileSets[idx]; }
		tEditableTileSet*				fTileSetByGuid( u32 guid ) const;
		tEditableTileSet*				fTileSetByFilePath( const tFilePathPtr& filePath ) const;
		tEditableTileSet*				fTileSetByFileName( const wxString& fileName ) const;
		b32								fAllTileSetsLoaded( ) const;
		b32								fAnyTileSetLoaded( ) const;

		wxArrayString fGetLoadedTileSetsForDisplay( ) const;

	private:
		u32 fGetFreeGuid( ) const;
	};
}

#endif // __tEditableTileDb__
