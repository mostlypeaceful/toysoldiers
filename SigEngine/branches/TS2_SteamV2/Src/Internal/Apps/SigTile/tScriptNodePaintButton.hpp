//------------------------------------------------------------------------------
// \file tScriptNodePaintButton.hpp - 16 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tScriptNodePaintButton__
#define __tScriptNodePaintButton__
#include "tEditorCursorControllerButton.hpp"

namespace Sig
{
	class tTileCanvas;
	class tEditableTileDb;

	///
	/// \class tScriptPaintBrushButton
	/// \brief 
	class tScriptPaintBrushButton : public tEditorCursorControllerButton
	{
		tTileCanvas* mCanvas;
		tEditableTileDb* mDatabase;
		u32 mScriptNodeGuid;

	public:
		tScriptPaintBrushButton( 
			tEditorCursorControllerButtonGroup* parent,
			tTileCanvas* canvas,
			tEditableTileDb* database,
			u32 scriptNodeGuid,
			const char* defaultIcon,
			const char* tooltip );
		~tScriptPaintBrushButton( ) { }

		virtual tEditorCursorControllerPtr fCreateCursorController( );
	};

	///
	/// \class tScriptEraseBrushButton
	/// \brief 
	class tScriptEraseBrushButton : public tEditorCursorControllerButton
	{
		tTileCanvas* mCanvas;

	public:
		tScriptEraseBrushButton( tEditorCursorControllerButtonGroup* parent, tTileCanvas* canvas );
		~tScriptEraseBrushButton( ) { }
		virtual tEditorCursorControllerPtr fCreateCursorController( );
	};
}

#endif //__tScriptNodePaintButton__
