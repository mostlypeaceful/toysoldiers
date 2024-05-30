//------------------------------------------------------------------------------
// \file tEditableTileEntity.hpp - 02 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tEditableTileEntity__
#define __tEditableTileEntity__
#include "tEditableObject.hpp"
#include "Gfx\tWorldSpaceQuads.hpp"
#include "tRefCounterPtr.hpp"
#include "tSgFileRefEntity.hpp"
#include "tSceneGraph.hpp"
#include "tTileEntity.hpp"

namespace Sig { namespace Sigml { 

	class tTileObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tTileObject, 0x3BABDE65 );
		define_dynamic_cast( tTileObject, tObject );

		u32					mTileType;
		u32					mSpecificTileSet; // Tile set guid, -1 if none
		std::string			mIdString;
		tFilePathPtr		mTexturePath;

		u32					mColor;
		f32					mTileWidth;
		Math::tVec2u		mGridCoords;
		Math::tVec2u		mDims;
		f32					mHeight;
		u32					mNumRotations;
		Math::tVec3f		mPivot;

	public:
		tTileObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	typedef tRefCounterPtr< tTileObject >		tTileObjectPtr;
} }

namespace Sig
{
	///
	/// \class tTileState
	/// \brief Contains a temporary state that a tTileCanvas can save to/restore from.
	class tools_export tTileState
	{
	public:
		tTileState( ) { }

		f32				mTileWidth;
		Math::tVec2u	mGridCoords;
		Math::tVec2u	mDims;
		f32				mHeight;
		u32				mNumRotations;
		Math::tVec3f	mPivot;

		u32				mColor;
		u32				mTileType;
		tFilePathPtr	mTexturePath;
		u32				mSpecificTileSet; // Tile set guid, -1 if none
		std::string		mIdString;
	};

	class tools_export tTileCanvasState
	{
	public:
		tGrowableArray< tTileState >	mTileStates;
		Math::tVec2i					mDims;
	};


	// Forward decs
	class tEditableTileEntity;
	typedef tRefCounterPtr< tEditableTileEntity > tEditableTileEntityPtr;
	class tEditableTileCanvas;
	typedef tRefCounterPtr< tEditableTileCanvas > tEditableTileCanvasPtr;


	class tools_export tEditableTileEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableTileEntity, tEditableObject );

		Gfx::tWorldSpaceQuadsPtr	mPanel;
		tSgFileRefEntityPtr			mModel;
		std::string					mIdString;
		tResourcePtr				mTextureFile;
		tTileTypes					mType;
		u32							mColor;
		Math::tVec2u				mDims;
		tGrowableArray<Math::tVec2u> mOccupiedCells;
		u32							mSpecificTileSet; // -1 if no specific
		b32							mShowingModels;

		f32							mTileWidth;
		Math::tVec2u				mGridCoords;
		f32							mHeight;
		u32							mNumRotations;
		Math::tVec3f				mPivot; // Used to track the center point of the canvas from within the tiles

		Gfx::tRenderableEntityPtr		mDummySphere;

		// Used by the tile canvas to indicate when it's acceptable to serialize. Specifically, when the tile
		// exists in the general object container and someone wants to save but the canvas needs to be in charge
		// of serializing things and cannot allow rogue tiles to get saved.
		b32							mAllowSerialize;

		tEditableTileCanvasPtr		mParentCanvas;

	public:
		tEditableTileEntity( tEditableObjectContainer& container );
		tEditableTileEntity( tEditableObjectContainer& container,  const Sigml::tTileObject* to );

		~tEditableTileEntity( );

		void fConfigure(
			tTileTypes type,
			tResourcePtr& texture,
			const std::string& idString, 
			u32 color,
			const Math::tVec2u& dims
			);

		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual tEntityPtr fClone( );

		virtual void fAddToWorld();
		virtual void fRemoveFromWorld();

		void fSetCanvas( tEditableTileCanvasPtr& canvas ) { mParentCanvas = canvas; }

		tTileState fSaveState( ) const;
		void fRestoreState( const tTileState& newState );

		std::string		fGetToolTip( ) const;
		std::string		fGetName( ) const { return fGetToolTip(); }

		tTileTypes		fTileType( ) const { return mType; }
		b32				fIsProp( ) const { return mType == cPropCeiling || mType == cPropFloor; }
		b32				fIsSpecificTileSet( ) const { return mSpecificTileSet != ~0u; }
		b32				fIsSpecificModel( ) const { return mIdString != ""; }
		u32				fTileSetGuid( ) const { return mSpecificTileSet; }
		Math::tVec2u	fDims( ) const { return mDims; }
		Math::tVec2u	fRotatedDims( ) const;
		void			fSetAllowSerialize( b32 doIt ) { mAllowSerialize = doIt; }

		void			fSetColor( u32 color );
		void			fSetHeight( f32 height, b32 andComputeXform = false );
		void			fSetSize( f32 size, b32 andComputeXform = false );
		void			fSetGridCoords( const Math::tVec2u& gridCoords, b32 andComputeXform = false );
		void			fSetNumRotations( u32 numRotations, b32 andComputeXform = false );
		void			fSetPivot( const Math::tVec3f& pivot, b32 andComputeXform = false );
		void			fSetTexture( const tResourcePtr& texture, b32 andComputeXform = false );
		Math::tMat3f	fComputeXform( ) const;
		void			fUpdatePanel( );
		void			fRebuildXform( ); // Recomputes transform and updates panel.

		u32				fGetColor() const { return mColor; }
		f32				fGetHeight() const { return mHeight; }
		f32				fGetSize() const { return mTileWidth; }
		Math::tVec2u	fGetGridCoords() const { return mGridCoords; }
		u32				fGetNumRotations() const { return mNumRotations; }
		Math::tVec3f	fGetPivot() const { return mPivot; }
		

		b32 fIsRandomTile( ) const { return !fIsSpecificModel(); }
		void fPreviewTileModel( const tSgFileRefEntityPtr& model );

		const tGrowableArray<Math::tVec2u>& fOccupiedCells( ) const { return mOccupiedCells; }
		void fOccupyCell( const Math::tVec2u& cellIdx ) { mOccupiedCells.fPushBack( cellIdx ); }
		void fClearOccupied( ) { mOccupiedCells.fDeleteArray( ); }

		/// 
		/// \brief showModels determines whether models should be visible or panels.
		void fSetViewMode( b32 showModels, b32 explicitOverride = false );

		// Use these instead of showing/hiding specific parts.
		void fShow();
		void fHide();

		Math::tAabbf fGetBounding( ) const;

	private:
		void fAddEditableProperties( );
		void fCreateSquare( u32 color );

		// Try to avoid using these.
		void fShowPanel( );
		void fHidePanel( );
		void fShowModel( );
		void fHideModel( );
	};
}

#endif // __tEditableTileEntity__
