//------------------------------------------------------------------------------
// \file tTileEntity.hpp - 24 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTileEntity__
#define __tTileEntity__
#include "tSceneRefEntity.hpp"
#include "tTilePackage.hpp"

namespace Sig
{
	enum tTileTypes
	{
		cFloor,
		cWall,
		cNiche,
		cCorner,

		cNumBasicTypes,

		cUniques = cNumBasicTypes,
		cWallDoor,	// Todo: these should probably just be a property you can flag any tile with
		cFloorDoor,
		cPropCeiling,
		cPropFloor,
		cWallExit, // TODO: ditto with exit flags

		cNumTileTypes,
	};

	///
	/// \class tTileEntityDef
	/// \brief 
	class base_export tTileEntityDef : public tSceneRefEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tTileEntityDef, 0xD07A0032 );

	public:
		tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceResourcePtr> > mTileScripts; // Defunct
		u32 mTileType; // Floor, wall, etc

		// Exists when the tile is a specific type. This string is matched to the currently
		// loaded tileset and if the ID exists in that tileset, a tile/prop is spawned.
		tLoadInPlaceStringPtr* mIdString;

		// Handle to the current tileset this
		tLoadInPlaceRuntimeObject< tTilePackage* > mTileSet;

	public:
		tTileEntityDef( );
		tTileEntityDef( tNoOpTag );

		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;

		virtual b32 fHasRenderableBounds( ) const { return true; }
	};

	///
	/// \class tTileEntity
	/// \brief 
	class base_export tTileEntity : public tSceneRefEntity
	{
		define_dynamic_cast( tTileEntity, tSceneRefEntity );
		
		const tTileEntityDef* mEntityDef;
		const tTilePackage::tTileDef* mTileDef;

	public:
		explicit tTileEntity( const tTileEntityDef* entityDef, const tResourcePtr& sgResource, const tTilePackage::tTileDef* tileDef, const tEntity* proxy = 0 );

		// An attachment matrix whose translation is the the attachment point on the edge
		// of the tile and the z-axis is aligned to face "in" toward the center of this tile.
		Math::tMat3f fGetAttachmentXform( b32 flipIt ) const;

		tTileTypes fGetType() const { return (tTileTypes)mEntityDef->mTileType; }

		static void fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tTileEntity );
}

#endif //__tTileEntity__
