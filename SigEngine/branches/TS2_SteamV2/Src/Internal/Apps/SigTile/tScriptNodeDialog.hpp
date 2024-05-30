//------------------------------------------------------------------------------
// \file tScriptNodeDialog.hpp - 15 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tScriptNodeDialog__
#define __tScriptNodeDialog__
#include "tSigTileDialog.hpp"

namespace Sig
{
	class tWxSlapOnTextBox;
	class tScriptNodesDialog;
	class tEditableScriptNodeDef;

	///
	/// \class tScriptNodeGui
	/// \brief 
	class tScriptNodeGui : public wxPanel
	{
		tScriptNodesDialog* mParentDialog;
		wxSizer* mParentSizer;

		wxTextCtrl* mTextName;
		wxButton* mButtonColor;
		wxButton* mButtonDelete;
		tWxSlapOnTextBox* mScriptPathBox;
		tFilePathPtr mScriptPath;

		// TODO: pointer to data
		tEditableScriptNodeDef* mNode;

	public:
		tScriptNodeGui( wxWindow* parent, tScriptNodesDialog* container, tEditableScriptNodeDef* node, u32 layerIndex );

		tEditableScriptNodeDef* fNode( ) const { return mNode; }
		std::string				fName( ) const;
		Math::tVec4f			fColor( ) const;
		tFilePathPtr			fScriptPath( ) const;

	private:
		void fSetColor( const Math::tVec4f& rgba );

		void fUpdateColorButton( );

		void fOnDeleteButtonPressed( wxCommandEvent& );
		void fOnColorButtonPressed( wxCommandEvent& );
		void fOnEnterPressed( wxCommandEvent& );
		void fOnTextFocus( wxFocusEvent& );
		void fOnTextLostFocus( wxFocusEvent& );
		void fOnBrowse( wxCommandEvent& evt );

		void fOnNodeChanged( );
	};

	///
	/// \class tScriptNodesDialog
	/// \brief 
	class tScriptNodesDialog : public tSigTileDialog
	{
		tSigTileMainWindow* mParent;
		wxScrolledWindow* mMainPanel;
		tGrowableArray<tScriptNodeGui*> mNodes;

	public:
		tScriptNodesDialog( tSigTileMainWindow* parent );

		void fClear( );

		void fRebuildNodes( );

		tSigTileMainWindow* fParent( ) { return mParent; }
		virtual bool Show( bool show = true );

		void fAddEmptyNode( );
		void fDeleteNode( tScriptNodeGui* node );

		void fOnNodeChanged( b32 markDirty );

	private:
		void fBuildGui( );

		void fOnNewNodePressed( wxCommandEvent& event );
	};
}

#endif //__tScriptNodeDialog__
