//------------------------------------------------------------------------------
// \file tTilePaintBrushButton.cpp - 07 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigEdPch.hpp"
#include "tTilePaintBrushButton.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tEditableObject.hpp"
#include "FileSystem.hpp"
#include "tTilePaintPanel.hpp"
#include "tEditableObjectContainer.hpp"

namespace Sig
{
	tTileBrushBase::tTileBrushBase( tEditorCursorControllerButton* button/*, tTileCanvas* canvas*/ )
		: tEditorButtonManagedCursorController( button )
		, mPainting( false )
	{
	}

	b32 tTileBrushBase::fHandleHover( )
	{
		tEditorCursorController::fHandleHover();

		tWxRenderPanel* renderPanel = fMainWindow( ).fRenderPanelContainer( )->fGetActiveRenderPanel( );
		if( !renderPanel )
			return false; // cursor is not over any render panels, nothing to do

		tEditableTileCanvas* canvas = mCurrentHoverObject ? mCurrentHoverObject->fDynamicCast< tEditableTileCanvas >( ) : 0;
		if( !mCurrentHoverObject || !canvas || canvas->fIsFrozen( ) )
			return false;

		mSelectPos = mLastHoverIntersection;

		return true;
	}

	void tTileBrushBase::fHandleCursor( )
	{
		tWxRenderPanel* renderPanel = fMainWindow( ).fRenderPanelContainer( )->fGetActiveRenderPanel( );
		tEditableTileCanvas* canvas = mCurrentHoverObject ? mCurrentHoverObject->fDynamicCast< tEditableTileCanvas >( ) : 0;
		if( !renderPanel || !mCurrentHoverObject || !canvas || canvas->fIsFrozen( ) )
		{
			fEndPaint( );
			return;
		}

		if( mPaintCanvas != mCurrentHoverObject )
			fEndPaint( );

		const Input::tMouse& mouse = renderPanel->fGetMouse( );
		const Input::tKeyboard& kb = Input::tKeyboard::fInstance( );


		if( 
			!kb.fButtonHeld( Input::tKeyboard::cButtonLAlt ) 
			&& !kb.fButtonHeld( Input::tKeyboard::cButtonRAlt ) 
			&& !fMainWindow( ).fPriorityInputActive( ) 
			&& mouse.fButtonHeld( Input::tMouse::cButtonLeft ) )// don't edit if other input is active
		{
			mPaintCanvas = mCurrentHoverObject;
			fBeginPaint();
			fDoPaintAction();
		}
		else
		{
			fEndPaint( );
		}
	}

	void tTileBrushBase::fBeginPaint( )
	{
		if( !mPainting )
		{
			// we're transitioning from not-painting to painting; do any setup here if necessary
			tEditableTileCanvas* canvas = mCurrentHoverObject->fDynamicCast< tEditableTileCanvas >( );
			mCurrentAction.fReset( new tModifyTileCanvasButtonAction( fMainWindow( ), canvas ) );
			fMainWindow( ).fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( mCurrentAction.fGetRawPtr( ) ) );
		}
		mPainting = true;
	}

	void tTileBrushBase::fEndPaint( )
	{
		if( mPainting )
		{
			// we're transitioning from painting to not-painting; do any cleanup here if necessary
			mCurrentAction->fEnd( );
			mCurrentAction.fRelease( );
		}

		mPainting = false;
	}


	//------------------------------------------------------------------------------
	// tTilePaintBrush
	//------------------------------------------------------------------------------
	class tTilePaintBrush : public tTileBrushBase
	{
		tTilePaintPanel* mPaintPanel;
		tEditableTileEntityPtr mDisplayTile;
		//f32 mHeight; No variable height within a single canvas.
		tTileTypes mType;
		s32 mNumRotations;

		tEditableTileCanvas* mCanv; // Tracks the last canvas this brush touched
		
	public:
		tTilePaintBrush( tTilePaintBrushButton* button, tTilePaintPanel* paintPanel, tTileTypes type )
			: tTileBrushBase( button )
			, mPaintPanel( paintPanel )
			//, mHeight( 0.f )
			, mType( type )
			, mNumRotations( 0 )
			, mCanv( NULL )
		{
			fMainWindow( ).fSetStatus( "Paint Tiles" );

			mPaintPanel->fAddCursorHotKeys( this, false );

			fSyncCursor( );
		}

		virtual void fOnTick( )
		{
			if( !mDisplayTile )
			{
				log_warning("Missing display tile on a tile paint cursor");
				return;
			}

			if( !fHandleHover() )
				return;

			if( !mDisplayTile )
			{
				log_warning( "fHandleHover reported everything succeeded but there's no display tile." );
				return;
			}

			// Putting the show here causes the tile to pop up only when the mouse cursor is
			// brought into the viewport. Also this brush isn't updated while in 3D mode so it
			// doesn't cause any newly-selected tiles to pop up.
			fShowCursor( );

			tEditableTileCanvas* canvas = mCurrentHoverObject->fDynamicCast< tEditableTileCanvas >( );
			mDisplayTile->fSetGridCoords( canvas->fGetGridCoords(mSelectPos) );
			mDisplayTile->fSetSize( canvas->fResolution(), true );

			// Get the world position for the grid cell and the height at that point.
			if( !mDisplayTile->fIsProp() )
				canvas->fSnapToGrid( mSelectPos, mDisplayTile->fRotatedDims( ) );
			else
			{
				tWxRenderPanelContainer* gfx = fGuiApp().fMainWindow().fRenderPanelContainer( );
				tWxRenderPanel* panel = gfx->fGetActiveRenderPanel( );
				if( panel->fSnapToGrid() )
					panel->fSnapVertex( mSelectPos );
			}
			//mHeight = canvas->fTileHeight( mSelectPos );
			mSelectPos.y += /*mHeight +*/ 0.5f;

			// Pick up the canvas' rotation but not position or scale.
			Math::tPRSXformf prs( canvas->fObjectToWorld() * mDisplayTile->fObjectToWorld() );
			prs.mPosition = mSelectPos;
			prs.mScale = Math::tVec3f(1.f, 1.f, 1.f);

			// Move the tile to it's real position.
			Math::tMat3f rotatedPos;
			prs.fToMatrix( rotatedPos );
			mDisplayTile->fMoveTo( rotatedPos );
			mDisplayTile->fSetHeight( 0.f );

			// Deal with previewing a prop's model after the world xform has been sorted out.
			b32 canvChanged = false;
			if( mDisplayTile->fIsProp() || (mType == cUniques && mDisplayTile->fIsSpecificModel()) )
			{
				if( canvas != mCanv )
					canvas->fPickPropAppareance( mPaintPanel->fDatabase(), mDisplayTile );
				canvChanged = true;
			}
			else if( mDisplayTile->fIsSpecificModel() )
			{
				// Attempt to get a tile icon from the canvas.
				if( canvas != mCanv )
					canvas->fPickTileIcon( mPaintPanel->fDatabase(), mDisplayTile );
				canvChanged = true;
			}

			if( canvChanged )
				mCanv = canvas;

			fHandleCursor( );
		}

		virtual void fRotateTile( b32 ccw )
		{
			if( !ccw )
				mNumRotations = ++mNumRotations%4;
			else
			{
				if( --mNumRotations < 0 ) mNumRotations = 3;
			}

			mDisplayTile->fSetNumRotations( mNumRotations, true );
		}

		virtual void fShowCursor( )
		{
			if( !mDisplayTile->fSceneGraph( ) )
				mDisplayTile->fSpawnImmediate( fGuiApp().fSceneGraph()->fRootEntity() );

			mDisplayTile->fShow();
		}

		virtual void fHideCursor( )
		{
			mDisplayTile->fHide();
		}

		virtual void fSyncCursor( )
		{
			mCanv = NULL;

			if( mDisplayTile )
			{
				mDisplayTile->fHide();
				mDisplayTile->fDeleteImmediate();
			}

			mDisplayTile = mPaintPanel->fGetTileWithPanelParams( mPaintPanel->fParent()->fGuiApp().fEditableObjects(), mType );
			mDisplayTile->fSetHeight( 0.f );
			mDisplayTile->fMoveTo( Math::tVec3f::cZeroVector );
		}

	protected:
		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			if( mDisplayTile )
			{
				mDisplayTile->fHide();
				mDisplayTile->fDeleteImmediate( );
				mDisplayTile.fRelease( );
			}

			if( !dynamic_cast<tTileBrushBase*>(nextController) )
				mPaintPanel->fClearIndividualsDisplay();

			tEditorButtonManagedCursorController::fOnNextCursor( nextController );
		}

		void fDoPaintAction( )
		{
			tToolsGuiMainWindow& mainWindow = fGuiApp().fMainWindow();
			if( (!mainWindow.fPriorityInputActive( ) && mainWindow.fRenderPanelContainer()->fGetActiveRenderPanel()->fGetMouse().fButtonDown( Input::tMouse::cButtonLeft )) || !mDisplayTile->fIsProp() )
			{
				tEditableTileCanvas* canvas = mCurrentHoverObject->fDynamicCast< tEditableTileCanvas >( );

				Math::tVec2u coords = mDisplayTile->fGetGridCoords();
				if( canvas->fOutOfBounds( coords.x, coords.y ) )
					return;

				tEditableTileEntityPtr clone( mDisplayTile->fClone( )->fDynamicCast< tEditableTileEntity >() );
				canvas->fPaintTilePos( clone, mSelectPos, mNumRotations );
			}
		}

		tEntityPtr fFilterHoverObject( const tEntityPtr& newHoverObject )
		{
			if( newHoverObject && !newHoverObject->fDynamicCast< tEditableTileCanvas >( ) )
				return tEntityPtr( );

			// this hover object supports editable tiles
			return newHoverObject;
		}

	private:
		tEntityPtr fPick( const Math::tRayf& ray, f32* bestTout, tEntity* const* ignoreList, u32 numToIgnore )
		{
			if( mDisplayTile->fIsProp() )
			{
				tEntity* ent = mDisplayTile.fGetRawPtr();
				return fGuiApp( ).fEditableObjects( ).fPickByType<tEditableTileCanvas::tCanvasDummyEntity>( ray, bestTout, &ent, 1 );
			}

			return fGuiApp( ).fEditableObjects( ).fPickByType<tEditableTileCanvas::tCanvasDummyEntity>( ray, bestTout, ignoreList, numToIgnore );
		}
	};

	tTilePaintBrushButton::tTilePaintBrushButton( 
		tEditorCursorControllerButtonGroup* parent,
		tTilePaintPanel* paintPanel,
		tTileTypes type,
		const char* buttonIcon,
		const char* buttonIconDes,
		const char* tooltip )
		: tEditorCursorControllerButton( parent, wxBitmap( buttonIcon ), wxBitmap( buttonIconDes ), tooltip )
		, mPaintPanel( paintPanel )
		, mType( type )
	{
	}

	tEditorCursorControllerPtr tTilePaintBrushButton::fCreateCursorController( )
	{
		tTilePaintBrush* paintCursor = new tTilePaintBrush( this, mPaintPanel, mType );
		return tEditorCursorControllerPtr( paintCursor );
	}


	//------------------------------------------------------------------------------
	// tTileEraseBrush
	//------------------------------------------------------------------------------
	class tTileEraseBrush : public tTileBrushBase
	{
	public:

		tTileEraseBrush( tTileEraseBrushButton* button, tTilePaintPanel* paintPanel )
			: tTileBrushBase( button )
		{
			fMainWindow( ).fSetStatus( "Erase Tiles" );

			paintPanel->fAddCursorHotKeys( this, true );
		}

		virtual void fOnTick( )
		{
			if( !fHandleHover( ) )
				return;

			fHandleCursor( );
		}

		virtual void fRotateTile( b32 ccw ) { }
		virtual void fHideCursor( ) { }
		virtual void fShowCursor( ) { }

	protected:
		void fDoPaintAction( )
		{
			tEditableTileCanvas* canvas = mCurrentHoverObject->fDynamicCast< tEditableTileCanvas >( );
			canvas->fEraseTile( mSelectPos );
		}

		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			if( !dynamic_cast<tTileBrushBase*>(nextController) )
				fGetButton()->fGetParent()->fClearSelection();
			tEditorButtonManagedCursorController::fOnNextCursor( nextController );
		}
	};

	tTileEraseBrushButton::tTileEraseBrushButton( tEditorCursorControllerButtonGroup* parent, tTilePaintPanel* paintPanel )
		: tEditorCursorControllerButton( parent, wxBitmap( "EraseTileSel" ), wxBitmap( "EraseTileDeSel" ), "Erase tiles" )
		, mPaintPanel( paintPanel )
	{
	}

	tEditorCursorControllerPtr tTileEraseBrushButton::fCreateCursorController( )
	{
		tTileEraseBrush* paintCursor = new tTileEraseBrush( this, mPaintPanel );
		return tEditorCursorControllerPtr( paintCursor );
	}


	//------------------------------------------------------------------------------
	// tAutoFloorBrush
	//------------------------------------------------------------------------------
	class tAutoFloorBrush : public tTileBrushBase
	{
		tTilePaintPanel* mPaintPanel;
		Gfx::tRenderableEntityPtr mBoxCursor;

	public:
		tAutoFloorBrush( tAutoFloorBrushButton* button, tTilePaintPanel* paintPanel )
			: tTileBrushBase( button )
			, mPaintPanel( paintPanel )
		{
			fMainWindow( ).fSetStatus( "Autopaint Floor Tiles" );

			mPaintPanel->fAddCursorHotKeys( this, true );

			const Math::tAabbf localSpaceBox = fGuiApp().fEditableObjects().fGetDummyBoxTemplate( ).fGetBounds( );
			mBoxCursor.fReset( new tEditableObject::tDummyObjectEntity( fGuiApp().fEditableObjects().fGetDummyBoxTemplate( ).fGetRenderBatch( ), localSpaceBox ) );
			mBoxCursor->fSpawnImmediate( fGuiApp().fSceneGraph()->fRootEntity() );
			fHideCursor();
		}

		virtual void fOnTick( )
		{
			if( !fHandleHover( ) )
				return;

			tEditableTileCanvas* canvas = mCurrentHoverObject->fDynamicCast< tEditableTileCanvas >( );
			canvas->fSnapToGrid( mSelectPos, Math::tVec2u( 1, 1 ) );
			const f32 height = canvas->fTileHeight( mSelectPos );
			const f32 size = canvas->fResolution();

			// Pick up the canvas' rotation but not position or scale.
			Math::tPRSXformf prs( canvas->fObjectToWorld() );
			prs.mPosition = mSelectPos;
			prs.mScale = Math::tVec3f(size/2.f, 0.01f, size/2.f);

			// Move the tile to it's real position.
			Math::tMat3f rotatedPos;
			prs.fToMatrix( rotatedPos );
			mBoxCursor->fMoveTo( rotatedPos );

			fHandleCursor( );

			// TODO: convert these to the grid coords
			std::stringstream ss;
			ss << "Autopaint Floor Tiles [" << mSelectPos.x << "][" << mSelectPos.z << "]";
			fMainWindow( ).fSetStatus( ss.str( ).c_str( ) );

			fShowCursor( );
		}

		virtual void fRotateTile( b32 ccw ) { }
		virtual void fHideCursor( ) { mBoxCursor->fSetRgbaTint( Math::tVec4f( 0.f, 1.f, 0.f, 0.f ) ); }
		virtual void fShowCursor( ) { mBoxCursor->fSetRgbaTint( Math::tVec4f( 0.f, 1.f, 0.f, 1.f ) ); }

	protected:
		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			if( mBoxCursor )
			{
				fHideCursor();
				mBoxCursor->fDeleteImmediate( );
				mBoxCursor.fRelease( );
			}

			if( !dynamic_cast<tTileBrushBase*>(nextController) )
				mPaintPanel->fClearIndividualsDisplay();

			tEditorButtonManagedCursorController::fOnNextCursor( nextController );
		}

		void fDoPaintAction( )
		{
			tEditableTileCanvas* canvas = mCurrentHoverObject->fDynamicCast< tEditableTileCanvas >( );
			canvas->fAutopaintTilePos( mPaintPanel->fDatabase(), mSelectPos );
		}
	};

	tAutoFloorBrushButton::tAutoFloorBrushButton( tEditorCursorControllerButtonGroup* parent, tTilePaintPanel* paintPanel )
		: tEditorCursorControllerButton( parent, wxBitmap( "TileFloorAutoSel" ), wxBitmap( "TileFloorAutoDeSel" ), "Autopaint floors" )
		, mPaintPanel( paintPanel )
	{
	}

	tEditorCursorControllerPtr tAutoFloorBrushButton::fCreateCursorController( )
	{
		tAutoFloorBrush* paintCursor = new tAutoFloorBrush( this, mPaintPanel );
		return tEditorCursorControllerPtr( paintCursor );
	}
}
