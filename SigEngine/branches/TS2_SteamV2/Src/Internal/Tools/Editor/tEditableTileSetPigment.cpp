//------------------------------------------------------------------------------
// \file tEditableTileSetPigment.cpp - 03 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tEditableTileSetPigment.hpp"
#include "Guid.hpp"
#include "tEditableTileEntity.hpp"
#include "tEditableTileDb.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tTileSetPalette
	//------------------------------------------------------------------------------
	tEditableTileSetPigment::tEditableTileSetPigment( tEditableTileSetPigmentSet* parent, std::string name, u32 guid, const Math::tVec4f& color )
		: mParent( parent )
		, mName( name )
		, mGuid( guid )
		, mColorRgba( color )
		, mTileHeight( 0.f )
		, mTileSize( 20.f )
	{
		mSpecificModels.fSetCount( cNumTileTypes );
		mHasRandom.fResize( cNumTileTypes );
		for( u32 i = 0; i < cNumTileTypes; ++i )
			mHasRandom[i] = false;
	}

	u32 tEditableTileSetPigment::fColorU32( ) const
	{
		u32 flatColor = 0;
		flatColor += mColorRgba.x * 0x00FF0000;
		flatColor += mColorRgba.y * 0x0000FF00;
		flatColor += mColorRgba.z * 0x000000FF;
		flatColor += mColorRgba.w * 0xFF000000;

		return flatColor;
	}

	void tEditableTileSetPigment::fAddTileSetGuid( u32 tileSetGuid )
	{
		tTileSetRef newTileSet;
		newTileSet.mGuid = tileSetGuid;
		mTileSetRefs.fFindOrAdd( newTileSet );

		fAdditiveCheckForValidSpecificModels( tileSetGuid );
	}

	void tEditableTileSetPigment::fDeleteTileSetGuid( u32 idx )
	{
		mTileSetRefs.fEraseOrdered( idx );
		fFullRebuildValidSpecificModels( );
	}

	u32 tEditableTileSetPigment::fGetRandomTileSetGuid( )
	{
		if( mSelectedTileSet == -1 )
		{
			fSeedRandom( );
			if( mSelectedTileSet == -1 )
			{
				log_warning( 0, "fGetRandomTileSetGuid failed to select a set randomly. Picking zero set." );
				return mTileSetRefs[0].mGuid;
			}
		}

		return mTileSetRefs[ mSelectedTileSet ].mGuid;
	}

	s32 tEditableTileSetPigment::fSelectRandomTileSet( ) const
	{
		if( mTileSetRefs.fCount( ) == 0 )
			return NULL;

		f32 totalWeight = 0.f;
		for( u32 i = 0; i < mTileSetRefs.fCount( ); ++i )
			totalWeight += mTileSetRefs[i].mRandomWeight;

		if( totalWeight == 0.f )
		{
			log_warning( 0, "Tried to randomize a tile but there were no random tiles available." );
			return 0;
		}

		f32 targetWeight =  tRandom::fSubjectiveRand( ).fFloatZeroToValue( totalWeight );
		for( u32 i = 0; i < mTileSetRefs.fCount( ); ++i )
		{
			if( targetWeight <= mTileSetRefs[i].mRandomWeight )
				return i;

			targetWeight -= mTileSetRefs[i].mRandomWeight;
		}

		return mTileSetRefs.fCount( ) - 1;
	}

	void tEditableTileSetPigment::fSeedRandom( )
	{
		mSelectedTileSet = fSelectRandomTileSet( );
	}

	u32 tEditableTileSetPigment::fGetTileSetGuidAuto( )
	{
		return fIsSpecificTileSet( ) ? fGetSpecificTileSetGuid( ) : fGetRandomTileSetGuid( );
	}

	void tEditableTileSetPigment::fOnTileDbChanged( )
	{
		fFullRebuildValidSpecificModels( );
	}

	void tEditableTileSetPigment::fSerialize( Tieml::tTilePigmentDef& palette )
	{
		palette.mColorRgba = mColorRgba;
		palette.mGuid = mGuid;
		palette.mName = mName;
		palette.mSize = mTileSize;
		palette.mHeight = mTileHeight;

		for( u32 i = 0; i < mTileSetRefs.fCount( ); ++i )
			palette.mTileSetGuids.fPushBack( mTileSetRefs[i].mGuid );
	}

	void tEditableTileSetPigment::fDeserialize( Tieml::tTilePigmentDef& palette )
	{
		mColorRgba = palette.mColorRgba;
		mGuid = palette.mGuid;
		mName = palette.mName;
		mTileSize = palette.mSize;
		mTileHeight = palette.mHeight;

		for( u32 i = 0; i < palette.mTileSetGuids.fCount( ); ++i )
		{
			if( !mParent->fDatabase( )->fTileSetByGuid( palette.mTileSetGuids[i] ) )
			{
				// TODO: handle this gracefully
				log_warning( 0, "Palette contains tile set GUID that doesn't exist anymore." );
				continue;
			}

			fAddTileSetGuid( palette.mTileSetGuids[i] );
		}
	}

	tEditableTileEntityPtr tEditableTileSetPigment::fDeserializeTile( const Tieml::tTile& tile ) const
	{
		tTileTypes type = (tTileTypes)tile.mTileType;
		
		tResourcePtr texture = mParent->fDatabase( )->fRandomTileMarker( type );
		tSgFileRefEntityPtr model;
		tFilePathPtr modelPath;
		Math::tVec2u dims( 1, 1 );
		if( tile.mRandType == cNotRandom || tile.mSpecificModel )
		{
			tEditableTileSet* tileSet = mParent->fDatabase( )->fTileSetByGuid( mTileSetRefs[0].mGuid );
			const u32 variationId = tileSet->fGetVariationId( type, tile.mModelPath );

			texture = tileSet->fGetTileTexture( type, variationId );
			model = tileSet->fGetTileModel( type, variationId );
			modelPath = tile.mModelPath;
			dims = tileSet->fGetTileDims( type, variationId );
		}

		tEditableTileEntity* newTile = new tEditableTileEntity( 
			type, 
			fGuid( ),
			texture, 
			model,
			modelPath,
			tile.mXform,
			fColorU32( ),
			mTileSize,
			dims,
			(tRandStage)tile.mRandType,
			tile.mSpecificTileSet,
			tile.mSpecificModel ); 

		tEditableTileEntityPtr returnTile( newTile );

		if( tile.mRandType != cNotRandom )
			mParent->fDatabase( )->fBakeRandomTileForEditor( returnTile );

		return returnTile;
	}

	void tEditableTileSetPigment::fUpdatePlacedTile( tEditableTileEntityPtr& tile ) const
	{
		tile->fSetColor( fColorU32( ) );
		tile->fSetHeight( mTileHeight );
		tile->fSetSize( mTileSize );
	}

	void tEditableTileSetPigment::fAddFirstSpecificModels( tEditableTileSet* tileSet )
	{
		// This is the initially added tile set. Copy everything over.
		for( u32 type = 0; type < cNumTileTypes; ++type )
		{
			tEditableTileTypeList* typeList = tileSet->fTileTypeList( (tTileTypes)type );
			tModelList& thisSpecModList = mSpecificModels[ type ];

			mHasRandom[ type ] = typeList->fHasRandomTiles( );

			for( u32 i = 0; i < typeList->fNumTileDefs( ); ++i )
				thisSpecModList.fPushBack( typeList->fTileDef( i )->mModelAssetPath );
		}

	}

	void tEditableTileSetPigment::fAdditiveCheckForValidSpecificModels( u32 newTileSetGuid )
	{
		tEditableTileSet* tileSet = mParent->fDatabase( )->fTileSetByGuid( newTileSetGuid );

		if( mTileSetRefs.fCount( ) == 1 )
		{
			fAddFirstSpecificModels( tileSet );
			return;
		}

		// Filter existing models by what's not in the new set.
		for( u32 type = 0; type < cNumTileTypes; ++type )
		{
			tEditableTileTypeList* typeList = tileSet->fTileTypeList( (tTileTypes)type );
			tModelList& thisSpecModList = mSpecificModels[ type ];
			
			// Only potentially disqualify things that aren't already disqualified.
			if( mHasRandom[ type ] )
				mHasRandom[ type ] = typeList->fHasRandomTiles( );

			for( u32 i = 0; i < thisSpecModList.fCount( ); ++i )
			{
				if( !typeList->fHasModel( thisSpecModList[i] ) )
					thisSpecModList.fEraseOrdered( i-- );
			}
		}
	}

	void tEditableTileSetPigment::fFullRebuildValidSpecificModels( )
	{
		for( u32 i = 0; i < cNumTileTypes; ++i )
			mSpecificModels[i].fDeleteArray( );

		if( mTileSetRefs.fCount( ) == 0 )
			return;

		tEditableTileSet* tileSet = mParent->fDatabase( )->fTileSetByGuid( mTileSetRefs[0].mGuid );
		fAddFirstSpecificModels( tileSet );

		for( u32 i = 1; i < mTileSetRefs.fCount( ); ++i )
			fAdditiveCheckForValidSpecificModels( mTileSetRefs[i].mGuid );
	}

	//------------------------------------------------------------------------------
	// tTileSetPalettes
	//------------------------------------------------------------------------------
	void tEditableTileSetPigmentSet::fClear( )
	{
		mPigments.fDeleteArray( );
	}

	tEditableTileSetPigment* tEditableTileSetPigmentSet::fAddEmptyPigment( )
	{
		const u32 guid = fGetFreeGuid( );
		tEditableTileSetPigment* brush = new tEditableTileSetPigment( this, "new palette", guid, Math::tVec4f( 1.f, 1.f, 1.f, 1.f ) );
		mPigments.fPushBack( tTileSetPigmentPtr( brush ) );

		return brush;
	}

	void tEditableTileSetPigmentSet::fDeletePigment( tEditableTileSetPigment* brush )
	{
		const b32 success = mPigments.fFindAndEraseOrdered( brush );
		sigassert( success );
	}

	tEditableTileSetPigment* tEditableTileSetPigmentSet::fTileSetPigmentByGuid( u32 guid ) const
	{
		for( u32 i = 0; i < mPigments.fCount( ); ++i )
		{
			if( mPigments[i]->fGuid( ) == guid )
				return mPigments[i].fGetRawPtr( );
		}

		return NULL;
	}

	void tEditableTileSetPigmentSet::fOnTileDbChanged( )
	{
		for( u32 i = 0; i < mPigments.fCount( ); ++i )
			mPigments[i]->fOnTileDbChanged( );
	}

	void tEditableTileSetPigmentSet::fSerialize( Tieml::tFile& file ) const
	{
		file.mTilePigmentDefs.fSetCount( mPigments.fCount( ) );
		for( u32 i = 0; i < file.mTilePigmentDefs.fCount( ); ++i )
			mPigments[i]->fSerialize( file.mTilePigmentDefs[i] );
	}

	void tEditableTileSetPigmentSet::fDeserialize( Tieml::tFile& file )
	{
		mPigments.fSetCount( file.mTilePigmentDefs.fCount( ) );
		for( u32 i = 0; i < mPigments.fCount( ); ++i )
		{
			mPigments[i] = tTileSetPigmentPtr( new tEditableTileSetPigment( this, "ERROR", 0, Math::tVec4f::cOnesVector ) );
			mPigments[i]->fDeserialize( file.mTilePigmentDefs[i] );
		}
	}

	u32 tEditableTileSetPigmentSet::fGetFreeGuid( )
	{
		b32 everythingIsOk = false;
		u32 potentialGuid = fGenerateGuid( );
		while( !everythingIsOk )
		{
			everythingIsOk = true;
			potentialGuid = fGenerateGuid( );

			for( u32 i = 0; i < mPigments.fCount( ); ++i )
			{
				if( mPigments[i]->fGuid( ) == potentialGuid )
				{
					everythingIsOk = false; // everything is not ok!
					break;
				}
			}
		}

		return potentialGuid;
	}
}
