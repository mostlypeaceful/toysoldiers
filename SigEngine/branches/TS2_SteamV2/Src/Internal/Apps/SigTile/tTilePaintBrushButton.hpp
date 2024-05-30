//------------------------------------------------------------------------------
// \file tTilePaintBrushButton.hpp - 07 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTilePaintBrushButton__
#define __tTilePaintBrushButton__
#include "tEditorCursorControllerButton.hpp"
#include "tTileCanvas.hpp"
#include "Tieml.hpp"

namespace Sig
{
	class tTilePaintPanel;
	enum tTileTypes;

	class tModifyTileCanvasAction : public tEditorButtonManagedCursorAction
	{
		tTileCanvasPtr mRealCanvas;
		Tieml::tFile mStartCanvas, mEndCanvas;
	public:
		tModifyTileCanvasAction( tToolsGuiMainWindow& mainWindow, tTileCanvas* canvas  )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mRealCanvas( canvas )
		{
			mRealCanvas->fSerialize( mStartCanvas );
			fSetIsLive( true );
		}
		virtual void fUndo( )
		{
			mRealCanvas->fClear( );
			mRealCanvas->fDeserialize( mStartCanvas );
		}
		virtual void fRedo( )
		{
			mRealCanvas->fClear( );
			mRealCanvas->fDeserialize( mEndCanvas );
		}
		virtual void fEnd( )
		{
			mRealCanvas->fSerialize( mEndCanvas );
			tEditorButtonManagedCursorAction::fEnd( );
		}
	};

	///
	/// \class tTileBrushBase
	/// \brief Base class interface for shared brush functionality.
	class tTileBrushBase : public tEditorButtonManagedCursorController
	{
	protected:
		tTileCanvas* mCanvas;
		Math::tVec3f mSelectPos;
		b32 mPainting;
		tRefCounterPtr<tModifyTileCanvasAction> mCurrentAction;

	public:
		tTileBrushBase( tEditorCursorControllerButton* button, tTileCanvas* canvas );

		/// 
		/// \brief Returns false if the OnTick should be skipped.
		b32 fHandleHover( );
		void fHandleCursor( );

		virtual void fSyncCursor( ) { }

		virtual void fRotateTile( b32 ccw ) = 0;
		virtual void fHideCursor( ) = 0;
		virtual void fShowCursor( ) = 0;

	protected:
		virtual void fDoPaintAction( ) = 0;

	private:
		void fBeginPaint( );
		void fEndPaint( );
	};


	///
	/// \class tTilePaintBrushButton
	/// \brief A configurable button for painting different types of tiles.
	class tTilePaintBrushButton : public tEditorCursorControllerButton
	{
		tTilePaintPanel*	mPaintPanel;
		tTileCanvas*		mCanvas;
		tTileTypes			mType;
		u32					mTileNum;

	public:
		tTilePaintBrushButton( 
			tEditorCursorControllerButtonGroup* parent,
			tTilePaintPanel* paintPanel,
			tTileCanvas* canvas,
			tTileTypes type,
			const char* buttonIcon,
			const char* buttonIconDes,
			const char* tooltip );
		~tTilePaintBrushButton( ) { }

		tTileTypes fTileType( ) const { return mType; }

		virtual tEditorCursorControllerPtr fCreateCursorController( );

		virtual b32 fReselectable( ) { return true; }
	};

	class tTileEraseBrushButton : public tEditorCursorControllerButton
	{
		tTileCanvas* mCanvas;

	public:
		tTileEraseBrushButton( tEditorCursorControllerButtonGroup* parent, tTileCanvas* canvas );
		~tTileEraseBrushButton( ) { }
		virtual tEditorCursorControllerPtr fCreateCursorController( );
	};

	class tAutoFloorBrushButton : public tEditorCursorControllerButton
	{
		tTileCanvas* mCanvas;

	public:
		tAutoFloorBrushButton( tEditorCursorControllerButtonGroup* parent, tTileCanvas* canvas );
		~tAutoFloorBrushButton( ) { }
		virtual tEditorCursorControllerPtr fCreateCursorController( );
	};
}

#endif // __tTilePaintBrushButton__
