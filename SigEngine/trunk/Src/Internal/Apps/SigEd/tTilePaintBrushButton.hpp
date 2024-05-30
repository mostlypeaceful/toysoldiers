//------------------------------------------------------------------------------
// \file tTilePaintBrushButton.hpp - 07 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTilePaintBrushButton__
#define __tTilePaintBrushButton__
#include "tEditorCursorControllerButton.hpp"
#include "tEditableTileCanvas.hpp"


namespace Sig
{
	class tTilePaintPanel;
	enum tTileTypes;
	
	///
	/// \class tModifyTileCanvasAction
	/// \brief The action that saves any modification to a tile canvas.
	class tModifyTileCanvasButtonAction : public tEditorButtonManagedCursorAction
	{
		tEditableTileCanvasPtr mRealCanvas;
		tTileCanvasState mStartCanvas, mEndCanvas;
	public:
		tModifyTileCanvasButtonAction( tToolsGuiMainWindow& mainWindow, tEditableTileCanvas* canvas  )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mRealCanvas( canvas )
		{
			mStartCanvas = mRealCanvas->fSaveState();
			fSetIsLive( true );
		}
		virtual void fUndo( )
		{
			mRealCanvas->fClear( );
			mRealCanvas->fLoadState( mStartCanvas );
		}
		virtual void fRedo( )
		{
			mRealCanvas->fClear( );
			mRealCanvas->fLoadState( mEndCanvas );
		}
		virtual void fEnd( )
		{
			mEndCanvas = mRealCanvas->fSaveState();
			tEditorButtonManagedCursorAction::fEnd();
		}
	};

	///
	/// \class tTileBrushBase
	/// \brief Base class interface for shared brush functionality.
	class tTileBrushBase : public tEditorButtonManagedCursorController
	{
	protected:
		Math::tVec3f mSelectPos;
		b32 mPainting;
		tRefCounterPtr<tModifyTileCanvasButtonAction> mCurrentAction;
		tEntityPtr mPaintCanvas;

	public:
		tTileBrushBase( tEditorCursorControllerButton* button/*, tTileCanvas* canvas*/ );

		/// 
		/// \brief Returns false if the OnTick should be skipped.
		b32 fHandleHover( );
		void fHandleCursor( );

		virtual void fSyncCursor( ) { }

		virtual void fRotateTile( b32 ccw ) { };
		virtual void fHideCursor( )  { };
		virtual void fShowCursor( ) { };

	protected:
		
		virtual void fDoPaintAction( ) { };

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
		tTileTypes			mType;

	public:
		tTilePaintBrushButton( 
			tEditorCursorControllerButtonGroup* parent,
			tTilePaintPanel* paintPanel,
			tTileTypes type,
			const char* buttonIcon,
			const char* buttonIconDes,
			const char* tooltip );
		~tTilePaintBrushButton( ) { }

		tTileTypes fTileType( ) const { return mType; }

		virtual tEditorCursorControllerPtr fCreateCursorController( );

		virtual b32 fReselectable( ) { return true; }
	};

	///
	/// \class tTileEraseBrushButton
	/// \brief 
	class tTileEraseBrushButton : public tEditorCursorControllerButton
	{
		tTilePaintPanel* mPaintPanel;
	public:
		tTileEraseBrushButton( tEditorCursorControllerButtonGroup* parent, tTilePaintPanel* paintPanel );
		~tTileEraseBrushButton( ) { }
		virtual tEditorCursorControllerPtr fCreateCursorController( );
	};

	///
	/// \class tAutoFloorBrushButton
	/// \brief 
	class tAutoFloorBrushButton : public tEditorCursorControllerButton
	{
		tTilePaintPanel* mPaintPanel;
	public:
		tAutoFloorBrushButton( tEditorCursorControllerButtonGroup* parent, tTilePaintPanel* paintPanel );
		~tAutoFloorBrushButton( ) { }
		virtual tEditorCursorControllerPtr fCreateCursorController( );
	};
}

#endif // __tTilePaintBrushButton__
