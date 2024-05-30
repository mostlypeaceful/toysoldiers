//------------------------------------------------------------------------------
// \file tAnimationTreePanel.cpp - 24 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigAnimPch.hpp"
#include "tAnimationTreePanel.hpp"
#include "Animation\tSkeletableSgFileRefEntity.hpp"
#include "tSigAnimMainWindow.hpp"
#include "wx/treectrl.h"
#include "Editor/tEditableObjectContainer.hpp"
#include "tAnimPackFile.hpp"
#include "tKeyFrameAnimTrack.hpp"
#include "tSigAnimEdDialog.hpp"
#include "tSigAnimTimeline.hpp"

namespace Sig
{
	namespace
	{
		static const wxColor cLoadingColor( 0, 0, 0x77 );
	}

	class tAnimPackTreeData : public wxTreeItemData
	{
	public:

		tAnimPackTreeData( const tResourcePtr & res )
			: mAnimPackResource( res ), mLoading( res->fLoading( ) ) { }

		tResourcePtr mAnimPackResource;
		b32 mLoading;
	};

	///
	/// \class tAnimPackTree
	/// \brief 
	class tAnimPackTree : public wxTreeCtrl, public tSigAnimEdDialog::tOwner
	{
	public:
		//------------------------------------------------------------------------------
		tAnimPackTree(
			tEditableObjectContainer & objContainer,
			wxWindow * parent,
			wxWindow * mainWindow,
			wxTextCtrl * packFilter,
			wxTextCtrl * animFilter,
			u32 minHeight )
			: wxTreeCtrl( 
				parent, wxID_ANY, wxDefaultPosition, 
				wxSize( wxDefaultSize.x, minHeight ), 
				wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_HIDE_ROOT | wxTR_SINGLE  )
			, mObjectContainer( objContainer )
			, mPackFilter( packFilter )
			, mAnimFilter( animFilter )
			, mSelectedAnimPack( -1 )
			, mPartialsMatch( false )
			, mAnimTimeline( 0 )
		{
			SetBackgroundColour( wxColour( 0xee, 0xee, 0xee ) );
			SetForegroundColour( wxColour( 0x00, 0x00, 0x00 ) );

			mAnimEd = new tSigAnimEdDialog( *this, mainWindow );

			mOnObjectAddedOrRemoved.fFromMethod<tAnimPackTree, &tAnimPackTree::fOnObjectAddedOrRemoved>( this );
			mObjectContainer.fGetObjectAddedEvent( ).fAddObserver( &mOnObjectAddedOrRemoved );
			mObjectContainer.fGetObjectRemovedEvent( ).fAddObserver( &mOnObjectAddedOrRemoved );
			
			mOnSelectionChanged.fFromMethod<tAnimPackTree, &tAnimPackTree::fOnSelectionChanged>( this );
			mObjectContainer.fGetSelectionList( ).fGetSelChangedEvent( ).fAddObserver( &mOnSelectionChanged );
			fOnSelectionChanged( mObjectContainer.fGetSelectionList( ) );

			mOnResourceLoaded.fFromMethod<tAnimPackTree, &tAnimPackTree::fOnResourceLoaded>( this );

			mPackFilter->Connect( 
				wxEVT_COMMAND_TEXT_UPDATED, 
				wxCommandEventHandler( tAnimPackTree::fOnPackFilterUpdate ), NULL, this );
			mAnimFilter->Connect( 
				wxEVT_COMMAND_TEXT_UPDATED, 
				wxCommandEventHandler( tAnimPackTree::fOnAnimFilterUpdate ), NULL, this );
		}

		tSigAnimEdDialog & fSigAnimEdDialog( ) const { return *mAnimEd; }
		const tAnimPackList & fAnimPackList( ) const { return mAnimPackList; }

		b32 fPartialsMatch( ) const { return mPartialsMatch; }
		void fSetPartialsMatch( b32 match ) { mPartialsMatch = match; }
		void fSetAnimationTimeline( tSigAnimTimeline* timeline ) { mAnimTimeline = timeline; }

		//------------------------------------------------------------------------------
		void fRefresh( )
		{
			fOnSelectionChanged( mObjectContainer.fGetSelectionList( ) );
		}

		//------------------------------------------------------------------------------
		virtual void fMarkCurrentAnimPackDirty( )
		{
			mAnimPackList[mSelectedAnimPack].mDirty = true;
			if( mAnimTimeline )
				mAnimTimeline->fMarkTimelineDirty( );
		}

		//------------------------------------------------------------------------------
		virtual void fSaveAllAnimPacks( )
		{
			const u32 packCount = mAnimPackList.fCount( );
			for( u32 p = 0; p < packCount; ++p )
			{
				tAnimPackData & data = mAnimPackList[ p ];
				if( !data.mDirty )
					continue;

				const tFilePathPtr absolutePath = ToolsPaths::fMakeResAbsolute( data.mAnipkPath );
				data.mAnipkFile.fSaveXml( absolutePath, true, true );
				data.mDirty = false;
			}

			mAnimEd->fClearDirty( );
		}

		//------------------------------------------------------------------------------
		virtual f32 fGetAnimTime( const std::string& animName ) const
		{
			tGrowableArray<tSkeletableSgFileRefEntity *> ents;
			mObjectContainer.fCollectSelectedOrOnly( ents, tSkeletableSgFileRefEntity::fIsCursor );

			const u32 entCount = ents.fCount( );
			for( u32 e = 0; e < entCount; ++e )
			{
				const Anim::tAnimatedSkeletonPtr & skeleton = ents[e]->fSkeleton( );
				if( !skeleton )
					continue;

				Anim::tAnimTrackPtr animTrack = skeleton->fFirstTrackWithTag( tStringPtr( animName ) );
				if( !animTrack )
					continue;

				return animTrack->fCurrentTime( );
			}

			return 0;
		}

		//------------------------------------------------------------------------------
		void fWarnAndSaveAnyDirtyAnimPacks( )
		{
			const u32 packCount = mAnimPackList.fCount( );
			for( u32 p = 0; p < packCount; ++p )
			{
				tAnimPackData & pack = mAnimPackList[ p ];

				if( !pack.mDirty )
					continue;

				std::stringstream ss;
				ss	<< "The animation pack [" 
					<< pack.mLongLabel 
					<< "] is about to be removed, but it has unsaved changes." 
					<< std::endl 
					<< std::endl 
					<< "Would you like to save these changes?";

				wxMessageDialog msgBox( this, 
					ss.str( ), 
					"Save Anipk?", wxCENTRE | wxYES_NO | wxYES_DEFAULT | wxICON_EXCLAMATION );

				const int retVal = msgBox.ShowModal( );
				if( retVal != wxID_YES )
					continue;

				const tFilePathPtr absolutePath = ToolsPaths::fMakeResAbsolute( pack.mAnipkPath );
				pack.mAnipkFile.fSaveXml( absolutePath, true );
				pack.mDirty = false;
			}

			mAnimEd->fClearDirty( );
		}

	private:

		//------------------------------------------------------------------------------
		void fOnResourceLoaded( tResource & theResource, b32 success )
		{
			tResourcePtr resPtr( &theResource );
			wxString animFilter = mAnimFilter->GetLineText( 0 );

			Freeze( );

			wxTreeItemIdValue cookie = this;
			wxTreeItemId nextItem = GetFirstChild( GetRootItem( ), cookie );
			while( nextItem.IsOk( ) )
			{
				wxTreeItemId item = nextItem;
				nextItem = GetNextChild( nextItem, cookie );

				tAnimPackTreeData * data = ( tAnimPackTreeData * )GetItemData( item );
				sigassert(data);

				if( data->mLoading && data->mAnimPackResource == resPtr )
				{
					if( fAddAnimations( item, resPtr, animFilter ) )
					{
						SetItemTextColour( item, GetItemTextColour( GetRootItem( ) ) );
						DeleteChildren( item );
					}

					data->mLoading = false;
				}
			}

			Thaw( );
		}

		//------------------------------------------------------------------------------
		void fOnPackFilterUpdate( wxCommandEvent & )
		{
			wxString filter = mPackFilter->GetLineText( 0 );

			Freeze( );

			tGrowableArray<tResourcePtr> packsFound;

			wxTreeItemIdValue cookie = this;

			// Remove any current that don't match and accumulate the registered packs
			wxTreeItemId nextItem = GetFirstChild( GetRootItem( ), cookie );
			while( nextItem.IsOk( ) )
			{
				wxTreeItemId item = nextItem;
				nextItem = GetNextChild( nextItem, cookie );

				tAnimPackTreeData * data = ( tAnimPackTreeData * )GetItemData( item );
				sigassert(data);

				// If it was loading but no longer, then remove it so it gets readded
				if( data->mLoading && !data->mAnimPackResource->fLoading( ) )
				{
					Delete( item );
					continue;
				}

				packsFound.fPushBack( data->mAnimPackResource );
				if( *filter && !StringUtil::fStrStrI( GetItemText( item ), filter ) )
					Delete( item );
			}

			const u32 packCount = mAnimPackIntersection.fCount( );
			for( u32 p = 0; p < packCount; ++p )
			{
				tResourcePtr pack = mAnimPackIntersection[ p ];

				// Was found so we can skip it
				if( packsFound.fFindAndErase( pack ) )
					continue;

				fAddAnimPack( pack, filter );
			}

			SortChildren( GetRootItem( ) );

			Thaw( );
		}

		//------------------------------------------------------------------------------
		void fOnAnimFilterUpdate( wxCommandEvent & )
		{
			wxString animFilter = mAnimFilter->GetLineText( 0 );

			Freeze( );

			tGrowableArray<tResourcePtr> packsToAdd;

			wxTreeItemIdValue cookie = this;
			wxTreeItemId nextPackItem = GetFirstChild( GetRootItem( ), cookie );
			while( nextPackItem.IsOk( ) )
			{
				wxTreeItemId packItem = nextPackItem;
				nextPackItem = GetNextChild( nextPackItem, cookie );

				tAnimPackTreeData * data = ( tAnimPackTreeData * )GetItemData( packItem );
				sigassert(data);

				// Was loading, but now isn't so remove for re-add
				if( data->mLoading && !data->mAnimPackResource->fLoading( ) )
				{
					packsToAdd.fPushBack( data->mAnimPackResource );
					Delete( packItem );
					continue;
				}

				DeleteChildren( packItem );
				fAddAnimations( packItem, data->mAnimPackResource, animFilter );
			}

			wxString packFilter = mPackFilter->GetLineText( 0 );
			const u32 packCount = packsToAdd.fCount( );
			for( u32 p = 0; p < packCount; ++p )
				fAddAnimPack( packsToAdd[ p ], packFilter );

			SortChildren( GetRootItem( ) );

			Thaw( );

		}

		//------------------------------------------------------------------------------
		void fOnAnimSelected( wxTreeEvent & e )
		{
			// Set id to the animation pack of the selected item
			wxTreeItemId id = e.GetItem( );
			wxTreeItemId parentId = GetItemParent( id );
			
			// An animation pack item
			if( id == GetRootItem( ) || parentId == GetRootItem( ) )
			{
				mAnimEd->fClearAnim( );
				return;
			}

			tAnimPackTreeData * data = ( tAnimPackTreeData * )GetItemData( parentId );
			sigassert(data);

			// Force the load so that we can grab the info immediately
			if( data->mLoading )
				data->mAnimPackResource->fBlockUntilLoaded( );

			mSelectedAnimPack = mAnimPackList.fIndexOf( data->mAnimPackResource );
			if( mSelectedAnimPack == -1 )
			{
				mSelectedAnimPack = mAnimPackList.fCount( );
				mAnimPackList.fPushBack( tAnimPackData( data->mAnimPackResource ) );
			}

			tAnimPackData & packData = mAnimPackList[ mSelectedAnimPack ];

			const tAnimPackFile * file = data->mAnimPackResource->fCast<tAnimPackFile>( );
			sigassert( file );

			const tKeyFrameAnimation * anim = file->fFindAnim( tStringPtr( GetItemText( id ) ) );
			sigassert( anim );

			mAnimEd->fSetAnim( 
				packData.mLongLabel, 
				std::string( GetItemText( id ) ), 
				packData.mAnipkFile,
				anim );

			if( mAnimTimeline )
			{
				mAnimTimeline->fSetAnimEvents( 
					packData.mLongLabel, 
					std::string( GetItemText( id ) ), 
					packData.mAnipkFile,
					anim );
			}
		}

		//------------------------------------------------------------------------------
		void fOnAnimActivated( wxTreeEvent & e )
		{
			wxTreeItemId id = e.GetItem( );
			wxTreeItemId parentId = GetItemParent( id );

			// An animation pack item
			if( parentId == GetRootItem( ) )
				return;

			tAnimPackTreeData * data = ( tAnimPackTreeData * )GetItemData( parentId );
			sigassert(data);

			// Wait till it's loaded if it's loading
			data->mAnimPackResource->fBlockUntilLoaded( );

			const tAnimPackFile * file = data->mAnimPackResource->fCast<tAnimPackFile>( );
			sigassert( file );

			const tKeyFrameAnimation * anim = file->fFindAnim( tStringPtr( GetItemText( id ) ) );
			sigassert( anim );

			const b32 oneShot = 
				Input::tKeyboard::fInstance( ).fButtonHeld( Input::tKeyboard::cButtonLShift ) ||
				Input::tKeyboard::fInstance( ).fButtonHeld( Input::tKeyboard::cButtonRShift );

			Anim::tKeyFrameAnimDesc animTrackDesc( anim, Anim::tAnimTrackDesc( .2f, oneShot ? 0.2f : 0.f ).fSetTag( anim->mName->fGetStringPtr( ) ) );

			tGrowableArray<tSkeletableSgFileRefEntity *> ents;
			mObjectContainer.fCollectSelectedOrOnly( ents, tSkeletableSgFileRefEntity::fIsCursor );

			const u32 selectedCount = ents.fCount( );
			for( u32 e = 0; e < selectedCount; ++e )
			{
				tSkeletableSgFileRefEntity * entity = ents[ e ];

				const Anim::tAnimatedSkeletonPtr & skeleton = entity->fSkeleton( );
				if( skeleton.fNull( ) )
					continue;

				// Attempt to time partials to the current clock
				animTrackDesc.mStartTime = 0;
				if( mPartialsMatch && anim->fPartial( ) && skeleton->fTrackCount( ) > 0 )
					animTrackDesc.mStartTime = skeleton->fTrack( 0 ).fCurrentTime( );

				// Create the track and set the tag
				Anim::tAnimTrackPtr track( new Anim::tKeyFrameAnimTrack( animTrackDesc ) );

				// Push!
				skeleton->fPushTrack( track );
				entity->fSetPaused( false );
			}
		}

		//------------------------------------------------------------------------------
		void fOnObjectAddedOrRemoved( tEditableObjectContainer &, const tEntityPtr & )
		{
			tGrowableArray<tSkeletableSgFileRefEntity *> ents;
			mObjectContainer.fCollectSelectedOrOnly( ents, tSkeletableSgFileRefEntity::fIsCursor );

			// If there's only one entity and no selection then update
			if(ents.fCount( ) && !mObjectContainer.fGetSelectionList( ).fCount( ) )
				fOnSelectionChanged( mObjectContainer.fGetSelectionList( ) );
		}

		//------------------------------------------------------------------------------
		void fOnSelectionChanged( tEditorSelectionList & list )
		{
			mAnimPackIntersection.fSetCount( 0 );

			tGrowableArray<tSkeletableSgFileRefEntity *> ents;
			mObjectContainer.fCollectSelectedOrOnly( ents, tSkeletableSgFileRefEntity::fIsCursor );

			// Run over the entities till we find the first one that's of the right type
			const u32 entityCount = ents.fCount( );
			for( u32 e0 = 0; e0 < entityCount; ++e0 )
			{
				tSkeletableSgFileRefEntity * entity0 = ents[ e0 ];

				// Not a sig anim reference entity
				if( !entity0 )
					continue;

				// Run over all the anim packs and add any completely intersecting ones
				const tAnimPackInfo & packInfo = entity0->fAnimPackInfo( );

				const u32 animPackCount0 = packInfo.mAnimPacks.fCount( );
				for( u32 ap0 = 0; ap0 < animPackCount0; ++ap0 )
				{
					tResourcePtr animPack0 = packInfo.mAnimPacks[ ap0 ];

					b32 intersects = true;
					for( u32 e1 = 0; e1 < entityCount; ++e1 )
					{
						// Of course it contains it's own entries
						if( e1 == e0 )
							continue;

						tSkeletableSgFileRefEntity * entity1 = ents[ e1 ];

						// Not a correct entity type
						if( !entity1 )
							continue;

						b32 found = false;
						const tAnimPackInfo & otherPackInfo = entity1->fAnimPackInfo( );
						
						// Run over the other entity's packs
						const u32 animPackCount1 = otherPackInfo.mAnimPacks.fCount( );
						for( u32 ap1 = 0; ap1 < animPackCount1; ++ap1 )	
						{
							if( otherPackInfo.mAnimPacks[ ap1 ] != animPack0 )
								continue;
							
							found = true;
							break;
						}

						// If we didn't find it then no intersection on this pack
						if( !found )
						{
							intersects = false;
							break;
						}
					}

					// Stop searching for this pack because we found an entity
					// without it
					if( !intersects )
						continue;

					// This pack is shared by all entities
					mAnimPackIntersection.fPushBack( animPack0 );
				}

				break;
			}

			fBuildTree( );
		}

		//------------------------------------------------------------------------------
		void fBuildTree ( )
		{
			// Update the tree
			Freeze( );
			DeleteAllItems( );

			// Empty invisible Root
			wxTreeItemId rootItem = AddRoot( "" ); 
			
			wxString filter = mPackFilter->GetLineText( 0 );

			// Add all the packs
			const u32 intersectionCount = mAnimPackIntersection.fCount( );
			for( u32 ap = 0; ap < intersectionCount; ++ap )
				fAddAnimPack( mAnimPackIntersection[ ap ], filter );

			SortChildren( rootItem );

			// Good to go
			Thaw( );
		}

		//------------------------------------------------------------------------------
		void fAddAnimPack( const tResourcePtr & apResource, const char * filter )
		{
			tFilePathPtr resPath = apResource->fGetPath( );

			// Do the filtering
			if( *filter && !StringUtil::fStrStrI( resPath.fCStr( ), filter ) )
				return;

			wxString animFilter = mAnimFilter->GetLineText( 0 );

			// For reading ease we put the name at the front before the path
			std::stringstream ss;
			ss	<< "[" 
				<< StringUtil::fNameFromPath( resPath.fCStr( ), true )
				<< "] "
				<< resPath.fCStr( );

			tAnimPackTreeData * data = new tAnimPackTreeData( apResource );

			wxTreeItemId id = AppendItem( 
				GetRootItem( ), 
				ss.str( ), 
				-1, // image
				-1, // selImage
				data );
			
			// Handle resources that are still loading
			if( data->mLoading )
			{
				SetItemTextColour( id, cLoadingColor );
				SetItemTextColour( AppendItem( id, "Loading..." ), cLoadingColor );
				data->mAnimPackResource->fCallWhenLoaded( mOnResourceLoaded );
				
			}
				
			// Add all the animation names
			else
				fAddAnimations( id, apResource, animFilter );
		}

		//------------------------------------------------------------------------------
		b32 fAddAnimations( 
			const wxTreeItemId & packNode, 
			const tResourcePtr & apResource, 
			const char * filter )
		{
			const tAnimPackFile * file = apResource->fCast<tAnimPackFile>( );
			if( !file )
			{
				log_warning( "fAddAnimations failed on: " << apResource->fGetResourceId().fGetPath().fCStr() );
				return false;
			}

			const u32 animCount = file->mAnims.fCount( );
			for( u32 a = 0; a < animCount; ++a )
			{
				const tStringPtr & name = file->mAnims[a].mName->fGetStringPtr( );
				if( *filter && !StringUtil::fStrStrI( name.fCStr( ), filter ) )
					continue;

				AppendItem( packNode, name.fCStr( ) );
			}
			return true;
		}

	private:
		
		DECLARE_EVENT_TABLE()

		tEditableObjectContainer & mObjectContainer;
		tEditableObjectContainer::tOnObjectAdded::tObserver mOnObjectAddedOrRemoved;
		tEditorSelectionList::tOnSelectionChanged::tObserver mOnSelectionChanged;
		tResource::tOnLoadComplete::tObserver mOnResourceLoaded;

		wxTextCtrl * mPackFilter;
		wxTextCtrl * mAnimFilter;

		tSigAnimEdDialog * mAnimEd;

		tGrowableArray<tResourcePtr> mAnimPackIntersection;

		s32				mSelectedAnimPack;
		b32				mPartialsMatch;
		tAnimPackList	mAnimPackList;
		tSigAnimTimeline* mAnimTimeline;
	};

	BEGIN_EVENT_TABLE(tAnimPackTree, wxTreeCtrl)
		EVT_TREE_SEL_CHANGED(		wxID_ANY,	tAnimPackTree::fOnAnimSelected )
		EVT_TREE_ITEM_ACTIVATED(	wxID_ANY,	tAnimPackTree::fOnAnimActivated )
	END_EVENT_TABLE()
	

	//------------------------------------------------------------------------------
	// tAnimationTreePanel
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	tAnimationTreePanel::tAnimationTreePanel( tWxToolsPanel * parent )
		: tWxToolsPanelTool(parent, "Animations", "Available animations on selected object(s)", "Anims")
		, mAnimTimeline( 0 )
	{
		wxPanel * panel = fGetMainPanel( );
		panel->SetForegroundColour( wxColor( 0xff, 0xff, 0xff ) );

		wxBoxSizer * bottomSizer = new wxBoxSizer( wxHORIZONTAL );
		
		wxTextCtrl * packFilter;
		{
			wxBoxSizer * sizer = new wxBoxSizer( wxVERTICAL );

			packFilter = new wxTextCtrl( panel, wxID_ANY );
			sizer->Add( packFilter, 1, wxEXPAND | wxALL | wxALIGN_CENTER, 5 );

			wxStaticText * title = new wxStaticText( panel, wxID_ANY, "Pack filter" );
			sizer->Add(title, 0, wxDOWN | wxRIGHT | wxLEFT | wxALIGN_CENTER_HORIZONTAL, 1 );
 
			bottomSizer->Add( sizer, wxEXPAND | wxALL | wxALIGN_CENTER );
		}

		wxTextCtrl * animFilter;
		{
			wxBoxSizer * sizer = new wxBoxSizer( wxVERTICAL );

			animFilter = new wxTextCtrl( panel, wxID_ANY );
			sizer->Add( animFilter, 1, wxEXPAND | wxALL | wxALIGN_CENTER, 5 );

			wxStaticText * title = new wxStaticText( panel, wxID_ANY, "Anim filter" );
			sizer->Add(title, 0, wxDOWN | wxRIGHT | wxLEFT | wxALIGN_CENTER_HORIZONTAL, 1 );

			bottomSizer->Add( sizer, wxEXPAND | wxALL | wxALIGN_CENTER );
		}

		mAnimPackTree = new tAnimPackTree( 
			parent->fGuiApp( ).fEditableObjects( ), 
			fGetMainPanel( ),
			&parent->fGuiApp( ).fMainWindow( ),
			packFilter,
			animFilter,
			400 );

		panel->GetSizer( )->Add( mAnimPackTree, 0, wxEXPAND | wxALL, 4 );
		panel->GetSizer( )->Add( bottomSizer, 0, wxEXPAND | wxALL | wxALIGN_CENTER, 5 );

		wxCheckBox * partialsMatch;
		{
			partialsMatch = new wxCheckBox( panel, wxID_ANY, "Match Partials", wxDefaultPosition, wxDefaultSize );
			partialsMatch->Connect( 
				wxEVT_COMMAND_CHECKBOX_CLICKED, 
				wxCommandEventHandler( tAnimationTreePanel::fOnPartialsMatch ), NULL, this );

			panel->GetSizer( )->Add( partialsMatch, 0, wxEXPAND | wxALL | wxALIGN_CENTER, 5 );
		}
	}

	//------------------------------------------------------------------------------
	void tAnimationTreePanel::fMarkForRefresh( )
	{
		mRefresh = true;
	}

	//------------------------------------------------------------------------------
	void tAnimationTreePanel::fToggelAnimEd( )
	{
		mAnimPackTree->fSigAnimEdDialog( ).fToggle( );
	}

	//------------------------------------------------------------------------------
	void tAnimationTreePanel::fWarnAndSaveAnyDirtyAnimPacks( )
	{
		mAnimPackTree->fWarnAndSaveAnyDirtyAnimPacks( );
	}

	//------------------------------------------------------------------------------
	void tAnimationTreePanel::fHandleDialogs( )
	{
		tSigAnimEdDialog & dlg = mAnimPackTree->fSigAnimEdDialog( );
		dlg.fAutoHandleTopMost( ( HWND )fParent( )->fGuiApp( ).fMainWindow( ).GetHWND( ) );
		if( dlg.fIsActive( ) )
			fParent( )->fGuiApp( ).fMainWindow( ).fSetDialogInputActive( true );
	}

	//------------------------------------------------------------------------------
	void tAnimationTreePanel::fOnTick( )
	{
		if( mRefresh )
		{
			mAnimPackTree->fRefresh( );
			mRefresh = false;
		}

		if( mAnimPackTree )
			mAnimPackTree->fSigAnimEdDialog().fOnTick();
	}

	//------------------------------------------------------------------------------
	const tAnimPackList & tAnimationTreePanel::fAnimPackList( )
	{
		return mAnimPackTree->fAnimPackList( );
	}

	//------------------------------------------------------------------------------
	void tAnimationTreePanel::fOnPartialsMatch( wxCommandEvent & e )
	{
		mAnimPackTree->fSetPartialsMatch( e.IsChecked( ) );
	}

	//------------------------------------------------------------------------------
	void tAnimationTreePanel::fSetAnimationTimeline( tSigAnimTimeline* timeline )
	{
		mAnimTimeline = timeline;
		mAnimPackTree->fSetAnimationTimeline( mAnimTimeline );
	}

}
