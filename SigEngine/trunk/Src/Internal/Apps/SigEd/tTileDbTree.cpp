//------------------------------------------------------------------------------
// \file tTileDbTree.cpp - 30 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigEdPch.hpp"
#include "tTileDbTree.hpp"
#include "tWxSlapOnTextBox.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tWxSlapOnColorPicker.hpp"
#include "tTilePaintPanel.hpp"
#include "tEditableObjectContainer.hpp"

namespace Sig
{
	enum tAction
	{
		cActionRefreshBrowser = 1,
		cActionAddTileSet,
		cActionRefreshTileSet,
		cActionLoadTileSet,

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

		virtual void fOnControlUpdated( )
		{
			mThingToNotify->fOnPropertiesUpdated( );
		}
	};

	/////
	///// \class tNotifierSpinner
	///// \brief 
	//class tNotifierSpinner : public tWxSlapOnSpinner
	//{
	//	tItemWatcher* mThingToNotify;
	//public:
	//	tNotifierSpinner( wxWindow* parent, tItemWatcher* notifyTarget, const char* label, f32 min, f32 max, f32 increment, u32 precision )
	//		: tWxSlapOnSpinner( parent, label, min, max, increment, precision, 100 )
	//		, mThingToNotify( notifyTarget )
	//	{
	//		parent->GetSizer( )->Detach( mSizer );
	//		notifyTarget->Add( mSizer );
	//	}

	//	virtual void fOnControlUpdated( )
	//	{
	//		mThingToNotify->fOnPropertiesUpdated( );
	//	}
	//};

	/////
	///// \class tNotifierColorPicker
	///// \brief 
	//class tNotifierColorPicker : public tWxSlapOnColorPicker
	//{
	//	tItemWatcher* mThingToNotify;
	//public:
	//	tNotifierColorPicker( wxWindow* parent, tItemWatcher* notifyTarget, const char* label )
	//		: tWxSlapOnColorPicker( parent, label )
	//		, mThingToNotify( notifyTarget )
	//	{
	//		parent->GetSizer( )->Detach( mSizer );
	//		notifyTarget->Add( mSizer );

	//		mButton->SetSize( 50, 50 );
	//		mButton->SetMinSize( wxSize( 50, 50 ) );
	//		mButton->SetMaxSize( wxSize( 50, 50 ) );
	//	}

	//	virtual void fOnControlUpdated( )
	//	{
	//		mThingToNotify->fOnPropertiesUpdated( );
	//	}
	//};

	///
	/// \class tSetWatcher
	/// \brief 
	class tSetWatcher : public tItemWatcher
	{
		tEditableTileSet* mItem;
		tTiledmlPropPanel* mPropertiesPanel;
		tNotifierTextBox* mName;
	public:
		tSetWatcher( wxWindow* parent, tTiledmlPropPanel* propPanel ) 
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
			mItem->fSetFileName( mName->fGetValue( ) );

			mPropertiesPanel->fRebuildTree( );
		}
		void fShow( tEditableTileSet* set )
		{
			mItem = set;
			mName->fSetValue( set->fFileName( ) );

			mParentSizer->Show( this );
		}
	};

	///
	/// \class tTileWatcher
	/// \brief 
	class tTileWatcher : public tItemWatcher
	{
		tEditableTileDef* mItem;
		tTiledmlPropPanel* mPropertiesPanel;
		tNotifierTextBox* mTexture;
		tResourceDepotPtr mResourceDepot;

	private:
		void fOnBrowseForTexture( wxCommandEvent& )
		{
			const std::string ext( ".tga" );

			// browse for a new path
			tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
				NULL, 
				"Select Texture",
				wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
				wxEmptyString,
				wxString( "*" + ext ),
				wxFD_OPEN ) );

			if( openFileDialog->ShowModal( ) == wxID_OK )
			{
				const tFilePathPtr absPath = tFilePathPtr( openFileDialog->GetPath( ).c_str( ) );
				const tFilePathPtr relPath = ToolsPaths::fMakeResRelative( absPath );
				mTexture->fSetValue( relPath.fCStr( ) );
				mTexture->fOnControlUpdated();
			}
		}

	public:
		tTileWatcher( wxWindow* parent, tTiledmlPropPanel* propPanel, const tResourceDepotPtr& resourceDepot ) 
			: tItemWatcher( parent )
			, mItem( NULL )
			, mPropertiesPanel( propPanel )
			, mTexture( NULL )
			, mResourceDepot( resourceDepot )
		{
			mTexture = new tNotifierTextBox( parent, this, "Texture" );
			wxButton* browse = new wxButton( parent, wxID_ANY, "...", wxDefaultPosition, wxSize( 22, 20 ) );
			browse->SetForegroundColour( wxColour( 0x22, 0x22, 0xff ) );
			mTexture->fAddWindowToSizer( browse, true );
			browse->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTileWatcher::fOnBrowseForTexture ), NULL, this );
			fHide( );
		}
		virtual void fOnPropertiesUpdated( )
		{
			tFilePathPtr filePath( mTexture->fGetValue() );
			mItem->fLoadAndSetTexture( filePath, mResourceDepot );

		}
		void fShow( tEditableTileDef* tile )
		{
			mItem = tile;

			if( tile->mTileAssetPath.fExists() )
				mTexture->fSetValue( tile->mTileAssetPath.fCStr() );
			else
				mTexture->fSetValue( "" );

			mParentSizer->Show( this );
		}
	};


	//------------------------------------------------------------------------------
	// tTiledmlPropPanel
	//------------------------------------------------------------------------------
	tTiledmlPropPanel::tTiledmlPropPanel( wxWindow* parent, const char* label, tTiledmlTree* tree, wxWindow* topLevel, const tResourceDepotPtr& resourceDepot )
		: tWxSlapOnGroup( parent, label, false )
		, mDbTree( tree )
		, mTopLevelResizer( topLevel )
		, mCurrent( NULL )
	{
		mSet = new tSetWatcher( fGetMainPanel( ), this );
		mTile = new tTileWatcher( fGetMainPanel( ), this, resourceDepot );
	}

	tTiledmlPropPanel::~tTiledmlPropPanel( )
	{
	}

	void tTiledmlPropPanel::fHideAll( )
	{
		mTopLevelResizer->Freeze( );
		if( mCurrent )
			mCurrent->fHide( );

		mCurrent = NULL;

		mTopLevelResizer->Layout( );
		mTopLevelResizer->Thaw( );
	}

	void tTiledmlPropPanel::fConfigureForSet( tEditableTileSet* set )
	{
		mTopLevelResizer->Freeze( );

		if( mCurrent )
			mCurrent->fHide( );

		mCurrent = mSet;
		mSet->fShow( set );

		mTopLevelResizer->Layout( );
		mTopLevelResizer->Thaw( );
	}

	void tTiledmlPropPanel::fConfigureForTile( tEditableTileDef* tile )
	{
		mTopLevelResizer->Freeze( );

		if( mCurrent )
			mCurrent->fHide( );

		mCurrent = mTile;
		mTile->fShow( tile );

		mTopLevelResizer->Layout( );
		mTopLevelResizer->Thaw( );
	}

	void tTiledmlPropPanel::fRebuildTree( )
	{
		mDbTree->fRefresh( );
	}

	//------------------------------------------------------------------------------
	// tTiledmlTree
	//------------------------------------------------------------------------------
	tTiledmlTree::tTiledmlTree( wxWindow* parent, tTiledmlPanel* dbPanel, tEditableTiledml* database )
		: wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxSize( -1, 200 ) )
		, mDatabase( database )
		, mDbPanel( dbPanel )
	{
		SetBackgroundColour( wxColour( 0xee, 0xee, 0xee ) );
		SetForegroundColour( wxColour( 0x00, 0x00, 0x00 ) );
		Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler(tTiledmlTree::fOnSelChanged), 0, this );
		//Connect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler(tTileDbTree::fOnSelChanged), 0, this );	
		Connect( wxEVT_COMMAND_TREE_ITEM_RIGHT_CLICK, wxTreeEventHandler(tTiledmlTree::fOnItemRightClick), 0, this );
	}

	wxColour tTiledmlTree::fProvideCustomEntryColor( const wxTreeItemData* data )
	{
		return wxColour( 0x00, 0x00, 0x00 );
	}

	void tTiledmlTree::fRefresh ( )
	{
		Freeze( );

		// save off the expanded directory names
		tGrowableArray< std::string > expandedNodes;
		fTrackExpanded( expandedNodes, GetRootItem() );

		DeleteAllItems( );
		Expand( fRebuild() );

		// now re-expand saved directories
		fReExpand( expandedNodes, GetRootItem() );

		Thaw( );
	}

	std::string tTiledmlTree::fGetAbsoluteItemName( wxTreeItemId id ) const
	{
		if( !id.IsOk( ) ) return "";

		std::string name = GetItemText( id ).c_str( );

		for( wxTreeItemId parent = GetItemParent( id ); parent.IsOk( ); parent = GetItemParent( parent ) )
			name = GetItemText( parent ) + "." + name;

		return name;
	}

	void tTiledmlTree::fTrackExpanded( tGrowableArray< std::string >& expandedNodes, wxTreeItemId root )
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

	void tTiledmlTree::fReExpand( const tGrowableArray< std::string >& expandedNodes, wxTreeItemId root )
	{
		if( !root.IsOk( ) ) return;

		const std::string name = fGetAbsoluteItemName( root );
		if( expandedNodes.fFind( name ) )
			Expand( root );

		wxTreeItemIdValue cookie;
		for( wxTreeItemId ichild = GetFirstChild( root, cookie ); ichild.IsOk( ); ichild = GetNextChild( ichild, cookie ) )
			fReExpand( expandedNodes, ichild );
	}

	wxTreeItemId tTiledmlTree::fRebuild( )
	{
		wxTreeItemId treeRoot = AddRoot( "Tile Sets" );

		for( u32 j = 0; j < mDatabase->fNumTileSets( ); ++j )
		{
			tEditableTileSet* thisSet = mDatabase->fTileSet( j );

			tSetEntryData* set = new tSetEntryData( this, thisSet );

			std::string displayName = thisSet->fFileName( );
			if( !thisSet->fLoaded() )
				displayName += " - UNLOADED";

			wxTreeItemId setRoot = AppendItem( treeRoot, displayName, -1, -1, set );

			for( u32 k = 0; k < thisSet->fNumTileTypeLists(); ++k )
			{
				tEditableTileTypeList* tileList = thisSet->fTileTypeList( (tTileTypes)k );

				wxTreeItemId typeRoot = AppendItem( setRoot, tileList->fTypeName() );

				for( u32 l = 0; l < tileList->fNumTileDefs(); ++l )
				{
					tTileEntryData* data = new tTileEntryData( this, tileList->fTileDef( l ) );
					AppendItem( typeRoot, tileList->fTileDef( l )->mShortName, -1, -1, data );
				}
			}
		}

		return treeRoot;
	}

	void tTiledmlTree::fOnSelChanged( wxTreeEvent& event )
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

	void tTiledmlTree::fOnItemRightClick( wxTreeEvent& event )
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
			menu.AppendSeparator();
			menu.Append( cActionRefreshTileSet, _T("Rebuild Tile Set") );

			if( !tileSetData->fTileSet()->fLoaded() )
				menu.Append( cActionLoadTileSet, _T("Load Tile Set") );
		}
		else if( tileData )
		{
		}
		else if( mRightClickItem == GetRootItem( ) )
		{
			menu.AppendSeparator();
			menu.Append( cActionAddTileSet, _T("Add Tile Set") );
		}

		// Present menu.
		PopupMenu(&menu, event.GetPoint( ).x, event.GetPoint( ).y);

		event.Skip( );
	}

	void tTiledmlTree::fOnAction( wxCommandEvent& event )
	{
		if( !mRightClickItem.IsOk( ) )
			return;

		s32 actionId = event.GetId( );

		switch( actionId )
		{
		case cActionRefreshBrowser:
			{
				fRefresh( );

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
		case cActionRefreshTileSet:
			{
				wxTreeItemData* treeData = GetItemData( mRightClickItem );
				const tSetEntryData* setEntry = dynamic_cast< const tSetEntryData* >( treeData );

				if( setEntry )
				{
					tStrongPtr<wxMessageDialog> confirmDialog( new wxMessageDialog( 
						this,
						"Rebuilding a tile set may cause strange behavior with placed tiles relying on this set. Continue?", 
						"Confirm Rebuild Tile Set",
						wxYES_NO ) );

					if( confirmDialog->ShowModal( ) == wxID_YES )
					{
						setEntry->fTileSet()->fRebuild( mDbPanel->fTilePaintPanel()->fGuiApp().fEditableObjects().fGetResourceDepot() );

						fRefresh( );
					}
				}
			}
			break;
		case cActionLoadTileSet:
			{
				wxTreeItemData* treeData = GetItemData( mRightClickItem );
				const tSetEntryData* setEntry = dynamic_cast< const tSetEntryData* >( treeData );

				if( setEntry )
				{
					setEntry->fTileSet()->fLoad( mDbPanel->fTilePaintPanel()->fGuiApp().fEditableObjects().fGetResourceDepot() );
					fRefresh( );
					mDbPanel->fTilePaintPanel()->fRefreshSelectionDrop();
					mDatabase->fLoadTileAssets( mDbPanel->fTilePaintPanel()->fGuiApp().fEditableObjects().fGetResourceDepot() );
				}
			}
			break;
		default: { log_warning( "Unrecognized action!" ); }
				 event.Skip( );
				 break;
		}
	}

	BEGIN_EVENT_TABLE(tTiledmlTree, wxTreeCtrl)
		EVT_MENU(					cActionRefreshBrowser,		tTiledmlTree::fOnAction)
		EVT_MENU(					cActionAddTileSet,			tTiledmlTree::fOnAction)
		EVT_MENU(					cActionRefreshTileSet,		tTiledmlTree::fOnAction)
		EVT_MENU(					cActionLoadTileSet,			tTiledmlTree::fOnAction)
	END_EVENT_TABLE()
}
