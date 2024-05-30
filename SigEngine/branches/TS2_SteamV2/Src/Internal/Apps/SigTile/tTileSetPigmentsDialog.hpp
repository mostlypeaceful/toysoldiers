//------------------------------------------------------------------------------
// \file tTileSetBrushesDialog.hpp - 01 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTileSetBrushesDialog__
#define __tTileSetBrushesDialog__
#include "tSigTileDialog.hpp"
#include "tEditableTileSetPigment.hpp"

namespace Sig
{
	class tSigTileMainWindow;
	class tTileSetPigmentsDialog;
	class tNotifySpinner;

	///
	/// \class tPigmentGui
	/// \brief 
	class tPigmentGui : public wxPanel
	{
		tTileSetPigmentsDialog* mParentDialog;
		wxSizer* mParentSizer;
		
		wxTextCtrl* mTextName;
		wxButton* mButtonColor;
		wxButton* mButtonAddTileSet;
		wxButton* mButtonRemoveTileSet;
		wxButton* mButtonDelete;
		tNotifySpinner* mHeight;
		tNotifySpinner* mSize;
		wxListBox* mListedTileSets;

		tEditableTileSetPigment* mPalette;

	public:
		tPigmentGui( wxWindow* parent, tTileSetPigmentsDialog* container, tEditableTileSetPigment* palette, u32 layerIndex );

		tEditableTileSetPigment*	fPalette( ) const { return mPalette; }
		std::string			fName( ) const;
		Math::tVec4f		fColor( ) const;
		f32					fHeight( ) const;
		f32					fSize( ) const;
		void				fSetColor( const Math::tVec4f& rgba );

		void fRefreshListedTileSets( );
		void fOnSpinnerChanged( );

	private:
		void fUpdateColorButton( );

		void fOnDeleteButtonPressed( wxCommandEvent& );
		void fOnColorButtonPressed( wxCommandEvent& );
		void fOnAddTileSetPressed( wxCommandEvent& );
		void fOnRemoveTileSetPressed( wxCommandEvent& );
		void fOnEnterPressed( wxCommandEvent& );
		void fOnTextFocus( wxFocusEvent& );
		void fOnTextLostFocus( wxFocusEvent& );

		void fOnPigmentChanged( );
	};

	///
	/// \class tTileSetPigmentsDialog
	/// \brief 
	class tTileSetPigmentsDialog : public tSigTileDialog
	{
		tSigTileMainWindow* mParent;
		wxScrolledWindow* mMainPanel;
		tGrowableArray<tPigmentGui*> mPalettes;

	public:
		tTileSetPigmentsDialog( tSigTileMainWindow* parent );

		void fClear( );

		void fRebuildPigments( );

		tSigTileMainWindow* fParent( ) { return mParent; }
		virtual bool Show( bool show = true );

		void fAddEmptyPigment( );
		void fDeletePigment( tPigmentGui* pigment );

		void fOnPigmentChanged( b32 markDirty );

	private:
		void fBuildGui( );

		void fOnNewPigmentPressed( wxCommandEvent& event );
	};

}

#endif//__tTileSetBrushesDialog__
