//------------------------------------------------------------------------------
// \file tTileCanvas.hpp - 02 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTileCanvas__
#define __tTileCanvas__
#include "tEditableTileEntity.hpp"
#include "tEditableScriptNodeEntity.hpp"

namespace Sig
{
	class tSigTileMainWindow;
	class tTileCanvas;
	typedef tRefCounterPtr< tTileCanvas > tTileCanvasPtr;
	namespace Tieml
	{
		class tFile;
	}
	enum tDirections;

	///
	/// \class tTileCanvas
	/// \brief The paintable canvas. Contains an array of tiles that track all the info needed.
	/// The tile canvas treats the space as a grid with x/y axes aligned with x/z world axes.
	class tTileCanvas : public tRefCounter, public tUncopyable
	{
		tSigTileMainWindow* mMainWindow;
		tGrowableArray< tGrowableArray< tEditableTileEntityPtr > > mTiles;
		s32 mWidth, mHeight;
		f32 mResolution; // Means 1 cell = mResolution many meters
		u32 mActionMask;

		tEditableTileEntityPtr mExistingTiles[3][3];

	public:
		tTileCanvas( tSigTileMainWindow* parent, u32 width, u32 height );
		~tTileCanvas( );
		
		void fClear( );

		void fSerialize( Tieml::tFile& file );
		void fDeserialize( Tieml::tFile& file );

		void fPaintTilePos( tEditableTileEntityPtr& paintTile, f32 x, f32 y );
		void fAutopaintTilePos( f32 x, f32 y );
		void fEraseTile( f32 x, f32 y );

		void fPaintScriptPos( tEditableScriptNodeEntityPtr& script, f32 x, f32 y );
		void fEraseScript( f32 x, f32 y );

		void fBakeRandomized( );

		void fShowTile( tEditableTileEntity* tile );
		void fHideTile( tEditableTileEntity* tile );

		f32 fTileHeight( f32 x, f32 y );

		void fRefreshTiles( );
		void fDeleteTilesWithGuid( b32 pigmentGuid );
		void fDeleteScriptNodesWithGuid( b32 nodeGuid );

		void fSetViewMode( b32 showModels );

		void fSnapToGrid( Math::tVec3f& pos, const Math::tVec2u& dims );

		Math::tAabbf fComputeBounding( );

	private:
		void fSerializeTile( Tieml::tFile& file, s32 x, s32 y );

		void fSetSize( u32 width, u32 height );

		void fWorldToGrid( const f32 worldX, const f32 worldY, s32& gridX, s32& gridY );
		void fGridToWorld( const s32 gridX, const s32 gridY, f32& worldX, f32& worldY );

		void fFillPixel( tEditableTileEntityPtr& paintTile, s32 gridX, s32 gridY );
		void fErasePixel( s32 gridX, s32 gridY );

		void fPaintScriptIdx( tEditableScriptNodeEntityPtr& script, s32 gridX, s32 gridY );
		void fEraseScriptIdx( s32 gridX, s32 gridY );

		tEditableTileEntityPtr fGetNullSpaceTileByDirection( tDirections dir, f32 x, f32 y );

		b32 fAnalyzeTile( tEditableTileEntityPtr& neededTile, s32 gridX, s32 gridY );

		void fPutExistingBack( u32 gridX, u32 gridY );
	};
}

#endif // __tTileCanvas__
