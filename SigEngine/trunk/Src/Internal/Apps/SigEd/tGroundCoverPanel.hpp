//------------------------------------------------------------------------------
// \file tGroundCoverPanel.hpp - 08 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tGroundCoverPanel__
#define __tGroundCoverPanel__
#include "tEditableObjectContainer.hpp"
#include "tWxToolsPanel.hpp"
#include "Sigml.hpp"

namespace Sig
{
	class tGroundCoverLayerGui;
	class tGroundCoverLayerDialog;
	class tGroundCoverPaintDensity;
	class tWxToolsPanelSlider;
	class tEditorAppWindow;
	class tHeightFieldPaintCursor;

	///
	/// \class tGroundCoverPanel
	/// \brief 
	class tGroundCoverPanel : public tWxToolsPanelTool
	{
	public:

		tGroundCoverPanel( tEditorAppWindow * window, tWxToolsPanel * parent );
		
		u32 fNumLayers( ) { return mLayers.fCount( ); }
		void fDeleteLayer( tGroundCoverLayerGui * layer );
		b32 fRenameLayer( tGroundCoverLayerGui * layer, const std::string & title );
		void fShowProperties( tGroundCoverLayerGui * layer );
		void fSelectLayer( tGroundCoverLayerGui * layer );

		tGroundCoverLayerGui * fFindLayer( u32 id );
		tGroundCoverLayerGui * fSelectedLayer( ) const { return mSelectedLayer; }

		void fSetDialogInputActive( b32 active ) { mDialogInputActive = active; }

		virtual void fOnTick( );

		void fReset( );
		void fReset( const Sigml::tFile& file );
		void fSave( Sigml::tFile & file );

		void fMarkDirty( );

		void fOnSlidersChanged( );
		void fUpdateCursorValues( tGroundCoverPaintDensity * cursor );
		void fRestoreLayer( const Sigml::tGroundCoverLayer & layer );

		void fAddCursorHotKeys( tHeightFieldPaintCursor* cursorBase );
		void fUpdateParametersOnCursor( tHeightFieldPaintCursor* cursor );
		void fNudgeCursorSize( f32 delta );
		void fNudgeCursorStrength( f32 delta );
		void fNudgeCursorFalloff( f32 delta );
		void fNudgeCursorShape( f32 delta );
		void fToggleCursorGrid( );

		

	private:

		void fOnRefreshHeights( wxCommandEvent & );
		void fOnAddLayerPressed( wxCommandEvent & );
		void fOnObjectAdded( tEditableObjectContainer &, const tEntityPtr & e );
		void fBeginBuildUI( );
		void fEndBuildUI( );

		b32 fTitleIsAvailable( const char * title, tGroundCoverLayerGui * ignore = NULL );

	private:

		tGroundCoverLayerGui * mSelectedLayer;
		tGrowableArray<tGroundCoverLayerGui *> mLayers;
		tGroundCoverLayerDialog * mPropertiesDialog;

		tWxToolsPanelSlider * mSizeSlider;
		tWxToolsPanelSlider * mStrengthSlider;
		tWxToolsPanelSlider * mFalloffSlider;
		tWxToolsPanelSlider * mShapeSlider;
		wxCheckBox *		  mRenderCursorGridCheckBox;

		tWxSlapOnGroup *	mLayerGroup;
		u32					mBaseLayerSizerIndex;
		b32					mDialogInputActive;
		b32					mRefreshDialog;
		b32					mWantsDialog;

		tEditableObjectContainer::tOnObjectAdded::tObserver mOnObjectAdded;
	};
}

#endif//__tGroundCoverPanel__
