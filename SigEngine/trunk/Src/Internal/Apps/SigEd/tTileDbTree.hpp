//------------------------------------------------------------------------------
// \file tTileDbTree.hpp - 30 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTileDbTree__
#define __tTileDbTree__
#include "tWxSlapOnGroup.hpp"
#include "wx/treectrl.h"

namespace Sig
{
	class tTiledmlTree;
	class tTiledmlPanel;

	class tEditableTiledml;
	class tEditableTileSet;
	struct tEditableTileDef;

	class tNotifierTextBox;
	class tNotifierSpinner;

	class tFamilyWatcher;
	class tSetWatcher;
	class tTileWatcher;

	class tItemWatcher : public wxEvtHandler, public wxBoxSizer
	{
	protected:
		wxSizer* mParentSizer;
	public:
		tItemWatcher( wxWindow* parent );
		virtual void fHide( )
		{
			mParentSizer->Hide( this );
		}
		virtual void fOnPropertiesUpdated( ) = 0;
	};

	///
	/// \class tTiledmlPropPanel
	/// \brief This panel hooks into the DbTree and shows properties specific to the
	/// currently selected item type.
	class tTiledmlPropPanel : public tWxSlapOnGroup
	{
		tTiledmlTree* mDbTree;
		wxWindow* mTopLevelResizer; //!< Needs to be whatever window needs to get resized to show the new options
		tItemWatcher* mCurrent;

		tFamilyWatcher* mFamily;
		tSetWatcher* mSet;
		tTileWatcher* mTile;

	public:
		tTiledmlPropPanel( wxWindow* parent, const char* label, tTiledmlTree* tree, wxWindow* topLevel, const tResourceDepotPtr& resourceDepot );
		~tTiledmlPropPanel( );

		void fHideAll( );

		void fConfigureForSet( tEditableTileSet* set );
		void fConfigureForTile( tEditableTileDef* tile );

		void fRebuildTree( );
	};

	///
	/// \class tTileDbTree
	/// \brief Shows an overview of the tile database and allows for some editing through right clicks.
	class tTiledmlTree : public wxTreeCtrl
	{
		tEditableTiledml*	mDatabase;
		tTiledmlPanel*		mDbPanel;
		wxTreeItemId		mRightClickItem;

	public:

		class tSetEntryData : public wxTreeItemData
		{
			tTiledmlTree* mOwner;
			tEditableTileSet* mSet;
		public:
			tSetEntryData( tTiledmlTree* owner, tEditableTileSet* set )
				: mOwner( owner ), mSet( set ) { }
			~tSetEntryData( ) { }
			wxColour fGetTextItemColor( ) const { return mOwner->fProvideCustomEntryColor( this ); }
			tEditableTileSet* fTileSet( ) const { return mSet; }
		};

		class tTileEntryData : public wxTreeItemData
		{
			tTiledmlTree* mOwner;
			tEditableTileDef* mTile;
		public:
			tTileEntryData( tTiledmlTree* owner, tEditableTileDef* tile )
				: mOwner( owner ), mTile( tile ) { }
			~tTileEntryData( ) { }
			wxColour fGetTextItemColor( ) const { return mOwner->fProvideCustomEntryColor( this ); }
			tEditableTileDef* fTileDef( ) const { return mTile; }
		};

		tTiledmlTree( wxWindow* parent, tTiledmlPanel* dbPanel, tEditableTiledml* database );

		wxColour fProvideCustomEntryColor( const wxTreeItemData* data );

		void fRefresh( );

		void fSelectDefaultTileSet( );

	private:

		std::string fGetAbsoluteItemName( wxTreeItemId id ) const;
		void fTrackExpanded( tGrowableArray< std::string >& expandedNodes, wxTreeItemId root );
		void fReExpand( const tGrowableArray< std::string >& expandedNodes, wxTreeItemId root );
		wxTreeItemId fRebuild( );

		void fOnSelChanged( wxTreeEvent& event );

		void fOnItemRightClick( wxTreeEvent& event );
		void fOnAction( wxCommandEvent& event );

		DECLARE_EVENT_TABLE()
	};
}

#endif __tTileDbTree__
