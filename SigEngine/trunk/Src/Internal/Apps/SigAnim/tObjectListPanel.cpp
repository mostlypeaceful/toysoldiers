//------------------------------------------------------------------------------
// \file tObjectListPanel.cpp - 25 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigAnimPch.hpp"
#include "tObjectListPanel.hpp"
#include "tWxColumnListBox.hpp"
#include "Animation/tSkeletableSgFileRefEntity.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "tSceneGraphFile.hpp"
#include "tSelectObjectsCursor.hpp"
#include "Sklml.hpp"
#include "tConfigurableBrowserTree.hpp"
#include "tWxSlapOnDialog.hpp"
#include "tSkeletonFile.hpp"
#include "tSklmlBrowser.hpp"

namespace Sig
{
	namespace
	{
		static const wxColor cLoadingColor( 0, 0, 0x77 );
		static const wxColor cNoSkeletonColor( 0x77, 0, 0 );

		enum tActionId
		{
			cActionDelete = 1,
			cActionSelectSkeleton,
		};
	}

	///
	/// \class tObjectListBox
	/// \brief 
	class tObjectListBox : public wxListCtrl
	{
	public:

		//------------------------------------------------------------------------------
		tObjectListBox( tToolsGuiApp & app, wxWindow * parent, u32 minHeight )
			: wxListCtrl( 
				parent, 
				wxID_ANY, 
				wxDefaultPosition, 
				wxSize( wxDefaultSize.x, minHeight ),
				wxLC_REPORT | wxLC_NO_HEADER | wxLC_SORT_ASCENDING )
			, mUpdating( false )
			, mRefreshColors( false )
			, mRefreshBrowser( true )
			, mApp( app )
		{
			//SetBackgroundColour( wxColour( 0xee, 0xee, 0xee ) );
			SetForegroundColour( wxColour( 0x00, 0x00, 0x00 ) );

			fBuildList( );

			tEditableObjectContainer & objContainer = app.fEditableObjects( );

			mOnObjectAdded.fFromMethod<tObjectListBox, &tObjectListBox::fOnObjectAdded>( this );
			objContainer.fGetObjectAddedEvent( ).fAddObserver( &mOnObjectAdded );

			mOnObjectRemoved.fFromMethod<tObjectListBox, &tObjectListBox::fOnObjectRemoved>( this );
			objContainer.fGetObjectRemovedEvent( ).fAddObserver( &mOnObjectRemoved );

			mOnSelectionChanged.fFromMethod<tObjectListBox, &tObjectListBox::fOnSelectionChanged>( this );
			objContainer.fGetSelectionList( ).fGetSelChangedEvent( ).fAddObserver( &mOnSelectionChanged );

			mOnResourceLoaded.fFromMethod<tObjectListBox, &tObjectListBox::fOnResourceLoaded>( this );

			mBrowser = new tSklmlBrowser( new wxDialog(this, wxID_ANY, "Select Skeleton...", wxDefaultPosition) );
		}

		void fRefresh( )
		{
			if( mRefreshBrowser )
			{
				mRefreshBrowser = false;
				mBrowser->fRefresh( );
			}

			if( mRefreshColors )	
			{
				mRefreshColors = false;

				Freeze( );

				const long count = GetItemCount( );
				for( long i = 0; i < count; ++i )
				{
					wxListItem item;
					item.SetId( i );
					item.SetMask( wxLIST_MASK_DATA );

					// Get the item
					if( !GetItem( item ) )
						continue;

					fSetupItemColor( item );

					// Set the item back
					SetItem( item );
				}

				Thaw( );
			}
		}

	private:

		//------------------------------------------------------------------------------
		void fOnObjectAdded( tEditableObjectContainer & container, const tEntityPtr & entityPtr )
		{
			tSkeletableSgFileRefEntity * entity = entityPtr->fDynamicCast<tSkeletableSgFileRefEntity>( );
			if( !entity )
				return;

			Freeze( );

			fAddObject( entity );
			SetColumnWidth( 0, wxLIST_AUTOSIZE );

			Thaw( );
		}

		//------------------------------------------------------------------------------
		void fOnObjectRemoved( tEditableObjectContainer & container, const tEntityPtr & entityPtr )
		{
			tSkeletableSgFileRefEntity * entity = entityPtr->fDynamicCast<tSkeletableSgFileRefEntity>( );
			if( !entity )
				return;
			
			Freeze( );

			fRemoveObject( entity );
			
			Thaw( );

		}

		//------------------------------------------------------------------------------
		void fOnSelectionChanged( tEditorSelectionList & list )
		{
			if( mUpdating )
				return;

			const long count = GetItemCount( );
			for( long i = 0; i < count; ++i )
			{
				tSkeletableSgFileRefEntity * entity = ( tSkeletableSgFileRefEntity * )GetItemData( i );
				u32 state = entity->fGetSelected( ) ? wxLIST_STATE_SELECTED : wxLIST_STATE_DONTCARE;

				mUpdating = true;
				SetItemState( i, state, -1 );
				mUpdating = false;
			}
		}

		//------------------------------------------------------------------------------
		void fOnResourceLoaded( tResource & theResource, b32 success )
		{
			mRefreshColors = true;
		}

		//------------------------------------------------------------------------------
		void fOnItemSelected( wxListEvent & e )
		{
			if( mUpdating )
				return;

			tSkeletableSgFileRefEntity * entity = (tSkeletableSgFileRefEntity *)e.GetItem( ).GetData( );

			mUpdating = true;
			mApp.fSelectionList( ).fAdd( tEntityPtr( entity ) );
			mUpdating = false;
		}

		//------------------------------------------------------------------------------
		void fOnItemDeselected( wxListEvent & e )
		{
			if( mUpdating )
				return;

			tSkeletableSgFileRefEntity * entity = (tSkeletableSgFileRefEntity *)e.GetItem( ).GetData( );

			mUpdating = true;
			mApp.fSelectionList( ).fRemove( tEntityPtr( entity ) );
			mUpdating = false;
		}

		//------------------------------------------------------------------------------
		void fOnRightClick( wxListEvent & e )
		{
			const wxListItem & item = e.GetItem( );

			wxMenu menu;
			menu.SetEventHandler( this );

			menu.Append( cActionDelete, _T("&Delete Object"));

			mSkeletonResource.fReset( 0 );
			tSkeletableSgFileRefEntity * entity = (tSkeletableSgFileRefEntity *)item.GetData( );
			if( !entity->fResource( )->fLoading( ) && !entity->fSkeletonResource( ) )
			{
				menu.Append( cActionSelectSkeleton, _T("&Select Skeleton...") );
			}

			GetParent( )->PopupMenu( &menu, e.GetPoint( ) );

			// A skeleton resource was picked from the menu
			if( mSkeletonResource )
			{
				entity->fSetSkeletonResource( mSkeletonResource );
				mSkeletonResource->fCallWhenLoaded( mOnResourceLoaded );
				mSkeletonResource.fReset( 0 );
			}
		}

		//------------------------------------------------------------------------------
		void fOnMenuSelected( wxCommandEvent & e )
		{
			switch( e.GetId( ) )
			{
			case cActionDelete:
				{
					mApp.fActionStack( ).fAddAction( tEditorActionPtr( new tDeleteSelectedObjectsAction( mApp.fMainWindow( ) ) ) );
				
				}break;
			case cActionSelectSkeleton:
				{
					if( mBrowser->fBrowse( ) )
					{
						tFilePathPtr file = mBrowser->fFilePath( );
						mSkeletonResource = mApp.fResourceDepot( )->fQuery( 
							tResourceId::fMake<tSkeletonFile>( 
								tSkeletonFile::fConvertToBinary( ToolsPaths::fMakeResRelative( file ) ) ) );
						
					}

				}break;
			}
		}

		//------------------------------------------------------------------------------
		void fBuildList( )
		{
			Freeze( );
			ClearAll( );

			InsertColumn( 0, wxListItem( ) );

			tGrowableArray<tSkeletableSgFileRefEntity *> ents;
			mApp.fEditableObjects( ).fCollectAllByType<tSkeletableSgFileRefEntity>( ents );

			const u32 count = ents.fCount( );
			for( u32 e = 0; e < count; ++e )
			{
				mUpdating = true;
				fAddObject( ents[ e ] );
				mUpdating = false;
			}

			SetColumnWidth( 0, wxLIST_AUTOSIZE );

			Thaw( );
		}

		//------------------------------------------------------------------------------
		void fAddObject( tSkeletableSgFileRefEntity * entity )
		{
			if( entity->fIsCursor( ) )
				return;

			tFilePathPtr path = entity->fResourcePath( );

			std::stringstream ss;
			ss	<< "[" 
				<< StringUtil::fStripExtension( StringUtil::fNameFromPath( path.fCStr( ) ).c_str( ) )
				<< "] "
				<< tSceneGraphFile::fConvertToSource( path ).fCStr( );

			wxListItem item;
			item.SetText( ss.str( ).c_str( ) );
			item.SetData( entity );

			fSetupItemColor( item );

			u32 state = entity->fGetSelected( ) ? wxLIST_STATE_SELECTED : wxLIST_STATE_DONTCARE;
			item.SetState( state );

			mUpdating = true;
			InsertItem( item );
			mUpdating = false;
		}

		//------------------------------------------------------------------------------
		void fRemoveObject( tSkeletableSgFileRefEntity * entity )
		{
			if( entity->fIsCursor( ) )
				return;

			long item = FindItem(-1, (wxUIntPtr)entity );
			if( item < GetItemCount( ) )
			{
				mUpdating = true;
				DeleteItem( item );
				mUpdating = false;
			}
		}

		//------------------------------------------------------------------------------
		void fSetupItemColor( wxListItem & item )
		{
			tSkeletableSgFileRefEntity * entity = (tSkeletableSgFileRefEntity *)item.GetData( );

			// Base resource is still loading
			if( entity->fResource( )->fLoading( ) )
			{
				entity->fResource( )->fCallWhenLoaded( mOnResourceLoaded );
				item.SetTextColour( cLoadingColor );
			}
			
			// Base resource is finished, but still no skeleton
			else if( !entity->fSkeletonResource( ) )
				item.SetTextColour( cNoSkeletonColor );

			// Has skeleton, but it's loading
			else if( entity->fSkeletonResource( )->fLoading( ) )
			{
				entity->fSkeletonResource( )->fCallWhenLoaded( mOnResourceLoaded );
				item.SetTextColour( cLoadingColor );
			}

			// All good to go
			else
				item.SetTextColour( GetTextColour( ) );

		}

	private:

		b32 mUpdating;
		b32 mRefreshColors;
		b32 mRefreshBrowser;

		tSklmlBrowser * mBrowser;
		tResourcePtr mSkeletonResource;

		tToolsGuiApp & mApp;
		tEditableObjectContainer::tOnObjectAdded::tObserver mOnObjectAdded;
		tEditableObjectContainer::tOnObjectRemoved::tObserver mOnObjectRemoved;
		tEditorSelectionList::tOnSelectionChanged::tObserver mOnSelectionChanged;
		tResource::tOnLoadComplete::tObserver mOnResourceLoaded;

		DECLARE_EVENT_TABLE()
	};

	BEGIN_EVENT_TABLE(tObjectListBox, wxListCtrl)
		EVT_LIST_ITEM_SELECTED(		wxID_ANY, tObjectListBox::fOnItemSelected )
		EVT_LIST_ITEM_DESELECTED(	wxID_ANY, tObjectListBox::fOnItemDeselected )
		EVT_LIST_ITEM_RIGHT_CLICK(	wxID_ANY, tObjectListBox::fOnRightClick )

		EVT_MENU( cActionDelete,			tObjectListBox::fOnMenuSelected )
		EVT_MENU( cActionSelectSkeleton,	tObjectListBox::fOnMenuSelected )
	END_EVENT_TABLE()


	//------------------------------------------------------------------------------
	tObjectListPanel::tObjectListPanel( tWxToolsPanel * parent )
		: tWxToolsPanelTool( parent, "Existing Object Browser", "Browse for object by name", "Objects" )
	{
		fGetMainPanel( )->SetForegroundColour( wxColor( 0xff, 0xff, 0xff ) );

		mListBox = new tObjectListBox( parent->fGuiApp( ), fGetMainPanel( ), 200 );
		fGetMainPanel( )->GetSizer( )->Add( mListBox, 0, wxEXPAND | wxALL, 5 );
	}

	//------------------------------------------------------------------------------
	void tObjectListPanel::fOnTick( )
	{
		mListBox->fRefresh( );
	}
}
