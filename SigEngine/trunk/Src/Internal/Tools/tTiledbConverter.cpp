#include "ToolsPch.hpp"
#include "tTiledbConverter.hpp"
#include "tFileWriter.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "tSceneGraphFile.hpp"
#include "Editor/tEditableTileDb.hpp"
#include "Editor/tEditableTileCanvas.hpp"

namespace Sig
{
	b32 tTiledmlConverter::fLoadTiledbFile( const tFilePathPtr& path )
	{
		return mTiledb.fLoadXml( path );
	}

	b32 tTiledmlConverter::fConvertPlatformCommon( )
	{
		// First by type.
		mTileDefsByType.fNewArray( mTiledb.mTileTypesList.fCount() );
		for( u32 i = 0; i < mTiledb.mTileTypesList.fCount(); ++i )
		{
			Tiledml::tTileTypeList& typesFromFile = mTiledb.mTileTypesList[i];

			tTileType newType;
			newType.mTotalWeight = 0.f;

			newType.mTileDefs.fNewArray( typesFromFile.mTileData.fCount() );
			for( u32 j = 0; j < typesFromFile.mTileData.fCount(); ++j )
			{
				Tiledml::tTileData& dataFromFile = typesFromFile.mTileData[j];

				newType.mTotalWeight += 1.f; // Save a weight and add it here.
				newType.mTileDefs[j].mWeight = 1.f;
				tFilePathPtr fullPath = tFilePathPtr::fConstructPath( mTiledb.mResDir, dataFromFile.mModelFilePath );
				tFilePathPtr fullTexPath = dataFromFile.mTileFilePath;
				newType.mTileDefs[j].mSigml = fAddLoadInPlaceResourcePtr( tResourceId::fMake<tSceneGraphFile>( fullPath ) );
				newType.mTileDefs[j].mTexture = fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tTextureFile>( fullTexPath ) );
				if( dataFromFile.mIdString.fExists() )
					newType.mTileDefs[j].mIdString = fAddLoadInPlaceStringPtr( dataFromFile.mIdString.fCStr( ) );
				newType.mTileDefs[j].mDims = tEditableTileSet::fExtractDimxDimInfo( dataFromFile.mModelFilePath.fCStr( ) );
			}

			mTileDefsByType[i] = newType;
		}

		// Save the guid and all the lists to the SceneGraphFile.
		mGuid = mTiledb.mGuid;

		return true;
	}

	b32 tTiledmlConverter::fConvertPlatformSpecific( tPlatformId pid )
	{
		return true;
	}

	void tTiledmlConverter::fOutput( tFileWriter& ofile, tPlatformId pid )
	{
		sigassert( ofile.fIsOpen( ) );

		// set the binary file signature
		this->fSetSignature<tTilePackage>( pid );

		// output
		tLoadInPlaceSerializer ser;
		ser.fSave( static_cast<tTilePackage&>( *this ), ofile, pid );
	}

}

