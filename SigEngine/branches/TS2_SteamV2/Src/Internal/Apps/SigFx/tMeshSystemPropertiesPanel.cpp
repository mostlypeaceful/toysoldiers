#include "SigFxPch.hpp"
#include "tMeshSystemPropertiesPanel.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "tWxSlapOnChoice.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tSigFxMainWindow.hpp"
#include "tWxDirectoryBrowser.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "FileSystem.hpp"
#include "iAssetPlugin.hpp"
#include "tAssetGenScanner.hpp"

namespace Sig
{
	using namespace FX;


	//enum tAction
	//{
	//	cActionRunAssetGen = 1,
	//	cActionRefreshDirectory,
	//	//cActionOpenFile,
	//};

	//class tMeshSystemResourceBrowserTree : public tWxDirectoryBrowser
	//{
	//private:
	//	tSigFxMainWindow*			mEditorWindow;
	//	tGrowableArray< tSigFxMeshSystem* >* mList;
	//	DECLARE_EVENT_TABLE( )

	//public:
	//	tMeshSystemResourceBrowserTree( tSigFxMainWindow* editorWindow, wxWindow* parent, u32 minHeight )
	//		: tWxDirectoryBrowser( parent, minHeight )
	//		, mEditorWindow( editorWindow )
	//		, mList( 0 )
	//	{			
	//		Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( tMeshSystemResourceBrowserTree::fOnItemRightClick ), 0, this );
	//	}

	//	void fSetList( tGrowableArray< tSigFxMeshSystem* >* list ) { mList = list; }

	//	virtual void fOnSelChanged( wxTreeEvent& event, const tFileEntryData& fileEntryData )
	//	{
	//		// this denotes an actual file was selected; load model and display as cursor
	//		const tFilePathPtr meshFile = fileEntryData.fXmlPath( );
	//		// pass that path into our systems to set that as our current mesh!

	//		if( mList )
	//		{
	//			for( u32 i = 0; i < mList->fCount( ); ++i )
	//			{
	//				( *mList )[ i ]->fSetMeshResourceFile( meshFile );
	//			}
	//		}
	//		
	//		mEditorWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
	//		SetItemTextColour( event.GetItem( ), fileEntryData.fGetTextItemColor( ) );
	//	}
	//	virtual wxColour fProvideCustomEntryColor( const tFileEntryData& fileEntryData )
	//	{
	//		if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".sigml" ) )
	//			return wxColour( 0x00, 0x77, 0x00 );
	//		if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".mshml" ) )
	//			return wxColour( 0x00, 0x00, 0x77 );
	//		return wxColour( 0, 0, 0 );
	//	}
	//	virtual b32 fFilterPath( const tFilePathPtr& path )
	//	{
	//		return !Sigml::fIsSigmlFile( path );
	//	}
	//	virtual std::string fConvertDisplayName( const std::string& simplePath, u32& sortValue )
	//	{
	//		std::stringstream displayName;

	//		if( StringUtil::fStrStrI( simplePath.c_str( ), ".mshml" ) )
	//		{
	//			displayName << "[mesh] ";
	//			sortValue = 1;
	//		}
	//		else
	//			displayName << "[scene] ";

	//		displayName << StringUtil::fStripExtension( simplePath.c_str( ) );

	//		return displayName.str( );
	//	}
	//	virtual tFilePathPtr fXmlPathToBinaryPath( const tFilePathPtr& xmlPath )
	//	{
	//		return Sigml::fSigmlPathToSigb( xmlPath );
	//	}
	//	void fOnItemRightClick( wxTreeEvent& event )
	//	{
	//		mRightClickItem = event.GetItem( );
	//		wxMenu menu;
	//		if( !fIsFileItem( event.GetItem( ) ) )
	//			menu.Append( cActionRefreshDirectory, _T("&Refresh directory"));
	//		else
	//		{
	//			menu.Append( cActionRunAssetGen, _T("Run &AssetGen on file and reload"));

	//			const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
	//			//if( fileEntryData && StringUtil::fCheckExtension( fileEntryData->fXmlPath( ).fCStr( ), ".sigml" ) )
	//			//	menu.Append( cActionOpenFile, _T("Open file in editor") );
	//		}
	//		PopupMenu(&menu, event.GetPoint( ).x, event.GetPoint( ).y);
	//		event.Skip( );
	//	}
	//	void fOnAction( wxCommandEvent& event )
	//	{
	//		switch( event.GetId( ) )
	//		{
	//		case cActionRunAssetGen:
	//			{
	//				if( !mRightClickItem.IsOk( ) )
	//					break;
	//				const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
	//				if( fileEntryData )
	//				{
	//					// run asset gen
	//					const tFilePathPtr pathToGenerate = fileEntryData->fXmlPath( );
	//					fRunAssetGen( mRightClickItem, fileEntryData, pathToGenerate );

	//					// now try to reload
	//					const tFilePathPtr pathToReload = fileEntryData->fBinaryPath( );
	//					const tResourceId rid = tResourceId::fMake<tSceneGraphFile>( pathToReload );
	//					tResourcePtr resource = mEditorWindow->fGuiApp( ).fEditableObjects( ).fGetResourceDepot( )->fQuery( rid );
	//					if( !resource->fLoading( ) && resource->fGetLoadCallerIds( ).fCount( ) > 0 )
	//					{
	//						// now we need to notify all entities that contain this resource or any sub-resources in this resource, and have them refresh
	//						tGrowableArray< tEditableSgFileRefEntity* > referenceEntities;
	//						mEditorWindow->fGuiApp( ).fEditableObjects( ).fCollectByType( referenceEntities );
	//						for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
	//							referenceEntities[ i ]->fRefreshDependents( resource, true );
	//						resource->fReload( false );
	//						for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
	//							referenceEntities[ i ]->fRefreshDependents( resource, false );
	//					}
	//				}

	//				// reset to null
	//				mRightClickItem = wxTreeItemId( );
	//			}
	//			break;

	//		case cActionRefreshDirectory:
	//			{
	//				if( !mRightClickItem.IsOk( ) )
	//					break;
	//				const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
	//				if( !fileEntryData )
	//				{
	//					// TODO refresh sub-directory only
	//					fRefresh( );
	//				}

	//				// reset to null
	//				mRightClickItem = wxTreeItemId( );
	//			}
	//			break;

	//		/*case cActionOpenFile:
	//			{
	//				if( !mRightClickItem.IsOk( ) )
	//					break;
	//				const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
	//				if( fileEntryData && StringUtil::fCheckExtension( fileEntryData->fXmlPath( ).fCStr( ), ".sigml" ) )
	//				{
	//					//UnselectAll( );
	//					//mEditorWindow->fOpenDoc( ToolsPaths::fMakeResAbsolute( fileEntryData->fXmlPath( ) ) );
	//				}

	//				// reset to null
	//				mRightClickItem = wxTreeItemId( );
	//			}
	//			break;
	//		*/
	//		default: { log_warning( 0, "Unrecognized action!" ); }
	//			event.Skip( );
	//			break;
	//		}
	//	}
	//	void fRunAssetGen( wxTreeItemId id, const tFileEntryData* fileEntryData, const tFilePathPtr& path )
	//	{
	//		log_line( 0, "Running assetgen on [" << path << "]" );

	//		tAssetGenScanner::fProcessSingleFile( tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectResFolder( ), path ), false, 
	//			fPlatformIdFlag( cCurrentPlatform ) );// | fPlatformIdFlag( mEditorWindow->fPreviewPlatform( ) ) );

	//		SetItemTextColour( id, fileEntryData->fGetTextItemColor( ) );
	//	}
	//};

	//BEGIN_EVENT_TABLE( tMeshSystemResourceBrowserTree, wxTreeCtrl )
	//	EVT_MENU( cActionRunAssetGen,		tMeshSystemResourceBrowserTree::fOnAction )
	//	EVT_MENU( cActionRefreshDirectory,	tMeshSystemResourceBrowserTree::fOnAction )
	//END_EVENT_TABLE( )



	class tMeshSystemEmitterType : public tWxSlapOnChoice
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxMeshSystem* >* mList;
	public:
		tMeshSystemEmitterType( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defChoice = ~0 )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnChoice( parent, label, enumNames, numEnumNames, defChoice )
			, mList( 0 )	{	}

		void fSetList( tGrowableArray< tSigFxMeshSystem* >* list ) { mList = list; }

	protected:

		virtual void fOnControlUpdated( )
		{
			tEmitterType type = ( tEmitterType )fGetValue( );

			if( mList )
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fFxMeshSystem( )->fSetEmitterType( type );
				}
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	};


	class tMeshSystemSyncWithParticleSystemChoice : public tWxSlapOnControl
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxMeshSystem* >* mList;
		wxChoice*		mChoice;

	public:
		tMeshSystemSyncWithParticleSystemChoice( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnControl( parent, label )
			, mList( 0 )
		{
			wxString syncNames[ 1 ] = { wxString( "None" ) };

			mChoice = new wxChoice( 
			parent, 
			wxID_ANY, 
			wxDefaultPosition, 
			wxSize( fControlWidth( ), wxDefaultSize.y ) );

			fAddWindowToSizer( mChoice, true );

			mChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( tMeshSystemSyncWithParticleSystemChoice::fOnControlUpdatedInternal ), NULL, this );

			mChoice->Insert( wxString( "None" ), 0 );
			mChoice->SetSelection( 0 );
		}

		void fOnControlUpdatedInternal( wxCommandEvent& )
		{
			fOnControlUpdated( );
		}
			
		virtual void fEnableControl( ) { mChoice->Enable( ); }
		virtual void fDisableControl( ) { mChoice->Disable( ); }

		void fSetList( tGrowableArray< tSigFxMeshSystem* >* list )
		{
			mList = list;

			tGrowableArray< tSigFxParticleSystem* > systems;
			mSigFxMainWindow->fGuiApp( ).fEditableObjects( ).fCollectAllByType< tSigFxParticleSystem >( systems );

			mChoice->Clear( );
			mChoice->Insert( wxString( "None" ), 0 );
			u32 selectionIdx = 0;
			for( u32 i = 0; i < systems.fCount( ); ++i )
			{
				const tStringPtr& name = systems[ i ]->fParticleSystemName( );

				for( u32 j = 0; mList && j < mList->fCount( ) && j < 1; ++j )
				{
					const tStringPtr& syncedWith = ( *mList )[ j ]->fParticleSystemToSyncWith( );
					if( syncedWith.fCStr( ) == name.fCStr( ) )
						selectionIdx = i+1;
				}

				mChoice->Insert( wxString( name.fCStr( ) ), i+1 );
			}

			mChoice->SetSelection( selectionIdx );
		}

	protected:

		virtual void fOnControlUpdated( )
		{
			u32 idx = ( u32 )mChoice->GetSelection( );
			wxString itemName = mChoice->GetString( idx );
			
			if( mList )
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fSetParticleSystemToSyncWith( tStringPtr( itemName ) );
				}
			}
			
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	};


	class tMeshSystemFlagsCheckListBox :  public tWxSlapOnControl
	{
		tSigFxMainWindow* mSigFxMainWindow;
		wxCheckListBox* mCheckListBox;
		tGrowableArray< tSigFxMeshSystem* >* mList;
	public:

		tMeshSystemFlagsCheckListBox( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnControl( parent, label )
			, mCheckListBox( 0 )
		{
			mCheckListBox = new wxCheckListBox( parent, wxID_ANY, wxDefaultPosition, wxSize( fControlWidth( ), wxDefaultSize.y ), 0, wxLB_EXTENDED | wxLB_NEEDED_SB );
			fAddWindowToSizer( mCheckListBox, true );
			mCheckListBox->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( tMeshSystemFlagsCheckListBox::fOnControlUpdated ), NULL, this );

			for( u32 i = 0; i < FX::cMeshSystemFlagsCount; ++i )
				mCheckListBox->Insert( tToolFxMeshSystemData::mSystemFlagStrings[ i ].fCStr( ), i );
		}

		virtual void fEnableControl( )
		{
			mCheckListBox->Enable( );
		}

		virtual void fDisableControl( )
		{
			mCheckListBox->Disable( );
		}

		void fOnControlUpdated( wxCommandEvent& event )
		{
			for( u32 i = 0; i < mCheckListBox->GetCount( ); ++i )
			{
				for( u32 j = 0; j < mList->fCount( ); ++j )
				{
					( *mList )[ j ]->fFxMeshSystem( )->fRemoveFlag( ( 1 << i ) );
					if( mCheckListBox->IsChecked( i ) )
						( *mList )[ j ]->fFxMeshSystem( )->fAddFlag( ( 1 << i ) );
				}					
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxMeshSystem* >* list )
		{
			mList = list;
			fRefresh( );
		}

		void fRefresh( )
		{
			for( u32 i = 0; i < mCheckListBox->GetCount( ); ++i )
				mCheckListBox->Check( i, false );

			for( u32 i = 0; i < mCheckListBox->GetCount( ); ++i )
			{
				for( u32 j = 0; j < mList->fCount( ); ++j )
				{
					if( ( *mList )[ j ]->fFxMeshSystem( )->fHasFlag( ( 1 << i ) ) )
						mCheckListBox->Check( i );
				}
			}
		}
	};
	


	tMeshSystemPropertiesPanel::tMeshSystemPropertiesPanel( tSigFxMainWindow* fxWindow, tEditableObjectContainer& list, tWxToolsPanel* parent )
		: mSigFxMainWindow( fxWindow )
		, tWxToolsPanelTool( parent, "Mesh System Properties", "Mesh System Properties", "PlaceObj" )
	{
		wxString enumNames[ FX::cEmitterTypeCount ] = {
			wxString( "Point" ),
			wxString( "Sphere" ),
			wxString( "Box" ),
			wxString( "Fountain" ),
			wxString( "Shockwave" ),
			wxString( "Cylinder" ),
		};

		wxBoxSizer* container = new wxBoxSizer( wxVERTICAL );
		fGetMainPanel( )->GetSizer( )->Add( container, 1, wxALIGN_CENTER, 1 );
	
		wxPanel* group = new wxPanel( fGetMainPanel( ), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
		mMeshSystemEmitterChoice = new tMeshSystemEmitterType( mSigFxMainWindow, group, "Emitter Type", enumNames, FX::cEmitterTypeCount, 0 );
		mMeshSystemSyncWithParticleSystem = new tMeshSystemSyncWithParticleSystemChoice( mSigFxMainWindow, group, "Sync With..." );
		mMeshSystemFlagsCheckList = new tMeshSystemFlagsCheckListBox( mSigFxMainWindow, group, "Mesh System Flags" );
		mAttractorIgnoreChecklist = new tIgnoreAttractorChecklist( group, "Ignore Attractors", list, mSigFxMainWindow );

		//wxPanel* browserPanel = new wxPanel( fGetMainPanel( ), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
		//wxStaticText* text = new wxStaticText( fGetMainPanel( ), wxID_ANY, wxString( "Choose Mesh Reference..." ) );
		//text->SetForegroundColour( *wxBLACK );
		//mMeshSystemResourceBrowser = new tMeshSystemResourceBrowserTree( mSigFxMainWindow, browserPanel, 256 );
		//mMeshSystemResourceBrowser->fRefresh( );
		//mMeshSystemResourceBrowser->SetSize( wxSize( 308, 256 ) );

		container->Add( group, 1, wxALIGN_LEFT | wxALL, 4 );
		//container->Add( text, 0, wxALIGN_LEFT | wxLEFT | wxTOP, 8 );
		//container->Add( browserPanel, 0, wxALIGN_RIGHT | wxALL, 4 );

		container->AddSpacer( 8 );
	}

	void tMeshSystemPropertiesPanel::fUpdateSelectedList( tEditorSelectionList& list )
	{
		mMeshSystemsList.fSetCount( 0 );
		list.fCullByType< tSigFxMeshSystem >( mMeshSystemsList );

		if( !mMeshSystemsList.fCount( ) )
			fSetCollapsed( true );
		else
			fSetCollapsed( false );

		mMeshSystemEmitterChoice->fSetList( &mMeshSystemsList );
		mMeshSystemFlagsCheckList->fSetList( &mMeshSystemsList );
		mMeshSystemSyncWithParticleSystem->fSetList( &mMeshSystemsList );
		//mMeshSystemResourceBrowser->fSetList( &mMeshSystemsList );
		mAttractorIgnoreChecklist->fRefresh( );

		if( mMeshSystemsList.fCount( ) )
		{
			u32 emitterType = ( u32 )mMeshSystemsList[ 0 ]->fFxMeshSystem( )->fEmitterType( );
			mMeshSystemEmitterChoice->fSetValue( emitterType );
		}
	}


}


