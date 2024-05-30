//------------------------------------------------------------------------------
// \file tTileCanvas.cpp - 26 Jul 2012
// \author ksorgetoomey
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tTileCanvas.hpp"

namespace Sig
{
	namespace
	{
		u32 gSpecificSeed = 0;
		b32 gUseSpecificSeed = false;

		tResourcePtr gOverrideTileset;
	}

	//------------------------------------------------------------------------------
	// tTileCanvasEntityDef
	//------------------------------------------------------------------------------
	void tTileCanvasEntityDef::fSeedRandom( u32 seed )
	{
		gUseSpecificSeed = true;
		gSpecificSeed = seed;
	}

	void tTileCanvasEntityDef::fClearSeed()
	{
		gUseSpecificSeed = false;
	}

	tResourcePtr tTileCanvasEntityDef::fTilesetOverride()
	{
		return gOverrideTileset;
	}

	void tTileCanvasEntityDef::fSetTilesetOverride( const tResourcePtr& newTileset )
	{
		gOverrideTileset = newTileset;
	}

	tTileCanvasEntityDef::tTileCanvasEntityDef()
	{
	}

	tTileCanvasEntityDef::tTileCanvasEntityDef( tNoOpTag )
		: tEntityDef( tNoOpTag() )
		, mTileDefs( tNoOpTag() )
	{
	}

	void tTileCanvasEntityDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		tTilePackage* tileSet = NULL;

		// If there's no overriding tileset specified by game code, use the default
		// that is on the parent scene graph file.
		if( gOverrideTileset.fNull() )
		{
			// Try to retrieve the default tile set.
			// NOTE: the parent here is a SceneRefEntity. It must be.
			tSceneRefEntity* sceneRefParent = params.mParent.fStaticCast< tSceneRefEntity >();

			const tSceneGraphFile* sgFile = sceneRefParent->fSgResource()->fCast< tSceneGraphFile >();
			if( !sgFile )
			{
				log_warning("TileCanvasEntityDef was found inside a parent that wasn't a scene ref.");
				return;
			}

			if( sgFile->mTilePackage.fTreatAsObject().fNull() )
			{
				log_warning("TilePackage not found on TileCanvasEntityDef's parent scene ref entity.");
				return;
			}

			tileSet = sgFile->mTilePackage.fTreatAsObject()->fCast< tTilePackage >();
			if( !tileSet )
			{
				log_warning("TilePackage failed to cast to tTilePackage." );
				return;
			}
		}
		else
		{
			// Use the override if there is one.
			tileSet = gOverrideTileset->fCast< tTilePackage >();
		}

		if( !tileSet )
		{
			log_warning("After strenous efforts, the TilePackage could not be located.");
			return;
		}

		// Use a specific number or whatever's lying around.
		if( gUseSpecificSeed )
		{
			tileSet->mGenerator.fTreatAsObject( ) = tRandom( gSpecificSeed );
			tileSet->mUseSpecificGen = true;
		}
		else
			tileSet->mUseSpecificGen = false; // Just to be sure

		tTileCanvasEntity* entity = NEW_TYPED(tTileCanvasEntity)( mBounds );

		entity->fMoveTo( mObjectToLocal );
		entity->fSpawn( params.mParent );

		for( u32 i = 0; i < mTileDefs.fCount(); ++i )
		{
			tTileEntityDef* tileDef = (tTileEntityDef*)mTileDefs[i].fRawPtr();

			// Set the tileset this tile will use to pick a random model.
			tileDef->mTileSet.fTreatAsObject( ) = tileSet;
			tileDef->fCollectEntities( tCollectEntitiesParams( *entity, params.mCreationFlags | fCreationFlags( ) ) );

			// Record any type of connection.
			if( tileDef->mTileType == cWallDoor || tileDef->mTileType == cFloorDoor || tileDef->mTileType == cWallExit )
			{
				tEntityPtr child = entity->fChild( entity->fChildCount()-1 );
				entity->mConnectionTiles.fPushBack( tTileEntityPtr( child->fDynamicCast< tTileEntity >() ) );
			}
			// Specifically record exit tiles.
			if( tileDef->mTileType == cWallExit )
			{
				tEntityPtr child = entity->fChild( entity->fChildCount()-1 );
				entity->mExitTiles.fPushBack( tTileEntityPtr( child->fDynamicCast< tTileEntity >() ) );
			}
		}

		fApplyPropsAndSpawnWithScript( *entity, tCollectEntitiesParams( params.mParent, params.mCreationFlags | fCreationFlags( ) ) );
	}

	//------------------------------------------------------------------------------
	// tTileCanvasEntity
	//------------------------------------------------------------------------------
	tTileCanvasEntity::tTileCanvasEntity( const Math::tAabbf& aabb )
		: tSpatialEntity( aabb )
	{
	}

	void tTileCanvasEntity::fExportScriptInterface( tScriptVm& vm )
	{
	}
}
