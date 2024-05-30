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
#include "tEditableScriptNodeEntity.hpp"

namespace Sig
{
	class tEditableTileEntity;
	typedef tRefCounterPtr< tEditableTileEntity > tEditableTileEntityPtr;

	enum tTileTypes
	{
		cFloor,
		cWall,
		cNiche,
		cCorner,

		cNumBasicTypes,

		cUniques = cNumBasicTypes,

		cNumTileTypes,
	};

	enum tRandStage
	{
		cNotRandom,
		cRandomizeAtAssetGen,
		cRandomizeOnLoad,

		cNumRandTypes,
	};

	class tools_export tEditableTileEntity : public tRefCounter, public tUncopyable
	{
		Gfx::tWorldSpaceQuadsPtr	mPanel;
		tSgFileRefEntityPtr			mModel;
		tFilePathPtr				mModelPath;
		tResourcePtr				mTextureFile;
		tTileTypes					mType;
		u32							mPigmentGuid;
		b32							mTileView;
		u32							mColor;
		f32							mTileWidth;
		f32							mShrinkScale;
		Math::tVec2u				mDims;
		tGrowableArray<Math::tVec2u> mOccupiedCells;
		u32							mActionMask;
		b32							mSpecificTileSet;
		b32							mSpecificModel;
		tRandStage					mRandomType;
		b32							mShowingModels;

		tGrowableArray<tEditableScriptNodeEntityPtr> mScripts;

	public:
		tEditableTileEntity( 
			tTileTypes type,
			u32 guid, 
			tResourcePtr& texture, 
			tSgFileRefEntityPtr& model, 
			const tFilePathPtr& modelPath, 
			const Math::tMat3f& objXform, 
			u32 color, 
			f32 tileWidth,
			const Math::tVec2u& dims,
			tRandStage randType,
			b32 specTileSet, 
			b32 specModel );

		~tEditableTileEntity( );

		u32				fActionMask( ) const { return mActionMask; }
		void			fSetActionMask( u32 newMask ) { mActionMask = newMask; }

		tEditableTileEntityPtr fClone( );

		tTileTypes		fTileType( ) const { return mType; }
		tRandStage		fRandType( ) const { return mRandomType; }
		b32				fSpecTileSet( ) const { return mSpecificTileSet; }
		b32				fSpecModel( ) const { return mSpecificModel; }
		Math::tMat3f	fObjectToWorld( ) const { return mPanel->fObjectToWorld( ); }
		u32				fPigmentGuid( ) const { return mPigmentGuid; }
		tFilePathPtr	fModelPath( ) const { return mModelPath; }
		Math::tVec2u	fDims( ) const { return mDims; }
		Math::tVec2u	fRotatedDims( ) const;

		void			fSetColor( u32 color );
		void			fSetHeight( f32 height );
		void			fSetSize( f32 size );

		b32 fIsRandomTile( ) const { return mRandomType != cNotRandom; }
		void fBakeRandomizedTile( const tFilePathPtr& modelPath, const tSgFileRefEntityPtr& model );

		const tGrowableArray<Math::tVec2u>& fOccupiedCells( ) const { return mOccupiedCells; }
		void fOccupyCell( const Math::tVec2u& cellIdx ) { mOccupiedCells.fPushBack( cellIdx ); }
		void fClearOccupied( ) { mOccupiedCells.fDeleteArray( ); }

		// Scripts
		void fAddScript( const tEditableScriptNodeEntityPtr& script );
		void fDeleteAllScripts( );
		void fCopyScripts( const tEditableTileEntityPtr& otherTile );
		tGrowableArray<u32> fSerializeScripts( ) const;
		void fDeserializeScripts( const tGrowableArray<u32>& guids, const tEditableTileDb* tileDb );
		void fUpdateScripts( const tEditableTileDb* tileDb );
		void fDeleteScriptsByGuid( u32 scriptGuid );

		/// 
		/// \brief showModels determines whether models should be visible or panels.
		void fSetViewMode( b32 showModels, const tSceneGraphPtr& sceneGraph );

		// Use these instead of showing/hiding specific parts.
		void fShow( const tSceneGraphPtr& sceneGraph );
		void fHide( );

		// Try to avoid using these.
		void fShowPanel( const tSceneGraphPtr& sceneGraph );
		void fHidePanel( );
		void fShowModel( const tSceneGraphPtr& sceneGraph );
		void fHideModel( );

		void fMoveTo( const Math::tVec3f& newPos );
		void fRotate( b32 ccw );

		Math::tAabbf fGetBounding( ) const;

	private:
		void fMoveTo( const Math::tMat3f& newXform );

		void fCreateSquare( f32 xPos, f32 yPos, u32 color );
	};
}

#endif // __tEditableTileEntity__
