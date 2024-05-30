//------------------------------------------------------------------------------
// \file tEditableTileCanvas.hpp - 02 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tEditableTileCanvas__
#define __tEditableTileCanvas__
#include "tEditableTileEntity.hpp"
#include "tEditableTileDb.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tWxSlapOnButton.hpp"
#include "Gfx\tSolidColorGrid.hpp"


namespace Sig { 
namespace Sigml { 
	class tools_export tTileCanvasObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tTileCanvasObject, 0x56ECFBE3 );
		tObjectPtrArray mTiles;
		tObjectPtrArray mProps;
	public:
		tTileCanvasObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	private:
		tFilePathPtr fTileSetPath( ) const;
	};
} }


namespace Sig
{
	// Forward decs
	class tToolsGuiMainWindow;
	class tTilePaintPanel;
	class tEditableTileCanvas;
	class tEditableTileSet;
	typedef tRefCounterPtr< tEditableTileCanvas > tEditableTileCanvasPtr;
	enum tDirections;

	///
	/// \class Sig
	/// \brief Holds basic data about a tile set that a canvas holds.
	struct tTileSetDataObject
	{
		tFilePathPtr mFilePath;
		u32 mTileSetGuid;
		f32 mWeight;

		tTileSetDataObject( )
			: mTileSetGuid( ~0u )
			, mWeight( 1.f )
		{ }

		b32 operator==( const tTileSetDataObject& rhs ) const
		{
			return mFilePath == rhs.mFilePath && mTileSetGuid == rhs.mTileSetGuid && mWeight == rhs.mWeight;
		}

		template<class tSerializer>
		void fSerializeXml( tSerializer& s )
		{
			s( "FileName", mFilePath );
			s( "Guid", mTileSetGuid );
			s( "Weight", mWeight );
		}
	};

	///
	/// \class Sig
	/// \brief Exists to wrap an array of tTileSetDatas.
	struct tTileSetListObject
	{
		tGrowableArray< tTileSetDataObject > mTileSetDataList;

		b32 operator==( const tTileSetListObject& rhs ) const
		{
			if( mTileSetDataList.fCount() != rhs.mTileSetDataList.fCount() )
				return false;

			for( u32 i = 0; i < mTileSetDataList.fCount(); ++i )
			{
				if( !(mTileSetDataList[i] == rhs.mTileSetDataList[i]) )
					return false;
			}

			return true;
		}

		template<class tSerializer>
		void fSerializeXml( tSerializer& s )
		{
			s( "TileSetList", mTileSetDataList );
		}
	};

	enum tDirections
	{
		cNW,
		cN,
		cNE,
		cE,
		cSE,
		cS,
		cSW,
		cW,

		cNumDirections,
	};

	enum tViewModes
	{
		cTiles,
		cModels,

		cModeCount
	};

	///
	/// \class Sig
	/// \brief This is a defunct property that only exists now to convrt to the proper property
	class tools_export tEditablePropertyTileSetList : public tRawDataEditableProperty< tTileSetListObject >
	{
		implement_rtti_serializable_base_class( tEditablePropertyTileSetList, 0xB3DE901F );

	public:
		tEditablePropertyTileSetList( );
		explicit tEditablePropertyTileSetList( const std::string& name, const tTileSetListObject& data = tTileSetListObject() );

	protected:
		virtual void fCreateGui( tCreateGuiData& data ) { }
		virtual void fRefreshGui( ) { }
		virtual void fClearGui( ) { }

		virtual void fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const;
		virtual void fSetRawData( Rtti::tClassId cid, const void* src, u32 size );

		virtual tEditableProperty* fClone( ) const;

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tRawDataEditableProperty< tTileSetListObject >::fSerializeXml( s );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }

	private:
		void fCreateNewGui( const tTileSetDataObject& data );
	};

	///
	/// \class tEditableTileCanvas
	/// \brief The paintable canvas. Contains an array of tiles that track all the info needed.
	/// The tile canvas treats the space as a grid with x/y axes aligned with x/z world axes.
	class tools_export tEditableTileCanvas : public tEditableObject
	{
		define_dynamic_cast( tEditableTileCanvas, tEditableObject );

		tGrowableArray< tGrowableArray< tEditableTileEntityPtr > > mTiles;
		tGrowableArray< tEditableTileEntityPtr > mProps;
		tGrowableArray< tEditableTileEntityPtr > mReaddProps; // Props that need to be added to the world after the canvas comes back.

		b32 mViewModels;
		b32 mHidingProps;

		// Visualization component
		Gfx::tSolidColorGridPtr mGrid;

		// Used for autopainting
		tEditableTileEntityPtr mExistingTiles[3][3];

		// Used to keep track of old dimensions. Totally sucks.
		Math::tVec2i mOldDims;

	public:
		// HEY! Cool.
		class tools_export tCanvasDummyEntity : public tDummyObjectEntity
		{
			define_dynamic_cast( tCanvasDummyEntity, Gfx::tRenderableEntity );
		public:
			tCanvasDummyEntity( const Gfx::tRenderBatchPtr& batchPtr, const Math::tAabbf& objectSpaceBox, b32 useSphereCollision = false )
				: tDummyObjectEntity( batchPtr, objectSpaceBox, useSphereCollision ) { }
		};

		static const char* fEditablePropDimensions( ) { return "TileCanvas.Dimensions"; }
		static const char* fEditablePropResolution( ) { return "TileCanvas.TileSize"; }
		static const char* fEditablePropTileSet( ) { return "TileCanvas.TileSet"; }
		static const char* fEditablePropTileSetListDEFUNCT( ) { return "TileCanvas.TileSetList"; }

		tEditableTileCanvas( tEditableObjectContainer& container );
		tEditableTileCanvas( tEditableObjectContainer& container, const Sigml::tTileCanvasObject* tco );
		~tEditableTileCanvas( );
		
		void fClear( );

		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;

		virtual void fAddToWorld();
		virtual void fRemoveFromWorld();

		tGrowableArray< tEntityPtr > fGetContents( const b32 propsOnly = false );

		// Undo/redo states.
		tTileCanvasState fSaveState( const Math::tVec2i* dimOverride = NULL ) const;
		void fLoadState( const tTileCanvasState& state );

		// Drawing/erasing.
		void fPaintTilePos( tEditableTileEntityPtr& paintTile, const Math::tVec3f& worldPos, u32 numRotations );
		void fAutopaintTilePos( tEditableTiledml* tileDb, const Math::tVec3f& worldPos );
		void fEraseTile( const Math::tVec3f& worldPos );
		void fEraseProp( tEditableTileEntityPtr& prop );
		void fRecordReaddProp( tEditableTileEntityPtr& prop ); // To be used very selectively

		// Visualization.
		void	fPreviewRandomized( tEditableTiledml* tileDb );
		void	fPickPropAppareance( tEditableTiledml* tileDb, tEditableTileEntityPtr& prop );
		void	fPickTileIcon( tEditableTiledml* tileDb, tEditableTileEntityPtr& tile );
		void	fSetViewModel( b32 showModel );
		b32		fGetViewingModels( ) const { return mViewModels; }
		void	fSetHideProps( b32 hideProps );
		b32		fGetHidingProps( ) const { return mHidingProps; }

		// Spatial info/manipulations.
		b32				fOutOfBounds( u32 x, u32 y ) const; // Checks if this array location is outside the bounds.
		Math::tVec2i	fDimensions() const;
		f32				fResolution() const;
		tFilePathPtr	fTileSetPath() const;
		f32				fTileHeight( const Math::tVec3f& worldPos );
		void			fSetSize( u32 width, u32 height, b32 clearTiles = false );
		void			fSnapToGrid( Math::tVec3f& pos, const Math::tVec2u& dims );
		Math::tVec2u	fGetGridCoords( Math::tVec3f& worldPos );
		b32				fShiftHorizontal( s32 shiftAmount, b32 moveProps = true );
		b32				fShiftVertical( s32 shiftAmount, b32 moveProps = true );

		Math::tAabbf fComputeBounding( );

	protected:
		void fNotifyPropertyChanged( tEditableProperty& property );

	private:
		void fAddEditableProperties();
		void fCommonCtor();

		// Spatial support.
		void fWorldToGrid( const Math::tVec3f& worldPos, s32& gridX, s32& gridY, const Math::tVec2u& tileDims = Math::tVec2u(1,1) );
		Math::tVec3f fGridToPosition( const s32 gridX, const s32 gridY, b32 worldSpace = false ); // otherwise, localspace
		void fShiftProps( s32 shiftX, s32 shiftY );

		// Painting/erasing support
		void fFillPixel( tEditableTileEntityPtr& paintTile, s32 gridX, s32 gridY, u32 numRotations );
		void fFillPixelNoPosCompute( tEditableTileEntityPtr& paintTile, s32 gridX, s32 gridY );
		void fErasePixel( s32 gridX, s32 gridY );

		// Visualization support.
		void fUpdateBoxTint();
		void fUpdateVisualSpacing();
		void fPreview( tEditableTileEntityPtr& object, tEditableTileSet* tileSet );
		tEditableTileSet* fPickTileSet( tEditableTiledml* db );

		// Used for the autotile brush.
		b32 fAnalyzeTile( tEditableTileEntityPtr& outNeededTile, u32& outNumRotations, tEditableTiledml* tileDb, s32 gridX, s32 gridY );
		tEditableTileEntityPtr fGetNullSpaceTileByDirection( u32& outNumRotations, tEditableTiledml* tileDb, tDirections dir );
	};
}

#endif // __tEditableTileCanvas__
