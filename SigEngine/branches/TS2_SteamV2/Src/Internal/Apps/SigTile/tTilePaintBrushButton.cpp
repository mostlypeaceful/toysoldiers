//------------------------------------------------------------------------------
// \file tTilePaintBrushButton.cpp - 07 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigTilePch.hpp"
#include "tTilePaintBrushButton.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tEditableObject.hpp"
#include "tTileDbPanel.hpp"
#include "FileSystem.hpp"
#include "tTilePaintPanel.hpp"

namespace Sig
{
	tTileBrushBase::tTileBrushBase( tEditorCursorControllerButton* button, tTileCanvas* canvas )
		: tEditorButtonManagedCursorController( button )
		, mCanvas( canvas )
		, mPainting( false )
	{
	}

	b32 tTileBrushBase::fHandleHover( )
	{
		tWxRenderPanel* renderPanel = fMainWindow( ).fRenderPanelContainer( )->fGetActiveRenderPanel( );
		if( !renderPanel )
			return false; // cursor is not over any render panels, nothing to do

		tEditableObject* eo = mCurrentHoverObject ? mCurrentHoverObject->fDynamicCast< tEditableObject >( ) : 0;
		if( (eo && eo->fIsFrozen( )) )
			return false;

		const Input::tMouse& mouse = renderPanel->fGetMouse( );
		const Gfx::tCamera& camera = renderPanel->fGetScreen( )->fViewport( 0 )->fLogicCamera( );
		const wxSize panelSize = renderPanel->GetSize( );

		Math::tVec3f org = camera.fComputePickRay( 
			Math::tVec2u( mouse.fGetState( ).mCursorPosX, mouse.fGetState( ).mCursorPosY ),
			Math::tVec2u( panelSize.x, panelSize.y ) ).mOrigin;

		mSelectPos = org;

		return true;
	}

	void tTileBrushBase::fHandleCursor( )
	{
		tWxRenderPanel* renderPanel = fMainWindow( ).fRenderPanelContainer( )->fGetActiveRenderPanel( );
		if( !renderPanel )
		{
			fEndPaint( );
			return;
		}

		const Input::tMouse& mouse = renderPanel->fGetMouse( );
		const Input::tKeyboard& kb = Input::tKeyboard::fInstance( );


		if( 
			!kb.fButtonHeld( Input::tKeyboard::cButtonLAlt ) 
			&& !kb.fButtonHeld( Input::tKeyboard::cButtonRAlt ) 
			&& !fMainWindow( ).fPriorityInputActive( ) 
			&& mouse.fButtonHeld( Input::tMouse::cButtonLeft ) )// don't edit if other input is active
		{
			fBeginPaint( );
			fDoPaintAction( );
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
			mCurrentAction.fReset( new tModifyTileCanvasAction( fMainWindow( ), mCanvas ) );
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


	class tTilePaintBrush : public tTileBrushBase
	{
		tTilePaintPanel* mPaintPanel;
		tEditableTileEntityPtr mDisplayTile;
		f32 mHeight;
		tTileTypes mType;
		s32 mNumRotations;
		
	public:
		tTilePaintBrush( tTilePaintBrushButton* button, tTilePaintPanel* paintPanel, tTileCanvas* canvas, tTileTypes type )
			: tTileBrushBase( button, canvas )
			, mPaintPanel( paintPanel )
			, mType( type )
			, mNumRotations( 0 )
		{
			fMainWindow( ).fSetStatus( "Paint Tiles" );

			fSyncCursor( );
		}

		virtual void fOnTick( )
		{
			if( !fHandleHover( ) )
				return;

			// Putting the show here causes the tile to pop up only when the mouse cursor is
			// brought into the viewport. Also this brush isn't updated while in 3D mode so it
			// doesn't cause any newly-selected tiles to pop up.
			fShowCursor( );

			mCanvas->fSnapToGrid( mSelectPos, mDisplayTile->fRotatedDims( ) );
			mSelectPos.y = mCanvas->fTileHeight( mSelectPos.x, mSelectPos.z ) + 0.5f;
			mDisplayTile->fMoveTo( mSelectPos );

			fHandleCursor( );
		}

		virtual void fRotateTile( b32 ccw )
		{
			mDisplayTile->fRotate( ccw );

			if( !ccw )
				mNumRotations = ++mNumRotations%4;
			else
			{
				if( --mNumRotations < 0 ) mNumRotations = 3;
			}
		}

		virtual void fShowCursor( )
		{
			mCanvas->fShowTile( mDisplayTile.fGetRawPtr( ) );
		}

		virtual void fHideCursor( )
		{
			mCanvas->fHideTile( mDisplayTile.fGetRawPtr( ) );
		}

		virtual void fSyncCursor( )
		{
			mDisplayTile = mPaintPanel->fGetTileWithPanelParams( mType, mNumRotations );
			mHeight = mDisplayTile->fObjectToWorld( ).fGetTranslation( ).y;
		}

	protected:
		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			if( mDisplayTile )
			{
				mCanvas->fHideTile( mDisplayTile.fGetRawPtr( ) );
				mDisplayTile.fRelease( );
			}

			if( !nextController )
				return;

			tEditorButtonManagedCursorController::fOnNextCursor( nextController );
		}

		void fDoPaintAction( )
		{
			tEditableTileEntityPtr clone = mDisplayTile->fClone( );
			Math::tVec3f cloneTranslation = clone->fObjectToWorld( ).fGetTranslation( );
			cloneTranslation.y = mHeight;
			clone->fMoveTo( cloneTranslation );
			mCanvas->fPaintTilePos( clone, mSelectPos.x, mSelectPos.z );

			fSyncCursor( );
		}
	};

	tTilePaintBrushButton::tTilePaintBrushButton( 
		tEditorCursorControllerButtonGroup* parent,
		tTilePaintPanel* paintPanel,
		tTileCanvas* canvas,
		tTileTypes type,
		const char* buttonIcon,
		const char* buttonIconDes,
		const char* tooltip )
		: tEditorCursorControllerButton( parent, wxBitmap( buttonIcon ), wxBitmap( buttonIconDes ), tooltip )
		, mPaintPanel( paintPanel )
		, mCanvas( canvas )
		, mType( type )
	{
	}

	tEditorCursorControllerPtr tTilePaintBrushButton::fCreateCursorController( )
	{
		tTilePaintBrush* paintCursor = new tTilePaintBrush( this, mPaintPanel, mCanvas, mType );
		return tEditorCursorControllerPtr( paintCursor );
	}



	class tTileEraseBrush : public tTileBrushBase
	{
	public:

		tTileEraseBrush( tTileEraseBrushButton* button, tTileCanvas* canvas )
			: tTileBrushBase( button, canvas )
		{
			fMainWindow( ).fSetStatus( "Erase Tiles" );
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
			mCanvas->fEraseTile( mSelectPos.x, mSelectPos.z );
		}
	};

	tTileEraseBrushButton::tTileEraseBrushButton( tEditorCursorControllerButtonGroup* parent, tTileCanvas* canvas )
		: tEditorCursorControllerButton( parent, wxBitmap( "EraseTileSel" ), wxBitmap( "EraseTileDeSel" ), "Erase tiles" )
		, mCanvas( canvas )
	{
	}

	tEditorCursorControllerPtr tTileEraseBrushButton::fCreateCursorController( )
	{
		tTileEraseBrush* paintCursor = new tTileEraseBrush( this, mCanvas );
		return tEditorCursorControllerPtr( paintCursor );
	}



	class tAutoFloorBrush : public tTileBrushBase
	{
	public:
		tAutoFloorBrush( tAutoFloorBrushButton* button, tTileCanvas* canvas )
			: tTileBrushBase( button, canvas )
		{
			fMainWindow( ).fSetStatus( "Autopaint Floor Tiles" );
		}

		virtual void fOnTick( )
		{
			if( !fHandleHover( ) )
				return;

			mCanvas->fSnapToGrid( mSelectPos, Math::tVec2u( 1, 1 ) );
			fHandleCursor( );

			std::stringstream ss;
			ss << "Autopaint Floor Tiles [" << mSelectPos.x << "][" << mSelectPos.z << "]";
			fMainWindow( ).fSetStatus( ss.str( ).c_str( ) );
		}

		virtual void fRotateTile( b32 ccw ) { }
		virtual void fHideCursor( ) { }
		virtual void fShowCursor( ) { }

	protected:
		void fDoPaintAction( )
		{
			mCanvas->fAutopaintTilePos( mSelectPos.x, mSelectPos.z );
		}
	};

	tAutoFloorBrushButton::tAutoFloorBrushButton( tEditorCursorControllerButtonGroup* parent, tTileCanvas* canvas )
		: tEditorCursorControllerButton( parent, wxBitmap( "TileFloorAutoSel" ), wxBitmap( "TileFloorAutoDeSel" ), "Autopaint floors" )
		, mCanvas( canvas )
	{
	}

	tEditorCursorControllerPtr tAutoFloorBrushButton::fCreateCursorController( )
	{
		tAutoFloorBrush* paintCursor = new tAutoFloorBrush( this, mCanvas );
		return tEditorCursorControllerPtr( paintCursor );
	}
}
