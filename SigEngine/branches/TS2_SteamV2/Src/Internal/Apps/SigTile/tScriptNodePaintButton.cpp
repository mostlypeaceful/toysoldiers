//------------------------------------------------------------------------------
// \file tScriptNodePaintButton.cpp - 16 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigTilePch.hpp"
#include "tScriptNodePaintButton.hpp"
#include "tEditableTileDb.hpp"
#include "tTilePaintBrushButton.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tScriptPaintBrushButton
	//------------------------------------------------------------------------------
	class tScriptPaintBrush : public tTileBrushBase
	{
		tEditableTileDb* mDatabase;
		tEditableScriptNodeEntityPtr mScriptSlot;
		u32 mNodeGuid;

	public:
		tScriptPaintBrush( tScriptPaintBrushButton* button, tEditableTileDb* database, tTileCanvas* canvas, u32 guid )
			: tTileBrushBase( button, canvas )
			, mDatabase( database )
			, mNodeGuid( guid )
		{
			fMainWindow( ).fSetStatus( "Place Scripts" );
			fSyncCursor( );
		}

		~tScriptPaintBrush( )
		{
			if( mScriptSlot )
			{
				fHideCursor( );
				mScriptSlot.fRelease( );
			}
		}

		virtual void fOnTick( )
		{
			if( !fHandleHover( ) )
				return;

			// Putting the show here causes the tile to pop up only when the mouse cursor is
			// brought into the viewport. Also this brush isn't updated while in 3D mode so it
			// doesn't cause any newly-selected tiles to pop up.
			fShowCursor( );

			mCanvas->fSnapToGrid( mSelectPos, Math::tVec2u( 1, 1 ) );
			mSelectPos.y = mCanvas->fTileHeight( mSelectPos.x, mSelectPos.z ) + 0.5f;
			mScriptSlot->fMoveTo( mSelectPos );

			fHandleCursor( );
		}

		virtual void fRotateTile( b32 ccw ) { }

		virtual void fShowCursor( )
		{
			mScriptSlot->fShowPanel( fMainWindow( ).fGuiApp( ).fSceneGraph( )->fRootEntity( ) );
		}

		virtual void fHideCursor( )
		{
			mScriptSlot->fHidePanel( );
		}

		virtual void fSyncCursor( )
		{
			mScriptSlot = tEditableScriptNodeEntityPtr( new tEditableScriptNodeEntity( mNodeGuid, mDatabase ) );
		}

	protected:
		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			if( mScriptSlot )
			{
				fHideCursor( );
				mScriptSlot.fRelease( );
			}

			if( !nextController )
				return;

			tEditorButtonManagedCursorController::fOnNextCursor( nextController );
		}

		void fDoPaintAction( )
		{
			mCanvas->fPaintScriptPos( mScriptSlot, mSelectPos.x, mSelectPos.z );

			fSyncCursor( );
		}
	};

	tScriptPaintBrushButton::tScriptPaintBrushButton( 
		tEditorCursorControllerButtonGroup* parent,
		tTileCanvas* canvas,
		tEditableTileDb* database,
		u32 scriptNodeGuid,
		const char* defaultIcon,
		const char* tooltip )
		: tEditorCursorControllerButton( parent, wxBitmap( defaultIcon ), wxBitmap( defaultIcon ), tooltip )
		, mCanvas( canvas )
		, mDatabase( database )
		, mScriptNodeGuid( scriptNodeGuid )
	{
		const tEditableScriptNodeDef* node = mDatabase->fNodeByGuid( scriptNodeGuid );
		sigassert( node );
		fSetColor( node->fColor( ) );
	}

	tEditorCursorControllerPtr tScriptPaintBrushButton::fCreateCursorController( )
	{
		tScriptPaintBrush* paintCursor = new tScriptPaintBrush( this, mDatabase, mCanvas, mScriptNodeGuid );
		return tEditorCursorControllerPtr( paintCursor );
	}

	//------------------------------------------------------------------------------
	// tScriptEraseBrushButton
	//------------------------------------------------------------------------------
	class tScriptEraseBrush : public tTileBrushBase
	{
	public:

		tScriptEraseBrush( tScriptEraseBrushButton* button, tTileCanvas* canvas )
			: tTileBrushBase( button, canvas )
		{
			fMainWindow( ).fSetStatus( "Erase Scripts" );
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
			mCanvas->fEraseScript( mSelectPos.x, mSelectPos.z );
		}
	};


	tScriptEraseBrushButton::tScriptEraseBrushButton( tEditorCursorControllerButtonGroup* parent, tTileCanvas* canvas )
		: tEditorCursorControllerButton( parent, wxBitmap( "EraseTileSel" ), wxBitmap( "EraseTileDeSel" ), "Erase scripts" )
		, mCanvas( canvas )
	{
	}

	tEditorCursorControllerPtr tScriptEraseBrushButton::fCreateCursorController( )
	{
		tScriptEraseBrush* eraseCursor = new tScriptEraseBrush( this, mCanvas );
		return tEditorCursorControllerPtr( eraseCursor );
	}
}
