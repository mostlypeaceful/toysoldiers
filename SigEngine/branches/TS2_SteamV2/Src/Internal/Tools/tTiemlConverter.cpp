#include "ToolsPch.hpp"
#include "tTiemlConverter.hpp"
#include "tLoadInPlaceSerializer.hpp"

namespace Sig
{

	tTiemlConverter::tTiemlConverter( )
	{
	}

	tTiemlConverter::~tTiemlConverter( )
	{
	}

	b32 tTiemlConverter::fLoadTiemlFile( const tFilePathPtr& TiemlFilePath, const tFilePathPtr& outputResourcePath )
	{
		tGrowableArray< TileDb::tFile > loadedFiles;
		if( TileDb::tFile::fLoadTileDbs( loadedFiles ) )
		{
			for( u32 i = 0; i < loadedFiles.fCount( ); ++i )
			{
				if( !mDatabase.fDeserializeTileSets( loadedFiles[i], NULL, tResourceDepotPtr( ) ) )
					return false;
			}
		}

		if( mTiemlFile.fLoadXml( TiemlFilePath ) )
		{
			mDatabase.fDeserializeTileSetPigments( mTiemlFile );
			mDatabase.fDeserializeScriptNodeDefs( mTiemlFile, tResourceDepotPtr( ) );
			mResourcePath = outputResourcePath;
			return true;
		}

		return false;
	}

	b32 tTiemlConverter::fConvertPlatformCommon( )
	{
		mDatabase.fSeedRandomTiles( );

		const Tieml::tFile& tiemlFile = fTiemlFile( );
		tGrowableArray<tEntityDef*> convertedObjects;
		for( u32 i = 0; i < tiemlFile.mTiles.fCount( ); ++i )
		{
			// convert object (virtually)
			tEntityDef* object = tiemlFile.mTiles[ i ].fCreateEntityDef( *this );
			if( object )
				convertedObjects.fPushBack( object );
		}

		// store objects
		mObjects.fNewArray( convertedObjects.fCount( ) );
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[ i ] = convertedObjects[ i ];

		// compute ray-casting total bounding volume
		mBounds.fInvalidate( );
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
		{
			if( mObjects[ i ]->mBounds.fIsValid( ) )
				mBounds |= mObjects[ i ]->mBounds.fTransform( mObjects[ i ]->mObjectToLocal );
		}

		// store pigment datas
		mTilePackage = new tTilePackage;
		tGrowableArray< u32 > relevantTileSetGuids;
		for( u32 i = 0; i < tiemlFile.mTilePigmentDefs.fCount( ); ++i )
		{
			const tEditableTileSetPigment* pigment = mDatabase.fPigmentByGuid( tiemlFile.mTilePigmentDefs[i].mGuid );
			tPigment newPigment;
			newPigment.mGuid = pigment->fGuid( );

			for( u32 j = 0; j < pigment->fNumTileSetGuids( ); ++j )
			{
				tPigmentChoice newChance;
				newChance.mChanceWeight = pigment->fTileSetChance( j );
				newChance.mGuid = pigment->fTileSetGuid( j );

				relevantTileSetGuids.fFindOrAdd( newChance.mGuid );

				newPigment.mPigmentChoices.fPushBack( newChance );
			}

			mTilePackage->mPigments.fPushBack( newPigment );
		}

		// store relevant tile sets
		for( u32 i = 0; i < relevantTileSetGuids.fCount( ); ++i )
		{
			tTileSet newSet;
			newSet.mGuid = relevantTileSetGuids[i];

			// TODO: copy out individual tile defs and resources?
		}

		return true;
	}

	b32 tTiemlConverter::fConvertPlatformSpecific( tPlatformId pid )
	{
		return true;
	}

	b32 tTiemlConverter::fOutput( tFileWriter& ofile, tPlatformId pid )
	{
		sigassert( ofile.fIsOpen( ) );

		// set the binary file signature
		tBinaryFileBase::fSetSignature( pid, Rtti::fGetClassId<tSceneGraphFile>( ), tSceneGraphFile::cVersion );

		// output
		tLoadInPlaceSerializer ser;
		ser.fSave( static_cast<tSceneGraphFile&>( *this ), ofile, pid );

		return true;
	}

	tFilePathPtr tTiemlConverter::fConstructModelFilePath( const Tieml::tTile& tile )
	{
		return mDatabase.fConstructTileModelPath( tile );
	}

	tFilePathPtr tTiemlConverter::fGetScriptPath( u32 scriptGuid )
	{
		return mDatabase.fGetScriptPath( scriptGuid );
	}
}
