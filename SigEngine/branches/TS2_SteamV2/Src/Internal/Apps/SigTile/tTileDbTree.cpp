//------------------------------------------------------------------------------
// \file tTileDbTree.cpp - 30 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigTilePch.hpp"
#include "tTileDbTree.hpp"
#include "tTileDbPanel.hpp"
#include "tWxSlapOnTextBox.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tWxSlapOnColorPicker.hpp"
#include "tTilePaintPanel.hpp"

namespace Sig
{
	enum tAction
	{
		cActionRefreshBrowser = 1,
		cActionAddTileSet,
		cActionAddTileSetToFamily, 
		cActionRemoveTileSet,
		cActionAddFamily,

		cNumActions,
	};

	tItemWatcher::tItemWatcher( wxWindow* parent ) 
		: wxBoxSizer( wxVERTICAL )
		, mParentSizer( parent->GetSizer( ) ) 
	{
		mParentSizer->Add( this, 0, wxEXPAND | wxBOTTOM, 5 ); 
	}

	///
	/// \class tNotifierTextBox
	/// \brief 
	class tNotifierTextBox : public tWxSlapOnTextBox
	{
		tItemWatcher* mThingToNotify;
	public:
		tNotifierTextBox( wxWindow* parent, tItemWatcher* notifyTarget, const char* label )
			: tWxSlapOnTextBox( parent, label, 150 )
			, mThingToNotify( notifyTarget )
		{
			parent->GetSizer( )->Detach( mSizer );
			notifyTarget->Add( mSizer );
		}

	protected:
		virtual void fOnControlUpdated( )
		{
			mThingToNotify->fOnPropertiesUpdated( );
		}
	};

	///
	/// \class tNotifierSpinner
	/// \brief 
	class tNotifierSpinner : public tWxSlapOnSpinner
	{
		tItemWatcher* mThingToNotify;
	public:
		tNotifierSpinner( wxWindow* parent, tItemWatcher* notifyTarget, const char* label, f32 min, f32 max, f32 increment, u32 precision )
			: tWxSlapOnSpinner( parent, label, min, max, increment, precision, 100 )
			, mThingToNotify( notifyTarget )
		{
			parent->GetSizer( )->Detach( mSizer );
			notifyTarget->Add( mSizer );
		}

	protected:
		virtual void fOnControlUpdated( )
		{
			mThingToNotify->fOnPropertiesUpdated( );
		}
	};

	///
	/// \class tNotifierColorPicker
	/// \brief 
	class tNotifierColorPicker : public tWxSlapOnColorPicker
	{
		tItemWatcher* mThingToNotify;
	public:
		tNotifierColorPicker( wxWindow* parent, tItemWatcher* notifyTarget, const char* label )
			: tWxSlapOnColorPicker( parent, label )
			, mThingToNotify( notifyTarget )
		{
			parent->GetSizer( )->Detach( mSizer );
			notifyTarget->Add( mSizer );

			mButton->SetSize( 50, 50 );
			mButton->SetMinSize( wxSize( 50, 50 ) );
			mButton->SetMaxSize( wxSize( 50, 50 ) );
		}

	protected:
		virtual void fOnControlUpdated( )
		{
			mThingToNotify->fOnPropertiesUpdated( );
		}
	};

	///
	/// \class tSetWatcher
	/// \brief 
	class tSetWatcher : public tItemWatcher
	{
		tEditableTileSet* mItem;
		tTileDbPropPanel* mPropertiesPanel;
		tNotifierTextBox* mName;
	public:
		tSetWatcher( wxWindow* parent, tTileDbPropPanel* propPanel ) 
			: tItemWatcher( parent )
			, mItem( NULL )
			, mPropertiesPanel( propPanel )
			, mName( NULL )
		{
			mName = new tNotifierTextBox( parent, this, "Name" );

			fHide( );
		}
		virtual void fOnPropertiesUpdated( )
		{
			mItem->fSetName( mName->fGetValue( ) );

			mPropertiesPanel->fRebuildTree( );
		}
		void fShow( tEditableTileSet* set )
		{
			mItem = set;
			mName->fSetValue( set->fName( ) );

			mParentSizer->Show( this );
		}
	};

	///
	/// \class tTileWatcher
	/// \brief 
	class tTileWatcher : public tItemWatcher
	{
		tEditableTileDef* mItem;
		tTileDbPropPanel* mPropertiesPanel;
		tNotifierTextBox* mTexture;
	public:
		tTileWatcher( wxWindow* parent, tTileDbPropPanel* propPanel ) 
			: tItemWatcher( parent )
			, mItem( NULL )
			, mPropertiesPanel( propPanel )
			, mTexture( NULL )
		{
			mTexture = new tNotifierTextBox( parent, this, "Texture" );
			fHide( );
		}
		virtual void fOnPropertiesUpdated( )
		{
			// TODO: reload texture

			mPropertiesPanel->fRebuildTree( );
		}
		void fShow( tEditableTileDef* tile )
		{
			mItem = tile;

			if( !tile->mTileResource.fNull( ) )
				mTexture->fSetValue( tile->mTileAssetPath.fCStr( ) );
			else
				mTexture->fSetValue( "" );

			mParentSizer->Show( this );
		}
	};


	//------------------------------------------------------------------------------
	// tTileDbPropPanel
	//------------------------------------------------------------------------------
	tTileDbPropPanel::tTileDbPropPanel( wxWindow* parent, const char* label, tTileDbTree* tree, wxWindow* topLevel )
		: tWxSlapOnGroup( parent, label, false )
		, mDbTree( tree )
		, mTopLevelResizer( topLevel )
		, mCurrent( NULL )
	{
		mSet = new tSetWatcher( fGetMainPanel( ), this );
		mTile = new tTileWatcher( fGetMainPanel( ), this );
	}

	tTileDbPropPanel::~tTileDbPropPanel( )
	{
	}

	void tTileDbPropPanel::fHideAll( )
	{
		mTopLevelResizer->Freeze( );
		if( mCurrent )
			mCurrent->fHide( );

		mCurrent = NULL;

		mTopLevelResizer->Layout( );
		mTopLevelResizer->Thaw( );
	}

	void tTileDbPropPanel::fConfigureForSet( tEditableTileSet* set )
	{
		mTopLevelResizer->Freeze( );

		if( mCurrent )
			mCurrent->fHide( );

		mCurrent = mSet;
		mSet->fShow( set );

		mTopLevelResizer->Layout( );
		mTopLevelResizer->Thaw( );
	}

	void tTileDbPropPanel::fConfigureForTile( tEditableTileDef* tile )
	{
		mTopLevelResizer->Freeze( );

		if( mCurrent )
			mCurrent->fHide( );

		mCurrent = mTile;
		mTile->fShow( tile );

		mTopLevelResizer->Layout( );
		mTopLevelResizer->Thaw( );
	}

	void tTileDbPropPanel::fRebuildTree( )
	{
		mDbTree->fRefresh( );
	}

	//------------------------------------------------------------------------------
	// tTileDbTree
	//------------------------------------------------------------------------------
	tTileDbTree::tTileDbTree( wxWindow* parent, tTileDbPanel* dbPanel, tEditableTileDb* database )
		: wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxSize( -1, 200 ) )
		, mDatabase( database )
		, mDbPanel( dbPanel )
	{
		SetBackgroundColour( wxColour( 0xee, 0xee, 0xee ) );
		SetForegroundColour( wxColour( 0x00, 0x00, 0x00 ) );
		Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler(tTileDbTree::fOnSelChanged), 0, this );
		//Connect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler(tTileDbTree::fOnSelChanged), 0, this );	
		Connect( wxEVT_COMMAND_TREE_ITEM_RIGHT_CLICK, wxTreeEventHandler(tTileDbTree::fOnItemRightClick), 0, this );
	}

	wxColour tTileDbTree::fProvideCustomEntryColor( const wxTreeItemData* data )
	{
		return wxColour( 0x00, 0x00, 0x00 );
	}

	void tTileDbTree::fRefresh ( )
	{
		Freeze( );

		// save off the expanded directory names
		tGrowableArray< std::string > expandedNodes;
		fTrackExpanded( expandedNodes, GetRootItem( ) );

		DeleteAllItems( );
		Expand( fRebuild( ) );

		// now re-expand saved directories
		fReExpand( expandedNodes, GetRootItem( ) );

		Thaw( );
	}

	std::string tTileDbTree::fGetAbsoluteItemName( wxTreeItemId id ) const
	{
		if( !id.IsOk( ) ) return "";

		std::string name = GetItemText( id ).c_str( );

		for( wxTreeItemId parent = GetItemParent( id ); parent.IsOk( ); parent = GetItemParent( parent ) )
			name = GetItemText( parent ) + "." + name;

		return name;
	}

	void tTileDbTree::fTrackExpanded( tGrowableArray< std::string >& expandedNodes, wxTreeItemId root )
	{
		if( !root.IsOk( ) ) 
			return;

		const std::string name = fGetAbsoluteItemName( root );
		if( IsExpanded( root ) )
			expandedNodes.fFindOrAdd( name );

		wxTreeItemIdValue cookie;
		for( wxTreeItemId ichild = GetFirstChild( root, cookie ); ichild.IsOk( ); ichild = GetNextChild( ichild, cookie ) )
			fTrackExpanded( expandedNodes, ichild );
	}

	void tTileDbTree::fReExpand( const tGrowableArray< std::string >& expandedNodes, wxTreeItemId root )
	{
		if( !root.IsOk( ) ) return;

		const std::string name = fGetAbsoluteItemName( root );
		if( expandedNodes.fFind( name ) )
			Expand( root );

		wxTreeItemIdValue cookie;
		for( wxTreeItemId ichild = GetFirstChild( root, cookie ); ichild.IsOk( ); ichild = GetNextChild( ichild, cookie ) )
			fReExpand( expandedNodes, ichild );
	}

	wxTreeItemId tTileDbTree::fRebuild( )
	{
		wxTreeItemId treeRoot = AddRoot( "Families" );

		tHashTable< std::string, wxTreeItemId> familyMap;

		for( u32 j = 0; j < mDatabase->fNumTileSets( ); ++j )
		{
			tEditableTileSet* thisSet = mDatabase->fTileSet( j );
			std::string thisSetFamily = thisSet->fFamily( );

			tSetEntryData* set = new tSetEntryData( this, thisSet );
			wxTreeItemId* familyId = familyMap.fFind( thisSetFamily );
			if( !familyId )
				familyId = familyMap.fInsert( thisSetFamily, AppendItem( treeRoot, thisSetFamily ) );

			wxTreeItemId setRoot = AppendItem( *familyId, thisSet->fName( ), -1, -1, set );

			for( u32 k = 0; k < cNumTileTypes; ++k )
			{
				tEditableTileTypeList* tileList = thisSet->fTileTypeList( (tTileTypes)k );

				wxTreeItemId typeRoot = AppendItem( setRoot, tileList->fTypeName( ) );

				for( u32 l = 0; l < tileList->fNumTileDefs( ); ++l )
				{
					tTileEntryData* data = new tTileEntryData( this, tileList->fTileDef( l ) );
					AppendItem( typeRoot, tileList->fTileDef( l )->mShortName, -1, -1, data );
				}
			}
		}

		return treeRoot;
	}

	void tTileDbTree::fOnSelChanged( wxTreeEvent& event )
	{
		const wxTreeItemId id = event.GetItem( );
		if( !id.IsOk( ) )
			return;

		wxTreeItemData* treeData = GetItemData( id );
		const tSetEntryData* tileSetData = dynamic_cast< tSetEntryData* >( treeData );
		if( tileSetData )
		{
			mDbPanel->fConfigureForSet( tileSetData->fTileSet( ) );
			return;
		}

		const tTileEntryData* tileData = dynamic_cast< tTileEntryData* >( treeData );
		if( tileData )
		{
			mDbPanel->fConfigureForTile( tileData->fTileDef( ) );
			return;
		}

		mDbPanel->fHideAll( );
	}

	void tTileDbTree::fOnItemRightClick( wxTreeEvent& event )
	{
		// Save the item for use in response functions.
		mRightClickItem = event.GetItem( );

		// Always allow refreshing the browser.
		wxMenu menu;
		menu.Append( cActionRefreshBrowser, _T("Refresh browser") );

		wxTreeItemData* treeData = GetItemData( mRightClickItem );
		const tSetEntryData* tileSetData = dynamic_cast< tSetEntryData* >( treeData );
		const tTileEntryData* tileData = dynamic_cast< tTileEntryData* >( treeData );

		if( tileSetData )
		{
			//menu.AppendSeparator( );
			//menu.Append( cActionRemoveTileSet, _T("Remove Tile Set") );
		}
		else if( tileData )
		{
		}
		else if( GetItemParent( mRightClickItem ) == GetRootItem( ) )
		{
			// These are families.
			menu.AppendSeparator( );
			menu.Append( cActionAddTileSetToFamily, _T("Add Tile Set To Family") );
		}
		else if( mRightClickItem == GetRootItem( ) )
		{
			menu.AppendSeparator( );
			menu.Append( cActionAddTileSet, _T("Add Tile Set") );
			//menu.Append( cActionAddFamily, _T("Add Family") );
		}

		// Present menu.
		PopupMenu(&menu, event.GetPoint( ).x, event.GetPoint( ).y);

		event.Skip( );
	}

	void tTileDbTree::fOnAction( wxCommandEvent& event )
	{
		if( !mRightClickItem.IsOk( ) )
			return;

		s32 actionId = event.GetId( );

		switch( actionId )
		{
		case cActionRefreshBrowser:
			{
				// Add/remove any tile defs to keep the database up to date.
				mDatabase->fRebuild( mDbPanel->fMainWindow( )->fGuiApp( ).fResourceDepot( ) );

				fRefresh( );

				mDbPanel->fMainWindow( )->fTilePaintPanel( )->fRefreshIndividualsDisplay( );

				// Clear right click item.
				mRightClickItem = wxTreeItemId( );
			}
			break;
		case cActionAddTileSet:
			{
				mDbPanel->fAddTileSet( );

				// Clear right click item.
				mRightClickItem = wxTreeItemId( );
			}
			break;
		case cActionAddTileSetToFamily:
			{
				wxString itemLabel = GetItemText( mRightClickItem );
				mDbPanel->fAddTileSet( itemLabel.c_str( ) );

				// Clear right click item.
				mRightClickItem = wxTreeItemId( );
			}
			break;
		case cActionRemoveTileSet:
			{
				wxTreeItemData* treeData = GetItemData( mRightClickItem );
				const tSetEntryData* setEntry = dynamic_cast< const tSetEntryData* >( treeData );

				if( setEntry )
				{
					tStrongPtr<wxMessageDialog> confirmDialog( new wxMessageDialog( 
						this,
						"Do you want to remove this tile set?", 
						"Confirm Remove Tile Set",
						wxYES_NO ) );

					if( confirmDialog->ShowModal( ) == wxID_YES )
					{
						tEditableTileSet& set = *setEntry->fTileSet( );

						fRefresh( );
					}
				}
			}
			break;
		case cActionAddFamily:
			{
			}
			break;
		default: { log_warning( 0, "Unrecognized action!" ); }
				 event.Skip( );
				 break;
		}
	}

	BEGIN_EVENT_TABLE(tTileDbTree, wxTreeCtrl)
		EVT_MENU(					cActionRefreshBrowser,		tTileDbTree::fOnAction)
		EVT_MENU(					cActionAddTileSet,			tTileDbTree::fOnAction)
		EVT_MENU(					cActionAddTileSetToFamily,	tTileDbTree::fOnAction)
		EVT_MENU(					cActionRemoveTileSet,		tTileDbTree::fOnAction)
		EVT_MENU(					cActionAddFamily,			tTileDbTree::fOnAction)
	END_EVENT_TABLE()
}
