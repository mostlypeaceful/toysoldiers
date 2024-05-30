#include "SigEdPch.hpp"
#include "tReferenceObjectPanel.hpp"
#include "tSceneGraphFile.hpp"
#include "tEditorAppWindow.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tWxDirectoryBrowser.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "FileSystem.hpp"
#include "iAssetPlugin.hpp"
#include "tAssetGenScanner.hpp"
#include "Fxml.hpp"
#include "FX/tFxFile.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "Editor/tFxmlReferenceEntity.hpp"
#include "MeshSimplify.hpp"
#include "Editor\tRenameFilepathDialog.hpp"

namespace Sig
{
	enum tAction
	{
		cActionRunAssetGen = 1,
		cActionRefreshDirectory,
		cActionOpenFile,
		cActionRenameFile,
		cActionPreviewFile,
	};

	class tSceneRefObjectCursor : public tEditorCursorController, public tPlaceObjectCursorBase
	{
	public:
		tSceneRefObjectCursor( tToolsGuiMainWindow& mainWindow, const tResourceId& rid, const tResourcePtr& sigbResource )
			: tEditorCursorController( mainWindow.fGuiApp( ) )
			, tPlaceObjectCursorBase( mainWindow, tEntityPtr( new tEditableSgFileRefEntity( mainWindow.fGuiApp( ).fEditableObjects( ), rid, sigbResource ) ) )
		{
			mainWindow.fSetStatus( "Place Object [Sigml Reference]" );
		}
		virtual void fOnTick( )
		{
			tPlaceObjectCursorBase::fOnTick( );
		}
		virtual b32 fComputePickRay( Math::tRayf& rayOut )
		{
			return tEditorCursorController::fComputePickRay( rayOut );
		}
		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			if( mEntityMaster )
			{
				mEntityMaster->fDeleteImmediate( );
				mEntityMaster.fRelease( );
			}
		}
	};

	class tEffectRefObjectCursor : public tEditorCursorController, public tPlaceObjectCursorBase
	{
	public:
		tEffectRefObjectCursor( tToolsGuiMainWindow& mainWindow, const tResourcePtr& fxbResource )
			: tEditorCursorController( mainWindow.fGuiApp( ) )
			, tPlaceObjectCursorBase( mainWindow, tEntityPtr( new tFxmlReferenceEntity( mainWindow.fGuiApp( ).fEditableObjects( ), fxbResource ) ) )
		{
			mainWindow.fSetStatus( "Place Object [Effect Reference]" );
		}
		virtual void fOnTick( )
		{
			tPlaceObjectCursorBase::fOnTick( );
		}
		virtual b32 fComputePickRay( Math::tRayf& rayOut )
		{
			return tEditorCursorController::fComputePickRay( rayOut );
		}
		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			if( mEntityMaster )
			{
				mEntityMaster->fDeleteImmediate( );
				mEntityMaster.fRelease( );
			}
		}
	};

	class tResourceBrowserTree : public tWxDirectoryBrowser
	{
	private:
		tEditorAppWindow*			mEditorWindow;

		wxCheckBox* mGetSigmls;
		wxCheckBox* mGetMshmls;
		wxCheckBox* mGetFxmls;
		wxCheckBox* mFullRefresh;
		wxTextCtrl* mFilterText;

		DECLARE_EVENT_TABLE()

	public:
		tResourceBrowserTree( tEditorAppWindow* editorWindow, wxWindow* parent, u32 minHeight )
			: tWxDirectoryBrowser( parent, minHeight )
			, mEditorWindow( editorWindow )
		{			
			Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler(tResourceBrowserTree::fOnItemRightClick), 0, this );

			wxPanel* filtersPanel = new wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
			wxBoxSizer* filtersSizer = new wxBoxSizer( wxVERTICAL );
			filtersPanel->SetSizer( filtersSizer );
			filtersPanel->SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );
			parent->GetSizer( )->Add( filtersPanel, 0, wxALL | wxEXPAND , 5 );
			{
				{
					wxStaticText* staticText = new wxStaticText( filtersPanel, wxID_ANY, wxT("Type Filters"), wxDefaultPosition, wxDefaultSize );
					filtersSizer->Add( staticText, 0, wxALL, 0 );
				}

				wxPanel* searchPanel = new wxPanel( filtersPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE );
				wxBoxSizer* searchSizer = new wxBoxSizer( wxHORIZONTAL );
				searchPanel->SetSizer( searchSizer );
				searchPanel->SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );
				filtersSizer->Add( searchPanel, 1, wxALL | wxEXPAND, 5 );
				{
					// Boxes.
					mGetSigmls = new wxCheckBox( searchPanel, wxID_ANY, wxT("Sigml") );
					mGetSigmls->SetValue( true );
					mGetSigmls->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tResourceBrowserTree::fOnNeedRefresh), NULL, this );
					searchSizer->Add( mGetSigmls, 0, wxALL, 0 );

					mGetMshmls = new wxCheckBox( searchPanel, wxID_ANY, wxT("Mshml") );
					mGetMshmls->SetValue( true );
					mGetMshmls->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tResourceBrowserTree::fOnNeedRefresh), NULL, this );
					searchSizer->Add( mGetMshmls, 0, wxALL, 0 );

					mGetFxmls = new wxCheckBox( searchPanel, wxID_ANY, wxT("Fxml") );
					mGetFxmls->SetValue( true );
					mGetFxmls->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tResourceBrowserTree::fOnNeedRefresh), NULL, this );
					searchSizer->Add( mGetFxmls, 0, wxALL, 0 );
				}

				{
					wxStaticText* staticText = new wxStaticText( filtersPanel, wxID_ANY, wxT("Text Filter"), wxDefaultPosition, wxDefaultSize );
					filtersSizer->Add( staticText, 0, wxALL, 0 );
				}

				mFilterText = new wxTextCtrl( filtersPanel, wxID_ANY, wxEmptyString, wxDefaultPosition );
				mFilterText->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(tResourceBrowserTree::fOnNeedRefresh), NULL, this );
				filtersSizer->Add( mFilterText, 0, wxALL | wxEXPAND, 5 );
			}
		}
		virtual void fOnSelChanged( wxTreeEvent& event, const tFileEntryData& fileEntryData )
		{
			// this denotes an actual file was selected; load model and display as cursor

			// first check if it's the same file we're currently editing; we can't very well allow
			// placing a file within itself (infinite recursion):
			const tFilePathPtr xmlFilePath = fileEntryData.fXmlPath( );
			if( xmlFilePath == ToolsPaths::fMakeResRelative( tFilePathPtr( mEditorWindow->fGuiApp( ).fDocName( ) ) ) )
			{
				event.Veto( );
				wxMessageBox( "You are trying to place the current file you're editing into itself; this would make the universe explode.",
					  "File Placement Recursion", wxOK | wxICON_WARNING );
				mEditorWindow->SetFocus( );
				return;
			}

			const tResourceDepotPtr& resourceDepot = mEditorWindow->fGetResourceDepot( );
			if( StringUtil::fStrStrI( xmlFilePath.fCStr( ), ".fxml" ) )
			{
				const tResourceId rid = tResourceId::fMake<FX::tFxFile>( fileEntryData.fBinaryPath( ) );
				tResourcePtr fxbResource = resourceDepot->fQuery( rid );
				mEditorWindow->fGuiApp( ).fSetCurrentCursor( tEditorCursorControllerPtr( new tEffectRefObjectCursor( *mEditorWindow, fxbResource ) ) );
			}
			else
			{
				const tResourceId rid = tResourceId::fMake<tSceneGraphFile>( fileEntryData.fBinaryPath( ) );
				tResourcePtr sigbResource = resourceDepot->fQuery( rid );
				mEditorWindow->fGuiApp( ).fSetCurrentCursor( tEditorCursorControllerPtr( new tSceneRefObjectCursor( *mEditorWindow, rid, sigbResource ) ) );
			}
				
			SetItemTextColour( event.GetItem( ), fileEntryData.fGetTextItemColor( ) );
		}
		virtual wxColour fProvideCustomEntryColor( const tFileEntryData& fileEntryData )
		{
			if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".sigml" ) )
				return wxColour( 0x00, 0x77, 0x00 );
			if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".mshml" ) )
				return wxColour( 0x00, 0x00, 0x77 );
			if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".fxml" ) )
				return wxColour( 0x77, 0x00, 0x77 );
			return wxColour( 0, 0, 0 );
		}
		virtual b32 fFilterPath( const tFilePathPtr& path )
		{
			// Disregard everything that isn't a reference file.
			if( !Sigml::fIsSigmlFile( path ) && !Fxml::fIsFxmlFile( path ) )
				return true;

			// If it's not any of the desired types, ignore it.
			if( !( (mGetSigmls->GetValue( ) && tSceneGraphFile::fIsSigmlFileExclusive( path ))
				|| (mGetMshmls->GetValue( ) && tSceneGraphFile::fIsMshmlFile( path ))
				|| (mGetFxmls->GetValue( ) && Fxml::fIsFxmlFile( path )) ) )
				return true;

			// Text filter.
			const tFilePathPtr clippedUpToRes = ToolsPaths::fMakeResRelative( path );
			return !mFilterText->IsEmpty( ) && !StringUtil::fStrStrI( clippedUpToRes.fCStr( ), mFilterText->GetLineText( 0 ).c_str( ) );
			
		}
		virtual std::string fConvertDisplayName( const std::string& simplePath, u32& sortValue )
		{
			std::stringstream displayName;

			if( StringUtil::fStrStrI( simplePath.c_str( ), ".mshml" ) )
			{
				displayName << "[mesh] ";
				sortValue = 1;
			}
			else if( StringUtil::fStrStrI( simplePath.c_str( ), ".fxml" ) )
			{
				displayName << "[fx] ";
				sortValue = 2;
			}
			else
				displayName << "[scene] ";

			displayName << StringUtil::fStripExtension( simplePath.c_str( ) );

			return displayName.str( );
		}
		virtual tFilePathPtr fXmlPathToBinaryPath( const tFilePathPtr& xmlPath )
		{
			if( StringUtil::fStrStrI( xmlPath.fCStr( ), ".fxml" ) )
				return FX::tFxFile::fFxmlPathToFxb( xmlPath );
			return Sigml::fSigmlPathToSigb( xmlPath );
		}
		void fOnNeedRefresh( wxCommandEvent& event )
		{
			// False means do not do a full refresh meaning do not gather all the
			// file paths from the hard disk. Saves time.
			fRefresh( false );
		}
		void fOnItemRightClick( wxTreeEvent& event )
		{
			mRightClickItem = event.GetItem( );
			wxMenu menu;
			if( !fIsFileItem( event.GetItem( ) ) )
				menu.Append( cActionRefreshDirectory, _T("&Refresh directory"));
			else
			{
				menu.Append( cActionPreviewFile, _T( "Build and Preview Release" ) );
				menu.Append( cActionRunAssetGen, _T("Run &AssetGen on file and reload"));

				const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
				if( fileEntryData && StringUtil::fCheckExtension( fileEntryData->fXmlPath( ).fCStr( ), ".sigml" ) )
					menu.Append( cActionOpenFile, _T("Open file in editor") );

				//menu.AppendSeparator( );
				//menu.Append( cActionRenameFile, _T("Rename file") );
			}
			PopupMenu(&menu, event.GetPoint( ).x, event.GetPoint( ).y);
			event.Skip( );
		}
		void fOnAction( wxCommandEvent& event )
		{
			switch( event.GetId( ) )
			{
			case cActionRunAssetGen:
				{
					if( !mRightClickItem.IsOk( ) )
						break;
					const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
					if( fileEntryData )
					{
						// run asset gen
						const tFilePathPtr pathToGenerate = fileEntryData->fXmlPath( );
						fRunAssetGen( mRightClickItem, fileEntryData, pathToGenerate );

						const b32 isFx = StringUtil::fStrStrI( pathToGenerate.fCStr( ), ".fxml" )!=0;

						// now try to reload
						const tFilePathPtr pathToReload = fileEntryData->fBinaryPath( );
						const tResourceId rid = tResourceId::fMake( isFx ? Rtti::fGetClassId<FX::tFxFile>( ) : Rtti::fGetClassId<tSceneGraphFile>( ), pathToReload );
						tResourcePtr resource = mEditorWindow->fGetResourceDepot( )->fQuery( rid );
						if( !resource->fLoading( ) && resource->fHasLoadCaller( ) )
						{
							if( dynamic_cast<tPlaceObjectCursorBase*>( mEditorWindow->fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) ) )
								mEditorWindow->fSetSelectionCursor( ); // place-object cursors might have references to fx or meshes, so clear it

							// now we need to notify all entities that contain this resource or any sub-resources in this resource, and have them refresh
							if( isFx )
							{
								// force reload resource and subs
								resource->fReload( false );

								// get all the ents
								tGrowableArray< tFxmlReferenceEntity* > referenceEntities;
								mEditorWindow->fGuiApp( ).fEditableObjects( ).fCollectByType( referenceEntities );

								// now refresh entities
								for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
									referenceEntities[ i ]->fRefreshDependents( resource );
							}
							else
							{
								// get all the ents
								tGrowableArray< tEditableSgFileRefEntity* > referenceEntities;
								mEditorWindow->fGuiApp( ).fEditableObjects( ).fCollectByType( referenceEntities );

								// first unload all of them: this step is necessary for sigmls/mshmls and not fx, bcz
								// sigmls/mshmls have a more complex referencing system
								for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
									referenceEntities[ i ]->fRefreshDependents( resource, true );

								// force reload resource and subs
								resource->fReload( false );

								// now refresh entities
								for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
									referenceEntities[ i ]->fRefreshDependents( resource, false );
							}
						}
					}

					// reset to null
					mRightClickItem = wxTreeItemId( );
				}
				break;

			case cActionRefreshDirectory:
				{
					if( !mRightClickItem.IsOk( ) )
						break;
					const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
					if( !fileEntryData )
					{
						// TODO refresh sub-directory only
						fRefresh( );
					}

					// reset to null
					mRightClickItem = wxTreeItemId( );
				}
				break;

			case cActionOpenFile:
				{
					if( !mRightClickItem.IsOk( ) )
						break;
					const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
					if( fileEntryData && StringUtil::fCheckExtension( fileEntryData->fXmlPath( ).fCStr( ), ".sigml" ) )
					{
						UnselectAll( );
						mEditorWindow->fOpenDoc( ToolsPaths::fMakeResAbsolute( fileEntryData->fXmlPath( ) ) );
					}

					// reset to null
					mRightClickItem = wxTreeItemId( );
				}
				break;

			// UNUSED, use SigMove
			//case cActionRenameFile:
			//	{
			//		if( !mRightClickItem.IsOk( ) )
			//			break;

			//		const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
			//		if( fileEntryData )
			//		{
			//			new tRenameFilepathDialog( this, fileEntryData->fXmlPath( ), false );
			//			fRefresh( );
			//			UnselectAll( );
			//		}
			//	}
			//	break;

			case cActionPreviewFile:
				{
					const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );

					// run asset gen
					const tFilePathPtr pathToGenerate = fileEntryData->fXmlPath( );

					tAssetGenScanner::fProcessSingleFile( pathToGenerate, true, 
						fPlatformIdFlag( cCurrentPlatform ) | fPlatformIdFlag( mEditorWindow->fPreviewPlatform( ) ) );

					// preview					
					const tFilePathPtr relSigbPath = Sigml::fSigmlPathToSigb( ToolsPaths::fMakeResRelative( pathToGenerate ) );
					if( FileSystem::fFileExists( relSigbPath ) )
					{
						log_line( 0, "Previewing current document" );
						std::string cmdLine;
						cmdLine += "-release ";
						cmdLine += std::string( "-preview " ) + relSigbPath.fCStr( );
						ToolsPaths::fLaunchGame( mEditorWindow->fPreviewPlatform( ), cmdLine );
					}

					// reset to null
					mRightClickItem = wxTreeItemId( );
				}
				break;

			default: { log_warning( "Unrecognized action!" ); }
				event.Skip( );
				break;
			}
		}
		void fRunAssetGen( wxTreeItemId id, const tFileEntryData* fileEntryData, const tFilePathPtr& path )
		{
			log_line( 0, "Running assetgen on [" << path << "]" );

			tAssetGenScanner::fProcessSingleFile( tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectResFolder( ), path ), false, 
				fPlatformIdFlag( cCurrentPlatform ) | fPlatformIdFlag( mEditorWindow->fPreviewPlatform( ) ) );

			SetItemTextColour( id, fileEntryData->fGetTextItemColor( ) );
		}
	};

	BEGIN_EVENT_TABLE(tResourceBrowserTree, wxTreeCtrl)
		EVT_MENU(				cActionRunAssetGen,			tResourceBrowserTree::fOnAction)
		EVT_MENU(				cActionRefreshDirectory,	tResourceBrowserTree::fOnAction)
		EVT_MENU(				cActionOpenFile,			tResourceBrowserTree::fOnAction)
		//EVT_MENU(				cActionRenameFile,			tResourceBrowserTree::fOnAction)
		EVT_MENU(				cActionPreviewFile,			tResourceBrowserTree::fOnAction)
	END_EVENT_TABLE()


	tReferenceObjectPanel::tReferenceObjectPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Reference", "Reference Browser", "BrowseObj" )
		, mBrowser( 0 )
		, mCurSel( 0 )
		, mRefreshed( false )
	{
		mBrowser = new tResourceBrowserTree( appWindow, fGetMainPanel( ), 400 );
		//mBrowser->fSetRefreshMT( true );
		fGetMainPanel( )->GetSizer( )->Add( mBrowser, 0, wxEXPAND | wxALL, 4 );
	}

	void tReferenceObjectPanel::fOnTick( )
	{
		if( mRefreshed )
			mBrowser->fHandleRefreshMTCompletion( );
		else
		{
			mRefreshed = true;
			mBrowser->fRefresh( );
		}

		wxTreeItemId curSel = mBrowser->GetSelection( );
		if( curSel != mCurSel )
		{
			mCurSel = curSel;
			fGuiApp( ).fMainWindow( ).SetFocus( );
		}

		//if( mRefreshTimer.fGetElapsedS( ) > 1.f )
		//{
		//	mRefreshTimer.fResetElapsedS( );
		//	mBrowser->fRefresh( );
		//}
	}

	tWxDirectoryBrowser* tReferenceObjectPanel::fGetBrowser( )
	{ 
		return mBrowser; 
	}
}
