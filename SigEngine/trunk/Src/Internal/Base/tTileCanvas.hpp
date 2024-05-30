//------------------------------------------------------------------------------
// \file tTileCanvas.hpp - 26 Jul 2012
// \author ksorgetoomey
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTileCanvas__
#define __tTileCanvas__
#include "tSpatialEntity.hpp"
#include "tEntityDef.hpp"
#include "tTileEntity.hpp"

namespace Sig
{

	///
	/// \class tTileCanvasEntityDef
	/// \brief Contains a list of tiles that are already oriented correctly in
	/// space. This used to be able to have a specific tile set specified on it
	/// but that was abandoned and the mTileSetGuid hasn't been cleared out yet.
	class base_export tTileCanvasEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tTileCanvasEntityDef, 0x982D49BC );

	public:
		tDynamicArray< tLoadInPlacePtrWrapper<tTileEntityDef> > mTileDefs;
		u32 mTileSetGuid; // UNUSED for now. Used to look inside list of the Tile Package to find tilset.

		// This will seed all future tile canvas entity defs at the time they call fCollectEntities();
		static void fSeedRandom( u32 seed );
		static void fClearSeed();

		// This will set the tileset for all canvas entities created after.
		static tResourcePtr fTilesetOverride();
		static void fSetTilesetOverride( const tResourcePtr& newTileset );

	public:
		tTileCanvasEntityDef();
		tTileCanvasEntityDef( tNoOpTag );

		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;

		virtual b32 fHasRenderableBounds() const { return true; }
	};

	///
	/// \class tTileCanvasEntity
	/// \brief An actual runtime game-side entity representing a collection of tiles.
	/// Stores all the exit tile spaces that can connect outside this canvas.
	class base_export tTileCanvasEntity : public tSpatialEntity
	{
		define_dynamic_cast( tTileCanvasEntity, tSpatialEntity );

		tGrowableArray< tTileEntityPtr > mExitTiles;
		tGrowableArray< tTileEntityPtr > mConnectionTiles;

	public:
		explicit tTileCanvasEntity( const Math::tAabbf& aabb );

		tGrowableArray< tTileEntityPtr > fGetExits() const { return mExitTiles; }
		tGrowableArray< tTileEntityPtr > fGetConnections() const { return mConnectionTiles; }

		static void fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tTileCanvasEntity );
}

#endif //__tTileCanvas__
